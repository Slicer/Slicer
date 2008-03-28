#ifndef BSPLINE3D_H
#define BSPLINE3D_H

#include "KnotVector.h"
#include "ControlMesh3D.h"

#include <iostream>


template <typename TCPoint>
class BSpline3D{


  public:
    typedef TCPoint TControlPoint;
    typedef typename TControlPoint::TPrecision TPrecision;
    typedef ControlMesh3D< TControlPoint > TControlMesh;
    typedef KnotVector<TPrecision> TKnotVector;


    BSpline3D(){
    };

    BSpline3D(  TControlMesh *cps, TPrecision *uknots, int uLength,
                TPrecision *vknots, int vLength, TPrecision *wknots, int wLength,  
                int p, int q, int r)
    {
      mesh = cps;
      this->uKnots = new TKnotVector(uknots, uLength, p);
      this->vKnots = new TKnotVector(vknots, vLength, q); 
      this->wKnots = new TKnotVector(wknots, wLength, r); 

      
      if( !validate()) throw "ControlPoints and Knotvetcors do not match";
    };

    
    BSpline3D(  TControlMesh &cps, TKnotVector &uknots, TKnotVector &vknots,
                TKnotVector &wknots)
    {
      mesh = cps;
      this->uKnots = uknots;
      this->vKnots = vknots;
      this->wKnots = wknots;

      if( !validate()) throw "ControlPoints and Knotvetcors do not match";
    }; 

    bool validate(){
      if (uKnots.Length() != uKnots.GetDegree() + mesh.LengthU() + 1) {
        return false;
      }
      if (vKnots.Length() != vKnots.GetDegree()  + mesh.LengthV() + 1 ) {
        return false;
      }
      if (wKnots.Length() != wKnots.GetDegree()  + mesh.LengthW() + 1 ) {
        return false;
      }

      return true;

    };
    
    void PointAt( const TPrecision u, const TPrecision v, const TPrecision w, 
                  TControlPoint &pOut, TPrecision *bfu = NULL, TPrecision *bfv = NULL,
                  TPrecision *bfw = NULL)
    {
      static int dummy1 = 0;
      static int dummy2 = 0;
      static int dummy3 = 0;

      PointOnAt(u, v, w, pOut, dummy1, dummy2, dummy3, bfu, bfv, bfw);
    };

    
    void PointAt(const TPrecision u, const TPrecision  v, const
        TPrecision  w, TControlPoint &pOut, int &uspan, int &vspan, int &wspan, 
        TPrecision  *bfu = NULL, TPrecision *bfv = NULL, TPrecision *bfw = NULL)
    
    {
      
      int p = uKnots.GetDegree();
      int q = vKnots.GetDegree();
      int r = wKnots.GetDegree();

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

      wspan = wKnots.FindSpan(w);
      if(bfw == NULL){
        bfw = wKnots.BasisFunctions(wspan, w);
      }
      else{
        wKnots.BasisFunctions(wspan, w, bfw);
      }


      pOut.Zero();
      TControlPoint tmp1;
      TControlPoint tmp2;
      for(int t=0; t<=r; t++){
        int wind = wspan -r +t;
        tmp2.Zero();
        for (int l = 0; l <= q; l++) {
          tmp1.Zero();
          int vind = vspan - q + l;
          for (int k = 0; k <= p; k++) {
            TControlPoint &cp = mesh.Get(uind + k, vind, wind);
            tmp1 += cp * bfu[k];
          }
          tmp2 += tmp1 * bfv[l];
        }
        pOut += tmp2 *bfw[t];
      }
    };


    
    int GetDegreeU(){
      return uKnots.GetDegree();
    };

    int GetDegreeV(){
      return vKnots.GetDegree();
    };

    int GetDegreeW(){
      return wKnots.GetDegree();
    };

    TKnotVector &GetUKnots(){
      return uKnots;
    };

    
    TKnotVector &GetVKnots(){
      return vKnots;
    };

    TKnotVector &GetWKnots(){
      return wKnots;
    };

    TControlMesh &GetControlMesh(){
      return mesh;
    };

    bool SetControlMesh(TControlMesh &cmesh){
      TControlMesh &old = mesh;
      this->mesh = cmesh;
      if( !validate() ){
        this->mesh = old;
        return false;
      }
      return true;
    };
  
    static BSpline3D<TCPoint> createIdentity( TPrecision *index,
                      TPrecision *size, int *nControlPoints, int *degree);
 
    static void fit(TPrecision x1, TPrecision x2, TPrecision nC, 
                    TKnotVector &knots, TPrecision &h, TPrecision &start);

    //Operators
    BSpline3D<TCPoint>& operator=(const BSpline3D<TCPoint>& rhs){

      uKnots = rhs.uKnots;
      vKnots = rhs.vKnots;
      wKnots = rhs.wKnots;
      mesh = rhs.mesh;      
      return *this;
    };
    

    friend std::ostream& operator << (std::ostream& os, BSpline3D<TCPoint>& bspline){
       os << bspline.GetUKnots() << std::endl;
       os << bspline.GetVKnots() << std::endl;
       os << bspline.GetWKnots() << std::endl;
       os << bspline.GetControlMesh() << std::endl;
       std::cout << "written bspline" << std::endl;

       return os;
    };

    friend std::istream& operator >> (std::istream& is, BSpline3D<TCPoint>& bspline){
        is >> bspline.GetUKnots();
        is >> bspline.GetVKnots();
        is >> bspline.GetWKnots();
        is >> bspline.GetControlMesh();
        return is;
    };

  private:
    TKnotVector uKnots;
    TKnotVector vKnots;
    TKnotVector wKnots;
    TControlMesh mesh;

  
};






template <typename T>
BSpline3D<T> 
BSpline3D<T>::createIdentity(BSpline3D<T>::TPrecision *index, BSpline3D<T>::TPrecision *size, int *nControlPoints, 
                             int *degree)

{

  int ulength = 0;
  TKnotVector uknots =
    TKnotVector::createUniformKnotsUnclamped(nControlPoints[0], degree[0], ulength);
  int vlength = 0;
  TKnotVector vknots =
    TKnotVector::createUniformKnotsUnclamped(nControlPoints[1], degree[1], vlength);
  int wlength = 0;
  TKnotVector wknots =
    TKnotVector::createUniformKnotsUnclamped(nControlPoints[2], degree[2], wlength);

  
  //Compute control grid start location
  TPrecision hu = 0;
  TPrecision xStart = 0;
  fit(index[0], index[0]+size[0], nControlPoints[0], uknots, hu, xStart);

  TPrecision hv = 0;
  TPrecision yStart = 0;
  fit(index[1], index[1]+size[1], nControlPoints[1], vknots, hv, yStart);
  
  TPrecision hw = 0;
  TPrecision zStart = 0;
  fit(index[2], index[2]+size[2], nControlPoints[2], wknots, hw, zStart);

  TControlMesh net( nControlPoints[0], nControlPoints[1], nControlPoints[2]); 
  for(int i = 0; i < nControlPoints[0]; i++){
    for( int j = 0; j < nControlPoints[1]; j++){
      for( int k = 0; k < nControlPoints[2]; k++){
        TControlPoint tmp(xStart + i * hu, yStart + j * hv, zStart + k * hw);
        net.Set(i, j, k, tmp);
      }
    }
  }

 BSpline3D<T>  result(net, uknots, vknots, wknots);
 return result; 
}



template <typename T>
void 
BSpline3D<T>::fit(BSpline3D<T>::TPrecision x1,
    BSpline3D<T>::TPrecision x2, BSpline3D<T>::TPrecision nC,
    BSpline3D<T>::TKnotVector &knots, BSpline3D<T>::TPrecision &h, 
    BSpline3D<T>::TPrecision &start){

  int degree = knots.GetDegree();

  TPrecision  u1 = knots.GetKnotA();
  int span1 = knots.FindSpan(u1);
  TPrecision  *b1 = knots.BasisFunctions(span1, u1); 
  
  TPrecision  u2 = knots.GetKnotB();
  int span2 = knots.FindSpan(u2);
  TPrecision  *b2 = knots.BasisFunctions(span2, u2);
  

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
  h = h / (nC-1);
  
  TPrecision h2 = 0;
  for(int i=1; i <=degree; i++){
    h2 += i*b1[i]*h;
  }
  start =  (x1 - h2)/s1;
  
  std::cout << "h: " << h << std::endl;
  std::cout << "start " << start << std::endl;
}


#endif
