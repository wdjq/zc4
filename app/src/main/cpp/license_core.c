// license_core.c - Native掌权版：验证通过立即跳转，保留完整错误提示
#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <dlfcn.h>
#include <link.h>
#include <android/log.h>
#include <sys/mman.h>

#define LOG_TAG "LicenseCore"
#define CONFIG_OFFSET 0x2000
#define CONFIG_SIZE 128
#define CFG_KEY_BASE 0x3C
#define CFG_MIX 0xA5
#define CODE_KEY 0x7B
#define CODE_SEED 0xA5

// ========== 代码段加壳（自解密） ==========
__attribute__((constructor)) void decrypt_code_section() {
    Dl_info info;
    dladdr((void*)decrypt_code_section, &info);
    unsigned char* base = (unsigned char*)info.dli_fbase;
    
    if (!base) return;
    mprotect(base, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);
    
    for (size_t i = 0; i < 0x1000; i++) {
        if (i >= CONFIG_OFFSET && i < CONFIG_OFFSET + CONFIG_SIZE) continue;
        base[i] ^= CODE_KEY ^ ((i & 0xFF) << 2) ^ CODE_SEED;
    }
}

// ========== 配置区解密 ==========
static void decrypt_config(unsigned char* data, int len) {
    for (int i = 0; i < len; i++) {
        data[i] ^= CFG_MIX ^ CFG_KEY_BASE ^ (i & 0x0F);
    }
}

static int load_config(char* out_key, char* out_activity) {
    Dl_info info;
    dladdr((void*)load_config, &info);
    unsigned char* base = (unsigned char*)info.dli_fbase;
    
    if (!base) return -1;
    
    unsigned char enc[CONFIG_SIZE], dec[CONFIG_SIZE];
    memcpy(enc, base + CONFIG_OFFSET, CONFIG_SIZE);
    memcpy(dec, enc, CONFIG_SIZE);
    decrypt_config(dec, CONFIG_SIZE);
    
    int len = *(int*)dec;
    if (len < 15 || len > 70) return -1;
    
    unsigned char* data = dec + 4;
    int ki = 0, ai = 0;
    
    while (ki < 11 && data[ki] != '\0') {
        out_key[ki] = data[ki];
        ki++;
    }
    out_key[ki] = '\0';
    if (ki < 3 || ki > 10) return -1;
    
    int pos = ki + 1;
    while (ai < 51 && data[pos + ai] != '\0') {
        out_activity[ai] = data[pos + ai];
        ai++;
    }
    out_activity[ai] = '\0';
    
    memset(enc, 0, sizeof(enc));
    memset(dec, 0, sizeof(dec));
    return (ai >= 10 && ai <= 50) ? 0 : -1;
}

// ========== 详细验证逻辑（保留所有错误类型） ==========
typedef struct {
    int success;
    int expired;
    char message[256];
} result_t;

static void verify(const char* code, int machine_id, int key_val, result_t* r) {
    memset(r, 0, sizeof(*r));
    
    // 1. 空检查
    if (!code || !*code) {
        strcpy(r->message, "注册码不能为空");
        return;
    }
    
    // 2. 解密/解析注册码
    char buf[64];
    strncpy(buf, code, 63);
    buf[63] = '\0';
    
    char* z = strchr(buf, 'z');
    char hex[16] = {0};
    long duration = 0;
    
    if (z) {
        int hlen = z - buf;
        if (hlen > 15) hlen = 15;
        strncpy(hex, buf, hlen);
        
        char* end;
        duration = strtol(z + 1, &end, 10);
        if (*end && *end != '\n') {
            strcpy(r->message, "时间格式错误");
            return;
        }
    } else {
        strcpy(hex, buf);
    }
    
    // 3. Hex解析
    char* end;
    int decrypted = (int)strtol(hex, &end, 16);
    if (*end) {
        strcpy(r->message, "机器码格式错误");
        return;
    }
    
    // 4. 核心验证
    int expected = (machine_id >> 1) + key_val;
    if (decrypted != expected) {
        strcpy(r->message, "注册码与设备不匹配");
        return;
    }
    
    // 5. 时间验证
    long now = (long)time(NULL) * 1000;
    long expire;
    
    if (duration > 0) {
        expire = now + duration;
        if (now > expire) {
            r->expired = 1;
            strcpy(r->message, "注册码已过期");
            return;
        }
    } else {
        expire = 0x7FFFFFFFFFFFFFFF;
    }
    
    // 6. 成功
    r->success = 1;
    if (expire == 0x7FFFFFFFFFFFFFFF) {
        strcpy(r->message, "软件使用无期限");
    } else {
        long days = (expire - now) / (1000 * 60 * 60 * 24);
        char dstr[32];
        time_t t = expire / 1000;
        struct tm* tm = localtime(&t);
        strftime(dstr, 32, "%Y-%m-%d", tm);
        sprintf(r->message, "软件使用期至：%s（剩余%ld天）", dstr, days);
    }
}

// ========== JNI：验证并执行跳转（生杀大权） ==========
JNIEXPORT jstring JNICALL
Java_com_yourpackage_license_LicenseProtector_nativeVerifyAndLaunch(
        JNIEnv* env, jobject thiz, jstring jCode, jint machineId, jobject jActivity) {
    
    char key[16] = {0}, target[64] = {0};
    if (load_config(key, target) != 0) {
        return (*env)->NewStringUTF(env, "ERR:配置读取失败");
    }
    
    int key_val = atoi(key);
    const char* code = (*env)->GetStringUTFChars(env, jCode, 0);
    
    result_t r;
    verify(code, machineId, key_val, &r);
    
    (*env)->ReleaseStringUTFChars(env, jCode, code);
    
    // 构建返回字符串
    char ret[300] = {0};
    
    if (r.success) {
        // === 成功：立即执行跳转 ===
        jclass act_cls = (*env)->GetObjectClass(env, jActivity);
        
        // Intent
        jclass intent_cls = (*env)->FindClass(env, "android/content/Intent");
        jmethodID intent_new = (*env)->GetMethodID(env, intent_cls, "<init>", "(Landroid/content/Context;Ljava/lang/Class;)V");
        
        // 类名转签名
        char sig[70];
        strcpy(sig, target);
        for (int i = 0; sig[i]; i++) if (sig[i] == '.') sig[i] = '/';
        
        jclass tgt_cls = (*env)->FindClass(env, sig);
        if (!tgt_cls) {
            return (*env)->NewStringUTF(env, "ERR:目标Activity不存在");
        }
        
        jobject intent = (*env)->NewObject(env, intent_cls, intent_new, jActivity, tgt_cls);
        
        // 设置Flags
        jmethodID add_flags = (*env)->GetMethodID(env, intent_cls, "addFlags", "(I)Landroid/content/Intent;");
        (*env)->CallObjectMethod(env, intent, add_flags, 0x10000000 | 0x04000000);
        
        // 启动（Native直接执行）
        jmethodID start = (*env)->GetMethodID(env, act_cls, "startActivity", "(Landroid/content/Intent;)V");
        (*env)->CallVoidMethod(env, jActivity, start, intent);
        
        // 保存注册标记（加密存储）
        jmethodID get_prefs = (*env)->GetMethodID(env, act_cls, "getSharedPreferences", "(Ljava/lang/String;I)Landroid/content/SharedPreferences;");
        jobject prefs = (*env)->CallObjectMethod(env, jActivity, get_prefs,
            (*env)->NewStringUTF(env, "LicenseConfig"), 0);
        
        jclass prefs_cls = (*env)->GetObjectClass(env, prefs);
        jmethodID edit = (*env)->GetMethodID(env, prefs_cls, "edit", "()Landroid/content/SharedPreferences$Editor;");
        jobject editor = (*env)->CallObjectMethod(env, prefs, edit);
        
        char flag[64];
        sprintf(flag, "VALID_%d_%ld", machineId ^ 0x5A5A, (long)time(NULL));
        
        jclass editor_cls = (*env)->GetObjectClass(env, editor);
        jmethodID put = (*env)->GetMethodID(env, editor_cls, "putString", "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/SharedPreferences$Editor;");
        (*env)->CallObjectMethod(env, editor, put,
            (*env)->NewStringUTF(env, "lic_flag"),
            (*env)->NewStringUTF(env, flag));
        
        jmethodID apply = (*env)->GetMethodID(env, editor_cls, "apply", "()V");
        (*env)->CallVoidMethod(env, editor, apply);
        
        sprintf(ret, "OK:%s", r.message);
        
    } else if (r.expired) {
        sprintf(ret, "EXP:%s", r.message);
    } else {
        sprintf(ret, "ERR:%s", r.message);
    }
    
    return (*env)->NewStringUTF(env, ret);
}

JNIEXPORT jstring JNICALL
Java_com_yourpackage_license_LicenseProtector_nativeGetTargetActivity(
        JNIEnv* env, jobject thiz) {
    char key[16] = {0}, act[64] = {0};
    load_config(key, act);
    return (*env)->NewStringUTF(env, act[0] ? act : NULL);
}
