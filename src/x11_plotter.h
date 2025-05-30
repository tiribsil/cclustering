/* date = May 27th 2025 10:08 am */
#ifndef X11_PLOTTER_H
#define X11_PLOTTER_H

#include "data_loader.h"
#include <X11/Xlib.h>

#define NUM_CLUSTER_COLORS 13

typedef struct {
    Display *display;
    Window window;
    GC gc;
    int screen;
    unsigned long black_pixel;
    unsigned long white_pixel;
    unsigned long cluster_color_pixels[NUM_CLUSTER_COLORS];
    int width;
    int height;
} X11Context;

typedef enum {
    CLUSTER_COLOR_BLACK = 0,
    CLUSTER_COLOR_RED,
    CLUSTER_COLOR_GREEN,
    CLUSTER_COLOR_BLUE,
    CLUSTER_COLOR_YELLOW,
    CLUSTER_COLOR_MAGENTA,
    CLUSTER_COLOR_CYAN,
    CLUSTER_COLOR_ORANGE,
    CLUSTER_COLOR_BROWN,
    CLUSTER_COLOR_PINK,
    CLUSTER_COLOR_GRAY,
    CLUSTER_COLOR_LIME,
    CLUSTER_COLOR_NAVY
} ClusterColor;

X11Context* init_x11(const char* window_title, int width, int height);

void draw_points_on_expose(X11Context* x_context, const DataSet* dataset);

void run_x11_event_loop(X11Context* x_context, const DataSet* dataset);

void close_x11(X11Context* x_context);

void initialize_cluster_colors(X11Context* x_context);

#endif // X11_PLOTTER_H