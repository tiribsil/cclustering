#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "data_loader.h"

#define INITIAL_DATASET_CAPACITY 100
#define LINE_BUFFER_SIZE 256

DataSet* create_dataset(int initial_capacity){
    DataSet* ds =(DataSet*)malloc(sizeof(DataSet));
    if(!ds){
        perror("Falha ao alocar DataSet");
        return 0;
    }
    ds->points =(DataPoint*)malloc(initial_capacity * sizeof(DataPoint));
    if(!ds->points){
        perror("Falha ao alocar DataPoints iniciais");
        free(ds);
        return 0;
    }
    ds->count = 0;
    ds->capacity = initial_capacity;
    ds->min_d1 = DBL_MAX;
    ds->max_d1 = -DBL_MAX;
    ds->min_d2 = DBL_MAX;
    ds->max_d2 = -DBL_MAX;
    return ds;
}

static int add_point(DataSet* dataset, const char* label, double d1, double d2){
    if(dataset->count >= dataset->capacity){
        int new_capacity = dataset->capacity << 1;
        if(new_capacity == 0) new_capacity = INITIAL_DATASET_CAPACITY; 
        DataPoint* new_points =(DataPoint*)realloc(dataset->points, new_capacity * sizeof(DataPoint));
        if(!new_points){
            perror("Falha ao realocar DataPoints");
            return 0;
        }
        dataset->points = new_points;
        dataset->capacity = new_capacity;
    }
    
    strncpy(dataset->points[dataset->count].label, label, MAX_LABEL_LEN - 1);
    dataset->points[dataset->count].label[MAX_LABEL_LEN - 1] = 0;
    dataset->points[dataset->count].d1 = d1;
    dataset->points[dataset->count].d2 = d2;
    
    if(d1 < dataset->min_d1) dataset->min_d1 = d1;
    if(d1 > dataset->max_d1) dataset->max_d1 = d1;
    if(d2 < dataset->min_d2) dataset->min_d2 = d2;
    if(d2 > dataset->max_d2) dataset->max_d2 = d2;
    
    dataset->count++;
    return 1;
}

DataSet* load_data_from_file(const char* filename){
    FILE* file = fopen(filename, "r");
    if(!file){
        perror("Erro ao abrir arquivo de dados.");
        return 0;
    }
    
    DataSet* dataset = create_dataset(INITIAL_DATASET_CAPACITY);
    if(!dataset){
        fclose(file);
        return 0;
    }
    
    char line_buffer[LINE_BUFFER_SIZE];
    char label_buffer[MAX_LABEL_LEN];
    double d1_val, d2_val;
    
    if(fgets(line_buffer, sizeof(line_buffer), file) == 0){
        fprintf(stderr, "Erro ao ler cabeçalho ou arquivo vazio: %s\n", filename);
        free_dataset(dataset);
        fclose(file);
        return 0;
    }
    
    int line_num = 1;
    while(fgets(line_buffer, sizeof(line_buffer), file) != 0){
        line_num++;
        sscanf(line_buffer, "%49s\t%lf\t%lf", label_buffer, &d1_val, &d2_val);
        if(!add_point(dataset, label_buffer, d1_val, d2_val)){
            fprintf(stderr, "Falha ao adicionar ponto da linha %d do arquivo %s\n", line_num, filename);
            free_dataset(dataset);
            fclose(file);
            return 0;
        }
    }
    
    if(ferror(file)){
        perror("Erro durante a leitura do arquivo");
        free_dataset(dataset);
        fclose(file);
        return 0;
    }
    
    fclose(file);
    
    if(!dataset->count) fprintf(stderr, "Nenhum ponto de dado carregado de %s.\n", filename);
    
    return dataset;
}

void free_dataset(DataSet* dataset){
    if(!dataset) return;
    if(dataset->points) free(dataset->points);
    free(dataset);
}

int* load_clusters(const char* filename, int num_points) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Aviso: Não foi possível abrir o arquivo de clusters de referência '%s'.\n", filename);
        return NULL;
    }
    
    int* clusters = (int*)malloc(sizeof(int) * num_points);
    if (!clusters) {
        perror("Falha ao alocar memória para os clusters de referência");
        fclose(file);
        return NULL;
    }
    
    char line_buffer[LINE_BUFFER_SIZE];
    int point_index = 0;
    int cluster_id;
    
    while (fgets(line_buffer, sizeof(line_buffer), file) != NULL && point_index < num_points) {
        if (sscanf(line_buffer, "%*s\t%d", &cluster_id) == 1) {
            clusters[point_index] = cluster_id;
        } else {
            fprintf(stderr, "Aviso: Linha mal formatada no arquivo de referência: %s", line_buffer);
            clusters[point_index] = 0;
        }
        point_index++;
    }
    
    if (point_index < num_points) {
        fprintf(stderr, "Aviso: O arquivo de referência '%s' contém menos pontos (%d) do que o dataset (%d).\n", filename, point_index, num_points);
    }
    
    fclose(file);
    return clusters;
}

void free_clusters(int* clusters) {
    if (clusters) {
        free(clusters);
    }
}

void write_clu(DataSet* dataset, char* filename, int k, int chosen_algorithm){
    // Criando o arquivo:
    char file_path[1 << 8];
    sprintf(file_path, "../data/resultados/G1_%s_%d_%d.clu", filename, chosen_algorithm, k);
    
    FILE* file = fopen(file_path, "w");
    
    // Escrevendo no arquivo:
    char line[1 << 8] = {'\0'};
    for (int i = 0; i < dataset->count; i++) {
    	snprintf(line, sizeof(line), "%s\t%d", dataset->points[i].label, dataset->points[i].cluster_id);
    	fwrite(line, sizeof(char), strlen(line), file);
    	if (i != dataset->count-1) fprintf(file, "\n");
    }
    fclose(file);
    
    return;
}
