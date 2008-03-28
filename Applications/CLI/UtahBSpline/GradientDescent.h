#ifndef GRADIENTDESCENT_H
#define GRADIENTDESCENT_H

#include "vnl/vnl_cost_function.h"
#include "vnl/vnl_vector.h"
#include <iostream>

//Simple gradient descent
class GradientDescent{

public:
  GradientDescent(){
    tol = 0.000001;
    ltol = 0.3;
    step = 1;
    verbose = 0;
  };

  void Minimize(vnl_cost_function &f, const vnl_vector<double> &xStart){
    double currentf = 0;
    double prevf = 10000;
    
    /*
    double f1 = 0;
    double f2 = 0;
    double s1 = 0;
    double s2 = 0;;;;;




    */


    vnl_vector<double> x = xStart;
    vnl_vector<double> gradient( f.get_number_of_unknowns() ); 
    vnl_vector<double> xtmp = x;

    while( !converged(currentf, prevf, x, gradient, step) ){

      prevf = currentf; 
      f.compute(x, &currentf, &gradient);

      if(prevf < currentf){
        step *=0.85;
      }

   /*   
      //find next stepsize
      double s1 = step;
      xtmp = gradient;
      xtmp *= step;
      xtmp += x;
      f1 = f.f(xtmp);
      
      
      s2 =  2*step;
      xtmp = gradient;
      xtmp *= s2;
      xtmp += x;
      f2 = f.f(xtmp);



      while(f1 > value || fabs(s1- s2) > ltol){
        if(f1 > value ){
          s2 = s1;
          s1 *= 0.5;
          f2 = f1;
          xtmp = gradient;
          xtmp *= s1;
          xtmp += x;
          f1 = f.f(xtmp);
        }
        else if(f2 < f1){
          f1 = f2;
          
          s1 = s2;
          s2 += s2/2;
          xtmp = gradient;
          xtmp *= s2;
          xtmp += x;
          f2 = f.f(xtmp);
        }
        else if(f2 < value){
          s2 = s1 +(s2-s1)/2;
          xtmp = gradient;
          xtmp *= s2;
          xtmp += x;
          f2 = f.f(xtmp);
        }  
        else{
          s2 = s1;
          s1 *= 0.5;
          f2 = f1;
          xtmp = gradient;
          xtmp *= s1;
          xtmp += x;
          f1 = f.f(xtmp);
        }
      
      }
      


      if(f1 < f2){
        step = s1;
      }
      else{
        step = s2;
      }

*/

      if(verbose > 0 ){
        std::cout << "step: " << step << std::endl;
      }
      xtmp = gradient;
      xtmp *= step;
      xtmp += x;
      x = xtmp;
    }
  };  
  

  void SetStepSize(double s){
    step = s;
  };

  void SetTolerance(double t){
    tol = t;
  };

  void SetVerbose(int v){
    verbose = v;
  };
 
protected:
  virtual bool converged(double f, double prevf, const vnl_vector<double> &x,
      const vnl_vector<double> &gradient, double &stepsize){
      if(prevf < f){
        stepsize *=0.85;
      }
      return fabs(f -prevf) < tol;
  };

  

private:
  double tol;
  double ltol;
  double step;
  int verbose;
  
};

#endif

