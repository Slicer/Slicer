#ifndef __itkStochasticTractographyFilter_h__
#define __itkStochasticTractographyFilter_h__

#include "itkImageToImageFilter.h"
#include "vnl/vnl_random.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix.h"
#include "itkArray.h"
#include "itkVectorContainer.h"
#include "vnl/algo/vnl_qr.h"
#include "itkVariableLengthVector.h"
#include "itkSlowPolyLineParametricPath.h"
#include "itkSimpleFastMutexLock.h"
#include "itkRealTimeClock.h"
#include "itkDiffusionTensor3D.h"
#include <vector>

namespace itk{

/**Types for Probability Distribution **/
typedef Image< Array< double >, 3 > ProbabilityDistributionImageType;

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage,
  class TOutputConnectivityImage >
class ITK_EXPORT StochasticTractographyFilter :
  public ImageToImageFilter< TInputDWIImage,
                    TOutputConnectivityImage >{
public:
  typedef StochasticTractographyFilter Self;
  typedef ImageToImageFilter< TInputDWIImage,
    TOutputConnectivityImage > Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;
  
  itkNewMacro(Self);
  itkTypeMacro( StochasticTractographyFilter,
            ImageToImageFilter );
  
  /** Types for the DWI Input Image **/
  typedef TInputDWIImage InputDWIImageType;
  
  /** Types for the Connectivity Output Image**/
  typedef TOutputConnectivityImage OutputConnectivityImageType;
  
  /** Types for the Mask Image **/
  typedef TInputWhiteMatterProbabilityImage InputWhiteMatterProbabilityImageType;
  
  /** Tract Types **/
  typedef SlowPolyLineParametricPath< 3 > TractType;
  
  /** Types for the TractContainer **/
  typedef VectorContainer< unsigned int, typename TractType::Pointer > 
    TractContainerType;
  
  /** Types for Tensor Output Image **/
  typedef Image< DiffusionTensor3D< double >, 3 > OutputTensorImageType;
  
  /** Types for the Image-wide Magnetic Field Gradient Directions **/
  typedef VectorContainer< unsigned int, vnl_vector_fixed< double, 3 > >
    GradientDirectionContainerType;
  
  /** Types for the Image-wide bValues **/
  typedef double bValueType;
  typedef VectorContainer< unsigned int, bValueType > bValueContainerType;
   
  /** Types for the Measurement Frame of the Gradients **/
  typedef vnl_matrix_fixed< double, 3, 3 > MeasurementFrameType;

  /** Type for the sample directions **/
  typedef VectorContainer< unsigned int, vnl_vector_fixed< double, 3 > > 
    TractOrientationContainerType;
  
  /** the number of Tracts to generate **/
  itkSetMacro( TotalTracts, unsigned int);
  itkGetMacro( TotalTracts, unsigned int);
  
  /** the maximum length of Tract **/
  itkSetMacro( MaxTractLength, unsigned int );
  itkGetMacro( MaxTractLength, unsigned int );
  
  /** Set/Get bvalues **/
  itkSetConstObjectMacro( bValues, bValueContainerType );
  itkGetConstObjectMacro( bValues, bValueContainerType );
  
  /** Set/Get of gradient directions **/
  itkSetConstObjectMacro( Gradients, GradientDirectionContainerType );
  itkGetConstObjectMacro( Gradients, GradientDirectionContainerType );
  
  /** Set/Get the White Matter Probability Input image **/
  /* At each voxel specifies the probability of a mylinated fiber existing
     at that location.  This probability is interpreted to be the probability
     that a fiber tract passes through that region.
     */
  itkSetInputMacro(WhiteMatterProbabilityImage, InputWhiteMatterProbabilityImageType, 1);
  itkGetInputMacro(WhiteMatterProbabilityImage, InputWhiteMatterProbabilityImageType, 1);
  
  //overide the built in set input function
  //we need to create a new cache everytime we change the input image
  //but we need to preserve it when the input image is the same
  void SetInput( typename InputDWIImageType::Pointer dwiimagePtr ){
    Superclass::SetInput( dwiimagePtr );
    //update the likelihood cache
    this->m_LikelihoodCachePtr = ProbabilityDistributionImageType::New();
    this->m_LikelihoodCachePtr->CopyInformation( this->GetInput() );
    this->m_LikelihoodCachePtr->SetBufferedRegion( this->GetInput()->GetBufferedRegion() );
    this->m_LikelihoodCachePtr->SetRequestedRegion( this->GetInput()->GetRequestedRegion() );
    this->m_LikelihoodCachePtr->Allocate();
    this->m_CurrentLikelihoodCacheElements = 0;
    //update the likelihoodcache mutex image
    this->m_LikelihoodCacheMutexImagePtr = LikelihoodCacheMutexImageType::New();
    this->m_LikelihoodCacheMutexImagePtr->CopyInformation( this->GetInput() );
    this->m_LikelihoodCacheMutexImagePtr->SetBufferedRegion( this->GetInput()->GetBufferedRegion() );
    this->m_LikelihoodCacheMutexImagePtr->SetRequestedRegion( this->GetInput()->GetRequestedRegion() );
    this->m_LikelihoodCacheMutexImagePtr->Allocate();
  }
  
  /** Set/Get the seed index **/
  itkSetMacro( SeedIndex, typename InputDWIImageType::IndexType );
  itkGetMacro( SeedIndex, typename InputDWIImageType::IndexType );
  
  /** Set/Get the list of directions to sample **/
  itkSetConstObjectMacro( SampleDirections, TractOrientationContainerType );
  itkGetConstObjectMacro( SampleDirections, TractOrientationContainerType );
  
  /** Set/Get the Measurement Frame **/
  itkSetMacro( MeasurementFrame, MeasurementFrameType );
  itkGetMacro( MeasurementFrame, MeasurementFrameType );
  
  /** Set/Get the Maximum Likelihood Cache Size, the max num. of cached voxels **/
  itkSetMacro( MaxLikelihoodCacheSize, unsigned int );
  itkGetMacro( MaxLikelihoodCacheSize, unsigned int );
            
  /** Get the Tracts that are generated **/
  itkGetObjectMacro( OutputTractContainer, TractContainerType );
  
  /** Get TensorImage **/
  itkGetObjectMacro( OutputTensorImage, OutputTensorImageType );
  
  void GenerateData();
  void GenerateTractContainerOutput( void );
  void GenerateTensorImageOutput( void );
  
protected:
  /** Convenience Types used only inside the filter **/
    
  /**Types for the parameters of the Tensor Model **/
  typedef vnl_vector_fixed< double, 7 > TensorModelParamType;
  
  /**Types for the parameters of the Constrained Model **/
  typedef vnl_vector_fixed< double, 6 > ConstrainedModelParamType;
  
  /**Type to hold generated DWI values**/
  typedef Image< VariableLengthVector< double >, 3 > DWIVectorImageType;
  
  /**Types for Probability Distribution **/
  typedef Image< Array< double >, 3 > ProbabilityDistributionImageType;
  
  /** Types for the Image of Mutexes of the Likelihood distribution **/
  typedef Image< SimpleFastMutexLock, 3 > LikelihoodCacheMutexImageType;
  
  StochasticTractographyFilter();
  virtual ~StochasticTractographyFilter();
  
  /** Load the default Sample Directions**/
  void LoadDefaultSampleDirections( void );
  
  /** Randomly chose a neighboring pixel weighted on distance **/
  void ProbabilisticallyInterpolate( vnl_random& randomgenerator, 
    const TractType::ContinuousIndexType& cindex,
    typename InputDWIImageType::IndexType& index);
                      
  /** Functions and data related to fitting the tensor model at each pixel **/
  void UpdateGradientDirections(void);
  void UpdateTensorModelFittingMatrices( void );
  void CalculateTensorModelParameters( const DWIVectorImageType::PixelType& dwivalues,
    vnl_diag_matrix<double>& W,
    TensorModelParamType& tensormodelparams);
  //these will be the same for every pixel in the image so
  //go ahead and do a QR decomposition to optimize the
  //LS fitting process for estimating the weighing matrix W
  //in this case we solve instead:
  //R*Beta = Q'logPhi
  vnl_matrix< double >* m_A;
  vnl_qr< double >* m_Aqr;    
  
  void CalculateConstrainedModelParameters( const TensorModelParamType& tensormodelparams,
    ConstrainedModelParamType& constrainedmodelparams);
  
  void CalculateNoiseFreeDWIFromConstrainedModel( const ConstrainedModelParamType& constrainedmodelparams,
    DWIVectorImageType::PixelType& noisefreedwi);
  
  void CalculateResidualVariance( const DWIVectorImageType::PixelType& noisydwi,
    const DWIVectorImageType::PixelType& noisefreedwi,
    const vnl_diag_matrix< double >& W,
    const unsigned int numberofparameters,
    double& residualvariance);
  
  void CalculateLikelihood( const DWIVectorImageType::PixelType &dwipixel, 
    TractOrientationContainerType::ConstPointer orientations,
    ProbabilityDistributionImageType::PixelType& likelihood);
  
  void CalculatePrior( TractOrientationContainerType::Element v_prev, 
    TractOrientationContainerType::ConstPointer orientations,
    ProbabilityDistributionImageType::PixelType& prior );
  
  void CalculatePosterior( const ProbabilityDistributionImageType::PixelType& likelihood,
    const ProbabilityDistributionImageType::PixelType& prior,
    ProbabilityDistributionImageType::PixelType& posterior);       
  
  void SampleTractOrientation( vnl_random& randomgenerator, 
    const ProbabilityDistributionImageType::PixelType& posterior,
    TractOrientationContainerType::ConstPointer orientations,
    TractOrientationContainerType::Element& choosendirection );
  
  void StochasticTractGeneration( typename InputDWIImageType::ConstPointer dwiimagePtr,
    typename InputWhiteMatterProbabilityImageType::ConstPointer maskimagePtr,
    typename InputDWIImageType::IndexType seedindex,
    unsigned long randomseed,
    TractType::Pointer tract );

  /** Callback routine used by the threading library. This routine just calls
      the ThreadedGenerateData method after setting the correct region for this
      thread. **/
  static ITK_THREAD_RETURN_TYPE StochasticTractGenerationCallback( void *arg );
  
  struct StochasticTractGenerationCallbackStruct{
    Pointer Filter;
  };

  /** Thread Safe Function to check/update an entry in the likelihood cache **/
  ProbabilityDistributionImageType::PixelType& 
    AccessLikelihoodCache( typename InputDWIImageType::IndexType index );
  /** Thread Safe Function to delegate a tract and obtain a randomseed to start tracking **/
  bool DelegateTract(unsigned long& randomseed);
  /** Function to write a tract to the connectivity map **/
  void TractContainerToConnectivityMap(TractContainerType::Pointer tractcontainer);
  /** Thread Safe Function to store a tract to a TractContainer **/
  void StoreTract(TractType::Pointer tract);
  /** Randomly samples the existence of a fiber tract in the current voxel **/
  bool FiberExistenceTest( vnl_random& randomgenerator,
    typename InputWhiteMatterProbabilityImageType::ConstPointer wmpimage,
    typename InputWhiteMatterProbabilityImageType::IndexType index );
  
  GradientDirectionContainerType::ConstPointer m_Gradients;
  GradientDirectionContainerType::Pointer m_TransformedGradients;
  
  bValueContainerType::ConstPointer m_bValues;
  MeasurementFrameType m_MeasurementFrame;
  ProbabilityDistributionImageType::Pointer m_LikelihoodCachePtr;
  LikelihoodCacheMutexImageType::Pointer m_LikelihoodCacheMutexImagePtr;
  unsigned int m_MaxTractLength;
  unsigned int m_TotalTracts;
  typename InputDWIImageType::IndexType m_SeedIndex;
  TractOrientationContainerType::ConstPointer m_SampleDirections;
  unsigned long m_MaxLikelihoodCacheSize;   //in Megabytes
  unsigned long m_MaxLikelihoodCacheElements;  //in Elements (Voxels)
  unsigned long m_CurrentLikelihoodCacheElements;
  SimpleFastMutexLock m_LikelihoodCacheMutex;
  
  unsigned int m_TotalDelegatedTracts;
  SimpleFastMutexLock m_TotalDelegatedTractsMutex;
  RealTimeClock::Pointer m_ClockPtr;
  //unsigned long m_RandomSeed;
  SimpleFastMutexLock m_OutputImageMutex;
  TractContainerType::Pointer m_OutputTractContainer;
  SimpleFastMutexLock m_OutputTractContainerMutex;
  
  OutputTensorImageType::Pointer m_OutputTensorImage;
  vnl_random m_RandomGenerator;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkStochasticTractographyFilter.txx"
#include "itkStochasticTractographyFilter_SD.txx"
#endif

#endif
