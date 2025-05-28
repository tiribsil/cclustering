#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "clustering.h"

double mean(double* v, size_t size){
    if(!size) return 0;
    
    double sum = 0;
    for(int i = 0; i < size; i++) sum += v[i];
    
    return sum / size;
}

DataPoint centroid(DataSet* dataset, int* cluster, int size){
    double* d1s = malloc(sizeof(double) * size);
    double* d2s = malloc(sizeof(double) * size);
    
    for(int i = 0; i < size; i++){
        d1s[i] = dataset->points[cluster[i]].d1;
        d2s[i] = dataset->points[cluster[i]].d2;
    }
    
    DataPoint pCentroid;
    pCentroid.d1 = mean(d1s, size);
    pCentroid.d2 = mean(d2s, size);
    
    free(d1s);
    free(d2s);
    
    return pCentroid;
}

double squared_distance(DataPoint* p1, DataPoint* p2){
    return pow(p1->d1 - p2->d1, 2) + pow(p1->d2 - p2->d2, 2);
}

void k_means(DataSet* dataset, int k){
    int** clusters = malloc(sizeof(int*) * k);
    int* oldSizes = malloc(sizeof(int) * k);
    int* newSizes = malloc(sizeof(int) * k);
    
    for(int i = 0; i < k; i++){
        clusters[i] = malloc(sizeof(int) * dataset->count);
        clusters[i][0] = (dataset->count / (k + 1)) * (i + 1);
        printf("Ponto do cluster %d = %d\n", i, clusters[i][0]);
        dataset->points[clusters[i][0]].cluster_id = i;
        oldSizes[i] = 1;
    }
    
    int converged = 0;
    int iterations = 0;
    while(!converged && iterations < 1000){
        converged = 1;
        memset(newSizes, 0, sizeof(int) * k);
        for(int i = 0; i < dataset->count; i++){
            DataPoint centroid_point = centroid(dataset, clusters[0], oldSizes[0]);
            double smallest_distance = squared_distance(&dataset->points[i], &centroid_point);
            int closest_cluster = 0;
            
            for(int j = 1; j < k; j++){
                centroid_point = centroid(dataset, clusters[j], oldSizes[j]);
                double distance_j = squared_distance(&dataset->points[i], &centroid_point);
                if(distance_j > smallest_distance) continue;
                smallest_distance = distance_j;
                closest_cluster = j;
            }
            
            clusters[closest_cluster][newSizes[closest_cluster]] = i;
            newSizes[closest_cluster]++;
            
            if(closest_cluster == dataset->points[i].cluster_id) continue;
            
            dataset->points[i].cluster_id = closest_cluster;
            
            converged = 0;
        }
        
        memcpy(oldSizes, newSizes, sizeof(int) * k);
        iterations++;
    }
    
    for(int i = 0; i < k; i++) free(clusters[i]);
    free(clusters);
    free(oldSizes);
    free(newSizes);
}