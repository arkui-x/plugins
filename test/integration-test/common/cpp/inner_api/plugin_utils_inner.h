/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Mock plugin_utils_inner - simplified CHECK_NULL macros without event_handler.
 */
#ifndef MOCK_PLUGIN_UTILS_INNER_H
#define MOCK_PLUGIN_UTILS_INNER_H
#define CHECK_NULL_VOID(ptr) do { if (!(ptr)) return; } while (0)
#define CHECK_NULL_RETURN(ptr, ret) do { if (!(ptr)) return ret; } while (0)
#define CHECK_NULL_VOID_DELREF(ptr, env, cls) do { if (!(ptr)) { env->DeleteLocalRef(cls); return; } } while (0)
#endif
