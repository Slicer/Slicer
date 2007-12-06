#include "vtkRigidRegistrator.h"
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

vtkCxxRevisionMacro(vtkRigidRegistrator, "$Revision: 0.0 $");
vtkStandardNewMacro(vtkRigidRegistrator);

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
  typedef CommandIterationUpdate   Self;
  typedef itk::Command             Superclass;
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
      std::cerr << "   " << std::setw(7) << std::right << std::setfill('.')
                << optimizer->GetCurrentIteration();
      std::cerr << std::setw(20) << std::right << std::setfill('.')
                << optimizer->GetValue();
      std::cerr << std::setw(17) << std::right << std::setfill('.')
                << optimizer->GetCurrentStepLength();
      std::cerr << std::endl;
  }
};

//----------------------------------------------------------------------------
vtkRigidRegistrator::
vtkRigidRegistrator()
{
  this->FixedImage  = NULL;
  this->MovingImage = NULL;
  this->Transform   = vtkTransform::New();
  this->Transform->Identity();

  this->ImageToImageMetric   = vtkRigidRegistrator::MutualInformation;
  this->MetricComputationSamplingRatio = 1.0;
}

//----------------------------------------------------------------------------
vtkRigidRegistrator::
~vtkRigidRegistrator()
{
  this->SetFixedImage(NULL);
  this->SetMovingImage(NULL);
  this->Transform->Delete();
  this->Transform = NULL;
}

//----------------------------------------------------------------------------
void
vtkRigidRegistrator::
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
vtkRigidRegistrator::
GetStringFromMetricType(MetricType id)
{
  switch (id)
    {
    case (vtkRigidRegistrator::MutualInformation):
      return "MutualInformation";
    case (vtkRigidRegistrator::CrossCorrelation):
      return "CrossCorrelation";
    case (vtkRigidRegistrator::MeanSquaredError):
      return "MeanSquaredError";
    default:
      return "Unknown";
    };
}

//----------------------------------------------------------------------------
const char*
vtkRigidRegistrator::
GetStringFromInterpolationType(InterpolationType id)
{
  switch (id)
    {
    case (vtkRigidRegistrator::NearestNeighbor):
      return "NearestNeighbor";
    case (vtkRigidRegistrator::Linear):
      return "Linear";
    default:
      return "Unknown";
    };
}

//----------------------------------------------------------------------------
const char*
vtkRigidRegistrator::
GetStringFromTransformInitializationType(InitializationType id)
{
  switch (id)
    {
    case (vtkRigidRegistrator::Identity):
      return "Identity";
    case (vtkRigidRegistrator::CentersOfMass):
      return "CentersOfMass";
    case (vtkRigidRegistrator::ImageCenters):
      return "ImageCenters";
    default:
      return "Unknown";
    };
}

//----------------------------------------------------------------------------
template <class TVoxel>
void 
vtkRigidRegistrator::
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
    case vtkRigidRegistrator::MutualInformation:
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
    case vtkRigidRegistrator::CrossCorrelation:
      {
      typedef itk::NormalizedCorrelationImageToImageMetric<
        ITKImageType, ITKImageType>   MetricType;
      typename MetricType::Pointer    metric  = MetricType::New();
      registration->SetMetric(metric);
      std::cerr << "Metric: MSE" << std::endl;

      break;
      }
    case vtkRigidRegistrator::MeanSquaredError:
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
    case vtkRigidRegistrator::NearestNeighbor:
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

    case vtkRigidRegistrator::Linear:
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
  typedef itk::VersorRigid3DTransform< double >        TransformType;
  TransformType::Pointer  transform =                  TransformType::New();
  transform->SetIdentity();

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
  typedef itk::VersorRigid3DTransformOptimizer       OptimizerType;
  OptimizerType::Pointer optimizer =                 OptimizerType::New();
  registration->SetOptimizer(optimizer);

  typedef OptimizerType::ScalesType       OptimizerScalesType;
  OptimizerScalesType optimizerScales(6);
  const double translationScale = 1.0 / 1000.0;

  double               initialStepLength               = 0.1;
  double               relaxationFactor                = 0.6;
  double               minimumStepLength               = 0.001;

  // matrix
  optimizerScales[0]  = 1.0;
  optimizerScales[1]  = 1.0;
  optimizerScales[2]  = 1.0;
  // translation
  optimizerScales[3] = translationScale;
  optimizerScales[4] = translationScale;
  optimizerScales[5] = translationScale;
  
  optimizer->SetScales( optimizerScales );
  optimizer->SetRelaxationFactor( relaxationFactor );
  optimizer->SetMaximumStepLength( initialStepLength );
  optimizer->SetMinimumStepLength( minimumStepLength );
  optimizer->SetNumberOfIterations( this->NumberOfIterations );

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
    std::cerr << "  Starting registration..." << std::endl;
    std::cerr << "   Iteration         Image Match        Step Size" 
              << std::endl;
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

  typename TransformType::MatrixType itkMatrix       = transform->GetMatrix();
  typename TransformType::OutputVectorType itkOffset = transform->GetOffset();
  vtkMatrix4x4* vtkMatrix = vtkMatrix4x4::New();
  vtkMatrix->Identity();
  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      vtkMatrix->SetElement(i, j, itkMatrix(i,j));
    }
    vtkMatrix->SetElement(i, 3, itkOffset[i]);
  }
  
  this->Transform->SetMatrix(vtkMatrix);

  //
  // clean up memory
  //

  fixedImageCaster->Delete();
  movingImageCaster->Delete();
  fixedImageVTKToITKExporter->Delete();
  movingImageVTKToITKExporter->Delete();
  vtkMatrix->Delete();

  this->Modified();
}

//----------------------------------------------------------------------------
template <class TFixedImageVoxel, class TMovingImageVoxel>
void 
vtkRigidRegistrator::
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
vtkRigidRegistrator::
RegisterImagesInternal1()
{
  switch (this->MovingImage->GetScalarType())
    {  
    vtkTemplateMacro((RegisterImagesInternal2<TFixedImageVoxel,VTK_TT>()));
    }
}

//----------------------------------------------------------------------------
void 
vtkRigidRegistrator::
RegisterImages()
{
  switch (this->FixedImage->GetScalarType())
    {  
    vtkTemplateMacro((RegisterImagesInternal1<VTK_TT>()));
    }
}


