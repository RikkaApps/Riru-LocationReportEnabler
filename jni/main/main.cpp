#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <jni.h>
#include <cstring>
#include <cstdlib>
#include <sys/mman.h>
#include <array>
#include <thread>
#include <vector>
#include <utility>
#include <string>
#include <sys/system_properties.h>

#include "logging.h"
#include "hook.h"
#include "misc.h"

#define CONFIG_PATH "/data/misc/riru/modules/location_report_enabler"

static char package_name[256];
static int uid;
static int enable_hook;
static std::vector<std::string> packages = {
        "com.google.android.gms",
        "com.google.android.gsf",
        "com.google.android.apps.maps"
};

int is_app_need_hook(JNIEnv *env, jstring appDataDir) {
    if (!appDataDir)
        return 0;


    const char *app_data_dir = env->GetStringUTFChars(appDataDir, NULL);

    int user = 0;
    if (sscanf(app_data_dir, "/data/%*[^/]/%d/%s", &user, package_name) != 2) {
        if (sscanf(app_data_dir, "/data/%*[^/]/%s", package_name) != 1) {
            package_name[0] = '\0';
            LOGW("can't parse %s", app_data_dir);
            return 0;
        }
    }

    env->ReleaseStringUTFChars(appDataDir, app_data_dir);

    if (access(CONFIG_PATH "/packages", R_OK) != 0) {
        for (auto &s : packages) {
            if (strcmp(s.c_str(), package_name) == 0)
                return 1;
        }
    } else {
        char path[PATH_MAX];
        snprintf(path, PATH_MAX, CONFIG_PATH "/packages/%s", package_name);
        return access(path, F_OK) == 0;
    }
    return 0;
}

void load_config() {
    char buf[PROP_VALUE_MAX + 1];
    int fd;
    fd = open(CONFIG_PATH "/gsm.sim.operator.numeric", O_RDONLY);
    if (fd > 0 && fdgets(buf, sizeof(buf), fd) > 0)
        set_sim_operator_numeric(buf);

    if (fd > 0)
        close(fd);

    fd = open(CONFIG_PATH "/gsm.sim.operator.iso-country", O_RDONLY);
    if (fd > 0 && fdgets(buf, sizeof(buf), fd) > 0)
        set_sim_operator_country(buf);

    if (fd > 0)
        close(fd);
}

void nativeForkAndSpecialize(int res, int enable_hook, const char *package_name, jint uid) {
    if (res == 0 && enable_hook) {
        install_hook(package_name, uid / 100000);
    }
}

extern "C" {
__attribute__((visibility("default"))) void nativeForkAndSpecializePre(JNIEnv *env, jclass clazz,
                                                                       jint _uid, jint gid,
                                                                       jintArray gids,
                                                                       jint runtime_flags,
                                                                       jobjectArray rlimits,
                                                                       jint _mount_external,
                                                                       jstring se_info,
                                                                       jstring se_name,
                                                                       jintArray fdsToClose,
                                                                       jintArray fdsToIgnore,
                                                                       jboolean is_child_zygote,
                                                                       jstring instructionSet,
                                                                       jstring appDataDir) {
    uid = _uid;
    enable_hook = is_app_need_hook(env, appDataDir);

    if (enable_hook)
        load_config();
}

__attribute__((visibility("default"))) int nativeForkAndSpecializePost(JNIEnv *env, jclass clazz,
                                                                       jint res) {
    nativeForkAndSpecialize(res, enable_hook, package_name, uid);
    return !enable_hook;
}
}
