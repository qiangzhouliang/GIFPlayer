#include <jni.h>
#include <string>
#include <malloc.h>
#include <cstring>
#include "gif_lib.h"

#include <android/log.h>
#include <android/bitmap.h>

#define LOG_TAG "swan"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define argb(a, r, g, b)(((a) & 0xff) << 24) | (((b) & 0xff) << 16) | (((g) & 0xff) << 8) | ((r) & 0xff)

typedef struct GifBean {
    // 播放帧数 第几帧
    int current_frame;
    int total_frame;
    // 延迟时间数组
    int *dealys;
}GifBean;

// 绘制一张图片
void drawFrame(GifFileType *gifFileType, GifBean *gifBean, AndroidBitmapInfo info, void *pixels) {
    // 获取当前帧
    SavedImage savedImage = gifFileType->SavedImages[gifBean->current_frame];
    // 整幅图片的首地址
    int *px = (int *)pixels;
    int pointPixels;
    GifImageDesc frameInfo = savedImage.ImageDesc;
    // 压缩数据
    GifByteType gifByteType;
    // 得到当前帧的字典 - 存放rgb数据 压缩工具
//    ColorMapObject* colorMapObject = frameInfo.ColorMap;
    ColorMapObject* colorMapObject = gifFileType->SColorMap;
    // bitmap 往下偏移
    px = (int *) ((char *)px + info.stride * frameInfo.Top);
    // 每一行的首地址
    int *line;

    for (int y = frameInfo.Top; y < frameInfo.Top + frameInfo.Height; ++y) {
        line = px;
        for (int x = frameInfo.Left; x < frameInfo.Left + frameInfo.Width; ++x) {
            // 拿到某一个坐标的位置 索引  ---》数据
            pointPixels = (y - frameInfo.Top) * frameInfo.Width + (x - frameInfo.Left);
            // 索引 -》 rgb lzw 压缩 ，缓存在字典中，
            // 解压
            gifByteType = savedImage.RasterBits[pointPixels];
            GifColorType gifColorType = colorMapObject->Colors[gifByteType];
            line[x] = argb(255, gifColorType.Red, gifColorType.Green, gifColorType.Blue);
        }
        px = (int *) ((char *)px + info.stride);
    }
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_swan_gifplayer_GifHandler_loadPath(JNIEnv *env, jobject thiz, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    int err;
    GifFileType *gifFileType = DGifOpenFileName(path, &err);
    DGifSlurp(gifFileType);

    // new GifBean
    GifBean *gifBean = (GifBean *)malloc(sizeof(GifBean));
    // 清空内存地址
    memset(gifBean, 0, sizeof(GifBean));
    gifFileType->UserData = gifBean;
    // 初始化延时数组
    gifBean->dealys = (int *) malloc(sizeof(int) * gifFileType->ImageCount);
    memset(gifBean->dealys, 0, sizeof(int) * gifFileType->ImageCount);
    // 延迟事件 读取
    // Delay Time - 单位 1/100秒，如果值不为1，表示暂停规定时间后再继续往下处理数据流
    // 获取时间
    gifFileType->UserData = gifBean;
    gifBean->current_frame = 0;
    gifBean->total_frame = gifFileType->ImageCount;
    ExtensionBlock* ext;
    for (int i = 0; i < gifFileType->ImageCount; ++i) {
        // 拿到当前帧
        SavedImage frame = gifFileType->SavedImages[i];
        // 遍历扩展块
        for (int j = 0; j < frame.ExtensionBlockCount; ++j) {
            // 判断是否是图像控制扩展块
            if (frame.ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE){
                ext = &frame.ExtensionBlocks[j];
                break;
            }
        }
        if (ext){
            int frame_delay = 10 * (ext->Bytes[1]|(ext->Bytes[2] << 8));
            LOGE("时间 %d   ", frame_delay);
            gifBean->dealys[i] = frame_delay;
        }
    }
    LOGE("gif 长度大小    %d  ", gifFileType->ImageCount);

    env->ReleaseStringUTFChars(path_, path);
    return reinterpret_cast<jlong>(gifFileType);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_swan_gifplayer_GifHandler_getWidth(JNIEnv *env, jobject thiz, jlong ndk_gif) {
    GifFileType *gifFileType = reinterpret_cast<GifFileType *>(ndk_gif);
    return gifFileType->SWidth;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_swan_gifplayer_GifHandler_getHeight(JNIEnv *env, jobject thiz, jlong ndk_gif) {
    GifFileType *gifFileType = reinterpret_cast<GifFileType *>(ndk_gif);
    return gifFileType->SHeight;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_swan_gifplayer_GifHandler_updateFrame(JNIEnv *env, jobject thiz, jlong ndk_gif,
                                               jobject bitmap) {
    GifFileType *gifFileType = (GifFileType *)ndk_gif;
    GifBean * gifBean = (GifBean *)gifFileType->UserData;
    AndroidBitmapInfo info;
    // 取址：表示入参出参对象
    AndroidBitmap_getInfo(env, bitmap, &info);
    // git - bitmap,锁住
    void *pixels;
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    // 绘制
    drawFrame(gifFileType, gifBean, info, pixels);
    gifBean->current_frame += 1;
    if (gifBean->current_frame >= gifBean->total_frame - 1){
        gifBean->current_frame = 0;
        LOGE("重新过来   %d   ", gifBean->current_frame);
    }

    AndroidBitmap_unlockPixels(env, bitmap);
    // 返回下一帧的延时时间
    return gifBean->dealys[gifBean->current_frame];

}