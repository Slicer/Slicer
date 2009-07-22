#ifndef NOMRALIZEDCORRELATIONCOSTFUNCTION3D_H
#define NOMRALIZEDCORRELATIONCOSTFUNCTION3D_H

#include "vnl/vnl_cost_function.h"
#include "vnl/vnl_least_squares_function.h"



#include "itkSubtractImageFilter.h"
#include "itkGradientImageFilter.h"
#include "itkDerivativeImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkLinearInterpolateImageFunction.h"

#include "Point3D.h"
#include "BSpline3D.h"
#include "ParametricImageTransformation3D.h"
#include "ControlMesh3D.h"
#include "KnotVector.h"


template <typename TPrecision, typename TImage>
class NormalizedCorrelationCostFunction3D : public vnl_cost_function {

    
  public:
    typedef Point3D< TPrecision> TControlPoint;
    typedef BSpline3D< TControlPoint , TPrecision> TParametric;
    typedef ParametricImageTransformation3D<TParametric, TImage> TImageTransformation;
    typedef typename TParametric::TControlMesh TControlMesh;
    typedef typename TParametric::TKnotVector TKnotVector;

    typedef typename TImageTransformation::Image Image;
    typedef typename Image::Pointer ImagePointer;
    typedef typename Image::IndexType ImageIndex;
    typedef typename Image::RegionType ImageRegion;
    typedef typename Image::PointType ImagePoint;
    typedef typename ImageRegion::SizeType ImageSize;
    typedef typename ImageSize::SizeValueType ImageSizeValue;


   
    NormalizedCorrelationCostFunction3D(int
        nUnknowns):vnl_cost_function(nUnknowns){
      
      useMask =  false;
      fixedBoundary = true;
      alpha = 0;
      verbose = 0;
    };
    
    double f(vnl_vector<double> const &x);
    void gradf(vnl_vector<double> const &x, vnl_vector<double> &g);
    void compute(vnl_vector<double> const &x, double *f, vnl_vector<double> *g);

    void GetVNLParametersFromParametric(vnl_vector<double> &params); 
    void SetParametricFromVNLParameters(vnl_vector<double> const &params);



    
    //Set / Get Methods
    void SetFixedImage(ImagePointer fixed){ 
      this->fixedImage = fixed;

      CastFilterPointer castFilter = CastFilter::New();
      castFilter->SetInput( fixedImage );
      castFilter->Update();
      movingTransformedImage = castFilter->GetOutput();
    };
   
    void SetMaskImage(ImagePointer maskImage){
      this->maskImage = maskImage;
    };

    void SetMovingImage(ImagePointer moving){
      this->movingImage = moving;  
    };
    
    void SetRange(const ImageRegion &range){ 
      this->transform.SetRange(range); 
    };


    void ComputePhysicalRange(ImageRegion range){
      transform.ComputePhysicalRange(fixedImage, range);
    }

    ImagePoint GetStart(){
      return transform.GetStart();
    };
 

    ImagePoint GetSize(){
       return transform.GetSize();
    };
    
    ImageRegion GetRange(){
       return transform.GetRange();
    };


    void SetParametric(TParametric &parametric){
      this->transform.SetParametric( parametric );
    };

    
    void SetUseMask(bool use){
      useMask = use;
    };
    
    void SetFixedBoundary(bool fixed){
      fixedBoundary = fixed;
    };


    void SetAlpha(TPrecision a){
      alpha = a;
    };

    TImageTransformation& GetTransformation(){
      return transform;
    };
    

    void SetVerbose(int level){
      verbose = level;
    }

    void Init();
    

    //Operators
    NormalizedCorrelationCostFunction3D<TPrecision, TImage>& operator=(const
        NormalizedCorrelationCostFunction3D<TPrecision, TImage>& rhs){
    
       transform  = rhs.transfrom;

       fixedImage = rhs.fixedImage;
       movingImage = rhs.movingImage;
       movingTransformedImage =rhs.movingTransformedImage;
       differenceImage = rhs.differenceImage;
       movingGradient = rhs.movingGradient;

       useMask = rhs.useMask;
       xDistance = rhs.xDistance;
       yDistance = rhs.yDistance;
       zDistance = rhs.zDistance;

       alpha = rhs.alpha;

    };


  private:


    typedef itk::LinearInterpolateImageFunction<Image, double> InterpolateFunction;
    typedef typename InterpolateFunction::Pointer InterpolateFunctionPointer;
    typedef typename InterpolateFunction::ContinuousIndexType ImageContinuousIndex;
    typedef typename itk::ImageRegionIteratorWithIndex<Image> ImageIterator;
       
    typedef itk::CastImageFilter<Image, Image> CastFilter;
    typedef typename CastFilter::Pointer CastFilterPointer;
    
//    typedef itk::DerivativeImageFilter<Image, Image> DerivativeFilter;
//    typedef typename DerivativeFilter::Pointer DerivativeFilterPointer;
    typedef itk::GradientImageFilter<Image, TPrecision, TPrecision> GradientFilter;
    typedef typename GradientFilter::Pointer GradientFilterPointer;
    
    typedef typename GradientFilter::OutputImageType GradientImage;
    typedef typename GradientImage::Pointer GradientImagePointer;
    typedef typename GradientImage::PixelType GradientVector;

    typedef typename itk::ImageRegionIterator<GradientImage> GradientImageIterator;

    typedef itk::SubtractImageFilter<Image, Image, Image> SubtractFilter;
    typedef typename SubtractFilter::Pointer SubtractFilterPointer;



    ImagePointer fixedImage;
    ImagePointer movingImage;
    ImagePointer maskImage;
    ImagePointer movingTransformedImage;
    ImagePointer differenceImage;
    GradientImagePointer movingGradient;


    
    TImageTransformation transform;

    bool useMask;
    bool fixedBoundary;
    TPrecision xDistance;
    TPrecision yDistance;
    TPrecision zDistance;

    TPrecision alpha;

    void ComputeTransformedImage();
    void ComputeDerivativeImages();

    double f();
    void gradf(vnl_vector<double> &g);

    int verbose;


};


//non-inline implementations
template <typename TPrecision, typename TImage>
void 
NormalizedCorrelationCostFunction3D<TPrecision, TImage>::GetVNLParametersFromParametric(vnl_vector<double> &params)
{
  TParametric &parametric = transform.GetParametric();
  TControlMesh &net = parametric.GetControlMesh();
  int index = 0;
  for(int i = 0; i < net.LengthU(); i++){
    for(int j = 0; j < net.LengthV(); j++){
      for(int k = 0; k < net.LengthW(); k++){
        TControlPoint &tmp = net.Get(i, j, k);
        params[index++] = tmp.x;
        params[index++] = tmp.y;
        params[index++] = tmp.z;
      }
    }
  }
}


template <typename TPrecision, typename TImage>
void 
NormalizedCorrelationCostFunction3D<TPrecision, TImage>::SetParametricFromVNLParameters(vnl_vector<double> const &params)
{

  TParametric &parametric = transform.GetParametric();
  TControlMesh &net = parametric.GetControlMesh();

  int index = 0;
  for(int i = 0; i < net.LengthU(); i++){
    for(int j = 0; j < net.LengthV(); j++){
      for(int k = 0; k < net.LengthW(); k++){
        TControlPoint &tmp = net.Get(i, j, k);
        tmp.x = params[index++];
        tmp.y = params[index++];
        tmp.z = params[index++];
        //net.Set(i, j, k, tmp);
      }
    }
  }
}


template <typename TPrecision, typename TImage>
void 
NormalizedCorrelationCostFunction3D<TPrecision, TImage>::ComputeTransformedImage()
{
  transform.Transform( movingTransformedImage, movingImage );
 
  SubtractFilterPointer subtractFilter = SubtractFilter::New();
  subtractFilter->SetInput1( fixedImage );
  subtractFilter->SetInput2( movingTransformedImage );
  subtractFilter->Modified();
  subtractFilter->Update();
  differenceImage = subtractFilter->GetOutput();
  
  
}



template <typename TPrecision, typename TImage>
void 
NormalizedCorrelationCostFunction3D<TPrecision, TImage>::ComputeDerivativeImages(){


  GradientFilterPointer gradientFilter = GradientFilter::New();
  gradientFilter->SetInput(movingTransformedImage);
  gradientFilter->Update();
  movingGradient = gradientFilter->GetOutput();

};

template <typename TPrecision, typename TImage>
double 
NormalizedCorrelationCostFunction3D<TPrecision, TImage>::f(vnl_vector<double> const &x){
  SetParametricFromVNLParameters(x); 
  ComputeTransformedImage();
  return f();
};

template <typename TPrecision, typename TImage>
double 
NormalizedCorrelationCostFunction3D<TPrecision, TImage>::f(){
  double val = 0;
  double tmp = 0;

  ImageRegion region = transform.GetRange();
  ImageIterator it( differenceImage, region);
  ImageIterator maskIt(maskImage, region);

  int nInMask = 0;
  for(it.GoToBegin(); !it.IsAtEnd(); ++it, ++maskIt ){
      if(!useMask || maskIt.Get() != 0){
        nInMask++;
        tmp =  it.Get();
        val += tmp * tmp;
      }
  }
  ImageSize regionSize = region.GetSize();
  val = val / nInMask;

  if(verbose > 0){
    std::cout <<"f: " << (1-alpha)* val << std::endl;
  }

  //Regularization term
  TParametric &parametric = transform.GetParametric();
  TControlMesh &net = parametric.GetControlMesh();
  double regTmp = 0;
  double reg = 0;
  
  if(alpha != 0){
  for(int i = 0; i < net.LengthU(); i++){
    for(int j = 0; j < net.LengthV(); j++){
      for(int k = 0; k < net.LengthW(); k++){
        
        TControlPoint &current = net.Get(i, j, k);
        regTmp = 0;
        int nP = 0;
        for(int l=-1; l<2; l++){
          for(int m = -1; m<2; m++){
            for(int n = -1; n<2; n++){

              if(l == 0 && n == 0 && m == 0){
                  continue;
              }
              if(net.CheckIndex(i+l, j+n, k+m)){
               TControlPoint &neighbor = net.Get(i+l, j+n, k+m);
                tmp = neighbor.x - current.x - l*xDistance;
                regTmp+= tmp*tmp;
                tmp = neighbor.y - current.y - n*yDistance;
                regTmp+=tmp*tmp;
                tmp = neighbor.z - current.z - m*zDistance;
                regTmp+=tmp*tmp;
                nP++;
              }
            }
          } 
        }
        reg += regTmp / nP; 
      }
    }
  }
  }
  int nCps = net.LengthU() * net.LengthV() * net.LengthW();
  
  reg = reg/nCps;

  val = (1-alpha) * val + alpha * reg;
  if(verbose > 1){
    std::cout <<"reg: " << alpha * reg << std::endl;
    std::cout <<"freg: " << val << std::endl;
  }

  return val;
}


template <typename TPrecision, typename TImage>
void 
NormalizedCorrelationCostFunction3D<TPrecision, TImage>::gradf(vnl_vector<double> const
    &x, vnl_vector<double> &g){
  SetParametricFromVNLParameters(x);
  ComputeTransformedImage();
  ComputeDerivativeImages();

  gradf(g);
};  

template <typename TPrecision, typename TImage>
void 
NormalizedCorrelationCostFunction3D<TPrecision, TImage>::gradf(vnl_vector<double> &g){

  TParametric &parametric = transform.GetParametric();
  TControlMesh &net = parametric.GetControlMesh();
  TKnotVector &knotsU = parametric.GetUKnots();
  TKnotVector &knotsV = parametric.GetVKnots();
  TKnotVector &knotsW = parametric.GetWKnots();
  int p = knotsU.GetDegree();
  int q = knotsV.GetDegree();
  int r = knotsW.GetDegree();
  
  ImagePoint size = transform.GetSize();
  ImagePoint start = transform.GetStart();



  
  TPrecision uMin = knotsU.GetKnotA();
  TPrecision uMax = knotsU.GetKnotB();
  TPrecision uStep = ( uMax - uMin ) / size[0]; 

  TPrecision vMin = knotsV.GetKnotA();
  TPrecision vMax = knotsV.GetKnotB();
  TPrecision vStep = ( vMax - vMin ) / size[1]; 

  TPrecision wMin = knotsW.GetKnotA();
  TPrecision wMax = knotsW.GetKnotB();
  TPrecision wStep = ( wMax - wMin ) / size[2]; 

  TPrecision uS, uE, vS, vE, wS, wE;
  TPrecision bSum1;
  TPrecision bSum2;
  TPrecision bSum3;

  int vspan = 0;
  int uspan = 0;
  int wspan = 0;
  int uind = 0;
  int vind = 0;
  int wind = 0;
  
  TPrecision *bfu = new TPrecision[p+1];
  TPrecision *bfv = new TPrecision[q+1];
  TPrecision *bfw = new TPrecision[r+1];

  ImagePoint pnt;
  TControlPoint pOut;
  int index = 0;  
  int nCps = net.LengthU() * net.LengthV() * net.LengthW();
  for(int i = 0; i < net.LengthU(); i++){
    for(int j = 0; j < net.LengthV(); j++){
      for(int k = 0; k < net.LengthW(); k++){

     
        //Fixed boundary controlpoints
        if(fixedBoundary){ 
        if(i == 0  || i == net.LengthU()-1 || j == 0 || j == net.LengthV()-1 || k ==0
                   || k == net.LengthW()-1 ){ 
          g[index++] = 0; 
          g[index++] = 0; 
          g[index++] = 0;
          continue; 
        }
        }


        //compute bspline parameter range for controlpoint i,j,k
        TControlPoint &current = net.Get(i, j, k);
        knotsU.findRange(i, uS, uE);
        knotsV.findRange(j, vS, vE);
        knotsW.findRange(k, wS, wE);


        //Regularization
        double regX = 0;
        double regY = 0;
        double regZ = 0;

        if(alpha != 0){
          int nP = 0;

          for(int l=-1; l<2; l++){
            for(int m = -1; m<2; m++){
              for(int n = -1; n<2; n++){

                if(l == 0 && n == 0 && m == 0){
                  continue;
                }
                if(net.CheckIndex(i+l, j+m, k+n)){
                  TControlPoint &neighbor = net.Get(i+l, j+m, k+n);
                  regX += 2*(neighbor.x - current.x - l*xDistance);
                  regY += 2*(neighbor.y - current.y - m*yDistance);
                  regZ += 2*(neighbor.z - current.z - n*zDistance);

                  nP++;
                }
              }
            } 
          }
          regX = regX / nP; 
          regY = regY / nP;
          regZ = regZ / nP;
        }


        //Gradient

      
        ImageRegion region = transform.GetImageRegion(fixedImage, uS, uE, vS, vE, wS, wE);


        ImageIterator maskIt(maskImage, region);
        ImageIterator diffIt(differenceImage, region);
        GradientImageIterator gIt(movingGradient, region);
        
        bSum1 = 0;
        bSum2 = 0; 
        bSum3 = 0;   

  

        int nInside = 0;
        for(; !diffIt.IsAtEnd(); ++diffIt, ++gIt){
          if(!useMask || maskIt.Get() != 0 ){
            ImageIndex current = diffIt.GetIndex();
            differenceImage->TransformIndexToPhysicalPoint(current, pnt);
    
            TPrecision u = uMin + ( pnt[0] - start[0] ) * uStep;
            TPrecision v = vMin + ( pnt[1] - start[1] ) * vStep;
            TPrecision w = wMin + ( pnt[2] - start[2] ) * wStep;
   
            if( TKnotVector::isInside(uS, uE, u) &&
                TKnotVector::isInside(vS, vE, v) &&
                TKnotVector::isInside(wS, wE, w)    ){

              nInside++;

              parametric.PointAt( u, v, w, pOut, uspan, vspan, wspan, bfu, bfv, bfw);
              uind = p - uspan + i;
              vind = q - vspan + j;
              wind = r - wspan + k;

              pnt[0] = pOut.x;
              pnt[1] = pOut.y;
              pnt[2] = pOut.z;
              


              GradientVector gv = gIt.Get();

              TPrecision bf = 2 * diffIt.Get() * bfu[uind] * bfv[vind] * bfw[wind];
              bSum1 += bf * gv[0]; 
              bSum2 += bf * gv[1]; 
              bSum3 += bf * gv[2];
            } 
          }

        }

        if(nInside > 0 ){
          TPrecision beta = (1.0-alpha)/nInside;
          TPrecision gamma = alpha/nCps;
          g[index++] = beta*bSum1 + gamma*regX;
          g[index++] = beta*bSum2 + gamma*regY;
          g[index++] = beta*bSum3 + gamma*regZ;
        }
        else{  
          g[index++] = 0; 
          g[index++] = 0; 
          g[index++] = 0;
        }



      }
    }
  }
  delete[] bfu;
  delete[] bfv;
  delete[] bfw;
}


template <typename TPrecision, typename TImage>
void
NormalizedCorrelationCostFunction3D<TPrecision, TImage>::compute(vnl_vector<double> const &x, double *val, vnl_vector<double> *g){

  SetParametricFromVNLParameters(x);
  ComputeTransformedImage();
  ComputeDerivativeImages();

  if( val != NULL){
    *val = f(); 
  }

  gradf(*g);
}


template <typename TPrecision, typename TImage>
void
NormalizedCorrelationCostFunction3D<TPrecision, TImage>::Init(){
  TParametric &parametric = transform.GetParametric();
  TControlMesh &net = parametric.GetControlMesh();
  TControlPoint p1 = net.Get(0, 0, 0);
  TControlPoint p2 = net.Get(1, 1, 1);
  xDistance = p2.x - p1.x;
  yDistance = p2.y - p1.y;
  zDistance = p2.z - p1.z;

}
#endif
