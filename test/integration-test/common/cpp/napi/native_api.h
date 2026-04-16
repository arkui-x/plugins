/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Mock NAPI header for Android JNI integration testing.
 * Provides all types, macros, and function declarations required by
 * napi_document_view_picker.cpp and napi_common.cpp.
 */
#ifndef MOCK_NAPI_NATIVE_API_H
#define MOCK_NAPI_NATIVE_API_H

#include <cstdint>
#include <string>
#include <vector>

// ===== Basic NAPI types =====
typedef void* napi_env;
typedef void* napi_value;
typedef void* napi_ref;
typedef void* napi_deferred;
typedef int napi_status;

constexpr napi_status napi_ok = 0;
constexpr napi_status napi_invalid_arg = 1;
constexpr napi_status napi_object_expected = 2;
constexpr napi_status napi_string_expected = 3;
constexpr napi_status napi_number_expected = 4;
constexpr napi_status napi_boolean_expected = 5;
constexpr napi_status napi_array_expected = 6;
constexpr napi_status napi_function_expected = 7;
constexpr napi_status napi_arraybuffer_expected = 8;

// ===== Callback types =====
typedef void* napi_callback_info;
typedef void* (*napi_callback)(napi_env, napi_callback_info);
typedef void (*napi_finalize)(napi_env, void*, void*);

// ===== Handle scope =====
typedef void* napi_handle_scope;
typedef void* napi_escapable_handle_scope;
typedef void* napi_escapable_handle;

// ===== Async work =====
typedef void* napi_async_work;

// ===== Thread-safe function =====
typedef void* napi_threadsafe_function;

// ===== Module registration =====
typedef struct {
    int nm_version;
    unsigned int nm_flags;
    const char* nm_filename;
    napi_callback nm_register_func;
    const char* nm_modname;
    void* nm_priv;
    void* reserved[4];
} napi_module;

#define NAPI_MODULE_VERSION 1
#define NAPI_MODULE(modname, regfunc) \
    static napi_module _napi_module = { NAPI_MODULE_VERSION, 0, __FILE__, regfunc, modname, nullptr, {nullptr} }

void napi_module_register(napi_module* mod);

// ===== Property attributes =====
typedef int napi_property_attributes;
constexpr napi_property_attributes napi_default = 0;
constexpr napi_property_attributes napi_writable = 1;

// ===== Auto length constant =====
#define NAPI_AUTO_LENGTH static_cast<size_t>(-1)

// ===== Value types =====
typedef unsigned int napi_valuetype;
constexpr napi_valuetype napi_undefined = 0;
constexpr napi_valuetype napi_null = 1;
constexpr napi_valuetype napi_boolean = 2;
constexpr napi_valuetype napi_number = 3;
constexpr napi_valuetype napi_string = 4;
constexpr napi_valuetype napi_symbol = 5;
constexpr napi_valuetype napi_object = 6;
constexpr napi_valuetype napi_function = 7;
constexpr napi_valuetype napi_external = 8;

// ===== Property descriptor =====
typedef struct {
    const char* utf8name;
    napi_value name;
    napi_callback method;
    napi_callback getter;
    napi_callback setter;
    napi_value value;
    napi_property_attributes attributes;
    void* data;
} napi_property_descriptor;

// ===== NAPI macros =====
#define DECLARE_NAPI_PROPERTY(name, val) \
    { (name), nullptr, nullptr, nullptr, nullptr, val, napi_default, nullptr }

#define DECLARE_NAPI_FUNCTION(name, func) \
    { (name), nullptr, (func), nullptr, nullptr, nullptr, napi_default, nullptr }

#define DECLARE_WRITABLE_NAPI_FUNCTION(name, func) \
    { (name), nullptr, (func), nullptr, nullptr, nullptr, napi_writable, nullptr }

// ===== NAPI function declarations =====

// Object creation
napi_status napi_create_object(napi_env env, napi_value* result);
napi_status napi_create_int32(napi_env env, int32_t value, napi_value* result);
napi_status napi_create_string_utf8(napi_env env, const char* str, size_t length, napi_value* result);
napi_status napi_create_array(napi_env env, napi_value* result);
napi_status napi_create_array_with_length(napi_env env, size_t length, napi_value* result);
napi_status napi_create_double(napi_env env, double value, napi_value* result);
napi_status napi_create_external(napi_env env, void* data, napi_finalize finalize_cb,
    void* finalize_hint, napi_value* result);

// Promise
napi_status napi_create_promise(napi_env env, napi_deferred* deferred, napi_value* promise);
napi_status napi_resolve_deferred(napi_env env, napi_deferred deferred, napi_value resolution);

// Object property operations
napi_status napi_set_element(napi_env env, napi_value array, uint32_t index, napi_value value);
napi_status napi_set_named_property(napi_env env, napi_value object, const char* key, napi_value value);
napi_status napi_has_named_property(napi_env env, napi_value object, const char* utf8name, bool* result);
napi_status napi_get_named_property(napi_env env, napi_value object, const char* utf8name, napi_value* result);
napi_status napi_get_property(napi_env env, napi_value object, napi_value key, napi_value* result);
napi_status napi_is_callable(napi_env env, napi_value value, bool* result);

// Value extraction
napi_status napi_get_value_string_utf8(napi_env env, napi_value value, char* buf, size_t bufsize,
    size_t* result);
napi_status napi_get_value_int32(napi_env env, napi_value value, int32_t* result);
napi_status napi_get_value_bool(napi_env env, napi_value value, bool* result);

// Callback info
napi_status napi_get_cb_info(napi_env env, napi_callback_info cbinfo, size_t* argc,
    napi_value* argv, napi_value* this_arg, void** data);

// Wrap/unwrap
napi_status napi_unwrap(napi_env env, napi_value js_object, void** result);
napi_status napi_wrap(napi_env env, napi_value js_object, void* native_object,
    napi_finalize finalize_cb, void* finalize_hint, napi_ref* result);

// Class definition
napi_status napi_define_class(napi_env env, const char* utf8name, size_t length,
    napi_callback constructor, void* data, size_t property_count,
    const napi_property_descriptor* properties, napi_value* result);
napi_status napi_define_properties(napi_env env, napi_value object, size_t property_count,
    const napi_property_descriptor* properties);

// Array operations
napi_status napi_is_array(napi_env env, napi_value value, bool* result);
napi_status napi_get_array_length(napi_env env, napi_value value, uint32_t* result);
napi_status napi_get_element(napi_env env, napi_value value, uint32_t index, napi_value* result);

// Type checking
napi_status napi_typeof(napi_env env, napi_value value, napi_valuetype* result);

// Handle scope
napi_status napi_open_handle_scope(napi_env env, napi_handle_scope* result);
napi_status napi_close_handle_scope(napi_env env, napi_handle_scope scope);

// Reference management
napi_status napi_create_reference(napi_env env, napi_value value, uint32_t initial_refcount, napi_ref* result);
napi_status napi_delete_reference(napi_env env, napi_ref ref);
napi_status napi_get_reference_value(napi_env env, napi_ref ref, napi_value* result);

// Value creation (extended)
napi_status napi_create_uint32(napi_env env, uint32_t value, napi_value* result);
napi_status napi_create_arraybuffer(napi_env env, size_t byte_length, void** data, napi_value* result);
napi_status napi_get_boolean(napi_env env, bool value, napi_value* result);
napi_status napi_get_undefined(napi_env env, napi_value* result);
napi_status napi_get_null(napi_env env, napi_value* result);
napi_status napi_get_global(napi_env env, napi_value* result);
napi_status napi_create_error(napi_env env, napi_value code, napi_value msg, napi_value* result);
napi_status napi_throw(napi_env env, napi_value error);

// Function call
napi_status napi_call_function(napi_env env, napi_value recv, napi_value func, size_t argc,
    const napi_value* argv, napi_value* result);

// Async work
napi_status napi_create_async_work(napi_env env, napi_value async_resource,
    napi_value async_resource_name, void (*execute)(napi_env env, void* data),
    void (*complete)(napi_env env, napi_status status, void* data), void* data,
    napi_async_work* result);
napi_status napi_delete_async_work(napi_env env, napi_async_work work);
napi_status napi_queue_async_work(napi_env env, napi_async_work work);

// UV event loop (for NAPI async callbacks)
struct uv_loop_s;
napi_status napi_get_uv_event_loop(napi_env env, struct uv_loop_s** loop);

// Typed arrays
typedef enum {
    napi_int8_array,
    napi_uint8_array,
    napi_int16_array,
    napi_uint16_array,
    napi_int32_array,
    napi_uint32_array,
    napi_float32_array,
    napi_float64_array,
} napi_typedarray_type;
napi_status napi_create_typedarray(napi_env env, napi_typedarray_type type, size_t length,
    napi_value arraybuffer, size_t byte_offset, napi_value* result);

// Cleanup hooks
napi_status napi_add_env_cleanup_hook(napi_env env, void (*func)(void*), void* arg);
napi_status napi_remove_env_cleanup_hook(napi_env env, void (*func)(void*), void* arg);

// Exception handling
napi_status napi_get_and_clear_last_exception(napi_env env, napi_value* result);
napi_status napi_is_exception_pending(napi_env env, bool* result);

// Instance creation
napi_status napi_new_instance(napi_env env, napi_value constructor, size_t argc,
    const napi_value* argv, napi_value* result);

// Deferred rejection
napi_status napi_reject_deferred(napi_env env, napi_deferred deferred, napi_value rejection);

// Equality check
napi_status napi_strict_equals(napi_env env, napi_value lhs, napi_value rhs, bool* result);

// Extended value creation
napi_status napi_create_int64(napi_env env, int64_t value, napi_value* result);

// Extended value extraction
napi_status napi_get_value_int64(napi_env env, napi_value value, int64_t* result);
napi_status napi_get_value_uint32(napi_env env, napi_value value, uint32_t* result);
napi_status napi_get_value_double(napi_env env, napi_value value, double* result);

// ArrayBuffer operations
napi_status napi_is_arraybuffer(napi_env env, napi_value value, bool* result);
napi_status napi_get_arraybuffer_info(napi_env env, napi_value arraybuffer, void** data, size_t* byte_length);

// Reference management (extended)
napi_status napi_reference_unref(napi_env env, napi_ref ref, uint32_t* result);

// Error throwing
napi_status napi_throw_error(napi_env env, const char* code, const char* msg);

// NAPI_CALL macro (2-arg form used by bluetooth NAPI code)
#define NAPI_CALL(env, theCall) ((theCall))



#include <functional>

// napi_send_event (for async callback scheduling)
napi_status napi_send_event(napi_env env, std::function<void()> func, int priority, const char* taskName);

// Extended NAPI functions for bluetooth - implemented in napi_mock.cpp
napi_status napi_get_property_names(napi_env env, napi_value object, napi_value* result);
napi_status napi_get_typedarray_info(napi_env env, napi_value typedarray, napi_typedarray_type* type,
    size_t* length, void** data, napi_value* arraybuffer, size_t* byte_offset);
// Priority constants
constexpr int napi_eprio_high = 0;


// ===== Mock test helpers =====

void MockNapiReset();
std::vector<std::string> GetMockResultUris();
int GetMockResultErrorCode();
bool WasDeferredResolved();

// Mock object construction (bypass napi_create_* to avoid result capture)
void MockSetCallbackArgs(size_t argc, napi_value* argv, napi_value thisVar);
napi_value MockCreateObject();
napi_value MockCreateInt32(int32_t value);
napi_value MockCreateString(const std::string& str);
napi_value MockCreateBool(bool value);
napi_value MockCreateStringArray(const std::vector<std::string>& strings);
void MockSetProperty(napi_value obj, const char* key, napi_value value);

// Mock wrap query (for verifying napi_wrap calls)
bool MockIsWrapped(napi_value js_object);

// Mock helpers for collections module (custom global and function call)
void MockSetGlobal(napi_value global);
void MockCreateCallable(napi_value* callable);
void MockSetCallFunctionResult(napi_value result);

// Mock property query helpers (for verifying napi_define_properties calls)
size_t MockGetPropertyCount(napi_value value);
const char* MockGetPropertyName(napi_value value, size_t index);
napi_value MockGetPropertyValue(napi_value value, size_t index);

#endif
