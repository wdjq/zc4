package com.yourpackage.license;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.io.InputStream;

public class LicenseActivity extends Activity {
    
    private LicenseProtector licenseProtector;
    private EditText etLicenseCode;
    private Button btnEnter;
    private int machineId;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        licenseProtector = new LicenseProtector(this);
        machineId = licenseProtector.getMachineId();
        
        // 检查是否已注册（仅优化体验，真正验证在Native）
        if (checkRegistered()) {
            // 已注册，尝试获取目标并跳转（或让Native处理）
            String target = licenseProtector.nativeGetTargetActivity();
            if (target != null && !target.isEmpty()) {
                gotoActivity(target);
                return;
            }
        }
        
        initUI();
    }
    
    private boolean checkRegistered() {
        SharedPreferences prefs = getSharedPreferences("LicenseConfig", Context.MODE_PRIVATE);
        String flag = prefs.getString("lic_flag", null);
        // 简单检查标记存在性，格式验证在Native
        return flag != null && flag.startsWith("VALID_");
    }
    
    private void initUI() {
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(50, 50, 50, 50);
        
        // 背景
        try {
            InputStream is = getAssets().open("background.jpg");
            Drawable d = Drawable.createFromStream(is, null);
            if (Build.VERSION.SDK_INT >= 16) {
                root.setBackground(d);
            } else {
                root.setBackgroundDrawable(d);
            }
            is.close();
        } catch (Exception e) {
            root.setBackgroundColor(0xFFFFFFFF);
        }
        
        // 标题
        TextView tvTitle = new TextView(this);
        tvTitle.setText("将以下的ID发给我来获取注册码。\n");
        tvTitle.setTextSize(16);
        tvTitle.setTextColor(0xFF000000);
        tvTitle.setPadding(0, 20, 0, 20);
        root.addView(tvTitle);
        
        // ID标签
        TextView tvIdLabel = new TextView(this);
        tvIdLabel.setText("你的ID:");
        tvIdLabel.setTextSize(18);
        tvIdLabel.setTextColor(0xFF000000);
        root.addView(tvIdLabel);
        
        TextView tvMachineId = new TextView(this);
        tvMachineId.setText(String.valueOf(machineId));
        tvMachineId.setTextSize(24);
        tvMachineId.setTextColor(0xFF000000);
        tvMachineId.setPadding(0, 10, 0, 30);
        root.addView(tvMachineId);
        
        // 复制按钮
        Button btnCopy = new Button(this);
        btnCopy.setText("复制ID");
        btnCopy.setOnClickListener(v -> {
            ClipboardManager cb = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
            ClipData clip = ClipData.newPlainText("Machine ID", String.valueOf(machineId));
            cb.setPrimaryClip(clip);
            Toast.makeText(this, "ID已复制", Toast.LENGTH_SHORT).show();
        });
        root.addView(btnCopy);
        
        // 输入框
        etLicenseCode = new EditText(this);
        etLicenseCode.setHint("请输入注册码");
        etLicenseCode.setTextColor(0xFF000000);
        LinearLayout.LayoutParams p = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT);
        p.setMargins(0, 30, 0, 30);
        etLicenseCode.setLayoutParams(p);
        root.addView(etLicenseCode);
        
        // 进入按钮
        btnEnter = new Button(this);
        btnEnter.setText("愉快进入");
        btnEnter.setOnClickListener(v -> verifyLicenseCode());
        root.addView(btnEnter);
        
        setContentView(root);
    }
    
    private void verifyLicenseCode() {
        String code = etLicenseCode.getText().toString().trim();
        
        if (code.isEmpty()) {
            Toast.makeText(this, "请输入注册码", Toast.LENGTH_SHORT).show();
            return;
        }
        
        // === 核心：调用Native验证并执行跳转 ===
        // Native内部已做所有验证，成功立即跳转，失败返回错误
        String result = licenseProtector.nativeVerifyAndLaunch(code, machineId, this);
        
        if (result == null) {
            Toast.makeText(this, "验证异常", Toast.LENGTH_LONG).show();
            return;
        }
        
        // 解析Native返回结果（仅展示用，决策权在Native）
        if (result.startsWith("OK:")) {
            // 成功：Native已启动目标Activity
            String msg = result.substring(3);
            new AlertDialog.Builder(this)
                .setTitle("注册成功")
                .setMessage(msg)
                .setPositiveButton("确定", (d, w) -> finish()) // 关闭自身
                .setCancelable(false)
                .show();
                
        } else if (result.startsWith("EXP:")) {
            // 过期
            String msg = result.substring(4);
            new AlertDialog.Builder(this)
                .setTitle("注册码已过期")
                .setMessage(msg)
                .setPositiveButton("重新输入", null)
                .show();
                
        } else if (result.startsWith("ERR:")) {
            // 其他错误（解密失败、格式错误、不匹配等）
            String msg = result.substring(4);
            btnEnter.setText("失败，点击重试!");
            Toast.makeText(this, msg, Toast.LENGTH_LONG).show();
        }
    }
    
    private void gotoActivity(String targetClass) {
        try {
            Class<?> cls = Class.forName(targetClass);
            Intent intent = new Intent(this, cls);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
            startActivity(intent);
            finish();
        } catch (Exception e) {
            Toast.makeText(this, "启动失败：" + e.getMessage(), Toast.LENGTH_LONG).show();
        }
    }
}
