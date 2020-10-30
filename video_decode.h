#ifndef __video_decode_h__

#define __video_decode_h__

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

struct __VideoDecode
{
    /* data */
    AVCodecContext  *codecCtx;
    AVCodec *codec;
    AVFrame *avFrame;
    struct SwsContext *swsContext;
    int initd;
    void (*callback_func)(AVFrame *avFrame );
};

typedef struct __VideoDecode VideoDecoder ;

int initVideoDecoder(VideoDecoder *decoder, AVStream * avStream,void (*func)(AVFrame * gotFrame ));

void decode_video(VideoDecoder *decoder, AVPacket * avPacket);

 #endif //__video_decode_h__