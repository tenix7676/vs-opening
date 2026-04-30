#ifndef OKLAB_H
#define OKLAB_H
typedef struct Lab {float L; float a; float b;} Lab;
typedef struct RGB {float r; float g; float b;} RGB;

Lab linear_srgb_to_oklab(RGB c);
RGB oklab_to_linear_srgb(Lab c);
#endif