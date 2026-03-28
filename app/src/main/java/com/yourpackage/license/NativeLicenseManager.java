package com.yourpackage.license;

import android.content.Context;
import android.content.Intent;
import android.app.Activity;

public class NativeLicenseManager {
    // 单例模式，全局唯一实例
    private static volatile NativeLicenseManager instance;
    private Context appContext;

    // 静态加载SO库
    static {
        System.loadLibrary("license_guard");
    }

    // 私有构造，禁止外部实例化
    private NativeLicenseManager() {}

    // 获取单例
    public static NativeLicenseManager getInstance() {
        if (instance == null) {
            synchronized (NativeLicenseManager.class) {
                if (instance == null) {
                    instance = new NativeLicenseManager();
                }
            }
        }
        return instance;
    }

    // 初始化：必须在Application启动时调用，这里在Activity里初始化
    public void init(Context context) {
        this.appContext = context.getApplicationContext();
        nativeInit(appContext);
    }

    // ====================  Native方法声明（Java层无任何实现，全在SO里） ====================
    private native void nativeInit(Context context);

    // 授权页生命周期回调：返回是否已注册，已注册直接跳主页面
    public native boolean onLicenseActivityCreate(Activity activity);

    // 获取机器ID
    public native String getMachineId();

    // 校验注册码：返回提示文案，内部处理成功后的跳转，Java层只负责展示
    public native String verifyLicenseCode(Activity activity, String licenseCode);

    // 主页面权限校验：返回是否允许进入，不允许直接关闭页面
    public native boolean checkMainActivityAccess(Activity activity);

    // 清除注册信息
    public native void clearRegistration();

    // ====================  Native回调方法（SO层调用，Java层仅执行跳转） ====================
    // 由Native层调用，跳转到主页面，Java层无任何判断
    public void jumpToMainActivity(Activity activity) {
        if (activity == null || activity.isFinishing() || activity.isDestroyed()) {
            return;
        }
        Intent intent = new Intent(activity, MainActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        activity.startActivity(intent);
        activity.finish();
    }

    // 由Native层调用，关闭主页面，退出应用
    public void finishMainActivity(Activity activity) {
        if (activity == null || activity.isFinishing() || activity.isDestroyed()) {
            return;
        }
        activity.finish();
        // 彻底退出应用，防止后台残留
        System.exit(0);
    }
}
