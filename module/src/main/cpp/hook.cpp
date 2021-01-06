#include <cstring>
#include <unistd.h>
#include <sys/system_properties.h>
#include <xhook/xhook.h>

#include "riru.h"
#include "logging.h"
#include "android.h"
#include "hook.h"
#include "config.h"

#define XHOOK_REGISTER(NAME) \
    if (xhook_register(".*", #NAME, (void*) new_##NAME, (void **) &old_##NAME) != 0) { \
        LOGE("failed to register hook " #NAME "."); \
    }

#define NEW_FUNC_DEF(ret, func, ...) \
    static ret (*old_##func)(__VA_ARGS__); \
    static ret new_##func(__VA_ARGS__)

NEW_FUNC_DEF(int, __system_property_get, const char *key, char *value) {
    int res = old___system_property_get(key, value);
    auto prop = Config::Properties::Find(key);
    if (prop) {
        LOGI("system_property_get: %s=%s -> %s", key, value, prop->value.c_str());
        strcpy(value, prop->value.c_str());
        res = (int)prop->value.length();
    }
    return res;
}

using callback_func = void(void *cookie, const char *name, const char *value, uint32_t serial);

thread_local callback_func *saved_callback = nullptr;

static void my_callback(void *cookie, const char *name, const char *value, uint32_t serial) {
    if (!saved_callback) return;

    auto prop = Config::Properties::Find(name);
    if (!prop) {
        saved_callback(cookie, name, value, serial);
        return;
    }

    LOGI("system_property_read_callback: %s=%s -> %s", name, value, prop->value.c_str());
    saved_callback(cookie, name, prop->value.c_str(), serial);
}

NEW_FUNC_DEF(void, __system_property_read_callback, const prop_info *pi, callback_func *callback, void *cookie) {
    saved_callback = callback;
    old___system_property_read_callback(pi, my_callback, cookie);
}

void Hook::install() {
    XHOOK_REGISTER(__system_property_get);

    if (android::GetApiLevel() >= 26) {
        XHOOK_REGISTER(__system_property_read_callback);
    }

    if (xhook_refresh(0) == 0)
        xhook_clear();
    else
        LOGE("failed to refresh hook");
}