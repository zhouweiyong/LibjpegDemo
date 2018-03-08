package com.vst.libjpegdemo;

import android.graphics.Bitmap;

/**
 * Created by zwy on 2018/2/1.
 * email:16681805@qq.com
 */

public class NativeCompress {

    static {
        System.loadLibrary("native-lib");
    }
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public static native void compress(String outPath, Bitmap bitmap, int ratio, boolean isUseHoffman, NativeCallBack nativeCallBack);
}
