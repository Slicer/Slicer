/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxRebinMaterialPropertyTest.cxx,v $
Language:  C++
Date:      $Date: 2007/04/09 03:00:35 $
Version:   $Revision: 1.2 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx
 
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
#include <vtkAbstractArray.h>
#include <vtkDoubleArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkFieldData.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkDataObjectWriter.h>
#include <vtkDataObjectReader.h>
#include <vtkDataWriter.h>

// MIMX Meshing Classes
#include <vtkMimxRebinMaterialProperty.h>
#include <itkMimxImageToVtkUnstructuredGridFilter.h>


double matProp(signed short value)
{
  return static_cast<double>(value); 
}

int main( int argc, char * argv[] )
{
  
  typedef signed short                       InputPixelType;
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
      ri.Set( itIndex[2]*12*12+itIndex[1]*12+itIndex[0] );
      }
    else
    {
      ri.Set( 0 );
    }
    ++ri;
    }
  // std::cerr << "Set Image Values" << std::endl;
    
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
  imageToHexMeshFilter->SetImageThreshold( 100 );
  imageToHexMeshFilter->SetCellTraitFunction( &matProp );
  imageToHexMeshFilter->SetUseCtProperties( false );
  try
    {
    imageToHexMeshFilter->Update( );
    }
  catch ( itk::ExceptionObject & ex )
    {
    std::cerr << "Error: ImageToVtkUnstructuredGridFilter - Test Failed" << std::endl;
    std::cerr << ex.GetDescription() << std::endl;
    return EXIT_FAILURE;
    }
    
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
    std::cerr << "Error: Invalid number of cells in unstructured grid. " << std::endl;
    std::cerr << "Expected 448 and obtained " << numberOfCells << std::endl;
    return EXIT_FAILURE;
    }
  
  if ( numberOfPoints != 648 )
    {
    std::cerr << "Error: Invalid number of points in unstructured grid. " << std::endl;
    std::cerr << "Expected 648 and obtained " << numberOfPoints << std::endl;
    return EXIT_FAILURE;
    }
  
  vtkDoubleArray *histogramArray = vtkDoubleArray::New();
  histogramArray->SetName("Histogram");
  double stepSize = 12.0*12.0*12.0/15.0;
  for (int i=0;i<16;i++)
  {
    histogramArray->InsertValue( i, static_cast<double>(i)*stepSize );
  }
  //vtkTable *materialTable = vtkTable::New();
  //materialTable->AddColumn( histogramArray );
  vtkFieldData *materialTable = vtkFieldData::New();
  materialTable->AddArray( histogramArray );

  
  vtkMimxRebinMaterialProperty *rebinFilter = vtkMimxRebinMaterialProperty::New();
  rebinFilter->SetInput( grid );
  rebinFilter->SetNumberOfHistogramBins( 16 );
  rebinFilter->SetPropertyTable( materialTable );
  rebinFilter->SetGeneratePropertyBins( false );
  rebinFilter->Update();
  
  vtkUnstructuredGrid *rebinGrid = rebinFilter->GetOutput( );
  vtkUnstructuredGridWriter *tmpWriter = vtkUnstructuredGridWriter::New();
  tmpWriter->SetInput( rebinGrid );
  tmpWriter->SetFileName("tmp.vtk");
  tmpWriter->Update( );
  numberOfCells = rebinGrid->GetNumberOfCells();
  numberOfPoints = rebinGrid->GetNumberOfPoints();
  if ( numberOfCells != 448 )
  {
    std::cerr << "Error: Invalid number of cells in rebinned grid. " << std::endl;
    std::cerr << "Expected 448 and obtained " << numberOfCells << std::endl;
    return EXIT_FAILURE;
  }
  
  if ( numberOfPoints != 648 )
  {
    std::cerr << "Error: Invalid number of points in rebinned grid. " << std::endl;
    std::cerr << "Expected 648 and obtained " << numberOfPoints << std::endl;
    return EXIT_FAILURE;
  }

  /****************** Check Cell Data ****************************/
  vtkFieldData *gridFieldData = rebinGrid->GetFieldData();
  vtkFieldData *gridPointData = rebinGrid->GetPointData( );
  vtkDoubleArray *materialPropertyArray = NULL;
  //vtkAbstractArray *tmpArray = gridFieldData->GetAbstractArray("Material_Properties");
  //vtkAbstractArray *tmpArray = vtkAbstractArray::SafeDownCast( gridFieldData->GetArray("Material_Properties") );
  //tmpArray->Print(std::cerr);
  //if ( tmpArray == NULL )
  //  {
  //  std::cerr << "Error: Test 1" << std::endl;
  //  std::cerr << "Failed to obtain 'Material_Properties' Field Data" << std::endl;
  //  return EXIT_FAILURE;
  //  }
    
  materialPropertyArray = (vtkDoubleArray *) gridFieldData->GetArray("Material_Properties");
  if ( materialPropertyArray == NULL )
    {
    std::cerr << "Error: Test 1" << std::endl;
    std::cerr << "Failed to obtain 'Material_Properties' Field Data" << std::endl;
    return EXIT_FAILURE;
    }

  // Temporary Check of Array
  //for (int i=0;i<histogramArray->GetNumberOfTuples( );i++)
  //{
  //  std::cerr << "Mat Element " << i << " " << histogramArray->GetValue(i) << std::endl;
  //}
  for (int i=0;i<numberOfCells;i++)
    {
    bool found = false;
    double value = materialPropertyArray->GetValue(i);
    
    //std::cerr << "Element Property " << i << " " << value << std::endl;
    
    for (int j=0;j<16;j++)
      {
      if (fabs(value-histogramArray->GetValue(j) ) < 0.0001)
        {
          found = true;
        }
      }
    if (! found ) 
      {
      std::cerr << "Error: Test 1" << std::endl;
      std::cerr << "Failed to obtain correct material properties after rebinning" << std::endl;
      return EXIT_FAILURE;
      }
    }


  /****************** Check Node Data ****************************/
  vtkUnsignedLongArray *nodexIndexArray = NULL;
  //tmpArray = gridPointData->GetAbstractArray("Node_Numbers");
  //tmpArray = vtkAbstractArray::SafeDownCast( gridPointData->GetArray("Node_Numbers") );
  //if ( ! tmpArray->IsA("vtkUnsignedLongArray") )
  //  {
  //  std::cerr << "Error: Test 1 - Failed to obtain 'Node_Numbers' Field Data" << std::endl;
  //  return EXIT_FAILURE;
  //  }
  nodexIndexArray = (vtkUnsignedLongArray *) gridPointData->GetArray("Node_Numbers");
  if ( nodexIndexArray == NULL )
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
  

  /*********************** Test 2 - Compute Table Internally ***********************/
  vtkMimxRebinMaterialProperty *rebinFilter2 = vtkMimxRebinMaterialProperty::New();
  rebinFilter2->SetInput( grid );
  rebinFilter2->SetNumberOfHistogramBins(15);
  rebinFilter2->SetComputeMaxBin( false );
  rebinFilter2->SetComputeMinBin( false );
  rebinFilter2->SetBinLowerBound( 0.0 );
  rebinFilter2->SetBinUpperBound( 3000.0 );
  rebinFilter2->SetGeneratePropertyBins( true );
  rebinFilter2->Update();
      
  vtkFieldData *materialTable2 = rebinFilter2->GetPropertyTable();
  vtkDoubleArray *histogramArray2 = NULL;
  //histogramArray2 = vtkDoubleArray::SafeDownCast( materialTable2->GetColumnByName( "Histogram" ) );
  histogramArray2 = vtkDoubleArray::SafeDownCast( materialTable2->GetArray( "Histogram" ) );
  
  vtkUnstructuredGrid *rebinGrid2 = rebinFilter2->GetOutput( );
  numberOfCells = rebinGrid2->GetNumberOfCells();
  numberOfPoints = rebinGrid2->GetNumberOfPoints();
  if ( numberOfCells != 448 )
    {
    std::cerr << "Error: Invalid number of cells in rebinned grid. (Test 2) " << std::endl;
    std::cerr << "Expected 448 and obtained " << numberOfCells << std::endl;
    return EXIT_FAILURE;
    }

  if ( numberOfPoints != 648 )
    {
    std::cerr << "Error: Invalid number of points in rebinned grid. (Test 2)" << std::endl;
    std::cerr << "Expected 648 and obtained " << numberOfPoints << std::endl;
    return EXIT_FAILURE;
    }
  
  /****************** Check Cell Data ****************************/
  gridFieldData = rebinGrid2->GetFieldData();
  gridPointData = rebinGrid2->GetPointData( );
  //tmpArray = gridFieldData->GetAbstractArray("Material_Properties");
  //tmpArray = vtkAbstractArray::SafeDownCast( gridFieldData->GetArray("Material_Properties") );
  //if ( ! tmpArray->IsA("vtkDoubleArray") )
  //{
  //  std::cerr << "Error: Test 2" << std::endl;
  //  std::cerr << "Failed to obtain 'Material_Properties' Field Data" << std::endl;
  //  return EXIT_FAILURE;
  //}
  vtkDoubleArray *materialPropertyArray1 = NULL;
  materialPropertyArray1 = (vtkDoubleArray *) gridFieldData->GetArray("Material_Properties");
  if ( materialPropertyArray1 == NULL )
  {
    std::cerr << "Error: Test 2" << std::endl;
    std::cerr << "Failed to obtain 'Material_Properties' Field Data" << std::endl;
    return EXIT_FAILURE;
  }
  //materialPropertyArray = vtkDoubleArray::SafeDownCast(tmpArray);
  //Temporary Sanitary Check
  //for (int i=0;i<histogramArray2->GetNumberOfTuples( );i++)
  //  {
  //  std::cerr << "Mat Element " << i << " " << histogramArray2->GetValue(i) << std::endl;
  //  }
  for (int i=0;i<numberOfCells;i++)
    {
    bool found = false;
    double value = materialPropertyArray1->GetValue(i);
    for (int j=0;j<15;j++)
      {
      if (fabs(value-histogramArray2->GetValue(j)) < 0.0001)
        {
        found = true;
        }
      }
    if (! found ) 
      {
      std::cerr << "Error: Test 2" << std::endl;
      std::cerr << "Failed to obtain correct material properties after rebinning" << std::endl;
      return EXIT_FAILURE;
      }
    }
  
  /****************** Check Node Data ****************************/
  //tmpArray = gridPointData->GetAbstractArray("Node_Numbers");
  //tmpArray = vtkAbstractArray::SafeDownCast( gridPointData->GetArray("Node_Numbers") );
  //if ( ! tmpArray->IsA("vtkUnsignedLongArray") )
  //  {
  //  std::cerr << "Error: Test 2 - Failed to obtain 'Node_Numbers' Field Data" << std::endl;
  //  return EXIT_FAILURE;
  //  }
  vtkUnsignedLongArray *nodexIndexArray1 = NULL;
  nodexIndexArray1 = (vtkUnsignedLongArray *) gridPointData->GetArray("Node_Numbers");
  if ( nodexIndexArray1 == NULL )
    {
    std::cerr << "Error: Test 2 - Failed to obtain 'Node_Numbers' Field Data" << std::endl;
    return EXIT_FAILURE;
    }
  //nodexIndexArray = vtkUnsignedLongArray::SafeDownCast(tmpArray);
  index0 = nodexIndexArray1->GetValue(0);
  indexLast = nodexIndexArray1->GetValue( numberOfPoints-1 );

  if (index0 != 3*12*12+3*12+3+100)
    {
    std::cerr << "Error: Test 2 - Invalid nodex 0 index value" << std::endl;
    return EXIT_FAILURE;
    }

  if (indexLast != 9*12*12+10*12+10+100)
    {
    std::cerr << "Error: Test 2 - Invalid last node index value" << std::endl;
    return EXIT_FAILURE;
    }
  

  /**************** Test 3 - Compute Table Given Lower Bound **************/
  vtkMimxRebinMaterialProperty *rebinFilter3 = vtkMimxRebinMaterialProperty::New();
  rebinFilter3->SetInput( grid );
  rebinFilter3->SetNumberOfHistogramBins(9);
  rebinFilter3->SetBinLowerBound( 300.0 );
  rebinFilter3->SetComputeMinBin( false );
  rebinFilter3->SetComputeMaxBin( true );
  rebinFilter3->SetGeneratePropertyBins( true );
  rebinFilter3->Update();
  
  vtkUnstructuredGrid *rebinGrid3 = rebinFilter3->GetOutput( );
  numberOfCells = rebinGrid3->GetNumberOfCells();
  numberOfPoints = rebinGrid3->GetNumberOfPoints();
  gridFieldData = rebinGrid3->GetFieldData();
  gridPointData = rebinGrid3->GetPointData( );
  
  vtkFieldData *materialTable3 = rebinFilter3->GetPropertyTable();
  //histogramArray = vtkDoubleArray::SafeDownCast( materialTable3->GetColumnByName( "Histogram" ) );
  //histogramArray = vtkDoubleArray::SafeDownCast( materialTable3->GetArray( "Histogram" ) );
  vtkDoubleArray *histogramArray3 = NULL;
  histogramArray3 = (vtkDoubleArray *) materialTable3->GetArray( "Histogram" );
  if ( histogramArray3->GetNumberOfTuples() != 9 )
    {
    std::cerr << "Error: Test 3 - Invalid number of material property array bins" << std::endl;
    return EXIT_FAILURE;
    }
  
  // Temporary Sanitary Check
  for (int i=0;i<histogramArray3->GetNumberOfTuples( );i++)
    {
    std::cerr << "Mat Element " << i << " " << histogramArray3->GetValue(i) << std::endl;
    }
  
 
  //tmpArray = gridFieldData->GetAbstractArray("Material_Properties");
  //tmpArray = vtkAbstractArray::SafeDownCast( gridFieldData->GetArray("Material_Properties") );
  //if ( ! tmpArray->IsA("vtkDoubleArray") )
  //  {
  //  std::cerr << "Error: Test 3" << std::endl;
  //  std::cerr << "Failed to obtain 'Material_Properties' Field Data" << std::endl;
  //  return EXIT_FAILURE;
  //  }
  //materialPropertyArray = vtkDoubleArray::SafeDownCast(tmpArray);
  vtkDoubleArray *materialPropertyArray2 = NULL;
  materialPropertyArray2 = (vtkDoubleArray *) gridFieldData->GetArray("Material_Properties");
  if ( materialPropertyArray2 == NULL )
    {
    std::cerr << "Error: Test 3" << std::endl;
    std::cerr << "Failed to obtain 'Material_Properties' Field Data" << std::endl;
    return EXIT_FAILURE;
    }

  for (int i=0;i<numberOfCells;i++)
    {
    bool found = false;
    double value = materialPropertyArray2->GetValue(i);
    for (int j=0;j<9;j++)
      {
      if (fabs(value-histogramArray3->GetValue(j)) < 0.0001)
        {
          found = true;
        }
      }
    if (! found ) 
      {
      std::cerr << "Error: Test 3" << std::endl;
      std::cerr << "Failed to obtain correct material properties after rebinning" << std::endl;
      return EXIT_FAILURE;
      }
    }
  
  /************* Test 4 - Compute Table Given Upper Bound ***************/
  vtkMimxRebinMaterialProperty *rebinFilter4 = vtkMimxRebinMaterialProperty::New();
  rebinFilter4->SetInput( grid );
  rebinFilter4->SetNumberOfHistogramBins( 12 );
  rebinFilter4->SetBinUpperBound( 3500.0 );
  rebinFilter4->SetGeneratePropertyBins( true );
  rebinFilter4->SetComputeMinBin( true );
  rebinFilter4->SetComputeMaxBin( false );
  rebinFilter4->Update();
  
  vtkUnstructuredGrid *rebinGrid4 = rebinFilter4->GetOutput( );
  numberOfCells = rebinGrid4->GetNumberOfCells();
  numberOfPoints = rebinGrid4->GetNumberOfPoints();
  gridFieldData = rebinGrid4->GetFieldData();
  gridPointData = rebinGrid4->GetPointData( );
  
  vtkFieldData *materialTable4 = rebinFilter4->GetPropertyTable();
  //histogramArray = vtkDoubleArray::SafeDownCast( materialTable4->GetColumnByName( "Histogram" ) );
  //histogramArray = vtkDoubleArray::SafeDownCast( materialTable4->GetArray( "Histogram" ) );
  vtkDoubleArray *histogramArray4 = NULL;
  histogramArray4 = (vtkDoubleArray *) materialTable4->GetArray( "Histogram" );
  if ( histogramArray4->GetNumberOfTuples() != 12 )
    {
      std::cerr << "Error: Test 4 - Invalid number of material property array bins " << histogramArray4->GetSize() << std::endl;
    //return EXIT_FAILURE;
    }
  /* Test for VTK 5.0 Field Data Writer */
  /*
  vtkDataWriter *writer = vtkDataWriter::New();
  writer->SetFileName( "histogram.vtk" );
  ostream *ofs = writer->OpenVTKFile();
  writer->WriteHeader( ofs );
  writer->WriteFieldData( ofs, materialTable4 );
  writer->CloseVTKFile( ofs );
  */
  //tmpArray = gridFieldData->GetAbstractArray("Material_Properties");
  //tmpArray = vtkAbstractArray::SafeDownCast( gridFieldData->GetArray("Material_Properties") );
  //if ( ! tmpArray->IsA("vtkDoubleArray") )
  //  {
  //  std::cerr << "Error: Test 4" << std::endl;
  //  std::cerr << "Failed to obtain 'Material_Properties' Field Data" << std::endl;
  //  return EXIT_FAILURE;
  //  }
  //materialPropertyArray = vtkDoubleArray::SafeDownCast(tmpArray);
  vtkDoubleArray *materialPropertyArray3 = NULL;
  materialPropertyArray3 = (vtkDoubleArray *) gridFieldData->GetArray("Material_Properties");
  if ( materialPropertyArray3 == NULL )
    {
    std::cerr << "Error: Test 4" << std::endl;
    std::cerr << "Failed to obtain 'Material_Properties' Field Data" << std::endl;
    return EXIT_FAILURE;
    }

  for (int i=0;i<numberOfCells;i++)
    {
    bool found = false;
    double value = materialPropertyArray3->GetValue(i);
    for (int j=0;j<12;j++)
      {
      if (fabs(value-histogramArray4->GetValue(j)) < 0.0001)
        {
        found = true;
        }
      }
    if (! found ) 
      {
      std::cerr << "Error: Test 4" << std::endl;
      std::cerr << "Failed to obtain correct material properties after rebinning" << std::endl;
      return EXIT_FAILURE;
      }
    }
  //std::cerr << "hist" << std::endl;
  histogramArray->Delete();
  histogramArray2->Delete();
  histogramArray3->Delete();
  histogramArray4->Delete();
  //std::cerr << "mat table" << std::endl;
  materialTable->Delete();
  materialTable2->Delete();
  materialTable3->Delete();
  materialTable4->Delete();
  //std::cerr << "mat prop" << std::endl;
  materialPropertyArray->Delete();
  materialPropertyArray1->Delete();
  materialPropertyArray2->Delete();
  materialPropertyArray3->Delete();
  
  //std::cerr << "grid" << std::endl;
  grid->Delete();
  //std::cerr << "writer" << std::endl;
  tmpWriter->Delete();
  //std::cerr << "1" << std::endl;
  rebinFilter->Delete();
  //std::cerr << "2" << std::endl;
  rebinFilter2->Delete();
  //std::cerr << "3" << std::endl;
  rebinFilter3->Delete();
  //std::cerr << "4" << std::endl;
  rebinFilter4->Delete();
    
    
  return EXIT_SUCCESS;
}
