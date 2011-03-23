/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile$
 *  Language:  C++
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even
 *  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/
#ifndef __genericRegistrationHelper_h
#define __genericRegistrationHelper_h
#if defined( _MSC_VER )
#  pragma warning ( disable : 4786 )
#endif


#include "itkImageToImageMetric.h"

#include "itkImageRegistrationMethod.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkImage.h"
#include "itkDataObjectDecorator.h"

#include "itkCenteredTransformInitializer.h"

#include <fstream>
#include <string>

#include "itkMultiThreader.h"
#include "itkResampleImageFilter.h"
#ifdef USE_DEBUG_IMAGE_VIEWER
#  include "DebugImageViewerClient.h"
#  include "itkLinearInterpolateImageFunction.h"
#  include "Imgmath.h"
#endif

#include <stdio.h>

enum {
  Dimension = 3,
  MaxInputDimension = 4
  };

//  The following section of code implements a Command observer
//  that will monitor the evolution of the registration process.
//
#include "itkCommand.h"

namespace BRAINSFit
{
template< typename TOptimizer, typename TTransform, typename TImage >
class CommandIterationUpdate:public itk::Command
{
public:
  typedef  CommandIterationUpdate   Self;
  typedef  itk::Command             Superclass;
  typedef itk::SmartPointer< Self > Pointer;
  itkNewMacro(Self);
protected:
  CommandIterationUpdate():m_DisplayDeformedImage(false),
    m_PromptUserAfterDisplay(false)
  {}
private:
  bool m_DisplayDeformedImage;
  bool m_PromptUserAfterDisplay;
  bool m_PrintParameters;

  typename TImage::Pointer m_MovingImage;
  typename TImage::Pointer m_FixedImage;
  typename TTransform::Pointer m_Transform;

#ifdef USE_DEBUG_IMAGE_VIEWER
  DebugImageViewerClient m_DebugImageDisplaySender;
#endif
public:
  typedef          TOptimizer  OptimizerType;
  typedef const OptimizerType *OptimizerPointer;
  void SetDisplayDeformedImage(bool x)
  {
    m_DisplayDeformedImage = x;
#if USE_DEBUG_IMAGE_VIEWER
    m_DebugImageDisplaySender.SetEnabled(x);
#endif
  }

  void SetPromptUserAfterDisplay(bool x)
  {
    m_PromptUserAfterDisplay = x;
#if USE_DEBUG_IMAGE_VIEWER
    m_DebugImageDisplaySender.SetPromptUser(x);
#endif
  }

  void SetPrintParameters(bool x)
  {
    m_PrintParameters = x;
  }

  void SetFixedImage(typename TImage::Pointer fixed)
  {
    m_FixedImage = fixed;
  }

  void SetMovingImage(typename TImage::Pointer moving)
  {
    m_MovingImage = moving;
  }

  void SetTransform(typename TTransform::Pointer & xfrm)
  {
    m_Transform = xfrm;
  }

  void Execute(itk::Object *caller, const itk::EventObject & event)
  {
    Execute( (const itk::Object *)caller, event );
  }

  typename TImage::Pointer Transform(typename TTransform::Pointer & xfrm)
  {
    //      std::cerr << "Moving Volume (in observer): " << this->m_MovingImage
    // << std::endl;
    typedef typename itk::LinearInterpolateImageFunction< TImage, double > InterpolatorType;
    typename InterpolatorType::Pointer interp = InterpolatorType::New();
    typedef typename itk::ResampleImageFilter< TImage, TImage > ResampleImageFilter;
    typename ResampleImageFilter::Pointer resample = ResampleImageFilter::New();
    resample->SetInput(m_MovingImage);
    resample->SetTransform(xfrm);
    resample->SetInterpolator(interp);
    resample->SetOutputParametersFromImage(m_FixedImage);
    resample->SetOutputDirection( m_FixedImage->GetDirection() );
    resample->SetDefaultPixelValue(0);
    resample->Update();
    typename TImage::Pointer rval = resample->GetOutput();
    rval->DisconnectPipeline();
    return rval;
  }

  void Execute(const itk::Object *object, const itk::EventObject & event)
  {
    OptimizerPointer optimizer = dynamic_cast< OptimizerPointer >( object );

    if ( optimizer == NULL )
      {
      std::cout << "ERROR::" << __FILE__ << " " << __LINE__ << std::endl;
      exit(-1);
      }
    if ( !itk::IterationEvent().CheckEvent(&event) )
      {
      return;
      }

    typename OptimizerType::ParametersType parms =
      optimizer->GetCurrentPosition();
    int  psize = parms.GetNumberOfElements();
    bool parmsNonEmpty = false;
    for ( int i = 0; i < psize; i++ )
      {
      if ( parms[i] != 0.0 )
        {
        parmsNonEmpty = true;
        break;
        }
      }

    if ( m_PrintParameters )
      {
      std::cout << optimizer->GetCurrentIteration() << "   ";
      std::cout << optimizer->GetValue() << "   ";
      if ( parmsNonEmpty )
        {
        std::cout << parms;
        }
      std::cout << std::endl;
      }

#ifdef USE_DEBUG_IMAGE_VIEWER
    if ( m_DisplayDeformedImage )
      {
      if ( parmsNonEmpty )
        {
        m_Transform->SetParametersByValue(parms);
        }
      // else, if it is a vnl optimizer wrapper, i.e., the BSpline optimizer,
      // the only hint you get
      // is in the transform object used by the optimizer, so don't erase it,
      // use it.
      typename TImage::Pointer transformResult =
        this->Transform(m_Transform);
      //      std::cerr << "Moving Volume (after transform): " <<
      // transformResult << std::endl;
      m_DebugImageDisplaySender.SendImage< TImage >(transformResult, 1);
      typename TImage::Pointer diff = Isub< TImage >(m_FixedImage, transformResult);

      m_DebugImageDisplaySender.SendImage< TImage >(diff, 2);
      }
#endif
  }
};
} // end namespace BRAINSFit

namespace itk
{
//TODO:  Remove default MetricType here, and force a choice
template< typename TTransformType, typename TOptimizer, typename TFixedImage,
          typename TMovingImage, typename MetricType>
class ITK_EXPORT MultiModal3DMutualRegistrationHelper:public ProcessObject
{
public:
  /** Standard class typedefs. */
  typedef MultiModal3DMutualRegistrationHelper Self;
  typedef ProcessObject                        Superclass;
  typedef SmartPointer< Self >                 Pointer;
  typedef SmartPointer< const Self >           ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(MultiModal3DMutualRegistrationHelper, ProcessObject);

  typedef          TFixedImage                  FixedImageType;
  typedef typename FixedImageType::ConstPointer FixedImageConstPointer;
  typedef typename FixedImageType::Pointer      FixedImagePointer;

  typedef          TMovingImage                  MovingImageType;
  typedef typename MovingImageType::ConstPointer MovingImageConstPointer;
  typedef typename MovingImageType::Pointer      MovingImagePointer;

  typedef          TTransformType         TransformType;
  typedef typename TransformType::Pointer TransformPointer;

  /** Type for the output: Using Decorator pattern for enabling
    *  the Transform to be passed in the data pipeline */
  typedef DataObjectDecorator< TransformType >  TransformOutputType;
  typedef typename TransformOutputType::Pointer TransformOutputPointer;
  typedef typename TransformOutputType::ConstPointer
  TransformOutputConstPointer;

  typedef          TOptimizer                    OptimizerType;
  typedef const OptimizerType *                  OptimizerPointer;
  typedef typename OptimizerType::ScalesType     OptimizerScalesType;
  typedef typename OptimizerType::ParametersType OptimizerParametersType;

  typedef typename MetricType::FixedImageMaskType  FixedBinaryVolumeType;
  typedef typename FixedBinaryVolumeType::Pointer  FixedBinaryVolumePointer;
  typedef typename MetricType::MovingImageMaskType MovingBinaryVolumeType;
  typedef typename MovingBinaryVolumeType::Pointer MovingBinaryVolumePointer;

  typedef LinearInterpolateImageFunction<
    MovingImageType,
    double >    InterpolatorType;

  typedef ImageRegistrationMethod< FixedImageType, MovingImageType > RegistrationType;
  typedef typename RegistrationType::Pointer                         RegistrationPointer;

  typedef itk::CenteredTransformInitializer<
    TransformType,
    FixedImageType,
    MovingImageType >    TransformInitializerType;

  typedef itk::ResampleImageFilter<
    MovingImageType,
    FixedImageType >     ResampleFilterType;

  /** Initialize by setting the interconnects between the components. */
  virtual void Initialize(void)
  throw ( ExceptionObject );

  /** Method that initiates the registration. */
  void StartRegistration(void);

  /** Set/Get the Fixed image. */
  void SetFixedImage(FixedImagePointer fixedImage);

  itkGetConstObjectMacro(FixedImage, FixedImageType);

  /** Set/Get the Moving image. */
  void SetMovingImage(MovingImagePointer movingImage);

  itkGetConstObjectMacro(MovingImage, MovingImageType);

  /** Set/Get the InitialTransfrom. */
  void SetInitialTransform(typename TransformType::Pointer initialTransform);
  itkGetConstObjectMacro(InitialTransform, TransformType);

  /** Set/Get the Transfrom. */
  itkSetObjectMacro(Transform, TransformType);
  typename TransformType::Pointer GetTransform(void);

  // itkSetMacro( PermitParameterVariation, std::vector<int>      );

  itkSetObjectMacro(CostMetricObject,MetricType);
  itkGetObjectMacro(CostMetricObject,MetricType);

  itkSetMacro(NumberOfSamples,               unsigned int);
  itkSetMacro(NumberOfHistogramBins,         unsigned int);
  itkSetMacro(NumberOfIterations,            unsigned int);
  itkSetMacro(RelaxationFactor,              double);
  itkSetMacro(MaximumStepLength,             double);
  itkSetMacro(MinimumStepLength,             double);
  itkSetMacro(TranslationScale,              double);
  itkSetMacro(ReproportionScale,             double);
  itkSetMacro(SkewScale,                     double);
  itkSetMacro(InitialTransformPassThruFlag,  bool);
  itkSetMacro(BackgroundFillValue,           double);
  itkSetMacro(DisplayDeformedImage,          bool);
  itkSetMacro(PromptUserAfterDisplay,        bool);
  itkGetConstMacro(FinalMetricValue,         double);
  itkGetConstMacro(ActualNumberOfIterations, unsigned int);
  itkSetMacro(ObserveIterations,        bool);
  itkGetConstMacro(ObserveIterations,        bool);
  /** Returns the transform resulting from the registration process  */
  const TransformOutputType * GetOutput() const;

  /** Make a DataObject of the correct type to be used as the specified
    * output. */
  virtual DataObjectPointer MakeOutput(unsigned int idx);

  /** Method to return the latest modified time of this object or
    * any of its cached ivars */
  unsigned long GetMTime() const;

  /** Method to set the Permission to vary by level  */
  void SetPermitParameterVariation(std::vector< int > perms)
  {
    m_PermitParameterVariation.resize( perms.size() );
    for ( unsigned int i = 0; i < perms.size(); i++ )
      {
      m_PermitParameterVariation[i] = perms[i];
      }
  }

protected:
  MultiModal3DMutualRegistrationHelper();
  virtual ~MultiModal3DMutualRegistrationHelper()
  {}

  void PrintSelf(std::ostream & os, Indent indent) const;

  /** Method invoked by the pipeline in order to trigger the computation of
    * the registration. */
  void  GenerateData();

private:
  MultiModal3DMutualRegistrationHelper(const Self &);             // purposely
                                                                  // not
  // implemented
  void operator=(const Self &);                                   // purposely

  // not

  // implemented

  // FixedImageConstPointer           m_FixedImage;
  // MovingImageConstPointer          m_MovingImage;

  FixedImagePointer  m_FixedImage;
  MovingImagePointer m_MovingImage;
  TransformPointer          m_InitialTransform;
  TransformPointer          m_Transform;
  //
  // make sure parameters persist until after they're used by the transform

  RegistrationPointer m_Registration;

  std::vector< int >            m_PermitParameterVariation;
  typename MetricType::Pointer  m_CostMetricObject;

  unsigned int m_NumberOfSamples;
  unsigned int m_NumberOfHistogramBins;
  unsigned int m_NumberOfIterations;
  double       m_RelaxationFactor;
  double       m_MaximumStepLength;
  double       m_MinimumStepLength;
  double       m_TranslationScale;
  double       m_ReproportionScale;
  double       m_SkewScale;
  bool         m_InitialTransformPassThruFlag;
  double       m_BackgroundFillValue;
  unsigned int m_ActualNumberOfIterations;
  bool         m_DisplayDeformedImage;
  bool         m_PromptUserAfterDisplay;
  double       m_FinalMetricValue;
  bool         m_ObserveIterations;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "genericRegistrationHelper.txx"
#endif

#endif
