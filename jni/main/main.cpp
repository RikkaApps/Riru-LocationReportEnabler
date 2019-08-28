#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <jni.h>
#include <cstdlib>
#include <array>
#include <thread>
#include <vector>
#include <string>
#include <sys/system_properties.h>

#include "logging.h"
#include "hook.h"
#include "misc.h"

#define CONFIG_PATH "/data/misc/riru/modules/miui_global_localization"

static char package_name[256];
static int uid;
static int enable_hook;

static bool is_app_need_hook(JNIEnv *env, jstring jAppDataDir, jstring jPackageName) {
    if (jPackageName) {
        const char *packageName = env->GetStringUTFChars(jPackageName, nullptr);
        sprintf(package_name, "%s", packageName);
        env->ReleaseStringUTFChars(jPackageName, packageName);
    } else if (jAppDataDir) {
        const char *appDataDir = env->GetStringUTFChars(jAppDataDir, nullptr);
        int user = 0;
        if (sscanf(appDataDir, "/data/%*[^/]/%d/%s", &user, package_name) != 2) {
            if (sscanf(appDataDir, "/data/%*[^/]/%s", package_name) != 1) {
                package_name[0] = '\0';
                LOGW("can't parse %s", appDataDir);
                return false;
            }
        }
        env->ReleaseStringUTFChars(jAppDataDir, appDataDir);
    } else {
        return false;
    }


    char path[PATH_MAX];
    snprintf(path, PATH_MAX, CONFIG_PATH "/packages/%s", package_name);
    return access(path, F_OK) == 0;
}

static void load_config() {
    char buf[PROP_VALUE_MAX + 1];
    int fd;
    fd = open(CONFIG_PATH "/ro.product.mod_device", O_RDONLY | O_CLOEXEC);
    if (fd > 0 && fdgets(buf, sizeof(buf), fd) > 0)
        set_ro_product_mod_device(buf);
    if (fd > 0)
        close(fd);
}

extern "C" {
__attribute__((visibility("default"))) void nativeForkAndSpecializePre(
        JNIEnv *env, jclass clazz, jint *_uid, jint *gid, jintArray *gids, jint *runtime_flags,
        jobjectArray *rlimits, jint *_mount_external, jstring *se_info, jstring *se_name,
        jintArray *fdsToClose, jintArray *fdsToIgnore, jboolean *is_child_zygote,
        jstring *instructionSet, jstring *appDataDir, jstring *packageName,
        jobjectArray *packagesForUID, jstring *sandboxId) {

    uid = *_uid;
    enable_hook = is_app_need_hook(env, *appDataDir, *packageName);

    if (enable_hook)
        load_config();
}

__attribute__((visibility("default"))) int nativeForkAndSpecializePost(
        JNIEnv *env, jclass clazz, jint res) {
    if (res == 0 && enable_hook) install_hook(package_name, uid / 100000);
    return !enable_hook;
}

__attribute__((visibility("default"))) void specializeAppProcessPre(
        JNIEnv *env, jclass clazz, jint *_uid, jint *gid, jintArray *gids, jint *runtimeFlags,
        jobjectArray *rlimits, jint *mountExternal, jstring *seInfo, jstring *niceName,
        jboolean *startChildZygote, jstring *instructionSet, jstring *appDataDir,
        jstring *packageName, jobjectArray *packagesForUID, jstring *sandboxId) {

    uid = *_uid;
    enable_hook = is_app_need_hook(env, *appDataDir, *packageName);

    if (enable_hook)
        load_config();
}

__attribute__((visibility("default"))) int specializeAppProcessPost(
        JNIEnv *env, jclass clazz) {

    if (enable_hook) install_hook(package_name, uid / 100000);
    return !enable_hook;
}
}
