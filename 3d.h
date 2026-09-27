#ifndef _3d_h
#define _3d_h
#include "SDL3/SDL.h"

typedef struct Point3d
{
    float x,y,z;
} Point3d;

void update_s_y_s_x(void);

extern Point3d cam_pos;
extern Point3d scr_pos;
extern int s_h,s_w;
extern float v_w, v_h;
extern Point3d cam_dir;
extern int cam_dist;
extern Point3d s_y, s_x;
extern Point3d cam_dir;
extern Point3d initial_cam_pos;
extern float cam_leftright;
extern float cam_updown;
// the closer i can get to a 3d point without it disappearing 
// (because it goes behind the screen)
// but i guess checking if a point goes behind the camera
// instead of if it goes behind the screen also works great
#define RENDER_IN_FRONT_OF_CAMERA
#ifndef RENDER_IN_FRONT_OF_CAMERA
#define RENDER_IN_FRONT_OF_SCREEN
#endif

void init_3d(int s_w_, int s_h_);
void init_points();
void update_points(Point3d* points, size_t point_count);
void draw_screen(SDL_Surface* surface);
void draw_screen_one_color(SDL_Surface* surface, SDL_Color color);
void turn_camera(float left_right,float up_down);
void move_camera(float dist);
void fly_camera(float dist);
void strafe_camera(float dist);
void print_scr_pos();
void backwards_render(float dist, float sex, float sey);
void save_points();
void reset_points();
void load_points();
void change_dist_exponent(float exp);
#endif
