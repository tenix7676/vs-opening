#include <stdio.h>
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

int main(int argc, char** argv)
{
    SDL_Window* win;
    SDL_Renderer* ren;
    SDL_CreateWindowAndRenderer("vs-opening",690, 420, SDL_WINDOW_BORDERLESS,&win,&ren);
    int width, height;
    SDL_GetWindowSize(win, &width, &height);
    Uint64 sum=0;
    int it=1;
    SDL_Texture* screen_text=SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    unsigned char* pixels = malloc(4*width*height*sizeof(char));
    

    while(true)
    {
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_EVENT_QUIT)
                return 0;
        }
        SDL_SetRenderDrawColorFloat(ren,0,0,0,1);
        SDL_RenderClear(ren);
        Uint64 start=SDL_GetTicksNS();
        for(int i=0; i < height; ++i)
            for(int j=0; j < width; ++j)
            {
                pixels[i*width*4+4*j+0]=255;
                pixels[i*width*4+4*j+1]=255;
                pixels[i*width*4+4*j+2]=255;
                pixels[i*width*4+4*j+3]=255;
            }
        SDL_UpdateTexture(screen_text, NULL, pixels, 4*width);
        SDL_RenderTexture(ren, screen_text, NULL, NULL);
        Uint64 end=SDL_GetTicksNS();
        sum+=end-start;
        if(it%10==0) //it is never 0 but whatever
        {
            printf("%d, %d\n",width, height);
            printf("avg: %.1fms\n",sum/(float)it/1000.);
            it=0;
            sum=0;
        }
        it++;
        SDL_RenderPresent(ren);
    }

    return 0;
}
