#ifndef __audio_decode_h__

#define __audio_decode_h__

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>

struct __AudioDecode
{
    /* data */
    AVCodecContext  *codecCtx;
    AVCodec *codec;
    AVFrame *avFrame;
    int initd;
    SwrContext *swrCtx;
    uint8_t *out_buffer;
    void (*callback_func)(uint8_t*, int );
};

typedef struct __AudioDecode AudioDecoder ;

struct __AudoParams
{
    
};

typedef struct __AudoParams AudioParmas;


int initAudioDecoder(AudioDecoder *decoder, AVStream * avStream,void (*func)(uint8_t*, int ));

void decode_audio(AudioDecoder *decoder, AVPacket * avPacket);

 #endif //__audio_decode_h__