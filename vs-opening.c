#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include "3d.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "oklab.h"
#include <assert.h>
#include <string.h> //for memcpy

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

Uint8* pxl_at(int x, int y, int pitch, Uint8* pixels)
{
    return &pixels[4*x+pitch*y];
}

SDL_Point stack[256];
void fill(int* pixels, int width, int height, int x0, int y0)
{
    int filled_pixels=0;
    
    float avg_x=0;
    float avg_y=0;
    int weight=0;
    
    int stack_size = 0;

    stack[stack_size]=(SDL_Point){x0,y0};
    stack_size++;
    do
    {
    int start_x,end_x;
    int x=stack[stack_size-1].x;
    int y=stack[stack_size-1].y;
    stack_size--;
    while(1) //leftmost fill_x
    {
        if(x-1 < 0)
            break;
        Uint8* pxl;
        pxl = &pixels[x-1 + (y)*width];
        
        if(*pxl == 0)
            break;
        x--;
    }
    start_x=x;
    while(1)
    {
        Uint8* pxl;
        Uint8* pxl_behind;
        Uint8* pxl_top;
        Uint8* pxl_topleft;
        if(x < 0 || x >= width)
        {
            break;
        }
        pxl = &pixels[x + (y)*width];
        if(*pxl == 0)
        {
            break;
        }
        int bottom = 0;
        while(bottom <= 2)
        {
            if(x-1 < 0)
            {
                if(y-1+bottom >= 0 && y-1+bottom < height)
                {
                    //add top/ bottom
                    pxl_top = &pixels[x-1 + (y-1+bottom)*width];
                    if(*pxl_top !=0)
                    {
                        stack[stack_size]=(SDL_Point){x,y-1+bottom};
                        stack_size++;
                    }
                }
            }
            else
            {
                pxl_behind = &pixels[x-1 + (y)*width];
                if(*pxl_behind == 0)
                {
                    if(y-1+bottom >= 0 && y-1+bottom < height)
                    {
                        //add top/ bottom
                        pxl_topleft = &pixels[x-1 + (y-1+bottom)*width];
                        pxl_top = &pixels[x + (y-1+bottom)*width];
                        if(*pxl_top !=0 &&
                           *pxl_topleft != 0) //this is needed so that we dont do the same pxl second time below
                        {
                            stack[stack_size]=(SDL_Point){x,y-1+bottom};
                            stack_size++;
                        }
                    }
                }
            }
            
            if(y-1+bottom >= 0 && y-1+bottom < height)
            {
                //add top/ bottom
                pxl_topleft = &pixels[x-1 + (y-1+bottom)*width];
                pxl_top = &pixels[x + (y-1+bottom)*width];
                if(*pxl_top !=0 &&
                   *pxl_topleft==0)
                {
                    stack[stack_size]=(SDL_Point){x,y-1+bottom};
                    stack_size++;
                }
            }
            bottom+=2;
        }
        x++;
    }
    end_x=x-1;
    filled_pixels+=end_x-start_x+1;
    //DrawLine
    for(int x=start_x; x<=end_x; ++x)
    {
        avg_x+=pixels[x + (y)*width]*x;
        avg_y+=pixels[x + (y)*width]*y;
        weight+=pixels[x + (y)*width];
        
        pixels[x + (y)*width] = 0;
    }

    //now repeat for everyting in the stack
    } while(stack_size!=0);
    
    avg_x = avg_x / weight;
    avg_y = avg_y / weight;
    
}

void update_surf(int* pixels, SDL_Surface* surf)
{
    Uint8* pixels_= surf->pixels;
    int width     = surf->w;
    int height    = surf->h;
    int pitch     = surf->pitch;
    for(int y=0; y<height; ++y)
        for(int x=0; x<width; ++x)
        {
            Uint8* pxl = pxl_at(x,y,pitch,pixels_);
            pxl[0]=0;
            pxl[1]=0;
            pxl[2]=0;
            if(pixels[x+width*y] > 0)
                pxl[2]=fmin(69+255*sqrtf(fabs(pixels[x+width*y]) / (3. * 255)),255);
            else if(pixels[x+width*y] < 0)
                pxl[0]=0*fmin(69+255*sqrtf(fabs(pixels[x+width*y]) / (3. * 255)),255);
        } 
}

int* ff(SDL_Surface* surf)
{
    Uint8* pixels_= surf->pixels;
    int width     = surf->w;
    int height    = surf->h;
    int pitch     = surf->pitch;
    int* pixels = malloc(sizeof(int)*width*height);
    int* copy_of_pixels = malloc(sizeof(int)*width*height);
    int min=0;
    for(int y=0; y<height; ++y)
        for(int x=0; x<width; ++x)
        {
            Uint8* pxl = pxl_at(x,y,pitch,pixels_);
            pixels[x+width*y]=pxl[0]+pxl[1]+pxl[2]-3*16;
            if(pixels[x+width*y]<min)
                min = pixels[x+width*y];
        }
    printf("min=%d\n",min);
    for(int y=0; y<height; ++y)
        for(int x=0; x<width; ++x)
        {
            if(pixels[x+width*y] !=0 )
                pixels[x+width*y] += -min;
        }

    for(int y=0; y<height; ++y)
        for(int x=0; x<width; ++x)
        {
            if(pixels[x+width*y]!=0)
            {
                memcpy(copy_of_pixels, pixels, sizeof(int)*width*height);
                // fill(fill_pixels,width,height,x,y);
            }
        }
        
    update_surf(pixels, surf);
    return pixels;
}

void decrease_by_one(int* pixels, SDL_Surface* surf)
{
    int width     = surf->w;
    int height    = surf->h;
    for(int y=0; y<height; ++y)
        for(int x=0; x<width; ++x)
        {
            pixels[x+width*y]-=1;
        }
    update_surf(pixels, surf);
}

void increase_by_one(int* pixels, SDL_Surface* surf)
{
    int width     = surf->w;
    int height    = surf->h;
    for(int y=0; y<height; ++y)
        for(int x=0; x<width; ++x)
        {
            pixels[x+width*y]+=1;
        }
    update_surf(pixels, surf);
}

int main(int argc, char* argv[])
{
    SDL_Window* win;
    win = SDL_CreateWindow("vs-opening",1360,840, 0&SDL_WINDOW_BORDERLESS | (0&SDL_WINDOW_FULLSCREEN));
    //SDL_SetWindowRelativeMouseMode(win,1);
    int width, height;
    SDL_Surface* bmp = SDL_LoadBMP("ho_lu_shit.bmp");
    width = bmp->w;
    height = bmp->h;
    SDL_SetWindowSize(win, width, height);
    SDL_Surface* screen_surf = SDL_GetWindowSurface(win);
    SDL_BlitSurface(bmp, NULL, screen_surf, NULL);
    //SDL_SaveBMP(screen_surf, "angel_modifried.bmp");
    //init_3d(width, height);

    int* pixels = ff(screen_surf);

        
    int it=1;
    Uint64 start,end,sum=0,delta=0;
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
        
        bool* keyboard = SDL_GetKeyboardState(NULL);
        static int decreased=0;
        if(keyboard[SDL_SCANCODE_MINUS]) { printf("Decreasing... now at %d\n", ++decreased); decrease_by_one(pixels, screen_surf);}
        if(keyboard[SDL_SCANCODE_EQUALS]) { printf("Increasing... now at %d\n", --decreased); increase_by_one(pixels, screen_surf);}

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
        SDL_DelayNS(10000000-delta); //100fps
    }

    return 0;
}