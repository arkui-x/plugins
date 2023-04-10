#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (c) 2023 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import optparse
import os
import sys
import getpass

sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir, os.pardir, os.pardir, os.pardir, "build"))
from scripts.util import build_utils  # noqa: E402

def decode_cfg_line(data):
    data.replace('\n', '').replace('\r', '')
    data = data.strip()
    if (len(data) == 0 or data[0] == '#'):
        return "", ""
    strs = data.split('=')
    if len(strs) <= 1:
        return "", ""
    return strs[0].strip(), strs[1].strip()


def get_param_from_cfg(cfg_name):
    data_dict = {}
    with open(cfg_name) as afile:
        data = afile.readline()
        while data:
            name, value = decode_cfg_line(data)
            if len(name) != 0 and len(value) != 0:
                data_dict[name] = value
                print("sample file name={%s %s}" % (name, value))
            data = afile.readline()
    return data_dict


def decode_code_line(data):
    data.replace('\n', '').replace('\r', '')
    data = data.strip()
    if (not data.startswith("PARAM_MAP")):
        return "", ""
    data_len = len(data)
    data = data[len("PARAM_MAP") + 1 :  data_len - 1]
    data = data.strip()
    strs = data.split(',')
    if len(strs) <= 1:
        return "", ""
    return strs[0].strip(), data[len(strs[0]) + 1:].strip()


def get_param_from_c_code(code_name):
    data_dict = {}
    with open(code_name, "r+") as afile:
        data = afile.readline()
        while data:
            name, value = decode_code_line(data)
            if len(name) != 0 and len(value) != 0:
                data_dict[name] = value
            data = afile.readline()
        afile.truncate(0)
    return data_dict


def write_map_to_code(code_name, data_dict):
    try:
        with open(code_name, "w") as f:
            # start with 0
            f.seek(0)
            # write file header
            f.write('#ifndef _PARAMETER_CFG_H_' + os.linesep)
            f.write('#define _PARAMETER_CFG_H_' + os.linesep)
            f.write('#include <stdint.h>' + os.linesep + os.linesep)
            f.write('#ifdef __cplusplus' + os.linesep)
            f.write('#if __cplusplus' + os.linesep)
            f.write('extern "C" {' + os.linesep)
            f.write('#endif' + os.linesep)
            f.write('#endif' + os.linesep + os.linesep)

            #define struct
            f.write('typedef struct Node_ {' + os.linesep)
            f.write('    const char *name;' + os.linesep)
            f.write('    const char *value;' + os.linesep)
            f.write('} Node;' + os.linesep  + os.linesep)
            f.write('#define PARAM_MAP(name, value) {(const char *)#name, (const char *)#value},')
            f.write(os.linesep  + os.linesep)
            # write data
            f.write('static Node g_paramDefCfgNodes[] = {' + os.linesep)
            for name, value in  data_dict.items():
                print("parameter %s = %s" % (name, value))
                if (value.startswith("\"")):
                    tmp_str = "    PARAM_MAP({0}, {1})".format(name, value)
                    f.write(tmp_str + os.linesep)
                else:
                    tmp_str = "    PARAM_MAP({0}, {1})".format(name, value)
                    f.write(tmp_str + os.linesep)
            f.write('};' + os.linesep + os.linesep)

            #end
            f.write('#ifdef __cplusplus' + os.linesep)
            f.write('#if __cplusplus' + os.linesep)
            f.write('}' + os.linesep)
            f.write('#endif' + os.linesep)
            f.write('#endif' + os.linesep)
            f.write('#endif // _PARAMETER_CFG_H_' + os.linesep)
            f.write(os.linesep)
            f.truncate()
    except IOError:
        print("Error: open or write file %s fail" % {code_name})
    return 0


def add_to_code_dict(code_dict, cfg_dict, high=True):
    for name, value in  cfg_dict.items():
        # check if name exit
        has_key = name in code_dict
        if has_key and high:
            code_dict[name] = value
        elif not has_key:
            code_dict[name] = value
    return code_dict

def fix_parameter_config_file(options):
    out_file = options.output
    print("out_file %s" % (out_file))

    dst_dict = {}
    for source in options.source_files:
        print("source %s" % (source))
        if not os.path.exists(source):
            continue
        src_dict = get_param_from_cfg(source)
        dst_dict = add_to_code_dict(dst_dict, src_dict, False)

    # write extra parameter
    extra_dict = {}
    for parameter in options.extra_parameter_cfgs:
        name, value = decode_cfg_line(parameter)
        if len(name) != 0 and len(value) != 0:
            extra_dict[name] = value
            print("extra parameter {%s %s}" % (name, value))

    dst_dict = add_to_code_dict(dst_dict, extra_dict, True)
    #add time for
    dst_dict["const.product.build.date"] = str(get_current_time('timestamp'))
    dst_dict["const.product.build.user"] = getpass.getuser()
    write_map_to_code(out_file, dst_dict)

def parse_args(args):
    args = build_utils.expand_file_args(args)
    parser = optparse.OptionParser()
    build_utils.add_depfile_option(parser)
    parser.add_option('--output', help='fixed parameter header file')
    parser.add_option('--source-file', action="append",
        type="string", dest="source_files", help='parameter file list')
    parser.add_option('--extra_parameter_cfg', action="append",
        type="string", dest="extra_parameter_cfgs", help='extra parameter')

    options, _ = parser.parse_args(args)
    return options

def main(args):
    options = parse_args(args)
    if not options.extra_parameter_cfgs:
        options.extra_parameter_cfgs = ()

    depfile_deps = (options.source_files)
    fix_parameter_config_file(options)
    build_utils.write_depfile(options.depfile, options.output, depfile_deps, add_pydeps=False)

def get_current_time(time_type='default'):
    from datetime import datetime
    if time_type == 'timestamp':
        return int(datetime.utcnow().timestamp() * 1000)
    if time_type == 'datetime':
        return datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    return datetime.now().replace(microsecond=0)

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
