#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <libavcodec/avcodec.h>
#include <stdio.h>
#include <stdint.h>
#include "decVideo.h"

static int width = 1000;
static int height = 650;
static SDL_Renderer * render;
Uint32 audio_len;
Uint8 *audio_pos;
int out_buffer_size ;
uint8_t *audio_chunk;
int out_nb_samples = 1024;
int out_sample_rate = 44100;


/** audio  **/
void read_audio_data(void *udata, Uint8 *stream, int len){
	SDL_memset(stream, 0, len);
	if(audio_len == 0){
		return ;
	}	
	len = (len > audio_len ? audio_len :len);

	SDL_MixAudio(stream, audio_pos, len ,SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
}


void m_render(uint8_t* yplan, int ypitch, uint8_t* uplan, int upitch,uint8_t* vplan, int vpitch){
	printf("\n this is in m_render \n");
	//rendder video 
	SDL_Texture *yuvTexture = SDL_CreateTexture(render,SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,width, height);
	SDL_UpdateYUVTexture(yuvTexture,NULL,yplan , ypitch, uplan, upitch, vplan ,vpitch );
	SDL_RenderClear(render);
	SDL_RenderCopy(render, yuvTexture, NULL, NULL);
	SDL_RenderPresent(render);
}

void set_audio_params(int out_channels, AVCodecContext* audo_codec_ctx){

	SDL_AudioSpec spec;
	spec.freq = out_sample_rate;
	spec.format = AUDIO_S16SYS;
	spec.channels = out_channels;
	spec.silence = 0;
	spec.samples = out_nb_samples;
	spec.callback = read_audio_data;  
	spec.userdata = audo_codec_ctx; 

	if((SDL_OpenAudio(&spec, NULL)<0)){
		printf("can`t open audio.  \n");
		return ;
	}
	SDL_PauseAudio(0);
}

int main(int argc , char* argv[]){

	SDL_Window *window = NULL;
	SDL_Surface * hello = NULL;
	SDL_Surface *screen = NULL;

	if(SDL_Init(SDL_INIT_VIDEO)<0){
		printf("erro for init  \n");
		return 0;
	}


	window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height, SDL_WINDOW_SHOWN);

	if(window == NULL){
		printf("window init err %s\n", SDL_GetError());
		return 0;
	}
	// screen = SDL_GetWindowSurface(window);
	// SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF));
	// SDL_UpdateWindowSurface(window);

	render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	
	hello = IMG_Load("res/abc.png"); 
	if(hello == NULL){
		printf("hello 图片不存在 \n");
		return 0;
	}
	SDL_Rect box = {0,0,hello->w , 400};
	SDL_Texture *texture = SDL_CreateTextureFromSurface(render ,hello);
	SDL_RenderCopy(render, texture, NULL, &box);
	//显示出来
	SDL_RenderPresent(render);


	dff_main(argv[1], &m_render, &set_audio_params);


//	SDL_Delay(5000);

	SDL_FreeSurface(hello);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
