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

#include "LinearRegistrationCLP.h"
#include <itkCommand.h>
#include <itkImage.h>
#include <itkOrientedImage.h>
#include <itkOrientImageFilter.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

// ITK Stuff
// Registration
#include <itkCenteredTransformInitializer.h>
#include <itkGradientDescentOptimizer.h>
#include <itkImageFileWriter.h>
#include <itkImageRegistrationMethod.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkMattesMutualInformationImageToImageMetric.h>
#include <itkQuaternionRigidTransform.h>
#include <itkQuaternionRigidTransformGradientDescentOptimizer.h>
#include <itkResampleImageFilter.h>
#include <itkStdStreamLogOutput.h>
#include <itkLogger.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

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
    this->m_LearningRates.resize(0);
    for (size_t i = 0; i < LearningRates.size(); i++)
      {
      this->m_LearningRates.push_back(LearningRates[i]);
      }
  }
  void SetNumberOfIterations ( std::vector<int> &NumberOfIterations ) {
    this->m_NumberOfIterations.resize(0);
    for (size_t i = 0; i < NumberOfIterations.size(); i++)
      {
      this->m_NumberOfIterations.push_back(NumberOfIterations[i]);
      }
    this->m_NextChange = NumberOfIterations[0];
  }
  void SetSchedule ( std::vector<int> &NumberOfIterations, std::vector<double> &LearningRates )
  {
    this->SetNumberOfIterations(NumberOfIterations);
    this->SetLearningRates(LearningRates);
  }
  void DoExecute ( itk::GradientDescentOptimizer* optimizer ) 
  {
    if ( m_Schedule < m_NumberOfIterations.size()-1 ) {
      if ( optimizer->GetCurrentIteration() >= this->m_NumberOfIterations[ m_Schedule ]) {
        m_Schedule++;
        optimizer->SetLearningRate ( this->m_LearningRates[m_Schedule] );
        this->m_NextChange = this->m_NumberOfIterations[m_Schedule];
        std::cout << "Iteration: " << optimizer->GetCurrentIteration() << " LearningRate: " << optimizer->GetLearningRate() << std::endl;
      }
    }
  }
  void Execute ( itk::Object *caller, const itk::EventObject & event )
  {
    itk::GradientDescentOptimizer* obj = dynamic_cast<itk::GradientDescentOptimizer*>(caller);
    if ( obj->GetCurrentIteration() >= this->m_NextChange ) {
      this->DoExecute ( obj );
    }
  }
  void Execute ( const itk::Object *caller, const itk::EventObject & event )
  {
    itk::GradientDescentOptimizer* obj = (itk::GradientDescentOptimizer*)(caller);
    if ( obj->GetCurrentIteration() >= this->m_NextChange ) {
      this->DoExecute ( obj );
    }
  }
 protected:
  std::vector<unsigned int> m_NumberOfIterations;
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
        
 void Split(const char* str, const char* delim, std::vector<std::string>& lines)
{
  std::string data(str);
  std::string::size_type lpos = 0;
  while(lpos < data.length())
    {
    std::string::size_type rpos = data.find_first_of(delim, lpos);
    if(rpos == std::string::npos)
      {
      // Line ends at end of string without a newline.
      lines.push_back(data.substr(lpos));
      return;
      }
    // Line ends in a "\n", remove the character.
    lines.push_back(data.substr(lpos, rpos-lpos));
    lpos = rpos+1;
    }
  return;
}

template<typename T> itk::Array<T> Parse ( std::string s )
{
  itk::Array<T> values;
  std::vector<std::string> l;
  Split ( s.c_str(), ",", l );
  values.SetSize ( l.size() );
  for ( int i = 0; i < l.size(); i++ )
    {
    values[i] = (T) atof ( l[i].c_str() );
    }
  return values;
}

typedef itk::OrientedImage<signed short, 3> Volume;

int main ( int argc, char* argv[] ) 
{  
  itk::OStringStream msg;
  itk::StdStreamLogOutput::Pointer coutput = itk::StdStreamLogOutput::New();
  coutput->SetStream(std::cout);
  itk::Logger::Pointer logger = itk::Logger::New();
  logger->SetName ( "CLRegistration" );
  logger->AddLogOutput ( coutput );

PARSE_ARGS;
  bool DoInitializeTransform = false;
  int RandomSeed = 1234567;
  double GradientMagnitudeTolerance = 1e-5;

  std::cout << "Parsed arguments" << std::endl
            << "HistogramBins: " << HistogramBins << std::endl
            << "RandomSeed: " << RandomSeed << std::endl
            << "GradientMagnitudeTolerance: " << GradientMagnitudeTolerance << std::endl;
  std::cout << "Iterations: ";
  for ( int i = 0; i < Iterations.size(); i++ )
    {
    std::cout << Iterations[i] << ", ";
    }
  std:: cout << "\nLearningRate: ";
  for ( int i = 0; i < LearningRate.size(); i++ )
    {
    std::cout << LearningRate[i] << ", ";
    }
  std::cout << "\nSpatialSamples: " << SpatialSamples << std::endl
            << "TranslationScale: " << TranslationScale << std::endl
            << "DoInitializeTransform: " << DoInitializeTransform << std::endl
            << "fixedImageFileName: " << fixedImageFileName << std::endl
            << "movingImageFileName: " << movingImageFileName << std::endl
            << "resampledImageFileName: " << resampledImageFileName << std::endl
            << std::endl;

  Volume::Pointer fixed, moving, rfixed, rmoving;
  typedef itk::OrientImageFilter<Volume,Volume> OrientFilterType;
  
  typedef itk::ImageFileReader<Volume> FileReaderType;
  FileReaderType::Pointer FixedReader = FileReaderType::New();
  FileReaderType::Pointer MovingReader = FileReaderType::New();
  FixedReader->SetFileName ( fixedImageFileName.c_str() );
  MovingReader->SetFileName ( movingImageFileName.c_str() );

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

  // read in the moving image and do nothing
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

  Volume::DirectionType dir;
  dir.SetIdentity();
  // dir[0][0] = -1;
  // fixed->SetDirection(dir);
  // moving->SetDirection(dir);

  
  msg.str("");
  msg << "Fixed Image: \n";
  fixed->Print ( msg );
  logger->Write ( itk::LoggerBase::INFO, msg.str() );
  
  msg.str("");
  msg << "Moving Image: \n";
  moving->Print ( msg );
  logger->Write ( itk::LoggerBase::INFO, msg.str() );


  Volume::PointType pt1, pt2;
  Volume::IndexType idx, idx1, idx2;
  pt1[0] = 100; pt1[1] = 120; pt1[2] = 30;
  fixed->TransformPhysicalPointToIndex ( pt1, idx );
  fixed->TransformIndexToPhysicalPoint ( idx, pt2 );
  msg.str(""); msg << "Round trip Difference: " << pt1 << " to " << pt2;
  logger->Write ( itk::LoggerBase::INFO, msg.str() );

  idx1[0] = 100; idx1[1] = 138; idx1[2] = 34;
  fixed->TransformIndexToPhysicalPoint ( idx1, pt1 );
  fixed->TransformPhysicalPointToIndex ( pt1, idx2 );
  msg.str(""); msg << "Round trip Difference: " << idx1 << " to " << idx2;
  logger->Write ( itk::LoggerBase::INFO, msg.str() );
  
  
  
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

  // Initialize the transform
  TransformType::InputPointType center;
  Volume::RegionType::SizeType s = fixed->GetLargestPossibleRegion().GetSize();
  // Find the center
  Volume::IndexType centerIndex;
  for ( unsigned j = 0; j < 3; j++ )
    {
    centerIndex[j] = (long) ( (s[j]-1) / 2.0 );
    }
  
  fixed->TransformIndexToPhysicalPoint ( centerIndex, center );

  if (1)
    {
    Volume::RegionType::SizeType s = fixed->GetLargestPossibleRegion().GetSize();
    Volume::IndexType idx = fixed->GetLargestPossibleRegion().GetIndex();
    
    msg.str ( "" ); msg << "Largest Possible Region Size: " << s;
    // logger->info ( msg.str() );
    logger->Write ( itk::LoggerBase::INFO, msg.str() );

    msg.str ( "" ); msg << "Largest Possible Region index: " << idx;
    // logger->info ( msg.str() );
    logger->Write ( itk::LoggerBase::INFO, msg.str() );

    // lower corner
    Volume::PointType pt0;
    fixed->TransformIndexToPhysicalPoint(idx, pt0);  

    // upper corner
    for ( unsigned int i = 0; i < 3; i++)
      {
      idx[i] += (s[i]-1);
      }
    Volume::PointType pt1;
    fixed->TransformIndexToPhysicalPoint(idx, pt1);

    msg.str ( "" ); msg << "Center: ";
    for ( int i = 0; i < 3; i++ )
      {
      center[i] = ( pt0[i] + pt1[i] ) / 2.0;
      msg << center[i] << " ";
      }
    //logger->info ( msg.str() );
    logger->Write ( itk::LoggerBase::INFO, msg.str() );
    }


  
  msg.str(""); msg << "Index: " << centerIndex << " to point: " << center;
  logger->Write ( itk::LoggerBase::INFO, msg.str() );
  transform->SetIdentity();
  transform->SetCenter ( center );
  msg.str(""); msg << "Transform: "; transform->Print ( msg );
  logger->Write ( itk::LoggerBase::INFO, msg.str() );

  TransformType::OutputPointType out;
  Volume::IndexType outIdx;
  out = transform->TransformPoint ( center );
  moving->TransformPhysicalPointToIndex ( out, outIdx );

  msg.str(""); msg << "To moving space: " << out << " index: " << outIdx;
  logger->Write ( itk::LoggerBase::INFO, msg.str() );
  

  
  msg.str(""); msg << "Transform: "; transform->Print ( msg );
  logger->Write ( itk::LoggerBase::INFO, msg.str() );
  
  
  registration->SetInitialTransformParameters ( transform->GetParameters() );


  
  metric->SetNumberOfHistogramBins ( HistogramBins );
  metric->SetNumberOfSpatialSamples( SpatialSamples );

  try
    {
    registration->StartRegistration();     
    logger->Write ( itk::LoggerBase::INFO, "Registration finished" );
    } 
  catch( itk::ExceptionObject & err )
    {
    msg.str(""); msg << err;
    logger->Write ( itk::LoggerBase::CRITICAL, msg.str() );

    exit ( EXIT_FAILURE );
    } 
  catch ( ... )
    {
    logger->Write ( itk::LoggerBase::FATAL, "Something bad!!!" );
    exit ( EXIT_FAILURE );
    }

  typedef itk::ResampleImageFilter<Volume,Volume> ResampleType;
  ResampleType::Pointer Resample = ResampleType::New();

  typedef itk::LinearInterpolateImageFunction<Volume, double> ResampleInterpolatorType;
  ResampleInterpolatorType::Pointer Interpolator = ResampleInterpolatorType::New();

  transform->SetParameters ( registration->GetLastTransformParameters() );

  out = transform->TransformPoint ( center );
  moving->TransformPhysicalPointToIndex ( out, outIdx );

  msg.str(""); msg << "After Registration: To moving space: " << out << " index: " << outIdx;
  logger->Write ( itk::LoggerBase::INFO, msg.str() );
  
  
  fixed->Print ( std::cout );
  moving->Print ( std::cout );
  transform->Print ( std::cout );

  if (resampledImageFileName != "")
    {
    Resample->SetInput ( moving ); 
    Resample->SetTransform ( transform );
    Resample->SetInterpolator ( Interpolator );
    Resample->SetOutputParametersFromImage ( fixed );
    
    Resample->Update();
    typedef itk::ImageFileWriter<Volume> WriterType;
    WriterType::Pointer ResampledWriter = WriterType::New();
    ResampledWriter->SetFileName ( resampledImageFileName.c_str() );
    ResampledWriter->SetInput ( Resample->GetOutput() );
    try
      {
      ResampledWriter->Write();
      }
    catch( itk::ExceptionObject & err )
      { 
      cerr << err << endl;
      exit ( EXIT_FAILURE );
      }
    }
  
  exit ( EXIT_SUCCESS );
}
  
