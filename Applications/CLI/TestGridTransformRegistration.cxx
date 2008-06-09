/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $HeadURL: http://www.na-mic.org/svn/Slicer3/trunk/Applications/CLI/BSplineDeformableRegistration.cxx $
  Language:  C++
  Date:      $Date: 2008-04-01 15:25:23 -0400 (Tue, 01 Apr 2008) $
  Version:   $Revision: 6348 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "TestGridTransformRegistrationCLP.h"


#include "itkCommand.h"

#include "itkImageRegistrationMethod.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkLBFGSBOptimizer.h"

#include "itkOrientedImage.h"
#include "itkOrientImageFilter.h"
#include "itkResampleImageFilter.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTransformFileReader.h"
#include "itkTransformFileWriter.h"

#include "itkPluginUtilities.h"

#include "itkTimeProbesCollectorBase.h"

#include <algorithm>


template<class T> int DoIt( int argc, char * argv[], T )
{
  PARSE_ARGS;

  // typedefs
  const    unsigned int  ImageDimension = 3;
  typedef  T  PixelType;
  typedef  T  OutputPixelType;
  typedef itk::OrientedImage< PixelType, ImageDimension >       InputImageType;
  typedef itk::OrientedImage< OutputPixelType, ImageDimension > OutputImageType;

  typedef itk::ImageFileReader< InputImageType > FixedImageReaderType;
  typedef itk::ImageFileReader< InputImageType > MovingImageReaderType;
  typedef itk::ImageFileWriter< OutputImageType >  WriterType;

  typedef itk::OrientImageFilter<InputImageType,InputImageType> OrientFilterType;
  typedef itk::ResampleImageFilter< 
                            InputImageType, 
                            OutputImageType >    ResampleFilterType;



  typedef itk::VectorImage< double, ImageDimension >  GridType;

  GridType::Pointer grid = GridType::New();

  // Set up the grid volume
  {
    grid->SetVectorLength( ImageDimension );

    GridType::IndexType start;

    start[0] = 0;
    start[1] = 0;
    start[2] = 0;

    GridType::SizeType  size;

    size[0]  = gridSize;
    size[1]  = gridSize;
    size[2]  = gridSize;

    GridType::RegionType region;
    region.SetSize( size );
    region.SetIndex( start );

    grid->SetRegions( region );
  }


  // Read fixed and moving images
  //
  //
  typename FixedImageReaderType::Pointer  fixedImageReader  = FixedImageReaderType::New();
  typename MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();

  fixedImageReader->SetFileName(  FixedImageFileName.c_str() );
  movingImageReader->SetFileName( MovingImageFileName.c_str() );


  // Add a time probe
  itk::TimeProbesCollectorBase collector;

  collector.Start( "Read fixed volume" );
  fixedImageReader->Update();
  collector.Stop( "Read fixed volume" );

  collector.Start( "Read moving volume" );
  movingImageReader->Update();
  collector.Stop( "Read moving volume" );


  collector.Start( "Constructing deformation field" );
  grid->SetOrigin( fixedImageReader->GetOutput()->GetOrigin() );

  // Set the spacing of the grid so that there are the gridSize grid
  // points span the image dimensions.
  GridType::SpacingType spacing = fixedImageReader->GetOutput()->GetSpacing();

  typename InputImageType::RegionType fixedRegion =
    fixedImageReader->GetOutput()->GetLargestPossibleRegion();
  typename InputImageType::SizeType fixedImageSize =
    fixedRegion.GetSize();

  for(unsigned int r=0; r<ImageDimension; r++)
    {
    spacing[r] *= floor( static_cast<double>(fixedImageSize[r] - 1)  / 
                  static_cast<double>(gridSize - 1) );
    }

  grid->SetSpacing( spacing );

  grid->Allocate();

  // Fill with zero
  GridType::PixelContainer* pixelContainer = grid->GetPixelContainer();
  std::fill( pixelContainer->GetBufferPointer(),
             pixelContainer->GetBufferPointer()+pixelContainer->Size(),
             0.0 );

  // Set some elements to non-zero
  {
    GridType::IndexType index;
    index[1] = gridSize/2;
    index[2] = gridSize/2;

    // Deform by 6 pixels in the x direction
    GridType::PixelType p( 3 );
    p[0] = 6;
    p[1] = 0;
    p[2] = 0;

    for( unsigned x = 0; x < gridSize; ++x )
      {
      index[0] = x;
      grid->SetPixel( index, p );
      }
  }

  if (OutputTransform != "")
    {
    typedef itk::ImageFileWriter< GridType > GridWriterType;

    GridWriterType::Pointer gridWriter = GridWriterType::New();
    gridWriter->SetFileName( OutputTransform );
    gridWriter->SetInput( grid );

    try
      {
      gridWriter->Update();
      }
    catch( itk::ExceptionObject & exp ) 
      {
      std::cerr << "Exception caught !" << std::endl;
      std::cerr << exp << std::endl;
      }
    }

  collector.Stop( "Constructing deformation field" );

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
