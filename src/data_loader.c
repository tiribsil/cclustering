// data_loader.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h> // Para DBL_MAX, -DBL_MAX
#include "data_loader.h"

#define INITIAL_DATASET_CAPACITY 100
#define LINE_BUFFER_SIZE 256

DataSet* create_dataset(int initial_capacity) {
    DataSet* ds = (DataSet*)malloc(sizeof(DataSet));
    if (!ds) {
        perror("Falha ao alocar DataSet");
        return NULL;
    }
    ds->points = (DataPoint*)malloc(initial_capacity * sizeof(DataPoint));
    if (!ds->points) {
        perror("Falha ao alocar DataPoints iniciais");
        free(ds);
        return NULL;
    }
    ds->count = 0;
    ds->capacity = initial_capacity;
    ds->min_d1 = DBL_MAX;
    ds->max_d1 = -DBL_MAX; // DBL_MIN é o menor valor positivo, queremos o "mais negativo"
    ds->min_d2 = DBL_MAX;
    ds->max_d2 = -DBL_MAX;
    return ds;
}

// Função auxiliar para adicionar um ponto e redimensionar se necessário
static int add_point(DataSet* dataset, const char* label, double d1, double d2) {
    if (dataset->count >= dataset->capacity) {
        int new_capacity = dataset->capacity * 2;
        if (new_capacity == 0) new_capacity = INITIAL_DATASET_CAPACITY; // Caso comece com 0
        DataPoint* new_points = (DataPoint*)realloc(dataset->points, new_capacity * sizeof(DataPoint));
        if (!new_points) {
            perror("Falha ao realocar DataPoints");
            return -1; // Falha
        }
        dataset->points = new_points;
        dataset->capacity = new_capacity;
    }
    
    strncpy(dataset->points[dataset->count].label, label, MAX_LABEL_LEN - 1);
    dataset->points[dataset->count].label[MAX_LABEL_LEN - 1] = '\0'; // Garantir terminação nula
    dataset->points[dataset->count].d1 = d1;
    dataset->points[dataset->count].d2 = d2;
    
    // Atualizar min/max
    if (d1 < dataset->min_d1) dataset->min_d1 = d1;
    if (d1 > dataset->max_d1) dataset->max_d1 = d1;
    if (d2 < dataset->min_d2) dataset->min_d2 = d2;
    if (d2 > dataset->max_d2) dataset->max_d2 = d2;
    
    dataset->count++;
    return 0; // Sucesso
}

DataSet* load_data_from_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir arquivo de dados");
        return NULL;
    }
    
    DataSet* dataset = create_dataset(INITIAL_DATASET_CAPACITY);
    if (!dataset) {
        fclose(file);
        return NULL;
    }
    
    char line_buffer[LINE_BUFFER_SIZE];
    char label_buffer[MAX_LABEL_LEN]; // Buffer temporário para o label
    double d1_val, d2_val;
    
    // Ler e ignorar a linha de cabeçalho
    if (fgets(line_buffer, sizeof(line_buffer), file) == NULL) {
        fprintf(stderr, "Erro ao ler cabeçalho ou arquivo vazio: %s\n", filename);
        free_dataset(dataset);
        fclose(file);
        return NULL;
    }
    
    int line_num = 1; // Começa em 1 para o cabeçalho, 2 para a primeira linha de dados
    while (fgets(line_buffer, sizeof(line_buffer), file) != NULL) {
        line_num++;
        // Usar sscanf para parsear a linha.
        // Formato: string<TAB>double<TAB>double
        // %49s para evitar overflow no label_buffer
        if (sscanf(line_buffer, "%49s\t%lf\t%lf", label_buffer, &d1_val, &d2_val) == 3) {
            if (add_point(dataset, label_buffer, d1_val, d2_val) != 0) {
                fprintf(stderr, "Falha ao adicionar ponto da linha %d do arquivo %s\n", line_num, filename);
                free_dataset(dataset);
                fclose(file);
                return NULL;
            }
        } else {
            // Remove a quebra de linha, se houver, para melhor impressão
            line_buffer[strcspn(line_buffer, "\r\n")] = 0;
            fprintf(stderr, "Aviso: Não foi possível parsear a linha %d no arquivo %s: \"%s\"\n", line_num, filename, line_buffer);
            // Decidir se continua ou para. Por ora, vamos continuar, ignorando a linha.
        }
    }
    
    if (ferror(file)) {
        perror("Erro durante a leitura do arquivo");
        free_dataset(dataset);
        fclose(file);
        return NULL;
    }
    
    fclose(file);
    
    if (dataset->count == 0) {
        fprintf(stderr, "Nenhum ponto de dado carregado de %s. O arquivo pode estar vazio após o cabeçalho ou ter problemas de formato.\n", filename);
        // free_dataset(dataset); // Dataset já está vazio, mas não custa.
        // return NULL; // Pode ser útil retornar um dataset vazio em vez de NULL
    }
    return dataset;
}

void free_dataset(DataSet* dataset) {
    if (dataset) {
        if (dataset->points) {
            free(dataset->points);
        }
        free(dataset);
    }
}

void print_dataset_summary(const DataSet* dataset) {
    if (!dataset) {
        printf("DataSet é NULL.\n");
        return;
    }
    printf("--- Resumo do DataSet ---\n");
    printf("  Pontos carregados: %d\n", dataset->count);
    if (dataset->count > 0) {
        printf("  Intervalo d1: [%.2f, %.2f]\n", dataset->min_d1, dataset->max_d1);
        printf("  Intervalo d2: [%.2f, %.2f]\n", dataset->min_d2, dataset->max_d2);
        // Exemplo: imprimir primeiro e último ponto
        // printf("  Primeiro ponto: %s (%.2f, %.2f)\n", dataset->points[0].label, dataset->points[0].d1, dataset->points[0].d2);
        // if (dataset->count > 1) {
        //    printf("  Último ponto: %s (%.2f, %.2f)\n", dataset->points[dataset->count-1].label, dataset->points[dataset->count-1].d1, dataset->points[dataset->count-1].d2);
        // }
    }
    printf("-------------------------\n");
}