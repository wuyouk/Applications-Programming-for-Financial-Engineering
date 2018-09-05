#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include "returncode.h"


int readit(char *nameoffile, int *addressofn, double **, double **, double **, double *, double **);

int algo(int n, double *x, double *lb, double *ub, double *mu, double *covariance, double lambda);

int first_order_algo(int n, double *x, double *lb, double *ub, double *mu, double *covariance, double lambda);

int feasible(int n, double *x, double *lb, double *ub);

double computeObjVal(double lambda, double *covariance, double *x, double *mu, int n);

int main(int argc, char **argv)
{
  int retcode = 0;
  int n;
  double *lb, *ub, *covariance, *mu, lambda, *x;

  if (argc != 2){
	  printf("usage: qp1 filename\n");  retcode = 1;
	  goto BACK;
  }

  retcode = readit(argv[1], &n, &lb, &ub, &mu, &lambda, &covariance);
  if (retcode) goto BACK;

  x = (double *)calloc(n, sizeof(double));
  if (x == NULL){
	  printf(" no memory for x\n"); retcode = NOMEMORY; goto BACK;
  }

  
  printf("Using coordinate descent algo:\n");
  retcode = algo(n, x, lb, ub, mu, covariance, lambda); 
  printf("Final objective value is %g\n", computeObjVal(lambda, covariance, x, mu, n));
  
  
  printf("Using first order algo:\n");
  retcode = first_order_algo(n, x, lb, ub, mu, covariance, lambda);
  printf("Final objective value is %g\n", computeObjVal(lambda, covariance, x, mu, n));
  

  /*
  printf("Final values of x:\n");
  double sum = 0;
  for (int i = 0; i < n; i++) {
	  printf("x%d = %g\n", i, x[i]);
	  sum += x[i];
  }
  
  printf("sum= %g\n", sum);
  */
  

  BACK:
  return retcode;
}

int readit(char *filename, int *address_of_n, double **plb, double **pub,
		double **pmu, double *plambda, double **pcovariance)
{
	int readcode = 0, fscancode;
	FILE *datafile = NULL;
	char buffer[100];
	int n, i, j;
	double *lb = NULL, *ub = NULL, *mu = NULL, *covariance = NULL;

	datafile = fopen(filename, "r");
	if (!datafile){
		printf("cannot open file %s\n", filename);
		readcode = CANNOTOPENFILE;  goto BACK;
	}

	printf("reading data file %s\n", filename);

	fscanf(datafile, "%s", buffer);
	fscancode = fscanf(datafile, "%s", buffer);
	if (fscancode == EOF){
		printf("problem: premature file end at ...\n");
		readcode = 4; goto BACK;
	}

	n = *address_of_n = atoi(buffer);

	printf("n = %d\n", n);

	lb = (double *)calloc(n, sizeof(double));
	*plb = lb;
	ub = (double *)calloc(n, sizeof(double));
	*pub = ub;
	mu = (double *)calloc(n, sizeof(double));
	*pmu = mu;
	covariance = (double *)calloc(n*n, sizeof(double));
	*pcovariance = covariance;

	if (!lb || !ub || !mu || !covariance){
		printf("not enough memory for lb ub mu covariance\n"); readcode = NOMEMORY; goto BACK;
	}

	fscanf(datafile, "%s", buffer);

	for (j = 0; j < n; j++){
		fscanf(datafile, "%s", buffer);
		fscanf(datafile, "%s", buffer);
		lb[j] = atof(buffer);
		fscanf(datafile, "%s", buffer);
		ub[j] = atof(buffer);
		fscanf(datafile, "%s", buffer);
		mu[j] = atof(buffer);
		printf("j = %d lb = %g ub = %g mu = %g\n", j, lb[j], ub[j], mu[j]);
	}


	fscanf(datafile, "%s", buffer);
	fscanf(datafile, "%s", buffer);
	*plambda = atof(buffer);

	fscanf(datafile, "%s", buffer); /* reading 'covariance'*/

	for (i = 0; i < n; i++){
		for (j = 0; j < n; j++){ 
			fscanf(datafile, "%s", buffer);
			covariance[i*n + j] = atof(buffer);
		}
	}


	fscanf(datafile, "%s", buffer);
	if (strcmp(buffer, "END") != 0){
		printf("possible error in data file: 'END' missing\n");
	}


	fclose(datafile);

BACK:

	return readcode;
}

double computeObjVal(double lambda, double *covariance, double *x, double *mu, int n) {
	double res = 0.0;
	for (int i = 0; i < n; i++) {
		res += lambda*covariance[i*n + i] * x[i] * x[i] - mu[i] * x[i];
		for (int j = i + 1; j < n; j++) {
			res += 2 * lambda*covariance[i*n + j] * x[i] * x[j];
		}
	}
	return res;

}
int algo(int n, double *x, double *lb, double *ub, double *mu, double *covariance, double lambda)
{
	int returncode = 0;

	printf("\n * Running coordinate descent algo:\n");

	returncode = feasible(n, x, lb, ub);

	double a, b, temp, e, e_pos, e_neg;

	int t = 0;
	while (t < 10) {
		for (int h = 0; h < n; h++) {
			for (int k = 0; k < n; k++) {
				if (h == k) continue;
				temp = 0;

				for (int i = 0; i < n; i++) {
					if (i == h || i == k) continue;
					temp += (covariance[i*n + k] - covariance[i*n + h])*x[i];
				}
				a = lambda*(covariance[h*n + h] + covariance[k*n + k] - 2 * covariance[h*n + k]);
				b = 2 * lambda*(covariance[k*n + k] * x[k] - covariance[h*n + h] * x[h] + covariance[h*n + k] * (x[h] - x[k]) + temp) + mu[h] - mu[k];
				

				e_pos = ((x[h] - lb[h]) < (ub[k] - x[k])) ? (x[h] - lb[h]) : (ub[k] - x[k]);
				e_neg = ((ub[h] - x[h]) < (x[k] - lb[k])) ? (ub[h] - x[h]) : (x[k] - lb[k]);

				if (a == 0) {
					if (b >= 0) e = -e_neg;
					else e = e_pos;
				}else{
					e = -b / (2 * a);
					if (e < -e_neg) { e = -e_neg; }
					if (e > e_pos) { e = e_pos; }
				}
				x[h] -= e;
				x[k] += e;
				
			}
		}
		t++;
	}
	printf("Reached loop limit\n"); 
	

	return returncode;
}


int first_order_algo(int n, double *x, double *lb, double *ub, double *mu, double *covariance, double lambda)
{
	int returncode = 0;

	printf("\n * Running first order algo:\n");

	returncode = feasible(n, x, lb, ub);

	double temp;
	std::vector<double> data;
	std::vector<int> index(n, 0);
	int min_delta_index;
	double y_sum, obj_delta, min_delta;
	double c, d, s;  //calculate step size

	double* g= (double *)calloc(n, sizeof(double));
	double* y = (double *)calloc(n, sizeof(double));
	double* y_orig = (double *)calloc(n, sizeof(double));

	if (!g || !y || !y_orig) {
		printf("not enough memory for g y y_orig\n"); returncode = NOMEMORY; return returncode;
	}


	int t = 0;
	while(t<100000){

		for (int i = 0; i < n; i++) {
			temp = 0;
			for (int j = 0; j < n; j++) {
				if (i == j) continue;
				temp += 2 * covariance[i*n + j] * x[j];
			}
			g[i] = lambda*(2 * covariance[i*n + i] * x[i] + temp) - mu[i];
		}
		/*
		printf("Derivatives: \n");
		for (int i = 0; i<n; i++) {
			printf("%d: %g\n", i, g[i]);
		}
		*/
		data.assign(g, g + n);
	
		for (int i = 0; i != index.size(); i++) {
			index[i] = i;
		}
		//index keeps track of the indices of the original unsorted gradients from largest to smallest
		std::sort(index.begin(), index.end(), [&](const int& a, const int& b) {return data[a] > data[b]; });
		/*
		for (int i = 0; i != index.size(); i++) {
			printf("%d: %d: %g\n", i, index[i], data[index[i]]);
		}
		*/

	
		min_delta_index = -1;
		min_delta = 0;
		for (int m = 0; m < n ; m++) {
			y_sum = 0.0;
			obj_delta = 0.0;
		
			for (int j=0;j<n;j++){
				if (j == m) continue;
				if (j < m) { y[j] = lb[index[j]] - x[index[j]]; }
				if (j > m) { y[j] = ub[index[j]] - x[index[j]]; }
				y_sum += y[j];	
				obj_delta += y[j] * g[index[j]];
			}
		
			y[m] = 0.0 - y_sum;

		
			//infeasible y[m]
			if (y[m] < lb[index[m]] - x[index[m]] || y[m] > ub[index[m]] - x[index[m]]) continue;

			obj_delta += y[m] * g[index[m]];
			if (obj_delta < min_delta) {
				min_delta_index = m;
				min_delta = obj_delta;
			}
		
		}

		if (min_delta_index == -1) {
			printf("Reached optimal...");
			return returncode;
		}
		else {
			y_sum = 0.0;
			for (int j = 0; j < n; j++) {
				if (j == min_delta_index) continue;
				if (j < min_delta_index) { y[j] = lb[index[j]] - x[index[j]]; }
				if (j > min_delta_index) { y[j] = ub[index[j]] - x[index[j]]; }
				y_sum += y[j];
			}

			y[min_delta_index] = 0.0 - y_sum;

			/*
			printf("y: \n");
			for (int i = 0; i<n; i++) {
			printf("%d: %g\n", index[i], y[i]);
			}
			*/

			//y_orig is ordered such that it corresponds to x
			for(int i=0;i<n;i++){
				y_orig[index[i]] = y[i];
			}

			/*
			printf("y_orig: \n");
			for (int i = 0; i<n; i++) {
				printf("%g\n", y_orig[i]);
			}
			*/
		}

	
		c = d = 0;
		for (int i = 0; i < n; i++) {
			c += 2 * lambda*covariance[i*n + i] * x[i] * y_orig[i] - mu[i] * y_orig[i];
			d += 2 * lambda*covariance[i*n + i] * y_orig[i] * y_orig[i];
			for (int j = i+1; j < n; j++) {
				c += 2 * lambda*covariance[i*n + j]*(x[i]*y_orig[j]+x[j]*y_orig[i]);
				d += 4 * lambda*covariance[i*n + j] * y_orig[i] * y_orig[j];
			}
		}
		s = -c / d;
		// printf("s:%g\n",s);


		if (s <= 0) { printf("Reached optimal"); return returncode; }
		if (s > 1) s = 1;
		for(int i=0;i<n;i++){
			x[i] += s*y_orig[i];
		}

		t++;
	}

	printf("Reached loop limit.\n");


	
	return returncode;
}

int feasible(int n, double *x, double *lb, double *ub)
{
	int returncode = 0, j;
	double sum, increase;

	printf(" computing first feasible solution\n");

	sum = 0;
	/* set variables to lower bounds */
	for (j = 0; j < n; j++){
		x[j] = lb[j];
		sum += lb[j];
	}
	printf("after first pass, sum = %g\n", sum);
	for (j = 0; j < n; j++){
		increase = 1.0 - sum;
		if (increase > ub[j] - x[j])
			increase = ub[j] - x[j];
		x[j] += increase;
		sum += increase;
		//printf("after j = %d, sum = %g\n", j, sum);
		if (sum >= 1.0)
			break;
	}

	if (sum < 1.0) { 
		printf("The problem is infeasible. Quit."); returncode = 100; 
	}else {
		printf("Start with initial values of x:\n");
		/*
		for (int i = 0; i < n; i++) {
			printf("x%d = %g\n", i, x[i]);
		}
		*/
	}

	return returncode;
}