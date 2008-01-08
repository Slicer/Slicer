#ifndef __itkStochasticTractographyFilter_h__
#define __itkStochasticTractographyFilter_h__

#include "itkProcessObject.h"
#include "vnl/vnl_random.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix.h"
#include "itkArray.h"
#include "itkVectorContainer.h"
#include "vnl/algo/vnl_qr.h"
#include "vnl/algo/vnl_svd.h"
#include "itkVariableLengthVector.h"
#include "itkSimpleFastMutexLock.h"
#include "itkRealTimeClock.h"
#include "itkDiffusionTensor3D.h"
#include "itkPolyLineParametricPath.h"
#include "itkImage.h"
#include "itkProgressReporter.h"
#include <vector>

namespace itk{

/**Types for Probability Distribution **/
typedef Image< Array< double >, 3 > ProbabilityDistributionImageType;

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage,
  class TInputROIImage >
class ITK_EXPORT StochasticTractographyFilter :
  public ProcessObject{
public:
  typedef StochasticTractographyFilter Self;
  typedef ProcessObject Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;
  
  itkNewMacro(Self);
  itkTypeMacro( StochasticTractographyFilter,
    ProcessObject );
  
  /** Types for the DWI Input Image **/
  typedef TInputDWIImage InputDWIImageType;
  
  /** Types for the Mask Image **/
  typedef TInputWhiteMatterProbabilityImage InputWhiteMatterProbabilityImageType;
  
  /** Types for the ROI Image **/
  typedef TInputROIImage InputROIImageType;
  
  /** Tract Types **/
  typedef PolyLineParametricPath< 3 > TractType;
  
  /** Types for the TractContainer **/
  typedef VectorContainer< unsigned int, typename TractType::Pointer > 
    TractContainerType;
  
  /** Types for Tensor Output Image **/
  typedef Image< DiffusionTensor3D< float >, 3 > OutputTensorImageType;
  
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
  
  /** Set/Get the DWI Input Image **/
  itkSetInputMacro(DWIImage, InputDWIImageType, 0);
  itkGetInputMacro(DWIImage, InputDWIImageType, 0);
  
  /** Set/Get the White Matter Probability Input image **/
  /* At each voxel specifies the probability of a mylinated fiber existing
     at that location.  This probability is interpreted to be the probability
     that a fiber tract passes through that region.
     */
  itkSetInputMacro(WhiteMatterProbabilityImage, InputWhiteMatterProbabilityImageType, 1);
  itkGetInputMacro(WhiteMatterProbabilityImage, InputWhiteMatterProbabilityImageType, 1);
  
  /** Set/Get the ROI Input Image **/
  itkSetInputMacro(ROIImage, InputROIImageType, 2);
  itkGetInputMacro(ROIImage, InputROIImageType, 2)
  
  /** Set/Get the list of directions to sample **/
  itkSetConstObjectMacro( SampleDirections, TractOrientationContainerType );
  itkGetConstObjectMacro( SampleDirections, TractOrientationContainerType );
  
  /** Set/Get the Measurement Frame **/
  itkSetMacro( MeasurementFrame, MeasurementFrameType );
  itkGetMacro( MeasurementFrame, MeasurementFrameType );
  
  /** Set/Get the Maximum Likelihood Cache Size, the max num. of cached voxels **/
  itkSetMacro( MaxLikelihoodCacheSize, unsigned int );
  itkGetMacro( MaxLikelihoodCacheSize, unsigned int );
  
  /** Set/Get the tract segment size **/
  itkSetMacro( StepSize, double );
  itkGetMacro( StepSize, double );
  
  /** Set/Get Gamma, a prior parameter controlling the smoothness of the tracts **/
  itkSetMacro( Gamma, double );
  itkGetMacro( Gamma, double );
            
  /** Set/Get ROI Label **/
  itkSetMacro( ROILabel, unsigned int );
  itkGetMacro( ROILabel, unsigned int );
  
  /** Get the Continuous Tracts that are generated **/
  itkGetObjectMacro( OutputContinuousTractContainer, TractContainerType );
  
  /** Get the Discret Tracts that are generated
      these tracts are generate using the voxels selected by the probabilistic
      interpolation **/
  itkGetObjectMacro( OutputDiscreteTractContainer, TractContainerType );
  
  /** Get TensorImage **/
  itkGetObjectMacro( OutputTensorImage, OutputTensorImageType );
  
  /** Set/Get NearestNeighborInterpolation flag
      true: Use NN voxel interpolation
      false (default): Use Stochastic(Probabilistic) Interpolation
  **/
  itkSetMacro( NearestNeighborInterpolation, bool );
  itkGetMacro( NearestNeighborInterpolation, bool );
  
  /** Set/Get StreamlineTractography flag
      true: Perform simple major Eigenvector following streamline tractography
      false (default): Perform Stochastic Tractography
  **/
  itkSetMacro( StreamlineTractography, bool );
  itkGetMacro( StreamlineTractography, bool );

  /** Entry Point For the Algorithm:  Is invoked when Update() is called
      either directly or through itk pipeline propagation
  **/
  void GenerateData();
  void GenerateTensorImageOutput( void );
  
  /** override the Process Object Update because we don't have a
      dataobject as an output.  We can change this later by wrapping the
      tractcontainer in a dataobject decorator and letting the Superclass
      know about it.
  **/
  virtual void Update(){
    this->GenerateData();
  }
  
  /* Extract the likelihood from one voxel, more for debugging purposes */
  void ExtractVoxelLikelihood( const typename InputDWIImageType::IndexType seedindex,
    ProbabilityDistributionImageType::PixelType& likelihood );
  
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
  
  /** Type to store the rotation matrix for each voxel.
      This matrix rotates the grid of sampled directions so that at least
      one of the vectors points in the same direction as the estimated
      eigenvector.  This is neccessary in situations of extremely low
      residual variance, otherwise all sampled directions will have
      zero likelihood which results in inappropriate behavior **/
  typedef Image< vnl_matrix< double >, 3 > RotationImageType;
  
  StochasticTractographyFilter();
  virtual ~StochasticTractographyFilter();
  
  /** Load the default Sample Directions**/
  void LoadDefaultSampleDirections( void );
  
  /** Randomly chose a neighboring pixel weighted on distance **/
  void ProbabilisticallyInterpolate( vnl_random& randomgenerator, 
    const TractType::ContinuousIndexType& cindex,
    typename InputDWIImageType::IndexType& index);
                      
  /** Chose the nearest neighboring pixel **/
  void NearestNeighborInterpolate( const TractType::ContinuousIndexType& cindex,
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
  vnl_matrix< double >* m_AApinverse;   
  
  void CalculateConstrainedModelParameters( const TensorModelParamType& tensormodelparams,
    ConstrainedModelParamType& constrainedmodelparams);
  
  void CalculateNoiseFreeDWIFromConstrainedModel( const ConstrainedModelParamType& constrainedmodelparams,
    const GradientDirectionContainerType::Pointer gradients,
    DWIVectorImageType::PixelType& noisefreedwi);
  
  void CalculateResidualVariance( const DWIVectorImageType::PixelType& noisydwi,
    const DWIVectorImageType::PixelType& noisefreedwi,
    const vnl_diag_matrix< double >& W,
    const unsigned int numberofparameters,
    double& residualvariance);
  
  void CalculateLikelihood( const DWIVectorImageType::PixelType &dwipixel, 
    TractOrientationContainerType::ConstPointer orientations,
    ProbabilityDistributionImageType::PixelType& likelihood,
    RotationImageType::PixelType& rotation);
  
  void CalculatePrior( const TractOrientationContainerType::Element& v_prev, 
    TractOrientationContainerType::ConstPointer orientations,
    const RotationImageType::PixelType& rotation,
    ProbabilityDistributionImageType::PixelType& prior );
  
  void CalculatePosterior( const ProbabilityDistributionImageType::PixelType& likelihood,
    const ProbabilityDistributionImageType::PixelType& prior,
    ProbabilityDistributionImageType::PixelType& posterior);       
  
  void SampleTractOrientation( vnl_random& randomgenerator, 
    const ProbabilityDistributionImageType::PixelType& posterior,
    TractOrientationContainerType::ConstPointer orientations,
    const RotationImageType::PixelType& rotation,
    TractOrientationContainerType::Element& choosendirection );
  
  void StochasticTractGeneration( typename InputDWIImageType::ConstPointer dwiimagePtr,
    typename InputWhiteMatterProbabilityImageType::ConstPointer maskimagePtr,
    typename InputDWIImageType::IndexType index,
    unsigned long randomseed,
    TractType::Pointer conttracts[2],
    TractType::Pointer discretetracts[2] );
    
  void CalculateNuisanceParameters( const DWIVectorImageType::PixelType& dwivalues,
    const vnl_diag_matrix< double >& W,
    ConstrainedModelParamType& constrainedparams );

  /** Callback routine used by the threading library. This routine just calls
      the ThreadedGenerateData method after setting the correct region for this
      thread. **/
  static ITK_THREAD_RETURN_TYPE StochasticTractGenerationCallback( void *arg );
  
  struct StochasticTractGenerationCallbackStruct{
    Pointer Filter;
  };

  /** Allocates the tract output container **/
  void AllocateOutputs();
  
  /** Thread Safe Function to check/update an entry in the likelihood cache (also updates the rotation image) **/
  ProbabilityDistributionImageType::PixelType& 
    AccessLikelihoodCache( const typename InputDWIImageType::IndexType index );
    
  /** Thread Safe Function to delegate a tract and obtain a randomseed to start tracking **/
  bool DelegateTract(unsigned long& randomseed, typename InputDWIImageType::IndexType& index);
  
  /** Thread Safe Function to store a tract to a TractContainer **/
  void StoreContinuousTract(TractType::Pointer tract);
  
  /** Thread Safe Function to store a discrete tract to a TractContainer **/
  void StoreDiscreteTract(TractType::Pointer tract);
  
  /** Randomly samples the existence of a fiber tract in the current voxel **/
  bool FiberExistenceTest( vnl_random& randomgenerator,
    typename InputWhiteMatterProbabilityImageType::ConstPointer wmpimage,
    typename InputWhiteMatterProbabilityImageType::IndexType index );
  /** Picks the eigenvector associated with the largest eigenvalue that is in the direction of v_prev**/
  void PickLargestEigenvector( const DWIVectorImageType::PixelType &dwipixel,
    TractOrientationContainerType::Element v_prev,
    TractOrientationContainerType::Element& v_curr);
  
  GradientDirectionContainerType::ConstPointer m_Gradients;
  GradientDirectionContainerType::Pointer m_TransformedGradients;
  
  bValueContainerType::ConstPointer m_bValues;
  MeasurementFrameType m_MeasurementFrame;
  ProbabilityDistributionImageType::Pointer m_LikelihoodCachePtr;
  LikelihoodCacheMutexImageType::Pointer m_LikelihoodCacheMutexImagePtr;
  unsigned int m_MaxTractLength;
  unsigned int m_TotalTracts;

  TractOrientationContainerType::ConstPointer   m_SampleDirections;
  unsigned long m_MaxLikelihoodCacheSize;   //in Megabytes
  unsigned long m_MaxLikelihoodCacheElements;  //in Elements (Voxels)
  unsigned long m_CurrentLikelihoodCacheElements;
  SimpleFastMutexLock m_LikelihoodCacheMutex;
  
  double m_StepSize;
  double m_Gamma;
  unsigned int m_ROILabel;
  unsigned int m_TotalDelegatedTracts;
  SimpleFastMutexLock m_TotalDelegatedTractsMutex;
  RealTimeClock::Pointer m_ClockPtr;
  //unsigned long m_RandomSeed;
  SimpleFastMutexLock m_OutputImageMutex;
  
  TractContainerType::Pointer m_OutputContinuousTractContainer;
  SimpleFastMutexLock m_OutputContinuousTractContainerMutex;
  
  TractContainerType::Pointer m_OutputDiscreteTractContainer;
  SimpleFastMutexLock m_OutputDiscreteTractContainerMutex;
  
  OutputTensorImageType::Pointer m_OutputTensorImage;
  vnl_random m_RandomGenerator;
  
  std::vector< typename InputDWIImageType::IndexType > m_SeedIndices;
  ProgressReporter* m_progress;
  
  bool m_NearestNeighborInterpolation;
  bool m_StreamlineTractography;
  
  RotationImageType::Pointer m_RotationImagePtr;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkStochasticTractographyFilter.txx"
#include "itkStochasticTractographyFilter_SD.txx"
#endif

#endif
