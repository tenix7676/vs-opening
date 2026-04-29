#include <stdio.h>
#include <math.h>
#define M_PI        3.14159265358979323846
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
typedef struct Lab {float L; float a; float b;} Lab;
typedef struct RGB {float r; float g; float b;} RGB;

Lab linear_srgb_to_oklab(RGB c) 
{
    float l = 0.4122214708f * c.r + 0.5363325363f * c.g + 0.0514459929f * c.b;
	float m = 0.2119034982f * c.r + 0.6806995451f * c.g + 0.1073969566f * c.b;
	float s = 0.0883024619f * c.r + 0.2817188376f * c.g + 0.6299787005f * c.b;

    float l_ = cbrtf(l);
    float m_ = cbrtf(m);
    float s_ = cbrtf(s);

    Lab lab={
        0.2104542553f*l_ + 0.7936177850f*m_ - 0.0040720468f*s_,
        1.9779984951f*l_ - 2.4285922050f*m_ + 0.4505937099f*s_,
        0.0259040371f*l_ + 0.7827717662f*m_ - 0.8086757660f*s_,
    };
    return lab;
}

RGB oklab_to_linear_srgb(Lab c) 
{
    float l_ = c.L + 0.3963377774f * c.a + 0.2158037573f * c.b;
    float m_ = c.L - 0.1055613458f * c.a - 0.0638541728f * c.b;
    float s_ = c.L - 0.0894841775f * c.a - 1.2914855480f * c.b;

    float l = l_*l_*l_;
    float m = m_*m_*m_;
    float s = s_*s_*s_;

    RGB rgb={
		+4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
		-1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
		-0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s,
    };
    return rgb;
}

int main(int argc, char** argv)
{
    SDL_Window* win;
    //SDL_Renderer* ren;
    //SDL_CreateWindowAndRenderer("vs-opening",690, 420, 0&SDL_WINDOW_BORDERLESS | (0&SDL_WINDOW_FULLSCREEN),&win,&ren);
    win = SDL_CreateWindow("vs-opening",690, 420, 0&SDL_WINDOW_BORDERLESS | (0&SDL_WINDOW_FULLSCREEN));
    int width, height;
    SDL_GetWindowSize(win, &width, &height);
    Uint64 sum=0;
    int it=1;
    SDL_Surface* screen_surf = SDL_GetWindowSurface(win);
    float L=.9;
    float C=0.125;
    float h=0.0;
    //SDL_ClearSurface
    while(true)
    {
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_EVENT_QUIT)
                return 0;
        }
        Uint64 start=SDL_GetTicksNS();
        for(int i=0; i < height; ++i)
        {
            for(int j=0; j < width; ++j)
            {
                Lab lab = {L,C*cos(h),C*sin(h)};
                RGB rgb = oklab_to_linear_srgb(lab);
                SDL_WriteSurfacePixelFloat(screen_surf, j, i,rgb.r,rgb.g,rgb.b,1);
                h+=1./(width*height)*2*M_PI;               
            }
        }
        static int i=0;
        h=i++*0.05;
        SDL_UpdateWindowSurface(win);
        Uint64 end=SDL_GetTicksNS();
        sum+=end-start;
        if(it>=100)
        {
            printf("%d, %d\n",width, height);
            printf("avg: %.3fms\n",SDL_NS_TO_MS(sum/(float)it));
            it=1;
            sum=0;
        }
        it++;

    }

    return 0;
}
