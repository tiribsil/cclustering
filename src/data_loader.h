/* date = May 27th 2025 10:06 am */
#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#define MAX_LABEL_LEN 50

typedef struct {
    char label[MAX_LABEL_LEN];
    double d1;
    double d2;
    int cluster_id;
} DataPoint;

typedef struct {
    DataPoint *points;
    int count;
    int capacity;
    double min_d1, max_d1;
    double min_d2, max_d2;
} DataSet;

DataSet* create_dataset(int initial_capacity);

DataSet* load_data_from_file(const char* filename);

void free_dataset(DataSet* dataset);

void print_dataset_summary(const DataSet* dataset);

#endif // DATA_LOADER_H