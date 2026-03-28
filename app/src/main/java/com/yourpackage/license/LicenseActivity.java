package com.yourpackage.license;

import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import androidx.appcompat.app.AppCompatActivity;

public class LicenseActivity extends AppCompatActivity {
    
    private EditText licenseCodeEditText;
    private Button verifyButton;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_license);
        
        licenseCodeEditText = findViewById(R.id.license_code);
        verifyButton = findViewById(R.id.verify_button);
        
        verifyButton.setOnClickListener(v -> {
            String licenseCode = licenseCodeEditText.getText().toString();
            String machineId = generateMachineId();
            
            if (nativeVerifyLicense(licenseCode, machineId)) {
                gotoMainActivity();
            } else {
                // 显示错误
            }
        });
    }
    
    @Override
    protected void onResume() {
        super.onResume();
        if (nativeIsTrusted()) {
            gotoMainActivity();
        }
    }
    
    private void gotoMainActivity() {
        Intent intent = new Intent(this, MainActivity.class);
        startActivity(intent);
        finish();
    }
    
    // 生成设备ID
    private native String generateMachineId();
    
    // 验证注册码
    private native boolean nativeVerifyLicense(String licenseCode, String machineId);
    
    // 检查是否可信
    private native boolean nativeIsTrusted();
    
    static {
        System.loadLibrary("native-lib");
    }
}
