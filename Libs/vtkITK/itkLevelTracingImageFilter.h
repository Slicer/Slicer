#ifndef itkLevelTracingImageFilter_h
#define itkLevelTracingImageFilter_h

#include "itkImage.h"
#include "itkImageToImageFilter.h"
#include "itkSimpleDataObjectDecorator.h"
#include "itkChainCodePath.h"

namespace itk
{

/** \class LevelTracingImageFilter
 * \brief Trace a level curve/surface given a seed point on the level curve/surface.
 *
 * LevelTracingImageFilter traces a level curve (or surface) from a
 * seed point.  The pixels on this level curve "boundary" are labeled
 * as 1. Does nothing if seed is in uniform area.
 *
 * LevelTracingImageFilter provides a quick method to select a point
 * on a boundary of an object in a grayscale image and retrieve the
 * entire boundary of the object.
 *
 * For 2D images, the algorithm follows the boundary using 8-connected
 * neighbors.  For ND images, the algorithm traces the boundary using
 * face connected neighbors.
 *
 */

template <class TInputImage, class TOutputImage>
class LevelTracingImageFilter:public ImageToImageFilter<TInputImage,TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef LevelTracingImageFilter                      Self;
  typedef ImageToImageFilter<TInputImage,TOutputImage> Superclass;
  typedef SmartPointer<Self>                           Pointer;
  typedef SmartPointer<const Self>                     ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods).  */
  itkTypeMacro(LevelTracingImageFilter,
               ImageToImageFilter);

  /** Image related typedefs. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      TInputImage::ImageDimension);

  typedef TInputImage                           InputImageType;
  typedef typename InputImageType::Pointer      InputImagePointer;
  typedef typename InputImageType::ConstPointer InputImageConstPointer;
  typedef typename InputImageType::RegionType   InputImageRegionType;
  typedef typename InputImageType::PixelType    InputImagePixelType;
  typedef typename InputImageType::IndexType    IndexType;
  typedef typename InputImageType::SizeType     SizeType;

  typedef TOutputImage                         OutputImageType;
  typedef typename OutputImageType::Pointer    OutputImagePointer;
  typedef typename OutputImageType::RegionType OutputImageRegionType;
  typedef typename OutputImageType::PixelType  OutputImagePixelType;

  typedef ChainCodePath<ImageDimension>       ChainCodePathType;
  typedef typename ChainCodePathType::Pointer ChainCodePathPointer;

  typedef typename ChainCodePathType::OffsetType OffsetType;

  /** Smart Pointer type to a DataObject. */
  typedef typename DataObject::Pointer DataObjectPointer;

  /** Image dimension constants */
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      TInputImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      TOutputImage::ImageDimension);

  void PrintSelf ( std::ostream& os, Indent indent ) const override;

  /// Set/Get the seed
  itkSetMacro(Seed, IndexType);
  itkGetMacro(Seed, IndexType);

  /// Did we move the seed point to put in on a boundary?
  itkGetMacro(MovedSeed, bool);

  int GetThreshold();
  InputImagePixelType GetMaxIntensity() {return m_Max;}
  InputImagePixelType GetMinIntensity() {return m_Min;}

  /// Get the output as a ChainCodePath.  This output is only generated in the 2D case.
  ChainCodePathType *GetPathOutput() { return static_cast<ChainCodePathType*>(this->ProcessObject::GetOutput(1)); }

protected:
  LevelTracingImageFilter();
  ~LevelTracingImageFilter() override = default;

  /// Override since the filter needs all the data for the algorithm
  void GenerateInputRequestedRegion() override;

  /// Override since the filter produces the entire dataset
  void EnlargeOutputRequestedRegion(DataObject *output) override;

  void GenerateData() override;

  using Superclass::MakeOutput;
  DataObjectPointer MakeOutput(unsigned int output);

  /// To control overloaded versions of ComputeThreshold
  struct DispatchBase {};
  template<signed int VDimension>
  struct Dispatch : DispatchBase {};

  /** This method traces the level curve/surface. */
  virtual void Trace( const Dispatch<2> & );
  virtual void Trace( const DispatchBase &);

private:
  LevelTracingImageFilter(const Self&) = delete;
  void operator=(const Self&) = delete;

  IndexType           m_Seed;
  InputImagePixelType m_Max;
  InputImagePixelType m_Min;
  bool                m_MovedSeed;

};


//#ifdef ITK_USE_CONCEPT_CHECKING
///  /** Begin concept checking */
///  itkConceptMacro(OutputEqualityComparableCheck,
///    (Concept::EqualityComparable<OutputImagePixelType>));
///  itkConceptMacro(InputEqualityComparableCheck,
///    (Concept::EqualityComparable<InputImagePixelType>));
///  itkConceptMacro(SameDimensionCheck,
///    (Concept::SameDimension<InputImageDimension, OutputImageDimension>));
///  itkConceptMacro(IntConvertibleToInputCheck,
///    (Concept::Convertible<int, InputImagePixelType>));
///  itkConceptMacro(OutputOStreamWritableCheck,
///    (Concept::OStreamWritable<OutputImagePixelType>));
///  /** End concept checking */
//#endif

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLevelTracingImageFilter.txx"
#endif

#endif
