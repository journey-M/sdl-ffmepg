#include <libavcodec/avcodec.h>
#include <libavformat/avio.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavutil/pixfmt.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <stdint.h>
#include<stdio.h>
#include<libavformat/avformat.h>
#include <string.h>
#include "constants.h"

static AVFrame *fram;
static AVPacket pkt;
static int video_st_index;
static int audio_st_index;
static AVCodecContext *video_dec_ctx;
static AVCodecContext *audio_dec_ctx;
static struct SwsContext *img_convert_ctx;
static struct SwrContext *au_convert_ctx;
static AVFrame* frameYUV;
static uint8_t * out_buffer;
static uint8_t * out_buffer_audio;
static int video_fram_count;
static int audio_fram_count;
static int width, height;
static enum AVPixelFormat pix_fmt;


void (*func_callback)(uint8_t* , int ,uint8_t*, int ,uint8_t*, int ) ;
void (*func_setupAudio)(int out_channels, AVCodecContext* audo_codec_ctx);

#define MAX_AUDIO_FRAME_SIZE 19200

struct buffer_data{
	uint8_t *ptr;
	size_t size;
};

static int read_packet(void* opaque, uint8_t *buf, int buf_size){
	struct buffer_data *bd = opaque;
	buf_size = FFMIN(buf_size, bd->size);
	//copy internal buffer data to buf.
	memcpy(buf ,bd->ptr, buf_size);
	bd->ptr += buf_size;
	bd->size -= buf_size;
	return buf_size;
}

static int open_codec_context(int *stream_idx, AVCodecContext **dec_c, AVFormatContext *fmt_ctx,
		enum AVMediaType type){
	int ret, stream_index;
	AVStream *st;
	AVCodec *dec;
	
	ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
	if(ret <0){
		fprintf(stderr, "could not find best stream in input file");
		return -1;
	}
	stream_index = ret;
	st = fmt_ctx -> streams[stream_index];
	
	/** find decoder for stream **/
	dec = avcodec_find_decoder(st->codecpar->codec_id);
	if(!dec){
		fprintf(stderr, "Failed to find codec_id \n");
		return -1;
	}

	/* Allocate a codec context for the decoder */	
	*dec_c = avcodec_alloc_context3(dec);
	if( !*dec_c){
		fprintf(stderr, "could not allocate dec context !");
		return -1;
	}
	/* copyt params from input strem to output codec context */
	if ((ret = avcodec_parameters_to_context(*dec_c, st->codecpar)) <0){
		fprintf(stderr, "Faild to copy codec params to decoder context ! \n");
		return ret;
	};

	/** init decoders */
	if((ret = avcodec_open2(*dec_c, dec, NULL ))<0){
		fprintf(stderr, "Faild to open codec !");
		return -1;
	}

	*stream_idx = stream_index;
	return 0;
}

static int decode_packet(int *got_frame, int cached){
	int ret =0 ;
	int decoded = pkt.size;

	*got_frame = 0;

	if(pkt.stream_index == video_st_index){
		ret = avcodec_decode_video2(video_dec_ctx, fram, got_frame, &pkt);

		if(ret <0){
			fprintf(stderr, "Error decoding video frame \n");
			return ret;
		}
		if(*got_frame){
			if(fram ->width != width || fram ->height != height || fram->format != pix_fmt){
				fprintf(stderr, "Error: width heigth pix_fmt change");
				return -1;
			}
			printf("video_frame %s n:%d coded_n: %d\n frame type : %d",
					cached? "(cached)":"",
					video_fram_count++,
					fram->coded_picture_number,
					fram->format);
			//av_image
			//av_image_alloc();
			//av_image_copy();

			if(func_callback){
				sws_scale(img_convert_ctx, fram->data, fram->linesize, 0, height, frameYUV->data, frameYUV->linesize);
				func_callback(frameYUV->data[0], frameYUV->linesize[0],
						frameYUV->data[1], frameYUV->linesize[1],
						frameYUV->data[2], frameYUV->linesize[2]);
			}

		}
	}else if(pkt.stream_index == audio_st_index){
		/** 解码音频数据 */
		ret = avcodec_decode_audio4(audio_dec_ctx, fram, got_frame, &pkt);
		if(ret <0){
			fprintf(stderr, "Failed: decoding audio frame ..");
			return -1;
		}
		decoded = FFMIN(ret, pkt.size);
		if(*got_frame){
			size_t unpadded_linesize = fram->nb_samples * av_get_bytes_per_sample(fram->format);	
			printf("audio_fram %s n: %d nb_samples: %d pts: \n",
					cached? "cached": "",
					audio_fram_count++, fram->nb_samples	);
			//multiple definition of 
			ret = swr_convert(au_convert_ctx,&out_buffer_audio, MAX_AUDIO_FRAME_SIZE, fram->data , fram->nb_samples);  
			if(ret < 0){
				fprintf(stderr, "err convert swr %d", ret);
			}

			audio_chunk = out_buffer;
			audio_len = out_buffer_size;
		  audio_pos = audio_chunk;


		}

	}
	if(*got_frame ){
		av_frame_unref(fram);
	}

	return decoded;
}

int dff_main(char* fileName , 
		void (*func)(uint8_t*, int , uint8_t*, int ,uint8_t*, int ),
		void (*func_setup)(void *udata, uint8_t  *stream, int len)){
	func_callback = func;
	func_setupAudio = func_setup; 
	int ret ;
	AVFormatContext *fmt_ctx = NULL;
	AVIOContext *avio_ctx = NULL;
	uint8_t *buffer=NULL, *avio_ctx_buffer = NULL;
	size_t buffer_size, avio_ctx_buffer_size = 4096; 
	struct buffer_data bd = {0};
	
	ret = av_file_map(fileName, &buffer, &buffer_size, 0, NULL);
	if(ret <0){
		printf("av_file_map err!");
		return -1;
	}
	bd.ptr = buffer;
	bd.size = buffer_size;
	
	//初始化avioctx:从读取的文件中
	avio_ctx_buffer = av_malloc(avio_ctx_buffer_size);
	if(!avio_ctx_buffer){
		printf("error to malloc avio_ctx_buffer ");
		return -1;
	}

	avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0, &bd, &read_packet, NULL, NULL);
	if(!avio_ctx){
		printf("this is a err ot avio_ctx !");
		return -1;
	}
	
	if(!(fmt_ctx = avformat_alloc_context())){
		printf("avformat_alloc_err !");
		return -1;	
	}
	fmt_ctx->pb = avio_ctx;
	ret = avformat_open_input(&fmt_ctx,NULL, NULL, NULL);
	if(ret<0){
		fprintf(stderr, "could not open input !");
		return -1;
	}
	ret = avformat_find_stream_info(fmt_ctx, NULL);
	if(ret < 0){
		fprintf(stderr, "could not find stream information \n");
		return  -1;
	}

	/* decode  video  data */
	AVStream* video_stream;

	if((open_codec_context(&video_st_index, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO))>=0){
		video_stream = fmt_ctx ->streams[video_st_index];
		/* allocate image where the docode image will be put */
		width = video_dec_ctx->width;
		height = video_dec_ctx->height;
		pix_fmt = video_dec_ctx->pix_fmt;

		img_convert_ctx = sws_getContext(width, height,pix_fmt, 
				width/4, height/4, AV_PIX_FMT_YUV420P,
				SWS_BICUBIC, NULL, NULL, NULL);
		frameYUV = av_frame_alloc();
		out_buffer = av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, width, height));
		avpicture_fill((AVPicture *)frameYUV, out_buffer, AV_PIX_FMT_YUV420P, width, height);

		//av_image_alloc();	
	}
	/* audio stream */
	AVStream* audio_stream;
	AVCodecParameters *audio_codecParams = NULL;
	if(open_codec_context(&audio_st_index,  &audio_dec_ctx, fmt_ctx, AVMEDIA_TYPE_AUDIO) >=0){
		audio_stream = fmt_ctx ->streams[audio_st_index];
		
		audio_codecParams = audio_stream->codecpar;

		/**音频相关 **/
		uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO; //输出声道
		enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16P;
		int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
		out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples,out_sample_fmt, 1);
		out_buffer_audio = av_malloc(MAX_AUDIO_FRAME_SIZE *2 );
	
		func_setupAudio(out_channels, audio_dec_ctx);

		
		uint64_t in_channel_layout ; 
		in_channel_layout = av_get_default_channel_layout(audio_dec_ctx->channel_layout);
		au_convert_ctx = swr_alloc();
		au_convert_ctx = swr_alloc_set_opts(au_convert_ctx,
				out_channel_layout, out_sample_fmt, out_sample_rate, 
				in_channel_layout, audio_dec_ctx->sample_fmt, audio_dec_ctx->sample_rate, 0, NULL  );
		swr_init(au_convert_ctx);
		
	}


	if(!video_stream){
		fprintf(stderr, "no video stream !");
		return -1;
	}
	int got_fram;
	fram = av_frame_alloc();
	
	/** initialize packet, set data to NULL, **/
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;

	while(av_read_frame(fmt_ctx, &pkt)>=0){
		AVPacket orig_pkt = pkt;
		do {
			ret = decode_packet(&got_fram, 0);
			if(ret <0){
				break;
			}
			pkt.data +=ret;
			pkt.size -= ret;
		}while(pkt.size >0);
		
		av_packet_unref(&orig_pkt);
	}

	/* flush cached frames */
	pkt.data = NULL;
	pkt.size = 0;
	do{
		decode_packet(&got_fram, 1);
	}while(got_fram);


	av_dump_format(fmt_ctx, 0, fileName, 0);
	return 0;	
}
