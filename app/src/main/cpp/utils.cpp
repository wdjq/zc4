#include <jni.h>
#include <string>

// 生成设备ID（Java层调用）
extern "C" {
    JNIEXPORT jstring JNICALL
    Java_com_yourpackage_license_LicenseActivity_generateMachineId(
            JNIEnv* env, jobject thiz) {
        std::string machineId = "device_" + std::to_string(time(nullptr) % 10000);
        return env->NewStringUTF(machineId.c_str());
    }
}
