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

//TODO: not counting black pixels get the average color and color the points with that
// try that on the other microsoft picture blend.png

//i guess this started working weight_deleted because 1. it was set 0 lol, but also
// since we do everything in one pass, we only have N==0, so... yeeeah.

//apropos the image ho_lu_shit.bmp:
//you can see the exact cutoff point where some sort of effect was used on the bottom points making them wider (fuzzy) because the points split in 3d space in that area

//TODO: why is N going up to 150 already enough to delete the text (even before -132)????

//TODO: also why is the visual studio text
// rendered as a lot of points?
// shouldnt it be one very close point?
//
// Maybe its due to optimazations? but it was there before...


//TODO: disassemble ff() into initialization and computation
// and put inside the 'game' loop

// TODO: maybe this creates wrong results:
//   after 2nd pass some pixels that have should be flooded are not?
//   like single pixels?? weird

//3_points_array.txt:
//The points are too quickly coming towaards the camera
// they should be furtther

//4_points_array.txt:
//Okay now it looks like the first idea i had
//  but shittier bruh


//TODO: also why is the visual studio text
// rendered as a lot of points?
// shouldnt it be one very close point?


//TODO: make the flood thicker by 1 pixl
// so it captures some pixels even if they are not perfectly
// neighbouring. Maybe allow diagonal connections?
// Either that or make fuzzy border, so we can catch some more pixels


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

#define MAX_STACK_SIZE 4*1024
SDL_Point stack[MAX_STACK_SIZE];
void fill(int* pixels, int width, int height, int x0, int y0, float* avg_x, float* avg_y, float* weight, int* filled_pixels, SDL_Point* min, SDL_Point* max, int filled_pixels_max, int* skip_these_pixels)
{
    *filled_pixels=0;
    min->x=x0;
    min->y=y0;
    max->x=x0;
    max->y=y0;
    
    *avg_x=0;
    *avg_y=0;
    *weight=0;
    
    int stack_size = 0;

    stack[stack_size]=(SDL_Point){x0,y0};
    stack_size++;
    do
    {
    int start_x,end_x;
    int x=stack[stack_size-1].x;
    int y=stack[stack_size-1].y;
    if(y < min->y)       min->y=y;
    if(y > max->y)       max->y=y;
    stack_size--;
    while(1) //leftmost x
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
            skip_these_pixels[x+width*y]=1;
            if(x < min->x)   min->x=x;
            if(x > max->x)   max->x=x;
            if(++(*filled_pixels) >= filled_pixels_max) return;
            x++;
            break;
        }
        pxl = &pixels[x + (y)*width];
        if(*pxl == 0)
        {
            skip_these_pixels[x+width*y]=1;
            if(x < min->x)   min->x=x;
            if(x > max->x)   max->x=x;
            if(++(*filled_pixels) >= filled_pixels_max) return;
            x++;
            break;
        }
        int bottom = 0;
        while(bottom <= 2)
        {
            if(stack_size >= MAX_STACK_SIZE-10)
                printf("MAX_STACK_SIZE about to be exceeded! ABORT! \n");
            if(x-1 < 0)
            {
                if(y-1+bottom >= 0 && y-1+bottom < height)
                {
                    //add top/ bottom
                    pxl_top = &pixels[x + (y-1+bottom)*width];
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
                if(*pxl_behind == 0)//I think this might be always true
                {
                    if(y-1+bottom >= 0 && y-1+bottom < height)
                    {
                        //add top/ bottom
                        pxl_topleft = &pixels[x-1 + (y-1+bottom)*width];
                        pxl_top = &pixels[x + (y-1+bottom)*width];
                        if(*pxl_top !=0 &&
                           *pxl_topleft != 0) //this is needed so that we dont do the same Point on the stack second time later
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
//later:            
                if(*pxl_top !=0 &&
                   *pxl_topleft==0)
                {
                    stack[stack_size]=(SDL_Point){x,y-1+bottom};
                    stack_size++;
                }
            }
            bottom+=2;
        }
        skip_these_pixels[x+width*y]=1;
        if(x < min->x)   min->x=x;
        if(x > max->x)   max->x=x;
        if(++(*filled_pixels) >= filled_pixels_max) return;
        x++;
    }
    end_x=x-1;
    *filled_pixels=end_x-start_x+1;
    //DrawLine
    for(int x=start_x; x<=end_x; ++x)
    {
        *avg_x+=pixels[x + (y)*width]*x;
        *avg_y+=pixels[x + (y)*width]*y;
        *weight+=pixels[x + (y)*width];
        
        pixels[x + (y)*width] = 0;
    }
    if(start_x < min->x) min->x=start_x;
    if(end_x > max->x)   max->x=end_x;
    //now repeat for everyting in the stack
    } while(stack_size!=0);
    
    *avg_x = *avg_x / *weight;
    *avg_y = *avg_y / *weight;
    
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
                pxl[0]=fmin(69+255*sqrtf(fabs(pixels[x+width*y]) / (3. * 255)),255);
        } 
}

void memcpy_area(int* to, int* from, int width, SDL_Point* min, SDL_Point* max)
{
    for(int y=min->y; y <= max->y; ++y)
    {
        size_t length = 1 + max->x - min->x;
        memcpy(to + y*width + min->x, from + y*width + min->x, sizeof(int)*length);
    }
}

void saving_points_test(SDL_Surface* surf)
{
    Uint8* pixels_= surf->pixels;
    int width     = surf->w;
    int height    = surf->h;
    int pitch     = surf->pitch;
    for(int y=0; y<height; y+=32)
        for(int x=0; x<width; x+=32)
        {
            backwards_render((x+y)/1000., x, y);
        }
    save_points();
    reset_points();
    load_points();
}


int* ff(SDL_Window* win)
{
    SDL_Surface* surf = SDL_GetWindowSurface(win);
    Uint8* pixels_= surf->pixels;
    int width     = surf->w;
    int height    = surf->h;
    int pitch     = surf->pitch;
    int* pixels = malloc(sizeof(int)*width*height);
    int* fill_pixels = malloc(sizeof(int)*width*height);
    int* skip_these_pixels = malloc(sizeof(int)*width*height);
    for(int i=0; i < width*height; ++i) skip_these_pixels[i]=0;
    
    for(int y=0; y<height; ++y)
        for(int x=0; x<width; ++x)
        {
            Uint8* pxl = pxl_at(x,y,pitch,pixels_);
            if(pxl[0]==16&&pxl[1]==16&&pxl[2]==16) pixels[x+width*y]=0;
            else 
            {
                pixels[x+width*y]=pxl[0]+pxl[1]+pxl[2]-132;
                if(pixels[x+width*y]<0) pixels[x+width*y]=0; 
            }
        }
    memcpy(fill_pixels, pixels, sizeof(int)*width*height);

    update_surf(fill_pixels, surf);
    SDL_UpdateWindowSurface(win);
    
    int iterations = 1;(3 * 255-1)-132;
    int N=0;
    int added_points=0;
    int added_points_this_N=0;
    Uint64 end, start=SDL_GetTicksNS();
    float time;
    while(N < iterations)
    {
    added_points_this_N=0;
    for(int y=0; y<height; ++y)
        for(int x=0; x<width; ++x)
        {
            if(pixels[x+width*y]!=0 && skip_these_pixels[x+width*y]==0)
            {
                int filled_pixels;
                SDL_Point min, max;
                float avg_x, avg_y;
                float weight;
                const int MAX_FILLED_PIXELS=169;
                fill(fill_pixels,width,height,x,y, &avg_x, &avg_y, &weight, &filled_pixels, &min, &max, MAX_FILLED_PIXELS, skip_these_pixels);
                if(filled_pixels != 0)
                {
                    //So i because we subtracted 132 from each pixel, they became way smaller, so even filled_pixels >= 6 is a big chunk of the image
                    if(filled_pixels < MAX_FILLED_PIXELS)// && filled_pixels >= 6)
                    {
                        float r=sqrt(filled_pixels / M_PI);
                        float correction_factor=0.75;
                        float weight_deleted=0;
                        for(int i=1; i<=N; ++i)
                        {
                            weight_deleted+=2*M_PI*(r+i)*(N+1-i);
                        }
                        weight_deleted*=correction_factor;
                        weight+=filled_pixels*N+weight_deleted;
                        
                        added_points++;
                        added_points_this_N++;
                        backwards_render((2200)/weight, avg_x, avg_y);
                        memcpy_area(pixels, fill_pixels, width, &min, &max);
                    }
                    else
                    {
                        memcpy_area(fill_pixels, pixels, width, &min, &max);
                    }
                }
                end=SDL_GetTicksNS();
                time=SDL_NS_TO_MS((float)end-start)/1000;
                printf("\rN=%3d added %4d points, overall we added %5d, time taken so far: %.1fs, %.2f%%",N, added_points_this_N, added_points, time, (x+width*y) * 100.f / (height*width));
            }
        }
    
    for(int i=0; i < width*height; ++i) skip_these_pixels[i]=0; //reset skip_these_pixels for each N
    
    for(int y=0; y<height; ++y)
        for(int x=0; x<width; ++x)
        {
            if(pixels[x+width*y] != 0)
            {
                pixels[x+width*y]-=1;
            }
        }
    char out_path[64];
    sprintf(out_path, "passes/holyshit_after_%d_pass.bmp", N);
    SDL_SaveBMP(surf, out_path);
    update_surf(pixels, surf);
    SDL_UpdateWindowSurface(win);
    save_points();
    
    end=SDL_GetTicksNS();
    time=SDL_NS_TO_MS((float)end-start)/1000;
    printf("\rN=%3d added %4d points, overall we added %5d, time taken so far: %.1fs, %.2f%%\n",N, added_points_this_N, added_points, time, 100.f);
    N++;
    }
    
    return pixels;
}

SDL_Color get_avg_color(Uint8* pixels, int width, int height)
{
    Lab sum = (Lab){0,0,0};
    int count=0;
    for(size_t i=0; i < width*height; i+=4)
    {
        Uint8* pxl=&pixels[i];
        int pxl_sum=pxl[0]+pxl[1]+pxl[2];
        if(pxl_sum>=300 && pxl_sum <=3*255-50)
        {
            RGB rgb;
            rgb.r=(float)pxl[2]/255;
            rgb.g=(float)pxl[1]/255;
            rgb.b=(float)pxl[0]/255;
            Lab lab = linear_srgb_to_oklab(rgb);
            sum.L += lab.L;
            sum.a += lab.a;
            sum.b += lab.b;
            count++;
        }
    }
    sum.L /= count;
    sum.a /= count;
    sum.b /= count;
    RGB avg=oklab_to_linear_srgb(sum);
    printf("\n%f, %f, %f\n", avg.r, avg.g, avg.b);
    SDL_Color color;
    color.r=fmin(avg.r*255,255);
    color.g=fmin(avg.g*255,255);
    color.b=fmin(avg.b*255,255);
    printf("\n%d, %d, %d\n", color.r, color.g, color.b);
    return color;
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
    SDL_Color color=get_avg_color(screen_surf->pixels, width, height);
    init_3d(width, height);
    
    int* pixels = ff(win);
    //load_points();
    
    SDL_SetWindowRelativeMouseMode(win,1);
    
    SDL_DisableScreenSaver();
    
    float flyinGspeed=0.0006;
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
            if(e.type == SDL_EVENT_MOUSE_WHEEL)
            {
                if(e.wheel.y > 0)
                    flyinGspeed *= 1.125;
                else
                    flyinGspeed /= 1.125;
            }
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
        static float exponent=1;
        static int fps_toggled=0;
        if(keyboard[SDL_SCANCODE_MINUS]) { printf("Decreasing... now at %d\n", ++decreased); }//decrease_by_one(pixels, screen_surf);}
        if(keyboard[SDL_SCANCODE_EQUALS]) { printf("Increasing... now at %d\n", --decreased); }//increase_by_one(pixels, screen_surf);}
        if(keyboard[SDL_SCANCODE_Q]) { exponent/=1.00625; printf("Current exponent: %f\n", exponent); change_dist_exponent(exponent); }
        if(keyboard[SDL_SCANCODE_E]) { exponent*=1.00625; printf("Current exponent: %f\n", exponent); change_dist_exponent(exponent); }
        if(keyboard[SDL_SCANCODE_F]) fps_toggled=!fps_toggled;
        
        SDL_ClearSurface(screen_surf, 0,0,0,0);
        draw_screen_one_color(screen_surf, color);
        SDL_UpdateWindowSurface(win);
        sum+=delta;
        if(it>=10)
        {
            if(fps_toggled)
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