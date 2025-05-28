#include <stdio.h>
#include <stdlib.h>
#include "data_loader.h"
#include "x11_plotter.h" // Este arquivo deve conter a declaração de initialize_cluster_colors
// e a definição atualizada de X11Context
#include "clustering.h"

#define INITIAL_WINDOW_WIDTH 800
#define INITIAL_WINDOW_HEIGHT 600

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo_dados.txt>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const char* data_filename = argv[1];
    
    printf("Carregando dados de: %s\n", data_filename);
    DataSet* dataset = load_data_from_file(data_filename);
    
    if(!dataset){
        fprintf(stderr, "Falha ao carregar os dados. Encerrando.\n");
        return EXIT_FAILURE;
    }
    
    print_dataset_summary(dataset); // Função para imprimir resumo do dataset (opcional, mas útil)
    
    if(!dataset->count){
        fprintf(stderr, "Nenhum ponto de dado para exibir. Encerrando.\n");
        free_dataset(dataset);
        return EXIT_SUCCESS;
    }
    
    printf("Inicializando X11 para visualização...\n");
    char window_title[128];
    snprintf(window_title, sizeof(window_title), "Visualizador de Dados: %s (Pressione 'q' para sair)", data_filename);
    
    X11Context* x_context = init_x11(window_title, INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT);
    if(!x_context){
        fprintf(stderr, "Falha ao inicializar o X11. Encerrando.\n");
        free_dataset(dataset);
        return EXIT_FAILURE;
    }
    
    // >>> NOVA LINHA: Inicializar as cores do cluster <<<
    // Esta função deve ser chamada APÓS init_x11 (que cria Display, GC, etc.)
    // e ANTES de qualquer desenho que dependa dessas cores.
    // Assume-se que init_x11() já inicializou x_context->black_pixel e x_context->white_pixel.
    initialize_cluster_colors(x_context);
    // >>> FIM DA NOVA LINHA <<<
    
    // ------------------------ <<< CLUSTERING AQUI >>> ------------------------
    k_means(dataset, 2);
    
    printf("Exibindo dados. Pressione 'q' na janela para sair.\n");
    // O primeiro desenho ocorrerá devido ao evento Expose após XMapWindow e o loop de eventos.
    run_x11_event_loop(x_context, dataset);
    
    printf("Fechando X11 e liberando recursos...\n");
    close_x11(x_context);
    free_dataset(dataset);
    
    printf("Programa finalizado com sucesso.\n");
    return EXIT_SUCCESS;
}