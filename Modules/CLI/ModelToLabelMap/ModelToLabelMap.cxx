/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Modules/CLI/OrientImage.cxx $
  Language:  C++
  Date:      $Date: 2007-12-20 18:30:38 -0500 (Thu, 20 Dec 2007) $
  Version:   $Revision: 5310 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// ModelToLabelMap includes
#include "ModelToLabelMapCLP.h"

// ITK includes
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryThresholdImageFunction.h"
#include "itkFloodFilledImageFunctionConditionalIterator.h"
#include "itkImageFileWriter.h"
#include "itkPluginUtilities.h"
#include <itksys/SystemTools.hxx>

// VTK includes
#include <vtkDebugLeaks.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataPointSampler.h>
#include <vtkPolyDataReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkVersion.h>

typedef itk::Image<unsigned char, 3> LabelImageType;

LabelImageType::Pointer BinaryErodeFilter3D( LabelImageType::Pointer & img, unsigned int ballsize )
{
  typedef itk::BinaryBallStructuringElement<unsigned char, 3>                     KernalType;
  typedef itk::BinaryErodeImageFilter<LabelImageType, LabelImageType, KernalType> ErodeFilterType;
  ErodeFilterType::Pointer erodeFilter = ErodeFilterType::New();
  erodeFilter->SetInput( img );

  KernalType           ball;
  KernalType::SizeType ballSize;
  for( int k = 0; k < 3; k++ )
    {
    ballSize[k] = ballsize;
    }
  ball.SetRadius(ballSize);
  ball.CreateStructuringElement();
  erodeFilter->SetKernel( ball );
  erodeFilter->Update();
  return erodeFilter->GetOutput();
}

LabelImageType::Pointer BinaryDilateFilter3D( LabelImageType::Pointer & img, unsigned int ballsize )
{
  typedef itk::BinaryBallStructuringElement<unsigned char, 3>                      KernalType;
  typedef itk::BinaryDilateImageFilter<LabelImageType, LabelImageType, KernalType> DilateFilterType;
  DilateFilterType::Pointer dilateFilter = DilateFilterType::New();
  dilateFilter->SetInput( img );
  KernalType           ball;
  KernalType::SizeType ballSize;
  for( int k = 0; k < 3; k++ )
    {
    ballSize[k] = ballsize;
    }
  ball.SetRadius(ballSize);
  ball.CreateStructuringElement();
  dilateFilter->SetKernel( ball );
  dilateFilter->Update();
  return dilateFilter->GetOutput();
}

LabelImageType::Pointer BinaryOpeningFilter3D( LabelImageType::Pointer & img, unsigned int ballsize )
{
  LabelImageType::Pointer imgErode = BinaryErodeFilter3D( img, ballsize);

  return BinaryDilateFilter3D( imgErode, ballsize );
}

LabelImageType::Pointer BinaryClosingFilter3D( LabelImageType::Pointer & img, unsigned int ballsize )
{
  LabelImageType::Pointer imgDilate = BinaryDilateFilter3D( img, ballsize );

  return BinaryErodeFilter3D( imgDilate, ballsize );
}

//
// Description: A templated procedure to execute the algorithm
template <class T>
int DoIt( int argc, char * argv[])
{

  PARSE_ARGS;
  vtkDebugLeaks::SetExitError(true);

  typedef    T InputPixelType;

  typedef itk::Image<InputPixelType,  3> InputImageType;

  typedef itk::ImageFileReader<InputImageType> ReaderType;
  typedef itk::ImageFileWriter<LabelImageType> WriterType;

  // Read the input volume
  typename ReaderType::Pointer reader = ReaderType::New();
  itk::PluginFilterWatcher watchReader(reader, "Read Input Volume",
                                       CLPProcessInformation);
  reader->SetFileName( InputVolume.c_str() );
  reader->Update();

  // output label map
  LabelImageType::Pointer label = LabelImageType::New();
  label->CopyInformation( reader->GetOutput() );
  label->SetRegions( label->GetLargestPossibleRegion() );
  label->Allocate();
  label->FillBuffer( 0 );

  // read the poly data
  vtkSmartPointer<vtkPolyData> polyData;
  vtkSmartPointer<vtkPolyDataReader> pdReader;
  vtkSmartPointer<vtkXMLPolyDataReader> pdxReader;

  // do we have vtk or vtp models?
  std::string extension = itksys::SystemTools::LowerCase( itksys::SystemTools::GetFilenameLastExtension(surface) );
  if( extension.empty() )
    {
    std::cerr << "Failed to find an extension for " << surface << std::endl;
    return EXIT_FAILURE;
    }

  if( extension == std::string(".vtk") )
    {
    pdReader = vtkSmartPointer<vtkPolyDataReader>::New();
    pdReader->SetFileName(surface.c_str() );
    pdReader->Update();
    polyData = pdReader->GetOutput();
    }
  else if( extension == std::string(".vtp") )
    {
    pdxReader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
    pdxReader->SetFileName(surface.c_str() );
    pdxReader->Update();
    polyData = pdxReader->GetOutput();
    }
  if( polyData == NULL )
    {
    std::cerr << "Failed to read surface " << surface << std::endl;
    return EXIT_FAILURE;
    }

  // LPS vs RAS

  vtkPoints * allPoints = polyData->GetPoints();
  for( int k = 0; k < allPoints->GetNumberOfPoints(); k++ )
    {
    double* point = polyData->GetPoint( k );
    point[0] = -point[0];
    point[1] = -point[1];
    allPoints->SetPoint( k, point[0], point[1], point[2] );
    }

  // do it
  vtkNew<vtkPolyDataPointSampler> sampler;

  sampler->SetInputData( polyData );
  sampler->SetDistance( sampleDistance );
  sampler->GenerateEdgePointsOn();
  sampler->GenerateInteriorPointsOn();
  sampler->GenerateVertexPointsOn();
  sampler->Update();

  std::cout << polyData->GetNumberOfPoints() << std::endl;
  std::cout << sampler->GetOutput()->GetNumberOfPoints() << std::endl;
  for( int k = 0; k < sampler->GetOutput()->GetNumberOfPoints(); k++ )
    {
    double *                  pt = sampler->GetOutput()->GetPoint( k );
    LabelImageType::PointType pitk;
    pitk[0] = pt[0];
    pitk[1] = pt[1];
    pitk[2] = pt[2];
    LabelImageType::IndexType idx;
    label->TransformPhysicalPointToIndex( pitk, idx );

    if( label->GetLargestPossibleRegion().IsInside(idx) )
      {
      label->SetPixel( idx, labelValue );
      }
    }

  // do morphological closing
  LabelImageType::Pointer                           closedLabel = BinaryClosingFilter3D( label, 2);
  itk::ImageRegionIteratorWithIndex<LabelImageType> itLabel(closedLabel, closedLabel->GetLargestPossibleRegion() );

  // do flood fill using binary threshold image function
  typedef itk::BinaryThresholdImageFunction<LabelImageType> ImageFunctionType;
  ImageFunctionType::Pointer func = ImageFunctionType::New();
  func->SetInputImage( closedLabel );
  func->ThresholdBelow(1);

  LabelImageType::IndexType idx;
  LabelImageType::PointType COG;

  // set the centre of gravity
  // double *bounds = polyData->GetBounds();
  COG.Fill(0.0);
  for( vtkIdType k = 0; k < polyData->GetNumberOfPoints(); k++ )
    {
    double *pt = polyData->GetPoint( k );
    for( int m = 0; m < 3; m++ )
      {
      COG[m] += pt[m];
      }
    }
  for( int m = 0; m < 3; m++ )
    {
    COG[m] /= static_cast<float>( polyData->GetNumberOfPoints() );
    }

  label->TransformPhysicalPointToIndex( COG, idx );

  itk::FloodFilledImageFunctionConditionalIterator<LabelImageType, ImageFunctionType> floodFill( closedLabel, func, idx );
  for( floodFill.GoToBegin(); !floodFill.IsAtEnd(); ++floodFill )
    {
    LabelImageType::IndexType i = floodFill.GetIndex();
    closedLabel->SetPixel( i, labelValue );
    }
  LabelImageType::Pointer finalLabel = BinaryClosingFilter3D( closedLabel, 2);
  for( itLabel.GoToBegin(); !itLabel.IsAtEnd(); ++itLabel )
    {
    LabelImageType::IndexType i = itLabel.GetIndex();
    label->SetPixel( i, finalLabel->GetPixel(i) );
    }

  typename WriterType::Pointer writer = WriterType::New();
  itk::PluginFilterWatcher watchWriter(writer,
                                       "Write Volume",
                                       CLPProcessInformation);
  writer->SetFileName( OutputVolume.c_str() );
  writer->SetInput( label );
  writer->SetUseCompression(1);
  writer->Update();

  return EXIT_SUCCESS;
}

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    itk::GetImageType(InputVolume, pixelType, componentType);

    // This filter handles all types on input, but only produces
    // signed types

    switch( componentType )
      {
      case itk::ImageIOBase::UCHAR:
      case itk::ImageIOBase::CHAR:
        return DoIt<char>( argc, argv );
        break;
      case itk::ImageIOBase::USHORT:
      case itk::ImageIOBase::SHORT:
        return DoIt<short>( argc, argv );
        break;
      case itk::ImageIOBase::UINT:
      case itk::ImageIOBase::INT:
        return DoIt<int>( argc, argv );
        break;
      case itk::ImageIOBase::ULONG:
      case itk::ImageIOBase::LONG:
        return DoIt<long>( argc, argv );
        break;
      case itk::ImageIOBase::FLOAT:
        return DoIt<float>( argc, argv );
        break;
      case itk::ImageIOBase::DOUBLE:
        return DoIt<double>( argc, argv );
        break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "unknown component type" << std::endl;
        break;
      }
    }
  catch( itk::ExceptionObject & excep )
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
