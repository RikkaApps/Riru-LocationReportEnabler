#include <cstdio>
#include <cstdlib>
#include <jni.h>
#include <riru.h>
#include <nativehelper/scoped_utf_chars.h>
#include <sys/system_properties.h>

#include "logging.h"
#include "hook.h"
#include "android.h"
#include "config.h"

static char saved_package_name[256] = {0};
static int saved_uid;

#ifdef DEBUG
static char saved_process_name[256] = {0};
#endif

static void appProcessPre(JNIEnv *env, jint *uid, jstring *jNiceName, jstring *jAppDataDir) {

    saved_uid = *uid;

#ifdef DEBUG
    memset(saved_process_name, 0, 256);

    if (*jNiceName) {
        sprintf(saved_process_name, "%s", ScopedUtfChars(env, *jNiceName).c_str());
    }
#endif

    memset(saved_package_name, 0, 256);

    if (*jAppDataDir) {
        auto appDataDir = ScopedUtfChars(env, *jAppDataDir).c_str();
        int user = 0;

        // /data/user/<user_id>/<package>
        if (sscanf(appDataDir, "/data/%*[^/]/%d/%s", &user, saved_package_name) == 2)
            goto found;

        // /mnt/expand/<id>/user/<user_id>/<package>
        if (sscanf(appDataDir, "/mnt/expand/%*[^/]/%*[^/]/%d/%s", &user, saved_package_name) == 2)
            goto found;

        // /data/data/<package>
        if (sscanf(appDataDir, "/data/%*[^/]/%s", saved_package_name) == 1)
            goto found;

        // nothing found
        saved_package_name[0] = '\0';

        found:;
    }
}

static void appProcessPost(
        JNIEnv *env, const char *from, const char *package_name, jint uid) {

    LOGD("%s: uid=%d, package=%s, process=%s", from, uid, package_name, saved_process_name);

    if (Config::Packages::Find(package_name)) {
        LOGI("install hook for %d:%s", uid / 100000, package_name);
        Hook::install();
    }
}

static void forkAndSpecializePre(
        JNIEnv *env, jclass clazz, jint *uid, jint *gid, jintArray *gids, jint *runtimeFlags,
        jobjectArray *rlimits, jint *mountExternal, jstring *seInfo, jstring *niceName,
        jintArray *fdsToClose, jintArray *fdsToIgnore, jboolean *is_child_zygote,
        jstring *instructionSet, jstring *appDataDir, jboolean *isTopApp, jobjectArray *pkgDataInfoList,
        jobjectArray *whitelistedDataInfoList, jboolean *bindMountAppDataDirs, jboolean *bindMountAppStorageDirs) {

    appProcessPre(env, uid, niceName, appDataDir);
}

static void forkAndSpecializePost(JNIEnv *env, jclass clazz, jint res) {
    if (res == 0) {
        appProcessPost(env, "forkAndSpecialize", saved_package_name, saved_uid);
    }
}

static void specializeAppProcessPre(
        JNIEnv *env, jclass clazz, jint *uid, jint *gid, jintArray *gids, jint *runtimeFlags,
        jobjectArray *rlimits, jint *mountExternal, jstring *seInfo, jstring *niceName,
        jboolean *startChildZygote, jstring *instructionSet, jstring *appDataDir,
        jboolean *isTopApp, jobjectArray *pkgDataInfoList, jobjectArray *whitelistedDataInfoList,
        jboolean *bindMountAppDataDirs, jboolean *bindMountAppStorageDirs) {

    appProcessPre(env, uid, niceName, appDataDir);
}

static void specializeAppProcessPost(
        JNIEnv *env, jclass clazz) {
    appProcessPost(env, "specializeAppProcess", saved_package_name, saved_uid);
}

static void onModuleLoaded() {
    Config::Load();
}

extern "C" {

int riru_api_version;
RiruApiV9 *riru_api_v9;

void *init(void *arg) {
    static int step = 0;
    step += 1;

    static void *_module;

    switch (step) {
        case 1: {
            auto core_max_api_version = *(int *) arg;
            riru_api_version = core_max_api_version <= RIRU_MODULE_API_VERSION ? core_max_api_version : RIRU_MODULE_API_VERSION;
            return &riru_api_version;
        }
        case 2: {
            switch (riru_api_version) {
                case 10:
                case 9: {
                    riru_api_v9 = (RiruApiV9 *) arg;

                    auto module = (RiruModuleInfoV9 *) malloc(sizeof(RiruModuleInfoV9));
                    memset(module, 0, sizeof(RiruModuleInfoV9));
                    _module = module;

                    module->supportHide = true;

                    module->version = RIRU_MODULE_VERSION;
                    module->versionName = RIRU_MODULE_VERSION_NAME;
                    module->onModuleLoaded = onModuleLoaded;
                    module->forkAndSpecializePre = forkAndSpecializePre;
                    module->forkAndSpecializePost = forkAndSpecializePost;
                    module->specializeAppProcessPre = specializeAppProcessPre;
                    module->specializeAppProcessPost = specializeAppProcessPost;
                    return module;
                }
                default: {
                    return nullptr;
                }
            }
        }
        case 3: {
            free(_module);
            return nullptr;
        }
        default: {
            return nullptr;
        }
    }
}
}