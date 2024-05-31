/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * SPDX-License-Identifier: curl
 *
 ***************************************************************************/

#include <curl/curl.h>
#include <string>

#define MIME_BOUNDARY_DASHES 24
#define MIME_RAND_BOUNDARY_CHARS 22
#define ENCODING_BUFFER_SIZE 256
#define MIME_USERHEADERS_OWNER (1 << 0)
#define MIME_FAST_READ (1 << 2)
#define MIME_BOUNDARY_LEN (MIME_BOUNDARY_DASHES + MIME_RAND_BOUNDARY_CHARS)
#define Curl_safefree(ptr) \
    do {                   \
        free((ptr));       \
        (ptr) = nullptr;   \
    } while (0)

enum mimestate {
    MIMESTATE_BEGIN,
    MIMESTATE_CURLHEADERS,
    MIMESTATE_USERHEADERS,
    MIMESTATE_EOH,
    MIMESTATE_BODY,
    MIMESTATE_BOUNDARY1,
    MIMESTATE_BOUNDARY2,
    MIMESTATE_CONTENT,
    MIMESTATE_END,
    MIMESTATE_LAST
};

enum mimekind { MIMEKIND_NONE = 0, MIMEKIND_DATA, MIMEKIND_FILE, MIMEKIND_CALLBACK, MIMEKIND_MULTIPART, MIMEKIND_LAST };

struct mime_state {
    enum mimestate state;
    void* ptr;
    curl_off_t offset;
};

struct curl_mime {
    curl_mimepart* parent;
    curl_mimepart* firstpart;
    curl_mimepart* lastpart;
    char boundary[MIME_BOUNDARY_LEN + 1];
    struct mime_state state;
};

struct mime_encoder {
    const char* name;
    size_t (*encodefunc)(char* buffer, size_t size, bool ateof, curl_mimepart* part);
    curl_off_t (*sizefunc)(curl_mimepart* part);
};

struct mime_encoder_state {
    size_t pos;
    size_t bufbeg;
    size_t bufend;
    char buf[ENCODING_BUFFER_SIZE];
};

struct curl_mimepart {
    curl_mime* parent;
    curl_mimepart* nextpart;
    enum mimekind kind;
    unsigned int flags;
    char* data;
    curl_read_callback readfunc;
    curl_seek_callback seekfunc;
    curl_free_callback freefunc;
    void* arg;
    FILE* fp;
    struct curl_slist* curlheaders;
    struct curl_slist* userheaders;
    char* mimetype;
    char* filename;
    char* name;
    curl_off_t datasize;
    struct mime_state state;
    const struct mime_encoder* encoder;
    struct mime_encoder_state encstate;
    size_t lastreadstatus;
};

struct curl_slist* curl_slist_append(struct curl_slist* list, const char* data);

struct curl_slist* Curl_slist_append_nodup(struct curl_slist* list, char* data);

struct curl_slist* slist_get_last(struct curl_slist* list);

void curl_slist_free_all(struct curl_slist* list);

void curl_mime_free(curl_mime* mime);

void mime_subparts_unbind(void* ptr);

void cleanup_part_content(curl_mimepart* part);

void cleanup_encoder_state(struct mime_encoder_state* p);

void Curl_mime_cleanpart(curl_mimepart* part);

void Curl_mime_initpart(curl_mimepart* part);

void mimesetstate(struct mime_state* state, enum mimestate tok, void* ptr);