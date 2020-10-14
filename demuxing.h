#include <libavformat/avformat.h>

#include "video_decode.h"

void demuxing_main(char* filePath, void (*func)(uint8_t*, int , uint8_t*, int ,uint8_t*, int ));