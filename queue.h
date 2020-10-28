#ifndef __queue_h__

#define __queue_h__

#include <stdlib.h>
#include <unistd.h>
#include <libavformat/avformat.h>

typedef struct __AudioData
{
    /* data */
    AVFrame * frame;

    struct __AudioData *next;
} AudioData;


typedef struct __VideoData
{
    /* data */
    AVFrame * frame;

    struct __VideoData * next;
} VideoData;


typedef struct __queueAudio
{
    //回掉函数
    AudioData *first;           
    AudioData *last;
    int size;     //当前栈空间的大小
} QueueAudio;

typedef struct __queueVideo
{
    /* data */
    VideoData *first;           //栈顶指针
    VideoData *last;
    int size;     //当前栈空间的大小
} QueueVideo;


void putAudioData(AudioData *audioData);

void putVideoData(VideoData *videoData);

AudioData* popFirstAudio();

VideoData* popFirstVideo();

int getVideoQueueSize();

int getAudioQueueSize();

QueueAudio* initQueueAuduio();

QueueVideo* initVideoQueue();



extern QueueAudio *maudioQueues; 

extern QueueVideo *mvideioQueues;

#endif