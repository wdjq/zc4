package com.yourpackage.license;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;

public class LicenseProtector {
    static {
        System.loadLibrary("license-core");
    }
    
    /**
     * Native验证并执行跳转（核心方法）
     * 返回格式：
     * - "OK:软件使用期至..."（成功，已跳转）
     * - "EXP:注册码已过期"（过期）
     * - "ERR:..."（其他错误）
     */
    public native String nativeVerifyAndLaunch(String code, int machineId, Activity currentActivity);
    
    /** 获取目标Activity（仅作显示参考） */
    public native String nativeGetTargetActivity();
    
    private final Context context;
    private final SharedPreferences prefs;
    private int machineId = -1;
    
    public LicenseProtector(Context context) {
        this.context = context.getApplicationContext();
        this.prefs = context.getSharedPreferences("LicenseData", Context.MODE_PRIVATE);
        initMachineId();
    }
    
    private void initMachineId() {
        machineId = prefs.getInt("mid", -1);
        if (machineId == -1) {
            // 生成随机6位数
            machineId = 100000 + (int)(Math.random() * 900000);
            prefs.edit().putInt("mid", machineId).apply();
        }
    }
    
    public int getMachineId() {
        return machineId;
    }
}
