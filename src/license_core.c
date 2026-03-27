#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <android/log.h>

#define LOG_TAG "LicenseCore"

// ==================== 可配置区域（脚本修改此处）====================
__attribute__((section(".config")))
volatile char CFG_MAGIC[6] = "CFGBLK";

__attribute__((section(".config")))
volatile char CFG_KEY_VALUE[11] = "12345     ";  // 10 chars

__attribute__((section(".config")))
volatile char CFG_TARGET[51] = "com.example.MainActivity____________________";  // 50 chars

// ==================== 工具函数 ====================
static int get_key_value() {
    char buf[16];
    memcpy(buf, (void*)CFG_KEY_VALUE, 10);
    buf[10] = '\0';
    for (int i = 9; i >= 0; i--) {
        if (buf[i] == ' ' || buf[i] == '_') buf[i] = '\0';
        else break;
    }
    if (buf[0] == '0' && (buf[1] == 'x' || buf[1] == 'X'))
        return (int)strtol(buf, NULL, 16);
    return atoi(buf);
}

static void get_target(char* out) {
    memcpy(out, (void*)CFG_TARGET, 50);
    out[50] = '\0';
    for (int i = 49; i >= 0; i--) {
        if (out[i] == '_' || out[i] == ' ') out[i] = '\0';
        else break;
    }
}

static int get_machine_id() {
    srand(getpid() * 0x9A3F);
    return 100000 + (rand() % 900000);
}

// ==================== 反调试（发现即自杀）====================
static void anti_debug() {
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
        __builtin_trap();
    }
    
    FILE* f = fopen("/proc/self/status", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, "TracerPid:") && atoi(line + 10) != 0) {
                fclose(f);
                __builtin_trap();
            }
        }
        fclose(f);
    }
    
    if (access("/data/local/tmp/frida-server", F_OK) == 0 ||
        access("/data/local/tmp/re.frida.server", F_OK) == 0) {
        __builtin_trap();
    }
}

// ==================== JNI 反射调用 Java ====================
static void call_suicide(JNIEnv* env) {
    jclass cls = (*env)->FindClass(env, "com/license/shell/LicenseActivity");
    jmethodID mid = (*env)->GetStaticMethodID(env, cls, "suicide", "()V");
    (*env)->CallStaticVoidMethod(env, cls, mid);
}

static void call_jump(JNIEnv* env, jobject ctx, const char* target) {
    jclass cls = (*env)->FindClass(env, "com/license/shell/LicenseActivity");
    jmethodID mid = (*env)->GetStaticMethodID(env, cls, "jumpToTarget", 
        "(Landroid/content/Context;Ljava/lang/String;)V");
    jstring jtarget = (*env)->NewStringUTF(env, target);
    (*env)->CallStaticVoidMethod(env, cls, mid, ctx, jtarget);
}

static void call_error(JNIEnv* env, jobject ctx, const char* msg) {
    jclass cls = (*env)->FindClass(env, "com/license/shell/LicenseActivity");
    jmethodID mid = (*env)->GetStaticMethodID(env, cls, "showError",
        "(Landroid/content/Context;Ljava/lang/String;)V");
    jstring jmsg = (*env)->NewStringUTF(env, msg);
    (*env)->CallStaticVoidMethod(env, cls, mid, ctx, jmsg);
}

// ==================== 核心验证逻辑 ====================
static void verify_and_act(JNIEnv* env, jobject ctx, const char* code) {
    int machine_id = get_machine_id();
    int key_val = get_key_value();
    int expected = (machine_id / 2) + key_val;
    
    // 解析注册码（假设是十六进制字符串）
    int actual = 0;
    if (sscanf(code, "%x", &actual) != 1) {
        call_error(env, ctx, "注册码格式错误");
        sleep(2);
        call_suicide(env);
        return;
    }
    
    if (actual == expected) {
        // 验证成功：直接跳转，Java 层无感知
        char target[64] = {0};
        get_target(target);
        call_jump(env, ctx, target);
    } else {
        // 验证失败：显示错误后自杀
        call_error(env, ctx, "注册码无效");
        sleep(2);
        call_suicide(env);
    }
}

// ==================== JNI 接口 ====================

JNIEXPORT void JNICALL
Java_com_license_shell_LicenseActivity_nativeCheckEnvironment(JNIEnv* env, jobject thiz, jobject ctx) {
    anti_debug();
}

JNIEXPORT int JNICALL
Java_com_license_shell_LicenseActivity_nativeGetMachineId(JNIEnv* env, jobject thiz) {
    anti_debug();
    return get_machine_id();
}

JNIEXPORT void JNICALL
Java_com_license_shell_LicenseActivity_nativeVerifyAndAct(JNIEnv* env, jobject thiz, 
                                                         jobject ctx, jstring code) {
    anti_debug();
    
    const char* c_code = (*env)->GetStringUTFChars(env, code, NULL);
    verify_and_act(env, ctx, c_code);
    (*env)->ReleaseStringUTFChars(env, code, c_code);
    
    // 如果执行到这里，说明上面流程被 Hook，强制自杀
    call_suicide(env);
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    anti_debug();
    return JNI_VERSION_1_6;
}
