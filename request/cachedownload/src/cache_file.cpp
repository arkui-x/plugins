/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cache_file.h"
#include "cache_ram.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
inline int MkdirWrapper(const char* path, int /* mode */) { return _mkdir(path); }
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
inline int MkdirWrapper(const char* path, mode_t mode) { return mkdir(path, mode); }
#endif

namespace OHOS::Plugin::CacheDownload {

std::string FileCache::s_cacheDir_;
std::mutex FileCache::s_mutex_;

namespace {

// ============================================================
// Self-contained SHA-256 implementation.
//
// The on-disk cache file name is the lowercase-hex SHA-256 of the URL,
// matching OpenHarmony's url_hash = sha256(url) convention. The file system
// name must be deterministic from the URL and filesystem-safe; hex is safe.
// A self-contained impl avoids platform dependency (CommonCrypto/OpenSSL).
// ============================================================

// SHA-256 algorithm constants (RFC 6234). All bare literals are named so the
// word-load / padding / rotation logic reads off the algorithm, not raw numbers.
static constexpr int SHA256_WORD_BITS = 32;          // uint32 working word width
static constexpr int SHA256_BITS_PER_BYTE = 8;
static constexpr int SHA256_BYTES_PER_WORD = SHA256_WORD_BITS / SHA256_BITS_PER_BYTE; // 4
static constexpr int SHA256_BITS_PER_NIBBLE = 4;
static constexpr int SHA256_STATE_SIZE = 8;          // H0..H7, eight 32-bit words
static constexpr int SHA256_BLOCK_SIZE = 64;          // bytes per compression block
static constexpr int SHA256_BITS_PER_BLOCK = SHA256_BLOCK_SIZE * SHA256_BITS_PER_BYTE; // 512
static constexpr int SHA256_INITIAL_MSG_COUNT = 16;   // words taken straight from the block
static constexpr int SHA256_PADDING_BOUNDARY = 56;   // block[56..63] reserved for length
static constexpr int SHA256_LENGTH_FIELD_SIZE = SHA256_BLOCK_SIZE - SHA256_PADDING_BOUNDARY; // 8
static constexpr int SHA256_HASH_SIZE = 32;           // digest length in bytes
static constexpr int SHA256_HEX_CHARS_PER_BYTE = 2;   // hex output: 2 chars per byte
static constexpr int SHA256_HEX_OUTPUT_LEN = SHA256_HASH_SIZE * SHA256_HEX_CHARS_PER_BYTE; // 64
static constexpr int SHA256_INTRA_WORD_OFFSET_1 = 1;  // byte 1 within a 4-byte word
static constexpr int SHA256_INTRA_WORD_OFFSET_2 = 2;  // byte 2 within a 4-byte word
static constexpr int SHA256_INTRA_WORD_OFFSET_3 = 3;  // byte 3 within a 4-byte word
static constexpr int SHA256_HIGH_BYTE_SHIFT = SHA256_WORD_BITS - SHA256_BITS_PER_BYTE; // 24
static constexpr uint8_t SHA256_PAD_BYTE = 0x80;
static constexpr uint32_t SHA256_BYTE_MASK = 0xFF;
static constexpr uint32_t SHA256_NIBBLE_MASK = 0xF;

// Message-schedule look-back offsets.
static constexpr int SHA256_MSG_OFFSET_16 = 16;
static constexpr int SHA256_MSG_OFFSET_15 = 15;
static constexpr int SHA256_MSG_OFFSET_7 = 7;
static constexpr int SHA256_MSG_OFFSET_2 = 2;

// Rotation amounts (RFC 6234 §4.1.2).
static constexpr int SHA256_LO_SIGMA0_R1 = 7;
static constexpr int SHA256_LO_SIGMA0_R2 = 18;
static constexpr int SHA256_LO_SIGMA0_R3 = 3;
static constexpr int SHA256_LO_SIGMA1_R1 = 17;
static constexpr int SHA256_LO_SIGMA1_R2 = 19;
static constexpr int SHA256_LO_SIGMA1_R3 = 10;
static constexpr int SHA256_HI_SIGMA0_R1 = 2;
static constexpr int SHA256_HI_SIGMA0_R2 = 13;
static constexpr int SHA256_HI_SIGMA0_R3 = 22;
static constexpr int SHA256_HI_SIGMA1_R1 = 6;
static constexpr int SHA256_HI_SIGMA1_R2 = 11;
static constexpr int SHA256_HI_SIGMA1_R3 = 25;

// Initial hash values H0..H7 (RFC 6234 §5.3.3).
static const uint32_t K_SHA256_H[SHA256_STATE_SIZE] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
};

struct Sha256Ctx {
    uint32_t state[SHA256_STATE_SIZE];
    uint64_t bitlen;
    uint8_t data[SHA256_BLOCK_SIZE];
    uint32_t datalen;
};

// Round constants K0..K63 (RFC 6234 §4.2.2).
static const uint32_t K_SHA256_K[SHA256_BLOCK_SIZE] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

static inline uint32_t Sha256Rotr(uint32_t value, int bits)
{
    return (value >> bits) | (value << (SHA256_WORD_BITS - bits));
}

// Load 16 big-endian words from a 64-byte block, then run the message schedule
// to derive words 16..63. Byte offsets/shifts are explicit named constants — do
// NOT reuse the rotation amounts (SIGMA*_R3) as byte indices; that was the bug.
static void Sha256Transform(Sha256Ctx& ctx, const uint8_t* data)
{
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    uint32_t e;
    uint32_t f;
    uint32_t g;
    uint32_t h;
    uint32_t t1;
    uint32_t t2;
    uint32_t m[SHA256_BLOCK_SIZE];
    for (int i = 0, j = 0; i < SHA256_INITIAL_MSG_COUNT; ++i, j += SHA256_BYTES_PER_WORD) {
        m[i] = (static_cast<uint32_t>(data[j]) << SHA256_HIGH_BYTE_SHIFT) |
               (static_cast<uint32_t>(data[j + SHA256_INTRA_WORD_OFFSET_1]) << (SHA256_HIGH_BYTE_SHIFT - SHA256_BITS_PER_BYTE)) |
               (static_cast<uint32_t>(data[j + SHA256_INTRA_WORD_OFFSET_2]) << SHA256_BITS_PER_BYTE) |
               static_cast<uint32_t>(data[j + SHA256_INTRA_WORD_OFFSET_3]);
    }
    for (int i = SHA256_INITIAL_MSG_COUNT; i < SHA256_BLOCK_SIZE; ++i) {
        uint32_t s0 = Sha256Rotr(m[i - SHA256_MSG_OFFSET_15], SHA256_LO_SIGMA0_R1) ^
                      Sha256Rotr(m[i - SHA256_MSG_OFFSET_15], SHA256_LO_SIGMA0_R2) ^
                      (m[i - SHA256_MSG_OFFSET_15] >> SHA256_LO_SIGMA0_R3);
        uint32_t s1 = Sha256Rotr(m[i - SHA256_MSG_OFFSET_2], SHA256_LO_SIGMA1_R1) ^
                      Sha256Rotr(m[i - SHA256_MSG_OFFSET_2], SHA256_LO_SIGMA1_R2) ^
                      (m[i - SHA256_MSG_OFFSET_2] >> SHA256_LO_SIGMA1_R3);
        m[i] = m[i - SHA256_MSG_OFFSET_16] + s0 + m[i - SHA256_MSG_OFFSET_7] + s1;
    }
    a = ctx.state[0];
    b = ctx.state[1];
    c = ctx.state[2];
    d = ctx.state[3];
    e = ctx.state[4];
    f = ctx.state[5];
    g = ctx.state[6];
    h = ctx.state[7];
    for (int i = 0; i < SHA256_BLOCK_SIZE; ++i) {
        uint32_t capSigma1 = Sha256Rotr(e, SHA256_HI_SIGMA1_R1) ^ Sha256Rotr(e, SHA256_HI_SIGMA1_R2) ^
                      Sha256Rotr(e, SHA256_HI_SIGMA1_R3);
        uint32_t ch = (e & f) ^ ((~e) & g);
        t1 = h + capSigma1 + ch + K_SHA256_K[i] + m[i];
        uint32_t capSigma0 = Sha256Rotr(a, SHA256_HI_SIGMA0_R1) ^ Sha256Rotr(a, SHA256_HI_SIGMA0_R2) ^
                      Sha256Rotr(a, SHA256_HI_SIGMA0_R3);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        t2 = capSigma0 + maj;
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }
    ctx.state[0] += a;
    ctx.state[1] += b;
    ctx.state[2] += c;
    ctx.state[3] += d;
    ctx.state[4] += e;
    ctx.state[5] += f;
    ctx.state[6] += g;
    ctx.state[7] += h;
}

static void Sha256Init(Sha256Ctx& ctx)
{
    ctx.datalen = 0;
    ctx.bitlen = 0;
    for (int i = 0; i < SHA256_STATE_SIZE; ++i) {
        ctx.state[i] = K_SHA256_H[i];
    }
}

static void Sha256Update(Sha256Ctx& ctx, const uint8_t* data, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        ctx.data[ctx.datalen++] = data[i];
        if (ctx.datalen == SHA256_BLOCK_SIZE) {
            Sha256Transform(ctx, ctx.data);
            ctx.bitlen += SHA256_BITS_PER_BLOCK;
            ctx.datalen = 0;
        }
    }
}

static void Sha256Final(Sha256Ctx& ctx, uint8_t out[SHA256_HASH_SIZE])
{
    uint32_t i = ctx.datalen;
    ctx.data[i++] = SHA256_PAD_BYTE;
    if (ctx.datalen < SHA256_PADDING_BOUNDARY) {
        while (i < SHA256_PADDING_BOUNDARY) { ctx.data[i++] = 0; }
    } else {
        while (i < SHA256_BLOCK_SIZE) { ctx.data[i++] = 0; }
        Sha256Transform(ctx, ctx.data);
        std::fill(ctx.data, ctx.data + SHA256_PADDING_BOUNDARY, 0);
    }
    ctx.bitlen += static_cast<uint64_t>(ctx.datalen) * SHA256_BITS_PER_BYTE;
    // Append message length as a 64-bit big-endian value in the final 8 bytes.
    for (int b = 0; b < SHA256_LENGTH_FIELD_SIZE; ++b) {
        ctx.data[SHA256_BLOCK_SIZE - 1 - b] =
            static_cast<uint8_t>(ctx.bitlen >> (b * SHA256_BITS_PER_BYTE));
    }
    Sha256Transform(ctx, ctx.data);
    // Emit state words big-endian: byte i of word j is at (j*4 + i).
    for (i = 0; i < SHA256_BYTES_PER_WORD; ++i) {
        for (int j = 0; j < SHA256_STATE_SIZE; ++j) {
            out[i + j * SHA256_BYTES_PER_WORD] = static_cast<uint8_t>(
                (ctx.state[j] >> (SHA256_HIGH_BYTE_SHIFT - i * SHA256_BITS_PER_BYTE)) & SHA256_BYTE_MASK);
        }
    }
}

// Returns the lowercase-hex SHA-256 of the input string.
std::string Sha256Hex(const std::string& input)
{
    Sha256Ctx ctx;
    Sha256Init(ctx);
    Sha256Update(ctx, reinterpret_cast<const uint8_t*>(input.data()), input.size());
    uint8_t hash[SHA256_HASH_SIZE];
    Sha256Final(ctx, hash);
    static const char hex[] = "0123456789abcdef";
    std::string out(SHA256_HEX_OUTPUT_LEN, '0');
    for (size_t i = 0; i < SHA256_HASH_SIZE; ++i) {
        out[i * SHA256_HEX_CHARS_PER_BYTE] = hex[(hash[i] >> SHA256_BITS_PER_NIBBLE) & SHA256_NIBBLE_MASK];
        out[i * SHA256_HEX_CHARS_PER_BYTE + 1] = hex[hash[i] & SHA256_NIBBLE_MASK];
    }
    return out;
}

// POSIX directory permissions: rwxr-xr-x
static constexpr mode_t CACHE_DIR_PERMS = 0755;

// Recursively creates a directory (like mkdir -p), POSIX-only path.
bool MakeDirsRecursive(const std::string& path)
{
#ifdef _WIN32
    return false;
#else
    if (path.empty()) {
        return false;
    }
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        return (st.st_mode & S_IFDIR) != 0;
    }
    size_t pos = path.find_last_of('/');
    if (pos != std::string::npos && pos > 0) {
        if (!MakeDirsRecursive(path.substr(0, pos))) {
            return false;
        }
    }
    return MkdirWrapper(path.c_str(), CACHE_DIR_PERMS) == 0 || errno == EEXIST;
#endif
}

} // namespace

bool FileCache::InitCacheDir(const std::string& cacheDir)
{
    std::lock_guard<std::mutex> lock(s_mutex_);
    s_cacheDir_ = cacheDir;

    // Ensure directory exists (recursively, like mkdir -p).
    if (!s_cacheDir_.empty()) {
#ifdef _WIN32
        MkdirWrapper(s_cacheDir_.c_str(), 0);
#else
        MakeDirsRecursive(s_cacheDir_);
#endif
        // Verify it's accessible
        struct stat st;
        if (stat(s_cacheDir_.c_str(), &st) == 0 && (st.st_mode & S_IFDIR)) {
            return true;
        }
    }
    return false;
}

std::string FileCache::CacheDir()
{
    std::lock_guard<std::mutex> lock(s_mutex_);
    return s_cacheDir_;
}

std::string FileCache::FilePath(const std::string& taskId)
{
    std::lock_guard<std::mutex> lock(s_mutex_);
    if (s_cacheDir_.empty()) {
        return "";
    }
    // On-disk file name = lowercase-hex SHA-256 of the URL (taskId), matching
    // the OH url_hash convention. Hex is filesystem-safe; no encoding needed.
    return s_cacheDir_ + "/" + Sha256Hex(taskId) + FINISH_SUFFIX;
}

bool FileCache::WriteToFile(const std::string& taskId,
                            const uint8_t* data,
                            size_t size)
{
    std::string path = FilePath(taskId);
    if (path.empty() || data == nullptr || size == 0) {
        return false;
    }

    // Write to a temp file first
    std::string tmpPath = path + ".tmp";

    std::ofstream file(tmpPath, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }

    file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
    file.close();

    if (file.fail()) {
        std::remove(tmpPath.c_str());
        return false;
    }

    // Rename temp file to final file (atomic on most filesystems)
    if (std::rename(tmpPath.c_str(), path.c_str()) != 0) {
        std::remove(tmpPath.c_str());
        return false;
    }

    return true;
}

std::unique_ptr<RamCache> FileCache::ReadFromFile(const std::string& taskId,
                                                  SpaceManager& spaceMgr,
                                                  size_t sizeHint)
{
    std::string path = FilePath(taskId);
    if (path.empty()) {
        return nullptr;
    }

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return nullptr;
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    size_t allocSize = sizeHint > 0 ? sizeHint : fileSize;
    auto cache = std::make_unique<RamCache>(taskId, spaceMgr, allocSize);

    // Read in chunks to avoid large allocations
    constexpr size_t CHUNK_SIZE = 64 * 1024; // 64 KB
    std::vector<uint8_t> buffer(CHUNK_SIZE);

    while (file.good()) {
        file.read(reinterpret_cast<char*>(buffer.data()), CHUNK_SIZE);
        size_t bytesRead = static_cast<size_t>(file.gcount());
        if (bytesRead > 0) {
            cache->Write(buffer.data(), bytesRead);
        }
    }

    cache->FinishWrite();
    return cache;
}

void FileCache::RemoveFile(const std::string& taskId)
{
    std::string path = FilePath(taskId);
    if (!path.empty()) {
        std::remove(path.c_str());
        // Also remove any temp file that might be left
        std::string tmpPath = path + ".tmp";
        std::remove(tmpPath.c_str());
    }
}

void FileCache::RemoveFileByName(const std::string& name)
{
    std::lock_guard<std::mutex> lock(s_mutex_);
    if (s_cacheDir_.empty()) {
        return;
    }
    // Use the name directly as the on-disk file name (it is already the
    // sha256 hex hash). Do NOT re-hash — that would produce a non-existent
    // path (the double-hash bug fixed in this change).
    std::string path = s_cacheDir_ + "/" + name + FINISH_SUFFIX;
    std::remove(path.c_str());
    std::string tmpPath = path + ".tmp";
    std::remove(tmpPath.c_str());
}

std::string FileCache::HashKey(const std::string& taskId)
{
    return Sha256Hex(taskId);
}

uint64_t FileCache::FileSize(const std::string& taskId)
{
    std::string path = FilePath(taskId);
    if (path.empty()) {
        return 0;
    }

    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        return static_cast<uint64_t>(st.st_size);
    }
    return 0;
}

bool FileCache::Exists(const std::string& taskId)
{
    std::string path = FilePath(taskId);
    if (path.empty()) {
        return false;
    }

    struct stat st;
    return stat(path.c_str(), &st) == 0 && (st.st_mode & S_IFREG);
}

namespace {

void ProcessCachedFilePosix(const std::string& dir, const std::string& fileName,
                            const std::string& suffix,
                            std::vector<std::pair<std::string, uint64_t>>& result)
{
    if (fileName.size() <= suffix.size() ||
        fileName.compare(fileName.size() - suffix.size(), suffix.size(), suffix) != 0) {
        return;
    }
    std::string taskId = fileName.substr(0, fileName.size() - suffix.size());
    // Stat the file directly. Calling FileSize(taskId) would
    // re-hash the hash (double-hash), producing a non-existent
    // path and always returning 0.
    std::string fullPath = dir + "/" + fileName;
    struct stat st;
    uint64_t size = 0;
    if (stat(fullPath.c_str(), &st) == 0) {
        size = static_cast<uint64_t>(st.st_size);
    }
    result.emplace_back(taskId, size);
}

#ifdef _WIN32
void ProcessCachedFileWin32(const std::string& fileName, const std::string& suffix,
                            std::vector<std::pair<std::string, uint64_t>>& result,
                            const WIN32_FIND_DATAA& findData)
{
    if (fileName.size() <= suffix.size()) {
        return;
    }
    // File name (minus _F) IS the task id (sha256 hex). No decoding.
    std::string taskId = fileName.substr(0, fileName.size() - suffix.size());
    uint64_t size = static_cast<uint64_t>(
        (static_cast<uint64_t>(findData.nFileSizeHigh) << 32) |
        findData.nFileSizeLow);
    result.emplace_back(taskId, size);
}
#endif

} // namespace

std::vector<std::pair<std::string, uint64_t>> FileCache::ListCachedFiles()
{
    std::vector<std::pair<std::string, uint64_t>> result;

    std::string dir = CacheDir();
    if (dir.empty()) {
        return result;
    }

    // Use platform-specific directory listing
#ifdef _WIN32
    std::string pattern = dir + "\\*" + FINISH_SUFFIX;
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        std::string suffix = FINISH_SUFFIX;
        do {
            std::string fileName = findData.cFileName;
            ProcessCachedFileWin32(fileName, suffix, result, findData);
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
#else
    // POSIX: enumerate finished cache files. The file name (minus "_F") is the
    // sha256 hex of the URL; no decoding is needed or possible.
    DIR* dp = opendir(dir.c_str());
    if (dp != nullptr) {
        struct dirent* entry;
        std::string suffix = FINISH_SUFFIX;
        while ((entry = readdir(dp)) != nullptr) {
            std::string fileName = entry->d_name;
            ProcessCachedFilePosix(dir, fileName, suffix, result);
        }
        closedir(dp);
    }
#endif
    return result;
}

} // namespace OHOS::Plugin::CacheDownload
