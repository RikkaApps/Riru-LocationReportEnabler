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
#include <riru.h>


#define CONFIG_PATH "/data/misc/riru/modules/mipush_fake"

#define FAKE_CONFIGURATION_GLOBAL "/data/misc/riru/modules/mipush_fake/packages/ALL"


static char package_name[256];
static int uid;
static int enable_hook;
static std::vector<std::string> blackList = {"com.google.android",
                                            "de.robv.android.xposed.installer",
                                            "com.xiaomi.xmsf",
                                            "com.tencent.mm",
                                            "top.trumeet.mipush"};

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

    std::string pkgName = package_name;
    for (auto &s : blackList) {
        if (pkgName.find(s) != std::string::npos) {
            return 0;
        }
    }


    if (access(FAKE_CONFIGURATION_GLOBAL, F_OK) == 0) {
        return 1;
    }

    if (access(CONFIG_PATH "/packages", R_OK) == 0) {
        char path[PATH_MAX];
        snprintf(path, PATH_MAX, CONFIG_PATH "/packages/%d.%s", user, package_name);
        return access(path, F_OK) == 0;
    }
    return 0;
}

void load_config() {
    //empty
}



void injectBuild(JNIEnv *env) {
    if (env == 0) {
        LOGW("failed to inject android.os.Build for %s due to env is null", package_name);
        return;
    }
    LOGI("inject android.os.Build for %s ", package_name);

    jclass build_class = env->FindClass("android/os/Build");
    if (build_class == 0) {
        LOGW("failed to inject android.os.Build for %s due to build is null", package_name);
        return;
    }

    jstring new_str = env->NewStringUTF("Xiaomi");

    jfieldID brand_id = env->GetStaticFieldID(build_class, "BRAND", "Ljava/lang/String;");
    if (brand_id != 0) {
        env->SetStaticObjectField(build_class, brand_id, new_str);
    }

    jfieldID manufacturer_id = env->GetStaticFieldID(build_class, "MANUFACTURER", "Ljava/lang/String;");
    if (manufacturer_id != 0) {
        env->SetStaticObjectField(build_class, manufacturer_id, new_str);
    }

    jfieldID product_id = env->GetStaticFieldID(build_class, "PRODUCT", "Ljava/lang/String;");
    if (product_id != 0) {
        env->SetStaticObjectField(build_class, product_id, new_str);
    }

    if(env->ExceptionCheck())
    {
        env->ExceptionClear();
    }

    env->DeleteLocalRef(new_str);

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

    if (res == 0 && enable_hook) {
        injectBuild(env);
    }

    nativeForkAndSpecialize(res, enable_hook, package_name, uid);
    return !enable_hook;
}


}
