package com.vst.libjpegdemo;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
//        File file = new File(Environment.getExternalStorageDirectory(), "tttimg.jpg");
//        Bitmap bitmap = ImageUtils.compressPxSampleSize(getResources(), R.mipmap.timg, 500, 500);
        Bitmap bm1 = ImageUtils.compressPxSampleSize(new File(Environment.getExternalStorageDirectory(), "tttest.jpg").getAbsolutePath(), 500, 500);
        ImageUtils.compressQC(new File(Environment.getExternalStorageDirectory(), "tttestpr.jpg").getAbsolutePath(), bm1, 100, true, new NativeCallBack() {
            @Override
            public void startCompress() {

            }

            @Override
            public void error(int errorCode, String des) {
                Log.i("zwy", des);
            }

            @Override
            public void finish(String filePath) {
                Log.i("zwy", "压缩完成>>" + filePath);
            }
        }, true);

        TextView tv = (TextView) findViewById(R.id.sample_text);
        ImageView iv = findViewById(R.id.iv);
        iv.setImageBitmap(bm1);
    }


}
