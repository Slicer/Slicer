/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $HeadURL: http://www.na-mic.org/svn/Slicer3/branches/TransformIO/Applications/CLI/BSplineDeformableRegistration.cxx $
  Language:  C++
  Date:      $Date: 2008-02-07 09:51:14 -0500 (Thu, 07 Feb 2008) $
  Version:   $Revision: 5746 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "UtahBSplineDeformableRegistrationCLP.h"


#include "itkCommand.h"

#include "itkOrientedImage.h"
#include "itkOrientImageFilter.h"
#include "itkResampleImageFilter.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"

#include "itkPluginUtilities.h"

#include "itkTimeProbesCollectorBase.h"

#include "GradientDescent.h"

#include "NormalizedCorrelationCostFunction3D.h"

#include "itkRescaleIntensityImageFilter.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDiscreteGaussianImageFilter.h"

#include <fstream>

typedef NormalizedCorrelationCostFunction3D<double> TCostFunction;


typedef TCostFunction::TParametric TParametric;
typedef TCostFunction::TControlPoint TControlPoint;
typedef TCostFunction::TControlMesh TControlMesh;

typedef TCostFunction::Image Image;
typedef TCostFunction::ImagePointer ImagePointer;
typedef Image::RegionType ImageRegion;
typedef Image::IndexType ImageIndex;
typedef Image::SizeType ImageSize;
typedef ImageIndex::IndexValueType IndexValue;

typedef itk::ImageFileReader<Image> ImageReader;
typedef ImageReader::Pointer ImageReaderPointer;

typedef itk::ImageFileWriter<Image> ImageWriter;


typedef itk::RescaleIntensityImageFilter<Image> RescaleFilter;
typedef RescaleFilter::Pointer RescaleFilterPointer;

typedef itk::DiscreteGaussianImageFilter<Image, Image> GaussianImageFilter;
typedef GaussianImageFilter::Pointer GaussianImageFilterPointer;

typedef itk::ImageFileReader<Image> ImageReader;
typedef ImageReader::Pointer ImageReaderPointer;

typedef itk::ImageFileWriter<Image> ImageWriter;
typedef ImageWriter::Pointer ImageWriterPointer;

ImageRegion findBoundingBox(ImagePointer maskImage){

  ImageIndex minIndex;
  minIndex.Fill( vnl_numeric_traits< IndexValue >::maxval );
  ImageIndex maxIndex;
  maxIndex.Fill(0);

  typedef itk::ImageRegionConstIteratorWithIndex<Image> MaskImageIterator;
  MaskImageIterator maskIt(maskImage, maskImage->GetLargestPossibleRegion());
  for(maskIt.GoToBegin(); !maskIt.IsAtEnd(); ++maskIt){
    if(maskIt.Get() != 0){
      ImageIndex tmp = maskIt.GetIndex();
      for(int i = 0; i < Image::GetImageDimension(); i++){
        if(maxIndex[i] < tmp[i]){
          maxIndex[i] = tmp[i];
        }
        if(minIndex[i] > tmp[i]){
          minIndex[i] = tmp[i];
        }
      }
    }
  }
    


  ImageIndex boundsIndex;
  ImageSize boundsSize;
  for(int i = 0; i < Image::GetImageDimension(); i++){
    boundsSize[i] = (maxIndex[i]  - minIndex[i]);
    boundsIndex[i] = minIndex[i] - boundsSize[i]/2;
    boundsSize[i] *= 2;
  }

  ImageRegion maxRegion = maskImage->GetLargestPossibleRegion();
  ImageSize maxSize = maxRegion.GetSize();
  for(int i = 0; i < Image::GetImageDimension();  i++){
    if(boundsIndex[i] < 0){
      boundsIndex[i] = 0;
    }
    if(boundsIndex[i] + boundsSize[i] > maxSize[i]){
      boundsSize[i] = maxSize[i] - boundsIndex[i];
    }
  }

  ImageRegion bounds(boundsIndex, boundsSize);
  return bounds;
}


template<class T> int DoIt( int argc, char * argv[], T )
{
  PARSE_ARGS;

  //Read Input Image
  ImageReaderPointer fixedImageReader = ImageReader::New();
  fixedImageReader->SetFileName( FixedImageFileName.c_str() );
  RescaleFilterPointer fixedRescaleFilter = RescaleFilter::New();
  fixedRescaleFilter->SetInput(fixedImageReader->GetOutput());
  fixedRescaleFilter->SetOutputMaximum(1);
  fixedRescaleFilter->SetOutputMinimum(0);
  fixedRescaleFilter->Update();
  GaussianImageFilterPointer gaussian1 = GaussianImageFilter::New();
  gaussian1->SetInput(fixedRescaleFilter->GetOutput());
  gaussian1->SetVariance(sigma*sigma);
  gaussian1->Update();
  ImagePointer fixedImage = gaussian1->GetOutput();



  ImageReaderPointer movingImageReader = ImageReader::New();
  movingImageReader->SetFileName( MovingImageFileName.c_str() );
  RescaleFilterPointer movingRescaleFilter = RescaleFilter::New();
  movingRescaleFilter->SetInput(movingImageReader->GetOutput());
  movingRescaleFilter->SetOutputMaximum(1);
  movingRescaleFilter->SetOutputMinimum(0);
  movingRescaleFilter->Update();
  GaussianImageFilterPointer gaussian2 = GaussianImageFilter::New();
  gaussian2->SetInput(movingRescaleFilter->GetOutput());
  gaussian2->SetVariance(sigma*sigma);
  gaussian2->Update();
  ImagePointer movingImage = gaussian2->GetOutput();

  ImageReaderPointer maskImageReader = ImageReader::New();
  maskImageReader->SetFileName( argv[3] );
  maskImageReader->Update();
  ImagePointer maskImage = maskImageReader->GetOutput();
  // typedefs

  
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
  //
  typedef itk::OrientImageFilter<Image,Image> OrientFilterType;
  typename OrientFilterType::Pointer fixedOrient = OrientFilterType::New();
  typename OrientFilterType::Pointer movingOrient = OrientFilterType::New();

  fixedOrient->UseImageDirectionOn();
  fixedOrient->SetDesiredCoordinateOrientationToAxial();
  fixedOrient->SetInput (fixedImage);

  movingOrient->UseImageDirectionOn();
  movingOrient->SetDesiredCoordinateOrientationToAxial();
  movingOrient->SetInput (movingImage);


  // TODO: add the mask image support from Register.cxx (and add universally)

  // Add a time probe
  itk::TimeProbesCollectorBase collector;

  collector.Start( "Read fixed volume" );
//   itk::PluginFilterWatcher watchOrientFixed(fixedOrient,
//                                             "Orient Fixed Image",
//                                             CLPProcessInformation,
//                                             1.0/3.0, 0.0);
  fixedOrient->Update();
  collector.Stop( "Read fixed volume" );

  collector.Start( "Read moving volume" );
//   itk::PluginFilterWatcher watchOrientMoving(movingOrient,
//                                             "Orient Moving Image",
//                                              CLPProcessInformation,
//                                             1.0/3.0, 1.0/3.0);
  movingOrient->Update();
  collector.Stop( "Read moving volume" );


  // Setup BSpline deformation
  //

  //Optimization
  ImageRegion region = movingImage->GetLargestPossibleRegion();
  ImageIndex index = region.GetIndex();
  ImageSize size = region.GetSize();
  double *rangeIndex =  new double[3];
  double *rangeSize =  new double[3];
  int ncps[3]; 
  int degree[3]; 
  int nElements = 3 * gridSize * gridSize * gridSize;
  for(int i=0; i < 3; i++){
    rangeIndex[i] = index[i];
    rangeSize[i] = size[i];
    ncps[i] = gridSize;
    degree[i] = 2;
  }
  TParametric surface = TParametric::createIdentity( rangeIndex, rangeSize, ncps, degree );
 
  TCostFunction costFunction(nElements);
  costFunction.SetFixedImage(fixedImage);
  costFunction.SetMovingImage(movingImage);
  costFunction.SetMaskImage(movingImage);
  costFunction.SetRange( region );
  costFunction.SetParametric(surface);
  costFunction.SetUseMask(/* useMask: */ 0);
  vnl_vector<double> params(nElements);
  costFunction.GetVNLParametersFromParametric(params);
  costFunction.SetAlpha(alpha);
  costFunction.Init();
  costFunction.SetVerbose(10);
  
  GradientDescent optimizer;
  optimizer.SetTolerance(tolerance);
  optimizer.SetStepSize(step);
  optimizer.Minimize(costFunction, params);
  optimizer.SetVerbose(10);

  ImagePointer output = costFunction.GetTransformedImage();

  try 
    { 
    collector.Start( "Registration" );
    output->Update();
    collector.Stop( "Registration" );
    } 
  catch( itk::ExceptionObject & err ) 
    { 
    std::cerr << "ExceptionObject caught !" << std::endl; 
    std::cerr << err << std::endl; 
    return EXIT_FAILURE;
    } 
  
  // Resample to the original coordinate frame (not the reoriented
  // axial coordinate frame) of the fixed image
  //
  if (ResampledImageFileName != "")
    {
    
    typedef itk::ImageFileWriter< Image >  WriterType;
    typename WriterType::Pointer      writer =  WriterType::New();
    writer->SetFileName( ResampledImageFileName.c_str() );
    writer->SetInput( output );

    try
      {
      collector.Start( "Write resampled volume" );
      writer->Update();
      collector.Stop( "Write resampled volume" );
      }
    catch( itk::ExceptionObject & err ) 
      { 
      std::cerr << "ExceptionObject caught !" << std::endl; 
      std::cerr << err << std::endl; 
      return EXIT_FAILURE;
      }
    }

  // Report the time taken by the registration
  collector.Report();

  return EXIT_SUCCESS;
}

int main( int argc, char * argv[] )
{
  
  // Print out the arguments (need to add --echo to the argument list 
  // 
  std::vector<char *> vargs;
  for (int vi=0; vi < argc; ++vi) vargs.push_back(argv[vi]);
  vargs.push_back("--echo");
  
  argc = vargs.size();
  argv = &(vargs[0]);

  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    itk::GetImageType (FixedImageFileName, pixelType, componentType);

    // This filter handles all types
    
    switch (componentType)
      {
      case itk::ImageIOBase::CHAR:
      case itk::ImageIOBase::UCHAR:
      case itk::ImageIOBase::USHORT:
      case itk::ImageIOBase::SHORT:
        return DoIt( argc, argv, static_cast<short>(0));
        break;
      case itk::ImageIOBase::ULONG:
      case itk::ImageIOBase::LONG:
      case itk::ImageIOBase::UINT:
      case itk::ImageIOBase::INT:
      case itk::ImageIOBase::DOUBLE:
      case itk::ImageIOBase::FLOAT:
        return DoIt( argc, argv, static_cast<float>(0));
        break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "unknown component type" << std::endl;
        break;
      }
    }
  catch( itk::ExceptionObject &excep)
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
