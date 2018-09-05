#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void computeCov(int day, int asset, double const *data, double *C) {

	double * return_data = (double *)calloc(asset*(day-1), sizeof(double));
	if (!return_data) {
		printf("cannot allocate space for return_data\n");
		return;
	}
	//  compute return data
	for (int i = 0; i < asset; i++) {
		for (int j = 0; j < day - 1; j++) {
			return_data[i*(day - 1) + j] = data[i*day + (j + 1)] / data[i*day + j] - 1;
		}
	}

	double * m_sum = (double *)calloc(asset, sizeof(double));
	if (!m_sum) {
		printf("cannot allocate space for m_sum\n");
		return;
	}
	double * m_sqr_sum = (double *)calloc(asset, sizeof(double));
	if (!m_sqr_sum) {
		printf("cannot allocate space for m_sqr_sum\n");
		return;
	}
	// compute sum of asset return, asset return squared
	for (int i = 0; i < asset; i++) {
		m_sum[i] = 0;
		m_sqr_sum[i] = 0;
		for (int j = 0; j < day - 1; j++) {
			m_sum[i] += return_data[i*(day-1)+j];
			m_sqr_sum[i] += return_data[i*(day - 1) + j]* return_data[i*(day - 1) + j];
		}
	}

	for (int i = 0; i < asset; i++) {
		for (int j = i; j < asset; j++) {
			if (j == i) {
				//  
				C[i*asset + j] = (m_sqr_sum[i]-m_sum[i]*m_sum[i]/(day-1))/ (day - 2);
			}
			else {
				double cross_prod = 0;
				for (int k = 0; k < day - 1; k++) {
					//  sum of ri * rj for each day
					cross_prod += return_data[i*(day - 1) + k] * return_data[j*(day - 1) + k];
				}
				//  E[X_i * X_j] - u_i * u_j
				C[i*asset + j] = (cross_prod-m_sum[i]*m_sum[j]/(day-1)) / (day - 2);

			}
		}
	}

	//copy the lower half triangle
	for (int i = 0; i < asset; i++) {
		for (int j = 0; j < i; j++) {
			C[i*asset + j] = C[j*asset + i];
		}
	}

	/*

	printf("Covariance matrix:\n");

	for (int i = 0; i < asset; i++) {
		printf("\n");
		for (int j = 0; j < asset; j++) {
			printf("%g ", C[i*asset + j]);
		}
	}
	*/

	free(m_sum);
	free(m_sqr_sum);
	free(return_data);


}


//  not use
void rollingCov(int day, int asset, double const *data, double *C, int window) {
	double * return_data = (double *)calloc(asset*(day - 1), sizeof(double));
	if (!return_data) {
		printf("cannot allocate space for return_data\n");
		return;
	}
	for (int i = 0; i < asset; i++) {
		for (int j = 0; j < day - 1; j++) {
			return_data[i*(day - 1) + j] = data[i*day + (j + 1)] / data[i*day + j] - 1;
		}
	}
	free(return_data);
}

//  normalize
void norm(double *a, int n) {
	double r = 0;
	for (int i = 0; i < n; i++) {
		r += a[i] * a[i];
	}
	r = sqrt(r);
	for (int i = 0; i < n; i++) {
		a[i] /= r;
	}
	
}

double power(int day, int asset, double* data, int iter){


	int d = asset;
	double max_eigen=0;
	double tolerance = 1e-10;
	double temp_norm;
	double diff;


	double * C = (double *)calloc(asset*asset, sizeof(double));
	if (!C) {
		printf("cannot allocate space for C\n");
		return 0;
	}

	computeCov(day, asset, data, C);



	double * w = (double *)calloc(d, sizeof(double));
	if (!w) {
		printf("cannot allocate space for w\n");
		return -1;
	}

	double * temp = (double *)calloc(d, sizeof(double));
	if (!temp) {
		printf("cannot allocate space for temp\n");
		return -1;
	}

	//initialize w, initial vector
	for (int i = 0; i < d; i++) {
		w[i] = ((double)rand() / (RAND_MAX));
	}
	norm(w,d);

	//iterations
	for (int t = 0; t < iter; t++) {
		//  cov * w
		for (int i = 0; i < d; i++) {
			temp[i] = 0;
			for (int j = 0; j < d; j++) {
				temp[i] += C[i*d + j] * w[j];
			}
		}
		norm(temp, d);

		//check if w converge
		diff = 0;
		for (int i = 0; i < d; i++) {
			diff += (temp[i] - w[i])*(temp[i] - w[i]);
		}

		//update w value anyway
		for (int i = 0; i < d; i++) {
			w[i] = temp[i];
		}
		//break out of loop if converge 
		if (diff < tolerance) {
			printf("\nConverge before iterarion limit, after %d loops.\n",t);
			break;

		}

		
	}

	//estimate eigenvalue, temp = eva * evc = C * w
	for (int i = 0; i < d; i++) {

		temp[i] = 0;
		for (int j = 0; j < d; j++) {
			temp[i] += C[i*d + j] * w[j];
		}
	}
	//  temp / eogenvector = eigenvalue
	int count = 0;
	double sum = 0;
	for (int i = 0; i < d; i++) {
		if (w[i]!=0 ) {
			sum += temp[i] / w[i];
			count++;
		}
	}

	max_eigen = sum/count;
	/*
	for (int i = 0; i < d; i++) {
		printf("\n %g %g %g", w[i], temp[i],temp[i]/w[i]);
	}
	*/
	printf("\n %g", max_eigen);

	return max_eigen;

}