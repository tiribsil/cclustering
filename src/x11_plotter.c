// x11_plotter.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "x11_plotter.h"

#define PADDING 50
#define POINT_RADIUS 2

const char* CLUSTER_COLOR_NAMES[NUM_CLUSTER_COLORS] = {
    "black",
    "red",
    "green",
    "blue",
    "yellow",
    "magenta",
    "cyan",
    "orange",
    "brown",
    "pink",
    "gray",
    "LimeGreen",
    "navy"
};

static void map_data_to_screen_coords(const DataPoint* p, const DataSet* ds,
                                      int window_width, int window_height,
                                      int* screen_x, int* screen_y){
    double data_range_d1 = ds->max_d1 - ds->min_d1;
    double data_range_d2 = ds->max_d2 - ds->min_d2;
    
    if(data_range_d1 == 0) data_range_d1 = 1;
    if(data_range_d2 == 0) data_range_d2 = 1;
    
    double drawable_width = window_width - 2 * PADDING;
    double drawable_height = window_height - 2 * PADDING;
    
    if(drawable_width <= 0) drawable_width = 1;
    if(drawable_height <= 0) drawable_height = 1;
    
    double scale;
    double final_plot_width;
    double final_plot_height;
    double offset_x = PADDING;
    double offset_y = PADDING;
    
    if(drawable_width / data_range_d1 < drawable_height / data_range_d2){
        scale = drawable_width / data_range_d1;
        final_plot_width = drawable_width;
        final_plot_height = data_range_d2 * scale;
        offset_y +=(drawable_height - final_plot_height) / 2.0;
    } else {
        scale = drawable_height / data_range_d2;
        final_plot_height = drawable_height;
        final_plot_width = data_range_d1 * scale;
        offset_x +=(drawable_width - final_plot_width) / 2.0;
    }
    
    *screen_x =(int)(offset_x +(p->d1 - ds->min_d1) * scale);
    *screen_y =(int)(offset_y + final_plot_height -(p->d2 - ds->min_d2) * scale);
}

X11Context* init_x11(const char* window_title, int width, int height){
    X11Context* context =(X11Context*)malloc(sizeof(X11Context));
    if(!context){
        perror("Falha ao alocar X11Context");
        return NULL;
    }
    
    context->display = XOpenDisplay(NULL);
    if(context->display == NULL){
        fprintf(stderr, "Não foi possível abrir o display X11\n");
        free(context);
        return NULL;
    }
    
    context->screen = DefaultScreen(context->display);
    context->black_pixel = BlackPixel(context->display, context->screen);
    context->white_pixel = WhitePixel(context->display, context->screen);
    
    context->window = XCreateSimpleWindow(context->display, RootWindow(context->display, context->screen),
                                          100, 100, width, height, 1,
                                          context->black_pixel, context->white_pixel);
    
    XStoreName(context->display, context->window, window_title);
    XSetStandardProperties(context->display, context->window, window_title, NULL, None, NULL, 0, NULL);
    
    Atom wm_delete_window = XInternAtom(context->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(context->display, context->window, &wm_delete_window, 1);
    
    context->gc = XCreateGC(context->display, context->window, 0, NULL);
    XSetBackground(context->display, context->gc, context->white_pixel);
    XSetForeground(context->display, context->gc, context->black_pixel);
    
    XSelectInput(context->display, context->window, ExposureMask | KeyPressMask | StructureNotifyMask);
    
    XMapWindow(context->display, context->window);
    
    XEvent event;
    do{
        XNextEvent(context->display, &event);
    } while(event.type != MapNotify && event.type != Expose); 
    
    context->width = width;
    context->height = height;
    
    return context;
}

void draw_points_on_expose(X11Context* x_context, const DataSet* dataset){
    if(!x_context || !dataset || !dataset->count) return;
    
    XSetForeground(x_context->display, x_context->gc, x_context->white_pixel);
    XFillRectangle(x_context->display, x_context->window, x_context->gc, 0, 0, x_context->width, x_context->height);
    
    for(int i = 0; i < dataset->count; i++){
        int sx, sy;
        map_data_to_screen_coords(&dataset->points[i], dataset,
                                  x_context->width, x_context->height, &sx, &sy);
        
        int cluster_id = dataset->points[i].cluster_id;
        unsigned long current_point_color_pixel;
        
        if(cluster_id >= 0 && cluster_id < NUM_CLUSTER_COLORS){
            current_point_color_pixel = x_context->cluster_color_pixels[cluster_id + 1];
        } else{
            fprintf(stderr, "Aviso: cluster_id %d para o ponto %d está fora do intervalo [0, %d). Usando preto.\n",
                    cluster_id, i, NUM_CLUSTER_COLORS);
            current_point_color_pixel = x_context->cluster_color_pixels[CLUSTER_COLOR_BLACK];
        }
        
        XSetForeground(x_context->display, x_context->gc, current_point_color_pixel);
        XFillArc(x_context->display, x_context->window, x_context->gc,
                 sx - POINT_RADIUS, sy - POINT_RADIUS,
                 2 * POINT_RADIUS, 2 * POINT_RADIUS,
                 0, 360 * 64);
    }
    XFlush(x_context->display);
}

void run_x11_event_loop(X11Context* x_context, const DataSet* dataset){
    XEvent event;
    KeySym key;
    char buffer[10];
    
    while(1){
        XNextEvent(x_context->display, &event);
        
        switch(event.type){
            case Expose:
            if(!event.xexpose.count){
                draw_points_on_expose(x_context, dataset);
            }
            break;
            
            case KeyPress:
            XLookupString(&event.xkey, buffer, sizeof(buffer), &key, NULL);
            if(key == XK_q || key == XK_Q){
                return;
            }
            break;
            
            case ConfigureNotify:
            {
                XConfigureEvent xce = event.xconfigure;
                if(xce.width != x_context->width || xce.height != x_context->height){
                    x_context->width = xce.width;
                    x_context->height = xce.height;
                }
            }
            break;
            
            case ClientMessage:
            {
                Atom wm_delete_window = XInternAtom(x_context->display, "WM_DELETE_WINDOW", False);
                if(event.xclient.message_type == XInternAtom(x_context->display, "WM_PROTOCOLS", False) &&
                   (Atom)event.xclient.data.l[0] == wm_delete_window){
                    return;
                }
            }
            break;
            
            default:
            break;
        }
    }
}

void close_x11(X11Context* x_context){
    if(!x_context) return;
    
    if(x_context->gc) XFreeGC(x_context->display, x_context->gc);
    if(x_context->display) XCloseDisplay(x_context->display);
    free(x_context);
}

void initialize_cluster_colors(X11Context* x_context){
    if(!x_context || !x_context->display) return;
    
    Colormap colormap = DefaultColormap(x_context->display, DefaultScreen(x_context->display));
    XColor screen_color, exact_color;
    
    for(int i = 0; i < NUM_CLUSTER_COLORS; i++){
        if(XAllocNamedColor(x_context->display, colormap, CLUSTER_COLOR_NAMES[i], &screen_color, &exact_color)){
            x_context->cluster_color_pixels[i] = screen_color.pixel;
        } else{
            fprintf(stderr, "Aviso: Não foi possível alocar a cor '%s'. Usando preto.\n", CLUSTER_COLOR_NAMES[i]);
            if(i == CLUSTER_COLOR_BLACK && x_context->black_pixel){
                x_context->cluster_color_pixels[i] = x_context->black_pixel;
            } else if(x_context->black_pixel){
                x_context->cluster_color_pixels[i] = x_context->black_pixel;
            } else{
                x_context->cluster_color_pixels[i] = WhitePixel(x_context->display, DefaultScreen(x_context->display));
                fprintf(stderr, "Erro crítico: black_pixel não inicializado e falha ao alocar %s. Usando branco.\n", CLUSTER_COLOR_NAMES[i]);
            }
        }
    }
}