#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>

void estimateFactor(int day, int asset, double* data, double lambda, int namesLimit);

int main(int argc, char* argv[])
{	
	//russel.csv 947 504
	int retcode = 0;
	FILE *in = NULL, *out = NULL;
	char mybuffer[100];

	if (argc != 6) {
		printf("Usage: main.exe datafilename asset# day# lambda names_limit\n"); retcode = 100; goto BACK;
	}

	//open the file to read
	in = fopen(argv[1], "r");
	if (in == NULL) {
		printf("could not open %s for reading\n", argv[1]);
		retcode = 200; goto BACK;
	}

	int asset = atoi(argv[2]);
	int day = atoi(argv[3]);
	double lambda = atoi(argv[4]);
	int namesLimit = atoi(argv[5]);

	printf("row %d, col %d\n", asset, day);

	double * data = (double *)calloc(asset*day, sizeof(double));
	if (!data) {
		printf("cannot allocate space for data\n");
		retcode = 200; goto BACK;
	}

	//read in data
	double temp = 0;
	for (int i = 0; i < asset; i++) {
		for (int j = 0; j < day; j++) {
			fscanf(in, "%[^,],", mybuffer);
			data[i*day + j] = atof(mybuffer);
		}
	}
	printf("Data read done!");
	fclose(in);

	/*
	for (int i = 0; i < asset; i++) {
		for (int j = 0; j < day; j++) {
			printf("%.2f ", data[i*day + j]);
		}
		printf("\n");
	}
	*/

	//power(day, asset, data, 1000);
	estimateFactor(day, asset, data, lambda, namesLimit);

	//power2(day, asset, data, 1000, 3);
	/* Testing
	double D[3][3] = { {2.0,-0.86,-0.15},{-0.86,3.4,0.48},{-0.15,0.48,0.82} };
	double C[3][3] = { {8,-6,2},{-6,7,-4},{2,-4,3} };
	power((double *)D, 3, 1000);

	*/
	
	free(data);
	
BACK:
	return retcode;
}