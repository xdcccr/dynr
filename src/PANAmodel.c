/**
 * This file implements the Model
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "headers/data_structure.h"
#include "headers/math_function.h"
#include "headers/brekfis.h"
#include "headers/adaodesolver.h"
#include <gsl/gsl_matrix.h>

/**
 * The measurement function
 * @param param mu1 mu2 mu3 lambda21 lambda31
 */
void function_measurement(size_t t, size_t regime, double *param, const gsl_vector *eta, const gsl_vector *co_variate, gsl_matrix *Ht,gsl_vector *y){

    gsl_matrix_set(Ht,0,0,1.0);
    gsl_matrix_set(Ht,1,2,1.0);


    gsl_vector_set(y, 0, gsl_vector_get(eta, 0));
    gsl_vector_set(y, 1, gsl_vector_get(eta, 2));

}

/**
 * The dx/dt function
 */
/**
 * jacobian for dynamic function
 * @param x is state variable
 */
void function_dx_dt(double t, size_t regime, const gsl_vector *x,double *param, const gsl_vector *co_variate, gsl_vector *F_dx_dt){

    /*  
     d1pa<-dpa
     d2pa <--parms[1]*pa+parms[3]*(na-pa)*dpa
     d1na<-dna
     d2na <--parms[2]*na+parms[4]*(pa-na)*dpa     
     */
    gsl_vector_set(F_dx_dt,0,gsl_vector_get(x,1));
    gsl_vector_set(F_dx_dt,1,-param[0]*gsl_vector_get(x,0)+param[2]*(gsl_vector_get(x,2)-gsl_vector_get(x,0))*gsl_vector_get(x,1));
    gsl_vector_set(F_dx_dt,2,gsl_vector_get(x,3));
    gsl_vector_set(F_dx_dt,3,-param[1]*gsl_vector_get(x,2)+param[3]*(gsl_vector_get(x,0)-gsl_vector_get(x,2))*gsl_vector_get(x,3));
}
void function_dynam_ada(const double tstart, const double tend, size_t regime, const gsl_vector *xstart,
        double *gparameters,const gsl_vector *co_variate,
        void (*g)(double, size_t, const gsl_vector *, double *, const gsl_vector *, gsl_vector *),
        gsl_vector *x_tend){

        double tau_max=(tend-tstart)/10;/*specify tau_max*/
        double global_error_limit=10;/*specify global error limit*/
        adaptive_ode_kf(tstart, tend, xstart,tau_max,global_error_limit, regime,  gparameters, co_variate, (*g),x_tend);

}
/**
 * The dF/dx function
 * The partial derivative of the jacobian of the dynamic function with respect to the variable x
 * @param param includes at the end the current state estimates in the same order as the states following the model parameters
 */

void function_dF_dx(double t, size_t regime, double *param, const gsl_vector *co_variate, gsl_matrix *F_dx_dt_dx){


    /*Supply the Jacobian matrix for the ODEs
      ODE functions go down the rows; latent states go across columns*/
    gsl_matrix_set_zero(F_dx_dt_dx);
    
    gsl_matrix_set(F_dx_dt_dx,0,1,1);
    
    gsl_matrix_set(F_dx_dt_dx,1,0,-param[0]-param[2]*param[7]);
    gsl_matrix_set(F_dx_dt_dx,1,1,param[2]*(param[8]-param[6]));
    gsl_matrix_set(F_dx_dt_dx,1,2,param[2]*param[7]);
    
    gsl_matrix_set(F_dx_dt_dx,2,3,1);
    
    gsl_matrix_set(F_dx_dt_dx,3,0,param[3]*param[9]);
    gsl_matrix_set(F_dx_dt_dx,3,2,-param[1]-param[3]*param[9]);
    gsl_matrix_set(F_dx_dt_dx,3,3,param[3]*(param[6]-param[8]));

}



void function_jacobdynamic(const double tstart, const double tend, size_t regime, const gsl_vector *xstart,
        double *param, size_t num_func_param, const gsl_vector *co_variate,
        void (*g)(double, size_t, double *, const gsl_vector *, gsl_matrix *),
	gsl_matrix *Jx){

        size_t np=xstart->size;
        gsl_matrix *k1=gsl_matrix_alloc(np,np);
        gsl_vector *diag=gsl_vector_alloc(np);
        gsl_matrix *k2=gsl_matrix_alloc(np,np);
        gsl_matrix *k3=gsl_matrix_alloc(np,np);
        gsl_matrix *k4=gsl_matrix_alloc(np,np);
        gsl_vector *x1=gsl_vector_alloc(np);
        gsl_vector *x2=gsl_vector_alloc(np);
        gsl_vector *x3=gsl_vector_alloc(np);
        size_t i;
        double params_aug[num_func_param+np];
        for (i=0;i<num_func_param;i++)
            params_aug[i]=param[i];

        /*printf("xstart:");
        print_vector(xstart);*/

	double delta=tend-tstart;
	for (i=0;i<np;i++)
           params_aug[num_func_param+i]=gsl_vector_get(xstart,i);
	(*g)(tstart, regime, params_aug, co_variate, k1);
	mathfunction_diagout_scale(k1,delta/2,diag);
	gsl_vector_memcpy(x1, xstart);
	gsl_vector_add(x1, diag);/*x1<-xstart+delta/2*diag(k1)*/
	gsl_matrix_scale(k1, delta/6);/*k1<-delta/6*k1*/

	for (i=0;i<np;i++)
           params_aug[num_func_param+i]=gsl_vector_get(x1,i);
	(*g)(tstart, regime, params_aug, co_variate, k2);
        mathfunction_diagout_scale(k2,delta/2,diag);
	gsl_vector_memcpy(x2, xstart);
	gsl_vector_add(x2, diag);/*x1<-xstart+delta/2*diagk2*/
	gsl_matrix_scale(k2, delta/3);/*k2<-delta/3*k2*/
	gsl_matrix_add(k1,k2);/*k1<-delta/6*k1+delta/3*k2*/
        for (i=0;i<np;i++)
           params_aug[num_func_param+i]=gsl_vector_get(x2,i);
        (*g)(tstart, regime, params_aug, co_variate, k3);
        mathfunction_diagout_scale(k3,delta,diag);
	gsl_vector_memcpy(x3, xstart);
	gsl_vector_add(x3, diag);/*x3<-xstart+delta*k3*/
        gsl_matrix_scale(k3, delta/3);/*k3<-delta*k3*/
	gsl_matrix_add(k1,k3);/*k1<-delta/6*k1+delta/3*k2+delta/3*k3*/
        for (i=0;i<np;i++)
           params_aug[num_func_param+i]=gsl_vector_get(x3,i);
	(*g)(tstart, regime, params_aug, co_variate, k4);
        gsl_matrix_scale(k4,delta/6);/*k4<-delta/6*k4*/
	gsl_matrix_add(k1,k4);/*k1<-delta/6*k1+delta/3*k2+delta/3*k3+delta/6*k4*/

	gsl_matrix_set_identity(Jx);
	gsl_matrix_add(Jx,k1);

	/*printf("xend:");
	print_vector(x_tend);*/
	gsl_matrix_free(k1);
	gsl_matrix_free(k2);
	gsl_matrix_free(k3);
	gsl_matrix_free(k4);
	gsl_vector_free(x1);
	gsl_vector_free(x2);
	gsl_vector_free(x3);
	gsl_vector_free(diag);

}
/**
 * The dP/dt function
 */

void function_dP_dt(double t, size_t regime, const gsl_vector *p, double *param, const gsl_vector *co_variate, gsl_vector *F_dP_dt){

    size_t nx;
    nx = (size_t) floor(sqrt(2*(double) p->size));

    gsl_matrix *P_mat=gsl_matrix_calloc(nx,nx);
    mathfunction_vec_to_mat(p,P_mat);

    gsl_matrix *F_dx_dt_dx=gsl_matrix_calloc(nx,nx);

    function_dF_dx(t, regime, param, co_variate, F_dx_dt_dx);


    gsl_matrix *dFP=gsl_matrix_calloc(nx,nx);
    gsl_matrix *dP_dt=gsl_matrix_calloc(nx,nx);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, F_dx_dt_dx, P_mat, 0.0, dFP);

    /*printf("P_mat:\n");
    print_matrix(P_mat);
    print_matrix(F_dx_dt_dx);
    print_matrix(dFP);*/

    gsl_matrix_transpose_memcpy(dP_dt, dFP);
    gsl_matrix_add(dP_dt, dFP);

    /*if 0, add small amont so that Ppred invertible*/
    size_t i;
    for(i=0;i<nx;i++){
      gsl_matrix_set(dP_dt,i,i,gsl_matrix_get(dP_dt,i,i)+1e-4);
    }

    /*print_matrix(dP_dt);*/
    mathfunction_mat_to_vec(dP_dt, F_dP_dt);
    /*print_vector(F_dP_dt);
    exit(0);*/

    gsl_matrix_free(P_mat);
    gsl_matrix_free(F_dx_dt_dx);
    gsl_matrix_free(dFP);
    gsl_matrix_free(dP_dt);

}

/**
 * Set the initial condition
 */

void function_initial_condition(double *param, gsl_vector **co_variate, gsl_vector *pr_0, gsl_vector **eta_0, gsl_matrix **error_cov_0){

    gsl_vector_set(pr_0,0,1);

    size_t num_regime=pr_0->size;
    size_t dim_latent_var=error_cov_0[0]->size1;
    size_t num_sbj=(eta_0[0]->size)/(dim_latent_var);
    /*printf("%lu %lu %lu\n",num_regime,dim_latent_var,num_sbj);*/
    size_t i,j;
    for(j=0;j<num_regime;j++){
        for(i=0;i<num_sbj;i++){
            /*printf("%lu %lu\n",i,j);*/
            gsl_vector_set((eta_0)[j],i*dim_latent_var,-0.06391744);
            gsl_vector_set((eta_0)[j],i*dim_latent_var+1,0.29310816);
            gsl_vector_set((eta_0)[j],i*dim_latent_var+2,0.14081910);
            gsl_vector_set((eta_0)[j],i*dim_latent_var+3,-0.14157076);
        }/*statevar_1_p1 statevar_2_p1 statevar_1_p2 statevar_2_p2 ..., eta_0[] with a length of num_sbj*dim_latent_var*/
        
        gsl_matrix_set((error_cov_0)[j],0,0,log(1));
        gsl_matrix_set((error_cov_0)[j],1,1,log(1));
        gsl_matrix_set((error_cov_0)[j],2,2,log(1));
        gsl_matrix_set((error_cov_0)[j],3,3,log(1));
        
        /*gsl_matrix_set((error_cov_0)[j],0,0,log(1.56881678));
        gsl_matrix_set((error_cov_0)[j],1,1,log(7.3848398));
        gsl_matrix_set((error_cov_0)[j],2,2,log(1.83974438));
        gsl_matrix_set((error_cov_0)[j],2,3,log(11.3952947));*/
        /*gsl_matrix_set((error_cov_0)[j],2,1,log(-2.57505534));
        gsl_matrix_set((error_cov_0)[j],1,2,log(-2.57505534));
        gsl_matrix_set((error_cov_0)[j],2,3,log(-2.57505534));
        gsl_matrix_set((error_cov_0)[j],2,3,log(-2.57505534));*/
        
    }
}

/**
 * Set the regime-switch transition probability matrix
 */

void function_regime_switch(size_t t, size_t type, double *param, const gsl_vector *co_variate, gsl_matrix *regime_switch_mat){

    gsl_matrix_set_identity(regime_switch_mat);

}

/**
 * Set the noise covariance matrix
 *They are to be LDL' transformed.
 *e.g., [a b
         b c]
 *-->LDL', L=[1 0;b 1], D=diag(a,c)
 */

void function_noise_cov(size_t t, size_t regime, double *param, gsl_matrix *y_noise_cov, gsl_matrix *eta_noise_cov){
    size_t i;
    for (i=0;i<eta_noise_cov->size1;i++){
        gsl_matrix_set(eta_noise_cov,i,i,-10);
    }
    /*gsl_matrix_set_zero(par.y_noise_cov);*/
    gsl_matrix_set(y_noise_cov,0,0, param[4]);
    gsl_matrix_set(y_noise_cov,1,1, param[5]);

}

/**
 * This function modifies some of the parameters so that it satisfies the model constraint.
 */
void function_transform(const ParamConfig *pc, ParamInit *pi, Param *par){
    size_t i;
    for (i=0;i<2;i++){
      par->func_param[i]=exp(par->func_param[i]);
    }
}

/******************************************************************************************************************/
/******************************************************************************************************************/
/******************************************************************************************************************/



ParamConfig model_configure(){
    ParamConfig pc;
    pc.num_sbj=217;/*number of subjects*/

    /*function specifications*/
    pc.func_measure=function_measurement;
    pc.func_dF_dx=function_dF_dx;
    pc.func_jacobdynamic=function_jacobdynamic;
    pc.func_dx_dt=function_dx_dt;
    pc.func_dP_dt=function_dP_dt;
    pc.func_initial_condition=function_initial_condition;
    pc.func_regime_switch=function_regime_switch;
    pc.func_noise_cov=function_noise_cov;
    pc.isnegloglikeweightedbyT=false;
    pc.second_order=false;/*true;*/
    pc.adaodesolver=false;/*true: use adapative ode solver; false: RK4*/
    if (pc.adaodesolver){
        pc.func_dynam=function_dynam_ada;
    }else{
        pc.func_dynam=rk4_odesolver;
    }


    pc.dim_latent_var=4;/*number of latent variables*/
    pc.dim_obs_var=2;/*number of observed variables*/
    pc.dim_co_variate=1; /*number of covariates*/
    pc.num_func_param=6; /*number of function parameters*/
    pc.num_regime=1;/*number of regimes*/


    pc.index_sbj=(size_t *)malloc((pc.num_sbj+1)*sizeof(size_t *));
    size_t i;

    /*specify the start position for each subject: User always need to provide a txt file called tStart.txt*/
    /*for example, 500 time points for each sbj, specify 0 500 1000 ... 10000 also the end point*/
    /*n subjects -> n+1 indices*/   
    FILE *file_data=fopen("../data/tStartPANAsim.txt","r");
    if (file_data == NULL) {
        perror("fopen");
        printf("-1");
    }

    int errorcheck;
    for(i=0;i<=pc.num_sbj;i++){
           errorcheck=fscanf(file_data,"%lu",pc.index_sbj+i);
            if (errorcheck == EOF) {
                if (ferror(file_data)) {
                    perror("fscanf");
                }
                else {
                    fprintf(stderr, "Error: fscanf reached end of file, no matching characters, no matching failure\n");
                }
                printf("-1");
            }
            else if (errorcheck != 1) {
                fprintf(stderr, "Error: fscanf successfully matched and assigned %i input items\n", errorcheck);
                printf("-1");
            }
        }
    if (fclose(file_data) == EOF) {
        perror("fclose");
        printf("-1");
    }
    
    pc.total_obs=*(pc.index_sbj+pc.num_sbj);/*total observations for all subjects*/

    

    return pc;
}

