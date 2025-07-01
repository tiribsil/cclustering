#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_loader.h"
#include "x11_plotter.h"
#include "clustering.h"

#define INITIAL_WINDOW_WIDTH 800
#define INITIAL_WINDOW_HEIGHT 600

int main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(stderr, "Uso: %s <arquivo_dados>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const char* data_filename = argv[1];
    
    printf("Carregando dados de: %s\n", data_filename);
    DataSet* dataset = load_data_from_file(data_filename);
    
    if(!dataset){
        fprintf(stderr, "Falha ao carregar os dados. Encerrando.\n");
        return EXIT_FAILURE;
    }
    
    if(!dataset->count){
        fprintf(stderr, "Nenhum ponto de dado para exibir. Encerrando.\n");
        free_dataset(dataset);
        return EXIT_SUCCESS;
    }
    
    printf("Inicializando X11 para visualização...\n");
    char window_title[128];
    snprintf(window_title, 128, "Visualizador de Dados: %s", data_filename);
    
    X11Context* x_context = init_x11(window_title, INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT);
    if(!x_context){
        fprintf(stderr, "Falha ao inicializar o X11. Encerrando.\n");
        free_dataset(dataset);
        return EXIT_FAILURE;
    }
    
    initialize_cluster_colors(x_context);
    
    // ------------------------ <<< PROGRAMA PRINCIPAL >>> ------------------------
    printf("Bem vindo(a) ao cclustering!\nEscolha o algoritmo desejado:\n");
    
    const char *message[2][2] = {
        {
            "Qual é o número de clusters (k) desejado?\n",
            "Qual é o número de clusters (k) mínimo desejado?\n"
        },
        {
            "Qual é o número máximo de iterações desejado?\n",
            "Qual é o número de clusters (k) máximo desejado?\n"
        }
    };
    
    
    int chosen_algorithm = 0;
    while(1){
        printf("1 - k-médias\n2 - single-link\n3 - complete-link\n");
        
        scanf("%d", &chosen_algorithm);
        if(chosen_algorithm >= 1 && chosen_algorithm <= 3) break;
        
        printf("Escolha uma opção válida.\n");
    }
    
    int is_link = chosen_algorithm > 1;
    
    int arg1 = 0, arg2 = 0;
    printf(message[0][is_link]);
    scanf("%d", &arg1);
    printf(message[1][is_link]);
    scanf("%d", &arg2);
    
    if(chosen_algorithm == 1){
        k_means(dataset, arg1, arg2);
        write_clu(dataset, (char*)data_filename);
    }
    
    else if(chosen_algorithm == 2){
        for(int i = arg1; i <= arg2; i++){
            single_link(dataset, i);
            write_clu(dataset, (char*)data_filename);
        }
    }
    
    else{
        for(int i = arg1; i <= arg2; i++){
            complete_link(dataset, i);
            write_clu(dataset, (char*)data_filename);
        }
    }
    
    int ref_choice = 0;
    char ref_filename[256];
    char group_filename[256];
    
    while(1){
        printf("\nEscolha um arquivo de resultado para comparar:\n");
        printf("1 - c2ds1-2sp.clu (Espiral)\n");
        printf("2 - c2ds3-2g.clu (Circulos)\n");
        printf("3 - monkey.clu (Macaco)\n");
        scanf("%d", &ref_choice);
        
        if(ref_choice >= 1 && ref_choice <= 3) break; 
        
        printf("Escolha inválida.\n");
    }
    
    const char* chosen_file;
    switch(ref_choice) {
        case 1: chosen_file = "c2ds1-2sp.clu"; break;
        case 2: chosen_file = "c2ds3-2g.clu"; break;
        case 3: chosen_file = "monkey.clu"; break;
    }
    
    snprintf(ref_filename, sizeof(ref_filename), "../data/resultados/%s", chosen_file);
    snprintf(group_filename, sizeof(group_filename), "../data/resultados/G1_%s", chosen_file);
    
    printf("Carregando clusters de referência de %s...\n", ref_filename);
    int* clusters_ref = load_clusters(ref_filename, dataset->count);
    printf("Carregando clusters produzidos...\n");
    int* clusters_prod = load_clusters(group_filename, dataset->count);
    double ari = 0.0;
    
    if (clusters_ref && clusters_prod) {
        ari = adjusted_rand_index(clusters_prod, clusters_ref, dataset->count);
        printf("Índice Rand Ajustado (ARI) calculado: %f\n", ari);
        
        free(clusters_prod);
    } else {
        printf("Não foi possível carregar os clusters de referência. O ARI não será calculado.\n");
    }
    
    printf("Exibindo dados. Pressione 'q' na janela para sair.\n");
    run_x11_event_loop(x_context, dataset, ari);
    
    printf("Fechando X11 e liberando recursos...\n");
    close_x11(x_context);
    free_dataset(dataset);
    free_clusters(clusters_ref);
    
    printf("Programa finalizado com sucesso.\n");
    return EXIT_SUCCESS;
}
