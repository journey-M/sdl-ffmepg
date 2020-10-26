#ifndef __threadwraper_h__

#define __threadwraper_h__

#include <libavformat/avformat.h>

#include "video_decode.h"

struct _ThreadWraper
{
    //回掉函数
    void (*fn) ();
    //发送数据
    void sendData();

};


typedef _ThreadWraper ThreadWraper;

#endif