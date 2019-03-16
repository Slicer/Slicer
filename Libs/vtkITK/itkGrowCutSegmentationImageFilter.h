#ifndef itkGrowCutSegmentationImageFilter_h
#define itkGrowCutSegmentationImageFilter_h

#include "itkImage.h"
#include "itkImageToImageFilter.h"
#include "itkSimpleDataObjectDecorator.h"
#include "itkVectorContainer.h"
//#include "itkCommand.h"

//#include "itkGrowCutSegmentationUpdateFilter.h"

#include <iostream>
#include <list>
#include <vector>

#ifndef PixelState
enum PixelState{
    UNLABELED = 0,
    LABELED = 1,
    LOCALLY_SATURATED = 2,
    SATURATED = 3 };
#endif

namespace itk
{

/**  \class GrowCutSegmentationFilter
 * \brief Segmentation or multiple objects based on a set of gestures.
 *
 * Given a mask image containing the gestures for foreground classes
 * and their background, employs grow cut segmentation to produce
 * segmentation. Supports passing an existing segmentation with
 * additional gestures for editing a segmentation produced as a
 * result of the same or different algorithm.
 *
 * The filter is based on the paper "GrowCut:Interactive Multi-Label
 * N-D Image Segmentation By Cellular Automata", Vladimir Vezhnevets,
 * Vadim Konouchine
 *
 * Modified Version: The inputs consist of the input intensity image,
 * optionally the input Label Image, and the weight image.  The Label
 * image and the weight image default to using unsigned chars and
 * double respectively.
 *
 * This algorithm is implemented scalar images. Vector Images are not
 * supported.
 *
 *
**/


/* template<class TInputImage,  */
/*   class TOutputImage, class TLabelPixelType = short,  */
/*   class TWeightPixelType = float >  */
template<class TInputImage,
  class TOutputImage,
  class TWeightPixelType = float>
  class GrowCutSegmentationImageFilter: public ImageToImageFilter<TInputImage,TOutputImage>
{

 public:
  /** Standard class typedefs. */
  typedef GrowCutSegmentationImageFilter               Self;
  typedef ImageToImageFilter<TInputImage,TOutputImage> Superclass;
  typedef SmartPointer<Self>                           Pointer;
  typedef SmartPointer<const Self>                     ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods).  */
  itkTypeMacro(GrowCutSegmentationImageFilter,
               ImageToImageFilter);

  /** Image related typedefs. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      TInputImage::ImageDimension );

  typedef TInputImage                           InputImageType;
  typedef typename InputImageType::Pointer      InputImagePointer;
  typedef typename InputImageType::ConstPointer InputImageConstPointer;

  typedef typename InputImageType::PixelType InputPixelType;
  typedef typename InputImageType::IndexType InputIndexType;
  typedef typename InputImageType::SizeType  SizeType;

  typedef TOutputImage                         OutputImageType;
  typedef typename OutputImageType::Pointer    OutputImagePointer;
  typedef typename OutputImageType::RegionType OutputImageRegionType;
  typedef typename OutputImageType::PixelType  OutputPixelType;
  typedef typename OutputImageType::IndexType  OutputIndexType;
  typedef typename InputImageType::SizeType    OutputSizeType;


  /** Smart Pointer type to a DataObject. */
  typedef typename DataObject::Pointer DataObjectPointer;


   /** Image dimension constants */
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      TInputImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      TOutputImage::ImageDimension);

   /** Index typedef support. */
  typedef Index<itkGetStaticConstMacro(InputImageDimension)> IndexType;

 /** InputSizeType typedef support **/
  typedef typename InputImageType::SizeType InputSizeType;

  /* NodeContainer typedef support for storing a set of seed points */
  typedef VectorContainer<unsigned int, IndexType> NodeContainer;

  /* NodeContainer pointer support */
  typedef typename NodeContainer::Pointer NodeContainerPointer;

  /** enum of the growcut segmentation. NoLabel represents points
    that have not been assigned any label. Object represents points
    that are assigned to foreground, and Background represents points
    that are assigned to the background **/
  enum LabelType { NoLabel, ObjectLabel, BackgroundLabel };


  /** WeightImage typedef support */
  /* indicates the strength of a label for a given cell */
  typedef Image<TWeightPixelType, itkGetStaticConstMacro(InputImageDimension) > WeightImageType;

  /** WeightImagePointer typedef support. */
  typedef typename WeightImageType::Pointer WeightImagePointer;

  typedef TWeightPixelType WeightPixelType;

  /** Set the Input Image **/
  void SetInputImage( const InputImageType *in)
  {
    this->ProcessObject::SetNthInput(0, const_cast< InputImageType *>(in) );
  }

  const InputImagePointer GetInputImage( );

  /** Set/Get the Label Image **/
  void SetLabelImage( const OutputImageType *f)
  {
    this->ProcessObject::SetNthInput(1, const_cast< OutputImageType *>(f) );
    m_LabelImage = static_cast< OutputImageType *>(this->ProcessObject::GetInput(1));
  }

  const OutputImagePointer GetLabelImage();

  /** Get the Weight image **/
  const WeightImagePointer GetStrengthImage();

  /** Set the Weight Image **/
  void SetStrengthImage( const WeightImageType *w )
  {
    this->ProcessObject::SetNthInput(2,const_cast< WeightImageType *>(w));
    //m_WeightImage = static_cast< WeightImageType *>(this->ProcessObject::GetInput(2));
  }

  const WeightImagePointer GetUpdatedStrengthImage();

  /** Set/Get Methods for state, distance, and maxSaturation **/
  void SetStateImage( const OutputImageType *l);
  const OutputImagePointer GetStateImage();

  void SetDistancesImage( const WeightImageType *d);
  const WeightImagePointer GetDistancesImage();

  void SetMaxSaturationImage( const WeightImageType *w);
  const WeightImagePointer GetMaxSaturationImage();

  /** Set the initial strength **/
  itkSetMacro( SeedStrength, double );

  /** Get the seed strength **/
  itkGetConstMacro( SeedStrength, double );

  /** Set/Get the number of Labeled pixels **/
  itkSetMacro( Labeled, unsigned int);
  itkGetMacro( Labeled, unsigned int);

  /** Set/Get the number of locally saturated pixels **/
  itkSetMacro( LocallySaturated, unsigned int);
  itkGetMacro( LocallySaturated, unsigned int);

  /** Set/Get the number of Saturated pixels **/
  itkSetMacro( Saturated, unsigned int);
  itkGetMacro( Saturated, unsigned int);

  /** Set the number of iterations **/
  itkSetMacro( MaxIterations, unsigned int );

  /** Get the number of iterations **/
  itkGetConstMacro( MaxIterations, unsigned int );

 /** Set the number of iterations **/
  itkSetMacro( ObjectRadius, unsigned int );

  /** Get the number of iterations **/
  itkGetConstMacro( ObjectRadius, unsigned int );

  /** Set/Get the ROI start and End **/
  void SetROIStart( const OutputIndexType &start)
  {
    m_RoiStart = start;
  }

  OutputIndexType GetROIStart() const
  {
    return m_RoiStart;
  }


  void SetROIEnd(const OutputIndexType &end)
  {
    m_RoiEnd = end;
  }


  OutputIndexType GetROIEnd() const
  {
    return m_RoiEnd;
  }

  /** Set the radius of the neighborhood for processing. Default is 1 **/
  itkSetMacro( Radius, InputSizeType );

  /** Get the radius of neighborhood used for processing **/
  itkGetConstMacro( Radius, InputSizeType );

  /**Set/Get whether the filter should run one iteration or until
     convergence.  When set to run to convergence, set OneIteration to
     Off which is the default behavior for this filter. **/
  itkSetMacro(RunOneIteration, bool);
  itkGetConstMacro(RunOneIteration, bool);
  itkBooleanMacro(RunOneIteration);


  /**Set/Get whether the distancesImage has already been set for the
  * filter.  Default setting is off in which case the filter
  * automatically initializes the distances image.
  **/
  itkSetMacro(SetDistancesImage, bool);
  itkGetConstMacro(SetDistancesImage, bool);
  itkBooleanMacro(SetDistancesImage);

  /**Set/Get whether the stateImage has already been set for the filter.
  * Default setting is off in which case the filter automatically initializes
  * the state image.
  **/
  itkSetMacro(SetStateImage, bool);
  itkGetConstMacro(SetStateImage, bool);
  itkBooleanMacro(SetStateImage);

  /**Set/Get whether the maxSaturationImage has already been set for the filter.
  * Default setting is off in which case the filter automatically initializes
  * the maxSaturationImage image.
  **/
  itkSetMacro(SetMaxSaturationImage, bool);
  itkGetConstMacro(SetMaxSaturationImage, bool);
  itkBooleanMacro(SetMaxSaturationImage);

 protected:

  GrowCutSegmentationImageFilter();
  ~GrowCutSegmentationImageFilter() override  = default;

   // Override since the filter needs all the data for the algorithm
  void GenerateInputRequestedRegion() override;

   // Override since the filter produces the entire dataset
  void EnlargeOutputRequestedRegion(DataObject *output) override;

  void GenerateData() override;

  void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread ,
                             ThreadIdType threadId ) override;

  void AfterThreadedGenerateData() override;

  void Initialize(OutputImageType* output);

  void PrintSelf ( std::ostream& os, Indent indent ) const override;

  void GrowCutSlowROI( TOutputImage *);


 private:

  GrowCutSegmentationImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); // purposely not implemented

  bool InitializeStateImage( OutputImageType *state );

  void InitializeDistancesImage(TInputImage *input,WeightImageType *distance);

  void GetRegionOfInterest();

  void ComputeLabelVolumes(TOutputImage *outputImage, std::vector< unsigned > &volumes, std::vector< unsigned > &phyVolumes);

  void MaskSegmentedImageByWeight(float upperThresh);


  WeightPixelType                            m_ConfThresh;
  InputSizeType                              m_Radius;
  OutputImagePointer                         m_LabelImage;
  WeightImagePointer                         m_WeightImage;
  unsigned int                               m_Labeled;
  unsigned int                               m_LocallySaturated;
  unsigned int                               m_Saturated;

  double                                     m_SeedStrength;
  bool                                       m_RunOneIteration;
  bool                                       m_SetStateImage;
  bool                                       m_SetDistancesImage;
  bool                                       m_SetMaxSaturationImage;

  unsigned int                               m_MaxIterations;
  unsigned int                               m_ObjectRadius;

  OutputPixelType                        m_ObjectLabel;
  OutputPixelType                        m_BackgroundLabel;
  OutputPixelType                        m_UnknownLabel;

  OutputIndexType                            m_RoiStart;
  OutputIndexType                            m_RoiEnd;

};

} // namespace itk


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGrowCutSegmentationImageFilter.txx"
#endif

#endif
