#ifndef __video_decode_h__

#define __video_decode_h__

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

struct __VideoDecode
{
    /* data */
    AVCodecContext  *codecCtx;
    AVCodec *codec;
    AVFrame *avFrame;
    int initd;
    void (*callback_func)(uint8_t*, int , uint8_t*, int ,uint8_t*, int );
};

typedef struct __VideoDecode VideoDecoder ;

int initVideoDecoder(VideoDecoder *decoder, AVStream * avStream,void (*func)(uint8_t*, int , uint8_t*, int ,uint8_t*, int ));

void decode_video(VideoDecoder *decoder, AVPacket * avPacket);

 #endif //__video_decode_h__