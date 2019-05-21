/*
Authors: Hui-Ju Hung
Date: 2019-05-20
Filename: mainSAEM.c
Purpose: Hello World for integrating SAEM and dynr 
*/

#include <math.h>/*sqrt(double)*/
/*#include <stdio.h>*/
#include <string.h>
#include "nlopt.h"
#include "math_function.h"
#include "ekf.h"
#include "data_structure.h"
#include "brekfis.h"
#include "adaodesolver.h"
#include "model.h"
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include "wrappernegloglike.h"
#include "numeric_derivatives.h"
#include "estimation.h"
#include <R.h>
#include <Rinternals.h>
#include <Rmath.h>
#include <Rdefines.h>
#include "print_function.h"

/* get the list element named str, or return NULL */
/*
SEXP getListElement(SEXP list, const char *str)
{
    SEXP elmt = R_NilValue, names = getAttrib(list, R_NamesSymbol);
    size_t i;
    for (i = 0; i < length(list); i++){
		if(strcmp(CHAR(STRING_ELT(names, i)), str) == 0) {
			elmt = VECTOR_ELT(list, i);
			break;
		}
	}
	return elmt;
}*/
/**
 * The gateway function for the R interface
 * @param model_list is a list in R of all model specifications.
 * @param data_list is a list in R of the outputs prepared by dynr.data()
 * @param weight_flag_in a flag for weighting the neg loglike function by individual data length
 * @param debug_flag_in a flag for returning a longer list of outputs for debugging purposes
 * @param optimization_flag_in a flag for running optimization
 * @param hessian_flag_in a flag for calculating hessian matrix
 * @param verbose_flag_in a flag of whether or not to print debugging statements before and during estimation.
 */
SEXP main_SAEM(SEXP model_list, SEXP data_list, SEXP weight_flag_in, SEXP debug_flag_in, SEXP optimization_flag_in, SEXP hessian_flag_in, SEXP verbose_flag_in)
{
	/*
    size_t index,index_col,index_row;
    bool debug_flag=*LOGICAL(PROTECT(debug_flag_in));
	bool optimization_flag=*LOGICAL(PROTECT(optimization_flag_in));
	bool hessian_flag=*LOGICAL(PROTECT(hessian_flag_in));
	bool verbose_flag=*LOGICAL(PROTECT(verbose_flag_in));
	bool weight_flag=*LOGICAL(PROTECT(weight_flag_in));
	*/
    /** =======================Interface : Start to Set up the data and the model========================= **/

   printf("Hello world! from SAEM\n");
   return model_list;
}




