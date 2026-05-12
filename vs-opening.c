#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "3d.h"
#define M_PI        3.14159265358979323846
#include "SDL3/SDL.h"
#undef main
#include "SDL3/SDL_main.h"
#include "oklab.h"

//actually i have a better idea
// lets make a backwards renderer
// we take the picture
// and we shoot a ray in the direction of the point
// the dist is 1/size of the point
// now i just a need a program to take a blurry circle
// and tell me its size
// so like a backwards anti aliasing
// but thats propably impossible
// so i will just roughly guess it
// maybe add up all the r g b values to get the size
// idk

//w ogole na poczatku bym zrobil pas
// wszystkie piksle < 16 16 16
// maja sie rownac 16 16 16
//piksel po pikselu
// jeśli piksel rgb > 16 16 16
// to poszukaj sąsiadów 
// pozycja punktu to śr ważona
// waga to jasność
// poz = (jasność0 * poz0 + jasność1 * poz1 + ...) / jasność;
// odległość ~ 1 / jasność;
// "usun" wszystkich sasiadiow z listy pikseli zeby ich nie przejsc wiecej niz raz
// w sensie oznacz ich jakos zeby nastepny razem je skipnac

//może jakiś signal processing trzeba bedzie poczytac
// bo mamy punktu wysokie 
// i chcemy zeby piksele na okolo nich tez zaliczaly sie do tych punktow
// ale to bedzie roboty...
// i tak ciekawsze niz zgadywanie tej gownianej funkcji


// !!!!!!!!!!!!!!!!!!!!!!!!
// ! Na razie zrób PROSTO !
// !!!!!!!!!!!!!!!!!!!!!!!!
typedef struct Button
{
    int down;
    int held;
    int up;
} Button;

#define PLENTY 32
Button buttons[PLENTY];

void reset_buttons()
{
    int i=0;
    while(i < PLENTY)
    {
        buttons[i].down=0; buttons[i].up=0;
        ++i;
    }
}


int main(int argc, char* argv[])
{
    SDL_Window* win;
    win = SDL_CreateWindow("vs-opening",680, 420, 0&SDL_WINDOW_BORDERLESS | (0&SDL_WINDOW_FULLSCREEN));
    SDL_SetWindowRelativeMouseMode(win,1);
    int width, height;
    SDL_GetWindowSize(win, &width, &height);
    Uint64 sum=0;
    int it=1;
    SDL_Surface* screen_surf = SDL_GetWindowSurface(win);
    float L=.9;
    float C=0.125;
    float h=0.0;
    SDL_Surface* bullshit;
    SDL_Surface* bmp = SDL_ConvertSurface(bullshit=SDL_LoadBMP("goal.bmp"), screen_surf->format);
    SDL_DestroySurface(bullshit);
    unsigned char* bmp_pixels=bmp->pixels;
    for(int y=0; y<bmp->h; ++y)
    {
        for(int x=0; x<bmp->w; ++x)
        {
            Uint8* b = &bmp_pixels[4*x+0+y*bmp->pitch];
            Uint8* g = &bmp_pixels[4*x+1+y*bmp->pitch];
            Uint8* r = &bmp_pixels[4*x+2+y*bmp->pitch];
            // Uint8* a = &bmp_pixels[4*x+3+y*bmp->pitch];
            if(*r == 16 && *g == 16 && *b == 16)
            {
                *r=0;
                *g=0;
                *b=0;
            }
            // if(*r+*g+*b>=725)
            // {
                // *r=0;
                // *g=0;
                // *b=0;
            // }
            else if(*r+*g+*b<100)
            {
                *r=0;
                *g=0;
                *b=0;
            }
            
            // else if(*r < 16 && *g < 16 && *b < 16)
            // {
                // *r=0;
                // *g=0;
                // *b=0;
            // }
            // else
            // {
                // *r=255;
                // *g=255;
                // *b=255;                
            // }
        }
    }
    printf("%d\n", bmp->format & SDL_PIXELFORMAT_ABGR32);
    init_3d(width, height);

    int brightness=765;
    Uint64 start,end,delta=0;
    while(true)
    {
        start=SDL_GetTicksNS();
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_EVENT_QUIT)
                return 0;
            if(e.type == SDL_EVENT_MOUSE_MOTION)
                turn_camera(e.motion.xrel,e.motion.yrel);
            if(e.type == SDL_EVENT_KEY_DOWN)
            {
                int i=0;
                if(e.key.key == SDLK_P) { buttons[i].held=1; buttons[i].down=1; } ++i;
                if(e.key.key == SDLK_W) { buttons[i].held=1; buttons[i].down=1; } ++i;
                if(e.key.key == SDLK_S) { buttons[i].held=1; buttons[i].down=1; } ++i;
                if(e.key.key == SDLK_D) { buttons[i].held=1; buttons[i].down=1; } ++i;
                if(e.key.key == SDLK_A) { buttons[i].held=1; buttons[i].down=1; } ++i;
                if(e.key.key == SDLK_SPACE) { buttons[i].held=1; buttons[i].down=1; } ++i;
                if(e.key.key == SDLK_LSHIFT) { buttons[i].held=1; buttons[i].down=1; } ++i;
                if(e.key.key == SDLK_EQUALS) { buttons[i].held=1; buttons[i].down=1; } ++i;
                if(e.key.key == SDLK_MINUS) { buttons[i].held=1; buttons[i].down=1; } ++i;
            }
            if(e.type == SDL_EVENT_KEY_UP)
            {
                int i=0;
                if(e.key.key == SDLK_P) { buttons[i].held=0; buttons[i].up=1; } ++i;
                if(e.key.key == SDLK_W) { buttons[i].held=0; buttons[i].up=1; } ++i;
                if(e.key.key == SDLK_S) { buttons[i].held=0; buttons[i].up=1; } ++i;
                if(e.key.key == SDLK_D) { buttons[i].held=0; buttons[i].up=1; } ++i;
                if(e.key.key == SDLK_A) { buttons[i].held=0; buttons[i].up=1; } ++i;
                if(e.key.key == SDLK_SPACE) { buttons[i].held=0; buttons[i].up=1; } ++i;
                if(e.key.key == SDLK_LSHIFT) { buttons[i].held=0; buttons[i].up=1; } ++i;
                if(e.key.key == SDLK_EQUALS) { buttons[i].held=0; buttons[i].up=1; } ++i;
                if(e.key.key == SDLK_MINUS) { buttons[i].held=0; buttons[i].up=1; } ++i;
            }
        }
        
        {
        int i=0;
        if(buttons[i].down) { print_scr_pos(); } ++i;
        }
        {
        int i=0;
        i++;
        const float flyinGspeed=0.006;
        if(buttons[i].held) { move_camera(flyinGspeed); } ++i;
        if(buttons[i].held) { move_camera(-flyinGspeed); } ++i;
        if(buttons[i].held) { strafe_camera(flyinGspeed); } ++i;
        if(buttons[i].held) { strafe_camera(-flyinGspeed); } ++i;
        if(buttons[i].held) { fly_camera(flyinGspeed); } ++i;
        if(buttons[i].held) { fly_camera(-flyinGspeed); } ++i;
        if(buttons[i].held) { ; } ++i;
        if(buttons[i].held) { ; } ++i;
        }
        reset_buttons();
        
        //SDL_ClearSurface(screen_surf,0,0,0,1);
        //draw_screen_one_color(screen_surf);
        SDL_BlitSurface(bmp, NULL, screen_surf, NULL);
        Uint8* screen_pixels = screen_surf->pixels;
        Uint8* bmp_pixels = bmp->pixels;
        for(; brightness>=100; --brightness)
        {
            for(int y=0; y<screen_surf->h; ++y)
            {
                for(int x=0; x<screen_surf->w; ++x)
                {
                    Uint8* b = &bmp_pixels[4*x+0+y*bmp->pitch];
                    Uint8* g = &bmp_pixels[4*x+1+y*bmp->pitch];
                    Uint8* r = &bmp_pixels[4*x+2+y*bmp->pitch];
                    int sum=*r+*g+*b;
                    if(sum>=brightness)
                    {
                        const float a0=-5846.13;
                        const float x0=-545.415;
                        const float y0=+10.0662;
                        float diameter = a0/(sum - x0) + y0;
                        int sum_=0;
                        for(int box_x=-diameter/2; box_x < diameter - diameter/2; ++box_x)
                        {
                            for(int box_y=-diameter/2; box_y < diameter - diameter/2; ++box_y)
                            {
                                if(box_x*box_x +box_y*box_y <= diameter)
                                {
                                    if(x+box_x >= 0 && x+box_x < bmp->w && y+box_y >=0 && y+box_y < bmp->h)
                                    {
                                        sum_+=bmp_pixels[4*(x+box_x)+0+(y+box_y)*bmp->pitch];
                                        sum_+=bmp_pixels[4*(x+box_x)+1+(y+box_y)*bmp->pitch];
                                        sum_+=bmp_pixels[4*(x+box_x)+2+(y+box_y)*bmp->pitch];
                                        bmp_pixels[4*(x+box_x)+0+(y+box_y)*bmp->pitch]=0;
                                        bmp_pixels[4*(x+box_x)+1+(y+box_y)*bmp->pitch]=0;
                                        bmp_pixels[4*(x+box_x)+2+(y+box_y)*bmp->pitch]=0;
                                    }
                                }
                            }
                        }
                        backwards_render(4000/(float)sum_,x,y);
                    }
                }
            }
            SDL_BlitSurface(bmp, NULL, screen_surf, NULL);
            SDL_UpdateWindowSurface(win);
            printf("brightness=%d\n",brightness);
        }
        draw_screen_one_color(screen_surf);
        SDL_UpdateWindowSurface(win);
        sum+=delta;
        if(it>=10)
        {
            printf("avg: %.3fms\n",SDL_NS_TO_MS(sum/(float)it));
            it=0;
            sum=0;
        }
        it++;
        end=SDL_GetTicksNS();
        delta = end-start;
        SDL_DelayNS(10'000'000-delta); //100fps
    }

    return 0;
}
    