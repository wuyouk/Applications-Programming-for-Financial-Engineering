#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>

#include <gurobi_c++.h>

int optimize(double * eigenvec_matrix, double * factor_covar, double* residual_var, double * mu, int assetNum, int factorNum, double lambda, int namesLimit)
{
  int retcode = 0;
  GRBenv   *env = NULL;
  GRBmodel *model = NULL;
  int n, j;
  double *obj      = NULL;
  double *lb       = NULL;
  double *ub       = NULL;
  double *x;
  int *qrow, *qcol, Nq;
  double *qval;
  int *cind;
  double rhs;
  char sense;
  double *cval;
  int numnonz;
  //set different lambda
  //double lambda = 1.0;
  //int namesLimit = 100;

  char **names, *vartype;

  //assetNum 'x' variables, factorNum 'y' factor variables, assetNum binary variables
  n = 2 * assetNum + factorNum; 
  
  retcode = GRBloadenv(&env, "factormodel.log");
  if (retcode) goto BACK;

 /* Create initial model */
  retcode = GRBnewmodel(env, &model, "second", n, 
                      NULL, NULL, NULL, NULL, NULL);
  if (retcode) goto BACK;

  /** next we create the remaining attributes for the n columns **/
  obj = (double *) calloc (n, sizeof(double));
  ub = (double *) calloc (n, sizeof(double));
  lb = (double *) calloc (n, sizeof(double));
  x = (double *) calloc (n, sizeof(double));
  vartype = (char *)calloc(n, sizeof(char));

  names = (char **)calloc(n, sizeof(char *));
  //define n num of x variables
  for(j = 0; j < assetNum; j++){
    names[j] = (char *)calloc(7, sizeof(char));
    if(names[j] == NULL){
      retcode = 1; goto BACK;
    }
	//store name of each x variables
    sprintf(names[j],"x%d", j);
	//set linear return term in obj : -u * x
	obj[j] = -mu[j];
	//set upper bounds on the x variables
	ub[j] = 0.02;
	lb[j] = 0.0;
  }

  //define r num of y variables
  for(j =assetNum; j <  assetNum + factorNum; j++){
    names[j] = (char *)calloc(7, sizeof(char));
    if(names[j] == NULL){
		  retcode = 1; goto BACK;
    }
	//store name of each y variables (factor variables)
    sprintf(names[j],"y%d", j - assetNum);
	//factor variables bound
	ub[j] = 1000;
	lb[j] = -1000;
  }

  //define n num of z binary variables
  for (j = assetNum + factorNum; j < n; j++) {
	  names[j] = (char *)calloc(7, sizeof(char));
	  if (names[j] == NULL) {
		  retcode = 1; goto BACK;
	  }
	  //store name of each x variables
	  sprintf(names[j], "x%d", j);
	  lb[j] = 0; // redundant because of calloc 
	  ub[j] = 1.0;
	  obj[j] = 0.0; // redundant, again 
  }

  /* initialize variables */
  for(j = 0; j < n; j++){
    retcode = GRBsetstrattrelement(model, "VarName", j, names[j]);
    if (retcode) goto BACK;

    retcode = GRBsetdblattrelement(model, "Obj", j, obj[j]);
    if (retcode) goto BACK;

    retcode = GRBsetdblattrelement(model, "LB", j, lb[j]);
    if (retcode) goto BACK;

    retcode = GRBsetdblattrelement(model, "UB", j, ub[j]);
    
	if (retcode) goto BACK;

	//continuous x and y
	if (j < assetNum + factorNum) vartype[j] = GRB_CONTINUOUS;
	else vartype[j] = GRB_BINARY;
	
	retcode = GRBsetcharattrelement (model, "VTYPE", j, vartype[j]);
	if (retcode) goto BACK;
  }

  //set the quadratic terms in obj
  //there are n + r nonzeroes: n residual variances (D) plus the r factor covariance matrix (F) diagonals
  //333 need to count the number of nonzero residuals
  Nq = assetNum + factorNum;
  qrow = (int *) calloc(Nq, sizeof(int));  /** row indices **/
  qcol = (int *) calloc(Nq, sizeof(int));  /** column indices **/
  qval = (double *) calloc(Nq, sizeof(double));  /** values **/

  if( ( qrow == NULL) || ( qcol == NULL) || (qval == NULL) ){
    printf("could not create quadratic\n");
    retcode = 1; goto BACK;
  }
  //set constaint: l * r
  for (int i = 0; i < assetNum; i++) {
	  qval[i] = lambda * residual_var[i];
	  qrow[i] = i;
	  qcol[i] = i;
  }
  //set constaint: l * F
  for (int i = 0; i < factorNum; i++) {
	  int temp = i + assetNum;
	  qval[temp] = lambda * factor_covar[i*factorNum + i];
	  qrow[temp] = temp;
	  qcol[temp] = temp;
  }

  //add quadratic terms
  retcode = GRBaddqpterms(model, Nq, qrow, qcol, qval);
  if (retcode) goto BACK;

  /** now we will add one constraint at a time **/
  /** we need to have a couple of auxiliary arrays **/

  //cind: index of variables in the constraint
  cind = (int *)calloc(n, sizeof(int));
  //cval: parameter of variables in the constraint
  cval= (double *)calloc(n, sizeof(double));
  //consName: name of constraints
  char * consName= (char *)calloc(n, sizeof(char));
  
  //constraint: Vx - y = 0
  for (int i = 0; i < factorNum; i++) {
	  for (int k = 0; k < assetNum; k++) {
		  cval[k] = eigenvec_matrix[i*assetNum + k];
	  }
	  cval[assetNum + i] = -1;
	  for (j = 0; j < assetNum; j++) cind[j] = j;
	  cind[assetNum + i] = assetNum + i;
	  numnonz = assetNum + 1;
	  rhs = 0;
	  sense = GRB_EQUAL;
	  sprintf(consName, "constraint_%d", i);
	  retcode = GRBaddconstr(model, numnonz, cind, cval, sense, rhs, consName);
	  if (retcode) goto BACK;
  }
  
  //constraint: sum of x variables = 1
  for (int i = 0; i < assetNum; i++) {
	  cval[i] = 1.0;
	  cind[i] = i;
  }
  numnonz = assetNum;
  rhs = 1.0;
  sense = GRB_EQUAL;
  retcode = GRBaddconstr(model, numnonz, cind, cval, sense, rhs, "convexity");
  if (retcode) goto BACK;
  retcode = GRBupdatemodel(model);
  if (retcode) goto BACK;

  //constraint: sum of z variables <= namesLimit
  for (int i = 0; i < assetNum; i++) {
	  cval[i] = 1.0;
	  cind[i] = i + assetNum + factorNum;
  }
  numnonz = assetNum;
  rhs = namesLimit;
  sense = GRB_LESS_EQUAL;
  retcode = GRBaddconstr(model, numnonz, cind, cval, sense, rhs, "names_limit");
  if (retcode) goto BACK;
  retcode = GRBupdatemodel(model);
  if (retcode) goto BACK;

  //constraint: xi = 0 if zi = 0, xi >= 0 if zi = 1
  for (int i = 0; i < assetNum; i++) {
	  cval[0] = 1.0;
	  cind[0] = i;
	  cval[1] = -1.0;
	  cind[1] = i + assetNum + factorNum;
	  numnonz = 2;
	  rhs = 1e-5;
	  sense = GRB_LESS_EQUAL;
	  sprintf(consName, "binary_%d", i);
	  retcode = GRBaddconstr(model, numnonz, cind, cval, sense, rhs, consName);
	  if (retcode) goto BACK;
	  retcode = GRBupdatemodel(model);
	  if (retcode) goto BACK;
  }

  /** optional: write the problem **/

  retcode = GRBwrite(model, "factorwithnames.lp");
  if (retcode) goto BACK;

  //model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);
  retcode = GRBoptimize(model);
  if (retcode) goto BACK;

  /** get solution **/
  retcode = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, n, x);
  if(retcode) goto BACK;

  /** check variable values **/
  for(j = 0; j < n; j++){
    printf("%s = %g\n", names[j], x[j]);
  }
  int count = 0;
  for (j = 0; j < assetNum; j++) {
	  if (x[j] > 0) {
		  count++;
	  }
  }
  printf("\nnumber of positive x %d\n", count);
  GRBfreeenv(env);
 BACK:
  printf("\nexiting with retcode %d\n", retcode);
  return retcode;
}