#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "clustering.h"

DataPoint* centroids(DataSet* dataset, int n_clusters){
    DataPoint* p_centroids = malloc(sizeof(DataPoint) * n_clusters);
    
    double* d1_sums = malloc(sizeof(double) * n_clusters);
    double* d2_sums = malloc(sizeof(double) * n_clusters);
    int* sizes = malloc(sizeof(int) * n_clusters);
    
    memset(sizes, 0, sizeof(int) * n_clusters);
    memset(d1_sums, 0, sizeof(double) * n_clusters);
    memset(d2_sums, 0, sizeof(double) * n_clusters);
    
    for(int i = 0; i < dataset->count; i++){
        int i_cluster = dataset->points[i].cluster_id;
        d1_sums[i_cluster] += dataset->points[i].d1;
        d2_sums[i_cluster] += dataset->points[i].d2;
        sizes[i_cluster]++;
    }
    
    for(int i = 0; i < n_clusters; i++){
        p_centroids[i].d1 = d1_sums[i] / sizes[i];
        p_centroids[i].d2 = d2_sums[i] / sizes[i];
    }
    
    free(d1_sums);
    free(d2_sums);
    free(sizes);
    
    return p_centroids;
}

void k_means(DataSet* dataset, int k){
    for(int i = 0; i < k; i++){
        int point_index = (dataset->count / (k + 1)) * (i + 1);
        
        //Para mostrar pontos iniciais escolhidos
        //printf("Ponto do cluster %d = %d\n", i, clusters[i][0]);
        
        dataset->points[point_index].cluster_id = i;
    }
    
    int converged = 0;
    int iterations = 0;
    while(!converged && iterations < 1000){
        converged = 1;
        DataPoint* p_centroids = centroids(dataset, k);
        
        for(int i = 0; i < dataset->count; i++){
            double smallest_distance = squared_distance(&dataset->points[i], &p_centroids[0]);
            int closest_cluster = 0;
            
            for(int j = 1; j < k; j++){
                double distance_j = squared_distance(&dataset->points[i], &p_centroids[j]);
                if(distance_j > smallest_distance) continue;
                smallest_distance = distance_j;
                closest_cluster = j;
            }
            
            if(closest_cluster == dataset->points[i].cluster_id) continue;
            
            dataset->points[i].cluster_id = closest_cluster;
            converged = 0;
        }
        
        iterations++;
        
        free(p_centroids);
    }
}
