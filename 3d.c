#include "SDL3/SDL.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include "3d.h"
#include "oklab.h"
#include <string.h> //for memcpy and memset

void update_scr_pos();
void update_cam_pos();
void update_s_y_s_x();
void normalise(Point3d*);
void update_cam_dir();

int cam_dist;
Point3d cam_dir;
Point3d cam_pos;
Point3d initial_cam_pos;
Point3d scr_pos; 
float cam_leftright=0;
float cam_updown=0.0f;
//the closer the screen, i mean virutal world viewport is to the camera
// the closer i can get to a 3d point without it disappearing 
// (because it goes behind the screen)
// but i guess checking if a point goes behind the camera
// instead of if it goes behind the screen also works great
#define RENDER_IN_FRONT_OF_CAMERA
#ifndef RENDER_IN_FRONT_OF_CAMERA
#define RENDER_IN_FRONT_OF_SCREEN
#endif
int s_h,s_w;
float v_w, v_h;
Point3d s_y, s_x;

size_t point_count=0;
Point3d* points=NULL;
SDL_Rect* rects=NULL;
float* dists=NULL;

void init_3d(int s_w_, int s_h_)
{
    s_h=s_h_;s_w=s_w_;
    v_w=1;
    v_h=(float)s_h/s_w*v_w;
    cam_dist=v_w*55/34.5;
    update_cam_dir();
    scr_pos=(Point3d){-1, 0, 0};
    update_cam_pos(scr_pos);
    initial_cam_pos = cam_pos;
    
    init_points();
}
void update_s_y_s_x()
{
    s_x=(Point3d){sinf(cam_leftright), -cosf(cam_leftright), 0};
    s_y.x=cosf(cam_updown-M_PI_2)*cosf(cam_leftright);
    s_y.y=cosf(cam_updown-M_PI_2)*sinf(cam_leftright);
    s_y.z=sinf(cam_updown-M_PI_2);
}
void update_cam_dir()
{
    cam_dir.z=sinf(cam_updown);
    cam_dir.x=cosf(cam_updown)*cosf(cam_leftright);
    cam_dir.y=cosf(cam_updown)*sinf(cam_leftright);
    update_s_y_s_x();
}
void update_scr_pos()
{
    scr_pos.x = cam_pos.x + cam_dist * cam_dir.x;
    scr_pos.y = cam_pos.y + cam_dist * cam_dir.y;
    scr_pos.z = cam_pos.z + cam_dist * cam_dir.z;
}
void update_cam_pos()
{
    cam_pos.x = scr_pos.x - cam_dist * cam_dir.x;
    cam_pos.y = scr_pos.y - cam_dist * cam_dir.y;
    cam_pos.z = scr_pos.z - cam_dist * cam_dir.z;
}
float dot(Point3d a, Point3d b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
float length(Point3d a)
{
    return sqrtf(dot(a,a));
}
Point3d calc_normalised(Point3d a)
{
    Point3d norm = a;
    float len = length(a);
    norm.x /= len;
    norm.y /= len;
    norm.z /= len;
    return norm;
}
void normalise(Point3d* a)
{
    float len = length(*a);
    if(len==0) return;
    a->x /= len;
    a->y /= len;
    a->z /= len;
}

float f(float x)
{
    // x*=4;
    // return (expf(16*-x*x))*M_PI_2*.35;
    return expf(-128*x*x)*M_PI_2*.5+M_PI_2*.5*x;
}
void init_points()
{
    point_count=1024*16;//PLENTY

    points = malloc(point_count*sizeof(Point3d));
    rects = malloc(sizeof(SDL_Rect)*point_count);
    dists = malloc(sizeof(float)*point_count);
}
void draw_pixel(unsigned char* pixels, int pitch, int x, int y, unsigned char r,  unsigned char g,  unsigned char b)
{
    pixels[y*pitch+4*x+0]=b;
    pixels[y*pitch+4*x+1]=g;
    pixels[y*pitch+4*x+2]=r;
    //pixels[y*pitch+4*x+3]=420;
}
float check_point(Point3d* point, int* sex, int* sey)
{
    Point3d p={
    point->x - cam_pos.x,
    point->y - cam_pos.y,
    point->z - cam_pos.z
    };

    float len_p1 = dot(p, cam_dir);
    #ifdef RENDER_IN_FRONT_OF_SCREEN
    //check if behind of screen plane
    if(len_p1 <= cam_dist)
        return 0;
    #endif
    #ifdef RENDER_IN_FRONT_OF_CAMERA
    //check if behind of camera plane
    if(len_p1 <= 0)
        return 0;
    #endif
    
    float len_p2 = dot(p,s_x);
    float sex_ = len_p2 / len_p1 * cam_dist / v_w * s_w/2;
    *sex = sex_;
    *sex += s_w/2;
    //check if out of screen
    if(!(*sex >= 0 && *sex < s_w))
       return 0;
    
    len_p2 = dot(p,s_y);
    float sey_ = len_p2 / len_p1 * cam_dist / v_h * s_h/2;
    *sey = sey_;
    *sey += s_h/2;
    //check if out of screen
    if(!(*sey >= 0 && *sey < s_h))
       return 0;

    return len_p1;
}

float check_point_float(Point3d* point, float* sex, float* sey)
{
    Point3d p={
    point->x - cam_pos.x,
    point->y - cam_pos.y,
    point->z - cam_pos.z
    };

    float len_p1 = dot(p, cam_dir);
    #ifdef RENDER_IN_FRONT_OF_SCREEN
    //check if behind of screen plane
    if(len_p1 <= cam_dist)
        return 0;
    #endif
    #ifdef RENDER_IN_FRONT_OF_CAMERA
    //check if behind of camera plane
    if(len_p1 <= 0)
        return 0;
    #endif
    
    float len_p2 = dot(p,s_x);
    float sex_ = len_p2 / len_p1 * cam_dist / v_w * s_w/2;
    *sex = sex_;
    *sex += s_w/2;
    //check if out of screen
    if(!(*sex >= 0 && *sex < s_w))
       return 0;
    
    len_p2 = dot(p,s_y);
    float sey_ = len_p2 / len_p1 * cam_dist / v_h * s_h/2;
    *sey = sey_;
    *sey += s_h/2;
    //check if out of screen
    if(!(*sey >= 0 && *sey < s_h))
       return 0;

    return len_p1;
}

Uint8* pxl_at(int x, int y, int pitch, Uint8* pixels);

void pxl_draw(int x, int y, int pitch, Uint8* pixels, SDL_Color color)
{
    Uint8* pxl = pxl_at(x,y,pitch,pixels);
    pxl[0]=color.b;
    pxl[1]=color.g;
    pxl[2]=color.r;
}

void pxl_draw_8_symmetry(int x, int y, int pitch, Uint8* pixels, SDL_Color color, int cx, int cy)
{
    pxl_draw(cx+x,cy+y,pitch,pixels, color);
    pxl_draw(cx+x,cy-y,pitch,pixels, color);
    pxl_draw(cx-x,cy+y,pitch,pixels, color);
    pxl_draw(cx-x,cy-y,pitch,pixels, color);
    pxl_draw(cx+y,cy+x,pitch,pixels, color);
    pxl_draw(cx+y,cy-x,pitch,pixels, color);
    pxl_draw(cx-y,cy+x,pitch,pixels, color);
    pxl_draw(cx-y,cy-x,pitch,pixels, color);
}

//https://groups.csail.mit.edu/graphics/classes/6.837/F98/Lecture6/circle.html
void draw_circle(int cx, int cy, int r, int pitch, Uint8* pixels)
{
    SDL_Color color=(SDL_Color){255,0%256,255};
    int x=0;
    int y=r;
    int f=(5-4*r)/4;
            pxl_draw_8_symmetry(x,y,pitch,pixels,color,cx,cy);
    while(y > x)
    {
        x++;
        f+=2*x+1;
        if(f>=0) { y--; f+=-2*y+1;  }
        int y_=y;
        // while(y_>=x)
        // {    
            pxl_draw_8_symmetry(x,y_,pitch,pixels,color,cx,cy);
            // --y_;
        // }
    }
}

void fuzzy_circle(float cx, float cy, float r, Lab color, SDL_Surface* surf)
{
    Uint8* pixels = surf->pixels;
    int width     = surf->w;
    int height    = surf->h;
    int pitch     = surf->pitch;
    
    float border=2;
    for(int y=-border-r-1; y<r+1+border; ++y)
        for(int x=-border-r-1; x<r+1+border; ++x)
        {
            Lab pxl_color = color;
            float dist = hypotf(x, y);
            if(dist < r) ;
            else if(dist > r+border) continue;
            else
            {
                pxl_color.L=color.L*(1-(dist-r) / border);
            }
            RGB rgb = oklab_to_linear_srgb(pxl_color);
            SDL_Color sdlcolor=(SDL_Color){fmin(rgb.r*255,255), fmin(rgb.g*255,255), fmin(rgb.b*255,255)};
            if(cx+x > 0 && cx+x < width)
            if(cy+y > 0 && cy+y < height)
            pxl_draw(cx+x, cy+y, pitch, pixels, sdlcolor); //instead of this we should take an average
            //of the Lab color thats already there
            //and of the Lab color we want to apply to it
        }
}

#define DIST_SCALING 1
void draw_screen(SDL_Surface* surface)
{
    static float L=.9;
    static float C=0.125;
    static double h=0.0;
    
    //Just in case
    normalise(&cam_dir);
    normalise(&s_x);
    normalise(&s_y);
    for(size_t i=0; i < point_count; ++i)
    {
        h+=1.006/(point_count)*2*M_PI;

        int sex, sey;
        float len_p1 = check_point(points+i,&sex,&sey);
        if(len_p1 > 0)
        {
            Lab lab = {L,C*cos(h),C*sin(h)};
            RGB rgb = oklab_to_linear_srgb(lab);
            SDL_Color color = (SDL_Color){
                fmin(255,rgb.b*255),
                fmin(255,rgb.g*255),
                fmin(255,rgb.r*255) };
            float rect_radius=1+DIST_SCALING/len_p1;//powf(len_p1,.85);
            const SDL_Rect rect=(SDL_Rect){sex-rect_radius,sey-rect_radius,rect_radius,rect_radius};
            
            SDL_FillSurfaceRect(surface, &rect, *(Uint32*)(void*)&color);
        }
    }
}

void draw_screen_one_color(SDL_Surface* surface, SDL_Color color)
{
    //Just in case
    normalise(&cam_dir);
    normalise(&s_x);
    normalise(&s_y);
    int num_rects_this_frame=0;
    for(size_t i=0; i < point_count; ++i)
    {
        int sex, sey;
        //if(dists[i] > 2.5)
        //    continue;
        float len_p1 = check_point(points+i,&sex,&sey);
        if(len_p1 > 0)
        {
            float rect_radius=1+DIST_SCALING/len_p1;//powf(len_p1,.85);
            const SDL_Rect rect=(SDL_Rect){sex-rect_radius,sey-rect_radius,rect_radius,rect_radius};
            rects[num_rects_this_frame] = rect;
            num_rects_this_frame++;
        }
    }
    //SDL_Color color_ = (SDL_Color){ 255, 157, 213 };//this goes b g r
    SDL_Color color_=(SDL_Color){ color.b, color.g, color.r };
    SDL_FillSurfaceRects(surface, rects, num_rects_this_frame, *(Uint32*)(void*)&color_);
}

void draw_screen_fuzzy_circles(SDL_Surface* surf, Lab color)
{
    //Just in case
    normalise(&cam_dir);
    normalise(&s_x);
    normalise(&s_y);
    for(size_t i=0; i < point_count; ++i)
    {
        float sex, sey;
        float len_p1 = check_point_float(points+i,&sex,&sey);
        if(len_p1 > 0)
        {
            float radius=.5+DIST_SCALING/len_p1;//powf(len_p1,.85);
            fuzzy_circle(sex, sey, radius, color, surf);
        }
    }
}
void turn_camera(float left_right,float up_down)
{
    const float turning_speed=-1/256.;
    cam_updown+= .5f * up_down * turning_speed;
    cam_leftright+= left_right * turning_speed;
    
    update_cam_dir();
}
void move_camera(float dist)
{
    cam_pos.x+=dist*cam_dir.x;
    cam_pos.y+=dist*cam_dir.y;
    cam_pos.z+=dist*cam_dir.z;
    update_scr_pos();
}
void fly_camera(float dist)
{
    cam_pos.z+=dist;
    update_scr_pos();
}
void strafe_camera(float dist)
{
    cam_pos.x+=dist*s_x.x;
    cam_pos.y+=dist*s_x.y;
    cam_pos.z+=dist*s_x.z;
    update_scr_pos();
}
void print_scr_pos()
{
    printf("\
float cam_leftright=%f;\n\
float cam_updown=%f;\n\
\tscr_pos=(Point3d){%f, %f, %f};\n", cam_leftright, cam_updown, scr_pos.x, scr_pos.y, scr_pos.z);
}
void backwards_render(float dist, float sex, float sey)
{
    sex -= s_w/2;
    float s_x_scale = sex * 2./s_w * v_w;
    sey -= s_h/2;
    float s_y_scale = sey * 2./s_h * v_h;
    
    Point3d p = cam_dir;
    p.x += s_x.x * s_x_scale;
    p.y += s_x.y * s_x_scale;
    p.z += s_x.z * s_x_scale;
    
    p.x += s_y.x * s_y_scale;
    p.y += s_y.y * s_y_scale;
    p.z += s_y.z * s_y_scale;
    
    p.x *= dist;
    p.y *= dist;
    p.z *= dist;
    
    p.x += initial_cam_pos.x;
    p.y += initial_cam_pos.y;
    p.z += initial_cam_pos.z;
    
    static int n=0;
    if(n<point_count) dists[n]=dist; 
    if(n<point_count) points[n++]=p;
    if(n==point_count) //not tested idc
    {        
        printf("resizing points array\n");
        point_count*=2;
        
        points = realloc(points, point_count*sizeof(Point3d));
        rects = realloc(rects, sizeof(SDL_Rect)*point_count);
        dists = realloc(dists, sizeof(float)*point_count);
    }
}

void change_dist_exponent(float exp)
{
    static float old_exp=1;
    for(int i=0; i < point_count; ++i)
    {
        points[i].x -= initial_cam_pos.x;
        points[i].y -= initial_cam_pos.y;
        points[i].z -= initial_cam_pos.z;
        //this is really stupid
        // but i dont want to make 2 copies of the arrays
        points[i].x /= powf(dists[i], old_exp);
        points[i].y /= powf(dists[i], old_exp);
        points[i].z /= powf(dists[i], old_exp);
        
        points[i].x *= powf(dists[i], exp);
        points[i].y *= powf(dists[i], exp);
        points[i].z *= powf(dists[i], exp);
        
        points[i].x += initial_cam_pos.x;
        points[i].y += initial_cam_pos.y;
        points[i].z += initial_cam_pos.z;
    }
    old_exp=exp;
}

void save_points()
{
    size_t size = 64+sizeof(Point3d)*point_count+sizeof(float)*point_count;
    char* p = malloc(size); // i guess instead of creating this one big chunk of memory we can use write and the and then append operations to write this data and 2 arrays but im not gonna update it now bruh
    memset(p, '.', 64);
    sprintf(p,"%d\n", point_count);
    memcpy(p+64                            , points, sizeof(Point3d)*point_count);
    memcpy(p+64+sizeof(Point3d)*point_count, dists, sizeof(float)*point_count);
    SDL_SaveFile("points_array.txt", p, size);
    free(p);
}

void reset_points()
{
    for(int i=0; i < point_count; ++i)
    {
        points[i].x=0;
        points[i].y=0;
        points[i].z=0;
    }
}
    
void load_points()
{
    size_t data_size;
    char* p = SDL_LoadFile("done_points_array_.txt", &data_size);
    point_count=atoi(p);
    //printf("loaded point_count == %d\n",point_count);
    memcpy(points, p+64,                             sizeof(Point3d)*point_count);
    memcpy(dists,  p+64+sizeof(Point3d)*point_count, sizeof(float)*point_count);
    SDL_free(p);
}