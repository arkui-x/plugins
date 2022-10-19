/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

/**
 * @addtogroup rawfile
 * @{
 *
 * @brief Provides native functions for the resource manager to operate raw file directories and their raw files.
 *
 * You can use the resource manager to traverse, open, seek, read, and close raw files.
 *
 * @since 8
 * @version 1.0
 */

/**
 * @file raw_file.h
 *
 * @brief Declares native functions related to raw file.
 *
 * For example, you can use the functions to search for, read, and close raw files.
 *
 * @since 8
 * @version 1.0
 */
#ifndef GLOBAL_RAW_FILE_H
#define GLOBAL_RAW_FILE_H

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

struct RawFile;

/**
 * @brief Provides access to a raw file.
 *
 *
 *
 * @since 8
 * @version 1.0
 */
typedef struct RawFile RawFile;

/**
 * @brief Represent the raw file descriptor's info.
 *
 * The RawFileDescriptor is an output parameter in the {@link OH_ResourceManager_GetRawFileDescriptor},
 * and describes the raw file's file descriptor, start position and the length in the HAP.
 *
 * @since 8
 * @version 1.0
 */
typedef struct {
    /** the raw file fd */
    int fd;

    /** the offset from where the raw file starts in the HAP */
    long start;

    /** the length of the raw file in the HAP. */
    long length;
} RawFileDescriptor;

/**
 * @brief Reads a raw file.
 *
 * This function attempts to read data of <b>length</b> bytes from the current offset.
 *
 * @param rawFile Indicates the pointer to {@link RawFile}.
 * @param buf Indicates the pointer to the buffer for receiving the data read.
 * @param length Indicates the number of bytes to read.
 * @return Returns the number of bytes read if any; returns <b>0</b> if the number reaches the end of file (EOF).
 * @since 8
 * @version 1.0
 */
int OH_ResourceManager_ReadRawFile(const RawFile *rawFile, void *buf, size_t length);

/**
 * @brief Uses the 32-bit data type to seek a data read/write position based on the specified offset within a raw file.
 *
 * @param rawFile Indicates the pointer to {@link RawFile}.
 * @param offset Indicates the specified offset.
 * @param whence Indicates the new read/write position, which can be one of the following values: \n
 * <b>0</b>: The new read/write position is set to <b>offset</b>. \n
 * <b>1</b>: The read/write position is set to the current position plus <b>offset</b>. \n
 * <b>2</b>: The read/write position is set to the end of file (EOF) plus <b>offset</b>.
 * @return Returns the new read/write position if the operation is successful; returns <b>(long) -1</b> if an error
 * occurs.
 * @since 8
 * @version 1.0
 */
int OH_ResourceManager_SeekRawFile(const RawFile *rawFile, long offset, int whence);

/**
 * @brief Obtains the raw file length represented by an int32_t.
 *
 * @param rawFile Indicates the pointer to {@link RawFile}.
 * @return Returns the total length of the raw file.
 * @since 8
 * @version 1.0
 */
long OH_ResourceManager_GetRawFileSize(RawFile *rawFile);

/**
 * @brief Closes an opened {@link RawFile} and releases all associated resources.
 *
 *
 *
 * @param rawFile Indicates the pointer to {@link RawFile}.
 * @see OH_ResourceManager_OpenRawFile
 * @since 8
 * @version 1.0
 */
void OH_ResourceManager_CloseRawFile(RawFile *rawFile);

/**
 * @brief Obtains the current offset of a raw file, represented by an int32_t.
 *
 * The current offset of a raw file.
 *
 * @param rawFile Indicates the pointer to {@link RawFile}.
 * @return Returns the current offset of a raw file.
 * @since 8
 * @version 1.0
 */
long OH_ResourceManager_GetRawFileOffset(const RawFile *rawFile);

/**
 * @brief Opens the file descriptor of a raw file based on the int32_t offset and file length.
 *
 * The opened raw file descriptor is used to read the raw file.
 *
 * @param rawFile Indicates the pointer to {@link RawFile}.
 * @param descriptor Indicates the raw file's file descriptor, start position and the length in the HAP.
 * @return Returns true: open the raw file descriptor successfully, false: the raw file is not allowed to access.
 * @since 8
 * @version 1.0
 */
bool OH_ResourceManager_GetRawFileDescriptor(const RawFile *rawFile, RawFileDescriptor &descriptor);

/**
 * @brief Closes the file descriptor of a raw file.
 *
 * The opened raw file descriptor must be released after used to avoid the file descriptor leak.
 *
 * @param descriptor Indicates the raw file's file descriptor, start position and the length in the HAP.
 * @return Returns true: closes the raw file descriptor successfully, false: closes the raw file descriptor failed.
 * @since 8
 * @version 1.0
 */
bool OH_ResourceManager_ReleaseRawFileDescriptor(const RawFileDescriptor &descriptor);

#ifdef __cplusplus
};
#endif

/** @} */
#endif // GLOBAL_RAW_FILE_H
