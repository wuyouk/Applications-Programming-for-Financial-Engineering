#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <time.h>

void computeReturn(int day, int asset, double const *data, double *return_data);
void computeMu(int day, int asset, double *return_data, double *mu);
void computeCov(int day, int asset, double const *data, double *C, double *mu);
void norm(double *a, int n);
int optimize(double * eigenvec_matrix, double * factor_covar, double* residual_var, double * mu, int assetNum, int factorNum, double lambda, int namesLimit);

void power2(int day, int asset, int iter, double * C, int eigenNum, double *eigens, double * eigenvec_matrix) {

	int d = asset;
	
	double max_eigen = 0;
	double tolerance = 1e-10;
	double diff;

	double * Q = (double *)calloc(asset*asset, sizeof(double));
	if (!Q) {
		printf("cannot allocate space for Q\n");
		return;
	}

	//copy Covariance C to Q
	for (int i = 0; i < asset; i++) {
		for (int j = 0; j < asset; j++) {
			Q[i*asset + j] = C[i*asset + j];
		}
	}
	//create vectors w and w(0)
	double * w = (double *)calloc(d, sizeof(double));
	double * w_0 = (double *)calloc(d, sizeof(double));
	if (!w) {
		printf("cannot allocate space for w\n");
		return;
	}
	if (!w_0) {
		printf("cannot allocate space for w_0\n");
		return;
	}

	double * temp = (double *)calloc(d, sizeof(double));
	if (!temp) {
		printf("cannot allocate space for temp\n");
		return;
	}

	//initialize w
	for (int i = 0; i < d; i++) {
		w[i] = ((double)rand() / (RAND_MAX));
	}
	norm(w, d);

	//for loop for 10 max eigenvalues
	for (int n = 0; n < eigenNum; n++) {

		for (int i = 0; i < d; i++) {
			w[i] = ((double)rand());
		}
		norm(w, d);
		/**
		for (int i = 0; i < d; i++) {
			w_0[i] = w[i];
		}
		**/

		//iterations
		for (int t = 0; t < iter; t++) {
			for (int i = 0; i < d; i++) {
				temp[i] = 0;
				for (int j = 0; j < d; j++) {
					temp[i] += Q[i*d + j] * w[j];
				}
			}
			norm(temp, d);

			//check if w converge
			diff = 0;
			for (int i = 0; i < d; i++) {
				diff += (temp[i] - w[i])*(temp[i] - w[i]);
			}

			//break out of loop if converge 
			if (diff < tolerance) {
				//printf("\nConverge before iterarion limit, after %d loops.\n", t);
				break;

			}

			//update w value 
			for (int i = 0; i < d; i++) {
				w[i] = temp[i];
			}
		}

		//estimate eigenvalue, temp = Q * w = eigenvalue * w
		double sum = 0.0;
		for (int i = 0; i < d; i++) {
			temp[i] = 0.0;
			for (int j = 0; j < d; j++) {
				temp[i] += Q[i*d + j] * w[j];
			}
			sum += temp[i] * temp[i];
		}
		/**
		int count = 0;
		double sum = 0;
		for (int i = 0; i < d; i++) {
			if (w[i] != 0) {
				sum += temp[i] / w[i];
				count++;
			}
		}
		**/
		max_eigen = sqrt(sum);
		/*
		for (int i = 0; i < d; i++) {
		printf("\n %g %g %g", w[i], temp[i],temp[i]/w[i]);
		}
		*/
		//printf("\n %g", max_eigen);

		//store new eigenvalues (eigens) and eigenvectors (eigenvec_matrix)
		eigens[n] = max_eigen;
		for (int j = 0; j < asset; j++) {
			eigenvec_matrix[n*asset + j] = w[j];
		}

		//return max_eigen;

		//update Q: Q' = Q - eigenvalue * w * w.T
		for (int i = 0; i < d; i++) {
			for (int j = 0; j < d; j++) {
				Q[i*d + j] -= max_eigen*w[i] * w[j];
			}
		}

		/**
		//update w: w = w(0) - (w.T * w(0)) * w
		double w_prod = 0;
		for (int i = 0; i < d; i++) {
			w_prod += w[i] * w_0[i];
		}
		for (int i = 0; i < d; i++) {
			w[i] = w_0[i] - w_prod*w[i];
		}
		**/
	}
	printf("\nEigen values:");
	for (int i = 0; i < eigenNum; i++) {
		printf("%g ", eigens[i]);
	}
	
}

void estimateFactor(int day, int asset, double* data, double lambda, int namesLimit) {
	//333 need to specify the number of factors (r)
	int factorNum = 10;

	double * return_data = (double *)calloc(asset*(day - 1), sizeof(double));
	if (!return_data) {
		printf("cannot allocate space for return_data\n");
		return;
	}
	computeReturn(day, asset, data, return_data);

	double * mu = (double *)calloc(asset, sizeof(double));
	if (!mu) {
		printf("cannot allocate space for mu\n");
		return;
	}
	computeMu(day, asset, return_data, mu);

	double * C = (double *)calloc(asset*asset, sizeof(double));
	if (!C) {
		printf("cannot allocate space for C\n");
		return;
	}
	
	computeCov(day, asset, return_data, C, mu);
	/**
	printf("\nCovariance values:");
	for (int i = 0; i < 10; i++) {
		printf("%g ", C[i]);
	}
	**/

	double * factor_covar = (double *)calloc(factorNum*factorNum, sizeof(double));
	if (!factor_covar) {
		printf("cannot allocate space for factor_covar\n");
		return;
	}

	double * eigens = (double *)calloc(factorNum, sizeof(double));
	double * eigenvec_matrix = (double *)calloc(factorNum*asset, sizeof(double));
	if (!eigenvec_matrix) {
		printf("cannot allocate space for eigenvec_matrix\n");
		return;
	}

	//333 need to change factorNum: eigenNum
	power2(day, asset, 1000, C, factorNum, eigens, eigenvec_matrix); //1000 iteration limit, 10 max eigenvalues
	
	//generate factor_covar F: r*r diagonal matrix with eigenvalues as diagonal entries
	for (int i = 0; i < factorNum; i++) {
		factor_covar[i*factorNum + i] = eigens[i];
	}
	
	double * residual_matrix = (double *)calloc(asset*asset, sizeof(double));
	//333 print
	printf("\n");
	//printf("\n***D= \n");
	double * temp_matrix = (double *)calloc(asset*asset, sizeof(double));
	//compute temp_matrix = V.T * F * V
	for (int i = 0; i < asset; i++) {
		for (int j = 0; j < asset; j++) {
			double temp = 0;
			for (int k = 0; k < factorNum; k++) {
				temp += eigens[k] * eigenvec_matrix[k*asset + i] * eigenvec_matrix[k*asset + j];
			}
			temp_matrix[i*asset + j] = temp;
		}
	}

	/*
	int count = 0;
	for (int i = 0; i < asset; i++) {
		for (int j = 0; j < asset; j++) {
			residual_matrix[i*asset + j] = C[i*asset + j] - temp_matrix[i*asset + j];
			if (residual_matrix[i*asset + j]>1e-3){
				count++;
			}
		}
		
	}
	*/

	//calloc initialize to 0
	double * residual_var = (double *)calloc(asset, sizeof(double));
	if (!residual_var) {
		printf("cannot allocate space for residual_var\n");
		return;
	}
	//compute residual variance: D = Q - V.T * F * V, diagonal matrix
	int count = 0;
	for (int i = 0; i < asset; i++) {
		residual_var[i]= C[i*asset + i] - temp_matrix[i*asset + i];
		//count the number of residuals > 10^-3
		if (residual_var[i] > 1e-3) {
			count++;
		}
	}

	/**
	printf("\nResiduals values:");
	for (int i = 0; i < asset; i++) {
		printf("%g ", residual_var[i]);
	}
	**/

	printf("\ncount=%d\n\n", count);
	optimize(eigenvec_matrix, factor_covar, residual_var, mu, asset, factorNum, lambda, namesLimit);
}