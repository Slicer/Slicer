#ifndef PARAMETRICIMAGETRANSFORMATION3D_H
#define PARAMETRICIMAGETRANSFORMATION3D_H

#include "itkImageRegionIteratorWithIndex.h"
#include "itkLinearInterpolateImageFunction.h"



template <typename TParametric, typename TImage>
class ParametricImageTransformation3D{
  public:
    typedef typename TParametric::TKnotVector TKnotVector;
    typedef typename TParametric::TControlMesh TControlMesh;
    typedef typename TParametric::TControlPoint TControlPoint;
    typedef typename TControlPoint::TPrecision TPrecision;  
    
    typedef TImage Image;
    typedef typename Image::Pointer ImagePointer;
    typedef typename Image::IndexType ImageIndex;
    typedef typename Image::RegionType ImageRegion;
    typedef typename Image::PointType ImagePoint;
    typedef typename ImageRegion::SizeType ImageSize;
    typedef typename ImageSize::SizeValueType ImageSizeValue;

    /*
    typedef itk::Image<TPrecision *, 3> BFImage;
    typedef typename BFImage::Pointer BFImagePointer;
    
    typedef itk::Image<int, 3> SpanImage;
    typedef typename SpanImage::Pointer SpanImagePointer; 
    */


    ParametricImageTransformation3D(){ 
    };

    void SetImage(ImagePointer image){  this->image = image; };
    
    void SetRange(const ImageRegion &range){ 
      this->range = range;
    };
    
    ImageRegion GetRange(){ 
      return range; 
    };
    
    void SetParametric(TParametric &surface){ 
      this->surface = surface; 
    };
   
    ImagePoint GetSize(){
      return size;
    };

    ImagePoint GetStart(){
      return start;
    }; 
    

    TParametric &GetParametric(){ 
      return surface; 
    };
   
    

    void Transform(ImagePointer transformed, ImagePointer image);
    
   /*
    void TransformAndStoreBFs(ImagePointer transformed, BFImagePointer uImage,
        BFImagePointer vImage, BFImagePointer wImage, SpanImagePointer uspans,
        SpanImagePointer vspans, SpanImagePointer wspans);
   */

    ImageRegion GetImageRegion(ImagePointer image, TPrecision uStart, TPrecision uEnd, TPrecision
        vStart, TPrecision vEnd, TPrecision wStart, TPrecision wEnd);


    //Operators
    ParametricImageTransformation3D<TParametric, TImage>& operator=(const
        ParametricImageTransformation3D<TParametric, TImage>& rhs){
       surface = rhs.surface;
       range = rhs.range;
       size = rhs.szie;
       start = rhs.size;
    };


    friend std::ostream& operator << (std::ostream&
      os, ParametricImageTransformation3D<TParametric, TImage>& transform){
      ImageSize size = transform.range.GetSize();
      ImageIndex rangeIndex = transform.range.GetIndex();

       os << transform.size[0] << std::endl;
       os << transform.size[1] << std::endl;
       os << transform.size[2] << std::endl;

       os << transform.start[0] << std::endl;
       os << transform.start[1] << std::endl;
       os << transform.start[2] << std::endl;

       os << transform.surface << std::endl;
       
       return os;
    };

    friend std::istream& operator >> (std::istream& is, 
        ParametricImageTransformation3D<TParametric, TImage>& transform){
        
        ImageSize size = transform.range.GetSize();
        ImageIndex rangeIndex = transform.range.GetIndex();

        is >> transform.size[0];
        is >> transform.size[1];
        is >> transform.size[2];
        is >> transform.start[0];
        is >> transform.start[1];
        is >> transform.start[2];

        is >> transform.surface;
        return is;

    };

    void ComputePhysicalRange(ImagePointer image, ImageRegion range){
      ImageIndex index = range.GetIndex();
      image->TransformIndexToPhysicalPoint(index, start);
      for(unsigned int i=0; i< Image::GetImageDimension(); i++){
        index[i] += range.GetSize(i);
      }
      image->TransformIndexToPhysicalPoint(index, size);
      size[0] -= start[0];
      size[1] -= start[1];
      size[2] -= start[2];
    };



  private:
    TParametric surface;
    ImageRegion range;
    ImagePoint size;
    ImagePoint start;

    typedef itk::ImageRegionIteratorWithIndex<Image> ImageIterator;

    typedef itk::LinearInterpolateImageFunction<Image, double> InterpolateFunction;
    typedef typename InterpolateFunction::Pointer InterpolateFunctionPointer;
    typedef typename InterpolateFunction::ContinuousIndexType ImageContinuousIndex;

};





template <typename TParametric, typename TImage>
typename ParametricImageTransformation3D<TParametric, TImage>::ImageRegion
ParametricImageTransformation3D<TParametric, TImage>::GetImageRegion(
      ImagePointer image ,TPrecision uStart, TPrecision uEnd, TPrecision vStart, 
      TPrecision vEnd, TPrecision wStart, TPrecision wEnd) 
{
  
  TKnotVector &knotsU =  surface.GetUKnots();
  TKnotVector &knotsV =  surface.GetVKnots();
  TKnotVector &knotsW =  surface.GetWKnots();
  
  TPrecision uMin = knotsU.GetKnotA();
  TPrecision uMax = knotsU.GetKnotB();
  TPrecision uStep = ( uMax - uMin ) / size[0]; 

  TPrecision vMin = knotsV.GetKnotA();
  TPrecision vMax = knotsV.GetKnotB();
  TPrecision vStep = ( vMax - vMin ) / size[1]; 

  TPrecision wMin = knotsW.GetKnotA();
  TPrecision wMax = knotsW.GetKnotB();
  TPrecision wStep = ( wMax - wMin ) / size[2]; 

 //Compute imageregion from physical coordinates
  ImagePoint tmp;
  tmp[0] = (long) ceil( start[0] + (uStart - uMin) / uStep );
  tmp[1] = (long) ceil( start[1] + (vStart - vMin) / vStep );
  tmp[2] = (long) ceil( start[2] + (wStart - wMin) / wStep );

  ImageIndex regionIndex;
  image->TransformPhysicalPointToIndex(tmp, regionIndex);
  
  tmp[0] = (long) ceil( start[0] + (uEnd - uMin) / uStep );
  tmp[1] = (long) ceil( start[1] + (vEnd - vMin) / vStep );
  tmp[2] = (long) ceil( start[2] + (wEnd - wMin) / wStep );

  ImageIndex endIndex;
  image->TransformPhysicalPointToIndex(tmp, endIndex);


  ImageRegion region;
  region.SetIndex(regionIndex);
  region.SetSize(0, endIndex[0] - regionIndex[0] );
  region.SetSize(1, endIndex[1] - regionIndex[1] );
  region.SetSize(2, endIndex[2] - regionIndex[2] );

  return region; 

}



template <typename TParametric, typename TImage>
void
ParametricImageTransformation3D<TParametric, TImage>::Transform(ImagePointer
    transformed, ImagePointer image){
 
  InterpolateFunctionPointer imageip = InterpolateFunction::New();  

  imageip->SetInputImage( image );

  TKnotVector &knotsU =  surface.GetUKnots();
  TKnotVector &knotsV =  surface.GetVKnots();
  TKnotVector &knotsW =  surface.GetWKnots();
  
  TPrecision uMin = knotsU.GetKnotA();
  TPrecision uMax = knotsU.GetKnotB();
  TPrecision uStep = ( uMax - uMin ) / size[0]; 

  TPrecision vMin = knotsV.GetKnotA();
  TPrecision vMax = knotsV.GetKnotB();
  TPrecision vStep = ( vMax - vMin ) / size[1]; 

  TPrecision wMin = knotsW.GetKnotA();
  TPrecision wMax = knotsW.GetKnotB();
  TPrecision wStep = ( wMax - wMin ) / size[2]; 

  
  ImageRegion region = transformed->GetLargestPossibleRegion();
  ImageSize size = region.GetSize();
  ImageIndex index = region.GetIndex();

  TControlPoint out; 
  ImageIterator it( transformed, range );

  
  ImageContinuousIndex cindex;
  ImagePoint pnt;
  pnt.Fill(0);

  for(it.GoToBegin(); !it.IsAtEnd(); ++it){
    ImageIndex current = it.GetIndex();
    transformed->TransformIndexToPhysicalPoint(current, pnt);
    TPrecision u = uMin + ( pnt[0] - start[0] ) * uStep;
    TPrecision v = vMin + ( pnt[1] - start[1] ) * vStep;
    TPrecision w = wMin + ( pnt[2] - start[2] ) * wStep;
   
    if( TKnotVector::isInside(uMin, uMax, u) &&
        TKnotVector::isInside(vMin, vMax, v) &&
        TKnotVector::isInside(wMin, wMax, w)    ){
    
      surface.PointAt( u, v, w, out);
      pnt[0] = out.x;
      pnt[1] = out.y;
      pnt[2] = out.z;
    }
    bool inside = image->TransformPhysicalPointToContinuousIndex(pnt, cindex);
    if(inside){
      TPrecision pixel = (TPrecision) imageip->EvaluateAtContinuousIndex( cindex );
      it.Set( pixel );
    }
  }
}



/*
template <typename TParametric, typename TImage>
void 
ParametricImageTransformation3D<TParametric, TImage>::TransformAndStoreBFs(
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


  TKnotVector &knotsU =  surface.GetUKnots();
  TKnotVector &knotsV =  surface.GetVKnots();
  TKnotVector &knotsW =  surface.GetWKnots();
  
  TPrecision uMin = knotsU.GetKnotA();
  TPrecision uMax = knotsU.GetKnotB();
  TPrecision uStep = ( uMax - uMin ) / size[0]; 

  TPrecision vMin = knotsV.GetKnotA();
  TPrecision vMax = knotsV.GetKnotB();
  TPrecision vStep = ( vMax - vMin ) / size[1]; 

  TPrecision wMin = knotsW.GetKnotA();
  TPrecision wMax = knotsW.GetKnotB();
  TPrecision wStep = ( wMax - wMin ) / size[2]; 
  
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
  ImagePoint pnt;
  for(it.GoToBegin(); !it.IsAtEnd(); ++it, ++uit, ++vit, ++wit, ++suit, ++svit,
       ++swit ){

    ImageIndex current = it.GetIndex();
    transformed->TransformIndexToPhysicalPoint(current, pnt);
    TPrecision *bfu = new TPrecision[p+1]; 
    TPrecision *bfv = new TPrecision[q+1]; 
    TPrecision *bfw = new TPrecision[r+1]; 
    
    TPrecision u = uMin + ( pnt[0] - start[0] ) * uStep;
    TPrecision v = vMin + ( pnt[1] - start[1] ) * vStep;
    TPrecision w = wMin + ( pnt[2] - start[2] ) * wStep;
    
    surface.PointAt( u, v, w, pOut, uspan, vspan, wspan, bfu, bfv, bfw);

    pnt[0] = pOut.x;
    pnt[1] = pOut.y;
    pnt[2] = pOut.z;

    TPrecision pixel = (TPrecision) imageip->Evaluate( pnt );
    it.Set( pixel );



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
  
}
*/


#endif

