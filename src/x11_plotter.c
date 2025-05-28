// x11_plotter.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Para XSetStandardProperties
#include <X11/Xlib.h>
#include <X11/Xutil.h> // Para XLookupString, XWMHints
#include "x11_plotter.h"
// data_loader.h já é incluído por x11_plotter.h se precisar de DataSet aqui

#define PADDING 50      // Espaçamento das bordas da janela para o gráfico
#define POINT_RADIUS 2  // Raio dos pontos desenhados


// Nomes de cores correspondentes para X11
// Certifique-se de que a ordem corresponde ao enum ClusterColor
const char* CLUSTER_COLOR_NAMES[NUM_CLUSTER_COLORS] = {
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
    "LimeGreen", // Nome X11 para verde limão
    "navy"
};


// Função auxiliar para mapear coordenadas do dataset para coordenadas da tela
static void map_data_to_screen_coords(const DataPoint* p, const DataSet* ds,
                                      int window_width, int window_height,
                                      int* screen_x, int* screen_y) {
    double range_d1 = ds->max_d1 - ds->min_d1;
    double range_d2 = ds->max_d2 - ds->min_d2;
    
    // Evitar divisão por zero se todos os pontos tiverem a mesma coordenada
    if (range_d1 == 0) range_d1 = 1.0; // Evita que todos os pontos fiquem na borda
    if (range_d2 == 0) range_d2 = 1.0;
    
    // Mapeia d1 para x na tela
    *screen_x = PADDING + (int)(((p->d1 - ds->min_d1) / range_d1) * (window_width - 2 * PADDING));
    
    // Mapeia d2 para y na tela. Y em X11 cresce para baixo, então invertemos
    // para que valores maiores de d2 fiquem mais acima na tela.
    *screen_y = (window_height - PADDING) - (int)(((p->d2 - ds->min_d2) / range_d2) * (window_height - 2 * PADDING));
}


X11Context* init_x11(const char* window_title, int width, int height) {
    X11Context* context = (X11Context*)malloc(sizeof(X11Context));
    if (!context) {
        perror("Falha ao alocar X11Context");
        return NULL;
    }
    
    context->display = XOpenDisplay(NULL);
    if (context->display == NULL) {
        fprintf(stderr, "Não foi possível abrir o display X11\n");
        free(context);
        return NULL;
    }
    
    context->screen = DefaultScreen(context->display);
    context->black_pixel = BlackPixel(context->display, context->screen);
    context->white_pixel = WhitePixel(context->display, context->screen);
    
    // Tentar alocar uma cor para os pontos (ex: azul)
    
    
    context->window = XCreateSimpleWindow(context->display, RootWindow(context->display, context->screen),
                                          100, 100, width, height, 1, // x, y, w, h, border_width
                                          context->black_pixel, context->white_pixel); // border, background
    
    // Definir propriedades da janela (título)
    XStoreName(context->display, context->window, window_title);
    XSetStandardProperties(context->display, context->window, window_title, NULL, None, NULL, 0, NULL);
    
    
    // Para poder fechar a janela pelo "X" do gerenciador de janelas
    Atom wm_delete_window = XInternAtom(context->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(context->display, context->window, &wm_delete_window, 1);
    
    context->gc = XCreateGC(context->display, context->window, 0, NULL);
    XSetBackground(context->display, context->gc, context->white_pixel);
    XSetForeground(context->display, context->gc, context->black_pixel); // Cor padrão para desenho
    
    // Selecionar os tipos de eventos que queremos receber
    XSelectInput(context->display, context->window, ExposureMask | KeyPressMask | StructureNotifyMask);
    
    XMapWindow(context->display, context->window); // Torna a janela visível
    
    // É uma boa prática esperar pelo evento MapNotify para garantir que a janela está visível
    // antes de tentar desenhar nela pela primeira vez, embora o Expose event também sirva.
    XEvent event;
    do {
        XNextEvent(context->display, &event);
    } while (event.type != MapNotify && event.type != Expose); // Expose também é aceitável aqui
    // Se foi Expose, precisamos tratá-lo no loop principal ou aqui.
    
    context->width = width;
    context->height = height;
    
    return context;
}

void draw_points_on_expose(X11Context* x_context, const DataSet* dataset) {
    if (!x_context || !dataset || dataset->count == 0) return;
    
    // Limpar a janela (preencher com a cor de fundo - branca)
    XSetForeground(x_context->display, x_context->gc, x_context->white_pixel);
    XFillRectangle(x_context->display, x_context->window, x_context->gc, 0, 0, x_context->width, x_context->height);
    
    for (int i = 0; i < dataset->count; i++) {
        int sx, sy;
        map_data_to_screen_coords(&dataset->points[i], dataset,
                                  x_context->width, x_context->height, &sx, &sy);
        
        int cluster_id = dataset->points[i].cluster_id;
        unsigned long current_point_color_pixel;
        
        // Mapear cluster_id para um índice de cor válido
        // Se cluster_id puder ser negativo ou muito grande, use o módulo
        // Se cluster_id já estiver no intervalo [0, NUM_CLUSTER_COLORS-1], o módulo não é estritamente necessário,
        // mas é uma boa proteção.
        if (cluster_id >= 0 && cluster_id < NUM_CLUSTER_COLORS) {
            current_point_color_pixel = x_context->cluster_color_pixels[cluster_id];
        } else {
            // Fallback para uma cor padrão (ex: preto) se cluster_id estiver fora do intervalo
            fprintf(stderr, "Aviso: cluster_id %d para o ponto %wd está fora do intervalo [0, %d). Usando preto.\n",
                    cluster_id, i, NUM_CLUSTER_COLORS);
            current_point_color_pixel = x_context->cluster_color_pixels[CLUSTER_COLOR_RED];
            // Alternativamente, você pode usar o operador módulo para "envolver" o id:
            // int color_index = (cluster_id % NUM_CLUSTER_COLORS + NUM_CLUSTER_COLORS) % NUM_CLUSTER_COLORS; // Garante positivo
            // current_point_color_pixel = x_context->cluster_color_pixels[color_index];
        }
        
        XSetForeground(x_context->display, x_context->gc, current_point_color_pixel);
        
        // Desenhar um pequeno círculo preenchido para cada ponto
        XFillArc(x_context->display, x_context->window, x_context->gc,
                 sx - POINT_RADIUS, sy - POINT_RADIUS, // x, y do canto do retângulo delimitador
                 2 * POINT_RADIUS, 2 * POINT_RADIUS,   // width, height do retângulo
                 0, 360 * 64);                         // angle1, angle2 (0 a 360 graus)
    }
    XFlush(x_context->display); // Garante que o desenho seja enviado ao servidor X
}

void run_x11_event_loop(X11Context* x_context, const DataSet* dataset) {
    XEvent event;
    KeySym key;
    char buffer[10]; // Buffer para XLookupString
    
    while (1) {
        XNextEvent(x_context->display, &event);
        
        switch (event.type) {
            case Expose:
            // Redesenhar a janela se ela foi exposta (e.g., descoberta)
            // event.xexpose.count == 0 significa que não há mais eventos Expose na fila
            if (event.xexpose.count == 0) {
                draw_points_on_expose(x_context, dataset);
            }
            break;
            
            case KeyPress:
            // Sair se 'q' ou 'Q' for pressionado
            XLookupString(&event.xkey, buffer, sizeof(buffer), &key, NULL);
            if (key == XK_q || key == XK_Q) {
                return; // Sai do loop, encerrando a visualização
            }
            break;
            
            case ConfigureNotify: // A janela foi redimensionada, movida, etc.
            {
                XConfigureEvent xce = event.xconfigure;
                // Atualizar dimensões se a janela foi redimensionada
                if (xce.width != x_context->width || xce.height != x_context->height) {
                    x_context->width = xce.width;
                    x_context->height = xce.height;
                    // Um evento Expose geralmente se seguirá, então não precisa redesenhar aqui
                    // a menos que queira uma resposta mais imediata ao redimensionamento.
                    // Se quiser forçar: draw_points_on_expose(x_context, dataset);
                }
            }
            break;
            
            case ClientMessage: // Para lidar com o botão de fechar da janela (WM_DELETE_WINDOW)
            {
                Atom wm_delete_window = XInternAtom(x_context->display, "WM_DELETE_WINDOW", False);
                if (event.xclient.message_type == XInternAtom(x_context->display, "WM_PROTOCOLS", False) &&
                    (Atom)event.xclient.data.l[0] == wm_delete_window) {
                    return; // Sai do loop
                }
            }
            break;
            
            default:
            // Ignorar outros eventos
            break;
        }
    }
}

void close_x11(X11Context* x_context) {
    if (x_context) {
        if (x_context->gc) XFreeGC(x_context->display, x_context->gc);
        // A janela é destruída quando o display é fechado, mas pode-se fazer explicitamente:
        // if (x_context->window) XDestroyWindow(x_context->display, x_context->window);
        if (x_context->display) XCloseDisplay(x_context->display);
        free(x_context);
    }
}

void initialize_cluster_colors(X11Context* x_context) {
    if (!x_context || !x_context->display) return;
    
    Colormap colormap = DefaultColormap(x_context->display, DefaultScreen(x_context->display));
    XColor screen_color, exact_color;
    
    for (int i = 0; i < NUM_CLUSTER_COLORS; i++) {
        if (XAllocNamedColor(x_context->display, colormap, CLUSTER_COLOR_NAMES[i], &screen_color, &exact_color)) {
            x_context->cluster_color_pixels[i] = screen_color.pixel;
        } else {
            fprintf(stderr, "Aviso: Não foi possível alocar a cor '%s'. Usando preto.\n", CLUSTER_COLOR_NAMES[i]);
            // Fallback para preto se a cor não puder ser alocada
            // Assumindo que black_pixel já foi inicializado
            if (i == CLUSTER_COLOR_RED && x_context->black_pixel) { // Se o próprio preto falhar, temos um problema maior
                x_context->cluster_color_pixels[i] = x_context->black_pixel;
            } else if (x_context->black_pixel) { // Use preto como fallback
                x_context->cluster_color_pixels[i] = x_context->black_pixel;
            } else {
                // Se black_pixel não estiver disponível, use o primeiro que conseguiu (ou WhitePixel)
                // Esta é uma situação de erro mais séria.
                x_context->cluster_color_pixels[i] = WhitePixel(x_context->display, DefaultScreen(x_context->display));
                fprintf(stderr, "Erro crítico: black_pixel não inicializado e falha ao alocar %s. Usando branco.\n", CLUSTER_COLOR_NAMES[i]);
            }
        }
    }
    // Se você não inicializou black_pixel e white_pixel antes, pode fazer aqui:
    // x_context->black_pixel = BlackPixel(x_context->display, DefaultScreen(x_context->display));
    // x_context->white_pixel = WhitePixel(x_context->display, DefaultScreen(x_context->display));
    // E garantir que cluster_color_pixels[CLUSTER_COLOR_BLACK] seja black_pixel.
    // A maneira mais segura é inicializar black_pixel e white_pixel antes de chamar esta função.
}