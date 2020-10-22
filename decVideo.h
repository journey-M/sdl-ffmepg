#ifndef __DEC_VIDEO_H__
#define __DEC_VIDEO_H__

#include<stdint.h>
#include<libavcodec/avcodec.h>
int dff_main(char* fileName,
	 	void * func(uint8_t* yplan, int ypitch, uint8_t* uplan, int upitch,const uint8_t* vplan, int vpitch),
		void 	*audioInit(int out_channels, AVCodecContext* audo_codec_ctx ));

#endif
