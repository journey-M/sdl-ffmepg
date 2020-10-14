#include <libavformat/avformat.h>

#include "video_decode.h"

static AVPacket packet ;

void demuxing_main(char* filePath, void (*func)(uint8_t*, int , uint8_t*, int ,uint8_t*, int )){

    AVFormatContext *avInputFormatContext;
    avInputFormatContext = avformat_alloc_context();
    if (!avInputFormatContext)
    {
        printf("alloc avinputformat error! \n");
        return;
    }
    

    int ret;

    ret = avformat_open_input(&avInputFormatContext,filePath,NULL, NULL);
    if (ret<0)
    {
        printf("open file error ! \n");
        return;
    }

    ret = avformat_find_stream_info(avInputFormatContext, NULL);
    if (ret !=0)
    {
        printf("avformat_find_stream_info  faild! \n");
        return;
    }
    


    int videoIndex, audioIndex;
    
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;


    videoIndex = av_find_best_stream(avInputFormatContext,AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoIndex < 0)
    {
        printf("no video stream ! \n");
        return;
    }

    audioIndex = av_find_best_stream(avInputFormatContext,AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audioIndex < 0)
    {
        printf("no audio stream ! \n");
        return;
    }

    //创建视频解码器
    VideoDecoder * videoDecoder = malloc(sizeof(VideoDecoder));
    ret = initVideoDecoder(videoDecoder, avInputFormatContext->streams[videoIndex], func);
    if (ret !=0)
    {
        printf("VideoDecoder init false ! \n");
        return;
    }
    


    int packetNum = 0;

    for (;;)
    {
        ret = av_read_frame(avInputFormatContext, &packet);
        packetNum ++ ;

        printf("recieved packet Num :  %d \n ", packetNum);
        if (ret != 0)
        {
            printf("av_read_fram to the end \n");
            break;
        }
    
        if (packet.stream_index == videoIndex)
        {
            //start decode video
            decode_video(videoDecoder, &packet);
        }else if (packet.stream_index == audioIndex)
        {
            /* code */
        }else
        {
            /* code */
            fprintf(stderr, "not video strem and video stream ! \n");
        }

        av_packet_unref(&packet);
    }
}