#include "vtkBSplineRegistrator.h"
#include "vtkObjectFactory.h"
#include "itkImage.h"
#include "itkVTKImageImport.h"
#include "vtkITKUtility.h"
#include "vtkImageExport.h"
#include "itkImageRegistrationMethod.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkNormalizedCorrelationImageToImageMetric.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkRealTimeClock.h"
//#include "itkTemporaryTraitsForLongLong.h"
#include "itkPixelTraits.h"
#include "vtkImageCast.h"
#include "vtkTypeTraits.h"
#include "itkVersorRigid3DTransform.h"
#include "itkCenteredVersorTransformInitializer.h"
#include "itkVersorRigid3DTransformOptimizer.h"
#include "vtkRegistratorTypeTraits.h"

vtkCxxRevisionMacro(vtkBSplineRegistrator, "$Revision: 0.0 $");
vtkStandardNewMacro(vtkBSplineRegistrator);

//
//  The following section of code implements a Command observer
//  that will monitor the evolution of the registration process.
//
#include "itkCommand.h"
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
      std::cout << optimizer->GetCurrentIteration() << "   ";
      std::cout << optimizer->GetValue() << "   ";
      std::cout << optimizer->GetCurrentStepLength() << std::endl;
  }
};

//----------------------------------------------------------------------------
vtkBSplineRegistrator::
vtkBSplineRegistrator()
{
  this->FixedImage  = NULL;
  this->MovingImage = NULL;
  this->Transform   = vtkGridTransform::New();
  this->Transform->Identity();

  this->ImageToImageMetric   = vtkBSplineRegistrator::MutualInformation;
  this->MetricComputationSamplingRatio = 1.0;
}

//----------------------------------------------------------------------------
vtkBSplineRegistrator::
~vtkBSplineRegistrator()
{
  this->SetFixedImage(NULL);
  this->SetMovingImage(NULL);
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
     << GetStringFromInterpolationType(this->IntensityInterpolationType);
  os << indent << "InitializationType: " 
     << GetStringFromTransformInitializationType(this->TransformInitializationType)
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
    default:
      return "Unknown";
    };
}

//----------------------------------------------------------------------------
const char*
vtkBSplineRegistrator::
GetStringFromTransformInitializationType(InitializationType id)
{
  switch (id)
    {
    case (vtkBSplineRegistrator::Identity):
      return "Identity";
    case (vtkBSplineRegistrator::CentersOfMass):
      return "CentersOfMass";
    case (vtkBSplineRegistrator::ImageCenters):
      return "ImageCenters";
    default:
      return "Unknown";
    };
}

//----------------------------------------------------------------------------
template <class TVoxel>
void 
vtkBSplineRegistrator::
RegisterImagesInternal3()
{
  //
  // create vtk --> itk pipelines
  //

  typedef itk::Image<TVoxel, 3>                 ITKImageType;
  typedef itk::VTKImageImport<ITKImageType>     ImageImportType;

  //
  // fixed image 
  vtkImageCast* fixedImageCaster              = vtkImageCast::New();
  fixedImageCaster->SetInput(this->FixedImage);
  fixedImageCaster->
    SetOutputScalarType(vtkTypeTraits<TVoxel>::VTKTypeID());
  vtkImageExport* fixedImageVTKToITKExporter  = vtkImageExport::New();
  fixedImageVTKToITKExporter->SetInput(fixedImageCaster->GetOutput());

  typename ImageImportType::Pointer fixedImageITKImporter = 
    ImageImportType::New();
  ConnectPipelines(fixedImageVTKToITKExporter, fixedImageITKImporter);
  fixedImageITKImporter->Update();

  //
  // moving image
  vtkImageCast*   movingImageCaster           = vtkImageCast::New();
  movingImageCaster->SetInput(this->MovingImage);
  movingImageCaster->
    SetOutputScalarType(vtkTypeTraits<TVoxel>::VTKTypeID());
  vtkImageExport* movingImageVTKToITKExporter = vtkImageExport::New();
  movingImageVTKToITKExporter->SetInput(movingImageCaster->GetOutput());

  typename ImageImportType::Pointer movingImageITKImporter = 
    ImageImportType::New();
  ConnectPipelines(movingImageVTKToITKExporter, movingImageITKImporter);
  movingImageITKImporter->Update();

  //
  // set up registration class
  //

  //
  // plug in fixed and moving images
  typedef itk::ImageRegistrationMethod<ITKImageType, ITKImageType> 
    RegistrationType;
  typename RegistrationType::Pointer registration = RegistrationType::New();
  registration->SetFixedImage(fixedImageITKImporter->GetOutput());
  registration->SetMovingImage(movingImageITKImporter->GetOutput());
  registration->
    SetFixedImageRegion(fixedImageITKImporter->GetOutput()->
                        GetBufferedRegion());

  //
  // set up metric
  switch (this->ImageToImageMetric)
    {
    case vtkBSplineRegistrator::MutualInformation:
      {
      typedef itk::MattesMutualInformationImageToImageMetric<
        ITKImageType, ITKImageType>   MetricType;
      typename MetricType::Pointer    metric  = MetricType::New();
      metric->SetNumberOfHistogramBins(50);
      metric->
        SetNumberOfSpatialSamples
        (static_cast<unsigned int>(registration->GetFixedImage()->
                                   GetLargestPossibleRegion().
                                   GetNumberOfPixels() *
                                   this->MetricComputationSamplingRatio));
      registration->SetMetric(metric);
      std::cerr << "Metric: MMI" << std::endl;

      break;
      }
    case vtkBSplineRegistrator::CrossCorrelation:
      {
      typedef itk::NormalizedCorrelationImageToImageMetric<
        ITKImageType, ITKImageType>   MetricType;
      typename MetricType::Pointer    metric  = MetricType::New();
      registration->SetMetric(metric);
      std::cerr << "Metric: MSE" << std::endl;

      break;
      }
    case vtkBSplineRegistrator::MeanSquaredError:
      {
      typedef itk::MeanSquaresImageToImageMetric<
        ITKImageType, ITKImageType>   MetricType;
      typename MetricType::Pointer    metric  = MetricType::New();
      registration->SetMetric(metric);
      std::cerr << "Metric: MSE" << std::endl;

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
        double          >    InterpolatorType;
      typename InterpolatorType::Pointer interpolator  = 
        InterpolatorType::New();
      registration->SetInterpolator(interpolator);
      std::cerr << "Interpolation: Nearest neighbor" << std::endl;
      }
      break;

    case vtkBSplineRegistrator::Linear:
      {
      typedef itk::LinearInterpolateImageFunction<
        ITKImageType,
        double          >    InterpolatorType;
      typename InterpolatorType::Pointer   
        interpolator  = InterpolatorType::New();
      registration->SetInterpolator(interpolator);
      std::cerr << "Interpolation: Linear" << std::endl;
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
  TransformType::Pointer               bSplineTransform = TransformType::New();
  TransformType::RegionType            bSplineRegion;
  TransformType::RegionType::SizeType  gridSizeOnImage;
  TransformType::RegionType::SizeType  gridBorderSize;
  TransformType::RegionType::SizeType  totalGridSize;

  gridSizeOnImage[0] = numberOfKnots[0];
  gridSizeOnImage[1] = numberOfKnots[1];
  gridSizeOnImage[2] = numberOfKnots[2];
  gridBorderSize.Fill(3);
  totalGridSize = gridSizeOnImage + gridBorderSize;
  bSplineRegion.SetSize(totalGridSize);

  TransformType::SpacingType spacing = 
    fixedImageReader->GetOutput()->GetSpacing();
  TransformType::OriginType  origin  = 
    fixedImageReader->GetOutput()->GetOrigin();
  
  FixedImageType::SizeType fixedImageSize = 
    fixedImageReader->GetOutput()->GetLargestPossibleRegion().GetSize();

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

  std::cerr << "BSpline spacing: " << spacing << std::endl;
  std::cerr << "BSpline origin: "  << origin  << std::endl;
  std::cerr << "BSpline region: "  << bSplineRegion  << std::endl;
  std::cerr << "BSpline number of parameters: " << numberOfParameters 
            << std::endl;

  TransformType::ParametersType params(numberOfParameters);
  params.Fill(0.0);
  bSplineTransform->SetParameters(params);
  registration->SetTransform(bSplineTransform);

  registration->
    SetInitialTransformParameters(bSplineTransform->GetParameters());

  typedef
    itk::CenteredVersorTransformInitializer<ITKImageType, ITKImageType>
    TransformInitializerType;

  typename TransformInitializerType::Pointer transformInitializer = 
    TransformInitializerType::New();
  transformInitializer->SetTransform(transform);
  transformInitializer->SetFixedImage(fixedImageITKImporter->GetOutput());
  transformInitializer->SetMovingImage(movingImageITKImporter->GetOutput());
  transformInitializer->InitializeTransform();

  if (this->TransformInitializationType == CentersOfMass)
  {
    transformInitializer->MomentsOn();
    std::cerr << "Initialization: Moments" << std::endl;
  }
  else if (this->TransformInitializationType == ImageCenters)
  {
    transformInitializer->GeometryOn();
    std::cerr << "Initialization: Image centers..." << std::endl;
  }
  transformInitializer->InitializeTransform();

  registration->SetTransform(transform);
  registration->SetInitialTransformParameters(transform->GetParameters());

  std::cerr << "After Initializtation: " << std::endl;
  transform->Print(std::cerr, 0);

  //
  // setup optomizer
  typedef itk::LBFGSBOptimizer                       OptimizerType;
  OptimizerType::Pointer optimizer =                 OptimizerType::New();
  registration->SetOptimizer(optimizer);
  
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
  optimizer->SetMaximumNumberOfIterations(numberOfIterations);
  optimizer->SetMaximumNumberOfEvaluations(numberOfIterations);
  optimizer->SetMaximumNumberOfCorrections(10);
  
  std::cerr << "Optimization: LBFGSB" << std::endl;
  
  //
  // set up command observer
  CommandIterationUpdate<OptimizerType>::Pointer observer =
    CommandIterationUpdate<OptimizerType>::New();
  optimizer->AddObserver( itk::IterationEvent(), observer );

  //
  // everything should be set up, run the registration
  //

  try 
    {
    itk::RealTimeClock::Pointer clock = itk::RealTimeClock::New();
    std::cerr << "Starting registration..." << std::endl;
    double timeStart = clock->GetTimeStamp();
    
    registration->StartRegistration();
    
    double timeStop = clock->GetTimeStamp();
    double timeLength = (timeStop - timeStart);
    std::cerr << "DONE, time = " << timeLength << std::endl;
    }
  catch( itk::ExceptionObject & err )
    {
    std::cerr << err << std::endl;
    throw;
    }

  std::cerr << "After Registration: " << std::endl;
  transform->Print(std::cerr, 0);

  //
  // copy transform from itk back to this vtk class
  //
  // !!!

  //
  // clean up memory
  //

  fixedImageCaster->Delete();
  movingImageCaster->Delete();
  fixedImageVTKToITKExporter->Delete();
  movingImageVTKToITKExporter->Delete();

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
  // and moving voxel type.  The, convert that type to one that we
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


