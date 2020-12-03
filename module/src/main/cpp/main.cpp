#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <jni.h>
#include <sys/types.h>
#include <riru.h>
#include <malloc.h>
#include <cstring>

#include "log.h"

static bool sHookEnable = false;
static char *sAppDataDir = NULL;

static char *jstringToC(JNIEnv * env, jstring jstr){
    char *ret = NULL;
    if (jstr) {
        const char* str = env->GetStringUTFChars(jstr, NULL);
        if (str != NULL) {
            int len = strlen(str);
            ret = (char*) malloc((len + 1) * sizeof(char));
            if (ret != NULL){
                memset(ret, 0, len + 1);
                memcpy(ret, str, len);
            }
            env->ReleaseStringUTFChars(jstr, str);
        }
    }
    return ret;
}

static bool equals(const char *str1, const char *str2) {
    if (str1 == NULL && str2 == NULL) {
        return true;
    } else {
        if (str1 != NULL && str2 != NULL) {
            return strcmp(str1, str2) == 0;
        } else {
            return false;
        }
    }
}


/**  参数说明：
 *  jdexPath        dex存储路径
 *  jodexPath       优化后的dex包存放位置
 *  jclassName      需要调用jar包中的类名
 *  jmethodName     需要调用的类中的静态方法
 */
static void loadDex(JNIEnv *env, jstring jdexPath, jstring jodexPath, jstring jclassName, const char* methodName, jstring jarg1) {

    if (!jdexPath) {
        LOGD("MEM ERR");
        return;
    }

    if (!jodexPath) {
        LOGD("MEM ERR");
        return;
    }

    if (!jclassName) {
        LOGD("MEM ERR");
        return;
    }

    if (!jarg1) {
        LOGD("MEM ERR");
        return;
    }

    jclass classloaderClass = env->FindClass("java/lang/ClassLoader");
    jmethodID getsysClassloaderMethod = env->GetStaticMethodID(classloaderClass, "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
    jobject loader = env->CallStaticObjectMethod(classloaderClass, getsysClassloaderMethod);
    jclass dexLoaderClass = env->FindClass("dalvik/system/DexClassLoader");
    jmethodID initDexLoaderMethod = env->GetMethodID(dexLoaderClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");
    jobject dexLoader = env->NewObject(dexLoaderClass,initDexLoaderMethod, jdexPath, jodexPath, NULL, loader);
    jmethodID findclassMethod = env->GetMethodID(dexLoaderClass, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");

    if (findclassMethod == NULL) {
        findclassMethod = env->GetMethodID(dexLoaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    }

    jclass javaClientClass = (jclass)env->CallObjectMethod(dexLoader, findclassMethod, jclassName);
    jmethodID targetMethod = env->GetStaticMethodID(javaClientClass, methodName, "(Ljava/lang/String;)V");

    if (targetMethod == NULL) {
        LOGD("target method(%s) not found", methodName);
        return;
    }

    env->CallStaticVoidMethod(javaClientClass, targetMethod, jarg1);
}

static void pre(JNIEnv *env, jstring *appDataDir, jstring *niceName) {
    char *cAppDataDir = jstringToC(env, *appDataDir);
    if (cAppDataDir == NULL) {
        LOGD("MEM ERR");
        return;
    }
    sAppDataDir = strdup(cAppDataDir);
    free(cAppDataDir);
    if (sAppDataDir == NULL) {
        LOGD("MEM ERR");
        return;
    }
    char *cNiceName = jstringToC(env, *niceName);
    sHookEnable = equals(cNiceName, "com.tencent.mm");
    if (cNiceName) {
        free(cNiceName);
    }
}

static void post(JNIEnv *env) {
    if (sHookEnable) {
        char appCacheDir[PATH_MAX] = {0};
        snprintf(appCacheDir, PATH_MAX - 1, "%s/cache", sAppDataDir);

        const char *dexPath = "/data/local/tmp/libriru_module_xfingerprint_pay_wechat.dex";
        if (access(dexPath, 0) != 0) {
            dexPath = "/system/framework/libriru_module_xfingerprint_pay_wechat.dex";
        }
        loadDex(env,
                env->NewStringUTF(dexPath),
                env->NewStringUTF(appCacheDir),
                env->NewStringUTF("com.yyxx.wechatfp.xposed.plugin.XposedWeChatPlugin"),
                "main",
                env->NewStringUTF(sAppDataDir)
        );
    }
}

static void forkAndSpecializePre(
        JNIEnv *env, jclass clazz, jint *_uid, jint *gid, jintArray *gids, jint *runtimeFlags,
        jobjectArray *rlimits, jint *mountExternal, jstring *seInfo, jstring *niceName,
        jintArray *fdsToClose, jintArray *fdsToIgnore, jboolean *is_child_zygote,
        jstring *instructionSet, jstring *appDataDir, jboolean *isTopApp, jobjectArray *pkgDataInfoList,
        jobjectArray *whitelistedDataInfoList, jboolean *bindMountAppDataDirs, jboolean *bindMountAppStorageDirs) {
    pre(env, appDataDir, niceName);
}

static void forkAndSpecializePost(JNIEnv *env, jclass clazz, jint res) {
    if (res == 0) {
        // in app process
        post(env);
    } else {
        // in zygote process, res is child pid
        // don't print log here, see https://github.com/RikkaApps/Riru/blob/77adfd6a4a6a81bfd20569c910bc4854f2f84f5e/riru-core/jni/main/jni_native_method.cpp#L55-L66
    }
}

static void specializeAppProcessPre(
        JNIEnv *env, jclass clazz, jint *_uid, jint *gid, jintArray *gids, jint *runtimeFlags,
        jobjectArray *rlimits, jint *mountExternal, jstring *seInfo, jstring *niceName,
        jboolean *startChildZygote, jstring *instructionSet, jstring *appDataDir,
        jboolean *isTopApp, jobjectArray *pkgDataInfoList, jobjectArray *whitelistedDataInfoList,
        jboolean *bindMountAppDataDirs, jboolean *bindMountAppStorageDirs) {
    // added from Android 10, but disabled at least in Google Pixel devices
    pre(env, appDataDir, niceName);
}

static void specializeAppProcessPost(
        JNIEnv *env, jclass clazz) {
    // added from Android 10, but disabled at least in Google Pixel devices
    post(env);
}

static void forkSystemServerPre(
        JNIEnv *env, jclass clazz, uid_t *uid, gid_t *gid, jintArray *gids, jint *runtimeFlags,
        jobjectArray *rlimits, jlong *permittedCapabilities, jlong *effectiveCapabilities) {

}

static void forkSystemServerPost(JNIEnv *env, jclass clazz, jint res) {
    if (res == 0) {
        // in system server process
    } else {
        // in zygote process, res is child pid
        // don't print log here, see https://github.com/RikkaApps/Riru/blob/77adfd6a4a6a81bfd20569c910bc4854f2f84f5e/riru-core/jni/main/jni_native_method.cpp#L55-L66
    }
}

static int shouldSkipUid(int uid) {
    // by default, Riru only call module functions in "normal app processes" (10000 <= uid % 100000 <= 19999)
    // false = don't skip
    return false;
}

static void onModuleLoaded() {
    // called when the shared library of Riru core is loaded
}

extern "C" {

int riru_api_version;
RiruApiV9 *riru_api_v9;

/*
 * Init will be called three times.
 *
 * The first time:
 *   Returns the highest version number supported by both Riru and the module.
 *
 *   arg: (int *) Riru's API version
 *   returns: (int *) the highest possible API version
 *
 * The second time:
 *   Returns the RiruModuleX struct created by the module.
 *   (X is the return of the first call)
 *
 *   arg: (RiruApiVX *) RiruApi strcut, this pointer can be saved for further use
 *   returns: (RiruModuleX *) RiruModule strcut
 *
 * The second time:
 *   Let the module to cleanup (such as RiruModuleX struct created before).
 *
 *   arg: null
 *   returns: (ignored)
 *
 */
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
                // RiruApiV10 and RiruModuleInfoV10 are equal to V9
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
                    module->shouldSkipUid = shouldSkipUid;
                    module->forkAndSpecializePre = forkAndSpecializePre;
                    module->forkAndSpecializePost = forkAndSpecializePost;
                    module->specializeAppProcessPre = specializeAppProcessPre;
                    module->specializeAppProcessPost = specializeAppProcessPost;
                    module->forkSystemServerPre = forkSystemServerPre;
                    module->forkSystemServerPost = forkSystemServerPost;
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