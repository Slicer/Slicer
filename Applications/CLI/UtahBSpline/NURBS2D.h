#ifndef NURBS2D_H
#define NURBS2D_H

#include "KnotVector.h"
#include "ControlMesh2D.h"
#include "ControlPoint2D.h"


template <typename TCPoint>
class NURBS2D{


  public:
    typedef TCPoint TControlPoint;  
    typedef typename TControlPoint::TPrecision TPrecision;
    typedef ControlMesh2D< TControlPoint > TControlMesh;
    typedef KnotVector<TPrecision> TKnotVector;

    NURBS2D(TControlMesh *cps, TPrecision *uknots, int uLength, TPrecision *vknots, int vLength, int p, int q){
      net = cps;
      this->uKnots =  new TKnotVector(uknots, uLength, p);
      this->vKnots = new TKnotVector(vknots, vLength, q); 
      
      if (uKnots->Length() != uKnots->GetDegree() + net->LengthU() + 1) {
        throw "Nurbs Surface has wrong Knot number in u Direction";
      }
      if (vKnots->Length() != vKnots->GetDegree()  + net->LengthV() + 1 ) {
        throw "Nurbs Surface has wrong Knot number in v Direction";
      }

    };

    NURBS2D(TControlMesh *cps, TKnotVector *uknots, TKnotVector *vknots){
      net = cps;
      this->uKnots = uknots;
      this->vKnots = vknots;

      if (uKnots->Length() != uKnots->GetDegree() + net->LengthU() + 1) {
        throw "Nurbs Surface has wrong Knot number in u Direction";
      }
      if (vKnots->Length() != vKnots->GetDegree()  + net->LengthV() + 1 ) {
        throw "Nurbs Surface has wrong Knot number in v Direction";
      }
    }; 


    void PointAt(const TPrecision u, const TPrecision v, TControlPoint &pOut, 
                        TPrecision *bfu = NULL, TPrecision *bfv = NULL)
    {
      int dummy = 0;
      PointOnSurface(u, v, pOut, dummy, dummy, bfu, bfv);
    }

    void PointAt(const TPrecision u, const TPrecision v, TControlPoint &pOut, 
                        int &uspan, int &vspan, TPrecision *bfu = NULL, TPrecision *bfv = NULL)
    {
      uspan = uKnots->FindSpan(u);
      std::cout << uspan << std::endl;
      bfu = uKnots->BasisFunctions(uspan, u);

      vspan = vKnots->FindSpan(v);
      bfv = vKnots->BasisFunctions(vspan, v);


      int p = uKnots->GetDegree();
      int q = vKnots->GetDegree();
        
      TControlPoint *tmp = new TControlPoint[q + 1];
      for (int l = 0; l <= q; l++) {
        //tmp[l].Set(0, 0, 0);
        for (int k = 0; k <= p; k++) {
          TControlPoint pw = net->Get(uspan - p + k, vspan - q + l);
          pw.Weight();
          tmp[l] += pw*bfu[k];
        }
      }
      
      TControlPoint sw;
      for (int l = 0; l <= q; l++) {
        tmp[l] *= bfv[l];
        sw += tmp[l];
      }
      sw.Unweight();
      pOut = sw;
      
    };


    TKnotVector *GetUKnots(){
      return uKnots;
    };


    TKnotVector *GetVKnots(){
      return vKnots;
    };

    TControlMesh *GetControlNet(){
      return net;
    };

    void SetControlNet(TControlMesh *net){
      this->net = net;
    };


  private:
    TKnotVector *uKnots;
    TKnotVector *vKnots;
    TControlMesh *net;
    

};

#endif
