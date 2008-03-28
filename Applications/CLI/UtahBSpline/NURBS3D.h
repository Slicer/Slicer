#ifndef NURBSVOLUME_H
#define NURBSVOLUME_H

#include "KnotVector.h"
#include "ControlVolumet.h"
#include "ControlPoint3D.h"


template <typename TCPoint>
class NURBSVolume{


  public:
    typedef TCPoint TControlPoint;  
    typedef typename TControlPoint::TPrecision TPrecision;
    typedef ControlVolume< TControlPoint > TControlVolume;
    typedef KnotVector<TPrecision> TKnotVector;

    NURBSSurface(TControlVolume *cps, TPrecision *uknots, int uLength,
        TPrecision *vknots, int vLength, , TPrecision *wknots, int wLength,  int
        p, int q, int r){

      vol = cps;
      this->uKnots =  new TKnotVector(uknots, uLength, p);
      this->vKnots = new TKnotVector(vknots, vLength, q); 
      this->wKnots = new TKnotVector(wknots, wLength, r); 

      if (uKnots->Length() != uKnots->GetDegree() + vol->LengthU() + 1) {
        throw "Nurbs Surface has wrong Knot number in u Direction";
      }
      if (vKnots->Length() != vKnots->GetDegree()  + vol->LengthV() + 1 ) {
        throw "Nurbs Surface has wrong Knot number in v Direction";
      }
      if (wKnots->Length() != wKnots->GetDegree()  + volt->LengthW() + 1 ) {
        throw "Nurbs Surface has wrong Knot number in w Direction";
      }


    };

    NURBSSurface(TControlNet *cps, TKnotVector *uknots, TKnotVector *vknots,
        TKnotVector *wknots){
      vol = cps;
      this->uKnots = uknots;
      this->vKnots = vknots;
      this->wKnots = wknots;

      if (uKnots->Length() != uKnots->GetDegree() + net->LengthU() + 1) {
        throw "Nurbs Surface has wrong Knot number in u Direction";
      }
      if (vKnots->Length() != vKnots->GetDegree()  + net->LengthV() + 1 ) {
        throw "Nurbs Surface has wrong Knot number in v Direction";
      }
      if (wKnots->Length() != wKnots->GetDegree()  + volt->LengthW() + 1 ) {
        throw "Nurbs Surface has wrong Knot number in w Direction";
      }


    }; 


    void PointOnVolume(const TPrecision u, const TPrecision v, const TPrecision
        w, TControlPoint &pOut, TPrecision *bfu = NULL, TPrecision *bfv = NULL,
        , TPrecision *bfw = NULL))
    {
      int dummy = 0;
      PointOnSurface(u, v, w, pOut, dummy, dummy, bfu, bfv, bfw);
    }

    void PointOnSurface(const TPrecision u, const TPrecision v, const TPrecision
        w, TControlPoint &pOut, int &uspan, int &vspan, int &wspan, TPrecision
        *bfu = NULL, TPrecision *bfv = NULL, TPrecision *bfw = NULL)
    {
      uspan = uKnots->FindSpan(u);
      bfu = uKnots->BasisFunctions(uspan, u);

      vspan = vKnots->FindSpan(v);
      bfv = vKnots->BasisFunctions(vspan, v);

      wspan = wKnots->FindSpan(v);
      bfw = wKnots->BasisFunctions(wspan, w);
  
      int p = uKnots->GetDegree();
      int q = vKnots->GetDegree();
      int r = wKnots->GetDegree();
 
      TControlPoint **gamma = new TControlPoint[r + 1][q + 1];
      for(int t = 0; t <= r; t++){
        for (int l = 0; l <= q; l++) {
          //tmp[l].Set(0, 0, 0);
          for (int k = 0; k <= p; k++) {
            TControlPoint pw = vol->Get(uspan - p + k, vspan - q + l, wspan - r + t);
            pw.Weight();
            gamma[t][l] += pw*bfu[k];
          }
        }
      }
      
      TControlPoint *lambda = new TControlPoint[r+1];
      for (int t = 0; t <= r; t++) {
        for (int l = 0; l <= q; l++) {
          lamda[l] += bfv[l] * gamma[t][l];
        }
      }

      TControlPoint sw;
      for (int t = 0; t <= r; t++) {
        lambda[t] *= bfw[t];
        sw += lambda[t];
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

    TKnotVector *GetWKnots(){
      return wKnots;
    };

    TControlVolume *GetControlVolume(){
      return vol;
    };

    void SetControlVolume(TControlVolume *vol){
      this->vol = vol;
    };


  private:
    TKnotVector *uKnots;
    TKnotVector *vKnots;
    TControlNet *vol;
    

};

#endif
