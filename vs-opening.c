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
    SDL_FPoint points[690*420];
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
        Uint64 start=SDL_GetTicks();
        //SDL_RenderPoints(SDL_Renderer *renderer, const SDL_FPoint *points, int count);
        //SDL_RenderTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_FRect *srcrect, const SDL_FRect *dstrect);
        for(int i=0; i < height; ++i)
            for(int j=0; j < width; ++j)
            {
                SDL_SetRenderDrawColorFloat(ren,1,1,1,1);
                SDL_RenderPoint(ren,j,i);
            }
        Uint64 end=SDL_GetTicks();
        sum+=end-start;
        if(it%10==0) //it is never 1 but whatever
        {
            printf("%d, %d\n",width, height);
            printf("avg: %.1fms\n",sum/(float)it);
            it=0;
            sum=0;
        }
        it++;
        SDL_RenderPresent(ren);
    }

    return 0;
}
