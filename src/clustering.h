/* date = May 27th 2025 10:22 am */

#ifndef CLUSTERING_H
#define CLUSTERING_H

#include "data_loader.h"

DataPoint* centroids(DataSet* dataset, int n_clusters);

void k_means(DataSet* dataset, int k, int iteration_limit);

void single_link(DataSet* dataset, int k);

void complete_link(DataSet* dataset, int k);

double adjusted_rand_index(const int* clusters_A, const int* clusters_B, int num_points); 

#endif //CLUSTERING_H
