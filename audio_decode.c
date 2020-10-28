#include "audio_decode.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>

#define MAX_AUDIO_FRAME_SIZE 192000

static const char *audio_dst_filename = NULL;
static FILE * dest_audio_file = NULL;


static int get_format_from_sample_fmt(const char **fmt,
                                      enum AVSampleFormat sample_fmt)
{
    int i;
    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
        { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
        { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
        { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
        { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
        { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
    };
    *fmt = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt) {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    fprintf(stderr,
            "sample format %s is not supported as output format\n",
            av_get_sample_fmt_name(sample_fmt));
    return -1;
}

int initAudioDecoder(AudioDecoder *decoder, AVStream * avStream,
    void (*audio_callData)( AVFrame * avFrame)){

    //输出 视频文件
    audio_dst_filename = "./dest.pcm";
    dest_audio_file = fopen(audio_dst_filename, "wb");
    if (!dest_audio_file)
    {
        fprintf(stderr, "dest file error ! \n");
        return -1;
    }
    

    decoder->callback_func = audio_callData;
    decoder->avFrame = av_frame_alloc();
    decoder->codec = avcodec_find_decoder(avStream->codecpar->codec_id);

    if (!decoder->codec)
    {
        printf("audio codec alloc faild !\n");
        return -1;
    }

    decoder->codecCtx = avcodec_alloc_context3(decoder->codec);
    if (!decoder->codecCtx)
    {
        printf("codec context alloc faild ! \n");
        return -1;
    }
    int ret ;
    ret = avcodec_parameters_to_context(decoder->codecCtx, avStream->codecpar);
    if(ret < 0){
        printf("audio code params copy fail !");
        return -1;
    }


    ret = avcodec_open2(decoder->codecCtx, decoder->codec, NULL);
    if (ret < 0)
    {
        printf("audio codec open fail !");
        return -1;
    }

    //Out Audio Param
    int out_channel_layout = AV_CH_LAYOUT_STEREO;
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    int out_sample_rate = 44100;
    int out_nb_samples = decoder->codecCtx->frame_size;
    int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
    int out_buffer_size = av_samples_get_buffer_size(NULL,out_channels, out_nb_samples, out_sample_fmt, 1);
    decoder->out_buffer = av_malloc(MAX_AUDIO_FRAME_SIZE *2);


    //FIX : some codec`s context infomation is missing.
    int64_t in_channel_layout = av_get_default_channel_layout(decoder->codecCtx->channels);

    //swr 
    SwrContext* swr_ctx = swr_alloc();
    swr_ctx = swr_alloc_set_opts(swr_ctx, out_channel_layout, out_sample_fmt, out_sample_rate, 
        in_channel_layout, decoder->codecCtx->sample_fmt, decoder->codecCtx->sample_rate, 0, NULL);

    /**set options **/
    // av_opt_set_init(swr_ctx, "in_channel_layout", in_channel_layout, 0);
    // av_opt_set_init(swr_ctx, "in_sample_rate", decoder->codecCtx->sample_rate, 0);
    // av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", decoder->codecCtx->sample_fmt , 0);

    // /** set options **/
    // av_opt_set_init(swr_ctx, "out_channel_layout", out_channel_layout, 0);
    // av_opt_set_init(swr_ctx, "out_sample_rate", out_sample_fmt , 0);
    // av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", out_sample_rate, 0);

	ret = swr_init(swr_ctx);
    if (ret < 0)
    {
       fprintf(stderr, "error is  : %s", AVERROR(ret));
       return -1;
    }
    decoder->swrCtx = swr_ctx;
    

    enum AVSampleFormat sfmt = decoder->codecCtx->sample_fmt;        
    int n_channels = decoder->codecCtx->channels;
    const char *fmt ;

    if (av_sample_fmt_is_planar(sfmt)) {
        const char *packed = av_get_sample_fmt_name(sfmt);
        printf("Warning: the sample format the decoder produced is planar "
                "(%s). This example will output the first channel only.\n",
                packed ? packed : "?");
        sfmt = av_get_packed_sample_fmt(sfmt);
        n_channels = 1;
    }

    if ((ret = get_format_from_sample_fmt(&fmt, sfmt)) < 0){
        fprintf(stderr, "get_format_from_sample_fmt  error !! \n ");
        return -1;
    }


    printf("Play the output audio file with the command:\n"
        "ffplay -f %s -ac %d -ar %d %s\n",
               fmt, n_channels, decoder->codecCtx->sample_rate,
               audio_dst_filename);

    return 0;
}

void decode_audio(AudioDecoder *decoder, AVPacket * avPacket){
    int ret = 0;
    ret = avcodec_send_packet(decoder->codecCtx ,avPacket);
    if (ret < 0)
    {
        printf("Error sending a audio packet for decoding  code = %d \n", ret);
        return ;
    }

    AVFrame * avFrame;
    avFrame = decoder->avFrame;
    while (ret >=0)
    {
        ret = avcodec_receive_frame(decoder->codecCtx, avFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            // fprintf(stderr ,"receive audio  frame error ! %d \n", ret);
            break;            
        }

        int dataSize = av_get_bytes_per_sample(decoder->codecCtx->sample_fmt);
        if (dataSize < 0)
        {
            fprintf(stderr, "Failed to calculate audio data size\n");
            break;
        }
        // int ch = 0;
        // for (size_t i = 0; i < decoder->codecCtx->channels; i++)
        // {
        //     for (ch = 0; ch < decoder->codecCtx->channels; ch++)
        //     {
        //         // ret = swr_convert(decoder->swrCtx, &(decoder->out_buffer), MAX_AUDIO_FRAME_SIZE, 
        //         //     (const uint8_t **)avFrame->data, avFrame->nb_samples);        
        //         // fprintf(stderr ,"receive audio  frame success ! \n%s",decoder->out_buffer);

        //         int dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels,
        //                                          ret, dst_sample_fmt, 1);
        //         fwrite(decoder->out_buffer, 1, dataSize, dest_audio_file);

        //     }
        // }

        //通过回掉，把数据传回sdl 去播放
        
        if(decoder->callback_func){
            decoder->callback_func(avFrame);
        }
    

        /**
         * 解析音频数据并保存到本地pcm文件中
         * **/
        // size_t unpadded_linesize = avFrame->nb_samples * av_get_bytes_per_sample(avFrame->format);
        // fwrite(avFrame->extended_data[0], 1, unpadded_linesize, dest_audio_file);

    }

}
