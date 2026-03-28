#include <jni.h>
#include <string>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <climits>
#include <sys/time.h>
#include <unistd.h>
#include <android/log.h>

// ====================  配置项（必须和原Java代码完全一致，保证注册码兼容） ====================
#define AES_KEY "Format2044153997"
#define KEY_VALUE 0x7B
#define PREF_NAME "FAppProtect"
#define KEY_REGISTERED "F1591123968328"
#define KEY_EXPIRE_TIME "ttt"
#define KEY_MACHINE_ID "sss"
#define KEY_ID "id"
// 【必须修改】替换成你的APK签名的MD5值（32位小写，去掉冒号），防止二次打包
#define APK_SIGN_MD5 "b6ba337a1926805805b01f1f1043e7e2"
#define AES_BLOCK_SIZE 16
#define LOG_TAG "LICENSE_GUARD"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ====================  AES标准实现（和Java AES/ECB/PKCS5Padding 100%兼容） ====================
static const uint8_t sbox[256] = {
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};
static const uint8_t rsbox[256] = {
    0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
    0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
    0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
    0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
    0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
    0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
    0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
    0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
    0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
    0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
    0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
    0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
    0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
    0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
    0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};
static const uint32_t Rcon[11] = {
    0x00000000, 0x00000001, 0x00000002, 0x00000004,
    0x00000008, 0x00000010, 0x00000020, 0x00000040,
    0x00000080, 0x0000001B, 0x00000036
};

#define ROTL8(x, shift) ((uint8_t)((x << shift) | (x >> (8 - shift))))
#define F2(x) (((x) << 1) ^ ((((x) >> 7) & 1) * 0x1B))
#define F3(x) (F2(x) ^ (x))

typedef struct {
    uint32_t ek[44];
    uint32_t dk[44];
} aes_context;

// 全局授权上下文（Native层独占，Java层完全访问不到）
typedef struct {
    jobject app_context;
    jclass manager_cls;
    jmethodID jump_method;
    jmethodID finish_method;
    bool is_registered;
    int64_t expire_time;
    int machine_id;
    bool is_sign_valid;
} LicenseContext;
static LicenseContext *g_ctx = nullptr;

// ====================  AES核心实现 ====================
static void aes_key_expansion(aes_context *ctx, const uint8_t *key) {
    uint32_t *ek = ctx->ek;
    for (int i = 0; i < 4; i++) {
        ek[i] = (key[4*i] << 24) | (key[4*i+1] << 16) | (key[4*i+2] << 8) | key[4*i+3];
    }
    for (int i = 4; i < 44; i++) {
        uint32_t temp = ek[i-1];
        if (i % 4 == 0) {
            temp = (temp << 8) | (temp >> 24);
            uint8_t *p = (uint8_t*)&temp;
            p[0] = sbox[p[0]]; p[1] = sbox[p[1]]; p[2] = sbox[p[2]]; p[3] = sbox[p[3]];
            temp ^= Rcon[i/4] << 24;
        }
        ek[i] = ek[i-4] ^ temp;
    }
    for (int i = 0; i < 44; i++) ctx->dk[i] = ek[44 - i];
    for (int i = 1; i < 10; i++) {
        for (int j = 0; j < 4; j++) {
            uint32_t t = ctx->dk[i*4 + j];
            uint8_t *p = (uint8_t*)&t;
            uint8_t a = p[0], b = p[1], c = p[2], d = p[3];
            p[0] = (F2(a)*14) ^ (F3(b)*11) ^ (F2(c)*13) ^ (F3(d)*9);
            p[1] = (F3(a)*9) ^ (F2(b)*14) ^ (F3(c)*11) ^ (F2(d)*13);
            p[2] = (F2(a)*13) ^ (F3(b)*9) ^ (F2(c)*14) ^ (F3(d)*11);
            p[3] = (F3(a)*11) ^ (F2(b)*13) ^ (F3(c)*9) ^ (F2(d)*14);
            ctx->dk[i*4 + j] = t;
        }
    }
}

static int aes_ecb_decrypt(aes_context *ctx, const uint8_t *input, uint8_t *output, int length) {
    if (length % AES_BLOCK_SIZE != 0) return -1;
    for (int offset = 0; offset < length; offset += AES_BLOCK_SIZE) {
        const uint8_t *in = input + offset;
        uint8_t *out = output + offset;
        uint32_t state[4];
        for (int i = 0; i < 4; i++) {
            state[i] = (in[4*i] << 24) | (in[4*i+1] << 16) | (in[4*i+2] << 8) | in[4*i+3];
            state[i] ^= ctx->dk[i];
        }
        for (int round = 1; round < 10; round++) {
            uint8_t tmp[16];
            uint8_t *p = (uint8_t*)state;
            tmp[0] = p[0]; tmp[1] = p[13]; tmp[2] = p[10]; tmp[3] = p[7];
            tmp[4] = p[4]; tmp[5] = p[1]; tmp[6] = p[14]; tmp[7] = p[11];
            tmp[8] = p[8]; tmp[9] = p[5]; tmp[10] = p[2]; tmp[11] = p[15];
            tmp[12] = p[12]; tmp[13] = p[9]; tmp[14] = p[6]; tmp[15] = p[3];
            for (int i = 0; i < 16; i++) tmp[i] = rsbox[tmp[i]];
            for (int i = 0; i < 4; i++) {
                uint8_t a = tmp[i*4], b = tmp[i*4+1], c = tmp[i*4+2], d = tmp[i*4+3];
                tmp[i*4] = (F2(a)*14) ^ (F3(b)*11) ^ (F2(c)*13) ^ (F3(d)*9);
                tmp[i*4+1] = (F3(a)*9) ^ (F2(b)*14) ^ (F3(c)*11) ^ (F2(d)*13);
                tmp[i*4+2] = (F2(a)*13) ^ (F3(b)*9) ^ (F2(c)*14) ^ (F3(d)*11);
                tmp[i*4+3] = (F3(a)*11) ^ (F2(b)*13) ^ (F3(c)*9) ^ (F2(d)*14);
            }
            for (int i = 0; i < 4; i++) {
                state[i] = (tmp[4*i] << 24) | (tmp[4*i+1] << 16) | (tmp[4*i+2] << 8) | tmp[4*i+3];
                state[i] ^= ctx->dk[round*4 + i];
            }
        }
        uint8_t tmp[16];
        uint8_t *p = (uint8_t*)state;
        tmp[0] = p[0]; tmp[1] = p[13]; tmp[2] = p[10]; tmp[3] = p[7];
        tmp[4] = p[4]; tmp[5] = p[1]; tmp[6] = p[14]; tmp[7] = p[11];
        tmp[8] = p[8]; tmp[9] = p[5]; tmp[10] = p[2]; tmp[11] = p[15];
        tmp[12] = p[12]; tmp[13] = p[9]; tmp[14] = p[6]; tmp[15] = p[3];
        for (int i = 0; i < 16; i++) tmp[i] = rsbox[tmp[i]];
        for (int i = 0; i < 4; i++) {
            state[i] = (tmp[4*i] << 24) | (tmp[4*i+1] << 16) | (tmp[4*i+2] << 8) | tmp[4*i+3];
            state[i] ^= ctx->dk[40 + i];
        }
        p = (uint8_t*)state;
        for (int i = 0; i < 16; i++) out[i] = p[i];
    }
    return 0;
}

static int pkcs7_unpad(uint8_t *data, int length) {
    if (length % AES_BLOCK_SIZE != 0) return -1;
    uint8_t pad_len = data[length - 1];
    if (pad_len < 1 || pad_len > AES_BLOCK_SIZE) return -1;
    for (int i = length - pad_len; i < length; i++) {
        if (data[i] != pad_len) return -1;
    }
    return length - pad_len;
}

// ====================  工具函数 ====================
static int hexToBytes(const char* hex, uint8_t* output, int max_len) {
    int len = strlen(hex);
    if (len % 2 != 0) return -1;
    int out_len = len / 2;
    if (out_len > max_len) return -1;
    for (int i = 0; i < len; i += 2) {
        char c1 = hex[i], c2 = hex[i+1];
        int v1 = 0, v2 = 0;
        if (c1 >= '0' && c1 <= '9') v1 = c1 - '0';
        else if (c1 >= 'a' && c1 <= 'f') v1 = 10 + c1 - 'a';
        else if (c1 >= 'A' && c1 <= 'F') v1 = 10 + c1 - 'A';
        else return -1;
        if (c2 >= '0' && c2 <= '9') v2 = c2 - '0';
        else if (c2 >= 'a' && c2 <= 'f') v2 = 10 + c2 - 'a';
        else if (c2 >= 'A' && c2 <= 'F') v2 = 10 + c2 - 'A';
        else return -1;
        output[i/2] = (uint8_t)((v1 << 4) | v2);
    }
    return out_len;
}

static std::string decrypt(const std::string& cipher_text) {
    if (cipher_text.empty()) return "";
    int data_len = cipher_text.length() / 2;
    uint8_t *encrypted = new uint8_t[data_len];
    if (hexToBytes(cipher_text.c_str(), encrypted, data_len) < 0) {
        delete[] encrypted;
        return "";
    }
    aes_context ctx;
    uint8_t key[16];
    memcpy(key, AES_KEY, 16);
    aes_key_expansion(&ctx, key);
    uint8_t *decrypted = new uint8_t[data_len];
    if (aes_ecb_decrypt(&ctx, encrypted, decrypted, data_len) != 0) {
        delete[] encrypted; delete[] decrypted;
        return "";
    }
    int plain_len = pkcs7_unpad(decrypted, data_len);
    if (plain_len < 0) {
        delete[] encrypted; delete[] decrypted;
        return "";
    }
    std::string result((char*)decrypted, plain_len);
    delete[] encrypted; delete[] decrypted;
    return result;
}

static int64_t getCurrentTimeMillis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// ====================  APK签名校验（防止二次打包） ====================
static std::string getSignatureMd5(JNIEnv *env, jobject context) {
    jclass context_cls = env->GetObjectClass(context);
    jmethodID get_pm_method = env->GetMethodID(context_cls, "getPackageManager", "()Landroid/content/pm/PackageManager;");
    jobject pm = env->CallObjectMethod(context, get_pm_method);
    jmethodID get_pn_method = env->GetMethodID(context_cls, "getPackageName", "()Ljava/lang/String;");
    jstring package_name = (jstring)env->CallObjectMethod(context, get_pn_method);

    jclass pm_cls = env->GetObjectClass(pm);
    jmethodID get_pi_method = env->GetMethodID(pm_cls, "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
    jobject package_info = env->CallObjectMethod(pm, get_pi_method, package_name, 0x40);

    jclass pi_cls = env->GetObjectClass(package_info);
    jfieldID signatures_field = env->GetFieldID(pi_cls, "signatures", "[Landroid/content/pm/Signature;");
    jobjectArray signatures = (jobjectArray)env->GetObjectField(package_info, signatures_field);
    jobject signature = env->GetObjectArrayElement(signatures, 0);

    jclass sig_cls = env->GetObjectClass(signature);
    jmethodID toByteArray_method = env->GetMethodID(sig_cls, "toByteArray", "()[B");
    jbyteArray signature_bytes = (jbyteArray)env->CallObjectMethod(signature, toByteArray_method);

    jclass md_cls = env->FindClass("java/security/MessageDigest");
    jmethodID getInstance_method = env->GetStaticMethodID(md_cls, "getInstance", "(Ljava/lang/String;)Ljava/security/MessageDigest;");
    jstring md5_str = env->NewStringUTF("MD5");
    jobject md = env->CallStaticObjectMethod(md_cls, getInstance_method, md5_str);
    jmethodID update_method = env->GetMethodID(md_cls, "update", "([B)V");
    env->CallVoidMethod(md, update_method, signature_bytes);
    jmethodID digest_method = env->GetMethodID(md_cls, "digest", "()[B");
    jbyteArray digest_bytes = (jbyteArray)env->CallObjectMethod(md, digest_method);

    jsize len = env->GetArrayLength(digest_bytes);
    jbyte *digest = env->GetByteArrayElements(digest_bytes, NULL);
    char md5_result[33] = {0};
    for (int i = 0; i < len; i++) {
        sprintf(md5_result + i*2, "%02x", (uint8_t)digest[i]);
    }
    env->ReleaseByteArrayElements(digest_bytes, digest, 0);

    // 释放局部引用
    env->DeleteLocalRef(context_cls);
    env->DeleteLocalRef(pm);
    env->DeleteLocalRef(package_name);
    env->DeleteLocalRef(pm_cls);
    env->DeleteLocalRef(package_info);
    env->DeleteLocalRef(pi_cls);
    env->DeleteLocalRef(signatures);
    env->DeleteLocalRef(signature);
    env->DeleteLocalRef(sig_cls);
    env->DeleteLocalRef(signature_bytes);
    env->DeleteLocalRef(md_cls);
    env->DeleteLocalRef(md5_str);
    env->DeleteLocalRef(md);
    env->DeleteLocalRef(digest_bytes);

    return std::string(md5_result);
}

// ====================  SharedPreferences操作（Native层独占，Java层碰不到） ====================
static jobject getSharedPreferences(JNIEnv *env, jobject context) {
    jclass context_cls = env->GetObjectClass(context);
    jmethodID get_sp_method = env->GetMethodID(context_cls, "getSharedPreferences", "(Ljava/lang/String;I)Landroid/content/SharedPreferences;");
    jstring pref_name = env->NewStringUTF(PREF_NAME);
    jobject sp = env->CallObjectMethod(context, get_sp_method, pref_name, 0);
    env->DeleteLocalRef(pref_name);
    env->DeleteLocalRef(context_cls);
    return sp;
}

static int getSPInt(JNIEnv *env, jobject context, const char* key, int default_value) {
    jobject sp = getSharedPreferences(env, context);
    jclass sp_cls = env->GetObjectClass(sp);
    jmethodID get_int_method = env->GetMethodID(sp_cls, "getInt", "(Ljava/lang/String;I)I");
    jstring key_str = env->NewStringUTF(key);
    int value = env->CallIntMethod(sp, get_int_method, key_str, default_value);
    env->DeleteLocalRef(key_str);
    env->DeleteLocalRef(sp_cls);
    env->DeleteLocalRef(sp);
    return value;
}

static void putSPInt(JNIEnv *env, jobject context, const char* key, int value) {
    jobject sp = getSharedPreferences(env, context);
    jclass sp_cls = env->GetObjectClass(sp);
    jmethodID edit_method = env->GetMethodID(sp_cls, "edit", "()Landroid/content/SharedPreferences$Editor;");
    jobject editor = env->CallObjectMethod(sp, edit_method);
    jclass editor_cls = env->GetObjectClass(editor);
    jmethodID put_int_method = env->GetMethodID(editor_cls, "putInt", "(Ljava/lang/String;I)Landroid/content/SharedPreferences$Editor;");
    jstring key_str = env->NewStringUTF(key);
    editor = env->CallObjectMethod(editor, put_int_method, key_str, value);
    jmethodID apply_method = env->GetMethodID(editor_cls, "apply", "()V");
    env->CallVoidMethod(editor, apply_method);
    env->DeleteLocalRef(key_str);
    env->DeleteLocalRef(editor_cls);
    env->DeleteLocalRef(editor);
    env->DeleteLocalRef(sp_cls);
    env->DeleteLocalRef(sp);
}

static int64_t getSPLong(JNIEnv *env, jobject context, const char* key, int64_t default_value) {
    jobject sp = getSharedPreferences(env, context);
    jclass sp_cls = env->GetObjectClass(sp);
    jmethodID get_long_method = env->GetMethodID(sp_cls, "getLong", "(Ljava/lang/String;J)J");
    jstring key_str = env->NewStringUTF(key);
    int64_t value = env->CallLongMethod(sp, get_long_method, key_str, default_value);
    env->DeleteLocalRef(key_str);
    env->DeleteLocalRef(sp_cls);
    env->DeleteLocalRef(sp);
    return value;
}

static void putSPLong(JNIEnv *env, jobject context, const char* key, int64_t value) {
    jobject sp = getSharedPreferences(env, context);
    jclass sp_cls = env->GetObjectClass(sp);
    jmethodID edit_method = env->GetMethodID(sp_cls, "edit", "()Landroid/content/SharedPreferences$Editor;");
    jobject editor = env->CallObjectMethod(sp, edit_method);
    jclass editor_cls = env->GetObjectClass(editor);
    jmethodID put_long_method = env->GetMethodID(editor_cls, "putLong", "(Ljava/lang/String;J)Landroid/content/SharedPreferences$Editor;");
    jstring key_str = env->NewStringUTF(key);
    editor = env->CallObjectMethod(editor, put_long_method, key_str, value);
    jmethodID apply_method = env->GetMethodID(editor_cls, "apply", "()V");
    env->CallVoidMethod(editor, apply_method);
    env->DeleteLocalRef(key_str);
    env->DeleteLocalRef(editor_cls);
    env->DeleteLocalRef(editor);
    env->DeleteLocalRef(sp_cls);
    env->DeleteLocalRef(sp);
}

static bool getSPBoolean(JNIEnv *env, jobject context, const char* key, bool default_value) {
    jobject sp = getSharedPreferences(env, context);
    jclass sp_cls = env->GetObjectClass(sp);
    jmethodID get_boolean_method = env->GetMethodID(sp_cls, "getBoolean", "(Ljava/lang/String;Z)Z");
    jstring key_str = env->NewStringUTF(key);
    bool value = env->CallBooleanMethod(sp, get_boolean_method, key_str, default_value);
    env->DeleteLocalRef(key_str);
    env->DeleteLocalRef(sp_cls);
    env->DeleteLocalRef(sp);
    return value;
}

static void putSPBoolean(JNIEnv *env, jobject context, const char* key, bool value) {
    jobject sp = getSharedPreferences(env, context);
    jclass sp_cls = env->GetObjectClass(sp);
    jmethodID edit_method = env->GetMethodID(sp_cls, "edit", "()Landroid/content/SharedPreferences$Editor;");
    jobject editor = env->CallObjectMethod(sp, edit_method);
    jclass editor_cls = env->GetObjectClass(editor);
    jmethodID put_boolean_method = env->GetMethodID(editor_cls, "putBoolean", "(Ljava/lang/String;Z)Landroid/content/SharedPreferences$Editor;");
    jstring key_str = env->NewStringUTF(key);
    editor = env->CallObjectMethod(editor, put_boolean_method, key_str, value);
    jmethodID apply_method = env->GetMethodID(editor_cls, "apply", "()V");
    env->CallVoidMethod(editor, apply_method);
    env->DeleteLocalRef(key_str);
    env->DeleteLocalRef(editor_cls);
    env->DeleteLocalRef(editor);
    env->DeleteLocalRef(sp_cls);
    env->DeleteLocalRef(sp);
}

static void removeSPKey(JNIEnv *env, jobject context, const char* key) {
    jobject sp = getSharedPreferences(env, context);
    jclass sp_cls = env->GetObjectClass(sp);
    jmethodID edit_method = env->GetMethodID(sp_cls, "edit", "()Landroid/content/SharedPreferences$Editor;");
    jobject editor = env->CallObjectMethod(sp, edit_method);
    jclass editor_cls = env->GetObjectClass(editor);
    jmethodID remove_method = env->GetMethodID(editor_cls, "remove", "(Ljava/lang/String;)Landroid/content/SharedPreferences$Editor;");
    jstring key_str = env->NewStringUTF(key);
    editor = env->CallObjectMethod(editor, remove_method, key_str);
    jmethodID apply_method = env->GetMethodID(editor_cls, "apply", "()V");
    env->CallVoidMethod(editor, apply_method);
    env->DeleteLocalRef(key_str);
    env->DeleteLocalRef(editor_cls);
    env->DeleteLocalRef(editor);
    env->DeleteLocalRef(sp_cls);
    env->DeleteLocalRef(sp);
}

// ====================  核心业务逻辑（全在Native层） ====================
static void loadSavedState(JNIEnv *env) {
    if (g_ctx == nullptr) return;
    g_ctx->is_registered = getSPBoolean(env, g_ctx->app_context, KEY_REGISTERED, false);
    g_ctx->expire_time = getSPLong(env, g_ctx->app_context, KEY_EXPIRE_TIME, 0);
    g_ctx->machine_id = getSPInt(env, g_ctx->app_context, KEY_MACHINE_ID, -1);

    // 校验有效期，过期自动清除
    if (g_ctx->is_registered && g_ctx->expire_time > 0 && g_ctx->expire_time != LLONG_MAX) {
        if (getCurrentTimeMillis() > g_ctx->expire_time) {
            g_ctx->is_registered = false;
            removeSPKey(env, g_ctx->app_context, KEY_REGISTERED);
            removeSPKey(env, g_ctx->app_context, KEY_EXPIRE_TIME);
            removeSPKey(env, g_ctx->app_context, KEY_MACHINE_ID);
            removeSPKey(env, g_ctx->app_context, KEY_ID);
            g_ctx->expire_time = 0;
            g_ctx->machine_id = -1;
        }
    }
}

static void clearRegistration(JNIEnv *env) {
    if (g_ctx == nullptr) return;
    putSPBoolean(env, g_ctx->app_context, KEY_REGISTERED, false);
    removeSPKey(env, g_ctx->app_context, KEY_EXPIRE_TIME);
    removeSPKey(env, g_ctx->app_context, KEY_MACHINE_ID);
    removeSPKey(env, g_ctx->app_context, KEY_ID);
    g_ctx->is_registered = false;
    g_ctx->expire_time = 0;
    g_ctx->machine_id = -1;
}

static int getMachineId(JNIEnv *env) {
    if (g_ctx == nullptr) return -1;
    if (g_ctx->machine_id == -1) {
        g_ctx->machine_id = getSPInt(env, g_ctx->app_context, KEY_ID, -1);
        if (g_ctx->machine_id == -1) {
            srand(time(NULL));
            g_ctx->machine_id = 100000 + (rand() % 900000);
            putSPInt(env, g_ctx->app_context, KEY_ID, g_ctx->machine_id);
        }
    }
    return g_ctx->machine_id;
}

static void saveRegistration(JNIEnv *env) {
    if (g_ctx == nullptr) return;
    putSPBoolean(env, g_ctx->app_context, KEY_REGISTERED, g_ctx->is_registered);
    putSPLong(env, g_ctx->app_context, KEY_EXPIRE_TIME, g_ctx->expire_time);
    putSPInt(env, g_ctx->app_context, KEY_MACHINE_ID, g_ctx->machine_id);
}

static bool checkRegistration(JNIEnv *env) {
    if (g_ctx == nullptr || !g_ctx->is_sign_valid) return false;
    if (!g_ctx->is_registered) return false;
    if (g_ctx->expire_time > 0 && g_ctx->expire_time != LLONG_MAX) {
        if (getCurrentTimeMillis() > g_ctx->expire_time) {
            g_ctx->is_registered = false;
            clearRegistration(env);
            return false;
        }
    }
    return true;
}

// ====================  JNI方法实现 ====================
static void nativeInit(JNIEnv *env, jobject thiz, jobject context) {
    if (g_ctx != nullptr) return;

    // 初始化全局上下文
    g_ctx = new LicenseContext();
    g_ctx->app_context = env->NewGlobalRef(context);
    g_ctx->is_registered = false;
    g_ctx->expire_time = 0;
    g_ctx->machine_id = -1;

    // 校验APK签名，防止二次打包
    std::string sign_md5 = getSignatureMd5(env, context);
    g_ctx->is_sign_valid = (sign_md5 == APK_SIGN_MD5);
    if (!g_ctx->is_sign_valid) {
        LOGE("APK签名校验失败，应用被篡改！");
        return;
    }

    // 缓存Java层回调方法ID
    g_ctx->manager_cls = (jclass)env->NewGlobalRef(env->GetObjectClass(thiz));
    g_ctx->jump_method = env->GetMethodID(g_ctx->manager_cls, "jumpToMainActivity", "(Landroid/app/Activity;)V");
    g_ctx->finish_method = env->GetMethodID(g_ctx->manager_cls, "finishMainActivity", "(Landroid/app/Activity;)V");

    // 加载本地存储的授权状态
    loadSavedState(env);
    LOGD("Native初始化完成，签名校验：%s，注册状态：%d", g_ctx->is_sign_valid ? "通过" : "失败", g_ctx->is_registered);
}

static jboolean onLicenseActivityCreate(JNIEnv *env, jobject thiz, jobject activity) {
    // 签名无效直接返回false，不允许任何操作
    if (g_ctx == nullptr || !g_ctx->is_sign_valid) return false;

    // 已注册，直接调用Java层跳转主页面
    if (checkRegistration(env)) {
        env->CallVoidMethod(thiz, g_ctx->jump_method, activity);
        return true;
    }
    return false;
}

static jstring getMachineId(JNIEnv *env, jobject thiz) {
    if (g_ctx == nullptr || !g_ctx->is_sign_valid) {
        return env->NewStringUTF("无效ID");
    }
    int machine_id = getMachineId(env);
    char id_str[20] = {0};
    sprintf(id_str, "%d", machine_id);
    return env->NewStringUTF(id_str);
}

static jstring verifyLicenseCode(JNIEnv *env, jobject thiz, jobject activity, jstring license_code) {
    // 签名无效直接拒绝
    if (g_ctx == nullptr || !g_ctx->is_sign_valid) {
        return env->NewStringUTF("应用被篡改，验证失败！");
    }

    const char *code = env->GetStringUTFChars(license_code, NULL);
    std::string result_msg;

    // 非空校验
    if (code == NULL || strlen(code) == 0) {
        result_msg = "请输入注册码";
        env->ReleaseStringUTFChars(license_code, code);
        return env->NewStringUTF(result_msg.c_str());
    }

    // AES解密
    std::string decrypted = decrypt(std::string(code));
    if (decrypted.empty()) {
        result_msg = "注册码解密失败，无效注册码";
        env->ReleaseStringUTFChars(license_code, code);
        return env->NewStringUTF(result_msg.c_str());
    }

    // 解析解密内容
    std::string hex_part;
    int64_t duration = 0;
    size_t z_index = decrypted.find('z');
    if (z_index != std::string::npos) {
        hex_part = decrypted.substr(0, z_index);
        std::string time_str = decrypted.substr(z_index + 1);
        try {
            duration = std::stoll(time_str);
        } catch (...) {
            result_msg = "注册码时间格式错误";
            env->ReleaseStringUTFChars(license_code, code);
            return env->NewStringUTF(result_msg.c_str());
        }
    } else {
        hex_part = decrypted;
    }

    // 解析机器码校验值
    int decrypted_value;
    try {
        decrypted_value = std::stoi(hex_part, nullptr, 16);
    } catch (...) {
        result_msg = "注册码机器码格式错误";
        env->ReleaseStringUTFChars(license_code, code);
        return env->NewStringUTF(result_msg.c_str());
    }

    // 设备绑定校验
    int current_machine_id = getMachineId(env);
    int expected_value = (current_machine_id / 2) + KEY_VALUE;
    if (decrypted_value != expected_value) {
        result_msg = "注册码与设备不匹配";
        env->ReleaseStringUTFChars(license_code, code);
        return env->NewStringUTF(result_msg.c_str());
    }

    // 有效期校验
    int64_t current_time = getCurrentTimeMillis();
    if (duration > 0) {
        g_ctx->expire_time = current_time + duration;
        if (current_time > g_ctx->expire_time) {
            result_msg = "注册码已过期";
            env->ReleaseStringUTFChars(license_code, code);
            return env->NewStringUTF(result_msg.c_str());
        }
    } else {
        g_ctx->expire_time = LLONG_MAX;
    }

    // 全部校验通过，保存注册状态
    g_ctx->is_registered = true;
    saveRegistration(env);

    // 生成提示文案
    if (g_ctx->expire_time == LLONG_MAX) {
        result_msg = "注册成功！软件使用无期限";
    } else {
        time_t expire_sec = g_ctx->expire_time / 1000;
        struct tm *tm_info = localtime(&expire_sec);
        char date_str[20];
        strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_info);
        int64_t remain_days = (g_ctx->expire_time - current_time) / (1000 * 60 * 60 * 24);
        result_msg = "注册成功！软件使用期至：" + std::string(date_str) + "（剩余" + std::to_string(remain_days) + "天）";
    }

    // 【核心】校验通过，Native层直接触发跳转，Java层无任何控制
    env->CallVoidMethod(thiz, g_ctx->jump_method, activity);

    env->ReleaseStringUTFChars(license_code, code);
    return env->NewStringUTF(result_msg.c_str());
}

static jboolean checkMainActivityAccess(JNIEnv *env, jobject thiz, jobject activity) {
    // 签名无效、注册状态无效，直接关闭页面+退出应用
    if (g_ctx == nullptr || !g_ctx->is_sign_valid || !checkRegistration(env)) {
        env->CallVoidMethod(thiz, g_ctx->finish_method, activity);
        return false;
    }
    return true;
}

static void clearRegistration(JNIEnv *env, jobject thiz) {
    clearRegistration(env);
}

static jboolean isRegistered(JNIEnv *env, jobject thiz) {
    return checkRegistration(env);
}

static jlong getExpireTime(JNIEnv *env, jobject thiz) {
    if (g_ctx == nullptr) return 0;
    return g_ctx->expire_time;
}

// ====================  JNI动态注册 ====================
static const JNINativeMethod gMethods[] = {
    {"nativeInit", "(Landroid/content/Context;)V", (void*)nativeInit},
    {"onLicenseActivityCreate", "(Landroid/app/Activity;)Z", (void*)onLicenseActivityCreate},
    {"getMachineId", "()Ljava/lang/String;", (void*)getMachineId},
    {"verifyLicenseCode", "(Landroid/app/Activity;Ljava/lang/String;)Ljava/lang/String;", (void*)verifyLicenseCode},
    {"checkMainActivityAccess", "(Landroid/app/Activity;)Z", (void*)checkMainActivityAccess},
    {"clearRegistration", "()V", (void*)clearRegistration},
    {"isRegistered", "()Z", (void*)isRegistered},
    {"getExpireTime", "()J", (void*)getExpireTime},
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    jclass cls = env->FindClass("com/yourpackage/license/NativeLicenseManager");
    if (cls == NULL) {
        LOGE("找不到NativeLicenseManager类，请检查包名是否一致！");
        return -1;
    }
    int num_methods = sizeof(gMethods) / sizeof(gMethods[0]);
    if (env->RegisterNatives(cls, gMethods, num_methods) < 0) {
        LOGE("JNI动态注册失败！");
        return -1;
    }
    return JNI_VERSION_1_6;
}
