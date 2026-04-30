#include <stdio.h>
#include <math.h>
#include "3d.h"
#define M_PI        3.14159265358979323846
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "oklab.h"

int main(int argc, char** argv)
{
    SDL_Window* win;
    win = SDL_CreateWindow("vs-opening",690, 420, 0&SDL_WINDOW_BORDERLESS | (0&SDL_WINDOW_FULLSCREEN));
    SDL_SetWindowRelativeMouseMode(win,1);
    int width, height;
    SDL_GetWindowSize(win, &width, &height);
    Uint64 sum=0;
    int it=1;
    SDL_Surface* screen_surf = SDL_GetWindowSurface(win);
    unsigned char* pixels = screen_surf->pixels;
    int pitch = screen_surf->pitch;
    float L=.9;
    float C=0.125;
    float h=0.0;
    init_3d(width, height);
    printf("%f\n",v_h);
    free(points);
    points = malloc(point_count*sizeof(Point3d));
    init_points(points, point_count);
    
    while(true)
    {
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_EVENT_QUIT)
                return 0;
            if(e.type == SDL_EVENT_MOUSE_MOTION)
                turn_camera(e.motion.xrel,e.motion.yrel);
            if(e.type == SDL_EVENT_KEY_DOWN)
            {
                
            }
        }
        Uint64 start=SDL_GetTicksNS();
        SDL_ClearSurface(screen_surf,0.5,0.5,0.5,1);
        draw_screen(pixels, pitch, points, point_count);
        SDL_UpdateWindowSurface(win);

        SDL_Delay(30);
        Uint64 end=SDL_GetTicksNS();
        sum+=end-start;
        if(it>=10)
        {
            printf("avg: %.3fms\n",SDL_NS_TO_MS(sum/(float)it));
            it=0;
            sum=0;
        }
        it++;
    }

    return 0;
}
