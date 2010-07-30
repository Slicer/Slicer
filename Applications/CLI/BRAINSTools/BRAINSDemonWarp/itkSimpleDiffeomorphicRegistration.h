#ifndef __itkSimpleDiffeomorphicRegistration_h
#define __itkSimpleDiffeomorphicRegistration_h

#include <string>
#include "itkImage.h"
#include "itkObject.h"
// #include "BRAINSDemonWarp.h"
#include "itkVector.h"
#include "DemonsPreprocessor.h"
#include "DemonsRegistrator.h"

#define DIM                         3
#define MaxStepLength               2
#define SmoothDeformationFieldSigma 1.5
#define NumberOfLevels              5
#define NumberOfIteration0          300
#define NumberOfIteration1          100                              // 100
#define NumberOfIteration2          30                               // 30
#define NumberOfIteration3          20                               // 20
#define NumberOfIteration4          15                               // 15
#define FixedPyramid                16
#define NumberOfMatchPoints         7
#define NumberOfHistogramLevels     1024

/** TODO:  Need to document this class
  */
class itkSimpleDiffeomorphicRegistration:public itk::Object
{
public:
  typedef itk::Image< float, DIM > TRealImage;
  typedef itk::DemonsPreprocessor< TRealImage,
                                   TRealImage > DemonsPreprocessorType;
  typedef itk::DemonsRegistrator< TRealImage, TRealImage,
                                  float > DemonsRegistratorType;
  typedef itk::Image< itk::Vector< float, DIM >, DIM > TDeformationField;

  itkSimpleDiffeomorphicRegistration();
  itkSetObjectMacro(FixedImage, TRealImage);
  itkSetObjectMacro(MovingImage, TRealImage);

  itkSetStringMacro(DeformationFieldName);
  itkSetStringMacro(DeformedImageName);
  itkGetStringMacro(DeformedImageName);
  itkGetObjectMacro(DeformationField, TDeformationField);
  void Update();

protected:
  // std::string GetFixedImage(void);
  // std::string GetMovingImage(void);
  // std::string GetDeformedImageName(void);
  // std::string GetDisplacementPrefixName(void);
  void InitializePreprocessor();

  void Initialization(void);

private:
  DemonsPreprocessorType::Pointer m_DemonsPreprocessor;
  DemonsRegistratorType::Pointer  m_DemonsRegistrator;
  TDeformationField::Pointer      m_DeformationField;
  TRealImage::Pointer             m_FixedImage;
  TRealImage::Pointer             m_MovingImage;
  std::string                     m_DeformedImageName;
  std::string                     m_DeformationFieldName;
};

#endif
