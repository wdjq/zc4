package com.license.shell;

import android.app.Activity;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

public class LicenseActivity extends Activity {
    
    static {
        try {
            System.loadLibrary("license_core");
        } catch (Throwable e) {
            suicide();
        }
    }
    
    public static native void nativeCheckEnvironment(Context ctx);
    public static native int nativeGetMachineId();
    public static native void nativeVerifyAndAct(Context ctx, String code);
    
    private EditText etCode;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        
        nativeCheckEnvironment(this);
        initUI();
    }
    
    private void initUI() {
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setBackgroundColor(Color.parseColor("#FF121212"));
        root.setPadding(dp(24), dp(48), dp(24), dp(24));
        
        TextView tvTitle = new TextView(this);
        tvTitle.setText("设备授权验证");
        tvTitle.setTextSize(22);
        tvTitle.setTextColor(Color.WHITE);
        tvTitle.setGravity(Gravity.CENTER);
        tvTitle.setPadding(0, 0, 0, dp(32));
        root.addView(tvTitle);
        
        TextView tvIdLabel = new TextView(this);
        tvIdLabel.setText("您的设备ID（长按复制）：");
        tvIdLabel.setTextSize(14);
        tvIdLabel.setTextColor(Color.GRAY);
        root.addView(tvIdLabel);
        
        final TextView tvId = new TextView(this);
        tvId.setText(String.valueOf(nativeGetMachineId()));
        tvId.setTextSize(28);
        tvId.setTextColor(Color.parseColor("#FF03DAC5"));
        tvId.setGravity(Gravity.CENTER);
        tvId.setPadding(0, dp(8), 0, dp(32));
        
        // 改为传统匿名类，不用Lambda
        tvId.setOnLongClickListener(new View.OnLongClickListener() {
            @Override
            public boolean onLongClick(View v) {
                copyToClipboard(tvId.getText().toString());
                return true;
            }
        });
        root.addView(tvId);
        
        etCode = new EditText(this);
        etCode.setHint("请输入注册码");
        etCode.setTextColor(Color.WHITE);
        etCode.setHintTextColor(Color.GRAY);
        etCode.setBackgroundColor(Color.parseColor("#FF1E1E1E"));
        etCode.setPadding(dp(16), dp(16), dp(16), dp(16));
        etCode.setGravity(Gravity.CENTER);
        LinearLayout.LayoutParams inputParams = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, dp(56));
        root.addView(etCode, inputParams);
        
        Button btn = new Button(this);
        btn.setText("验证并进入");
        btn.setTextColor(Color.WHITE);
        btn.setBackgroundColor(Color.parseColor("#FF6200EE"));
        LinearLayout.LayoutParams btnParams = new LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT, dp(56));
        btnParams.setMargins(0, dp(24), 0, 0);
        
        // 改为传统匿名类，不用Lambda
        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String code = etCode.getText().toString().trim();
                nativeVerifyAndAct(LicenseActivity.this, code);
                suicide();
            }
        });
        root.addView(btn, btnParams);
        
        setContentView(root);
    }
    
    public static void suicide() {
        android.os.Process.killProcess(android.os.Process.myPid());
        System.exit(0);
    }
    
    public static void jumpToTarget(Context ctx, String className) {
        try {
            Intent intent = new Intent(ctx, Class.forName(className));
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
            ctx.startActivity(intent);
            ((Activity)ctx).finish();
        } catch (Exception e) {
            suicide();
        }
    }
    
    public static void showError(Context ctx, String msg) {
        if (ctx instanceof Activity) {
            ((Activity)ctx).runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(ctx, msg, Toast.LENGTH_SHORT).show();
                }
            });
        }
    }
    
    private void copyToClipboard(String text) {
        ClipboardManager cb = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
        cb.setPrimaryClip(ClipData.newPlainText("Machine ID", text));
        Toast.makeText(this, "ID已复制", Toast.LENGTH_SHORT).show();
    }
    
    private int dp(int px) {
        return (int)(px * getResources().getDisplayMetrics().density);
    }
    
    @Override
    public void onBackPressed() {
        suicide();
    }
}
