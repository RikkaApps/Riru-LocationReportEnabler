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

static const char *sim_operator_numeric = "310030";
static const char *sim_operator_country = "us";

void set_sim_operator_numeric(const char *string) {
    sim_operator_numeric = strdup(string);
}

void set_sim_operator_country(const char *string) {
    sim_operator_country = strdup(string);
}

#define XHOOK_REGISTER(NAME) \
    if (xhook_register(".*", #NAME, (void*) new_##NAME, (void **) &old_##NAME) == 0) { \
        if (riru_get_version() >= 8) { \
            void *f = riru_get_func(#NAME); \
            if (f != nullptr) \
                memcpy(&old_##NAME, &f, sizeof(void *)); \
            riru_set_func(#NAME, (void *) new_##NAME); \
        } \
    } else { \
        LOGE("failed to register hook " #NAME "."); \
    }

#define NEW_FUNC_DEF(ret, func, ...) \
    static ret (*old_##func)(__VA_ARGS__); \
    static ret new_##func(__VA_ARGS__)

NEW_FUNC_DEF(int, __system_property_get, const char *key, char *value) {
    int res = old___system_property_get(key, value);
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

NEW_FUNC_DEF(std::string, _ZN7android4base11GetPropertyERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_, const std::string &key, const std::string &default_value) {
    std::string res = old__ZN7android4base11GetPropertyERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_(key, default_value);
    if (strcmp("gsm.sim.operator.numeric", key.c_str()) == 0) {
        res = sim_operator_numeric;
        LOGI("android::base::GetProperty: %s -> %s", key.c_str(), res.c_str());
    } else if (strcmp("gsm.sim.operator.iso-country", key.c_str()) == 0) {
        res = sim_operator_country;
        LOGI("android::base::GetProperty: %s -> %s", key.c_str(), res.c_str());
    }
    return res;
}

void install_hook(const char *package_name, int user) {
    LOGI("install hook for %s (%d)", package_name, user);

    XHOOK_REGISTER(__system_property_get);

    char sdk[PROP_VALUE_MAX + 1];
    if (__system_property_get("ro.build.version.sdk", sdk) > 0 && atoi(sdk) >= 28) {
        XHOOK_REGISTER(_ZN7android4base11GetPropertyERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_);
    }

    if (xhook_refresh(0) == 0)
        xhook_clear();
    else
        LOGE("failed to refresh hook");
}