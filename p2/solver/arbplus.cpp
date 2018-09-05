#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <sys/types.h>
#include <sys/stat.h>

#include <windows.h>

char does_it_exist(char *filename);
int parser(char *sourcefilename, double *solution, int numsec, int *pstatus);
int output(char *solutionfilename, double *solution, int numsec, int status);
int main(int argc, char* argv[])
{
	int retcode = 0;
	FILE *in = NULL, *out = NULL;
	char mybuffer[100];
	int numsec, numscen, j, k, numnonz, solutionstatus;
	double r;
	double *p, optimalvalue, xvalue, *portfolio;
	FILE *results = NULL;
	FILE *histogram = NULL;

	if (argc != 3 + 1) {
		printf("Usage:  arb1.exe datafilename lpfilename histogramfilename\n"); retcode = 100; goto BACK;
	}

	in = fopen(argv[1], "r");
	if (in == NULL) {
		printf("could not open %s for reading\n", argv[1]);
		retcode = 200; goto BACK;
	}

	fscanf(in, "%s", mybuffer);
	fscanf(in, "%s", mybuffer);
	numsec = atoi(mybuffer);
	fscanf(in, "%s", mybuffer);
	fscanf(in, "%s", mybuffer);
	numscen = atoi(mybuffer);
	fscanf(in, "%s", mybuffer);
	fscanf(in, "%s", mybuffer);
	r = atof(mybuffer);

	printf("securities: %d, scenarios: %d;;  r = %g\n",
		numsec, numscen, r);

	p = (double *)calloc((1 + numscen)*(1 + numsec), sizeof(double));
	if (p == NULL) {
		printf("no memory\n"); retcode = 400; goto BACK;
	}
	for (k = 0; k <= numscen; k++) {
		fscanf(in, "%s", mybuffer);
		p[k*(1 + numsec)] = 1 + r*(k != 0);
		for (j = 1; j <= numsec; j++) {
			fscanf(in, "%s", mybuffer);
			p[k*(1 + numsec) + j] = atof(mybuffer);
		}
	}

	fscanf(in, "%s", mybuffer);

	fclose(in);

	out = fopen(argv[2], "w");
	if (out == NULL) {
		printf("can't open %s\n", argv[2]); retcode = 500; goto BACK;
	}
	printf("printing LP to file %s\n", argv[2]);

	fprintf(out, "Minimize ");
	for (j = 0; j <= numsec; j++) {
		if (p[j] >= 0) fprintf(out, "+ "); fprintf(out, "%g x%d ", p[j], j);
	}
	fprintf(out, "\n");
	fprintf(out, "Subject to\n");

	for (k = 1; k <= numscen; k++) {
		fprintf(out, "scen%d: ", k);

		for (j = 0; j <= numsec; j++) {
			if (p[k*(1 + numsec) + j] >= 0) fprintf(out, "+ ");
			fprintf(out, "%g x%d ", p[k*(1 + numsec) + j], j);
		}
		fprintf(out, " >= 0\n");
	}

	fprintf(out, "Bounds\n");
	for (j = 0; j <= numsec; j++) {
		fprintf(out, "-1 <= x%d <= 1\n", j);
	}
	fprintf(out, "End\n");


	fclose(out);


	out = fopen("hidden.dat", "w");
	fclose(out);

	if (does_it_exist("script.py") == 0) {
		printf("need 'script.py'\n"); retcode = 1; goto BACK;
	}

	sprintf(mybuffer, "python script.py %s hidden.dat nothidden.dat", argv[2]);

	printf("mybuffer: %s\n", mybuffer);

	if (does_it_exist("nothidden.dat")) {
		remove("nothidden.dat");
	}

	system(mybuffer);

	/** sleep-wake cycle **/

	for (;;) {
		if (does_it_exist("nothidden.dat")) {
			printf("\ngurobi done!\n");
			Sleep(1000);
			break;
		}
		else {
			Sleep(100);
		}
	}

	portfolio = (double *)calloc(1 + numsec, sizeof(double));
	if (!portfolio) {
		printf("cannot allocate portfolio solution\n");
		retcode = 200; goto BACK;
	}

	/** next, read mygurobi.log **/

	solutionstatus = 700;
	retcode = parser("mygurobi.log", portfolio, numsec, &solutionstatus);
	if (retcode)
		goto BACK;

	retcode = output("solution.dat", portfolio, numsec, solutionstatus);









	histogram = fopen(argv[3], "w");
	int* scores;
	scores = (int *)calloc(1000000, sizeof(int));

	if (!scores) {
		printf("cannot allocate score\n");
		retcode = 1000000; goto BACK;
	}

	int i, s, n;

	int score;
	double value;
	double vol, vp;

	for (i = 0; i < 1000000; i++)
	{
		score = 0;
		for (s = 1; s <= numscen; s++)
		{
			value = 0;
			value += portfolio[0] * p[s*(1 + numsec)];
			for (n = 1; n <= numsec; n++)
			{
				vol = (((double)rand() / (RAND_MAX)) - 0.5)*0.1;
				vp = p[s*(1 + numsec) + n] * vol;
				value += portfolio[n] * vp;
			}
			if (value>0)
			{
				score += 1;
			}
		}
		scores[i] = score;
	}
	fprintf(histogram, "the scores is:");
	for (int i = 0; i < 1000000; i++)
	{
		fprintf(histogram, "%d,", scores[i]);
	}

	free(p);
	free(scores);












BACK:
	return retcode;
}


char does_it_exist(char *filename)
{
	struct stat buf;

	// the function stat returns 0 if the file exists

	if (0 == stat(filename, &buf)) {
		return 1;
	}
	else return 0;
}