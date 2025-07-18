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
    
    // ------------------------ <<< PROGRAMA PRINCIPAL >>> ------------------------
    DataSet* dataset = 0;
    double ari = 1.0;
    
    char* filename_start = (char*)data_filename + strlen(data_filename);
    while(*--filename_start != '/');
    filename_start++;
    
    char chosen_file[1 << 6];
    strcpy(chosen_file, filename_start);
    
    char* extension_start = chosen_file + strlen(chosen_file);
    while(*--extension_start != '.');
    *extension_start = 0;
    
    if(!strcmp(data_filename + strlen(data_filename) - 3, "clu")){
        char dataset_path[1 << 8];
        sprintf(dataset_path, "../data/%s.txt", chosen_file);
        dataset = load_data_from_file(dataset_path);
        
        int* real_clusters = load_clusters(data_filename, dataset->count);
        
        for(int i = 0; i < dataset->count; i++)
            dataset->points[i].cluster_id = real_clusters[i];
    }
    else {
        printf("Carregando dados de: %s\n", data_filename);
        dataset = load_data_from_file(data_filename);
        
        if(!dataset){
            fprintf(stderr, "Falha ao carregar os dados. Encerrando.\n");
            return EXIT_FAILURE;
        }
        
        if(!dataset->count){
            fprintf(stderr, "Nenhum ponto de dado para exibir. Encerrando.\n");
            free_dataset(dataset);
            return EXIT_SUCCESS;
        }
        
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
            write_clu(dataset, chosen_file, arg1, chosen_algorithm);
        }
        
        else if(chosen_algorithm == 2){
            for(int i = arg1; i <= arg2; i++){
                single_link(dataset, i);
                write_clu(dataset, chosen_file, i, chosen_algorithm);
            }
        }
        
        else{
            for(int i = arg1; i <= arg2; i++){
                complete_link(dataset, i);
                write_clu(dataset, chosen_file, i, chosen_algorithm);
            }
        }
        
        char ref_filename[1 << 8];
        char group_filename[1 << 8];
        
        snprintf(ref_filename, 1 << 8, "../data/resultados/%s.clu", chosen_file);
        printf("Carregando clusters de referência de %s...\n", ref_filename);
        int* clusters_ref = load_clusters(ref_filename, dataset->count);
        
        if(!is_link) arg2 = arg1;
        for(int i = arg1; i <= arg2; i++){
            snprintf(group_filename, 1 << 8, "../data/resultados/G1_%s_%d_%d.clu", chosen_file, chosen_algorithm, i);
            
            printf("Carregando clusters produzidos...\n");
            int* clusters_prod = load_clusters(group_filename, dataset->count);
            
            if (clusters_ref && clusters_prod) {
                ari = adjusted_rand_index(clusters_prod, clusters_ref, dataset->count);
                printf("Índice Rand Ajustado (ARI) calculado para k = %d: %f\n", i, ari);
                
                free(clusters_prod);
            } else {
                printf("Não foi possível carregar os clusters de referência. O ARI não será calculado.\n");
            }
        }
        
        free_clusters(clusters_ref);
    }
    
    
    printf("Inicializando X11 para visualização...\n");
    char window_title[128];
    snprintf(window_title, 128, "Visualizador de Dados: %s", data_filename);
    
    X11Context* x_context = init_x11(window_title, INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT);
    if(!x_context){
        fprintf(stderr, "Falha ao inicializar o X11. Encerrando.\n");
        return EXIT_FAILURE;
    }
    
    initialize_cluster_colors(x_context);
    
    
    printf("Exibindo dados. Pressione 'q' na janela para sair.\n");
    run_x11_event_loop(x_context, dataset, ari);
    
    printf("Fechando X11 e liberando recursos...\n");
    close_x11(x_context);
    free_dataset(dataset);
    
    printf("Programa finalizado com sucesso.\n");
    return EXIT_SUCCESS;
}
