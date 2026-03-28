package com.yourpackage.license;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // 强制Native层校验，不允许进入直接关闭+退出应用
        boolean hasAccess = NativeLicenseManager.getInstance().checkMainActivityAccess(this);
        if (!hasAccess) {
            return;
        }

        // 校验通过，正常加载业务页面
        TextView tvContent = new TextView(this);
        tvContent.setText("欢迎进入主页面，授权验证通过！");
        tvContent.setTextSize(20);
        tvContent.setPadding(50, 50, 50, 50);
        setContentView(tvContent);
    }
}
