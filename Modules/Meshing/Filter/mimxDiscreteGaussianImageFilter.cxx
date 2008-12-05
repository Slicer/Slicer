/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: mimxDiscreteGaussianImageFilter.cxx,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
Version:   $Revision: 1.2 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif
//  Software Guide : BeginCommandLineArgs
#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif


#include "itkImage.h"
#include "itkMesh.h"
#include "itkVertexCell.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkSpatialOrientation.h"
#include "itkOrientImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkImageToHexahedronMeshSource.h"
#include <itkBrains2MaskImageIO.h>
#include <itkBrains2MaskImageIOFactory.h>
#include <itkMeshSpatialObject.h>
#include "itkSpatialObjectWriter.h"
#include "itkSpatialObjectReader.h"
#include "itkAbaqusMeshFileWriter.h"


int main( int argc, char * argv[] )
{
   itk::Brains2MaskImageIOFactory::RegisterOneFactory();

   if( argc < 4 ) 
   { 
                std::cerr << "Usage: " << std::endl;
                std::cerr << argv[0] << "  inputImageFile  maskFile outputMeshFile " << std::endl;
                return EXIT_FAILURE;
    }

  typedef    signed short    InputPixelType;
  typedef    signed short    MaskPixelType;
  typedef    signed short    OutputPixelType;

  typedef itk::Image< InputPixelType,  3 >   InputImageType;
  typedef itk::Image< MaskPixelType,   3 >   MaskImageType;
  typedef itk::Image< OutputPixelType, 3 >   OutputImageType;

  typedef itk::ImageFileReader< InputImageType >  ReaderType;


 // Orients the Given Image into Coronal View

  ReaderType::Pointer imageReader = ReaderType::New();
  imageReader->SetFileName( argv[1] );
  InputImageType::Pointer image = imageReader->GetOutput();

  std::cout << "\t Read Image 1" << std::endl;

  itk::SpatialOrientation::ValidCoordinateOrientationFlags imageOrientation;
  itk::ExposeMetaData<itk::SpatialOrientation::ValidCoordinateOrientationFlags> 
  (image->GetMetaDataDictionary(),itk::ITK_CoordinateOrientation, imageOrientation);
  
  typedef itk::OrientImageFilter<InputImageType,InputImageType> OrientFilterType;
  OrientFilterType::Pointer OrientImageFilter = OrientFilterType::New();
  OrientImageFilter ->SetGivenCoordinateOrientation(imageOrientation);
  OrientImageFilter ->SetDesiredCoordinateOrientation( itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIP  );
  OrientImageFilter->SetInput(imageReader->GetOutput());
  OrientImageFilter->Update();
  InputImageType::Pointer orientImage = OrientImageFilter->GetOutput();

// Orients the Given Mask into Coronal View

  ReaderType::Pointer maskReader = ReaderType::New();
  maskReader->SetFileName( argv[2] );
  InputImageType::Pointer mask = maskReader->GetOutput();

  std::cout << "\t Read Mask" << std::endl;

  itk::SpatialOrientation::ValidCoordinateOrientationFlags maskOrientation;
  itk::ExposeMetaData<itk::SpatialOrientation::ValidCoordinateOrientationFlags>
  (mask->GetMetaDataDictionary(),itk::ITK_CoordinateOrientation, maskOrientation);

  OrientFilterType::Pointer OrientMaskFilter = OrientFilterType::New();
  OrientMaskFilter ->SetGivenCoordinateOrientation(maskOrientation);
  OrientMaskFilter ->SetDesiredCoordinateOrientation( itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIP   );
  OrientMaskFilter->SetInput(maskReader->GetOutput());
  OrientMaskFilter->Update();
  InputImageType::Pointer orientMask = OrientMaskFilter->GetOutput();


  typedef itk::MaskImageFilter<
                           InputImageType,
                           InputImageType,
                           InputImageType > myFilterType;
            

  // Create a mask  Filter                                
  myFilterType::Pointer filter = myFilterType::New();


  // Connect the input images
  filter->SetInput1( orientImage  ); 
  filter->SetInput2( orientMask  );
  filter->SetOutsideValue( 0 );
  filter->Update(  );

  std::cout << "\t Mask image" << std::endl;
 
  typedef itk::Mesh< double > MeshType;  
  typedef itk::ImageToHexahedronMeshSource<InputImageType, 
                                                                                        MeshType> ImageToMeshSourceType;
  
  ImageToMeshSourceType::Pointer meshSource = ImageToMeshSourceType::New();

  meshSource->SetInput( filter->GetOutput() );
  meshSource->Update( );
  std::cout << "\t Create Mesh" << std::endl;
  MeshType::Pointer myMesh = meshSource->GetOutput( );

  typedef itk::AbaqusMeshFileWriter< MeshType > MeshWriterType;  

  MeshWriterType::Pointer AbqWriter = MeshWriterType::New();
  AbqWriter->SetFileName("E:\\Ritesh\\MyPrograms\\TestProgram\\test.inp");
  AbqWriter->SetInput(myMesh);
  AbqWriter->Update();


//  std::cout << "\t Hex Mesh: " << myMesh << std::endl;
  //std::cout << "\t # Mesh Cells: " << myMesh->GetNumberOfCells() << std::endl;

/*
  typedef itk::MeshSpatialObject< MeshType > MeshSpatialObjectType;  
  
  MeshSpatialObjectType::Pointer meshSpatialObject = MeshSpatialObjectType::New();
  meshSpatialObject->SetMesh( myMesh );
  meshSpatialObject->ComputeLocalBoundingBox();
  //std::cout << "Set Mesh: " << meshSpatialObject << std::endl;
  
  typedef itk::SpatialObjectWriter<3> WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput( meshSpatialObject );
  writer->SetFileName( argv[3] );
  try
  {
          printf ("Before writer update\n");
    writer->Update( );
          printf ("Vivek1\n");
  }

  catch (itk::ExceptionObject &ex)
  {
          printf ("After writer update\n");
          std::cout << ex << std::endl;
          throw;
  }
  
  */
//  std::cout << "\t Write Spatial Object: " << argv[3] << std::endl;

  return EXIT_SUCCESS;

 }
