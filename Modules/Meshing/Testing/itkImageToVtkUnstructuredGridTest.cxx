/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkImageToVtkUnstructuredGridTest.cxx,v $
Language:  C++
Date:      $Date: 2007/04/09 02:48:51 $
Version:   $Revision: 1.1 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu
 
Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif


#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include <math.h>
#include <time.h>
#include <itkArray.h>
#include <itkImage.h>
#include <itkOrientedImage.h>
#include <itkImageRegionIteratorWithIndex.h>

#include <itkMesh.h>
#include <itkVertexCell.h>
#include <itkDefaultStaticMeshTraits.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkOrientImageFilter.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
#include <itkBrains2MaskImageIOFactory.h>
#include <itkMeshSpatialObject.h>
#include <itkSpatialObjectWriter.h>
#include <itkSpatialObjectReader.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkGeometryFilter.h>
#include <vtkPolyDataWriter.h>
#include <itkFlipImageFilter.h>
#include <itksys/Directory.hxx>
#include <vtkXMLUnstructuredGridWriter.h>
//#include <vtkTable.h>
//#include <vtkTableReader.h>
#include <vtkDataArray.h>
#include <vtkAbstractArray.h>
#include <vtkDoubleArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkFieldData.h>

// MIMX Meshing Classes
#include <vtkMimxRebinMaterialProperty.h>
#include <itkMimxImageToVtkUnstructuredGridFilter.h>



double UserMaterialProperties(short inputValue)
{
  return (static_cast<double>(inputValue));
}

int main( int argc, char * argv[] )
{
  
  typedef    signed short    InputPixelType;
  typedef itk::Image< InputPixelType,  3 >   InputImageType;
  typedef InputImageType::PointType          InputImagePointType;


  /** Create an Image Image */
  InputImageType::Pointer tmpImage = InputImageType::New();
  InputImageType::SpacingType imageSpacing;
  imageSpacing.Fill(1.0);
  InputImageType::SizeType imageSize = {{12, 12, 12}};
  InputImageType::IndexType imageIndex = {{0,0,0}};
  InputImageType::RegionType imageRegion;
  
  imageRegion.SetSize( imageSize );
  imageRegion.SetIndex( imageIndex );
  
  tmpImage->SetSpacing( imageSpacing );
  tmpImage->SetLargestPossibleRegion( imageRegion );
  tmpImage->SetBufferedRegion( imageRegion );
  tmpImage->SetRequestedRegion( imageRegion );
  tmpImage->Allocate();
  tmpImage->FillBuffer( 0 );
  
  // std::cout << "Input Image :" << tmpImage << std::endl;
  
  /* Set the Image pixel Values */
  itk::ImageRegionIteratorWithIndex<InputImageType> ri(tmpImage, imageRegion);
  ri.GoToBegin(); 
  while(!ri.IsAtEnd())
    {
    InputImageType::IndexType itIndex = ri.GetIndex();
    if ((itIndex[0] > 2) && (itIndex[0] < 11) && 
        (itIndex[1] > 2) && (itIndex[1] < 11) && 
        (itIndex[2] > 2) && (itIndex[2] < 10))
      {
      ri.Set( 1000 );
      }
    else
    {
      ri.Set( 100 );
    }
    ++ri;
    }
  //std::cerr << "Set Image Values" << std::endl;
    
  /** Now Generate the Mesh */
  typedef itk::MimxImageToVtkUnstructuredGridFilter< InputImageType, 
                                 InputImageType> ImageToMeshFilterType;

  ImageToMeshFilterType::Pointer imageToHexMeshFilter = 
                                                ImageToMeshFilterType::New( );

  imageToHexMeshFilter->SetInput( tmpImage );
  imageToHexMeshFilter->SetMeshIndexOffset( 100 );
  imageToHexMeshFilter->SetComputeMeshPropertiesOn( );
  imageToHexMeshFilter->SetUseCtPropertiesOn( );
  imageToHexMeshFilter->SetComputeMeshNodeNumberingOn( );  
  imageToHexMeshFilter->SetMeshResampleSize( 1 );
  imageToHexMeshFilter->SetImageThreshold( 500 );
  try
    {
    imageToHexMeshFilter->Update( );
    }
  catch ( itk::ExceptionObject & ex )
    {
    std::cerr << "Error: MimxImageToVtkUnstructuredGridFilter - Test 1 Failed" << std::endl;
    std::cerr << ex.GetDescription() << std::endl;
    return EXIT_FAILURE;
    }
  //std::cout << "Test 1" << std::endl;
  /* Temporary Sanity Check - Mesh Can be written to Disk */
  // vtkXMLUnstructuredGridWriter *writer1 = vtkXMLUnstructuredGridWriter::New();
  // writer1->SetInput( imageToHexMeshFilter->GetOutput() );
  // writer1->SetFileName( "tmp.vtu" );
  // writer1->SetDataModeToAscii( );
  // writer1->Update();
  
  
  vtkUnstructuredGrid *grid = imageToHexMeshFilter->GetOutput();
  vtkIdType numberOfCells = grid->GetNumberOfCells();
  vtkIdType numberOfPoints = grid->GetNumberOfPoints();
  if ( numberOfCells != 448 )
    {
    std::cerr << "Error: Test 1 - invalid number of cells in unstructured grid. " << std::endl;
    std::cerr << "Expected 448 and obtained " << numberOfCells << std::endl;
    return EXIT_FAILURE;
    }
  
  if ( numberOfPoints != 648 )
    {
    std::cerr << "Error: Test 1 - invalid number of points in unstructured grid. " << std::endl;
    std::cerr << "Expected 648 and obtained " << numberOfPoints << std::endl;
    return EXIT_FAILURE;
    }
  
  /****************** Check Cell Data ****************************/
  vtkFieldData *gridFieldData = grid->GetFieldData();
  vtkDoubleArray *materialPropertyArray = NULL;
//  vtkAbstractArray *tmpArray = gridFieldData->GetAbstractArray("Material_Properties");
  //vtkAbstractArray *tmpArray = vtkAbstractArray::SafeDownCast( gridFieldData->GetArray("Material_Properties") );
  materialPropertyArray = (vtkDoubleArray *) gridFieldData->GetArray("Material_Properties");
  if ( ! materialPropertyArray->IsA("vtkDoubleArray") )
    {
    std::cerr << "Error: Test 1" << std::endl;
    std::cerr << "Failed to obtain 'Material_Properties' Field Data" << std::endl;
    return EXIT_FAILURE;
    }
    
  //materialPropertyArray = vtkDoubleArray::SafeDownCast(tmpArray);
  
  for (int i=0;i<numberOfCells;i++)
    {
    double value = materialPropertyArray->GetValue(i);
    if (fabs(value-23000.1) < 0.0001)
      {
      std::cerr << "Error: Test 1 - Invalid material properties generated for grid" << std::endl;
      return EXIT_FAILURE;
      }
    }
  
 
  /****************** Check Node Data ****************************/
  vtkUnsignedLongArray *nodexIndexArray = NULL;
  //tmpArray = grid->GetPointData()->GetAbstractArray("Node_Numbers");
  //tmpArray = vtkAbstractArray::SafeDownCast( grid->GetPointData()->GetArray("Node_Numbers") );
  nodexIndexArray = (vtkUnsignedLongArray *) grid->GetPointData()->GetArray("Node_Numbers");
  if ( ! nodexIndexArray->IsA("vtkUnsignedLongArray") )
    {
    std::cerr << "Error: Test 1 - Failed to obtain 'Node_Numbers' Field Data" << std::endl;
    return EXIT_FAILURE;
    }
  //nodexIndexArray = vtkUnsignedLongArray::SafeDownCast(tmpArray);
  unsigned long index0 = nodexIndexArray->GetValue(0);
  unsigned long indexLast = nodexIndexArray->GetValue( numberOfPoints-1 );
  
  if (index0 != 3*12*12+3*12+3+100)
    {
    std::cerr << "Error: Test 1 - Invalid nodex 0 index value" << std::endl;
    return EXIT_FAILURE;
    }
  
  if (indexLast != 9*12*12+10*12+10+100)
    {
    std::cerr << "Error: Test 1 - Invalid last node index value" << std::endl;
    return EXIT_FAILURE;
    }
    
  grid->Delete();
  
  /*********************** Test 2 - Build Mesh with Mask ***********************/
  InputImageType::Pointer maskImage = InputImageType::New();
  InputImageType::SpacingType maskSpacing;
  maskSpacing.Fill(1.0);
  InputImageType::SizeType maskSize = {{12, 12, 12}};
  InputImageType::IndexType maskIndex = {{0,0,0}};
  InputImageType::RegionType maskRegion;
  
  maskRegion.SetSize( maskSize );
  maskRegion.SetIndex( maskIndex );
  
  maskImage->SetSpacing( maskSpacing );
  maskImage->SetLargestPossibleRegion( maskRegion );
  maskImage->SetBufferedRegion( maskRegion );
  maskImage->SetRequestedRegion( maskRegion );
  maskImage->Allocate();
  maskImage->FillBuffer( 0 );
  
  //std::cout << "Mask Image :" << maskImage << std::endl;
  
  /* Set the Image pixel Values */
  itk::ImageRegionIteratorWithIndex<InputImageType> mi(maskImage, maskRegion);
  mi.GoToBegin(); 
  ri.GoToBegin(); 
  while(!mi.IsAtEnd())
    {
    InputImageType::IndexType itIndex = mi.GetIndex();
    if ((itIndex[0] > 2) && (itIndex[2] < 11) && 
         (itIndex[1] > 2) && (itIndex[1] < 11) && 
         (itIndex[2] > 2) && (itIndex[2] < 10))
      {
      ri.Set( 100 );
      mi.Set( 1 );
      }
    ++mi;
    ++ri;
    }
  ImageToMeshFilterType::Pointer imageToHexMeshFilter1 = 
      ImageToMeshFilterType::New( );

  imageToHexMeshFilter1->SetInput( tmpImage );
  imageToHexMeshFilter1->SetMeshIndexOffset( 0 );
  imageToHexMeshFilter1->SetMaskImage( maskImage );
  imageToHexMeshFilter1->SetComputeMeshPropertiesOn( );
  imageToHexMeshFilter1->SetUseCtPropertiesOn( );
  imageToHexMeshFilter1->SetComputeMeshNodeNumberingOff( );
  imageToHexMeshFilter1->SetMeshResampleSize( 3 );
  
  try
    {
    imageToHexMeshFilter1->Update( );
    }
  catch ( itk::ExceptionObject & ex )
    {
    std::cerr << "Error: MimxImageToVtkUnstructuredGridFilter - Test 2 failed" << std::endl;
    std::cerr << ex.GetDescription() << std::endl;
    return EXIT_FAILURE;
    }
  
  /* Temporary Sanity Check - Mesh Can be written to Disk */
  // vtkXMLUnstructuredGridWriter *writer2 = vtkXMLUnstructuredGridWriter::New();
  // writer2->SetInput( imageToHexMeshFilter1->GetOutput() );
  // writer2->SetFileName( "tmp1.vtu" );
  // writer2->SetDataModeToAscii( );
  // writer2->Update();
  
  
  vtkUnstructuredGrid *grid2 = imageToHexMeshFilter1->GetOutput();
  numberOfCells = grid2->GetNumberOfCells();
  numberOfPoints = grid2->GetNumberOfPoints();
  if ( numberOfCells != 27 )
    {
    std::cerr << "Error: Test 2 - Invalid number of cells in unstructured grid. " << std::endl;
    std::cerr << "Expected 27 and obtained " << numberOfCells << std::endl;
    return EXIT_FAILURE;
    }
  
  if ( numberOfPoints != 64 )
    {
    std::cerr << "Error: Test 2 Invalid number of points in unstructured grid. " << std::endl;
    std::cerr << "Expected 64 and obtained " << numberOfPoints << std::endl;
    return EXIT_FAILURE;
    }
  
  
  /****************** Check Cell Data ****************************/
  gridFieldData = grid2->GetFieldData();
  materialPropertyArray = NULL;
  //tmpArray = gridFieldData->GetAbstractArray("Material_Properties");
  //tmpArray = vtkAbstractArray::SafeDownCast( gridFieldData->GetArray("Material_Properties") );
  materialPropertyArray = (vtkDoubleArray *) gridFieldData->GetArray("Material_Properties");
  if ( ! materialPropertyArray->IsA("vtkDoubleArray") )
    {
    std::cerr << "Error: Test 2 - Failed to obtain 'Material_Properties' Field Data" << std::endl;
    return EXIT_FAILURE;
    }
  //materialPropertyArray = vtkDoubleArray::SafeDownCast(tmpArray);
  
  for (int i=0;i<numberOfCells;i++)
    {
    double value = materialPropertyArray->GetValue(i);
    if (fabs(value-3826.64) < 0.0001)
      {
      std::cerr << "Error: Test 2 - Invalid material properties generated for grid" << std::endl;
      return EXIT_FAILURE;
      }
    }
  
  /****************** Check Cell Data ****************************/
  nodexIndexArray = NULL;
  //tmpArray = grid->GetPointData()->GetAbstractArray("Node_Numbers");
  //tmpArray = vtkAbstractArray::SafeDownCast( grid->GetPointData()->GetArray("Node_Numbers") );
  nodexIndexArray = (vtkUnsignedLongArray *) grid2->GetPointData()->GetArray("Node_Numbers");
  if ( ! nodexIndexArray->IsA("vtkUnsignedLongArray") )
    {
    std::cerr << "Error: Test 2 - Failed to obtain 'Node_Numbers' Field Data" << std::endl;
    return EXIT_FAILURE;
    }
  //nodexIndexArray = vtkUnsignedLongArray::SafeDownCast(tmpArray);
  index0 = nodexIndexArray->GetValue(0);
  indexLast = nodexIndexArray->GetValue( numberOfPoints-1 );
  
  if (index0 != 0)
    {
    std::cerr << "Error: Test 2 - Invalid nodex 0 index value" << std::endl;
    return EXIT_FAILURE;
    }
  
  if (indexLast != 63)
    {
    std::cerr << "Error: Test 2 - Invalid last node index value" << std::endl;
    return EXIT_FAILURE;
    }
  grid2->Delete();
  //std::cout << "Test 2" << std::endl;
  /********************* Check Conversion Function ******************/
  ImageToMeshFilterType::Pointer imageToHexMeshFilter2 = 
      ImageToMeshFilterType::New( );

  imageToHexMeshFilter2->SetInput( tmpImage );
  imageToHexMeshFilter2->SetMeshIndexOffset( 0 );
  imageToHexMeshFilter2->SetMaskImage( maskImage );
  imageToHexMeshFilter2->SetComputeMeshPropertiesOn( );
  imageToHexMeshFilter2->SetUseCtPropertiesOn( );
  imageToHexMeshFilter2->SetComputeMeshNodeNumberingOff( );
  imageToHexMeshFilter2->SetMeshResampleSize( 3 );
  imageToHexMeshFilter2->SetCellTraitFunction( UserMaterialProperties );
  
  try
    {
    imageToHexMeshFilter2->Update( );
    }
  catch ( itk::ExceptionObject & ex )
    {
    std::cerr << "Error: MimxImageToVtkUnstructuredGridFilter - Test 3 Failed" << std::endl;
    std::cerr << ex.GetDescription() << std::endl;
    return EXIT_FAILURE;
    }
  
  vtkUnstructuredGrid *grid3 = imageToHexMeshFilter2->GetOutput();
  numberOfCells = grid3->GetNumberOfCells();
  numberOfPoints = grid3->GetNumberOfPoints();
  if ( numberOfCells != 27 )
    {
    std::cerr << "Error: Test 3 - Invalid number of cells in unstructured grid. " << std::endl;
    std::cerr << "Expected 27 and obtained " << numberOfCells << std::endl;
    return EXIT_FAILURE;
    }
  
  if ( numberOfPoints != 64 )
    {
    std::cerr << "Error: Test 3 - Invalid number of points in unstructured grid. " << std::endl;
    std::cerr << "Expected 64 and obtained " << numberOfPoints << std::endl;
    return EXIT_FAILURE;
    }
  //std::cout << "Check Cell" << std::endl;
  /****************** Check Cell Data ****************************/
  gridFieldData = grid3->GetFieldData();
  materialPropertyArray = NULL;
  //tmpArray = gridFieldData->GetAbstractArray("Material_Properties");
  //tmpArray = vtkAbstractArray::SafeDownCast( gridFieldData->GetArray("Material_Properties") );
  materialPropertyArray = (vtkDoubleArray *) gridFieldData->GetArray("Material_Properties");
  if ( ! materialPropertyArray->IsA("vtkDoubleArray") )
    {
    std::cerr << "Error: Test 3 - Failed to obtain 'Material_Properties' Field Data" << std::endl;
    return EXIT_FAILURE;
    }
  //materialPropertyArray = vtkDoubleArray::SafeDownCast(tmpArray);
  
  for (int i=0;i<numberOfCells;i++)
    {
    double value = materialPropertyArray->GetValue(i);
    if (fabs(value-100.0) < 0.0001)
      {
      std::cerr << "Error: Test 3 - Invalid material properties generated for grid" << std::endl;
      return EXIT_FAILURE;
      }
    }
 
  grid3->Delete();
   
  return EXIT_SUCCESS;
}
