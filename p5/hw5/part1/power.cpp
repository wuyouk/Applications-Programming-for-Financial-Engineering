#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void computeCov(int day, int asset, double const *data, double *C) {
	double * return_data = (double *)calloc(asset*(day - 1), sizeof(double));
	if (!return_data) {
		printf("cannot allocate space for return_data\n");
		return;
	}
	//compute daily return
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

	//compute sum of return and sum of return squared for each asset
	for (int i = 0; i < asset; i++) {
		m_sum[i] = 0;
		m_sqr_sum[i] = 0;
		for (int j = 0; j < day - 1; j++) {
			m_sum[i] += return_data[i*(day - 1) + j];
			m_sqr_sum[i] += return_data[i*(day - 1) + j] * return_data[i*(day - 1) + j];
		}
	}

	//compute covariance
	for (int i = 0; i < asset; i++) {
		for (int j = i; j < asset; j++) {
			if (j == i) {
				//C(ii) = E[i^2] - E[i]^2
				C[i*asset + j] = (m_sqr_sum[i] - m_sum[i] * m_sum[i] / (day - 1)) / (day - 2);
			}
			else {
				double cross_prod = 0;
				//compute sum of cross product
				for (int k = 0; k < day - 1; k++) {
					cross_prod += return_data[i*(day - 1) + k] * return_data[j*(day - 1) + k];
				}
				//C(ij) = E[i * j] - E[i]E[j]
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

//normalize the array
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

double power(int day, int asset, double* data, int iter) {

	int d = asset;
	double max_eigen = 0;
	double tolerance = 1e-10;
	double diff;

	double * C = (double *)calloc(asset*asset, sizeof(double));
	if (!C) {
		printf("cannot allocate space for C\n");
		return 0;
	}

	computeCov(day, asset, data, C);

	double * Q = (double *)calloc(asset*asset, sizeof(double));
	if (!Q) {
		printf("cannot allocate space for Q\n");
		return 0;
	}


	//copy C to Q
	for (int i = 0; i < asset; i++) {
		for (int j = 0; j < asset; j++) {
			Q[i*asset+j] = C[i*asset+j];
		}
	}


	double * w = (double *)calloc(d, sizeof(double));
	double * w_0 = (double *)calloc(d, sizeof(double));
	if (!w) {
		printf("cannot allocate space for w\n");
		return -1;
	}
	if (!w_0) {
		printf("cannot allocate space for w_0\n");
		return -1;
	}

	double * temp = (double *)calloc(d, sizeof(double));
	if (!temp) {
		printf("cannot allocate space for temp\n");
		return -1;
	}

	//initialize w
	for (int i = 0; i < d; i++) {
		w[i] = ((double)rand() / (RAND_MAX));
	}
	norm(w, d);

	//for loop for 10 max eigenvalues
	for (int n = 0; n < 10; n++) {
		//store initial vector
		for (int i = 0; i < d; i++) {
			w_0[i] = w[i];
		}

		//iterations
		for (int t = 0; t < iter; t++) {
			//matrix multiplication temp = Q * w
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

			//update w value anyway
			for (int i = 0; i < d; i++) {
				w[i] = temp[i];
			}
			//break out of loop if converge 
			if (diff < tolerance) {
				printf("\nConverge before iterarion limit, after %d loops.\n", t);
				break;
			}
		}

		//estimate eigenvalue, temp = Q * w = eigenvalue * w
		for (int i = 0; i < d; i++) {
			temp[i] = 0;
			for (int j = 0; j < d; j++) {
				temp[i] += Q[i*d + j] * w[j];
			}
		}

		int count = 0;
		double sum = 0;
		for (int i = 0; i < d; i++) {
			if (w[i] != 0) {
				sum += temp[i] / w[i];
				count++;
			}
		}

		max_eigen = sum / count;
		/*
		for (int i = 0; i < d; i++) {
		printf("\n %g %g %g", w[i], temp[i],temp[i]/w[i]);
		}
		*/
		printf("\n %g \n", max_eigen);
		//printf("\n %g", C[240]);

		//return max_eigen;

		//update Q: Q' = Q - eigenvalue * w * w.T
		for (int i = 0; i < d; i++) {
			for (int j = 0; j < d; j++) {
				Q[i*d + j] -= max_eigen*w[i]*w[j];
			}
		}

		//update w: w' = w(0) - (w.T * w(0)) * w
		double w_prod = 0;
		for (int i = 0; i < d; i++) {
			w_prod += w[i] * w_0[i];
		}
		for (int i = 0; i < d; i++) {
			w[i] = w_0[i] - w_prod*w[i];
		}

	}
	return 0;
}