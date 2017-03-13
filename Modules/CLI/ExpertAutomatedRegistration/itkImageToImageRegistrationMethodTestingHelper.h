/*=========================================================================

  Program:   Surface Extraction Program
  Module:    $RCSfile: ExtractSurface.cxx,v $

  Copyright (c) Kitware Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef itkImageToImageRegistrationMethodTestingHelper_h
#define itkImageToImageRegistrationMethodTestingHelper_h

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkResampleImageFilter.h"
#include "itkTestingComparisonImageFilter.h"

namespace itk
{

/** \class ImageToImageRegistrationMethodTestingHelper
 *
 *  This class provides common tasks required to test all registration methods.
 *
 */
template <class TRegistrationMethod>
class ImageToImageRegistrationMethodTestingHelper
{

  typedef TRegistrationMethod                        RegistrationMethodType;
  typedef typename RegistrationMethodType::ImageType ImageType;
  typedef typename RegistrationMethodType::Pointer   RegistrationMethodPointer;

  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  typedef itk::ImageFileWriter<ImageType> ImageWriterType;

  typedef typename ImageReaderType::Pointer ImageReaderPointer;
  typedef typename ImageWriterType::Pointer ImageWriterPointer;

  typedef ResampleImageFilter<ImageType, ImageType> ResampleFilterType;
  typedef typename ResampleFilterType::Pointer      ResampleFilterPointer;

  typedef itk::Testing::ComparisonImageFilter<ImageType, ImageType> DifferenceFilterType;
  typedef typename DifferenceFilterType::Pointer                    DifferenceFilterPointer;
public:

  /** Constructor */
  ImageToImageRegistrationMethodTestingHelper();

  /** Return specific instance of the registration method being tested */
  RegistrationMethodType * GetRegistrationMethod();

  /** Set the filename of the FixedImage. This helper class will create a
   * reader and will read the image in. */
  void SetFixedImageFileName( const char * filename );

  /** Set the filename of the MovingImage. This helper class will create a
   * reader and will read the image in. */
  void SetMovingImageFileName( const char * filename );

  /** Set the filename where the Resampled MovingImage will be saved. This
   * helper class will create a resample image filter and a writer, and will
   * save the image. */
  void SetResampledImageFileName( const char * filename );

  /** Set the filename of the image that should be used as baseline for the
   * regression test of the resampled moving image.. */
  void SetBaselineImageFileName( const char * filename );

  /** Set the filename of the difference image that should be output */
  void SetDifferenceImageFileName( const char * filename );

  /** Prepare the elements of the registration. In particular, this method
   * connects the fixed and moving image readers as inputs to the
   * registration method. */
  void PrepareRegistration();

  /** Initiate the execution of the registration process. This method catches
   * potential exceptions that may be thrown from the registration method. */
  void RunRegistration();

  /** Print out information relevant to the results of the registration
   * process. In particular the Transform parameters. Note that you may not
   * want to do this with transforms having large number of parameters, such
   * as the BSplineDeformableTransform. */
  void ReportResults();

  /** Print out meta data from the input Fixed and Moving images.
   */
  void ReportImagesMetaData();

  /** Use the parameters resulting from the optimization process, set them in
   * a Transform, and use it to resample the Moving image into the coordinate
   * system of the Fixed image. */
  void ResampleOutputImage();

  /** Run the regression test by comparing the result of resampling the
   * moving image against the baseline image whose filename was provided in
   * SetBaselineImageFileName() */
  void PerformRegressionTest();

  /** Exercise the Print() method of the registration class. This should
   * internally call the PrintSelf() method of the registration class. */
  void PrintTest();

  /** Return the status of the test helper at the end of all the tests */
  int GetTestResult() const;

  /** Set the number of pixel errors that can be tolerated before marking
      the test as failed. */
  void SetNumberOfFailedPixelsTolerance( unsigned long numberOfPixels );

  /** Set the misregistration offset that can be tolerated before marking
      the test as failed. */
  void SetRadiusTolerance( unsigned long radius );

  /** Set the intensity difference that can be tolerated before marking
      the test as failed. */
  void SetIntensityTolerance( double intensity );

protected:
private:

  RegistrationMethodPointer m_RegistrationMethod;
  ImageReaderPointer        m_FixedImageReader;
  ImageReaderPointer        m_MovingImageReader;
  ImageWriterPointer        m_MovingImageWriter;
  ImageReaderPointer        m_BaselineImageReader;
  ImageWriterPointer        m_DifferenceImageWriter;
  DifferenceFilterPointer   m_DifferenceFilter;
  ResampleFilterPointer     m_ResampleFilter;
  unsigned long             m_NumberOfFailedPixelsTolerance;
  unsigned long             m_RadiusTolerance;
  double                    m_IntensityTolerance;

  bool m_ErrorState;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkImageToImageRegistrationMethodTestingHelper.txx"
#endif

#endif
