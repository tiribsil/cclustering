#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "clustering.h"

double squared_distance(DataPoint* p1, DataPoint* p2){
    return pow(p1->d1 - p2->d1, 2) + pow(p1->d2 - p2->d2, 2);
}

void uncluster(DataSet* dataset){
    for(int i = 0; i < dataset->count; i++)
        dataset->points[i].cluster_id = 0;
}

DataPoint* centroids(DataSet* dataset, int n_clusters){
    DataPoint* centroid_points = malloc(sizeof(DataPoint) * n_clusters);
    
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
        centroid_points[i].d1 = d1_sums[i] / sizes[i];
        centroid_points[i].d2 = d2_sums[i] / sizes[i];
    }
    
    free(d1_sums);
    free(d2_sums);
    free(sizes);
    
    return centroid_points;
}

void k_means(DataSet* dataset, int k, int iteration_limit){
    uncluster(dataset);
    
    // Escolhe os pontos iniciais
    for(int i = 0; i < k; i++){
        int chosen_index = (dataset->count / (k + 1)) * (i + 1);
        
        dataset->points[chosen_index].cluster_id = i;
    }
    
    int converged = 0;
    int iterations = 0;
    // Enquanto nao convergir e nao passar do limite
    while(!converged && iterations < iteration_limit){
        converged = 1;
        // Vetor com o centroide de cada cluster
        DataPoint* centroid_points = centroids(dataset, k);
        
        // Para cada ponto...
        for(int i = 0; i < dataset->count; i++){
            // Acha o cluster com o centroide mais proximo
            double smallest_distance = squared_distance(&dataset->points[i], &centroid_points[0]);
            int closest_cluster = 0;
            
            for(int j = 1; j < k; j++){
                double distance_j = squared_distance(&dataset->points[i], &centroid_points[j]);
                if(distance_j > smallest_distance) continue;
                smallest_distance = distance_j;
                closest_cluster = j;
            }
            
            // Se nunca passar desse if, convergiu
            if(closest_cluster == dataset->points[i].cluster_id) continue;
            
            dataset->points[i].cluster_id = closest_cluster;
            converged = 0;
        }
        
        iterations++;
        
        free(centroid_points);
    }
}

void colour_setting(DataSet* dataset, bool* existing_clusters, int k) {
	int qtd_points = dataset->count, current_id = 0, iterations = k;
	
	for (int i = 0; i < qtd_points; i++) {
		if (iterations == 0) break;
		if (existing_clusters[i] == true) {
			int aux = dataset->points[i].cluster_id;
			for (int j = 0; j < qtd_points; j++) {
				if (dataset->points[j].cluster_id == aux)
					dataset->points[j].cluster_id = current_id;
			}
			current_id++;
			iterations--;
		}
	}
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
		for (int j = 0; j < qtd_points; j++) clusters_distance[i][j] = squared_distance(&dataset->points[i], &dataset->points[j]);
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
	
	// Corrigindo as cores:
	colour_setting(dataset, existing_clusters, k);
	
	// Desalocando a matriz:
	for (int i = 0; i < qtd_points; i++) free(clusters_distance[i]);
	free(clusters_distance);
	free(existing_clusters); // Desalocando existing_clusters

}

void find_closest_points_different_clusters(DataSet* dataset, double** matrix, int* point1_index, int* point2_index) {
    int quant_points = dataset->count;
    double min_distance = INFINITY;
    
    //acha os pontos mais proximos
    for (int i = 0; i < quant_points; i++) {
        for (int j = i + 1; j < quant_points; j++) {
            if(dataset->points[i].cluster_id == dataset->points[j].cluster_id) continue; //pula pontos do mesmo cluster
            double distance = matrix[i][j];
            if (distance >= min_distance) continue; 
            min_distance = distance;
            *point1_index = i;
            *point2_index = j;
        }
    }
}

void merge_clusters_single(DataSet* dataset, int point1_index, int point2_index) {
    int cluster_apagado = dataset->points[point2_index].cluster_id;
    for (int i = 0; i < dataset->count; i++) {
        if(dataset->points[i].cluster_id != cluster_apagado) continue;
        dataset->points[i].cluster_id = dataset->points[point1_index].cluster_id;
    }
}

void single_link(DataSet* dataset, int k) {
    uncluster(dataset);
    
    int quant_clusters = dataset->count;
    
    //cada ponto começa como um cluster
	for (int i = 0; i < dataset->count; i++)
		dataset->points[i].cluster_id = i;
    
    //cria matriz de distancias entre pontos
	double** distance_matrix = (double**) malloc(sizeof(double*) * quant_clusters);
	for (int i = 0; i < quant_clusters; i++) {
		distance_matrix[i] = (double*) malloc(sizeof(double) * quant_clusters);
		for (int j = 0; j < quant_clusters; j++) 
            distance_matrix[i][j] = squared_distance(&dataset->points[i], &dataset->points[j]);
	}
    
    //reduz quant_clusters até k fazendo junção
    while(quant_clusters != k) {
		int point1_index = -1, point2_index = -1;
        find_closest_points_different_clusters(dataset, distance_matrix, &point1_index, &point2_index);
        merge_clusters_single(dataset, point1_index, point2_index);
        quant_clusters--;
    }
    
    // Deixando os clusters com as corzinha tudo certo:
    int* new_cluster_id_hash = malloc(sizeof(int) * dataset->count);
    for (int i = 0; i < dataset->count; i++) {
        new_cluster_id_hash[i] = -1;
    }
    
    for (int i = 0, k = 0; i < dataset->count; i++) {
        if(new_cluster_id_hash[dataset->points[i].cluster_id] != -1) continue;
        new_cluster_id_hash[dataset->points[i].cluster_id] = k;
        k++;
    }
    
    for (int i = 0; i < dataset->count; i++) {
        dataset->points[i].cluster_id = new_cluster_id_hash[dataset->points[i].cluster_id];
    }
    
    for(int i = 0; i < dataset->count; i++) free(distance_matrix[i]);
    free(distance_matrix);
    free(new_cluster_id_hash);
}

long long combinations(int n, int k) {
    if (k < 0 || k > n) {
        return 0;
    }
    if (k == 0 || k == n) {
        return 1;
    }
    if (k > n / 2) {
        k = n - k;
    }
    if (k != 2) { 
        return 0;
    }
    return (long long)n * (n - 1) / 2;
}

double adjusted_rand_index(const int* clusters_A, const int* clusters_B, int num_points) {
    if (clusters_A == NULL || clusters_B == NULL || num_points == 0) {
        return 0.0;
    }

    int max_A = 0, max_B = 0;
    for (int i = 0; i < num_points; i++) {
        if (clusters_A[i] > max_A) max_A = clusters_A[i];
        if (clusters_B[i] > max_B) max_B = clusters_B[i];
    }
    int k_A = max_A + 1;
    int k_B = max_B + 1;

    int** contingency_table = (int**)malloc(k_A * sizeof(int*));
    for (int i = 0; i < k_A; i++) {
        contingency_table[i] = (int*)calloc(k_B, sizeof(int));
    }

    for (int i = 0; i < num_points; i++) {
        contingency_table[clusters_A[i]][clusters_B[i]]++;
    }

    long long sum_nij_choose_2 = 0;
    for (int i = 0; i < k_A; i++) {
        for (int j = 0; j < k_B; j++) {
            sum_nij_choose_2 += combinations(contingency_table[i][j], 2);
        }
    }

    long long sum_a_choose_2 = 0;
    for (int i = 0; i < k_A; i++) {
        int a_i = 0;
        for (int j = 0; j < k_B; j++) {
            a_i += contingency_table[i][j];
        }
        sum_a_choose_2 += combinations(a_i, 2);
    }

    long long sum_b_choose_2 = 0;
    for (int j = 0; j < k_B; j++) {
        int b_j = 0;
        for (int i = 0; i < k_A; i++) {
            b_j += contingency_table[i][j];
        }
        sum_b_choose_2 += combinations(b_j, 2);
    }
    
    for (int i = 0; i < k_A; i++) {
        free(contingency_table[i]);
    }
    free(contingency_table);

    // Calcula o ARI usando a fórmula
    // ARI = (Index - ExpectedIndex) / (MaxIndex - ExpectedIndex)
    long long total_combinations = combinations(num_points, 2);
    double expected_index = (double)(sum_a_choose_2 * sum_b_choose_2) / total_combinations;
    double max_index = 0.5 * (sum_a_choose_2 + sum_b_choose_2);
    double index = sum_nij_choose_2;

    double numerator = index - expected_index;
    double denominator = max_index - expected_index;
    
    if (denominator == 0) {
        return (numerator == 0) ? 1.0 : 0.0;
    }
    
    return numerator / denominator;
}
