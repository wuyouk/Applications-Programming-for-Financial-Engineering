#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//utility functions
//compute daily return for each asset
void computeReturn(int day, int asset, double const *data, double *return_data) {
	for (int i = 0; i < asset; i++) {
		for (int j = 0; j < day - 1; j++) {
			return_data[i*(day - 1) + j] = data[i*day + (j + 1)] / data[i*day + j] - 1;
		}
	}
}

//compute return mean
void computeMu(int day, int asset, double *return_data, double *mu) {   
	int n = day - 1;
	
	for (int i = 0; i < asset; i++) {
		double sum = 0.0;
		for (int j = 0; j < n; j++) {
			sum += return_data[i*n + j];
		}
		mu[i] = sum / n;
	}
}

//compute covariance
void computeCov(int day, int asset, double const *return_data, double *C, double *mu) {
	
	double sum = 0.0;

	for (int i = 0; i < asset; i++) {
		for (int j = 0; j < asset; j++) {
			sum = 0;
			for (int k = 0; k < day - 1; k++) {
				sum += (return_data[i*(day - 1) + k] - mu[i]) * (return_data[j*(day - 1) + k] - mu[j]);
			}
			C[i*asset + j] = (sum) / (day - 2);
		}
	}
	/**
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

	for (int i = 0; i < asset; i++) {
		m_sum[i] = 0;
		m_sqr_sum[i] = 0;
		for (int j = 0; j < day - 1; j++) {
			m_sum[i] += return_data[i*(day - 1) + j];
			m_sqr_sum[i] += return_data[i*(day - 1) + j] * return_data[i*(day - 1) + j];
		}
	}

	for (int i = 0; i < asset; i++) {
		for (int j = i; j < asset; j++) {
			if (j == i) {
				C[i*asset + j] = (m_sqr_sum[i] - m_sum[i] * m_sum[i] / (day - 1)) / (day - 2);
			}
			else {
				double cross_prod = 0;
				for (int k = 0; k < day - 1; k++) {
					cross_prod += return_data[i*(day - 1) + k] * return_data[j*(day - 1) + k];
				}
				C[i*asset + j] = (cross_prod - m_sum[i] * m_sum[j] / (day - 1)) / (day - 2);

			}
		}
	}

	//copy the lower half triangle
	for (int i = 0; i < asset; i++) {
		for (int j = 0; j < i; j++) {
			C[i*asset + j] = C[j*asset + i];
		}
	}
	free(m_sum);
	free(m_sqr_sum);
	**/
	/*

	printf("Covariance matrix:\n");

	for (int i = 0; i < asset; i++) {
	printf("\n");
	for (int j = 0; j < asset; j++) {
	printf("%g ", C[i*asset + j]);
	}
	}
	*/


}

//normalize a vector
void norm(double *a, int n) {
	double r = 0.0;
	for (int i = 0; i < n; i++) {
		r += a[i] * a[i];
	}
	r = sqrt(r);
	for (int i = 0; i < n; i++) {
		a[i] /= r;
	}
}