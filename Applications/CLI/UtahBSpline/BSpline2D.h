#ifndef BSPLINE2D_H
#define BSPLINE2D_H

#include "KnotVector.h"
#include "ControlMesh2D.h"

#include <iostream>


template <typename TCPoint>
class BSpline2D{


  public:
    typedef TCPoint TControlPoint;
    typedef typename TControlPoint::TPrecision TPrecision;
    typedef ControlMesh2D< TControlPoint > TControlMesh;
    typedef KnotVector<TPrecision> TKnotVector;


    BSpline2D(){
    };

    BSpline2D(TControlMesh &cps, TPrecision *uknots, 
                   int uLength, TPrecision *vknots, int vLength, 
                   int p, int q)
    {
      net = cps;
      this->uKnots =  TKnotVector(uknots, uLength, p);
      this->vKnots = TKnotVector(vknots, vLength, q); 
      
      if( !validate()) throw "ControlPoints and Knotvetcors do not match";
    };


    BSpline2D(TControlMesh &cps, TKnotVector &uknots, TKnotVector &vknots){
      net = cps;
      this->uKnots = uknots;
      this->vKnots = vknots;
      
      if( !validate()) throw "ControlPoints and Knotvetcors do not match";
    }; 

    bool validate(){
      if (uKnots.Length() != uKnots.GetDegree() + net.LengthU() + 1) {
        return false;
      }
      if (vKnots.Length() != vKnots.GetDegree()  + net.LengthV() + 1 ) {
        return false;
      }
      return true;

    };
    
    void PointAt(const TPrecision u, const TPrecision v, TControlPoint &pOut, 
                        TPrecision *bfu = NULL, TPrecision *bfv = NULL)
    {
      int dummy1 = 0;
      int dummy2 = 0;
      PointAt(u, v, pOut, dummy1, dummy2, bfu, bfv);
    };


    void PointAt(const TPrecision u, const TPrecision  v, TControlPoint
        &pOut, int &uspan, int &vspan, TPrecision  *bfu = NULL, TPrecision *bfv = NULL)
    
    {
      
      int p = uKnots.GetDegree();
      int q = vKnots.GetDegree();

      
      uspan = uKnots.FindSpan(u);
      if(bfu == NULL){
        bfu = uKnots.BasisFunctions(uspan, u);
      }
      else{
        uKnots.BasisFunctions(uspan, u, bfu);
      }
      int uind = uspan - p;

      vspan = vKnots.FindSpan(v);
      if(bfv == NULL){
        bfv = vKnots.BasisFunctions(vspan, v);
      }
      else{
        vKnots.BasisFunctions(vspan, v, bfv);
      }
      
      //std::cout << "u: " << u <<" v: " << v;
      //std::cout << "uspan " << uspan << " vspan " << vspan << std::endl;

      pOut.Zero();
      TControlPoint tmp;
      for (int l = 0; l <= q; l++) {
        tmp.Zero();
        int vind = vspan - q + l;
        for (int k = 0; k <= p; k++) {
          TControlPoint &cp = net.Get(uind + k, vind);
          tmp += cp * bfu[k];
        }
        pOut += tmp * bfv[l];
      }
    };

    int GetDegreeU(){
      return uKnots.GetDegree();
    };

    int GetDegreeV(){
      return vKnots.GetDegree();
    };

    TKnotVector &GetUKnots(){
      return uKnots;
    };


    TKnotVector &GetVKnots(){
      return vKnots;
    };

    TControlMesh &GetControlMesh(){
      return net;
    };
    
    bool SetControlMesh(TControlMesh &cnet){
      this->net = cnet;
      return validate();
    };
 
    
    static BSpline2D<TCPoint> createIdentity( TPrecision *index, TPrecision *size,
                                              int *nControlPoints, int *degree);
 
    static void fit(TPrecision x1, TPrecision x2, TPrecision nC, 
                    TKnotVector &knots, TPrecision &h, TPrecision &start);

    

    
    //Operators
    BSpline2D<TCPoint>& operator=(const BSpline2D<TCPoint>& rhs){

      uKnots = rhs.uKnots;
      vKnots = rhs.vKnots;
      net = rhs.net;      
      return *this;
    };

     

    friend std::ostream& operator << (std::ostream& os, BSpline2D<TCPoint>& surface){
       os << surface.GetUKnots() << std::endl;
       os << surface.GetVKnots() << std::endl;
       os << surface.GetControlMesh() << std::endl;

       return os;
    };

    friend std::istream& operator >> (std::istream& is, BSpline2D<TCPoint>& surface){
        is >> surface.GetUKnots();
        is >> surface.GetVKnots();
        is >> surface.GetControlMesh();
        return is;
    };
  
  private:
    TKnotVector uKnots;
    TKnotVector vKnots;
    TControlMesh net;

};



template <typename T>
BSpline2D<T> 
BSpline2D<T>::createIdentity(BSpline2D<T>::TPrecision *index,
    BSpline2D<T>::TPrecision *size,  int *nControlPoints, int *degree)

{

  int ulength = 0;
  TKnotVector uknots =
    TKnotVector::createUniformKnotsUnclamped(nControlPoints[0], degree[0], ulength);
  int vlength = 0;
  TKnotVector vknots =
    TKnotVector::createUniformKnotsUnclamped(nControlPoints[1], degree[1], vlength);
  
  //Compute control grid start location
  TPrecision hu = 0;
  TPrecision xStart = 0;
  fit(index[0], index[0]+size[0], nControlPoints[0], uknots, hu, xStart);

  TPrecision hv = 0;
  TPrecision yStart = 0;
  fit(index[1], index[1]+size[1], nControlPoints[1], vknots, hv, yStart);
  
  
  TControlMesh net( nControlPoints[0], nControlPoints[1]); 
  for(int i = 0; i < nControlPoints[0]; i++){
    for( int j = 0; j < nControlPoints[1]; j++){
      TControlPoint tmp(xStart + i * hu, yStart + j * hv);
      net.Set(i, j, tmp);
    }
  }

 BSpline2D<T>  result(net, uknots, vknots);
 return result; 
}

template <typename T>
void 
BSpline2D<T>::fit(BSpline2D<T>::TPrecision x1,
    BSpline2D<T>::TPrecision x2, BSpline2D<T>::TPrecision nC,
    BSpline2D<T>::TKnotVector &knots, BSpline2D<T>::TPrecision &h, 
    BSpline2D<T>::TPrecision &start){

  int degree = knots.GetDegree();

  TPrecision  u1 = knots.GetKnotA();
  int span1 = knots.FindSpan(u1);
  TPrecision  *b1 = knots.BasisFunctions(span1, u1); 
  
  TPrecision  u2 = knots.GetKnotB();
  int span2 = knots.FindSpan(u2);
  TPrecision  *b2 = knots.BasisFunctions(span2, u2);
  
  std::cout << "u1: " << u1 <<" u2: " << u2 << std::endl;
  std::cout << "span1 " << span1 << " span2 " << span2 << std::endl;

  TPrecision  s1 = 0;
  TPrecision  sb1 = 0;

  TPrecision  s2 = 0;
  TPrecision  sb2 = 0;  
  
  for(int i = 0; i <= degree; i++){
    s1 += b1[i];
    sb1 += b1[i] * (span1 - degree + i);
    s2 += b2[i];
    sb2 += b2[i] * (span2 - degree + i);
  }
  

  h = (span2 - span1+degree)*(x2 - x1) / (sb2 - sb1);
  //TPrecision start2 =  span2 + span1-degree - h + (x2-x1)/2;
  h = h / (nC-1);
  
  TPrecision h2 = 0;
  for(int i=1; i <=degree; i++){
    h2 += i*b1[i]*h;
  }
  start =  (x1 - h2)/s1;
  
  std::cout << "h: " << h << std::endl;
  std::cout << "start " << start << std::endl;
  //h = ( (x1 - x2) *s1/s2 ) / ( sb1 - sb2 * s1 / s2);
  //start =  (x2 - h*sb2) / s2;
}
#endif
