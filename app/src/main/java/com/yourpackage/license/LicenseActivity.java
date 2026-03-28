package com.yourpackage.license;

import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;

public class LicenseActivity extends AppCompatActivity {
    
    private EditText licenseCodeEditText;
    private Button verifyButton;
    private TextView machineIdTextView; // 新增：用于显示设备ID的文本控件
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_license);
        
        // 1. 初始化视图
        machineIdTextView = findViewById(R.id.tv_machine_id); // 对应 XML 中新增的 ID
        licenseCodeEditText = findViewById(R.id.license_code);
        verifyButton = findViewById(R.id.verify_button);
        
        // 2. 获取并显示设备 ID
        // 调用 Native 方法生成 ID
        String machineId = generateMachineId();
        // 将 ID 显示在 TextView 上，方便用户查看或复制
        machineIdTextView.setText("设备ID: " + machineId);
        
        // 3. 设置验证按钮点击事件
        verifyButton.setOnClickListener(v -> {
            String licenseCode = licenseCodeEditText.getText().toString();
            
            // 再次获取 ID 进行验证（或者直接使用上面生成的变量 machineId 也可以）
            String currentMachineId = generateMachineId();
            
            // 调用 Native 验证方法
            if (nativeVerifyLicense(licenseCode, currentMachineId)) {
                gotoMainActivity();
            } else {
                // 验证失败，提示用户
                Toast.makeText(this, "许可码验证失败，请检查输入", Toast.LENGTH_SHORT).show();
            }
        });
    }
    
    @Override
    protected void onResume() {
        super.onResume();
        // 如果之前验证过且被信任（例如缓存了状态），直接跳转
        if (nativeIsTrusted()) {
            gotoMainActivity();
        }
    }
    
    private void gotoMainActivity() {
        Intent intent = new Intent(this, MainActivity.class);
        startActivity(intent);
        finish(); // 关闭当前验证页面
    }
    
    // --- Native 方法声明 ---
    
    // 生成设备ID
    public native String generateMachineId();
    
    // 验证注册码
    public native boolean nativeVerifyLicense(String licenseCode, String machineId);
    
    // 检查是否可信
    public native boolean nativeIsTrusted();
    
    static {
        System.loadLibrary("native-lib");
    }
}
