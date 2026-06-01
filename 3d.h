#ifndef _3d_h
#define _3dh_h
#include "SDL3/SDL.h"

typedef struct Point3d
{
    float x,y,z;
} Point3d;

void update_s_y_s_x(void);

Point3d cam_pos;
Point3d scr_pos;
int s_h,s_w;
float v_w, v_h;
Point3d cam_dir;
int cam_dist;
Point3d s_y, s_x;

void init_3d(int s_w_, int s_h_);
void init_points();
void update_points(Point3d* points, size_t point_count);
void draw_screen(SDL_Surface* surface);
void draw_screen_one_color(SDL_Surface* surface);
void turn_camera(float left_right,float up_down);
void move_camera(float dist);
void fly_camera(float dist);
void strafe_camera(float dist);
void print_scr_pos();
void backwards_render(float dist, float sex, float sey);
void save_points();
void change_dist_exponent(float exp);
#endif