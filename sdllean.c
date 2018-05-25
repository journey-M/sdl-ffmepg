#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

int main(int argc , char* argv[]){



	SDL_Window *window = NULL;
	SDL_Surface * hello = NULL;
	SDL_Surface *screen = NULL;

	if(SDL_Init(SDL_INIT_VIDEO)<0){
		printf("erro for init  \n");
		return 0;
	}
	window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		400, 400, SDL_WINDOW_SHOWN);

	if(window == NULL){
		printf("window init err %s\n", SDL_GetError());
		return 0;
	}
	// screen = SDL_GetWindowSurface(window);
	// SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF));
	// SDL_UpdateWindowSurface(window);

	SDL_Renderer * render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

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


	SDL_Delay(10000);

	SDL_FreeSurface(hello);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
