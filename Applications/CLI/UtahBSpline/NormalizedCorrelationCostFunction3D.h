#ifndef NOMRALIZEDCORRELATIONCOSTFUNCTION3D_H
#define NOMRALIZEDCORRELATIONCOSTFUNCTION3D_H

#include "vnl/vnl_cost_function.h"
#include "vnl/vnl_least_squares_function.h"

#include "itkImage.h"
#include "itkDerivativeImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkSubtractImageFilter.h"

#include "Point3D.h"
#include "BSpline3D.h"
#include "ParametricImageTransformation3D.h"
#include "ControlMesh3D.h"
#include "KnotVector.h"


template <typename TPrecision>
class NormalizedCorrelationCostFunction3D : public vnl_cost_function {

    
  public:
    typedef Point3D< TPrecision> TControlPoint;
    typedef BSpline3D< TControlPoint > TParametric;
    typedef ParametricImageTransformation3D<TParametric> TImageTransformation;
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
   
    NormalizedCorrelationCostFunction3D(int
        nUnknowns):vnl_cost_function(nUnknowns){
     
      castFilter = CastFilter::New();
      dxFilter =  DerivativeFilter::New();
      dxFilter->SetDirection(0);
      dxFilter->SetOrder(1);
      dyFilter = DerivativeFilter::New();
      dyFilter->SetDirection(1);
      dyFilter->SetOrder(1);
      dzFilter = DerivativeFilter::New();
      dzFilter->SetDirection(2);
      dzFilter->SetOrder(1);

      subtractFilter = SubtractFilter::New();
      useMask =  false;
      
      alpha = 0;

      verbose = 0;

      bfuImage = BFImage::New();
      bfvImage = BFImage::New();
      bfwImage = BFImage::New();

      uspanImage = SpanImage::New();
      vspanImage = SpanImage::New();
      wspanImage = SpanImage::New();

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
      bfwImage->SetRegions( movingImage->GetLargestPossibleRegion() );
      bfwImage->Allocate();

      uspanImage->SetRegions( movingImage->GetLargestPossibleRegion() );
      uspanImage->Allocate();
      vspanImage->SetRegions( movingImage->GetLargestPossibleRegion() );
      vspanImage->Allocate();
      wspanImage->SetRegions( movingImage->GetLargestPossibleRegion() );
      wspanImage->Allocate();

    };
    
    void SetRange(const ImageRegion &range){ 
      this->transform.SetRange(range); 
    };

    void SetParametric(TParametric &parametric){
      this->transform.SetParametric( parametric );
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
    NormalizedCorrelationCostFunction3D<TPrecision>& operator=(const
        NormalizedCorrelationCostFunction3D<TPrecision>& rhs){
    
       transform  = rhs.transfrom;

       fixedImage = rhs.fixedImage;
       movingImage = rhs.movingImage;
       movingTransformedImage =rhs.movingTransformedImage;
       differenceImage = rhs.differenceImage;
       movingDxImage = rhs.movingDxImage;
       movingDyImage = rhs.movingDyImage;
       movingDzImage = rhs.movingDzImage;

       bfuImage = rhs.bfuImage;
       bfvImage = rhs.bfvImage;
       bfwImage = rhs.bfwImage;

       uspanImage = rhs.uspanImage;
       vspanImage = rhs.vspanImage;
       wspanImage = rhs.wspanImage;

       useMask = rhs.useMask;
       xDistance = rhs.xDistance;
       yDistance = rhs.yDistance;
       zDistance = rhs.zDistance;

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
    DerivativeFilterPointer dzFilter;

    typedef itk::SubtractImageFilter<Image, Image, Image> SubtractFilter;
    typedef typename SubtractFilter::Pointer SubtractFilterPointer;
    SubtractFilterPointer subtractFilter;

    ImagePointer fixedImage;
    ImagePointer movingImage;
    ImagePointer movingTransformedImage;
    ImagePointer differenceImage;
    ImagePointer movingDxImage;
    ImagePointer movingDyImage;
    ImagePointer movingDzImage;

    BFImagePointer bfuImage;
    BFImagePointer bfvImage;
    BFImagePointer bfwImage;


    SpanImagePointer uspanImage;
    SpanImagePointer vspanImage;
    SpanImagePointer wspanImage;

    
    TImageTransformation transform;

    bool useMask;
    TPrecision xDistance;
    TPrecision yDistance;
    TPrecision zDistance;

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
NormalizedCorrelationCostFunction3D<TPrecision>::GetVNLParametersFromParametric(vnl_vector<double> &params)
{
  TParametric &parametric = transform.GetParametric();
  TControlMesh &net = parametric.GetControlMesh();
  int index = 0;
  for(int i = 0; i < net.LengthU(); i++){
    for(int j = 0; j < net.LengthV(); j++){
      for(int k = 0; k < net.LengthW(); k++){
        TControlPoint tmp = net.Get(i, j, k);
        params[index++] = tmp.x;
        params[index++] = tmp.y;
        params[index++] = tmp.z;
      }
    }
  }
}


template <typename TPrecision>
void 
NormalizedCorrelationCostFunction3D<TPrecision>::SetParametricFromVNLParameters(vnl_vector<double> const &params)
{

  TParametric &parametric = transform.GetParametric();
  TControlMesh &net = parametric.GetControlMesh();

  int index = 0;
  for(int i = 0; i < net.LengthU(); i++){
    for(int j = 0; j < net.LengthV(); j++){
      for(int k = 0; k < net.LengthW(); k++){
        TControlPoint tmp = net.Get(i, j, k);
        tmp.x = params[index++];
        tmp.y = params[index++];
        tmp.z = params[index++];
        net.Set(i, j, k, tmp);
      }
    }
  }
  //TODO necessary?
  //parametric.SetControlMesh2D(net);
  //transform.SetParametric(parametric);
}


template <typename TPrecision>
void 
NormalizedCorrelationCostFunction3D<TPrecision>::ComputeTransformedImage(vnl_vector<double> const &x)
{
  SetParametricFromVNLParameters(x);  
  transform.TransformAndStoreBFs( movingTransformedImage, bfuImage, bfvImage,
      bfwImage, uspanImage, vspanImage, wspanImage );

  subtractFilter->SetInput1( fixedImage );
  subtractFilter->SetInput2( movingTransformedImage );
  subtractFilter->Modified();
  subtractFilter->Update();
  differenceImage = subtractFilter->GetOutput();
  
}


template <typename TPrecision>
void 
NormalizedCorrelationCostFunction3D<TPrecision>::ComputeDerivativeImages(){
  
  dxFilter->SetInput(movingTransformedImage);
  dxFilter->Modified();
  dxFilter->Update();
  movingDxImage = dxFilter->GetOutput();
  
  dyFilter->SetInput(movingTransformedImage);
  dyFilter->Modified();
  dyFilter->Update();
  movingDyImage = dyFilter->GetOutput();

  dzFilter->SetInput(movingTransformedImage);
  dzFilter->Modified();
  dzFilter->Update();
  movingDzImage = dzFilter->GetOutput();

}


template <typename TPrecision>
double 
NormalizedCorrelationCostFunction3D<TPrecision>::f(vnl_vector<double> const &x){
  ComputeTransformedImage(x);  
  
  ImageRegion region = transform.GetRange();
  ImageIterator it( differenceImage, region);

  return f();
}


template <typename TPrecision>
double 
NormalizedCorrelationCostFunction3D<TPrecision>::f(){

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

  if(verbose > 1){
    reg = reg / n;
    std::cout <<"reg: " << reg << std::endl;
  
    val = (1-alpha) * val + alpha * reg;
    std::cout <<"freg: " << val << std::endl;
  }

  return val;
}


template <typename TPrecision>
void 
NormalizedCorrelationCostFunction3D<TPrecision>::gradf(vnl_vector<double> const
    &x, vnl_vector<double> &g){
  ComputeTransformedImage(x);
  ComputeDerivativeImages();
  gradf(g);
}


template <typename TPrecision>
void 
NormalizedCorrelationCostFunction3D<TPrecision>::gradf(vnl_vector<double> &g){


  TPrecision *bfu = NULL;
  TPrecision *bfv = NULL;
  TPrecision *bfw = NULL;

  TParametric &parametric = transform.GetParametric();
  TControlMesh &net = parametric.GetControlMesh();
  TKnotVector &uknots = parametric.GetUKnots();
  TKnotVector &vknots = parametric.GetVKnots();
  TKnotVector &wknots = parametric.GetWKnots();
  int p = uknots.GetDegree();
  int q = vknots.GetDegree();
  int r = wknots.GetDegree();

  TPrecision uS, uE, vS, vE, wS, wE;
  TPrecision bSum1;
  TPrecision bSum2;
  TPrecision bSum3;

  int index = 0;
  int uind = 0;
  int vind = 0;
  int wind = 0;

  ImagePointer transformedMaskImage = transform.GetTransformedMaskImage();
  for(int i = 0; i < net.LengthU(); i++){
    for(int j = 0; j < net.LengthV(); j++){
      for(int k = 0; k < net.LengthW(); k++){

        TControlPoint &current = net.Get(i, j, k);
        uknots.findRange(i, uS, uE);
        vknots.findRange(j, vS, vE);
        wknots.findRange(k, wS, wE);


        //regularization
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
      
      ImageRegion region = transform.GetImageRegion(uS, uE, vS, vE, wS, wE);

      ImageIterator diffIt(differenceImage, region);
      ImageIterator dxIt(movingDxImage, region);
      ImageIterator dyIt(movingDyImage, region);
      ImageIterator dzIt(movingDzImage, region);

      ImageIterator maskIt(transformedMaskImage, region);
      
      BFImageIterator bfuIt(bfuImage, region);
      BFImageIterator bfvIt(bfvImage, region);
      BFImageIterator bfwIt(bfwImage, region);

      SpanImageIterator suIt(uspanImage, region);
      SpanImageIterator svIt(vspanImage, region);
      SpanImageIterator swIt(wspanImage, region);

      bSum1 = 0;
      bSum2 = 0; 
      bSum3 = 0;   

      int n = 1;
      int nInMask = 0;

      for(; !diffIt.IsAtEnd(); ++diffIt, ++maskIt,
                               ++dxIt, ++dyIt, ++dzIt, 
                               ++bfuIt, ++bfvIt, ++bfwIt,
                               ++suIt, ++svIt, ++swIt)
      {
        
        n++;
        if(!useMask || maskIt.Get() != 0 ){
          nInMask++;
          uind = p - suIt.Get() + i;
          vind = q - svIt.Get() + j;
          wind = r - swIt.Get() + k;

          TPrecision *bfu = bfuIt.Get();
          TPrecision *bfv = bfvIt.Get();
          TPrecision *bfw = bfwIt.Get();


          TPrecision bf = 2 * diffIt.Get() * bfu[uind] * bfv[vind] * bfw[wind];
          bSum1 += bf * dxIt.Get(); 
          bSum2 += bf * dyIt.Get(); 
          bSum3 += bf * dzIt.Get(); 
        }

      }

      TPrecision pInMask = (TPrecision)nInMask/n;
      g[index++] = bSum1 + alpha * regX * pInMask;
      g[index++] = bSum2 + alpha * regY * pInMask;
      g[index++] = bSum3 + alpha * regZ * pInMask;
      }
    }
  }
}


template <typename TPrecision>
void
NormalizedCorrelationCostFunction3D<TPrecision>::compute(vnl_vector<double> const &x, double *val, vnl_vector<double> *g){
  ComputeTransformedImage(x); 
  ComputeDerivativeImages();

  if( val != NULL){
    *val = f(); 
  } 
  gradf(*g);
}


template <typename TPrecision>
void
NormalizedCorrelationCostFunction3D<TPrecision>::Init(){
  TParametric &parametric = transform.GetParametric();
  TControlMesh &net = parametric.GetControlMesh();
  TControlPoint p1 = net.Get(0, 0, 0);
  TControlPoint p2 = net.Get(1, 1, 1);
  xDistance = p2.x - p1.x;
  yDistance = p2.y - p1.y;
  zDistance = p2.z - p1.z;

}
#endif
