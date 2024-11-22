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

#include "curl_slist.h"

struct curl_slist* slist_get_last(struct curl_slist* list)
{
    struct curl_slist* item;

    if (!list) {
        return nullptr;
    }

    item = list;
    while (item->next) {
        item = item->next;
    }

    return item;
}

struct curl_slist* Curl_slist_append_nodup(struct curl_slist* list, char* data)
{
    struct curl_slist* last;
    struct curl_slist* new_item;

    new_item = (struct curl_slist*)malloc(sizeof(struct curl_slist));
    if (!new_item) {
        return nullptr;
    }

    new_item->next = nullptr;
    new_item->data = data;

    if (!list) {
        return new_item;
    }

    last = slist_get_last(list);
    last->next = new_item;
    return list;
}

struct curl_slist* curl_slist_append(struct curl_slist* list, const char* data)
{
    char* dupdata = strdup(data);
    if (!dupdata) {
        return nullptr;
    }

    list = Curl_slist_append_nodup(list, dupdata);
    if (!list) {
        free(dupdata);
    }

    return list;
}

void curl_slist_free_all(struct curl_slist* list)
{
    struct curl_slist* next;
    struct curl_slist* item;

    if (!list) {
        return;
    }

    item = list;
    do {
        next = item->next;
        Curl_safefree(item->data);
        free(item);
        item = next;
    } while (next);
}

void cleanup_encoder_state(struct mime_encoder_state* p)
{
    p->pos = 0;
    p->bufbeg = 0;
    p->bufend = 0;
}

void cleanup_part_content(curl_mimepart* part)
{
    if (part->freefunc) {
        part->freefunc(part->arg);
    }

    part->readfunc = NULL;
    part->seekfunc = NULL;
    part->freefunc = NULL;
    part->arg = (void*)part;
    part->data = NULL;
    part->fp = NULL;
    part->datasize = (curl_off_t)0;
    cleanup_encoder_state(&part->encstate);
    part->kind = MIMEKIND_NONE;
    part->flags &= ~MIME_FAST_READ;
    part->lastreadstatus = 1;
    part->state.state = MIMESTATE_BEGIN;
}

void mime_subparts_unbind(void* ptr)
{
    curl_mime* mime = static_cast<curl_mime*>(ptr);
    if (mime && mime->parent) {
        mime->parent->freefunc = nullptr;
        cleanup_part_content(mime->parent);
        mime->parent = NULL;
    }
}

void mimesetstate(struct mime_state* state, enum mimestate tok, void* ptr)
{
    state->state = tok;
    state->ptr = ptr;
    state->offset = 0;
}

void Curl_mime_initpart(curl_mimepart* part)
{
    memset((char*)part, 0, sizeof(*part));
    part->lastreadstatus = 1;
    mimesetstate(&part->state, MIMESTATE_BEGIN, NULL);
}

void Curl_mime_cleanpart(curl_mimepart* part)
{
    if (part) {
        cleanup_part_content(part);
        curl_slist_free_all(part->curlheaders);
        if (part->flags & MIME_USERHEADERS_OWNER) {
            curl_slist_free_all(part->userheaders);
        }
        Curl_safefree(part->mimetype);
        Curl_safefree(part->name);
        Curl_safefree(part->filename);
        Curl_mime_initpart(part);
    }
}

void curl_mime_free(curl_mime* mime)
{
    curl_mimepart* part;
    if (mime) {
        mime_subparts_unbind(mime);
        while (mime->firstpart) {
            part = mime->firstpart;
            mime->firstpart = part->nextpart;
            Curl_mime_cleanpart(part);
            free(part);
        }
        free(mime);
    }
}