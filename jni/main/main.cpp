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

#include "misc.h"
#include "logging.h"
#include "hook.h"

#define CONFIG_PATH "/data/misc/riru/location_report_enabler"

static char package_name[256];
static int uid;
static int enable_hook;
static std::vector<std::string> list;

int is_app_need_hook(JNIEnv *env, jstring appDataDir) {
    if (!appDataDir)
        return 0;


    const char *app_data_dir = env->GetStringUTFChars(appDataDir, NULL);

    int user = 0;
    if (sscanf(app_data_dir, "/data/%*[^/]/%d/%s", &user, package_name) != 2) {
        if (sscanf(app_data_dir, "/data/%*s/%s", package_name) != 1) {
            package_name[0] = '\0';
            LOGW("can't parse %s", app_data_dir);
            return 0;
        }
    }

    env->ReleaseStringUTFChars(appDataDir, app_data_dir);

    for (auto& s : list) {
        if (strcmp(s.c_str(), package_name) == 0)
            return 1; 
    }

    return 0;
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
    if (list.size() == 0) {
        list.push_back("com.google.android.gms");
        list.push_back("com.google.android.gsf");
        list.push_back("com.google.android.apps.maps");
        list.push_back("com.google.android.apps.fitness");
    }
    uid = _uid;
    enable_hook = is_app_need_hook(env, appDataDir);
}

__attribute__((visibility("default"))) int nativeForkAndSpecializePost(JNIEnv *env, jclass clazz,
                                                                        jint res) {
    nativeForkAndSpecialize(res, enable_hook, package_name, uid);
    return !enable_hook;
}
}
