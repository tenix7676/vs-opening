#include "SDL3/SDL.h"
#include <math.h>
#include <stdlib.h>
#define M_PI        3.14159265358979323846
#include "3d.h"
#include "oklab.h"

void update_scr_pos();
void update_cam_pos();
void update_s_y_s_x(void);
void normalise(Point3d*);

int cam_dist;
Point3d cam_dir;
Point3d cam_pos;
Point3d scr_pos; 
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
SDL_Rect* rects = NULL;

void init_3d(int s_w_, int s_h_)
{
    s_h=s_h_;s_w=s_w_;
    v_w=1;
    v_h=(float)s_h/s_w*v_w;
    cam_dist=v_w*55/34.5;
    cam_dir=(Point3d){-1,0,-.25};
    normalise(&cam_dir);
    scr_pos=(Point3d){1,0,1};
    update_cam_pos(scr_pos);
    update_s_y_s_x();
    
    init_points();
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
    a->x /= len;
    a->y /= len;
    a->z /= len;
}
void update_s_y_s_x()
{
    s_x=(Point3d){cam_dir.y, -cam_dir.x, 0};
    normalise(&s_x);
    Point3d cam_dir_xy={cam_dir.x, cam_dir.y, 0};
    float len_cam_dir_xy=hypotf(cam_dir.x, cam_dir.y);
    normalise(&cam_dir_xy);
    s_y.x=cam_dir.z * cam_dir_xy.x;
    s_y.y=cam_dir.z * cam_dir_xy.y;
    s_y.z=-len_cam_dir_xy;
    normalise(&s_y);
}

float f(float x)
{
    return expf(-pow(4*x,2));
}
void init_points()
{
    
    int n=0;
    float s=1;
    float r1=0;
    float diff=1;
    float rn=7;
    float R=4;
    point_count=0;
    for(float r=r1; r<=rn; r+=diff)
    {
        int points_around=2*M_PI*r/s + 1;
        point_count+=points_around;
    }
    points = malloc(point_count*sizeof(Point3d));
    rects = malloc(sizeof(SDL_Rect)*point_count);
    
    //for now 1 circle
    for(float r=r1; r<=rn; r+=diff)
    {
        int points_around=2*M_PI*r/s + 1;
        
        for(int i=-points_around/2; i<points_around-points_around/2; ++i)
        {
        Point3d p;
        float delta_t=s / r;
        float t=i*delta_t;
        if(r==0) t=0;
        p.x=r*cosf(t);
        p.y=r*sinf(t);
        p.z=0;
        points[n]=p;
        n++;
        }
    }
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
#define DIST_SCALING 32
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
            float rect_radius=DIST_SCALING/len_p1;
            const SDL_Rect rect=(SDL_Rect){sex-rect_radius,sey-rect_radius,fmax(1,rect_radius),fmax(1,rect_radius)};
            SDL_FillSurfaceRect(surface, &rect, *(Uint32*)(void*)&color);
        }
    }
}

void draw_screen_one_color(SDL_Surface* surface)
{
    //Just in case
    normalise(&cam_dir);
    normalise(&s_x);
    normalise(&s_y);
    int num_rects_this_frame=0;
    for(size_t i=0; i < point_count; ++i)
    {
        int sex, sey;
        float len_p1 = check_point(points+i,&sex,&sey);
        if(len_p1 > 0)
        {
            float rect_radius=DIST_SCALING/len_p1;//powf(len_p1,.85);
            const SDL_Rect rect=(SDL_Rect){sex-rect_radius,sey-rect_radius,fmax(1,rect_radius),fmax(1,rect_radius)};
            rects[num_rects_this_frame] = rect;
            num_rects_this_frame++;
        }
    }
    SDL_Color color = (SDL_Color){ 255, 255, 255 };
    SDL_FillSurfaceRects(surface, rects, num_rects_this_frame, *(Uint32*)(void*)&color);
}
void turn_camera(float left_right,float up_down)
{
    //its annoying to look straight down, propably could fix it with some if
    const float turning_speed=-1/256.;
    Point3d cam_dir_xy=(Point3d){cam_dir.x,cam_dir.y,0};
    float len_cam_dir_xy=hypotf(cam_dir.x,cam_dir.y);
    float radius=hypotf(cam_dir.z, len_cam_dir_xy);
    float ang0=atan2f(cam_dir.z,len_cam_dir_xy);
    float ang1=ang0+v_h/v_w*turning_speed*up_down;
    cam_dir.z=radius*sinf(ang1);
    normalise(&cam_dir_xy);
    cam_dir.x=radius*cosf(ang1)*cam_dir_xy.x;
    cam_dir.y=radius*cosf(ang1)*cam_dir_xy.y;
    
    ang0=atan2f(cam_dir.y,cam_dir.x);
    ang1=ang0+turning_speed*left_right;
    cam_dir.y=len_cam_dir_xy*sinf(ang1);
    cam_dir.x=len_cam_dir_xy*cosf(ang1);
    
    update_s_y_s_x(); //guess i just forgor this lol
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