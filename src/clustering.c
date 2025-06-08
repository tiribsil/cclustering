#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "clustering.h"

double squared_distance(DataPoint* p1, DataPoint* p2){
    return pow(p1->d1 - p2->d1, 2) + pow(p1->d2 - p2->d2, 2);
}

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

double points_distance(DataPoint* p1, DataPoint* p2) {
	return sqrt(pow(p1->d1 - p2->d1, 2) + pow(p1->d2 - p2->d2, 2));
}

void merge_clusters(DataSet* dataset, double** clusters_distance, bool* existing_clusters, int cluster1, int cluster2) {

	int qtd_points = dataset->count;

	existing_clusters[cluster2] = false;
	for (int i = 0; i < qtd_points; i++)
		if (dataset->points[i].cluster_id == cluster2) dataset->points[i].cluster_id = cluster1;
	
	// Atualizando matriz das distancias entre os clusters:
	for (int i = 0; i < qtd_points; i++) {
		if (i == cluster1) continue;
		if (clusters_distance[cluster1][i] < clusters_distance[cluster2][i]) {
			clusters_distance[cluster1][i] = clusters_distance[cluster2][i];
			clusters_distance[i][cluster1] = clusters_distance[cluster2][i];
		}
	}

}

void complete_link(DataSet* dataset, int k) {
	
	int qtd_points = dataset->count, qtd_clusters = dataset->count;
	
	// Cada ponto é um cluster fechado:
	bool* existing_clusters = (bool*)malloc(sizeof(bool)*qtd_points);
	for (int i = 0; i < qtd_points; i++) {
		existing_clusters[i] = true;
		dataset->points[i].cluster_id = i;
	}
	
	// Array de distancias entres clusters:
	double** clusters_distance = (double**)malloc(sizeof(double*)*qtd_points);
	for (int i = 0; i < qtd_points; i++) {
		clusters_distance[i] = (double*)malloc(sizeof(double)*qtd_points);
		for (int j = 0; j < qtd_points; j++) clusters_distance[i][j] = points_distance(&dataset->points[i], &dataset->points[j]);
	}
	
	// Comeco do algoritmo de fato:
	while(qtd_clusters != k) {
		
		double shortest_distance = INFINITY;
		int cluster1 = -1, cluster2 = -1;
		
		// Encontrando a menor distancia max na matriz de distancias dos clusters
		for (int i = 0; i < qtd_points; i++) {
			if (existing_clusters[i] == false) continue;
			for (int j = 0; j < qtd_points; j++) {
				if (existing_clusters[j] == false || clusters_distance[i][j] == 0) continue;
				if (clusters_distance[i][j] < shortest_distance) {
					shortest_distance = clusters_distance[i][j];
					cluster1 = i;
					cluster2 = j;
				}
			}
		}
		merge_clusters(dataset, clusters_distance, existing_clusters, cluster1, cluster2);
		qtd_clusters--;
		
	}
	
	// Deixando os clusters com as corzinha tudo certo:
	int correct_id = 0, first = 0;
	for (int last = 1; last < qtd_points; correct_id++, last++) {
		while (last < qtd_points && existing_clusters[last] == false) last++;
		while (first < last) dataset->points[first++].cluster_id = correct_id;
	}

}

