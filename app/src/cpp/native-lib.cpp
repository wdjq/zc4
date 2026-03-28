#include <jni.h>
#include <string>
#include <fstream>
#include <vector>
#include <ctime>
#include <android/log.h>

#define LOG_TAG "NativeLicense"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// 私有状态存储路径（Java层不知道）
#define LICENSE_FILE "/data/data/com.yourpackage/license.dat"
// 私有密钥（用于混淆）
#define PRIVATE_KEY "your_private_key_123"

// 生成设备ID
std::string generateMachineId() {
    // 这里可以使用更复杂的设备ID生成逻辑
    return "device_" + std::to_string(time(nullptr) % 10000);
}

// 验证注册码
bool verifyLicenseCode(const std::string& code, const std::string& machineId) {
    // 实际验证逻辑（这里简化为固定码）
    std::string expectedCode = "VALID_" + machineId;
    return code == expectedCode;
}

// 写入验证状态
void writeLicenseStatus(bool valid, time_t expiryTime) {
    std::ofstream file(LICENSE_FILE);
    if (file.is_open()) {
        // 加密存储（简单异或）
        std::string data = std::to_string(valid) + "|" + std::to_string(expiryTime);
        for (char& c : data) {
            c ^= PRIVATE_KEY[0];
        }
        file << data;
        file.close();
    }
}

// 读取验证状态
bool readLicenseStatus(time_t& expiryTime) {
    std::ifstream file(LICENSE_FILE);
    if (file.is_open()) {
        std::string data;
        file >> data;
        file.close();
        
        // 解密
        for (char& c : data) {
            c ^= PRIVATE_KEY[0];
        }
        
        size_t pos = data.find('|');
        if (pos != std::string::npos) {
            bool valid = std::stoi(data.substr(0, pos));
            expiryTime = std::stol(data.substr(pos + 1));
            return valid;
        }
    }
    return false;
}

// 检查是否可信（主要检查时间）
bool isTrusted() {
    time_t expiryTime;
    bool valid = readLicenseStatus(expiryTime);
    
    if (!valid) return false;
    
    time_t currentTime = time(nullptr);
    return currentTime <= expiryTime;
}

// JNI函数声明
extern "C" {
    JNIEXPORT jboolean JNICALL
    Java_com_yourpackage_license_LicenseActivity_nativeVerifyLicense(
            JNIEnv* env, jobject thiz, jstring license_code, jstring machine_id) {
        
        const char* code = env->GetStringUTFChars(license_code, 0);
        const char* id = env->GetStringUTFChars(machine_id, 0);
        
        bool verify_ok = verifyLicenseCode(code, id);
        
        if (verify_ok) {
            // 设置一个较短的测试有效期（实际应用中应根据注册码设置）
            time_t expiryTime = time(nullptr) + 3600; // 1小时后过期
            writeLicenseStatus(true, expiryTime);
        }
        
        env->ReleaseStringUTFChars(license_code, code);
        env->ReleaseStringUTFChars(machine_id, id);
        
        return verify_ok;
    }
    
    JNIEXPORT jboolean JNICALL
    Java_com_yourpackage_license_LicenseActivity_nativeIsTrusted(
            JNIEnv* env, jobject thiz) {
        return isTrusted();
    }
}
