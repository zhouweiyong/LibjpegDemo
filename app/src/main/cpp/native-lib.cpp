#include <jni.h>
#include <string>
#include <iostream>
#include <csignal>
#include <setjmp.h>
#include <stdio.h>
#include <malloc.h>
#include <android/bitmap.h>
#include <mconst.h>

extern "C" {
#include <jpeglib.h>
#include <jerror.h>
#include <jmorecfg.h>
#include <jconfig.h>
}
using namespace std;

//导入日志头文件
#include <android/log.h>
//修改日志tag中的值
#define LOG_TAG "logfromc"
//日志显示的等级
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)


jobject callBack;
JNIEnv *menv;
typedef unsigned char BYTE;

extern "C"
JNIEXPORT void JNICALL
Java_com_vst_libjpegdemo_NativeCompress_compress(JNIEnv *env, jobject instance, jstring outPath_,
                                                 jobject bitmap, jint ratio, jboolean isUseHoffman,
                                                 jobject nativeCallBack);

int generateJPEG(BYTE *data, int w, int h, int quality,
                 const char *outfilename, jboolean optimize);


struct my_error_mgr {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};
typedef struct my_error_mgr *my_error_ptr;

//错误的方法回调
METHODDEF(void) my_error_exit(j_common_ptr cinfo) {
    my_error_ptr myerr = (my_error_ptr) cinfo->err;
    (*cinfo->err->output_message)(cinfo);
    char *error = (char *) myerr->pub.jpeg_message_table[myerr->pub.msg_code];

    jclass nativeCallBackClass = menv->FindClass("org/jpegutil/fjpeg/minterface/NativeCallBack");
    jmethodID errorMthodId = menv->GetMethodID(nativeCallBackClass, "error",
                                               "(ILjava/lang/String;)V");
    char description[100];
    sprintf(description, "jpeg_message_table[%d]:%s", myerr->pub.msg_code,
            myerr->pub.jpeg_message_table[myerr->pub.msg_code]);
    if (callBack != NULL) {

        menv->CallVoidMethod(callBack, errorMthodId, INTERNAL_ERROR,
                             menv->NewStringUTF(description));
    }


    //跳转setjmp 并且返回值为1结束
    longjmp(myerr->setjmp_buffer, 1);


}

void freeResource() {
    menv->DeleteGlobalRef(callBack);
    callBack = NULL;
    menv = NULL;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_vst_libjpegdemo_NativeCompress_compress(JNIEnv *env, jobject instance, jstring outPath_,
                                                 jobject bitmap, jint ratio, jboolean isUseHoffman,
                                                 jobject nativeCallBack) {
    //文件输出地址
    const char *outPath = env->GetStringUTFChars(outPath_, 0);
    LOGI("outpath>>>%s", outPath);
    //用于保存bitmap的二进制数据
    BYTE *pixels;
    //保存回调地址为全局引用
    callBack = env->NewGlobalRef(nativeCallBack);
    menv = env;

    AndroidBitmapInfo info;
    memset(&info, 0, sizeof(info));
    AndroidBitmap_getInfo(env, bitmap, &info);
    int w = info.width;
    int h = info.height;
    LOGI("w=%d h=%d", w, h);
    //D:\workspace\wk01\jni\LibjpegDemo\app\src\main\java\com\vst\libjpegdemo
    jclass callBackClazz = env->FindClass("com/vst/libjpegdemo/NativeCallBack");

    //校验图片合法性
    if (w < 0 || h < 0) {
        jmethodID errorMethodID = env->GetMethodID(callBackClazz, "error",
                                                   "(ILjava/lang/String;)V");
        char des[100];
        sprintf(des, "bitmap高度或者宽度为0，具体数值为：w=%d,h=%d", w, h);
        if (callBack != NULL) {
            env->CallVoidMethod(callBack, errorMethodID, BITMAP_HEIGHT_WIDTH_ERROR,
                                env->NewStringUTF(des));
        }
        freeResource();
    }

    //校验图片格式
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        jmethodID errorMethodID = env->GetMethodID(callBackClazz, "error",
                                                   "(ILjava/lang/String;)V");
        if (callBack != NULL) {
            env->CallVoidMethod(callBack, errorMethodID, BITMAP_HEIGHT_WIDTH_ERROR,
                                env->NewStringUTF("图片格式错误"));
        }
        freeResource();
    }

    AndroidBitmap_lockPixels(env, bitmap, (void **) &pixels);
    BYTE *data;
    BYTE a, r, g, b;
    data = (BYTE *) malloc(w * h * 3);
    BYTE *tmpData;
    tmpData = data;
    LOGI("data>>>%d", *tmpData);
    for (int i = 0; i < w; ++i) {
        for (int j = 0; j < h; ++j) {
            int color = *((int *) pixels);

            //得到透明度
            //*a = ((color & 0xFF000000) >> 24);
            //红色值
            r = ((color & 0x00FF0000) >> 16);
            //绿色值
            g = ((color & 0x0000FF00) >> 8);
            //蓝色值
            b = ((color & 0x000000FF));

            *data = r;
            *(data + 1) = g;
            *(data + 2) = b;

            data += 3;
            pixels += 4;
        }
    }
    AndroidBitmap_unlockPixels(env, bitmap);

    LOGI("data>>>%d", *tmpData);
    char *tmpPath = (char *) malloc(sizeof(char) * (strlen(outPath) + 1));
    strcpy(tmpPath, outPath);
    LOGI("tmppath>>>%s", tmpPath);
    generateJPEG(tmpData, w, h, ratio, tmpPath, isUseHoffman);

    env->ReleaseStringUTFChars(outPath_, outPath);
}

int
generateJPEG(BYTE *data, int w, int h, int quality, const char *outfilename, jboolean optimize) {
    jclass callBackClazz = menv->FindClass("com/vst/libjpegdemo/NativeCallBack");
    struct jpeg_compress_struct jcs;

    struct my_error_mgr jem;
    jcs.err = jpeg_std_error(&jem.pub);
    jem.pub.error_exit = my_error_exit;

    //使用longjmp将跳转到这样
    if (setjmp(jem.setjmp_buffer)) {

        //关闭资源
        freeResource();
        return 0;
    }


    //初始化jsc结构体
    jpeg_create_compress(&jcs);
    LOGI("outfilename>>>%s", outfilename);
    //打开输出文件 wb:可写byte
    FILE *f = fopen(outfilename, "wb");
    if (f == NULL) {
//        LOGE("打开文件失败");
        jmethodID errorMthodId = menv->GetMethodID(callBackClazz, "error",
                                                   "(ILjava/lang/String;)V");
        char description[100];
        sprintf(description, "以二进制打开读写文件路径[%s]失败", outfilename);
        if (callBack != NULL) {

            menv->CallVoidMethod(callBack, errorMthodId, FILE_ERROR,
                                 menv->NewStringUTF(description));
        }
        //关闭资源
        freeResource();
        return 0;
    }

    //设置结构体的文件路径
    jpeg_stdio_dest(&jcs, f);
    jcs.image_width = w;//设置宽高
    jcs.image_height = h;

    //看源码注释，设置哈夫曼编码：/* TRUE=arithmetic coding, FALSE=Huffman */
    jcs.arith_code = false;
    int nComponent = 3;
    /* 颜色的组成 rgb，三个 # of color components in input image */
    jcs.input_components = nComponent;
    //设置结构体的颜色空间为rgb
    jcs.in_color_space = JCS_RGB;

    //全部设置默认参数/* Default parameter setup for compression */
    jpeg_set_defaults(&jcs);
    //是否采用哈弗曼表数据计算 品质相差5-10倍
    jcs.optimize_coding = optimize;
    //设置质量 quality是个0～100之间的整数，表示压缩比率
    jpeg_set_quality(&jcs, quality, true);
    //开始压缩，(是否写入全部像素)
    jpeg_start_compress(&jcs, TRUE);

    JSAMPROW row_pointer[1];
    int row_stride;
    //一行的rgb数量
    row_stride = jcs.image_width * nComponent;
    //一行一行遍历
    while (jcs.next_scanline < jcs.image_height) {
        //得到一行的首地址
        row_pointer[0] = &data[jcs.next_scanline * row_stride];

        //此方法会将jcs.next_scanline加1
        jpeg_write_scanlines(&jcs, row_pointer, 1);//row_pointer就是一行的首地址，1：写入的行数
    }
    jpeg_finish_compress(&jcs);//结束
    jpeg_destroy_compress(&jcs);//销毁 回收内存
    fclose(f);//关闭文件

    jmethodID pID = menv->GetMethodID(callBackClazz, "finish",
                                      "(Ljava/lang/String;)V");
    if (callBack != NULL) {

        menv->CallVoidMethod(callBack, pID, menv->NewStringUTF(outfilename));
    }
    //关闭资源
    freeResource();
//    LOGE("完成");
    return 1;
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_vst_libjpegdemo_NativeCompress_stringFromJNI(JNIEnv *env, jobject instance) {
//    JNIEnv* jniEnv = env;
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

