#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[])
{
  int i, j, h, runs;
  
  long rows, columns, total;
  double **v = NULL, *w = NULL, *p;

  if(argc < 5){
    printf("usage: sarrays r_or_c_or_w rows columns runs\n"); goto BACK;
  }

  /* default values */
  rows = 1000;
  columns = 100000;
  runs = 1;


  rows = atoi(argv[2]);
  columns = atoi(argv[3]);
  runs = atoi(argv[4]);
  printf("rows = %d columns = %d runs = %d\n", rows, columns, runs);

  total = columns*rows;
  w = (double *)malloc(total*sizeof(double));
  if(w == NULL){
    printf(" not enough memory for w\n"); goto BACK;
  }

  v = (double **) malloc(rows*sizeof(double *));
  for(i = 0; i < rows; i++){
      v[i] = (double *) malloc(columns*sizeof(double));
  }

  printf("total allocated: %ld\n", total);

  if(argv[1][0] == 'r'){

    printf(" by rows\n"); fflush(stdout);

    for(h = 0; h < runs; h++){
      for(i = 0; i < rows; i++){
	for(j = 0; j < columns; j++){

	  v[i][j] = (h+1)*(i+j);
	}
      }


    }

  }
  else if(argv[1][0] == 'c'){

    printf(" by columns\n"); fflush(stdout);

    for(h = 0; h < runs; h++){
      for(j = 0; j < columns; j++){
	for(i = 0; i < rows; i++){

	  v[i][j] = (h+1)*(i+j);
	}
      }
    }
  }
  else if(argv[1][0] == 'w'){

    printf(" by rows, but using the single array\n"); fflush(stdout);

    for(h = 0; h < runs; h++){
      p = w;
      for(i = 0; i < rows; i++){
	for(j = 0; j < columns; j++){
	  *p = (h+1)*(i+j);
	  p += 1;
	}
      }
    }
  }
  
  
  //printf("v[100][100] %g\n", v[100][100]);

  free(w); free(v);
 BACK:
  return 0;
}

/*		  w[i*columns + j] = h*(i+j);*/
