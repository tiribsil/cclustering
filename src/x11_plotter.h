/* date = May 27th 2025 10:08 am */
#ifndef X11_PLOTTER_H
#define X11_PLOTTER_H

#include "data_loader.h" // Para usar DataSet
#include <X11/Xlib.h>     // Tipos básicos do X11

#define NUM_CLUSTER_COLORS 13

// Estrutura para manter o contexto X11
typedef struct {
    Display *display;
    Window window;
    GC gc; // Graphics Context
    int screen;
    unsigned long black_pixel;
    unsigned long white_pixel;
    unsigned long cluster_color_pixels[NUM_CLUSTER_COLORS];
    int width;  // Largura atual da janela
    int height; // Altura atual da janela
} X11Context;

// Novo: Enum para as cores do cluster
typedef enum {
    CLUSTER_COLOR_BLACK = 0,
    CLUSTER_COLOR_RED,
    CLUSTER_COLOR_GREEN,
    CLUSTER_COLOR_BLUE,
    CLUSTER_COLOR_YELLOW,
    CLUSTER_COLOR_MAGENTA, // Roxo
    CLUSTER_COLOR_CYAN,    // Azul claro
    CLUSTER_COLOR_ORANGE,
    CLUSTER_COLOR_BROWN,
    CLUSTER_COLOR_PINK,
    CLUSTER_COLOR_GRAY,
    CLUSTER_COLOR_LIME,    // Verde limão
    CLUSTER_COLOR_NAVY     // Azul marinho
        // Adicione mais se NUM_CLUSTER_COLORS aumentar
} ClusterColor;

/**
 * @brief Inicializa a conexão com o servidor X e cria uma janela.
 * @param window_title Título da janela.
 * @param width Largura inicial da janela.
 * s@param height Altura inicial da janela.
 * @return Ponteiro para X11Context, ou NULL em caso de falha.
 */
X11Context* init_x11(const char* window_title, int width, int height);

/**
 * @brief Desenha os pontos do DataSet na janela X11.
 * Esta função deve ser chamada quando um evento Expose ocorre.
 * @param x_context O contexto X11.
 * @param dataset O conjunto de dados a ser plotado.
 */
void draw_points_on_expose(X11Context* x_context, const DataSet* dataset);


/**
 * @brief Inicia o loop de eventos X11.
 * Mantém a janela aberta, lida com redimensionamento, redesenho e entrada do teclado.
 * @param x_context O contexto X11.
 * @param dataset O conjunto de dados (usado para redesenhar).
 */
void run_x11_event_loop(X11Context* x_context, const DataSet* dataset);

/**
 * @brief Fecha a conexão com o servidor X e libera recursos.
 * @param x_context O contexto X11 a ser fechado.
 */
void close_x11(X11Context* x_context);

// Protótipo da função auxiliar para inicializar cores (você precisará chamá-la)
void initialize_cluster_colors(X11Context* x_context);

#endif // X11_PLOTTER_H