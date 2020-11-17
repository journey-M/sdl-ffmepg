#include <pthread.h>
#include "queue.h"
#include <libswscale/swscale.h>

QueueAudio *maudioQueues; 
pthread_mutex_t audioMutex;

QueueVideo *mvideioQueues;
pthread_mutex_t videoMutext;


struct swsContext* swsContext = NULL;


void putAudioData(AudioData *audioData){
    pthread_mutex_lock(&audioMutex);

    if (audioData != NULL)
    {
        if (maudioQueues->size == 0)
        {
            /* code */
            maudioQueues->first = audioData;
            maudioQueues->last = audioData;
            maudioQueues->size = 1;
        }else
        {
            AudioData* last = maudioQueues->last;
            last->next = audioData;
            maudioQueues->last = audioData;
            maudioQueues->size = maudioQueues->size + 1;
        }

    }
    pthread_mutex_unlock(&audioMutex);
}

void putVideoData(AVFrame *frame){
    pthread_mutex_lock(&videoMutext);
    if (frame != NULL)
    {
        if (getVideoQueueSize() >= MAX_VIDEO_SIZE-1)
        {
            //TODO block  thread
            // mvideioQueues
        }else{

            if (!swsContext)
            {
                swsContext = sws_getContext(frame->width,  frame->height, frame->format, 
                            frame->width/4, frame->height/4, frame->format,
                            SWS_BILINEAR, NULL, NULL, NULL);
            }

            AVFrame *  copyFrame = mvideioQueues->datas[mvideioQueues->head];
            av_frame_unref(copyFrame);

            copyFrame->format = frame->format; 
            copyFrame->width = frame->width; 
            copyFrame->height = frame->height; 
            copyFrame->channels = frame->channels; 
            copyFrame->channel_layout = frame->channel_layout; 
            copyFrame->nb_samples = frame->nb_samples; 
            copyFrame->pts = frame->pts;
            copyFrame->pkt_dts = frame->pkt_dts;
            copyFrame->pkt_pos = frame->pkt_pos;
            // av_frame_get_buffer(copyFrame, 32); 
            // av_frame_copy_props(copyFrame, frame); 

            // av_frame_move_ref(copyFrame, frame);


            av_frame_get_buffer(copyFrame,32);
            // av_frame_copy(copyFrame, frame); 
            av_frame_copy_props(copyFrame, frame);

            int ret = sws_scale(swsContext , frame->data, frame->linesize, 0, frame->height, 
                    copyFrame->data, copyFrame->linesize);

            // fprintf(stderr, "sws_scale ret = %d \n", ret);

            int next =( mvideioQueues->head + 1) % MAX_VIDEO_SIZE; 
            mvideioQueues-> head = next;
        }
        
    }
    fprintf(stderr, "put  head: %d  tail: %d size %d  \n",mvideioQueues->head, mvideioQueues->tail, 
         getVideoQueueSize());
    pthread_mutex_unlock(&videoMutext);
}

AVFrame* popFirstVideo(){
    pthread_mutex_lock(&videoMutext);

    AVFrame * tmpData;
    if (getVideoQueueSize()== 0)
    {
        tmpData = NULL;
    }else
    {
        tmpData = mvideioQueues->datas[mvideioQueues->tail];
        int nextFrist =( mvideioQueues->tail + 1) % MAX_VIDEO_SIZE; 
        mvideioQueues->tail = nextFrist;
    }
    fprintf(stderr, "pop  head: %d  tail: %d size %d  \n",mvideioQueues->head, mvideioQueues->tail, 
         getVideoQueueSize());

    pthread_mutex_unlock(&videoMutext);

    return tmpData;
}

AudioData* popFirstAudio(){
    pthread_mutex_lock(&audioMutex);

    AudioData *tmpData = NULL;
    if (maudioQueues->size == 0)
    {
        /* code */
        tmpData = NULL;
        maudioQueues->first = NULL;
    }else
    {
        tmpData = maudioQueues->first;
        AudioData* next = tmpData->next;
        maudioQueues->first = next;
        maudioQueues->size = maudioQueues->size - 1;
    }
    pthread_mutex_unlock(&audioMutex);

    return tmpData;
}


int getVideoQueueSize(){
    int num = mvideioQueues->head - mvideioQueues->tail ;
    if (num < 0)
    {
        num = num + MAX_VIDEO_SIZE;
    }
    return num;
}

int getAudioQueueSize(){
    return maudioQueues->size;
}

QueueAudio* initQueueAuduio(){
    maudioQueues = malloc(sizeof(QueueAudio));
    maudioQueues->size = 0;
    maudioQueues->first = NULL;
    maudioQueues->last = NULL;
    pthread_mutex_init(&audioMutex, NULL);

}

QueueVideo* initVideoQueue(){
    mvideioQueues = malloc(sizeof(QueueVideo));
    mvideioQueues->head = 0;
    mvideioQueues->tail = 0;

    for (size_t i = 0; i < MAX_VIDEO_SIZE; i++)
    {
        mvideioQueues->datas[i] = av_frame_alloc();
    }
    pthread_mutex_init(&videoMutext, NULL);
}



void clearAudioCache(){
    pthread_mutex_lock(&audioMutex);
    while (maudioQueues->size >0)
    {
        AudioData* ret = maudioQueues->first;
        maudioQueues->first = ret->next;
        maudioQueues->size = maudioQueues->size -1;
        av_frame_unref(ret->frame);
        free(ret);
    }
    pthread_mutex_unlock(&audioMutex);
}

/**
 * 清空缓存中的视频数据
 */

void clearVideoCache(){
    pthread_mutex_lock(&videoMutext);

    mvideioQueues->head = 0;
    mvideioQueues->tail = 0;
    pthread_mutex_unlock(&videoMutext);
}