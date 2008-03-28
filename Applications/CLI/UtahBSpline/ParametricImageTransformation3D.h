#ifndef PARAMETRICIMAGETRANSFORMATION3D_H
#define PARAMETRICIMAGETRANSFORMATION3D_H

#include "itkImage.h"
#include "itkCastImageFilter.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkLinearInterpolateImageFunction.h"



template <typename TParametric>
class ParametricImageTransformation3D{
  public:
    typedef typename TParametric::TKnotVector TKnotVector;
    typedef typename TParametric::TControlMesh TControlMesh;
    typedef typename TParametric::TControlPoint TControlPoint;
    typedef typename TControlPoint::TPrecision TPrecision;  
    
    typedef itk::Image<TPrecision, 3> Image;
    typedef typename Image::Pointer ImagePointer;
    typedef typename Image::IndexType ImageIndex;
    typedef typename Image::RegionType ImageRegion;
    typedef typename ImageRegion::SizeType ImageSize;
    typedef typename ImageSize::SizeValueType ImageSizeValue;

    typedef itk::Image<TPrecision *, 3> BFImage;
    typedef typename BFImage::Pointer BFImagePointer;
    
    typedef itk::Image<int, 3> SpanImage;
    typedef typename SpanImage::Pointer SpanImagePointer; 


    ParametricImageTransformation3D(){ 
      useMask = false;
    };

    void SetImage(ImagePointer image){  this->image = image; };
    
    void SetMaskImage(ImagePointer maskImage) { 
      this->maskImage = maskImage; 
      CastFilterPointer castFilter = CastFilter::New();
      castFilter->SetInput(maskImage);
      castFilter->Update();
      transformedMaskImage = castFilter->GetOutput();
    };
    
    ImagePointer GetTransformedMaskImage(){ 
      return transformedMaskImage; 
    };

    void SetUseMask(bool use){
      //useMask = use;
    };

    void SetRange(const ImageRegion &range){ this->range = range;
      std::cout << range << std::endl; };
    
    ImageRegion GetRange(){ return range; };
    
    void SetParametric(TParametric &surface){ 
      this->surface = surface; 
    };
    
    TParametric &GetParametric(){ return surface; };
   
    ImagePointer Transform();
    

    void Transform(ImagePointer transformed);
    
    void TransformAndStoreBFs(ImagePointer transformed, BFImagePointer uImage,
        BFImagePointer vImage, BFImagePointer wImage, SpanImagePointer uspans,
        SpanImagePointer vspans, SpanImagePointer wspans);

    ImageRegion GetImageRegion(TPrecision uStart, TPrecision uEnd, TPrecision
        vStart, TPrecision vEnd, TPrecision wStart, TPrecision wEnd);


    //Operators
    ParametricImageTransformation3D<TParametric>& operator=(const ParametricImageTransformation3D<TParametric>& rhs){
       surface = rhs.surface;
       image = rhs.image;
       maskImage = rhs.maskImage;
       transformedMaskImage = rhs.transformedMaskImage;
       range = rhs.range;
       useMask = rhs.useMask;
    };


    friend std::ostream& operator << (std::ostream&
      os, ParametricImageTransformation3D<TParametric>& transform){
      ImageSize size = transform.range.GetSize();
      ImageIndex rangeIndex = transform.range.GetIndex();

       os << size[0] << std::endl;
       os << size[1] << std::endl;
       os << size[2] << std::endl;

       os << rangeIndex[0] << std::endl;
       os << rangeIndex[1] << std::endl;
       os << rangeIndex[2] << std::endl;

       os << transform.surface << std::endl;
       

       std::cout << "transform written" << std::endl;

       return os;
    };

    friend std::istream& operator >> (std::istream& is, 
        ParametricImageTransformation3D<TParametric>& transform){
        
        ImageSize size = transform.range.GetSize();
        ImageIndex rangeIndex = transform.range.GetIndex();

        is >> size[0];
        is >> size[1];
        is >> size[2];
        is >> rangeIndex[0];
        is >> rangeIndex[1];
        is >> rangeIndex[2];

        transform.range.SetSize(size);
        transform.range.SetIndex(rangeIndex);

        is >> transform.surface;
        transform.useMask = false;
        return is;

    };

  private:
    TParametric surface;
    ImagePointer image;
    ImagePointer maskImage;
    ImagePointer transformedMaskImage;
    ImageRegion range;

    //CastFilter for copying Images
    typedef itk::CastImageFilter<Image, Image> CastFilter;
    typedef typename CastFilter::Pointer CastFilterPointer;

    typedef itk::ImageRegionIteratorWithIndex<Image> ImageIterator;
    typedef itk::ImageRegionIterator<BFImage> BFImageIterator;
    typedef itk::ImageRegionIterator<SpanImage> SpanImageIterator;


    typedef itk::LinearInterpolateImageFunction<Image, TPrecision> InterpolateFunction;
    typedef typename InterpolateFunction::Pointer InterpolateFunctionPointer;
    typedef typename InterpolateFunction::ContinuousIndexType ImageContinuousIndex;

    bool useMask;

};



//Non-inline implementations
template <typename TParametric>
typename ParametricImageTransformation3D<TParametric>::ImagePointer
ParametricImageTransformation3D<TParametric>::Transform(){
  if( image.IsNull() ){
    return NULL;
  }

  CastFilterPointer castFilter = CastFilter::New();
  castFilter->SetInput( image );
  castFilter->Update();
  ImagePointer transformed = castFilter->GetOutput();

  this->Transform(transformed);
  return transformed;

}


template <typename TParametric>
typename ParametricImageTransformation3D<TParametric>::ImageRegion
ParametricImageTransformation3D<TParametric>::GetImageRegion(TPrecision
    uStart, TPrecision uEnd, TPrecision vStart, TPrecision vEnd, TPrecision
    wStart, TPrecision wEnd) 
{
  ImageSize rangeSize = range.GetSize();

  ImageIndex rangeIndex = range.GetIndex();
  
  TKnotVector &knotsU =  surface.GetUKnots();
  TKnotVector &knotsV =  surface.GetVKnots();
  TKnotVector &knotsW =  surface.GetWKnots();
  
  TPrecision uMin = knotsU.GetKnotA();
  TPrecision uMax = knotsU.GetKnotB();
  TPrecision uStep = ( uMax - uMin ) / rangeSize[0]; 

  TPrecision vMin = knotsV.GetKnotA();
  TPrecision vMax = knotsV.GetKnotB();
  TPrecision vStep = ( vMax - vMin ) / rangeSize[1]; 

  TPrecision wMin = knotsW.GetKnotA();
  TPrecision wMax = knotsW.GetKnotB();
  TPrecision wStep = ( wMax - wMin ) / rangeSize[2]; 

  ImageIndex regionIndex;
  regionIndex[0] = (long) ceil( rangeIndex[0] + (uStart - uMin) / uStep );
  regionIndex[1] = (long) ceil( rangeIndex[1] + (vStart - vMin) / vStep );
  regionIndex[2] = (long) ceil( rangeIndex[2] + (wStart - wMin) / wStep );

  ImageRegion region;
  region.SetIndex(regionIndex);
  region.SetSize(0, (long) floor( (uEnd - uStart)/uStep) );
  region.SetSize(1, (long) floor( (vEnd - vStart)/vStep) );
  region.SetSize(2, (long) floor( (wEnd - wStart)/wStep) );

  return region; 
}



template <typename TParametric>
void
ParametricImageTransformation3D<TParametric>::Transform(ImagePointer
    transformed){
 InterpolateFunctionPointer imageip = InterpolateFunction::New();  
 InterpolateFunctionPointer maskip = InterpolateFunction::New();
  
 
  std::cout<<"image region: " << image->GetLargestPossibleRegion() << std::endl; 
  imageip->SetInputImage( image );
  if(useMask){
    maskip->SetInputImage( maskImage );
  }
  

  ImageSize rangeSize = range.GetSize();

  ImageIndex rangeIndex = range.GetIndex();
  
  TKnotVector &knotsU =  surface.GetUKnots();
  TKnotVector &knotsV =  surface.GetVKnots();
  TKnotVector &knotsW =  surface.GetWKnots();
  
  TPrecision uMin = knotsU.GetKnotA();
  TPrecision uMax = knotsU.GetKnotB();
  TPrecision uStep = ( uMax - uMin ) / rangeSize[0]; 

  TPrecision vMin = knotsV.GetKnotA();
  TPrecision vMax = knotsV.GetKnotB();
  TPrecision vStep = ( vMax - vMin ) / rangeSize[1]; 

  TPrecision wMin = knotsW.GetKnotA();
  TPrecision wMax = knotsW.GetKnotB();
  TPrecision wStep = ( wMax - wMin ) / rangeSize[2]; 

  
  ImageRegion region = transformed->GetLargestPossibleRegion();
  ImageSize size = region.GetSize();
  ImageIndex index = region.GetIndex();

  TControlPoint out; 
  ImageIterator it( transformed, range );

  ImageIterator maskIt;
  if(useMask){
    maskIt = ImageIterator( transformedMaskImage, range );
  }
  
  int p = surface.GetDegreeU();
  int q = surface.GetDegreeV();
  int r = surface.GetDegreeW();
  
  TPrecision *bfu = new TPrecision[p+1]; 
  TPrecision *bfv = new TPrecision[q+1];
  TPrecision *bfw = new TPrecision[r+1];

  int dummy1 = 0; 
  int dummy2 = 0;
  int dummy3 = 0;

  for(it.GoToBegin(); !it.IsAtEnd(); ++it){
    ImageIndex current = it.GetIndex();
    //std::cout << "index: " << current << std::endl;
    //std::cout << "rangeindex: " << rangeIndex << std::endl;
    TPrecision u = uMin + ( current[0] - rangeIndex[0] ) * uStep;
    TPrecision v = vMin + ( current[1] - rangeIndex[1] ) * vStep;
    TPrecision w = wMin + ( current[2] - rangeIndex[2] ) * wStep;
    
    surface.PointAt( u, v, w, out, dummy1, dummy2, dummy3, bfu, bfv, bfw);
    ImageContinuousIndex cindex;

    cindex[0] = out.x;
    cindex[1] = out.y;
    cindex[2] = out.z;
    for(int i=0; i < 3; i++){
      if(cindex[i] < index[i]){
        cindex[i] = index[i];
      }
      else if(cindex[i] > index[i] + size[i] - 1){
        cindex[i] = index[i] + size[i] - 1;
      }
    }

    TPrecision pixel = (TPrecision) imageip->EvaluateAtContinuousIndex( cindex );
    it.Set( pixel );
   
    if(useMask){ 
      pixel = (TPrecision) maskip->EvaluateAtContinuousIndex( cindex ); 
      maskIt.Set( pixel );
      ++maskIt;
    }

  }

}


template <typename TParametric>
void 
ParametricImageTransformation3D<TParametric>::TransformAndStoreBFs(
    ImagePointer transformed, BFImagePointer uImage, BFImagePointer vImage,
    BFImagePointer wImage, SpanImagePointer suImage, SpanImagePointer svImage,
    SpanImagePointer swImage )
{

  InterpolateFunctionPointer imageip = InterpolateFunction::New();  
  InterpolateFunctionPointer maskip = InterpolateFunction::New();

  imageip->SetInputImage(image );
  if(useMask){
    maskip->SetInputImage (maskImage );
  }

  ImageSize rangeSize = range.GetSize();

  ImageIndex rangeIndex = range.GetIndex();
  
  TKnotVector &knotsU =  surface.GetUKnots();
  TKnotVector &knotsV =  surface.GetVKnots();
  TKnotVector &knotsW =  surface.GetWKnots();
  
  TPrecision uMin = knotsU.GetKnotA();
  TPrecision uMax = knotsU.GetKnotB();
  TPrecision uStep = ( uMax - uMin ) / rangeSize[0]; 

  TPrecision vMin = knotsV.GetKnotA();
  TPrecision vMax = knotsV.GetKnotB();
  TPrecision vStep = ( vMax - vMin ) / rangeSize[1]; 

  TPrecision wMin = knotsW.GetKnotA();
  TPrecision wMax = knotsW.GetKnotB();
  TPrecision wStep = ( wMax - wMin ) / rangeSize[2]; 
  
  BFImageIterator uit(uImage, range);
  BFImageIterator vit(vImage, range);
  BFImageIterator wit(wImage, range);

  SpanImageIterator suit(suImage, range);
  SpanImageIterator svit(svImage, range);
  SpanImageIterator swit(swImage, range);

  int vspan = 0;
  int uspan = 0;
  int wspan = 0;

  TControlPoint pOut;
  ImageIterator it( transformed, range );

  ImageIterator maskIt;
  if(useMask){
    maskIt = ImageIterator( transformedMaskImage, range );
  }
  ImageRegion region = transformed->GetLargestPossibleRegion();
  ImageSize size = region.GetSize();
  ImageIndex index = region.GetIndex();
  

  int p = surface.GetDegreeU();
  int q = surface.GetDegreeV();
  int r = surface.GetDegreeW();
  for(it.GoToBegin(); !it.IsAtEnd(); ++it, ++uit, ++vit, ++wit, ++suit, ++svit,
       ++swit ){

    ImageIndex current = it.GetIndex();
    TPrecision *bfu = new TPrecision[p+1]; 
    TPrecision *bfv = new TPrecision[q+1]; 
    TPrecision *bfw = new TPrecision[r+1]; 
    
    TPrecision u = uMin + ( current[0] - rangeIndex[0] ) * uStep;
    TPrecision v = vMin + ( current[1] - rangeIndex[1] ) * vStep;
    TPrecision w = wMin + ( current[2] - rangeIndex[2] ) * wStep;
    
    surface.PointAt( u, v, w, pOut, uspan, vspan, wspan, bfu, bfv, bfw);
    ImageContinuousIndex cindex;

    cindex[0] = pOut.x;
    cindex[1] = pOut.y;
    cindex[2] = pOut.z;
    for(int i=0; i < 3; i++){
      if(cindex[i] < index[i]){
        cindex[i] = index[i];
      }
      else if(cindex[i] > index[i] + size[i] - 1){
        cindex[i] = index[i] + size[i] - 1;
      }
    }



    TPrecision pixel = (TPrecision) imageip->EvaluateAtContinuousIndex( cindex );
    it.Set( pixel );


   if(useMask){ 
      pixel = (TPrecision) maskip->EvaluateAtContinuousIndex( cindex );
      maskIt.Set( pixel );
      ++maskIt;
   }


    delete[] uit.Get();
    delete[] vit.Get();
    delete[] wit.Get();
    uit.Set(bfu);
    vit.Set(bfv);
    wit.Set(bfw);


    suit.Set(uspan);
    svit.Set(vspan);
    swit.Set(wspan);


  }
  
};

#endif

