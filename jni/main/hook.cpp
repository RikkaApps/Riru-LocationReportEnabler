#include <cstdio>
#include <cstring>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <dirent.h>
#include <dlfcn.h>
#include <xhook/xhook.h>

#include "logging.h"

#define XHOOK_REGISTER(NAME) \
    if (xhook_register(".*", #NAME, (void*) new_##NAME, (void **) &old_##NAME) != 0) \
        LOGE("failed to register hook " #NAME "."); \

#define NEW_FUNC_DEF(ret, func, ...) \
    static ret (*old_##func)(__VA_ARGS__); \
    static ret new_##func(__VA_ARGS__)

NEW_FUNC_DEF(int, __system_property_get, const char *key, char *value) {
    int res = old___system_property_get(key, value);
    if (key) {
        if (strcmp("gsm.sim.operator.numeric", key) == 0) {
            strcpy(value, "310030");
            LOGI("system_property_get: %s -> %s", key, value);
        } else if (strcmp("gsm.sim.operator.iso-country", key) == 0) {
            strcpy(value, "us");
            LOGI("system_property_get: %s -> %s", key, value);
        }
    }
    return res;
}

void install_hook(const char* package_name, int user) {
    LOGI("install hook for %s (%d)", package_name, user);

    XHOOK_REGISTER(__system_property_get);

    if (xhook_refresh(0) == 0)
        xhook_clear();
    else
        LOGE("failed to refresh hook");
}