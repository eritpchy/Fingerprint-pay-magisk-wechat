#include <stdbool.h>
#include <stdio.h>
#include <jni.h>
#include <sys/types.h>
#include <unistd.h>

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

extern "C" {
__attribute__((visibility("default")))
void nativeForkAndSpecializePre(
        JNIEnv *env, jclass clazz, jint *_uid, jint *gid, jintArray *gids, jint *,
        jobjectArray *rlimits, jint *_mount_external, jstring *se_info, jstring *jse_name,
        jintArray *fdsToClose, jintArray *fdsToIgnore, jboolean *is_child_zygote,
        jstring *instructionSet, jstring *jappDataDir, jstring *packageName,
        jobjectArray *packagesForUID, jobjectArray *visibleVolIDs) {
    // packageName, packagesForUID, visibleVolIDs exists from Android Q
    char *appDataDir = jstringToC(env, *jappDataDir);
    if (appDataDir == NULL) {
        LOGD("MEM ERR");
        return;
    }
    sAppDataDir = strdup(appDataDir);
    free(appDataDir);
    if (sAppDataDir == NULL) {
        LOGD("MEM ERR");
        return;
    }
    char *niceName = jstringToC(env, *jse_name);
    sHookEnable = equals(niceName, "com.tencent.mm");
    if (niceName) {
        free(niceName);
    }
}

__attribute__((visibility("default")))
int nativeForkAndSpecializePost(JNIEnv *env, jclass clazz, jint res) {
    if (res == 0) {
        // in app process
        if (sHookEnable) {
            char appCacheDir[PATH_MAX] = {0};
            snprintf(appCacheDir, PATH_MAX - 1, "%s/cache", sAppDataDir);

            loadDex(env, 
                env->NewStringUTF("/system/framework/libxfingerprint_pay_wechat.dex"),
                env->NewStringUTF(appCacheDir),
                env->NewStringUTF("com.yyxx.wechatfp.xposed.plugin.XposedWeChatPlugin"),
                "main",
                env->NewStringUTF(sAppDataDir)
            );
        }
    } else {
        // in zygote process, res is child pid
        // don't print log here, see https://github.com/RikkaApps/Riru/blob/77adfd6a4a6a81bfd20569c910bc4854f2f84f5e/riru-core/jni/main/jni_native_method.cpp#L55-L66
    }
    return 0;
}

__attribute__((visibility("default")))
void nativeForkSystemServerPre(
        JNIEnv *env, jclass clazz, uid_t *uid, gid_t *gid, jintArray *gids, jint *debug_flags,
        jobjectArray *rlimits, jlong *permittedCapabilities, jlong *effectiveCapabilities) {

}

__attribute__((visibility("default")))
int nativeForkSystemServerPost(JNIEnv *env, jclass clazz, jint res) {
    if (res == 0) {
        // in system server process
    } else {
        // in zygote process, res is child pid
        // don't print log here, see https://github.com/RikkaApps/Riru/blob/77adfd6a4a6a81bfd20569c910bc4854f2f84f5e/riru-core/jni/main/jni_native_method.cpp#L55-L66
    }
    return 0;
}
}
