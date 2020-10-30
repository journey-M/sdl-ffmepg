#include <libavformat/avformat.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#include "video_decode.h"
#include "audio_decode.h"
#include "audio_decode.h"
#include "queue.h"

static AVPacket packet ;
static pthread_t videoThread, audioThread;
static AVRational time_base;
static AVRational time_base_audio;
static long lastTime;

/**
 * 视频播放剩余的进度
 */

static double audio_frame_time ;

static void (*host_playvideo)(uint8_t*, int , uint8_t*, int ,uint8_t*, int );

static void (*host_playaudio)(uint8_t * , int);

/** 解码视频的线程
 */
static void videoDecoderTh(){
    while (1)
    {
        /* code */

        if (getVideoQueueSize()>0)
        {
            /* code */
            VideoData* vData = popFirstVideo();
            if (vData != NULL)
            {
                //当前帧应该在的时间
                long  vtime = vData->frame->pts * av_q2d(time_base) * 1000000;

                int delt = vtime - audio_frame_time;
                if (delt > 20)
                {
                    fprintf(stderr, "sleeping ....\nvtime: %f, \natime: %f   delt = %d  \n",vtime, audio_frame_time,delt);
                    av_usleep(delt);
                }

                host_playvideo(vData->frame->data[0], vData->frame->linesize[0], 
                    vData->frame->data[1], vData->frame->linesize[1], 
                    vData->frame->data[2], vData->frame->linesize[2]);
                av_frame_unref(vData->frame);
                free(vData);
            }

        }        
    }
}



/** 解码音频的线程
 */
static void audioDecoderTh(){
    while (1)
    {
        /* code */

        if (getAudioQueueSize()> 0)
        {
            AudioData* aData = popFirstAudio();
            if (aData != NULL)
            {
                AVFrame * frame = aData->frame;
                audio_frame_time = frame->pts * av_q2d(time_base_audio) * 1000000;


                size_t size = frame->nb_samples * av_get_bytes_per_sample(frame->format) * frame->channels;
                host_playaudio(frame->data[0], size);
                av_frame_unref(frame);
                free(aData);
            }
 
        }
    }
    
}


static void maudio_palydata(AVFrame * avFrame){

    // fprintf(stderr, "video - frame  pts : %d  \n" + avFrame->pts);
    //应该把数据放到队列里去，  然后在队列里面去解码，读值
    AVFrame * audioFrame = av_frame_alloc();
    av_frame_move_ref(audioFrame, avFrame);
    AudioData* audioData = malloc(sizeof(AudioData));

    audioData->frame = audioFrame;
    audioData->next = NULL;

    putAudioData(audioData);

    // host_playaudio(data, size);
}

static void mvideo_playdata(AVFrame *avFrame){
    // fprintf(stderr, "video - frame  pts : %d  \n" + avFrame->pts);

    // avFrame->opaque

    int ret = 0;

    AVFrame *copyFrame;
    copyFrame = av_frame_alloc();
    // copyFrame->format = avFrame->format;
    // copyFrame->width = avFrame->width;
    // copyFrame->height = avFrame->height;
    // copyFrame->channels = avFrame->channels;
    // copyFrame->channel_layout = avFrame->channel_layout;
    // copyFrame->nb_samples = avFrame->nb_samples;
    // av_frame_get_buffer(copyFrame, 32);
    // ret = av_frame_copy(copyFrame, avFrame);
    // av_frame_copy_props(copyFrame, avFrame);

    av_frame_move_ref(copyFrame, avFrame);

    if (ret<0)
    {
        fprintf(stderr, "avframe copy error ! \n");
        return ;
    }
    
    //视频数据放入单独的队列中
    VideoData* vidoeData = malloc(sizeof(VideoData));
    vidoeData->frame = copyFrame;
    vidoeData->next = NULL;

    putVideoData(vidoeData);

    //直接返回播放
    // host_playvideo(yplan, ypitch,uplan,upitch, vplan,vpi                currentAudioTime();                currentAudioTime();tch );   
}


void demuxing_main(char* filePath, 
    void (*video_playdata)(uint8_t*, int , uint8_t*, int ,uint8_t*, int ),
    void (*audio_config)(),
    void (*audio_palydata)(uint8_t * , int)){

    host_playvideo = video_playdata;
    host_playaudio = audio_palydata;

    AVFormatContext *avInputFormatContext;
    avInputFormatContext = avformat_alloc_context();
    if (!avInputFormatContext)
    {
        printf("alloc avinputformat error! \n");
        return;
    }

    //初始化视频和音频流
    initQueueAuduio();
    initVideoQueue();

    int ret;
    ret = pthread_create(&videoThread, NULL,videoDecoderTh, NULL);
    if (ret  < 0)
    {
        fprintf(stderr, "创建视频线程失败！");
        return ;
    }

    ret = pthread_create(&audioThread, NULL,audioDecoderTh, NULL);
    if (ret  < 0)
    {
        fprintf(stderr, "创建音频线程失败！");
        return ;
    }
    


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
    ret = initVideoDecoder(videoDecoder, avInputFormatContext->streams[videoIndex], mvideo_playdata);
    AVStream * videoStream = avInputFormatContext->streams[videoIndex];
    fprintf(stderr, "duration %2d \n", videoStream -> duration );
    fprintf(stderr, "time base : %f \n", av_q2d(videoStream -> time_base) );
    fprintf(stderr, "avg frame rate : %f \n", av_q2d(videoStream -> r_frame_rate) );
    fprintf(stderr, "avg frame rate : %f \n", av_q2d(videoStream -> r_frame_rate) );
    time_base = videoStream->time_base;

    if (ret !=0)
    {
        printf("VideoDecoder init false ! \n");
        return;
    }
    
    //创建音频解码器
    AudioDecoder * audioDecoder = malloc(sizeof(AudioDecoder));
    ret = initAudioDecoder(audioDecoder, avInputFormatContext->streams[audioIndex], maudio_palydata);
    time_base_audio = avInputFormatContext->streams[audioIndex]->time_base;
    audio_config();
    if (ret != 0)
    {
        printf("AudioDecoder init false ! \n");
        return;
    }
    

    int packetNum = 0;

    for (;;)
    {
        ret = av_read_frame(avInputFormatContext, &packet);
        packetNum ++ ;

        if (ret != 0)
        {
            printf("av_read_fram to the end \n");
            break;
        }
    
        /**  TODO 
         *  
         *  音频和视频需要单独解码，放在同一个线程中会导致 
         *  同一时刻只能播放音频或者视频
         *  next:
         *  1.视频的单独解码
         *  2.音频的单独解码
         */

        if (packet.stream_index == videoIndex)
        {
            //start decode video
            decode_video(videoDecoder, &packet);
        }else if (packet.stream_index == audioIndex)
        {
            //start deoce audio 
            decode_audio(audioDecoder, &packet);
        }else
        {
            /* code */
            fprintf(stderr, "not video strem and video stream ! \n");
        }

        av_packet_unref(&packet);
    }
}