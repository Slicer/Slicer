#ifndef NOMRALIZEDCORRELATIONCOSTFUNCTION2D_H
#define NOMRALIZEDCORRELATIONCOSTFUNCTION2D_H

#include "vnl/vnl_cost_function.h"
#include "vnl/vnl_least_squares_function.h"

#include "itkImage.h"
#include "itkDerivativeImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkSubtractImageFilter.h"

#include "Point2D.h"
#include "BSpline2D.h"
#include "ParametricImageTransformation2D.h"
#include "ControlMesh2D.h"
#include "KnotVector.h"


template <typename TPrecision>
class NormalizedCorrelationCostFunction2D : public vnl_cost_function {

    
  public:
    typedef Point2D< TPrecision> TControlPoint;
    typedef BSpline2D< TControlPoint > TParametric;
    typedef ParametricImageTransformation2D<TParametric> TImageTransformation;
    typedef typename TParametric::TControlMesh TControlMesh;
    typedef typename TParametric::TKnotVector TKnotVector;

    typedef typename TImageTransformation::Image Image;
    typedef typename Image::Pointer ImagePointer;
    typedef typename Image::IndexType ImageIndex;
    typedef typename Image::RegionType ImageRegion;
    typedef typename ImageRegion::SizeType ImageSize;
    typedef typename ImageSize::SizeValueType ImageSizeValue;

    typedef typename TImageTransformation::BFImage BFImage;
    typedef typename BFImage::Pointer BFImagePointer;
    
    typedef typename TImageTransformation::SpanImage SpanImage;
    typedef typename SpanImage::Pointer SpanImagePointer;
   
    NormalizedCorrelationCostFunction2D(int
        nUnknowns):vnl_cost_function(nUnknowns){
     
      castFilter = CastFilter::New();
      dxFilter =  DerivativeFilter::New();
      dxFilter->SetDirection(0);
      dxFilter->SetOrder(1);
      dyFilter = DerivativeFilter::New();
      dyFilter->SetDirection(1);
      dyFilter->SetOrder(1);
      subtractFilter = SubtractFilter::New();
      useMask =  false;
      
      alpha = 0;

      verbose = 0;

      bfuImage = BFImage::New();
      bfvImage = BFImage::New();
      uspanImage = SpanImage::New();
      vspanImage = SpanImage::New();

    };
    
    double f(vnl_vector<double> const &x);
    void gradf(vnl_vector<double> const &x, vnl_vector<double> &g);
    void compute(vnl_vector<double> const &x, double *f, vnl_vector<double> *g);

    void GetVNLParametersFromParametric(vnl_vector<double> &params); 
    void SetParametricFromVNLParameters(vnl_vector<double> const &params);



    
    //Set / Get Methods
    void SetFixedImage(ImagePointer fixed){ 
      this->fixedImage = fixed; 
    };
   
    void SetMaskImage(ImagePointer maskImage){
      transform.SetMaskImage(maskImage);
    };

    void SetMovingImage(ImagePointer moving){
      this->movingImage = moving;
      this->transform.SetImage(movingImage);
      castFilter->SetInput( movingImage );
      castFilter->Update();
      movingTransformedImage = castFilter->GetOutput();
      
      bfuImage->SetRegions( movingImage->GetLargestPossibleRegion() );
      bfuImage->Allocate();
      bfvImage->SetRegions( movingImage->GetLargestPossibleRegion() );
      bfvImage->Allocate();
      uspanImage->SetRegions( movingImage->GetLargestPossibleRegion() );
      uspanImage->Allocate();
      vspanImage->SetRegions( movingImage->GetLargestPossibleRegion() );
      vspanImage->Allocate();
    };
    
    void SetRange(const ImageRegion &range){ 
      this->transform.SetRange(range); 
    };

    void SetParametric(TParametric &surface){
      this->transform.SetParametric( surface );
    };

    ImagePointer GetTransformedImage(){
      return movingTransformedImage;
    };

    ImagePointer GetTransformedMaskImage(){
      return transform.GetTransformedMaskImage();
    };
    
    void SetUseMask(bool use){
      useMask = use;
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
    NormalizedCorrelationCostFunction2D<TPrecision>& operator=(const
        NormalizedCorrelationCostFunction2D<TPrecision>& rhs){
    
       transform  = rhs.transfrom;

       fixedImage = rhs.fixedImage;
       movingImage = rhs.movingImage;
       movingTransformedImage =rhs.movingTransformedImage;
       differenceImage = rhs.differenceImage;
       movingDxImage = rhs.movingDxImage;
       movingDyImage = rhs.movingDyImage;
       bfuImage = rhs.bfuImage;
       bfvImage = rhs.bfvImage;
       uspanImage = rhs.uspanImage;
       vspanImage = rhs.vspanImage;

       useMask = rhs.useMask;
       vDistance = rhs.vDistance;
       hDistance = rhs.hDistance;
       alpha = rhs.alpha;

    };


  private:

    typedef typename itk::ImageRegionIterator<Image> ImageIterator;
    typedef typename itk::ImageRegionIterator<BFImage> BFImageIterator;
    typedef typename itk::ImageRegionIterator<SpanImage> SpanImageIterator;

    typedef itk::CastImageFilter<Image, Image> CastFilter;
    typedef typename CastFilter::Pointer CastFilterPointer;
    CastFilterPointer castFilter;
    
    typedef itk::DerivativeImageFilter<Image, Image> DerivativeFilter;
    typedef typename DerivativeFilter::Pointer DerivativeFilterPointer;
    DerivativeFilterPointer dxFilter;
    DerivativeFilterPointer dyFilter;

    typedef itk::SubtractImageFilter<Image, Image, Image> SubtractFilter;
    typedef typename SubtractFilter::Pointer SubtractFilterPointer;
    SubtractFilterPointer subtractFilter;

    ImagePointer fixedImage;
    ImagePointer movingImage;
    ImagePointer movingTransformedImage;
    ImagePointer differenceImage;
    ImagePointer movingDxImage;
    ImagePointer movingDyImage;
    

    BFImagePointer bfuImage;
    BFImagePointer bfvImage;

    SpanImagePointer uspanImage;
    SpanImagePointer vspanImage;

    
    TImageTransformation transform;

    bool useMask;
    TPrecision vDistance;
    TPrecision hDistance;
    TPrecision alpha;

    void ComputeTransformedImage(vnl_vector<double> const &x);
    void ComputeDerivativeImages();
    double f();
    void gradf(vnl_vector<double> &g);
   

    int verbose;


};


//non-inline implementations
template <typename TPrecision>
void 
NormalizedCorrelationCostFunction2D<TPrecision>::GetVNLParametersFromParametric(vnl_vector<double> &params)
{
  TParametric &surface = transform.GetParametric();
  TControlMesh &net = surface.GetControlMesh();
  int index = 0;
  for(int i = 0; i < net.LengthU(); i++){
    for(int j = 0; j < net.LengthV(); j++){
      TControlPoint tmp = net.Get(i, j);
      params[index++] = tmp.x;
      params[index++] = tmp.y;
    }
  }
}


template <typename TPrecision>
void 
NormalizedCorrelationCostFunction2D<TPrecision>::SetParametricFromVNLParameters(vnl_vector<double> const &params)
{

  TParametric &surface = transform.GetParametric();
  TControlMesh &net = surface.GetControlMesh();

  int index = 0;
  for(int i = 0; i < net.LengthU(); i++){
    for(int j = 0; j < net.LengthV(); j++){
      TControlPoint &tmp = net.Get(i, j);
      tmp.x = params[index++];
      tmp.y = params[index++];
      net.Set(i, j, tmp);
    }
  }
  //TODO necessary?
  //surface.SetControlMesh2D(net);
  //transform.SetParametric(surface);
}


template <typename TPrecision>
void 
NormalizedCorrelationCostFunction2D<TPrecision>::ComputeTransformedImage(vnl_vector<double> const &x)
{
  SetParametricFromVNLParameters(x);  
  transform.TransformAndStoreBFs( movingTransformedImage, bfuImage, bfvImage, uspanImage, vspanImage );

  subtractFilter->SetInput1( fixedImage );
  subtractFilter->SetInput2( movingTransformedImage );
  subtractFilter->Modified();
  subtractFilter->Update();
  differenceImage = subtractFilter->GetOutput();
  
}


template <typename TPrecision>
void 
NormalizedCorrelationCostFunction2D<TPrecision>::ComputeDerivativeImages(){
  
  dxFilter->SetInput(movingTransformedImage);
  dxFilter->Modified();
  dxFilter->Update();
  movingDxImage = dxFilter->GetOutput();
  
  dyFilter->SetInput(movingTransformedImage);
  dyFilter->Modified();
  dyFilter->Update();
  movingDyImage = dyFilter->GetOutput();

}


template <typename TPrecision>
double 
NormalizedCorrelationCostFunction2D<TPrecision>::f(vnl_vector<double> const &x){
  ComputeTransformedImage(x);  
  
  ImageRegion region = transform.GetRange();
  ImageIterator it( differenceImage, region);

  return f();
}


template <typename TPrecision>
double 
NormalizedCorrelationCostFunction2D<TPrecision>::f(){

 /* typedef typename itk::ImageFileWriter<Image> ImageWriter;
  typedef typename ImageWriter::Pointer ImageWriterPointer;
  ImageWriterPointer writer = ImageWriter::New();
  char name[100];
  sprintf(name, "./transformed_%d.mhd", iter++);
  writer->SetFileName(name);
  writer->SetInput(movingTransformedImage);
  writer->Update();*/

    
  double val = 0;
  double tmp = 0;

  ImageRegion region = transform.GetRange();
  ImagePointer transformedMaskImage = transform.GetTransformedMaskImage();
  ImageIterator it( differenceImage, region);
  ImageIterator maskIt(transformedMaskImage, region);

  int n = 0;
  for(it.GoToBegin(); !it.IsAtEnd(); ++it, ++maskIt ){
      if(!useMask || maskIt.Get() !=0){
        n++;
        tmp =  it.Get();
        val += tmp * tmp;
      }
  }
  ImageSize regionSize = region.GetSize();
  val = val / n;

  if(verbose > 0){
    std::cout <<"f: " << val << std::endl;
  }

  //Regularization term
  TParametric &surface = transform.GetParametric();
  TControlMesh &net = surface.GetControlMesh();
  double regTmp = 0;
  double reg = 0;
  for(int i = 0; i < net.LengthU(); i++){
    for(int j = 0; j < net.LengthV(); j++){
      TControlPoint &current = net.Get(i, j);
      regTmp = 0;
      int nP = 0;
      for(int k=-1; k<2; k++){
        for(int l = -1; l<2; l++){
          if(k == 0 && l == 0){
            continue;
          }
          if(net.CheckIndex(i+k, j+l)){
            TControlPoint &neighbor = net.Get(i+k, j+l);
            tmp = neighbor.x - current.x - k*hDistance;
            regTmp+= tmp*tmp;
            tmp = neighbor.y - current.y - l*vDistance;
            regTmp+=tmp*tmp;
            nP++;
          } 
        }
      }
      reg += regTmp / nP; 
    }
  }

  if(verbose > 1){
    reg = reg / n;
    std::cout <<"reg: " << reg << std::endl;
  
    val = (1-alpha)*val + alpha*reg;
    std::cout <<"freg: " << val << std::endl;
  }

  return val;
}


template <typename TPrecision>
void 
NormalizedCorrelationCostFunction2D<TPrecision>::gradf(vnl_vector<double> const
    &x, vnl_vector<double> &g){
  ComputeTransformedImage(x);
  ComputeDerivativeImages();
  gradf(g);
}


template <typename TPrecision>
void 
NormalizedCorrelationCostFunction2D<TPrecision>::gradf(vnl_vector<double> &g){


  TPrecision *bfu = NULL;
  TPrecision *bfv = NULL;

  TParametric &surface = transform.GetParametric();
  TControlMesh &net = surface.GetControlMesh();
  TKnotVector &uknots = surface.GetUKnots();
  TKnotVector &vknots = surface.GetVKnots();
  int p = uknots.GetDegree();
  int q = vknots.GetDegree();

  TPrecision uS, uE, vS, vE;
  TPrecision bSum1;
  TPrecision bSum2;
  int index = 0;
  int uind = 0;
  int vind = 0;

  ImagePointer transformedMaskImage = transform.GetTransformedMaskImage();
  for(int i = 0; i < net.LengthU(); i++){
    for(int j = 0; j < net.LengthV(); j++){
      TControlPoint &current = net.Get(i, j);
      uknots.findRange(i, uS, uE);
      vknots.findRange(j, vS, vE);


      //regularization
      double regX = 0;
      double regY = 0;
      int nP = 0;
      for(int k=-1; k<2; k++){
        for(int l = -1; l<2; l++){
          if(k == 0 && l == 0){
            continue;
          }

          if(net.CheckIndex(i+k, j+l)){
            TControlPoint &neighbor = net.Get(i+k, j+l);
            regX += 2*(neighbor.x - current.x - k*hDistance);
            regY += 2*(neighbor.y - current.y - l*vDistance);

            nP++;
          } 
        }
      }
      regX = regX / nP; 
      regY = regY / nP;

      ImageRegion region = transform.GetImageRegion(uS, uE, vS, vE);

      ImageIterator diffIt(differenceImage, region);
      ImageIterator dxIt(movingDxImage, region);
      ImageIterator dyIt(movingDyImage, region);

      ImageIterator maskIt(transformedMaskImage, region);
      
      BFImageIterator bfuIt(bfuImage, region);
      BFImageIterator bfvIt(bfvImage, region);

      SpanImageIterator suIt(uspanImage, region);
      SpanImageIterator svIt(vspanImage, region);

      bSum1 = 0;
      bSum2 = 0;    

      int n = 1;
      int nInMask = 0;

      for(; !diffIt.IsAtEnd(); ++diffIt, ++dxIt, ++dyIt, ++bfuIt, ++bfvIt,
          ++suIt, ++svIt, ++maskIt)
      {
        
        n++;
        if(!useMask || maskIt.Get() != 0 ){
          nInMask++;
          uind = p - suIt.Get() + i;
          vind = q - svIt.Get() + j;

          TPrecision *bfu = bfuIt.Get();
          TPrecision *bfv = bfvIt.Get();
      
          TPrecision bf = 2 * diffIt.Get() * bfu[uind] * bfv[vind];
          bSum1 += bf * dxIt.Get(); 
          bSum2 += bf * dyIt.Get(); 

         // bSum1 += 2 * diffIt.Get() *( bfu[uind] * bfv[vind] * dxIt.Get()); 
         // bSum2 += 2 * diffIt.Get() *( bfu[uind] * bfv[vind] * dyIt.Get()); 
        }

      }
      
      TPrecision pInMask = (TPrecision)nInMask/n;
      g[index++] = bSum1 + alpha * regX * pInMask;
      g[index++] = bSum2 + alpha * regY * pInMask;
    }
  }
}


template <typename TPrecision>
void
NormalizedCorrelationCostFunction2D<TPrecision>::compute(vnl_vector<double> const &x, double *val, vnl_vector<double> *g){
  ComputeTransformedImage(x); 
  ComputeDerivativeImages();

  if( val != NULL){
    *val = f(); 
  } 
  gradf(*g);
}


template <typename TPrecision>
void
NormalizedCorrelationCostFunction2D<TPrecision>::Init(){
  TParametric &surface = transform.GetParametric();
  TControlMesh &net = surface.GetControlMesh();
  TControlPoint p1 = net.Get(0, 0);
  TControlPoint p2 = net.Get(1, 1);
  hDistance = p2.x - p1.x;
  vDistance = p2.y - p1.y;


}
#endif
