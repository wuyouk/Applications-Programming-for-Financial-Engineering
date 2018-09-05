#include <stdio.h>
#include <stdlib.h>



int main(int argc, char **thelist)
{
  int returncode = 0;
  FILE *input;
  char buffer[1000];

  if (argc != 3){
	  printf(" usage:  arb2  datafile lpfile\n");
	  returncode = 1;
	  goto BACK;
  }

  input = fopen(thelist[1], "r");
  if (input == NULL){
	  printf(" cannot open file %s\n", thelist[1]);
	  returncode = 2;
	  goto BACK;
  }
  fscanf(input, "%s", buffer);

  printf("buffer is %s\n", buffer);

  fclose(input);

  BACK:
  return returncode;
}
