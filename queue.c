#include <pthread.h>
#include "queue.h"

QueueAudio *maudioQueues; 
pthread_mutex_t audioMutex;

QueueVideo *mvideioQueues;
pthread_mutex_t videoMutext;

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

void putVideoData(VideoData *videoData){
    pthread_mutex_lock(&videoMutext);
    if (videoData != NULL)
    {
        if (mvideioQueues->size == 0)
        {
            /* code */
            mvideioQueues->first = videoData;
            mvideioQueues->last = videoData;
            mvideioQueues->size = 1;
        }else
        {
            VideoData * last = mvideioQueues->last;
            last->next = videoData;
            mvideioQueues->last = videoData;
            mvideioQueues->size = mvideioQueues->size + 1;
        }
    }
    pthread_mutex_unlock(&videoMutext);
}

AudioData* popFirstAudio(){
    pthread_mutex_lock(&audioMutex);

    AudioData *tmpData = NULL;
    if (maudioQueues->size == 0)
    {
        /* code */
        tmpData = NULL;
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

VideoData* popFirstVideo(){
    pthread_mutex_lock(&videoMutext);

    VideoData * tmpData;
    if (mvideioQueues->size == 0)
    {
        /* code */
        tmpData = NULL;
    }else
    {
        tmpData = mvideioQueues->first;
        if (tmpData)
        {
            VideoData* next = tmpData->next;
            mvideioQueues->first = next;
            mvideioQueues->size = mvideioQueues->size - 1;
        }

    }
    pthread_mutex_unlock(&videoMutext);

    return tmpData;
}

int getVideoQueueSize(){
    return mvideioQueues->size;
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
    mvideioQueues->size = 0;
    mvideioQueues->first = NULL;
    mvideioQueues->last = NULL;
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

    while (mvideioQueues->size >0)
    {
        VideoData* ret = mvideioQueues->first;
        if (ret)
        {
            mvideioQueues->first = ret->next;
            mvideioQueues->size = mvideioQueues->size -1;
            av_frame_unref(ret->frame);
            free(ret);
        }
        

    }
    pthread_mutex_unlock(&videoMutext);
}