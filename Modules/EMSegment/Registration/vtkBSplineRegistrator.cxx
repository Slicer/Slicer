#define ITK_LEAN_AND_MEAN
#include "vtkBSplineRegistrator.h"
#include "vtkObjectFactory.h"

#include "vtkCommand.h"
#include "vtkITKUtility.h"
#include "vtkImageExport.h"
#include "vtkImageCast.h"
#include "vtkTypeTraits.h"

#include "itkImage.h"
#include "itkVTKImageImport.h"

#include "itkLBFGSBOptimizer.h"
#include "itkMultiResolutionImageRegistrationMethod.h"
#include "itkMultiResolutionPyramidImageFilter.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkNormalizedCorrelationImageToImageMetric.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkLinearInterpolateImageFunction.h"

#include "itkRealTimeClock.h"
#include "itkPixelTraits.h"

#include "vtkTransformToGrid.h"
#include "vtkRegistratorTypeTraits.h"
#include "vtkImageChangeInformation.h"
#include "vtkImagePermute.h"
#include "vtkMatrix4x4.h"
#include "itkAffineTransform.h"
#include "vtkITKTransformAdapter.h"

vtkCxxRevisionMacro(vtkBSplineRegistrator, "$Revision: 0.0 $");
vtkStandardNewMacro(vtkBSplineRegistrator);

//
//  The following section of code implements a Command observer
//  that will monitor the evolution of the registration process.
//
#include "itkCommand.h"
#include <iomanip>

template <class TOptimizer>
class CommandIterationUpdate : public itk::Command
{
public:
  typedef  CommandIterationUpdate   Self;
  typedef  itk::Command             Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  itkNewMacro( Self );
protected:
  CommandIterationUpdate() {};
public:
  typedef TOptimizer                 OptimizerType;
  typedef const OptimizerType   *    OptimizerPointer;

  void Execute(itk::Object *caller, const itk::EventObject & event)
  {
    Execute( (const itk::Object *)caller, event);
  }

  // this should be specialized later...
  void Execute(const itk::Object * object, const itk::EventObject & event)
  {
    OptimizerPointer optimizer =
      dynamic_cast< OptimizerPointer >( object );
    if( ! itk::IterationEvent().CheckEvent( &event ) )
      {
      return;
      }
    std::cerr << "       " << std::setw(7) << std::right << std::setfill('.')
              << optimizer->GetCurrentIteration();
    std::cerr << std::setw(20) << std::right << std::setfill('.')
              << optimizer->GetValue();
    std::cerr << std::endl;
  }
};

template <class TRegistration, class TMetric, class TOptimizer>
class CommandStartLevelUpdate : public itk::Command
{
public:
  typedef  CommandStartLevelUpdate   Self;
  typedef  itk::Command              Superclass;
  typedef itk::SmartPointer<Self>    Pointer;
  itkNewMacro( Self );
protected:
  CommandStartLevelUpdate() 
  { 
    this->m_SamplingRatio = 0.33; 
    this->m_NumberOfIterations = 10;
  }
  double m_SamplingRatio;
  int    m_NumberOfIterations;
public:
  typedef TRegistration                 RegistrationType;
  typedef RegistrationType   *          RegistrationPointer;
  typedef TMetric                       MetricType;
  typedef MetricType*                   MetricPointer;
  typedef TOptimizer                    OptimizerType;
  typedef OptimizerType*                OptimizerPointer;

  itkSetMacro(SamplingRatio, double);
  itkSetMacro(NumberOfIterations, int);

  void Execute(const itk::Object *caller, const itk::EventObject & event)
  {
    return;
  }

  // this should be specialized later...
  void Execute(itk::Object * object, const itk::EventObject & event)
  {
    RegistrationPointer registration =
      dynamic_cast< RegistrationPointer >( object );
    if( ! itk::IterationEvent().CheckEvent( &event ) )
      {
      return;
      }
    int level       = registration->GetCurrentLevel();
    int totalLevels = registration->GetNumberOfLevels();
    std::cerr << "   ### Starting registration level: " 
              <<  level << " of " << totalLevels << " ###" << std::endl;
    MetricPointer metric = 
      dynamic_cast<MetricPointer>(registration->GetMetric());
    if (metric != NULL)
      {
      int numVoxels = 
        registration->GetFixedImagePyramid()->GetOutput(level)->
        GetLargestPossibleRegion().GetNumberOfPixels();
      
      double samplingRatio = 
        1.0 -
        registration->GetCurrentLevel() *
        (1.0 - m_SamplingRatio) / (registration->GetNumberOfLevels() - 1.0);

      metric->
        SetNumberOfSpatialSamples(samplingRatio * numVoxels);

      std::cerr << "       Image Size: " << 
        registration->GetFixedImagePyramid()->GetOutput(level)->
        GetLargestPossibleRegion().GetSize() << std::endl;
      std::cerr << "       Number of spatial samples: " 
                << metric->GetNumberOfSpatialSamples() 
                << " (" << floor(100 * samplingRatio) << "%)"
                << std::endl;
      }

    OptimizerPointer optimizer = dynamic_cast<OptimizerPointer>
      (registration->GetOptimizer());
    if (optimizer)
    {
      int numIterations = this->m_NumberOfIterations * 
        (registration->GetNumberOfLevels() - registration->GetCurrentLevel());

      optimizer->SetMaximumNumberOfIterations(numIterations);
      optimizer->SetMaximumNumberOfEvaluations(numIterations);
      std::cerr << "       Max Iterations: " 
                << optimizer->GetMaximumNumberOfIterations() 
                << std::endl;
    }
  }
};

class vtkMyCallback : public vtkCommand
{
public:
  static vtkMyCallback *New() 
  { return new vtkMyCallback; }
  virtual void Execute(vtkObject *caller, unsigned long, void*)
  {
    std::cerr << ".";
  }
};

//----------------------------------------------------------------------------
vtkBSplineRegistrator::
vtkBSplineRegistrator()
{
  this->FixedImage  = NULL;
  this->MovingImage = NULL;

  this->FixedIJKToXYZ  = NULL;
  this->MovingIJKToXYZ = NULL;

  this->BulkTransform = NULL;
  this->Transform   = vtkGridTransform::New();

  this->NumberOfIterations = 0;
  this->IntensityInterpolationType = vtkBSplineRegistrator::Linear;
  
  this->ImageToImageMetric   = vtkBSplineRegistrator::MutualInformation;
  this->MetricComputationSamplingRatio = 1.0;
  this->NumberOfKnotPoints = 5;
}

//----------------------------------------------------------------------------
vtkBSplineRegistrator::
~vtkBSplineRegistrator()
{
  this->SetFixedImage(NULL);
  this->SetMovingImage(NULL);
  this->SetFixedIJKToXYZ(NULL);
  this->SetMovingIJKToXYZ(NULL);
  this->SetBulkTransform(NULL);

  this->Transform->Delete();
  this->Transform = NULL;
}

//----------------------------------------------------------------------------
void
vtkBSplineRegistrator::
PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  this->Transform->PrintSelf(os, indent);
  os << indent << "NumberOfIterations: " << this->NumberOfIterations 
     << std::endl;
  os << indent << "MetricComputationSamplingRatio: " 
     << this->MetricComputationSamplingRatio << std::endl;
  os << indent << "ImageToImageMetric: " 
     << GetStringFromMetricType(this->ImageToImageMetric);
  os << indent << "InterpolationType: " 
     << GetStringFromInterpolationType(this->IntensityInterpolationType)
     << std::endl;
}

//----------------------------------------------------------------------------
const char*
vtkBSplineRegistrator::
GetStringFromMetricType(MetricType id)
{
  switch (id)
    {
    case (vtkBSplineRegistrator::MutualInformation):
      return "MutualInformation";
    case (vtkBSplineRegistrator::CrossCorrelation):
      return "CrossCorrelation";
    case (vtkBSplineRegistrator::MeanSquaredError):
      return "MeanSquaredError";
    default:
      return "Unknown";
    };
}

//----------------------------------------------------------------------------
const char*
vtkBSplineRegistrator::
GetStringFromInterpolationType(InterpolationType id)
{
  switch (id)
    {
    case (vtkBSplineRegistrator::NearestNeighbor):
      return "NearestNeighbor";
    case (vtkBSplineRegistrator::Linear):
      return "Linear";
    case (vtkBSplineRegistrator::Cubic):
      return "Cubic";
    default:
      return "Unknown";
    };
}

//----------------------------------------------------------------------------
void
vtkBSplineRegistrator::
ComputeReorientationInformation(const vtkMatrix4x4* IJKToXYZ,
                                int*    filteredAxesForPermuteFilter,
                                double* originForChangeInformationFilter,
                                double* spacingForChangeInformationFilter)
{
  // origin is easy...
  originForChangeInformationFilter[0] = (*IJKToXYZ)[0][3];
  originForChangeInformationFilter[1] = (*IJKToXYZ)[1][3];
  originForChangeInformationFilter[2] = (*IJKToXYZ)[2][3];

  // figure out spacing and permutation.  Assumes one nonzero entry
  // per row/column of directions matrix.
  for (int c = 0; c < 3; ++c)
    {
    for (int r = 0; r < 3; ++r)
      {
      double t = (*IJKToXYZ)[r][c];
      if (t != 0)
        {
        filteredAxesForPermuteFilter[c]      = r;
        spacingForChangeInformationFilter[r] = t;
        break;
        }
      }
    }
}

//----------------------------------------------------------------------------
template <class TVoxel>
void 
vtkBSplineRegistrator::
RegisterImagesInternal3()
{
  //
  // Deal with orientation.  Permute images and setup origin and
  // spacing so that both images are measured in XYZ basis vectors
  // with only spacing and origin information.  This way ITK will do
  // registration in XYZ coordinates.
  //
  int     filteredAxesForPermuteFilter[3];
  double  originForChangeInformationFilter[3];
  double  spacingForChangeInformationFilter[3];

  // fixed ------
  vtkMatrix4x4* IJKToXYZMatrixFixed = vtkMatrix4x4::New();
  IJKToXYZMatrixFixed->Identity();
  if (this->FixedIJKToXYZ != NULL)
    {
    IJKToXYZMatrixFixed->DeepCopy(this->FixedIJKToXYZ);
    }
  vtkBSplineRegistrator::
    ComputeReorientationInformation(IJKToXYZMatrixFixed,
                                    filteredAxesForPermuteFilter,
                                    originForChangeInformationFilter,
                                    spacingForChangeInformationFilter);

  vtkImagePermute* permuteFixedImage = vtkImagePermute::New();
  vtkImageChangeInformation* changeInformationFixedImage = 
    vtkImageChangeInformation::New();
  
  permuteFixedImage->SetInput(this->FixedImage);
  permuteFixedImage->SetFilteredAxes(filteredAxesForPermuteFilter);

  changeInformationFixedImage->SetInput(permuteFixedImage->GetOutput());
  changeInformationFixedImage->
    SetOutputSpacing(spacingForChangeInformationFilter);
  changeInformationFixedImage->
    SetOutputOrigin(originForChangeInformationFilter);

  // moving ------
  vtkMatrix4x4* IJKToXYZMatrixMoving = vtkMatrix4x4::New();
  IJKToXYZMatrixMoving->Identity();
  if (this->MovingIJKToXYZ != NULL)
    {
    IJKToXYZMatrixMoving->DeepCopy(this->MovingIJKToXYZ);
    }
  vtkBSplineRegistrator::
    ComputeReorientationInformation(IJKToXYZMatrixMoving,
                                    filteredAxesForPermuteFilter,
                                    originForChangeInformationFilter,
                                    spacingForChangeInformationFilter);

  vtkImagePermute* permuteMovingImage = vtkImagePermute::New();
  vtkImageChangeInformation* changeInformationMovingImage = 
    vtkImageChangeInformation::New();
  
  permuteMovingImage->SetInput(this->MovingImage);
  permuteMovingImage->SetFilteredAxes(filteredAxesForPermuteFilter);

  changeInformationMovingImage->SetInput(permuteMovingImage->GetOutput());
  changeInformationMovingImage->
    SetOutputSpacing(spacingForChangeInformationFilter);
  changeInformationMovingImage->
    SetOutputOrigin(originForChangeInformationFilter);

  //
  // create vtk --> itk pipelines
  //

  typedef itk::Image<TVoxel, 3>                 ITKImageType;
  typedef itk::VTKImageImport<ITKImageType>     ImageImportType;

  //
  // fixed image ------
  vtkImageCast* fixedImageCaster              = vtkImageCast::New();
  fixedImageCaster->SetInput(changeInformationFixedImage->GetOutput());
  fixedImageCaster->
    SetOutputScalarType(vtkTypeTraits<TVoxel>::VTKTypeID());
  vtkImageExport* fixedImageVTKToITKExporter  = vtkImageExport::New();
  fixedImageVTKToITKExporter->SetInput(fixedImageCaster->GetOutput());

  typename ImageImportType::Pointer fixedImageITKImporter = 
    ImageImportType::New();
  ConnectPipelines(fixedImageVTKToITKExporter, fixedImageITKImporter);
  fixedImageITKImporter->Update();

  //
  // moving image ------
  vtkImageCast*   movingImageCaster           = vtkImageCast::New();
  movingImageCaster->SetInput(changeInformationMovingImage->GetOutput());
  movingImageCaster->
    SetOutputScalarType(vtkTypeTraits<TVoxel>::VTKTypeID());
  vtkImageExport* movingImageVTKToITKExporter = vtkImageExport::New();
  movingImageVTKToITKExporter->SetInput(movingImageCaster->GetOutput());

  typename ImageImportType::Pointer movingImageITKImporter = 
    ImageImportType::New();
  ConnectPipelines(movingImageVTKToITKExporter, movingImageITKImporter);
  movingImageITKImporter->Update();

  //
  // set up multires registration
  typedef typename itk::MultiResolutionPyramidImageFilter
    <ITKImageType, ITKImageType>   ImagePyramidType;
  typename ImagePyramidType::Pointer fixedImagePyramid = 
    ImagePyramidType::New();
  typename ImagePyramidType::Pointer movingImagePyramid = 
    ImagePyramidType::New();
  typedef typename itk::MultiResolutionImageRegistrationMethod
    <ITKImageType, ITKImageType> MultiResolutionRegistrationType;
  typename MultiResolutionRegistrationType::Pointer multiResRegistration = 
    MultiResolutionRegistrationType::New();

  // set images
  multiResRegistration->SetFixedImage(fixedImageITKImporter->GetOutput());
  multiResRegistration->SetMovingImage(movingImageITKImporter->GetOutput());
  multiResRegistration->
    SetFixedImageRegion(fixedImageITKImporter->GetOutput()->
                        GetLargestPossibleRegion());

  //
  // use heuristics to determine pyramid schedule
  std::cerr << "   Determining registration pyramid schedule...";
  typename ITKImageType::SizeType fixedImageSize = 
    fixedImageITKImporter->GetOutput()->GetBufferedRegion().GetSize();  
  
  // don't downsample too much or too little
  // int minDesiredSide = 64;
  int maxDesiredSide = 128;

  int initialNumberOfLevels = 3;
  int initialShrinkFactor = 
    static_cast<int>(std::pow(static_cast<double>(2.0),
                              static_cast<double>(initialNumberOfLevels)));

  fixedImagePyramid->SetNumberOfLevels(initialNumberOfLevels);
  fixedImagePyramid->SetStartingShrinkFactors(initialShrinkFactor);
  movingImagePyramid->SetNumberOfLevels(initialNumberOfLevels);
  movingImagePyramid->SetStartingShrinkFactors(initialShrinkFactor);

  typename ImagePyramidType::ScheduleType initialSchedule = 
    fixedImagePyramid->GetSchedule();

  // don't upsamaple too much
  // use a fudge factor of 2, it is ok if it is close
  unsigned long maxVoxels = maxDesiredSide*maxDesiredSide*maxDesiredSide*2;
  std::vector<bool> deleteLevel(initialNumberOfLevels, false);
  int  finalNumberOfLevels   = 0;
  for (int i = 0; i < initialNumberOfLevels; ++i)
    {
    unsigned long voxels = 1;
    for (unsigned int j = 0; j < 3; ++j)
      {
      voxels *= static_cast<unsigned long>
        (static_cast<float>(fixedImageSize[j]) / initialSchedule[i][j]);
      }
     if (voxels > maxVoxels)
       {
       // too many voxels
       deleteLevel[i] = true;
       }
     else
       {
       // its ok, use it
       ++finalNumberOfLevels;
       }
    }

  // fill in the final schedule
  typename ImagePyramidType::ScheduleType finalSchedule;
  finalSchedule.SetSize(finalNumberOfLevels, 3);
  int finalScheduleIndex = 0;
  for (int i = 0; i < initialNumberOfLevels; ++i)
    {
    if (!deleteLevel[i])
      {
      for (int j = 0; j < 3; ++j)
        {
        finalSchedule[finalScheduleIndex][j] = initialSchedule[i][j];
        }
      ++finalScheduleIndex;
      }
    }

  fixedImagePyramid->SetNumberOfLevels(finalNumberOfLevels);
  movingImagePyramid->SetNumberOfLevels(finalNumberOfLevels);
  fixedImagePyramid->SetSchedule(finalSchedule);
  movingImagePyramid->SetSchedule(finalSchedule);
  
  multiResRegistration->SetFixedImagePyramid(fixedImagePyramid);
  multiResRegistration->SetMovingImagePyramid(movingImagePyramid);
  multiResRegistration->SetNumberOfLevels(finalNumberOfLevels);

  std::cerr << "DONE" << std::endl;
      
  //
  // set up metric
  switch (this->ImageToImageMetric)
    {
    case vtkBSplineRegistrator::MutualInformation:
      {
      typedef itk::MattesMutualInformationImageToImageMetric<
      ITKImageType, ITKImageType>   MetricType;
      typename MetricType::Pointer    metric  = MetricType::New();
      metric->ReinitializeSeed(0);
      metric->SetNumberOfHistogramBins(50);
      metric->
        SetNumberOfSpatialSamples
        (static_cast<unsigned int>(multiResRegistration->GetFixedImage()->
                                   GetLargestPossibleRegion().
                                   GetNumberOfPixels() *
                                   this->MetricComputationSamplingRatio));
      multiResRegistration->SetMetric(metric);
      std::cerr << "   Metric: MMI" << std::endl;
      std::cerr << "   Sampling Ratio: " 
                << this->MetricComputationSamplingRatio << std::endl;
      break;
      }
    case vtkBSplineRegistrator::CrossCorrelation:
      {
      typedef itk::NormalizedCorrelationImageToImageMetric<
        ITKImageType, ITKImageType>   MetricType;
      typename MetricType::Pointer    metric  = MetricType::New();
      multiResRegistration->SetMetric(metric);
      std::cerr << "   Metric: NCC" << std::endl;
      std::cerr << "   Sampling Ratio: 1 (NOT IMPLEMENTED FOR NCC) " 
                << this->MetricComputationSamplingRatio << std::endl;
      break;
      }
    case vtkBSplineRegistrator::MeanSquaredError:
      {
      typedef itk::MeanSquaresImageToImageMetric<
        ITKImageType, ITKImageType>   MetricType;
      typename MetricType::Pointer    metric  = MetricType::New();
      multiResRegistration->SetMetric(metric);
      std::cerr << "   Metric: MSE" << std::endl;

      break;
      }
    default:
      vtkErrorMacro("Unknown metric type: " << this->ImageToImageMetric);
      return;
    };
    
  //
  // set up interpolator
  switch (this->IntensityInterpolationType)
    {
    case vtkBSplineRegistrator::NearestNeighbor:
      {
      typedef itk::NearestNeighborInterpolateImageFunction<
        ITKImageType,
        double >    InterpolatorType;
      typename InterpolatorType::Pointer interpolator  = 
        InterpolatorType::New();
      multiResRegistration->SetInterpolator(interpolator);
      std::cerr << "   Interpolation: Nearest neighbor" << std::endl;
      }
      break;

    case vtkBSplineRegistrator::Linear:
      {
      typedef itk::LinearInterpolateImageFunction<
        ITKImageType,
        double          >    InterpolatorType;
      typename InterpolatorType::Pointer   
        interpolator  = InterpolatorType::New();
      multiResRegistration->SetInterpolator(interpolator);
      std::cerr << "   Interpolation: Linear" << std::endl;
      }
      break;
    default:
      vtkErrorMacro(<< "Unknown interpolation type: " 
                    << this->IntensityInterpolationType);
      return;
    };

  //
  // setup transform
  typedef itk::BSplineDeformableTransform<double, 3, 3> TransformType ;
  TransformType::Pointer bSplineTransform = TransformType::New();

  // specify bulk transform if available
  if (this->BulkTransform)
    {
    typedef itk::AffineTransform<double, 3>    ITKBulkTransformType;
    ITKBulkTransformType::MatrixType       itkMatrix;
    ITKBulkTransformType::OutputVectorType itkOffset;

    vtkMatrix4x4* vtkMatrix = this->BulkTransform->GetMatrix();
    for (int i = 0; i < 3; ++i)
      {
      for (int j = 0; j < 3; ++j)
        {
        itkMatrix(i,j) = (*vtkMatrix)[i][j];
        }
      itkOffset[i] = (*vtkMatrix)[i][3];
      }    

    ITKBulkTransformType::Pointer itkBulkTransform = 
      ITKBulkTransformType::New();
    itkBulkTransform->SetMatrix(itkMatrix);
    itkBulkTransform->SetOffset(itkOffset);

    bSplineTransform->SetBulkTransform(itkBulkTransform);
    }

  TransformType::RegionType            bSplineRegion;
  TransformType::RegionType::SizeType  gridSizeOnImage;
  TransformType::RegionType::SizeType  gridBorderSize;
  TransformType::RegionType::SizeType  totalGridSize;

  gridSizeOnImage.Fill(this->NumberOfKnotPoints);
  gridBorderSize.Fill(3);
  totalGridSize = gridSizeOnImage + gridBorderSize;
  bSplineRegion.SetSize(totalGridSize);

  TransformType::SpacingType spacing = 
    fixedImageITKImporter->GetOutput()->GetSpacing();
  TransformType::OriginType  origin  = 
    fixedImageITKImporter->GetOutput()->GetOrigin();

  for (unsigned int r = 0; r < 3; ++r)
    {
    spacing[r] *= floor(static_cast<double>(fixedImageSize[r]-1) /
                        static_cast<double>(gridSizeOnImage[r]-1));
    origin[r]  -= spacing[r];
    }

  bSplineTransform->SetGridSpacing(spacing);
  bSplineTransform->SetGridOrigin(origin);
  bSplineTransform->SetGridRegion(bSplineRegion);
  unsigned int numberOfParameters = bSplineTransform->GetNumberOfParameters();

  std::cerr << "   BSpline spacing: " << spacing << std::endl;
  std::cerr << "   BSpline origin: "  << origin  << std::endl;
  std::cerr << "   BSpline region: "  << bSplineRegion  << std::endl;
  std::cerr << "   BSpline knots on image:       "     
            << gridSizeOnImage << std::endl;
  std::cerr << "   BSpline knots total:          "     
            << totalGridSize << std::endl;
  std::cerr << "   BSpline number of parameters: " << numberOfParameters 
            << std::endl;

  TransformType::ParametersType params(numberOfParameters);
  params.Fill(0.0);
  bSplineTransform->SetParameters(params);
  multiResRegistration->SetTransform(bSplineTransform);

  multiResRegistration->
    SetInitialTransformParameters(bSplineTransform->GetParameters());

  //
  // setup optomizer
  typedef itk::LBFGSBOptimizer                       OptimizerType;
  OptimizerType::Pointer optimizer =                 OptimizerType::New();
  multiResRegistration->SetOptimizer(optimizer);
  
  OptimizerType::BoundSelectionType boundSelect(numberOfParameters);
  OptimizerType::BoundValueType     upperBound(numberOfParameters);
  OptimizerType::BoundValueType     lowerBound(numberOfParameters);
  boundSelect.Fill(0);
  upperBound.Fill(0.0);
  lowerBound.Fill(0.0);
  optimizer->SetBoundSelection(boundSelect);
  optimizer->SetUpperBound(upperBound);
  optimizer->SetLowerBound(lowerBound);
  
  optimizer->SetCostFunctionConvergenceFactor(1e+4);
  optimizer->SetProjectedGradientTolerance(1e-10);
  optimizer->SetMaximumNumberOfIterations(this->NumberOfIterations);
  optimizer->SetMaximumNumberOfEvaluations(this->NumberOfIterations);
  optimizer->SetMaximumNumberOfCorrections(10);
  
  std::cerr << "   Optimization: LBFGSB" << std::endl;
  std::cerr << "   Max iterations: " << this->NumberOfIterations << std::endl;
  
  //
  // set up command observer
  CommandIterationUpdate<OptimizerType>::Pointer observer =
    CommandIterationUpdate<OptimizerType>::New();
  optimizer->AddObserver( itk::IterationEvent(), observer );

  typename CommandStartLevelUpdate<MultiResolutionRegistrationType, itk::MattesMutualInformationImageToImageMetric<ITKImageType, ITKImageType>, OptimizerType >::Pointer 
    startLevelCommand = 
    CommandStartLevelUpdate
    <MultiResolutionRegistrationType, 
    itk::MattesMutualInformationImageToImageMetric<ITKImageType, ITKImageType>,
    OptimizerType>::New();
  startLevelCommand->SetSamplingRatio(this->MetricComputationSamplingRatio);
  startLevelCommand->SetNumberOfIterations(this->NumberOfIterations);
  multiResRegistration->AddObserver(itk::IterationEvent(), startLevelCommand);

  //
  // everything should be set up, run the registration
  //

  try 
    {
    itk::RealTimeClock::Pointer clock = itk::RealTimeClock::New();
    std::cerr << "   Iteration         Image Match" 
              << std::endl;
    double timeStart = clock->GetTimeStamp();

    multiResRegistration->StartRegistration();
    
    double timeStop = clock->GetTimeStamp();
    double timeLength = (timeStop - timeStart);
    std::cerr << "  DONE, time = " << timeLength << std::endl;
    }
  catch( itk::ExceptionObject & err )
    {
    std::cerr << err << std::endl;
    throw;
    }

  //
  // copy transform from itk back to this vtk class
  //

  std::cerr << "  Copy BSpline to Grid..";
  vtkITKTransformAdapter* itkTransformWrapper = vtkITKTransformAdapter::New();
  itkTransformWrapper->SetITKTransform(bSplineTransform);

  // allow transform to be sampled at less than every voxel
  double gridSamplingFactor = 2.0;
  double transformSpacing[3];
  int    transformExtent[6];
  changeInformationFixedImage->GetOutput()->GetSpacing(transformSpacing);  
  changeInformationFixedImage->GetOutput()->GetExtent(transformExtent);  
  for (int i = 0; i < 3; ++i)
    {
    transformSpacing[i] *= gridSamplingFactor;
    transformExtent[i*2+1] = 
      static_cast<int>(std::ceil(transformExtent[i*2+1] / gridSamplingFactor));
    }

  vtkTransformToGrid* transformGenerator = vtkTransformToGrid::New();
  transformGenerator->SetInput(itkTransformWrapper);
  // wait on change in vtk to support floats here
  //transformGenerator->SetGridScalarType(VTK_FLOAT);
  transformGenerator->SetGridExtent(transformExtent);
  transformGenerator->SetGridOrigin(changeInformationFixedImage->GetOutput()->GetOrigin());
  transformGenerator->SetGridSpacing(transformSpacing);

  vtkMyCallback *mob = vtkMyCallback::New();
  transformGenerator->AddObserver(vtkCommand::ProgressEvent, mob);
  mob->Delete();
  transformGenerator->Update();
  this->Transform->SetDisplacementGrid(transformGenerator->GetOutput());
  std::cerr << "DONE" << std::endl;

  //
  // clean up memory
  //

  fixedImageCaster->Delete();
  movingImageCaster->Delete();
  fixedImageVTKToITKExporter->Delete();
  movingImageVTKToITKExporter->Delete();
  changeInformationFixedImage->Delete();
  changeInformationMovingImage->Delete();
  permuteFixedImage->Delete();
  permuteMovingImage->Delete();
  IJKToXYZMatrixFixed->Delete();
  IJKToXYZMatrixMoving->Delete();
  transformGenerator->Delete();
  itkTransformWrapper->Delete();

  this->Modified();
}

//----------------------------------------------------------------------------
template <class TFixedImageVoxel, class TMovingImageVoxel>
void 
vtkBSplineRegistrator::
RegisterImagesInternal2()
{
  //
  // First, find the smallest voxel type that can represent both fixed
  // and moving voxel type.  Then, convert that type to one that we
  // have instantiated (currently short, unsigned short, float, and
  // double) in order to reduce code bloat.
  typedef itk::JoinTraits<TFixedImageVoxel, TMovingImageVoxel> TraitsType;
  typedef typename TraitsType::ValueType             CommonImageVoxelType;
  typedef 
    typename RegistrationVoxelTypeTraits<CommonImageVoxelType>::
    RegistrationVoxelType 
    RegistrationVoxelType;
  this->RegisterImagesInternal3<RegistrationVoxelType>();
}

//----------------------------------------------------------------------------
template <class TFixedImageVoxel>
void 
vtkBSplineRegistrator::
RegisterImagesInternal1()
{
  switch (this->MovingImage->GetScalarType())
    {  
    vtkTemplateMacro((RegisterImagesInternal2<TFixedImageVoxel,VTK_TT>()));
    }
}

//----------------------------------------------------------------------------
void 
vtkBSplineRegistrator::
RegisterImages()
{
  switch (this->FixedImage->GetScalarType())
    {  
    vtkTemplateMacro((RegisterImagesInternal1<VTK_TT>()));
    }
}


