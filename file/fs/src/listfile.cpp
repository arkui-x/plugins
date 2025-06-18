/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "listfile.h"

#include <fnmatch.h>
#include <memory>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <thread>
#include <tuple>

#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

thread_local OptionArgs g_optionArgs;

static bool CheckSuffix(const vector<string> &suffixs)
{
    for (string suffix : suffixs) {
        if (suffix.length() <= 1 || suffix.length() > MAX_SUFFIX_LENGTH) {
            return false;
        }
        if (suffix[0] != '.') {
            return false;
        }
        for (size_t i = 1; i < suffix.length(); i++) {
            if (!isalnum(suffix[i])) {
                return false;
            }
        }
    }
    return true;
}

static bool GetFileFilterParam(const NVal &argv, FileFilter *filter)
{
    bool ret = false;
    if (argv.HasProp("suffix") && !argv.GetProp("suffix").TypeIs(napi_undefined)) {
        vector<string> suffixs;
        tie(ret, suffixs, ignore) = argv.GetProp("suffix").ToStringArray();
        if (!ret) {
            HILOGE("Failed to get suffix prop.");
            return false;
        }
        if (!CheckSuffix(suffixs) || suffixs.size() == 0) {
            HILOGE("Invalid suffix.");
            return false;
        }
        filter->SetSuffix(suffixs);
    }
    if (argv.HasProp("displayName") && !argv.GetProp("displayName").TypeIs(napi_undefined)) {
        vector<string> displayNames;
        tie(ret, displayNames, ignore) = argv.GetProp("displayName").ToStringArray();
        if (!ret) {
            HILOGE("Failed to get displayname prop.");
            return false;
        }
        if (displayNames.size() == 0) {
            HILOGE("Invalid displayName.");
            return false;
        }
        filter->SetDisplayName(displayNames);
    }
    if (argv.HasProp("fileSizeOver") && !argv.GetProp("fileSizeOver").TypeIs(napi_undefined)) {
        int64_t fileSizeOver = 0;
        tie(ret, fileSizeOver) = argv.GetProp("fileSizeOver").ToInt64();
        if (!ret || fileSizeOver < 0) {
            HILOGE("Failed to get fileSizeOver prop.");
            return false;
        }
        filter->SetFileSizeOver(fileSizeOver);
    }
    if (argv.HasProp("lastModifiedAfter") && !argv.GetProp("lastModifiedAfter").TypeIs(napi_undefined)) {
        double lastModifiedAfter = 0;
        tie(ret, lastModifiedAfter) = argv.GetProp("lastModifiedAfter").ToDouble();
        if (!ret || lastModifiedAfter < 0) {
            HILOGE("Failed to get lastModifiedAfter prop.");
            return false;
        }
        filter->SetLastModifiedAfter(lastModifiedAfter);
    }
    return true;
}

static bool GetOptionParam(const NVal &argv, OptionArgs *optionArgs)
{
    bool succ = false;
    if (argv.HasProp("listNum")) {
        tie(succ, optionArgs->listNum) = argv.GetProp("listNum").ToInt64(0);
        if (!succ || optionArgs->listNum < 0) {
            HILOGE("Failed to get listNum prop");
            return false;
        }
    }

    if (argv.HasProp("recursion")) {
        tie(succ, optionArgs->recursion) = argv.GetProp("recursion").ToBool(false);
        if (!succ) {
            HILOGE("Failed to get recursion prop.");
            return false;
        }
    }

    if (argv.HasProp("filter")) {
        NVal filterProp = argv.GetProp("filter");
        if (!filterProp.TypeIs(napi_undefined)) {
            auto ret = GetFileFilterParam(filterProp, &optionArgs->filter);
            if (!ret) {
                HILOGE("Failed to get filter prop.");
                return false;
            }
        }
    }
    return true;
}

static bool GetOptionArg(napi_env env, const NFuncArg &funcArg, OptionArgs &optionArgs, const string &path)
{
    optionArgs.Clear();
    optionArgs.path = path;
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return true;
    }
    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        auto options = NVal(env, funcArg[NARG_POS::SECOND]);
        if (options.TypeIs(napi_object)) {
            return GetOptionParam(options, &optionArgs);
        } else if (options.TypeIs(napi_undefined) || options.TypeIs(napi_function)) {
            return true;
        }
    }
    return false;
}

static bool FilterSuffix(const vector<string> &suffixs, const struct dirent &filename)
{
    if (filename.d_type == DT_DIR) {
        return true;
    }
    size_t found = string(filename.d_name).rfind('.');
    if (found == std::string::npos) {
        return false;
    }
    string suffixStr = string(filename.d_name).substr(found);
    for (const auto &iter : suffixs) {
        if (iter == suffixStr) {
            return true;
        }
    }
    return false;
}

static bool FilterDisplayname(const vector<string> &displaynames, const struct dirent &filename)
{
    for (const auto &iter : displaynames) {
        int ret = fnmatch(iter.c_str(), filename.d_name, FNM_PATHNAME | FNM_PERIOD);
        if (ret == 0) {
            return true;
        }
    }
    return false;
}

static bool FilterFilesizeOver(const int64_t fFileSizeOver, const struct dirent &filename)
{
    if (fFileSizeOver < 0) {
        return true;
    }
    struct stat info;
    string stPath = (g_optionArgs.path + '/' + string(filename.d_name));
    int32_t res = stat(stPath.c_str(), &info);
    if (res != 0) {
        HILOGE("Failed to stat file.");
        return false;
    }
    if (info.st_size > fFileSizeOver) {
        return true;
    }
    return false;
}

static bool FilterLastModifyTime(const double lastModifiedAfter, const struct dirent &filename)
{
    if (lastModifiedAfter < 0) {
        return true;
    }
    struct stat info;
    string stPath = g_optionArgs.path + '/' + string(filename.d_name);
    int32_t res = stat(stPath.c_str(), &info);
    if (res != 0) {
        HILOGE("Failed to stat file.");
        return false;
    }
    if (static_cast<double>(info.st_mtime) > lastModifiedAfter) {
        return true;
    }
    return false;
}

static bool FilterResult(const struct dirent &filename)
{
    vector<string> fSuffixs = g_optionArgs.filter.GetSuffix();
    if (!FilterSuffix(fSuffixs, filename) && fSuffixs.size() > 0) {
        return false;
    }
    vector<string> fDisplaynames = g_optionArgs.filter.GetDisplayName();
    if (!FilterDisplayname(fDisplaynames, filename) && fDisplaynames.size() > 0) {
        return false;
    }
    int64_t fFileSizeOver = g_optionArgs.filter.GetFileSizeOver();
    if (!FilterFilesizeOver(fFileSizeOver, filename)) {
        return false;
    }
    double fLastModifiedAfter = g_optionArgs.filter.GetLastModifiedAfter();
    if (!FilterLastModifyTime(fLastModifiedAfter, filename)) {
        return false;
    }
    g_optionArgs.countNum++;
    return true;
}

static int32_t FilterFunc(const struct dirent *filename)
{
    if (string_view(filename->d_name) == "." || string_view(filename->d_name) == "..") {
        return FILTER_DISMATCH;
    }

    if (g_optionArgs.countNum < g_optionArgs.listNum || g_optionArgs.listNum == 0) {
        if ((filename->d_type == DT_DIR && g_optionArgs.recursion) || FilterResult(*filename)) {
            return FILTER_MATCH;
        }
    }
    return FILTER_DISMATCH;
}

static void Deleter(struct NameListArg *arg)
{
    for (int i = 0; i < arg->direntNum; i++) {
        free((arg->namelist)[i]);
        (arg->namelist)[i] = nullptr;
    }
    free(arg->namelist);
    arg->namelist = nullptr;
    delete arg;
    arg = nullptr;
}

static int FilterFileRes(const string &path, vector<string> &dirents)
{
    unique_ptr<struct NameListArg, decltype(Deleter)*> pNameList = { new (nothrow) struct NameListArg, Deleter };
    if (!pNameList) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int num = scandir(path.c_str(), &(pNameList->namelist), FilterFunc, nullptr);
    int err = errno;
    if (num < 0) {
        HILOGE("Failed to scan dir");
        return err;
    }
    pNameList->direntNum = num;
    for (int i = 0; i < num; i++) {
        dirents.emplace_back(pNameList->namelist[i]->d_name);
    }
    return ERRNO_NOERR;
}

static int RecursiveFunc(const string &path, vector<string> &dirents)
{
    unique_ptr<struct NameListArg, decltype(Deleter)*> pNameList = { new (nothrow) struct NameListArg, Deleter };
    if (!pNameList) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int num = scandir(path.c_str(), &(pNameList->namelist), FilterFunc, nullptr);
    if (num < 0) {
        HILOGE("Failed to scan dir");
        return errno;
    }
    pNameList->direntNum = num;
    for (int i = 0; i < num; i++) {
        if ((*(pNameList->namelist[i])).d_type == DT_REG) {
            dirents.emplace_back(path + '/' + pNameList->namelist[i]->d_name);
        } else if ((*(pNameList->namelist[i])).d_type == DT_DIR) {
            string pathTemp = g_optionArgs.path;
            g_optionArgs.path += '/' + string((*(pNameList->namelist[i])).d_name);
            int ret = RecursiveFunc(g_optionArgs.path, dirents);
            if (ret != ERRNO_NOERR) {
                return ret;
            }
            g_optionArgs.path = pathTemp;
        }
    }
    return ERRNO_NOERR;
}

static napi_value DoListFileVector2NV(napi_env env, const string &path, vector<string> &dirents, bool recursion)
{
    if (recursion) {
        for (size_t i = 0; i < dirents.size(); i++) {
            dirents[i] = dirents[i].substr(path.length());
        }
    }
    return NVal::CreateArrayString(env, dirents).val_;
}

napi_value ListFile::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succPath, path, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8StringPath();
    if (!succPath) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    if (!GetOptionArg(env, funcArg, g_optionArgs, string(path.get()))) {
        HILOGE("Invalid options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    vector<string> direntsRes;
    int ret = 0;
    ret = g_optionArgs.recursion ? RecursiveFunc(path.get(), direntsRes) : FilterFileRes(path.get(), direntsRes);
    if (ret) {
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    auto res = DoListFileVector2NV(env, string(path.get()), direntsRes, g_optionArgs.recursion);
    g_optionArgs.Clear();
    return res;
}

napi_value ListFile::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succPath, path, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8StringPath();
    if (!succPath) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    OptionArgs optionArgsTmp = {};
    if (!GetOptionArg(env, funcArg, optionArgsTmp, string(path.get()))) {
        HILOGE("Invalid options");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto arg = CreateSharedPtr<ListFileArgs>();
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [arg, optionArgsTmp]() -> NError {
        g_optionArgs = optionArgsTmp;
        int ret = 0;
        ret = g_optionArgs.recursion ? RecursiveFunc(g_optionArgs.path, arg->dirents) :
            FilterFileRes(g_optionArgs.path, arg->dirents);
        g_optionArgs.Clear();
        return ret ? NError(ret) : NError(ERRNO_NOERR);
    };

    auto cbCompl = [arg, optionArgsTmp, path = string(path.get())](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { env, DoListFileVector2NV(env, path, arg->dirents, optionArgsTmp.recursion) };
    };

    NVal thisVar(env, funcArg.GetThisVar());

    if (funcArg.GetArgc() == NARG_CNT::ONE || (funcArg.GetArgc() == NARG_CNT::TWO &&
           !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function))) {
        return NAsyncWorkPromise(env, thisVar).Schedule(LIST_FILE_PRODUCE_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[((funcArg.GetArgc() == NARG_CNT::TWO) ? NARG_POS::SECOND : NARG_POS::THIRD)]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(LIST_FILE_PRODUCE_NAME, cbExec, cbCompl).val_;
    }
}
} // namespace OHOS::FileManagement::ModuleFileIO