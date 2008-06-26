/*=========================================================================

  Program:   Registration stand-alone
  Module:    $RCSfile: $
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

=========================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>

#include "RigidRegistrationCLP.h"
#include "itkPluginUtilities.h"
#include <itkCommand.h>
#include <itkImage.h>
#include <itkContinuousIndex.h>
#include <itkOrientedImage.h>
#include <itkOrientImageFilter.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkTransformFileReader.h>
#include <itkTransformFileWriter.h>

#include <itkChangeInformationImageFilter.h>
#include <itkGradientDescentOptimizer.h>
#include <itkImageFileWriter.h>
#include <itkImageRegistrationMethod.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkMattesMutualInformationImageToImageMetric.h>
#include <itkQuaternionRigidTransform.h>
#include <itkAffineTransform.h>
#include <itkQuaternionRigidTransformGradientDescentOptimizer.h>
#include <itkResampleImageFilter.h>
#include <itkStdStreamLogOutput.h>
#include <itkLogger.h>

class ScheduleCommand : public itk::Command
{
 public:
  typedef ScheduleCommand Self;
  typedef itk::SmartPointer<Self>  Pointer;
  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Run-time type information (and related methods). */
  itkTypeMacro(ScheduleCommand,itk::Command);

  void SetLearningRates ( std::vector<double> &LearningRates ) {
    m_LearningRates = LearningRates;
  }
  void SetNumberOfIterations ( std::vector<int> &NumberOfIterations ) {
    m_NumberOfIterations = NumberOfIterations;
    this->m_NextChange = NumberOfIterations[0];
  }
  void SetSchedule ( std::vector<int> &NumberOfIterations,
                     std::vector<double> &LearningRates )
  {
    this->SetNumberOfIterations(NumberOfIterations);
    this->SetLearningRates(LearningRates);
  }
  void DoExecute ( itk::GradientDescentOptimizer* optimizer ) 
  {
    if ( m_Schedule < m_NumberOfIterations.size()-1 )
      {
      if ( optimizer->GetCurrentIteration()
           >= this->m_NumberOfIterations[ m_Schedule ])
        {
        m_Schedule++;
        optimizer->SetLearningRate ( this->m_LearningRates[m_Schedule] );
        this->m_NextChange = optimizer->GetCurrentIteration()
          + this->m_NumberOfIterations[m_Schedule];
        std::cout << "Iteration: " << optimizer->GetCurrentIteration()
                  << " LearningRate: " << optimizer->GetLearningRate()
                  << std::endl;
//         std::cout << "<filter-comment>"
//                   << "Iteration: "
//                   << optimizer->GetCurrentIteration() << ", "
//                   << " Switching LearningRate: "
//                   << optimizer->GetLearningRate() << " "
//                   << "</filter-comment>"
//                   << std::endl;
        }
      }
  }
  void Execute ( itk::Object *caller, const itk::EventObject & event )
  {
    itk::GradientDescentOptimizer* obj
      = dynamic_cast<itk::GradientDescentOptimizer*>(caller);
//     std::cout << "<filter-comment>"
//               << "Optimizer Iteration: "
//               << obj->GetCurrentIteration() << ", "
//               << " Metric: "
//               << obj->GetValue() << " "
//               << "</filter-comment>"
//               << std::endl;
    std::cout << "<filter-progress>"
              << obj->GetCurrentIteration()/(float)obj->GetNumberOfIterations()
              << "</filter-progress>" << std::endl;

    if ( obj->GetCurrentIteration() >= this->m_NextChange )
      {
      this->DoExecute ( obj );
      }
  }
  void Execute ( const itk::Object *caller, const itk::EventObject & event )
  {
    itk::GradientDescentOptimizer* obj = (itk::GradientDescentOptimizer*)(caller);
//     std::cout << "<filter-comment>"
//               << "Optimizer Iteration: "
//               << obj->GetCurrentIteration() << ", "
//               << " Metric: "
//               << obj->GetValue() << " "
//               << "</filter-comment>"
//               << std::endl;
    std::cout << "<filter-progress>"
           << obj->GetCurrentIteration()/(float)obj->GetNumberOfIterations()
           << "</filter-progress>" << std::endl;

    if ( obj->GetCurrentIteration() >= this->m_NextChange )
      {
      this->DoExecute ( obj );
      }
  }
  
 protected:
  std::vector<int> m_NumberOfIterations;
  std::vector<double> m_LearningRates;
  unsigned int m_Schedule;
  unsigned int m_NextChange;
  ScheduleCommand()
  {
    m_Schedule = 0;
  }
  ~ScheduleCommand()
  {
  }
};
        
typedef itk::OrientedImage<signed short, 3> Volume;

int main ( int argc, char* argv[] ) 
{
  // Set up a logger
  // 
  itk::OStringStream msg;
  itk::StdStreamLogOutput::Pointer coutput = itk::StdStreamLogOutput::New();
  coutput->SetStream(std::cout);
  itk::Logger::Pointer logger = itk::Logger::New();
  logger->SetName ( "RigidRegistration" );
  logger->AddLogOutput ( coutput );

  // Parse the arguments and Print out the arguments (need to add
  // --echo to the argument list 
  // 
  std::vector<char *> vargs;
  for (int vi=0; vi < argc; ++vi) vargs.push_back(argv[vi]);
  vargs.push_back("--echo");
  
  argc = vargs.size();
  argv = &(vargs[0]);

  PARSE_ARGS;

  //
  //
  int RandomSeed = 1234567;
  double GradientMagnitudeTolerance = 1e-5;

  Volume::Pointer fixed, moving, rfixed, rmoving;
  typedef itk::OrientImageFilter<Volume,Volume> OrientFilterType;
  typedef itk::ChangeInformationImageFilter<Volume> ChangeFilterType;
  typedef itk::ContinuousIndex<double, 3> ContinuousIndexType;
  
  typedef itk::ImageFileReader<Volume> FileReaderType;
  FileReaderType::Pointer FixedReader = FileReaderType::New();
  FileReaderType::Pointer MovingReader = FileReaderType::New();
  FixedReader->SetFileName ( FixedImageFileName.c_str() );
  MovingReader->SetFileName ( MovingImageFileName.c_str() );

  // read in the fixed image
  try
    {
    FixedReader->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    msg.str(""); msg << "Error Reading Fixed image: " << err;
    logger->Write ( itk::LoggerBase::CRITICAL, msg.str() );
    return EXIT_FAILURE;
    }

  // read in the moving image 
  try
    {
    MovingReader->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    msg.str(""); msg << "Error Reading Moving image: " << err;
    logger->Write ( itk::LoggerBase::CRITICAL, msg.str() );
    return EXIT_FAILURE;
    }

  // if an initial transform was specified, read it
  typedef itk::TransformFileReader TransformReaderType;
  TransformReaderType::Pointer initialTransform;

  if (InitialTransform != "")
    {
    initialTransform= TransformReaderType::New();
    initialTransform->SetFileName( InitialTransform );
    try
      {
      initialTransform->Update();
      }
    catch (itk::ExceptionObject &err)
      {
      std::cerr << err << std::endl;
      exit( EXIT_FAILURE );
      }
    }

  // Reorient to axials to avoid issues with registration metrics not
  // transforming image gradients with the image orientation in
  // calculating the derivative of metric wrt transformation
  // parameters.
  //
  // Forcing image to be axials avoids this problem. Note, that
  // reorientation only affects the internal mapping from index to
  // physical coordinates.  The reoriented data spans the same
  // physical space as the original data.  Thus, the registration
  // transform calculated on the reoriented data is also the
  // transform forthe original un-reoriented data. 
  OrientFilterType::Pointer orientFixed = OrientFilterType::New();
  orientFixed->UseImageDirectionOn();
  orientFixed->SetDesiredCoordinateOrientationToAxial();
  orientFixed->SetInput (FixedReader->GetOutput());
  orientFixed->Update();

  OrientFilterType::Pointer orientMoving = OrientFilterType::New();
  orientMoving->UseImageDirectionOn();
  orientMoving->SetDesiredCoordinateOrientationToAxial();
  orientMoving->SetInput (MovingReader->GetOutput());
  orientMoving->Update();
  
  fixed = orientFixed->GetOutput();
  moving = orientMoving->GetOutput();

//   // Align the centers of the volumes (this block of code is needed to
//   // circumvent errors in the calculations of FlipImageFilter,
//   // PermuteImageFilter, and OrientImageFilter. after reorientation,
//   // the image needs to span the same physical space as the original image).
//   //
//   Volume::PointType newOrigin;
//   itk::AlignVolumeCenters<Volume>(FixedReader->GetOutput(),
//                                   orientMoving->GetOutput(),
//                                   newOrigin);
//   ChangeFilterType::Pointer changeMoving = ChangeFilterType::New();
//   changeMoving->SetInput( orientMoving->GetOutput() );
//   changeMoving->ChangeOriginOn();
//   changeMoving->SetOutputOrigin(newOrigin);
//   changeMoving->Update();

//   itk::AlignVolumeCenters<Volume>(FixedReader->GetOutput(),
//                                   orientFixed->GetOutput(),
//                                   newOrigin);
//   ChangeFilterType::Pointer changeFixed = ChangeFilterType::New();
//   changeFixed->SetInput( orientFixed->GetOutput() );
//   changeFixed->ChangeOriginOn();
//   changeFixed->SetOutputOrigin(newOrigin);
//   changeFixed->Update();
    
//   fixed = changeFixed->GetOutput();
//   moving = changeMoving->GetOutput();
  
  //
  //
  msg.str("");
  msg << "Original Fixed Image: \n";
  FixedReader->GetOutput()->Print ( msg );
  logger->Write ( itk::LoggerBase::INFO, msg.str() );

  msg.str("");
  msg << "Reoriented Fixed Image: \n";
  fixed->Print ( msg );
  logger->Write ( itk::LoggerBase::INFO, msg.str() );

//   msg.str("");
//   msg << "Fixed image was reoriented using: \n";
//   orientFixed->Print( msg );
//   logger->Write ( itk::LoggerBase::INFO, msg.str() );

  msg.str("");
  msg << "Original Moving Image: \n";
  MovingReader->GetOutput()->Print ( msg );
  logger->Write ( itk::LoggerBase::INFO, msg.str() );

  msg.str("");
  msg << "Reoriented Moving Image: \n";
  moving->Print ( msg );
  logger->Write ( itk::LoggerBase::INFO, msg.str() );

//   msg.str("");
//   msg << "Moving image was reoriented using: \n";
//   orientMoving->Print( msg );
//   logger->Write ( itk::LoggerBase::INFO, msg.str() );
  
  // Set up the registration.
  //
  // 
  typedef itk::QuaternionRigidTransform< double > TransformType;
  typedef itk::QuaternionRigidTransformGradientDescentOptimizer OptimizerType;
  typedef itk::MattesMutualInformationImageToImageMetric<Volume,Volume> MetricType;
  typedef itk::LinearInterpolateImageFunction<Volume,double> InterpolatorType;
  typedef itk::ImageRegistrationMethod<Volume,Volume> RegistrationType;

  MetricType::Pointer         metric        = MetricType::New();
  OptimizerType::Pointer      optimizer     = OptimizerType::New();
  InterpolatorType::Pointer   interpolator  = InterpolatorType::New();
  RegistrationType::Pointer   registration  = RegistrationType::New();
  TransformType::Pointer  transform = TransformType::New();

  registration->SetMetric ( metric );
  registration->SetOptimizer ( optimizer );
  registration->SetInterpolator ( interpolator );
  registration->SetTransform ( transform );
  registration->SetFixedImage ( fixed );
  registration->SetMovingImage ( moving );

  ScheduleCommand::Pointer Schedule = ScheduleCommand::New();
  Schedule->SetSchedule ( Iterations, LearningRate );
  int sum = 0;
  for (size_t i = 0; i < Iterations.size(); i++)
    {
    sum += Iterations[i];
    }
  optimizer->SetNumberOfIterations ( sum );
  optimizer->SetLearningRate ( LearningRate[0] );
  optimizer->AddObserver ( itk::IterationEvent(), Schedule );
  typedef OptimizerType::ScalesType OptimizerScalesType;
  OptimizerScalesType scales( transform->GetNumberOfParameters() );
  scales.Fill ( 1.0 );
  for( unsigned j = 4; j < 7; j++ )
    {
    scales[j] = 1.0 / vnl_math_sqr(TranslationScale);
    }
  optimizer->SetScales( scales );
  optimizer->SetMinimize ( true );

  metric->SetNumberOfHistogramBins ( HistogramBins );
  metric->SetNumberOfSpatialSamples( SpatialSamples );


  // Initialize the transform by centering the transform and
  // aligning the centers of the volumes
  //
  TransformType::InputPointType centerFixed;
  Volume::RegionType::SizeType sizeFixed = fixed->GetLargestPossibleRegion().GetSize();
  // Find the center
  ContinuousIndexType indexFixed;
  for ( unsigned j = 0; j < 3; j++ )
    {
    indexFixed[j] = (sizeFixed[j]-1) / 2.0;
    }
  fixed->TransformContinuousIndexToPhysicalPoint( indexFixed, centerFixed );

  TransformType::InputPointType centerMoving;
  Volume::RegionType::SizeType sizeMoving = moving->GetLargestPossibleRegion().GetSize();
  // Find the center
  ContinuousIndexType indexMoving;
  for ( unsigned j = 0; j < 3; j++ )
    {
    indexMoving[j] = (sizeMoving[j]-1) / 2.0;
    }
  moving->TransformContinuousIndexToPhysicalPoint( indexMoving, centerMoving );

  transform->SetCenter( centerFixed );
  transform->Translate(centerMoving-centerFixed);
  msg.str(""); msg << "Centering transform: "; transform->Print ( msg );
  logger->Write ( itk::LoggerBase::INFO, msg.str() );


  // If an initial transformation was provided, then grab the rigid
  // part and use it instead of the the centering transform.
  // (Should this be instead of the centering transform or composed
  // with the centering transform?
  //
  if (InitialTransform != ""
      && initialTransform->GetTransformList()->size() != 0)
    {
    TransformReaderType::TransformType::Pointer initial
      = *(initialTransform->GetTransformList()->begin());
    
    // most likely, the transform coming in is a subclass of
    // MatrixOffsetTransformBase 
    typedef itk::MatrixOffsetTransformBase<double,3,3> DoubleMatrixOffsetType;
    typedef itk::MatrixOffsetTransformBase<float,3,3> FloatMatrixOffsetType;
    
    DoubleMatrixOffsetType::Pointer da
      = dynamic_cast<DoubleMatrixOffsetType*>(initial.GetPointer());
    FloatMatrixOffsetType::Pointer fa
      = dynamic_cast<FloatMatrixOffsetType*>(initial.GetPointer());

    if (da)
      {
      vnl_svd<double> svd(da->GetMatrix().GetVnlMatrix());

      transform->SetMatrix( svd.U() * vnl_transpose(svd.V()) );
      transform->SetOffset( da->GetOffset() );
      }
    else if (fa)
      {
      vnl_matrix<double> t(3,3);
      for (int i=0; i < 3; ++i)
        {
        for (int j=0; j <3; ++j)
          {
          t.put(i, j, fa->GetMatrix().GetVnlMatrix().get(i, j));
          }
        }

      vnl_svd<double> svd( t );
      
      transform->SetMatrix( svd.U() * vnl_transpose(svd.V()) );
      transform->SetOffset( fa->GetOffset() );
      }
    else
      {
      logger->Write( itk::LoggerBase::CRITICAL, "Initial transform is an unsupported type.\n");
      }

    msg.str(""); msg << "Initial transform: "; transform->Print ( msg );
    logger->Write ( itk::LoggerBase::INFO, msg.str() );
    }


  // Run the registration
  //
  //
  registration->SetInitialTransformParameters ( transform->GetParameters() );
  try
    {
    registration->Update();     
    logger->Write ( itk::LoggerBase::INFO, "Registration finished\n" );
    } 
  catch( itk::ExceptionObject & err )
    {
    msg.str("ITK exception caught during registration.\n"); msg << err;
    logger->Write ( itk::LoggerBase::CRITICAL, msg.str() );

    exit ( EXIT_FAILURE );
    } 
  catch ( ... )
    {
    logger->Write ( itk::LoggerBase::FATAL, "Unknown exception caught during registration.\n" );
    exit ( EXIT_FAILURE );
    }

  transform->SetParameters ( registration->GetLastTransformParameters() );

  msg.str(""); msg << "Final registration transform: "; transform->Print(msg);
  logger->Write ( itk::LoggerBase::INFO, msg.str() );

  if (OutputTransform != "")
    {
    typedef itk::TransformFileWriter TransformWriterType;
    TransformWriterType::Pointer outputTransformWriter;

    outputTransformWriter= TransformWriterType::New();
    outputTransformWriter->SetFileName( OutputTransform );
    outputTransformWriter->SetInput( transform );
    try
      {
      outputTransformWriter->Update();
      }
    catch (itk::ExceptionObject &err)
      {
      std::cerr << err << std::endl;
      exit( EXIT_FAILURE );
      }
    }

  // Resample to the original coordinate frame (not the reoriented
  // axial coordinate frame) of the fixed image
  //
  if (ResampledImageFileName != "")
    {
    typedef itk::ResampleImageFilter<Volume,Volume> ResampleType;
    ResampleType::Pointer Resample = ResampleType::New();
    
    typedef itk::LinearInterpolateImageFunction<Volume, double>
      ResampleInterpolatorType;
    ResampleInterpolatorType::Pointer Interpolator
      = ResampleInterpolatorType::New();

    Resample->SetInput ( MovingReader->GetOutput() ); 
    Resample->SetTransform ( transform );
    Resample->SetInterpolator ( Interpolator );
    Resample->SetOutputParametersFromImage ( FixedReader->GetOutput() );
    Resample->Update();

    typedef itk::ImageFileWriter<Volume> WriterType;
    WriterType::Pointer ResampledWriter = WriterType::New();
    ResampledWriter->SetFileName ( ResampledImageFileName.c_str() );
    ResampledWriter->SetInput ( Resample->GetOutput() );
    try
      {
      ResampledWriter->Write();
      }
    catch( itk::ExceptionObject & err )
      { 
      std::cerr << err << std::endl;
      exit ( EXIT_FAILURE );
      }
    }

  
  exit ( EXIT_SUCCESS );
}
  
