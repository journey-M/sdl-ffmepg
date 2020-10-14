#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include "video_decode.h"



/**
 * 初始化
 * 
 * return 0 is success
 **/
int initVideoDecoder(VideoDecoder *decoder, AVStream * avStream, 
    void (*func)(uint8_t*, int , uint8_t*, int ,uint8_t*, int )){

    decoder->callback_func = func;        
    decoder->avFrame = av_frame_alloc();
    decoder->codec = avcodec_find_decoder(avStream->codecpar->codec_id);
    if (!decoder->codec)
    {
        printf("Failed to find video codec \n");
        return -1;
    }
    decoder->codecCtx = avcodec_alloc_context3(decoder->codec);
    if (!decoder->codecCtx)
    {
        printf("can not alloc decodeContext \n");
        return -1;

    }

    //copy codec parameters into code
    int ret;
    ret = avcodec_parameters_to_context(decoder->codecCtx, avStream->codecpar);
    if (ret < 0)
    {
        printf("can not converr video codec parameters to context \n");
        return -1;

    }

    printf("video width : %d \nvideo height : %d \n", decoder->codecCtx->width, decoder->codecCtx->height);

    ret = avcodec_open2(decoder->codecCtx, decoder->codec, NULL);
    if (ret <0)
    {
        printf("avcodec open video stream error\n");
        return -1;

    }
    return 0;
}


// void saveFrame2Image(AVFrame *fram, AVCodecContext* codecCtx){
//     struct SwsContext *sws_ctx = sws_getContext(codecCtx->width, codecCtx->height, codecCtx->pix_fmt,
//     codecCtx->width ,codecCtx->height, AV_PIX_FMT_YUVJ420P,SWS_BICUBIC ,NULL , NULL,NULL);

//     if (!sws_ctx)
//     {
//         fprintf(stderr, "swc_ctx create error ! \n");
//         return ;
//     }

//     //allocate image  space
//     // int numBytes = avpicture_get_size(AV_PIX_FMT_YUVJ420P, codecCtx->width, codecCtx->height);
//     // unsigned char *out_buffer;
//     // out_buffer = av_malloc(numBytes * sizeof(unsigned char));
// }

// static int indexName = 0;
// void saveAsJPEG(AVFrame * avFrame, int width ,int height){
//     indexName ++;
//     char fileName[20] ;
//     sprintf(fileName, "image_%d.png",indexName);
// }

void decode_video(VideoDecoder *decoder, AVPacket *avPacket){
    
    int ret =0;
    ret = avcodec_send_packet(decoder->codecCtx, avPacket);
    if(ret < 0){
        printf("Error sending a packet for decoding  code = %d \n", ret);
        return ;
    }

    AVFrame * avFrame = decoder->avFrame;
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(decoder->codecCtx, avFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            break;            
        }
        if (decoder->callback_func)
        {
            /* code */
            decoder->callback_func((uint8_t*)avFrame->data[0], avFrame->linesize[0],
                    (uint8_t*)avFrame->data[1], avFrame->linesize[1],
                    (uint8_t*)avFrame->data[2], avFrame->linesize[2]);
        }
        
        
    }

}