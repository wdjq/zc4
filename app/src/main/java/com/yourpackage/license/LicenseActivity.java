package com.yourpackage.license;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
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
    private EditText etLicenseCode;
    private TextView tvMachineId;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // 初始化Native管理器
        NativeLicenseManager.getInstance().init(this);

        // Native层判断是否已注册，已注册直接跳主页面，Java层无任何判断
        boolean isRegistered = NativeLicenseManager.getInstance().onLicenseActivityCreate(this);
        if (isRegistered) {
            return;
        }

        // 未注册，初始化UI（纯UI渲染，无业务逻辑）
        initUI();
    }

    private void initUI() {
        LinearLayout rootLayout = new LinearLayout(this);
        rootLayout.setOrientation(LinearLayout.VERTICAL);
        rootLayout.setPadding(50, 50, 50, 50);

        // 背景设置
        try {
            InputStream is = getAssets().open("background.jpg");
            Drawable drawable = Drawable.createFromStream(is, null);
            if (drawable != null) {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
                    rootLayout.setBackground(drawable);
                } else {
                    rootLayout.setBackgroundDrawable(drawable);
                }
            }
            is.close();
        } catch (Exception e) {
            rootLayout.setBackgroundColor(0xFFFFFFFF);
        }

        // 标题
        TextView tvTitle = new TextView(this);
        tvTitle.setText("合理安排时间，健康观看影视\n\n将以下的ID发给你的监护人来获取注册码。\n");
        tvTitle.setTextSize(16);
        tvTitle.setTextColor(0xFF000000);
        tvTitle.setPadding(0, 20, 0, 20);
        rootLayout.addView(tvTitle);

        // 机器ID标签
        TextView tvIdLabel = new TextView(this);
        tvIdLabel.setText("你的ID:");
        tvIdLabel.setTextSize(18);
        tvIdLabel.setTextColor(0xFF000000);
        rootLayout.addView(tvIdLabel);

        // 机器ID展示：从Native层获取
        tvMachineId = new TextView(this);
        tvMachineId.setText(NativeLicenseManager.getInstance().getMachineId());
        tvMachineId.setTextSize(24);
        tvMachineId.setTextColor(0xFF000000);
        tvMachineId.setPadding(0, 10, 0, 30);
        rootLayout.addView(tvMachineId);

        // 复制ID按钮
        Button btnCopyId = new Button(this);
        btnCopyId.setText("复制ID");
        btnCopyId.setOnClickListener(v -> {
            // 纯UI操作，ID从Native层获取
            String machineId = NativeLicenseManager.getInstance().getMachineId();
            ClipboardManager clipboard = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
            ClipData clip = ClipData.newPlainText("Machine ID", machineId);
            clipboard.setPrimaryClip(clip);
            Toast.makeText(this, "ID已复制", Toast.LENGTH_SHORT).show();
        });
        rootLayout.addView(btnCopyId);

        // 注册码输入框
        etLicenseCode = new EditText(this);
        etLicenseCode.setHint("请输入注册码");
        etLicenseCode.setTextColor(0xFF000000);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT);
        params.setMargins(0, 30, 0, 30);
        etLicenseCode.setLayoutParams(params);
        rootLayout.addView(etLicenseCode);

        // 验证按钮
        Button btnEnter = new Button(this);
        btnEnter.setText("愉快进入");
        btnEnter.setOnClickListener(v -> {
            // 所有校验逻辑全在Native层，Java层只负责传参和展示返回的提示
            String inputCode = etLicenseCode.getText().toString().trim();
            String resultMsg = NativeLicenseManager.getInstance().verifyLicenseCode(this, inputCode);

            // 仅展示提示，无任何成功/失败判断，成功跳转由Native层内部触发
            if (resultMsg != null && !resultMsg.isEmpty()) {
                new AlertDialog.Builder(this)
                        .setMessage(resultMsg)
                        .setPositiveButton("确定", null)
                        .setCancelable(true)
                        .show();
            }
        });
        rootLayout.addView(btnEnter);

        setContentView(rootLayout);
    }
}
