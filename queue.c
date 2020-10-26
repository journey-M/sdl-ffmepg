#include "queue.h"

QueueAudio *maudioQueues; 

QueueVideo *mvideioQueues;

void putAudioData(AudioData *audioData){
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

}

void putVideoData(VideoData *videoData){
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
            mvideioQueues->size = maudioQueues->size + 1;
        }
    }
}

AudioData* popFirstAudio(){
    if (maudioQueues->size == 0)
    {
        /* code */
        return NULL;
    }else
    {
        AudioData* ret = maudioQueues->first;
        AudioData* next = ret->next;
        maudioQueues->first = next;
        maudioQueues->size = maudioQueues->size - 1;
        return ret;
    }
}

VideoData* popFirstVideo(){
    if (mvideioQueues->size == 0)
    {
        /* code */
        return NULL;
    }else
    {
        VideoData* ret = mvideioQueues->first;
        VideoData* next = ret->next;
        mvideioQueues->first = next;
        mvideioQueues->size = mvideioQueues->size - 1;
        return ret;
    }
}

int getVideoQueueSize(){
    return mvideioQueues->size;
}

int getAudioQueueSize(){
    return maudioQueues->size;
}

QueueAudio* initQueueAuduio(){
    maudioQueues = malloc(sizeof(QueueAudio));

}

QueueVideo* initVideoQueue(){
    mvideioQueues = malloc(sizeof(QueueVideo));
}