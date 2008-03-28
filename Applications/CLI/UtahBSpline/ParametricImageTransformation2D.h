#ifndef PARAMETRICIMAGETRANSFORMATION2D_H
#define PARAMETRICIMAGETRANSFORMATION2D_H

#include "itkImage.h"
#include "itkCastImageFilter.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkLinearInterpolateImageFunction.h"



template <typename TParametric>
class ParametricImageTransformation2D{
  public:
    typedef typename TParametric::TKnotVector TKnotVector;
    typedef typename TParametric::TControlMesh TControlMesh;
    typedef typename TParametric::TControlPoint TControlPoint;
    typedef typename TControlPoint::TPrecision TPrecision;  
    
    typedef itk::Image<TPrecision, 2> Image;
    typedef typename Image::Pointer ImagePointer;
    typedef typename Image::IndexType ImageIndex;
    typedef typename Image::RegionType ImageRegion;
    typedef typename ImageRegion::SizeType ImageSize;
    typedef typename ImageSize::SizeValueType ImageSizeValue;

    typedef itk::Image<TPrecision *, 2> BFImage;
    typedef typename BFImage::Pointer BFImagePointer;
    
    typedef itk::Image<int, 2> SpanImage;
    typedef typename SpanImage::Pointer SpanImagePointer; 


    ParametricImageTransformation2D(){ 
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
        BFImagePointer vImage, SpanImagePointer uspans, SpanImagePointer vspans);

    ImageRegion GetImageRegion(TPrecision uStart, TPrecision uEnd, TPrecision vStart, TPrecision vEnd);


    //Operators
    ParametricImageTransformation2D<TParametric>& operator=(const ParametricImageTransformation2D<TParametric>& rhs){
       surface = rhs.surface;
       image = rhs.image;
       maskImage = rhs.maskImage;
       transformedMaskImage = rhs.transformedMaskImage;
       range = rhs.range;
       useMask = rhs.useMask;
    };


    friend std::ostream& operator << (std::ostream&
      os, ParametricImageTransformation2D<TParametric>& transform){
      ImageSizeValue width = transform.range.GetSize( 0 );
      ImageSizeValue height = transform.range.GetSize( 1 );
      ImageIndex rangeIndex = transform.range.GetIndex();

       os << width << std::endl;
       os << height << std::endl;
       os << rangeIndex[0] << std::endl;
       os << rangeIndex[1] << std::endl;
       os << transform.surface << std::endl;
       

       std::cout << "transform written" << std::endl;

       return os;
    };

    friend std::istream& operator >> (std::istream& is, 
        ParametricImageTransformation2D<TParametric>& transform){
        
        ImageSize size = transform.range.GetSize();
        ImageIndex rangeIndex = transform.range.GetIndex();

        is >> size[0];
        is >> size[1];
        is >> rangeIndex[0];
        is >> rangeIndex[1];

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
typename ParametricImageTransformation2D<TParametric>::ImagePointer
ParametricImageTransformation2D<TParametric>::Transform(){
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
typename ParametricImageTransformation2D<TParametric>::ImageRegion
ParametricImageTransformation2D<TParametric>::GetImageRegion(TPrecision uStart, TPrecision uEnd, TPrecision vStart, TPrecision vEnd) 
{
  ImageSizeValue width = range.GetSize( 0 );
  ImageSizeValue height = range.GetSize( 1 );
  ImageIndex rangeIndex = range.GetIndex();
  
  TKnotVector &knotsU =  surface.GetUKnots();
  TKnotVector &knotsV =  surface.GetVKnots();

  TPrecision uMin = knotsU.GetKnotA();
  TPrecision uMax = knotsU.GetKnotB();
  TPrecision uStep = ( uMax - uMin ) / width; 

  TPrecision vMin = knotsV.GetKnotA();
  TPrecision vMax = knotsV.GetKnotB();
  TPrecision vStep = ( vMax - vMin ) / height; 

  ImageIndex regionIndex;
  regionIndex[0] = (long) ceil( rangeIndex[0] + (uStart - uMin) / uStep );
  regionIndex[1] = (long) ceil( rangeIndex[1] + (vStart - vMin) / vStep );

  ImageRegion region;
  region.SetIndex(regionIndex);
  region.SetSize(0, (long) floor( (uEnd - uStart)/uStep) );
  region.SetSize(1, (long) floor( (vEnd - vStart)/vStep) );

  return region; 
}



template <typename TParametric>
void
ParametricImageTransformation2D<TParametric>::Transform(ImagePointer
    transformed){
 InterpolateFunctionPointer imageip = InterpolateFunction::New();  
 InterpolateFunctionPointer maskip = InterpolateFunction::New();
  
 
  std::cout<<"image region: " << image->GetLargestPossibleRegion() << std::endl; 
  imageip->SetInputImage( image );
  if(useMask){
    maskip->SetInputImage( maskImage );
  }

  ImageSizeValue width = range.GetSize( 0 );
  ImageSizeValue height = range.GetSize( 1 );
  ImageIndex rangeIndex = range.GetIndex();
  
  TKnotVector &knotsU =  surface.GetUKnots();
  TKnotVector &knotsV =  surface.GetVKnots();

  TPrecision uMin = knotsU.GetKnotA();
  TPrecision uMax = knotsU.GetKnotB();
  TPrecision uStep = ( uMax - uMin ) / width; 

  TPrecision vMin = knotsV.GetKnotA();
  TPrecision vMax = knotsV.GetKnotB();
  TPrecision vStep = ( vMax - vMin ) / height; 
  
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

  TPrecision *bfu = new TPrecision[p+1]; 
  TPrecision *bfv = new TPrecision[q+1];
  int dummy1 = 0; 
  int dummy2 = 0;

  for(it.GoToBegin(); !it.IsAtEnd(); ++it){
    ImageIndex current = it.GetIndex();
    //std::cout << "index: " << current << std::endl;
    //std::cout << "rangeindex: " << rangeIndex << std::endl;
    TPrecision u = uMin + ( current[0] - rangeIndex[0] ) * uStep;
    TPrecision v = vMin + ( current[1] - rangeIndex[1] ) * vStep;

    surface.PointAt( u, v, out, dummy1, dummy2, bfu, bfv);
    ImageContinuousIndex cindex;

    cindex[0] = out.x;
    cindex[1] = out.y;
    if(cindex[0] < index[0]){
      cindex[0] = index[0];
    }
    else if(cindex[0] > index[0] + size[0] - 1){
      cindex[0] = index[0] + size[0] - 1;
    }
  
    if(cindex[1] < index[1]){
      cindex[1] = index[1];
    }
    else if(cindex[1] > index[1] + size[1] - 1){
      cindex[1] = index[1] + size[1] -1;
    }

    TPrecision pixel = (TPrecision) imageip->EvaluateAtContinuousIndex( cindex );
    if(pixel > 100){
      std::cout << "cindex: " << cindex << std::endl;
      std::cout << "u: " << u << std::endl;
      std::cout << "v: " << v << std::endl;
      std::cout << "index: " << current << std::endl;
    } 
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
ParametricImageTransformation2D<TParametric>::TransformAndStoreBFs(
    ImagePointer transformed, BFImagePointer uImage, BFImagePointer vImage,
    SpanImagePointer suImage, SpanImagePointer svImage )
{

  InterpolateFunctionPointer imageip = InterpolateFunction::New();  
 InterpolateFunctionPointer maskip = InterpolateFunction::New();

  imageip->SetInputImage(image );
  if(useMask){
    maskip->SetInputImage (maskImage );
  }

  ImageSizeValue width = range.GetSize( 0 );
  ImageSizeValue height = range.GetSize( 1 );
  ImageIndex rangeIndex = range.GetIndex();
  
  TKnotVector &knotsU =  surface.GetUKnots();
  TKnotVector &knotsV =  surface.GetVKnots();

  TPrecision uMin = knotsU.GetKnotA();
  TPrecision uMax = knotsU.GetKnotB();
  TPrecision uStep = ( uMax - uMin ) / width; 

  TPrecision vMin = knotsU.GetKnotA();
  TPrecision vMax = knotsU.GetKnotB();
  TPrecision vStep = ( vMax - vMin ) / height; 
  
  
  BFImageIterator uit(uImage, range);
  BFImageIterator vit(vImage, range);
  SpanImageIterator suit(suImage, range);
  SpanImageIterator svit(svImage, range);
  
  int vspan = 0;
  int uspan = 0;
  TControlPoint pOut;
  ImageIterator it( transformed, range );

  ImageIterator maskIt;
  if(useMask){
    maskIt = ImageIterator( transformedMaskImage, range );
  }
  ImageRegion region = transformed->GetLargestPossibleRegion();
  ImageSize size = region.GetSize();

  

  int p = surface.GetDegreeU();
  int q = surface.GetDegreeV();
  for(it.GoToBegin(); !it.IsAtEnd(); ++it, ++uit, ++vit, ++suit, ++svit ){

    ImageIndex current = it.GetIndex();
    TPrecision u = uMin + ( current[0] - rangeIndex[0] ) * uStep;
    TPrecision v = vMin + ( current[1] - rangeIndex[1] ) * vStep;
    TPrecision *bfu = new TPrecision[p+1]; 
    TPrecision *bfv = new TPrecision[q+1]; 
   
    surface.PointAt( u, v, pOut, uspan, vspan, bfu, bfv );


    ImageContinuousIndex cindex;
    cindex[0] = pOut.x;
    cindex[1] = pOut.y;

    if(cindex[0] < 0){
      cindex[0] = 0;
    }
    else if(cindex[0] > size[0] - 1){
      cindex[0] = size[0] -1;
    }
  
    if(cindex[1] < 0){
      cindex[1] = 0;
    }
    else if(cindex[1] > size[1] - 1){
      cindex[1] = size[1] -1;
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
    uit.Set(bfu);
    vit.Set(bfv);


    suit.Set(uspan);
    svit.Set(vspan);


  }
  
}

#endif

