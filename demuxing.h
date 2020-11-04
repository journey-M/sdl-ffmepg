#ifndef __demuxing_h__

#define __demuxing_h__

#include <libavformat/avformat.h>

#include "video_decode.h"
/**
 * 
 * **/
void demuxing_main(char* filePath, 
    void (*func)(uint8_t*, int , uint8_t*, int ,uint8_t*, int ),
    void (*audio_config)(),
    void (*audio_callback)(uint8_t * pcmBufferData, int len),
    void (*playover)());


void closeDemuxing();

#endif