/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Mock NAPI function implementations with MockNapiValue system.
 * Supports JS object property read/write, value extraction, arrays,
 * callbacks, and wrap/unwrap for testing NAPI layer code.
 */
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "inner_api/plugin_utils_inner.h"
#include <cstring>
#include <memory>
#include <map>
#include <android/log.h>

#define LOG_TAG "NapiMock"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ===== MockNapiValue =====

enum class MockNapiValueType { INT32, DOUBLE, INT64, UINT32, STRING, BOOL, ARRAY, OBJECT, EXTERNAL, UNDEFINED, NULL_VAL, ARRAYBUFFER };

struct MockNapiValue {
    MockNapiValueType type = MockNapiValueType::UNDEFINED;
    int32_t intVal = 0;
    double doubleVal = 0.0;
    int64_t int64Val = 0;
    uint32_t uint32Val = 0;
    std::string strVal;
    bool boolVal = false;
    std::vector<MockNapiValue*> arrayElements;
    std::map<std::string, MockNapiValue*> properties;
    void* externalData = nullptr;
    napi_callback constructor = nullptr;
    std::vector<napi_property_descriptor> propertyDescs;
    std::vector<uint8_t> bufferData;
};

// ===== Global State =====

static std::vector<std::unique_ptr<MockNapiValue>> g_allValues;
static std::map<napi_value, void*> g_wrapMap;

// Custom global / callable function result for collections module
static MockNapiValue* g_customGlobal = nullptr;
static MockNapiValue* g_callFunctionResult = nullptr;

// Result capture state
static std::vector<std::string> g_resultUris;
static int g_errorCode = -1;
static bool g_deferredResolved = false;

// Callback info state
static struct {
    size_t argc = 0;
    napi_value argv[10] = {nullptr};
    napi_value thisVar = nullptr;
} g_cbInfo;

// ===== Internal helpers =====

static MockNapiValue* allocValue() {
    auto val = std::make_unique<MockNapiValue>();
    MockNapiValue* raw = val.get();
    g_allValues.push_back(std::move(val));
    return raw;
}

static MockNapiValue* toMock(napi_value v) {
    return static_cast<MockNapiValue*>(v);
}

// ===== Mock Reset & Query =====

void MockNapiReset() {
    g_allValues.clear();
    g_wrapMap.clear();
    g_resultUris.clear();
    g_errorCode = -1;
    g_deferredResolved = false;
    g_cbInfo.argc = 0;
    memset(g_cbInfo.argv, 0, sizeof(g_cbInfo.argv));
    g_cbInfo.thisVar = nullptr;
    g_customGlobal = nullptr;
    g_callFunctionResult = nullptr;
}

std::vector<std::string> GetMockResultUris() { return g_resultUris; }
int GetMockResultErrorCode() { return g_errorCode; }
bool WasDeferredResolved() { return g_deferredResolved; }

// ===== Mock Object Construction =====

void MockSetCallbackArgs(size_t argc, napi_value* argv, napi_value thisVar) {
    g_cbInfo.argc = argc;
    memset(g_cbInfo.argv, 0, sizeof(g_cbInfo.argv));
    for (size_t i = 0; i < argc && i < 10; i++) {
        g_cbInfo.argv[i] = argv[i];
    }
    g_cbInfo.thisVar = thisVar;
}

napi_value MockCreateObject() {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::OBJECT;
    return static_cast<napi_value>(val);
}

napi_value MockCreateInt32(int32_t value) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::INT32;
    val->intVal = value;
    return static_cast<napi_value>(val);
}

napi_value MockCreateString(const std::string& str) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::STRING;
    val->strVal = str;
    return static_cast<napi_value>(val);
}

napi_value MockCreateBool(bool value) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::BOOL;
    val->boolVal = value;
    return static_cast<napi_value>(val);
}

napi_value MockCreateStringArray(const std::vector<std::string>& strings) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::ARRAY;
    for (const auto& s : strings) {
        MockNapiValue* elem = allocValue();
        elem->type = MockNapiValueType::STRING;
        elem->strVal = s;
        val->arrayElements.push_back(elem);
    }
    return static_cast<napi_value>(val);
}

void MockSetProperty(napi_value obj, const char* key, napi_value value) {
    MockNapiValue* m = toMock(obj);
    if (m) {
        m->properties[key] = toMock(value);
    }
}

bool MockIsWrapped(napi_value js_object) {
    return g_wrapMap.find(js_object) != g_wrapMap.end();
}

// ===== NAPI Function Implementations =====

// --- Object creation ---

napi_status napi_create_object(napi_env env, napi_value* result) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::OBJECT;
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

napi_status napi_create_int32(napi_env env, int32_t value, napi_value* result) {
    g_errorCode = value;
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::INT32;
    val->intVal = value;
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

napi_status napi_create_string_utf8(napi_env env, const char* str, size_t length, napi_value* result) {
    if (str) {
        if (length == static_cast<size_t>(-1)) {
            g_resultUris.emplace_back(str);
        } else {
            g_resultUris.emplace_back(str, length);
        }
    }
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::STRING;
    if (str) {
        if (length == static_cast<size_t>(-1)) {
            val->strVal = std::string(str);
        } else {
            val->strVal = std::string(str, length);
        }
    }
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

napi_status napi_create_array(napi_env env, napi_value* result) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::ARRAY;
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

napi_status napi_create_array_with_length(napi_env env, size_t length, napi_value* result) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::ARRAY;
    val->arrayElements.resize(length);
    for (size_t i = 0; i < length; i++) {
        val->arrayElements[i] = nullptr;
    }
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

napi_status napi_create_double(napi_env env, double value, napi_value* result) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::DOUBLE;
    val->doubleVal = value;
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

napi_status napi_create_external(napi_env env, void* data, napi_finalize finalize_cb,
    void* finalize_hint, napi_value* result) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::EXTERNAL;
    val->externalData = data;
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

// --- Promise ---

napi_status napi_create_promise(napi_env env, napi_deferred* deferred, napi_value* promise) {
    MockNapiValue* defVal = allocValue();
    MockNapiValue* promVal = allocValue();
    if (deferred) *deferred = static_cast<napi_deferred>(defVal);
    if (promise) *promise = static_cast<napi_value>(promVal);
    return napi_ok;
}

napi_status napi_resolve_deferred(napi_env env, napi_deferred deferred, napi_value resolution) {
    g_deferredResolved = true;
    return napi_ok;
}

// --- Object property operations ---

napi_status napi_set_element(napi_env env, napi_value array, uint32_t index, napi_value value) {
    MockNapiValue* arr = toMock(array);
    if (arr && arr->type == MockNapiValueType::ARRAY) {
        if (index >= arr->arrayElements.size()) {
            arr->arrayElements.resize(index + 1, nullptr);
        }
        arr->arrayElements[index] = toMock(value);
    }
    return napi_ok;
}

napi_status napi_set_named_property(napi_env env, napi_value object, const char* key, napi_value value) {
    MockNapiValue* obj = toMock(object);
    if (obj) {
        obj->properties[key] = toMock(value);
    }
    return napi_ok;
}

napi_status napi_has_named_property(napi_env env, napi_value object, const char* utf8name, bool* result) {
    if (result) {
        MockNapiValue* obj = toMock(object);
        *result = obj && obj->properties.find(utf8name) != obj->properties.end();
    }
    return napi_ok;
}

napi_status napi_get_named_property(napi_env env, napi_value object, const char* utf8name, napi_value* result) {
    if (result) {
        MockNapiValue* obj = toMock(object);
        if (obj) {
            auto it = obj->properties.find(utf8name);
            if (it != obj->properties.end()) {
                *result = static_cast<napi_value>(it->second);
            } else {
                MockNapiValue* val = allocValue();
                val->type = MockNapiValueType::UNDEFINED;
                *result = static_cast<napi_value>(val);
            }
        } else {
            MockNapiValue* val = allocValue();
            val->type = MockNapiValueType::UNDEFINED;
            *result = static_cast<napi_value>(val);
        }
    }
    return napi_ok;
}

// --- Value extraction ---

napi_status napi_get_value_string_utf8(napi_env env, napi_value value, char* buf,
    size_t bufsize, size_t* result) {
    MockNapiValue* v = toMock(value);
    if (v && v->type == MockNapiValueType::STRING) {
        size_t len = v->strVal.length();
        if (buf == nullptr) {
            // First call: return string length
            if (result) *result = len;
        } else {
            // Second call: copy string content
            size_t copyLen = (bufsize > 0) ? ((bufsize - 1 < len) ? bufsize - 1 : len) : 0;
            memcpy(buf, v->strVal.c_str(), copyLen);
            buf[copyLen] = '\0';
            if (result) *result = copyLen;
        }
    } else {
        if (result) *result = 0;
    }
    return napi_ok;
}

napi_status napi_get_value_int32(napi_env env, napi_value value, int32_t* result) {
    if (!result) return napi_ok;
    MockNapiValue* v = toMock(value);
    if (v) {
        switch (v->type) {
            case MockNapiValueType::INT32:  *result = v->intVal; break;
            case MockNapiValueType::DOUBLE: *result = static_cast<int32_t>(v->doubleVal); break;
            case MockNapiValueType::INT64:  *result = static_cast<int32_t>(v->int64Val); break;
            case MockNapiValueType::UINT32: *result = static_cast<int32_t>(v->uint32Val); break;
            default:                        *result = 0; break;
        }
    } else {
        *result = 0;
    }
    return napi_ok;
}

napi_status napi_get_value_bool(napi_env env, napi_value value, bool* result) {
    if (!result) return napi_ok;
    MockNapiValue* v = toMock(value);
    if (!v) { *result = false; return napi_ok; }
    switch (v->type) {
        case MockNapiValueType::BOOL:     *result = v->boolVal; break;
        case MockNapiValueType::INT32:    *result = (v->intVal != 0); break;
        case MockNapiValueType::DOUBLE:   *result = (v->doubleVal != 0.0); break;
        case MockNapiValueType::INT64:    *result = (v->int64Val != 0); break;
        case MockNapiValueType::UINT32:   *result = (v->uint32Val != 0); break;
        case MockNapiValueType::STRING:   *result = !v->strVal.empty(); break;
        case MockNapiValueType::NULL_VAL:
        case MockNapiValueType::UNDEFINED: *result = false; break;
        default:                          *result = true; break;
    }
    return napi_ok;
}

// --- Callback info ---

napi_status napi_get_cb_info(napi_env env, napi_callback_info cbinfo, size_t* argc,
    napi_value* argv, napi_value* this_arg, void** data) {
    if (argc) {
        size_t count = *argc < g_cbInfo.argc ? *argc : g_cbInfo.argc;
        *argc = g_cbInfo.argc;
        if (argv) {
            for (size_t i = 0; i < count; i++) {
                argv[i] = g_cbInfo.argv[i];
            }
        }
    }
    if (this_arg) {
        *this_arg = g_cbInfo.thisVar;
    }
    if (data) {
        *data = nullptr;
    }
    return napi_ok;
}

// --- Wrap/unwrap ---

napi_status napi_unwrap(napi_env env, napi_value js_object, void** result) {
    if (result) {
        auto it = g_wrapMap.find(js_object);
        if (it != g_wrapMap.end()) {
            *result = it->second;
        } else {
            *result = nullptr;
        }
    }
    return napi_ok;
}

napi_status napi_wrap(napi_env env, napi_value js_object, void* native_object,
    napi_finalize finalize_cb, void* finalize_hint, napi_ref* result) {
    g_wrapMap[js_object] = native_object;
    if (result) *result = nullptr;
    return napi_ok;
}

// --- Class definition ---

napi_status napi_define_class(napi_env env, const char* utf8name, size_t length,
    napi_callback constructor, void* data, size_t property_count,
    const napi_property_descriptor* properties, napi_value* result) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::OBJECT;
    val->constructor = constructor;
    for (size_t i = 0; i < property_count; i++) {
        val->propertyDescs.push_back(properties[i]);
    }
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

napi_status napi_define_properties(napi_env env, napi_value object, size_t property_count,
    const napi_property_descriptor* properties) {
    MockNapiValue* obj = toMock(object);
    if (obj) {
        for (size_t i = 0; i < property_count; i++) {
            obj->propertyDescs.push_back(properties[i]);
        }
    }
    return napi_ok;
}

// --- Array operations ---

napi_status napi_is_array(napi_env env, napi_value value, bool* result) {
    if (result) {
        MockNapiValue* v = toMock(value);
        *result = v && v->type == MockNapiValueType::ARRAY;
    }
    return napi_ok;
}

napi_status napi_get_array_length(napi_env env, napi_value value, uint32_t* result) {
    if (result) {
        MockNapiValue* v = toMock(value);
        if (v && v->type == MockNapiValueType::ARRAY) {
            *result = static_cast<uint32_t>(v->arrayElements.size());
        } else {
            *result = 0;
        }
    }
    return napi_ok;
}

napi_status napi_get_element(napi_env env, napi_value value, uint32_t index, napi_value* result) {
    if (result) {
        MockNapiValue* v = toMock(value);
        if (v && v->type == MockNapiValueType::ARRAY && index < v->arrayElements.size()) {
            *result = static_cast<napi_value>(v->arrayElements[index]);
        } else {
            MockNapiValue* val = allocValue();
            val->type = MockNapiValueType::UNDEFINED;
            *result = static_cast<napi_value>(val);
        }
    }
    return napi_ok;
}

// --- Type checking ---

napi_status napi_typeof(napi_env env, napi_value value, napi_valuetype* result) {
    if (result) {
        MockNapiValue* v = toMock(value);
        if (!v) {
            *result = napi_undefined;
            return napi_ok;
        }
        switch (v->type) {
            case MockNapiValueType::INT32:    *result = napi_number;   break;
            case MockNapiValueType::DOUBLE:   *result = napi_number;   break;
            case MockNapiValueType::INT64:    *result = napi_number;   break;
            case MockNapiValueType::UINT32:   *result = napi_number;   break;
            case MockNapiValueType::STRING:   *result = napi_string;  break;
            case MockNapiValueType::BOOL:     *result = napi_boolean; break;
            case MockNapiValueType::ARRAY:
            case MockNapiValueType::OBJECT:
            case MockNapiValueType::ARRAYBUFFER: *result = napi_object;  break;
            case MockNapiValueType::EXTERNAL: *result = napi_external; break;
            case MockNapiValueType::NULL_VAL: *result = napi_null;     break;
            case MockNapiValueType::UNDEFINED:
            default:                          *result = napi_undefined; break;
        }
    }
    return napi_ok;
}

// --- Handle scope (stub) ---

napi_status napi_open_handle_scope(napi_env env, napi_handle_scope* result) {
    if (result) *result = nullptr;
    return napi_ok;
}

napi_status napi_close_handle_scope(napi_env env, napi_handle_scope scope) {
    return napi_ok;
}

// --- Reference management (stub) ---

napi_status napi_create_reference(napi_env env, napi_value value, uint32_t initial_refcount, napi_ref* result) {
    if (result) *result = reinterpret_cast<napi_ref>(value);
    return napi_ok;
}

napi_status napi_delete_reference(napi_env env, napi_ref ref) {
    return napi_ok;
}

napi_status napi_get_reference_value(napi_env env, napi_ref ref, napi_value* result) {
    if (result) *result = reinterpret_cast<napi_value>(ref);
    return napi_ok;
}

// --- Value creation (extended) ---

napi_status napi_create_uint32(napi_env env, uint32_t value, napi_value* result) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::UINT32;
    val->uint32Val = value;
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

napi_status napi_create_arraybuffer(napi_env env, size_t byte_length, void** data, napi_value* result) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::ARRAYBUFFER;
    val->bufferData.resize(byte_length, 0);
    if (data) *data = val->bufferData.data();
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

napi_status napi_get_boolean(napi_env env, bool value, napi_value* result) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::BOOL;
    val->boolVal = value;
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

napi_status napi_get_undefined(napi_env env, napi_value* result) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::UNDEFINED;
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

napi_status napi_get_null(napi_env env, napi_value* result) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::NULL_VAL;
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

napi_status napi_get_global(napi_env env, napi_value* result) {
    if (result) {
        if (g_customGlobal) {
            *result = static_cast<napi_value>(g_customGlobal);
        } else {
            MockNapiValue* val = allocValue();
            val->type = MockNapiValueType::OBJECT;
            *result = static_cast<napi_value>(val);
        }
    }
    return napi_ok;
}

napi_status napi_create_error(napi_env env, napi_value code, napi_value msg, napi_value* result) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::OBJECT;
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

napi_status napi_throw(napi_env env, napi_value error) {
    return napi_ok;
}

// --- Function call (stub) ---

napi_status napi_call_function(napi_env env, napi_value recv, napi_value func, size_t argc,
    const napi_value* argv, napi_value* result) {
    if (result) {
        if (g_callFunctionResult) {
            *result = static_cast<napi_value>(g_callFunctionResult);
        } else {
            MockNapiValue* val = allocValue();
            val->type = MockNapiValueType::UNDEFINED;
            *result = static_cast<napi_value>(val);
        }
    }
    return napi_ok;
}

// --- Async work (stub) ---

napi_status napi_create_async_work(napi_env env, napi_value async_resource,
    napi_value async_resource_name, void (*execute)(napi_env env, void* data),
    void (*complete)(napi_env env, napi_status status, void* data), void* data,
    napi_async_work* result) {
    if (result) *result = nullptr;
    return napi_ok;
}

napi_status napi_delete_async_work(napi_env env, napi_async_work work) {
    return napi_ok;
}

napi_status napi_queue_async_work(napi_env env, napi_async_work work) {
    return napi_ok;
}

// --- UV event loop (stub) ---

napi_status napi_get_uv_event_loop(napi_env env, struct uv_loop_s** loop) {
    if (loop) *loop = nullptr;
    return napi_ok;
}

// --- Typed arrays (stub) ---

napi_status napi_create_typedarray(napi_env env, napi_typedarray_type type, size_t length,
    napi_value arraybuffer, size_t byte_offset, napi_value* result) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::OBJECT;
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

// --- Cleanup hooks (stub) ---

napi_status napi_add_env_cleanup_hook(napi_env env, void (*func)(void*), void* arg) {
    return napi_ok;
}

napi_status napi_remove_env_cleanup_hook(napi_env env, void (*func)(void*), void* arg) {
    return napi_ok;
}

// --- Module registration (stub) ---

void napi_module_register(napi_module* mod) {
    // No-op for test environment
}

// --- Exception handling (stub) ---

napi_status napi_get_and_clear_last_exception(napi_env env, napi_value* result) {
    if (result) *result = nullptr;
    return napi_ok;
}

napi_status napi_is_exception_pending(napi_env env, bool* result) {
    if (result) *result = false;
    return napi_ok;
}

// --- Instance creation (stub) ---

napi_status napi_new_instance(napi_env env, napi_value constructor, size_t argc,
    const napi_value* argv, napi_value* result) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::OBJECT;
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

// --- Deferred rejection (stub) ---

napi_status napi_reject_deferred(napi_env env, napi_deferred deferred, napi_value rejection) {
    return napi_ok;
}

// --- Strict equals (stub) ---

napi_status napi_strict_equals(napi_env env, napi_value lhs, napi_value rhs, bool* result) {
    if (result) *result = (lhs == rhs);
    return napi_ok;
}

// --- NAPI_CALL macro (2-arg form used by bluetooth NAPI code) ---
#define NAPI_CALL(env, theCall) ((theCall))

// --- Extended property/typedarray (stub) ---

napi_status napi_get_property_names(napi_env env, napi_value object, napi_value* result) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::ARRAY;
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

napi_status napi_get_typedarray_info(napi_env env, napi_value typedarray, napi_typedarray_type* type,
    size_t* length, void** data, napi_value* arraybuffer, size_t* byte_offset) {
    if (type) *type = napi_uint8_array;
    if (length) *length = 0;
    if (data) *data = nullptr;
    if (arraybuffer) *arraybuffer = nullptr;
    if (byte_offset) *byte_offset = 0;
    return napi_ok;
}


// --- napi_send_event (stub) ---

napi_status napi_send_event(napi_env env, std::function<void()> func, int priority, const char* taskName) {
    return napi_ok;
}

// --- Extended value creation ---

napi_status napi_create_int64(napi_env env, int64_t value, napi_value* result) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::INT64;
    val->int64Val = value;
    if (result) *result = static_cast<napi_value>(val);
    return napi_ok;
}

// --- Extended value extraction ---

napi_status napi_get_value_int64(napi_env env, napi_value value, int64_t* result) {
    if (!result) return napi_ok;
    MockNapiValue* v = toMock(value);
    if (v) {
        switch (v->type) {
            case MockNapiValueType::INT64:  *result = v->int64Val; break;
            case MockNapiValueType::INT32:  *result = static_cast<int64_t>(v->intVal); break;
            case MockNapiValueType::UINT32: *result = static_cast<int64_t>(v->uint32Val); break;
            default:                        *result = 0; break;
        }
    } else {
        *result = 0;
    }
    return napi_ok;
}

napi_status napi_get_value_uint32(napi_env env, napi_value value, uint32_t* result) {
    if (!result) return napi_ok;
    MockNapiValue* v = toMock(value);
    if (v) {
        switch (v->type) {
            case MockNapiValueType::UINT32: *result = v->uint32Val; break;
            case MockNapiValueType::INT32:  *result = static_cast<uint32_t>(v->intVal); break;
            case MockNapiValueType::INT64:  *result = static_cast<uint32_t>(v->int64Val); break;
            default:                        *result = 0; break;
        }
    } else {
        *result = 0;
    }
    return napi_ok;
}

napi_status napi_get_value_double(napi_env env, napi_value value, double* result) {
    if (!result) return napi_ok;
    MockNapiValue* v = toMock(value);
    if (v) {
        switch (v->type) {
            case MockNapiValueType::DOUBLE: *result = v->doubleVal; break;
            case MockNapiValueType::INT32:  *result = static_cast<double>(v->intVal); break;
            case MockNapiValueType::INT64:  *result = static_cast<double>(v->int64Val); break;
            default:                        *result = 0.0; break;
        }
    } else {
        *result = 0.0;
    }
    return napi_ok;
}

// --- ArrayBuffer operations (stub) ---

napi_status napi_is_arraybuffer(napi_env env, napi_value value, bool* result) {
    if (result) {
        MockNapiValue* v = toMock(value);
        *result = v && v->type == MockNapiValueType::ARRAYBUFFER;
    }
    return napi_ok;
}

napi_status napi_get_arraybuffer_info(napi_env env, napi_value arraybuffer, void** data, size_t* byte_length) {
    MockNapiValue* v = toMock(arraybuffer);
    if (data) {
        if (v && v->type == MockNapiValueType::ARRAYBUFFER) {
            *data = v->bufferData.data();
        } else {
            *data = nullptr;
        }
    }
    if (byte_length) {
        if (v && v->type == MockNapiValueType::ARRAYBUFFER) {
            *byte_length = v->bufferData.size();
        } else {
            *byte_length = 0;
        }
    }
    return napi_ok;
}

// --- Reference management (extended) ---

napi_status napi_reference_unref(napi_env env, napi_ref ref, uint32_t* result) {
    if (result) *result = 0;
    return napi_ok;
}

// --- Error throwing (stub) ---

napi_status napi_throw_error(napi_env env, const char* code, const char* msg) {
    return napi_ok;
}

// ===== Mock helpers for collections module =====

void MockSetGlobal(napi_value global) {
    g_customGlobal = toMock(global);
}

void MockCreateCallable(napi_value* callable) {
    MockNapiValue* val = allocValue();
    val->type = MockNapiValueType::OBJECT;
    if (callable) *callable = static_cast<napi_value>(val);
}

void MockSetCallFunctionResult(napi_value result) {
    g_callFunctionResult = toMock(result);
}

// ===== Extended property operations (collections) =====

napi_status napi_get_property(napi_env env, napi_value object, napi_value key, napi_value* result) {
    if (result) {
        MockNapiValue* obj = toMock(object);
        MockNapiValue* keyVal = toMock(key);
        if (obj && keyVal && keyVal->type == MockNapiValueType::STRING) {
            auto it = obj->properties.find(keyVal->strVal);
            if (it != obj->properties.end()) {
                *result = static_cast<napi_value>(it->second);
            } else {
                MockNapiValue* val = allocValue();
                val->type = MockNapiValueType::UNDEFINED;
                *result = static_cast<napi_value>(val);
            }
        } else {
            MockNapiValue* val = allocValue();
            val->type = MockNapiValueType::UNDEFINED;
            *result = static_cast<napi_value>(val);
        }
    }
    return napi_ok;
}

napi_status napi_is_callable(napi_env env, napi_value value, bool* result) {
    if (result) {
        MockNapiValue* v = toMock(value);
        // Non-null, non-undefined values are considered callable in mock
        *result = (v != nullptr && v->type != MockNapiValueType::UNDEFINED);
    }
    return napi_ok;
}

// ===== napi_module_with_js_register (stub for collections) =====

void napi_module_with_js_register(napi_module_with_js* mod) {
    // No-op for test environment
}

// ===== Mock property query helpers =====

size_t MockGetPropertyCount(napi_value value) {
    MockNapiValue* v = toMock(value);
    return v ? v->propertyDescs.size() : 0;
}

const char* MockGetPropertyName(napi_value value, size_t index) {
    MockNapiValue* v = toMock(value);
    if (v && index < v->propertyDescs.size()) {
        return v->propertyDescs[index].utf8name;
    }
    return nullptr;
}

napi_value MockGetPropertyValue(napi_value value, size_t index) {
    MockNapiValue* v = toMock(value);
    if (v && index < v->propertyDescs.size()) {
        return v->propertyDescs[index].value;
    }
    return nullptr;
}

