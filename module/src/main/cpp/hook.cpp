#include <cstdio>
#include <cstring>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <dirent.h>
#include <dlfcn.h>
#include <cstdlib>
#include <string>
#include <sys/system_properties.h>
#include <xhook/xhook.h>

#include "riru.h"
#include "logging.h"
#include "android.h"

static const char *sim_operator_numeric = "310030";
static const char *sim_operator_country = "us";

void set_sim_operator_numeric(const char *string) {
    sim_operator_numeric = strdup(string);
}

void set_sim_operator_country(const char *string) {
    sim_operator_country = strdup(string);
}

#define XHOOK_REGISTER(NAME) \
    if (xhook_register(".*", #NAME, (void*) new_##NAME, (void **) &old_##NAME) != 0) { \
        LOGE("failed to register hook " #NAME "."); \
    }

#define NEW_FUNC_DEF(ret, func, ...) \
    static ret (*old_##func)(__VA_ARGS__); \
    static ret new_##func(__VA_ARGS__)

NEW_FUNC_DEF(int, __system_property_get, const char *key, char *value) {
    int res = old___system_property_get(key, value);
    LOGD("get: %s=%s", key, value);
    if (key) {
        if (strcmp("gsm.sim.operator.numeric", key) == 0) {
            strcpy(value, sim_operator_numeric);
            LOGI("system_property_get: %s -> %s", key, value);
        } else if (strcmp("gsm.sim.operator.iso-country", key) == 0) {
            strcpy(value, sim_operator_country);
            LOGI("system_property_get: %s -> %s", key, value);
        }
    }
    return res;
}

using callback_func = void(void *cookie, const char *name, const char *value, uint32_t serial);

thread_local callback_func *last_callback = nullptr;

static void my_callback(void *cookie, const char *name, const char *value, uint32_t serial) {
    if (!last_callback) return;

    if (name && strcmp("gsm.sim.operator.numeric", name) == 0) {
        LOGI("system_property_read_callback: %s -> %s", name, sim_operator_numeric);
        last_callback(cookie, name, sim_operator_numeric, serial);

    } else if (name && strcmp("gsm.sim.operator.iso-country", name) == 0) {
        LOGI("system_property_read_callback: %s -> %s", name, sim_operator_country);
        last_callback(cookie, name, sim_operator_country, serial);

    } else {
        LOGD("callback: %s=%s", name, value);
        last_callback(cookie, name, value, serial);
    }
}

NEW_FUNC_DEF(void, __system_property_read_callback, const prop_info *pi, callback_func *callback, void *cookie) {
    last_callback = callback;
    old___system_property_read_callback(pi, my_callback, cookie);
}

void install_hook() {
    XHOOK_REGISTER(__system_property_get);
    LOGD("__system_property_get");

    if (android::GetApiLevel() >= 26) {
        XHOOK_REGISTER(__system_property_read_callback);
        LOGD("__system_property_read_callback");
    }

    if (xhook_refresh(0) == 0)
        xhook_clear();
    else
        LOGE("failed to refresh hook");
}