#ifndef __queue_h__

#define __queue_h__

#include <stdlib.h>
#include <unistd.h>
#include <libavformat/avformat.h>

#define MAX_VIDEO_SIZE 10
#define MIN_VIDEO_SIZE 2


typedef struct __AudioData
{
    /* data */
    AVFrame * frame;

    struct __AudioData *next;
} AudioData;


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
    AVFrame *datas[MAX_VIDEO_SIZE];           //栈顶指针
    int head;
    int tail;
} QueueVideo;


void putAudioData(AudioData *audioData);

void putVideoData(AVFrame *videoData);

AudioData* popFirstAudio();

AVFrame* popFirstVideo();

int getVideoQueueSize();

int getAudioQueueSize();

QueueAudio* initQueueAuduio();

QueueVideo* initVideoQueue();

void clearAudioCache();

void clearVideoCache();

extern QueueAudio *maudioQueues; 

extern QueueVideo *mvideioQueues;

#endif