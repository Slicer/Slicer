#include "vtkBSplineRegistrator.h"
#include "vtkObjectFactory.h"
#include "itkImage.h"
#include "itkVTKImageImport.h"
#include "vtkITKUtility.h"
#include "vtkImageExport.h"
#include "itkLBFGSBOptimizer.h"
#include "itkImageRegistrationMethod.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkNormalizedCorrelationImageToImageMetric.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkRealTimeClock.h"
#include "itkPixelTraits.h"
#include "vtkImageCast.h"
#include "vtkTypeTraits.h"
#include "itkVersorRigid3DTransform.h"
#include "itkCenteredVersorTransformInitializer.h"
#include "itkVersorRigid3DTransformOptimizer.h"
#include "vtkRegistratorTypeTraits.h"
#include "vtkImageChangeInformation.h"
#include "vtkImagePermute.h"
#include "vtkMatrix4x4.h"

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
    std::cerr << "   " << std::setw(7) << std::right << std::setfill('.')
              << optimizer->GetCurrentIteration();
    std::cerr << std::setw(20) << std::right << std::setfill('.')
              << optimizer->GetValue();
    std::cerr << std::endl;
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

  this->Transform   = vtkGridTransform::New();

  this->ImageToImageMetric   = vtkBSplineRegistrator::MutualInformation;
  this->MetricComputationSamplingRatio = 1.0;
}

//----------------------------------------------------------------------------
vtkBSplineRegistrator::
~vtkBSplineRegistrator()
{
  this->SetFixedImage(NULL);
  this->SetMovingImage(NULL);
  this->SetFixedIJKToXYZ(NULL);
  this->SetMovingIJKToXYZ(NULL);
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
      std::cerr << "Metric: Normalized Cross Correlation" << std::endl;

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

  // for now just make up a number of knots!!!
  TransformType::RegionType::SizeType  numberOfKnots;  
  numberOfKnots.Fill(20);

  gridSizeOnImage[0] = numberOfKnots[0];
  gridSizeOnImage[1] = numberOfKnots[1];
  gridSizeOnImage[2] = numberOfKnots[2];
  gridBorderSize.Fill(3);
  totalGridSize = gridSizeOnImage + gridBorderSize;
  bSplineRegion.SetSize(totalGridSize);

  TransformType::SpacingType spacing = 
    fixedImageITKImporter->GetOutput()->GetSpacing();
  TransformType::OriginType  origin  = 
    fixedImageITKImporter->GetOutput()->GetOrigin();
  
  typename ITKImageType::SizeType fixedImageSize = 
    fixedImageITKImporter->GetOutput()->GetLargestPossibleRegion().GetSize();

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
  optimizer->SetMaximumNumberOfIterations(this->NumberOfIterations);
  optimizer->SetMaximumNumberOfEvaluations(this->NumberOfIterations);
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
  //transform->Print(std::cerr, 0);

  //
  // copy transform from itk back to this vtk class
  //
  // !!!

  // resize the grid transform and set it up so it can be applied in
  // vtk as an RAS space transform: needs origin and spacing of fixed
  // image.  Needs to be natually RAS oriented.
  vtkImageData* displacementGrid = vtkImageData::New();
  displacementGrid->CopyStructure(changeInformationFixedImage->GetOutput());
  displacementGrid->SetNumberOfScalarComponents(3); // x, y, z
  displacementGrid->SetScalarTypeToFloat();

  vtkIdType numVoxels = displacementGrid->GetNumberOfPoints();
  TransformType::InputPointType inPtRAS;
  TransformType::OutputPointType outPtRAS;
  double inPtVTK[3];
  float* outDataPtr = 
    static_cast<float*>(displacementGrid->GetScalarPointer());
  for (vtkIdType i = 0; i < numVoxels; ++i)
    {
    // index to RAS
    displacementGrid->GetPoint(i, inPtVTK);    
    inPtRAS[0] = inPtVTK[0];
    inPtRAS[1] = inPtVTK[1];
    inPtRAS[2] = inPtVTK[2];

    // what was the transform...
    outPtRAS = bSplineTransform->TransformPoint(inPtRAS);

    // save result in grid xform
    *outDataPtr++ = outPtRAS[0];
    *outDataPtr++ = outPtRAS[1];
    *outDataPtr++ = outPtRAS[2];
    }

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


