/* date = May 27th 2025 10:06 am */
#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#define MAX_LABEL_LEN 50 // Comprimento máximo para o rótulo da amostra

// Estrutura para um único ponto de dado
typedef struct {
    char label[MAX_LABEL_LEN];
    double d1;
    double d2;
    int cluster_id;
} DataPoint;

// Estrutura para o conjunto de dados completo
typedef struct {
    DataPoint *points;    // Array dinâmico de pontos
    int count;            // Número de pontos carregados
    int capacity;         // Capacidade atual do array 'points'
    double min_d1, max_d1; // Para normalização/escala na plotagem
    double min_d2, max_d2; // Para normalização/escala na plotagem
} DataSet;

/**
 * @brief Cria e inicializa um novo DataSet.
 * @param initial_capacity Capacidade inicial para o array de pontos.
 * @return Ponteiro para o DataSet criado, ou NULL em caso de falha.
 */
DataSet* create_dataset(int initial_capacity);

/**
 * @brief Carrega dados de um arquivo .txt para um DataSet.
 * A primeira linha (cabeçalho) do arquivo é ignorada.
 * As colunas esperadas são: label(string) <TAB> d1(double) <TAB> d2(double)
 * @param filename O nome do arquivo a ser carregado.
 * @return Ponteiro para o DataSet contendo os dados, ou NULL em caso de erro.
 */
DataSet* load_data_from_file(const char* filename);

/**
 * @brief Libera toda a memória alocada para o DataSet.
 * @param dataset O DataSet a ser liberado.
 */
void free_dataset(DataSet* dataset);

/**
 * @brief Imprime um resumo do DataSet (para debug).
 * @param dataset O DataSet a ser resumido.
 */
void print_dataset_summary(const DataSet* dataset);

#endif // DATA_LOADER_H