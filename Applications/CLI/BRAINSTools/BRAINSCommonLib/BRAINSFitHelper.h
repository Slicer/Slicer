#ifndef  __BRAINSFITHELPER__
#define  __BRAINSFITHELPER__

/**
 * \author Hans J. Johnson
 *
 * The intension of the BRIANSFitHelper is to provide a simple non-templated class
 * that can be used in other programs in a way that is very similar to the
 * command line version of the program from the SlicerExecutionModel version
 * of the BRAINSFitPrimary program.
 *
 * Almost all the command line options are available in this version, but
 * there is no need to read or write files to disk in order to use this class.
 *
 */
#include <fstream>
#include <vector>
#include <string>


// TODO:  This needs to be moved to the top, and header files moved to this
// header where needed.
#include "BRAINSFitBSpline.h"
#include "BRAINSFitUtils.h"

#include "itkFindCenterOfBrainFilter.h"
#include "itkMedianImageFilter.h"
#include "itkHistogramMatchingImageFilter.h"

#include "itkTransformFactory.h"
#include "itkIO.h"

#include "itkCenteredVersorTransformInitializer.h"
#include "itkCenteredTransformInitializer.h"

#include "itkVersorRigid3DTransformOptimizer.h"
#include "itkVersorRigid3DTransformOptimizer.h"
#include "itkVersorTransformOptimizer.h"

#include "itkVector.h"
#include "itkMultiThreader.h"

#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"
#include "itkExtractImageFilter.h"

#include "itkResampleImageFilter.h"
#include "itkExtractImageFilter.h"
#include "itkAffineTransform.h"
#include <stdio.h>
#include "ConvertToRigidAffine.h"

#include "itkMultiModal3DMutualRegistrationHelper.h"
#include "itkMattesMutualInformationImageToImageMetric.h"

#include "ReadMask.h"
#include "BRAINSMacro.h"

#include "itkImageMaskSpatialObject.h"
typedef itk::SpatialObject<3>  SpatialObjectType;
typedef SpatialObjectType::Pointer ImageMaskPointer;

//TODO:  The next two should be hidden in the cxx files again.
typedef itk::TransformFileReader                    TransformReaderType;
typedef itk::TransformFileReader::TransformListType TransformListType;

//TODO:  This should really be taken as a typedef from the BSpline class.
//TODO:  These should be hidden in the BRAINSFit namespace.
typedef itk::Transform<double,3,3> GenericTransformType;

namespace itk {
  //Functions for writing out transforms to disk
  extern int WriteBothTransformsToDisk(GenericTransformType::Pointer & genericTransformToWrite, const std::string & outputTransform, const std::string & strippedOutputTransform);
  extern int WriteStrippedRigidTransformToDisk(GenericTransformType::Pointer & genericTransformToWrite, const std::string & strippedOutputTransform);
  extern int WriteTransformToDisk(GenericTransformType::Pointer & genericTransformToWrite, const std::string & outputTransform);
  extern GenericTransformType::Pointer ReadTransformFromDisk(const std::string initialTransform);

  /** Method for verifying that the ordering of the transformTypes is consistent with converting routines. */
  extern void ValidateTransformRankOrdering(const std::vector<std::string> & transformType);
}

namespace itk
{
class ITK_EXPORT BRAINSFitHelper : public Object
  {
public:
  /** Standard class typedefs. */
  typedef BRAINSFitHelper                Self;
  typedef ProcessObject                  Superclass;
  typedef SmartPointer<Self>             Pointer;
  typedef SmartPointer<const Self>       ConstPointer;

  typedef float                          PixelType;
  typedef itk::Image<PixelType, 3>       FixedVolumeType;
  typedef FixedVolumeType::ConstPointer  FixedImageConstPointer;
  typedef FixedVolumeType::Pointer       FixedImagePointer;

  typedef itk::Image<PixelType, 3>       MovingVolumeType;
  typedef MovingVolumeType::ConstPointer MovingImageConstPointer;
  typedef MovingVolumeType::Pointer      MovingImagePointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(BRAINSFitHelper, ProcessObject);

  /** Set/Get the Fixed image. */
  itkSetObjectMacro(FixedVolume, FixedVolumeType );
  itkGetConstObjectMacro( FixedVolume, FixedVolumeType );

  /** Set/Get the Moving image. */
  itkSetObjectMacro( MovingVolume, MovingVolumeType)
  itkGetConstObjectMacro( MovingVolume, MovingVolumeType );

  /** The preprocessedMoving volume SHOULD NOT BE SET, you can get it out of the algorithm.*/
  itkGetConstObjectMacro( PreprocessedMovingVolume, MovingVolumeType );

  typedef MattesMutualInformationImageToImageMetric<FixedVolumeType,
    MovingVolumeType> MetricType;
  typedef MetricType::FixedImageMaskType
  FixedBinaryVolumeType;
  typedef FixedBinaryVolumeType::Pointer
  FixedBinaryVolumePointer;
  typedef MetricType::MovingImageMaskType
  MovingBinaryVolumeType;
  typedef MovingBinaryVolumeType::Pointer
  MovingBinaryVolumePointer;


  itkSetObjectMacro ( FixedBinaryVolume, FixedBinaryVolumeType );
  itkGetConstObjectMacro( FixedBinaryVolume, FixedBinaryVolumeType );

  itkSetObjectMacro( MovingBinaryVolume, MovingBinaryVolumeType );
  itkGetConstObjectMacro( MovingBinaryVolume, MovingBinaryVolumeType );

  typedef enum {
    LINEAR_INTERP = 0,
    WINDOWSINC_INTERP = 1,
    } InterpolationType;

  // FixedVolumeType::Pointer GetResampledImage(
  //   InterpolationType newInterp) const;



  itkSetMacro( NumberOfSamples,        unsigned int  );
  itkSetMacro( NumberOfHistogramBins,         unsigned int  );
  itkSetMacro( NumberOfMatchPoints,           unsigned int  );
  VECTORitkSetMacro( NumberOfIterations,   std::vector<int>/**/ );
  itkSetMacro( MaximumStepLength,             double        );
  VECTORitkSetMacro( MinimumStepLength, std::vector<double> );
  itkSetMacro( RelaxationFactor,              double        );
  itkSetMacro( TranslationScale,              double        );
  itkSetMacro( ReproportionScale,             double        );
  itkSetMacro( SkewScale,                     double        );
  itkSetMacro( UseExplicitPDFDerivativesMode, std::string   );
  itkSetMacro( UseCachingOfBSplineWeightsMode,std::string   );
  itkSetMacro( CostFunctionConvergenceFactor, double        );
  itkSetMacro( ProjectedGradientTolerance,    double        );
  itkSetMacro( MaxBSplineDisplacement,        double        );
  itkSetMacro( BackgroundFillValue,           double        );
  VECTORitkSetMacro( TransformType, std::vector<std::string>);
  itkSetMacro( InitializeTransformMode, std::string         );
  itkSetMacro( MaskInferiorCutOffFromCenter, double         );
  itkSetMacro( CurrentGenericTransform,  GenericTransformType::Pointer );
  itkGetConstMacro( CurrentGenericTransform,  GenericTransformType::Pointer );
  itkSetMacro( UseWindowedSinc, bool                        );
  VECTORitkSetMacro( SplineGridSize, std::vector<int>       );

  itkGetConstMacro( ActualNumberOfIterations,      unsigned int  );
  itkGetConstMacro( PermittedNumberOfIterations,   unsigned int  );
  //itkGetConstMacro( AccumulatedNumberOfIterationsForAllLevels, unsigned int);
  itkGetConstMacro( FinalMetricValue,         double        );
  //
  // Set/Get the Debugging level for filter verboseness
  itkSetMacro(DebugLevel, unsigned int);
  itkGetConstMacro(DebugLevel, unsigned int);
  itkSetMacro(DisplayDeformedImage,bool);
  itkSetMacro(PromptUserAfterDisplay,bool);
  itkSetMacro(      ObserveIterations,        bool          );
  itkGetConstMacro( ObserveIterations,        bool          );
  /** Method to set the Permission to vary by level  */
  void SetPermitParameterVariation(std::vector<int> perms)
  {
    m_PermitParameterVariation.resize( perms.size() );
    for ( unsigned int i = 0; i < perms.size(); i++ )
      {
      m_PermitParameterVariation[i] = perms[i];
      }
  }

  itkSetMacro(HistogramMatch, bool);
  itkGetConstMacro(HistogramMatch, bool);

  /** Method that initiates the registration. */
  void StartRegistration(void);
  void PrintCommandLine(const bool dumpTempVolumes, const std::string suffix) const;

protected:
  BRAINSFitHelper();
  virtual ~BRAINSFitHelper() {};

  void PrintSelf(std::ostream & os, Indent indent) const;

  /** Method invoked by the pipeline in order to trigger the computation of
   * the registration. */
  void  GenerateData ();

private:
  BRAINSFitHelper(const Self &); // purposely not implemented
  void operator=(const Self &);  // purposely not implemented

  FixedImagePointer  m_FixedVolume;
  MovingImagePointer m_MovingVolume;
  MovingImagePointer m_PreprocessedMovingVolume;

  FixedBinaryVolumePointer  m_FixedBinaryVolume;
  MovingBinaryVolumePointer m_MovingBinaryVolume;
  std::vector<int> m_PermitParameterVariation;

  unsigned int     m_NumberOfSamples;
  unsigned int     m_NumberOfHistogramBins;
  bool             m_HistogramMatch;
  unsigned int     m_NumberOfMatchPoints;
  // TODO:  Would be better to have unsigned int
  std::vector<int> m_NumberOfIterations;
  double                   m_MaximumStepLength;
  std::vector<double>      m_MinimumStepLength;
  double                   m_RelaxationFactor;
  double                   m_TranslationScale;
  double                   m_ReproportionScale;
  double                   m_SkewScale;
  std::string              m_UseExplicitPDFDerivativesMode;
  std::string              m_UseCachingOfBSplineWeightsMode;
  double                   m_BackgroundFillValue;
  std::vector<std::string> m_TransformType;
  std::string              m_InitializeTransformMode;
  double                   m_MaskInferiorCutOffFromCenter;
  bool                     m_UseWindowedSinc;
  std::vector<int>         m_SplineGridSize;
  double                   m_CostFunctionConvergenceFactor;
  double                   m_ProjectedGradientTolerance;
  double                   m_MaxBSplineDisplacement;
  unsigned int             m_ActualNumberOfIterations;
  unsigned int             m_PermittedNumberOfIterations;
  //unsigned int             m_AccumulatedNumberOfIterationsForAllLevels;
  unsigned int             m_DebugLevel;
  GenericTransformType::Pointer m_CurrentGenericTransform;
  bool   m_DisplayDeformedImage;
  bool   m_PromptUserAfterDisplay;
  double m_FinalMetricValue;
  bool   m_ObserveIterations;
  }; // end BRAINSFitHelper class
} // end namespace itk


template <class TransformType>
void WriteTransform(const typename TransformType::Pointer & MyTransform,
  const std::string & TransformFilename)
{
  /*
   *  Convert the transform to the appropriate assumptions and write it out as requested.
   */
    {
    typedef itk::TransformFileWriter TransformWriterType;
    TransformWriterType::Pointer transformWriter =  TransformWriterType::New();
    transformWriter->SetFileName( TransformFilename.c_str() );
    transformWriter->SetInput( MyTransform );
    transformWriter->Update();
    std::cout << "Wrote ITK transform to text file: "
              << TransformFilename.c_str() << std::endl;
    }
}

template <class TransformType>
void WriteTransformWithBulk(const typename TransformType::Pointer & MyTransform,
  const std::string & TransformFilename)
{
  /*
   *  Convert the transform to the appropriate assumptions and write it out as requested.
   */
    {
    typedef itk::TransformFileWriter TransformWriterType;
    TransformWriterType::Pointer transformWriter =  TransformWriterType::New();
    transformWriter->SetFileName( TransformFilename.c_str() );
    transformWriter->AddTransform( MyTransform->GetBulkTransform() );
    transformWriter->AddTransform( MyTransform );
    transformWriter->Update();
    std::cout << "Appended ITK transform to text file: "
              << TransformFilename.c_str() << std::endl;
    }
}

#endif  //__BRAINSFITHELPER__
