/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: VoxelMesh.cxx,v $
Language:  C++
Date:      $Date: 2007/04/09 02:15:42 $
Version:   $Revision: 1.3 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/

Copyright (c) MIMX. All rights reserved.
See http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

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
//#include <itkArray.h>
#include <itkImage.h>
#include <itkOrientedImage.h>
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
#include <vtkDataObject.h>
#include <vtkDataObjectReader.h>
#include <vtkDataReader.h>

#include <vtkPluginFilterWatcher.h>
#include <itkPluginFilterWatcher.h>


#include <vtkMimxRebinMaterialProperty.h>
#include <itkMimxImageToVtkUnstructuredGridFilter.h>
#include <vtkAbaqusFileWriter.h>
#include "VoxelMeshCLP.h"



/*
 * Typedefs used throughout the program
 */
typedef    signed short                               InputPixelType;
typedef    itk::OrientedImage< InputPixelType,  3 >   ImageType;
typedef    ImageType::DirectionType                   DirectionType;
typedef    ImageType::PointType                       PointType;

/*
 * Locally Defined Functions
 */
void FixDirectionsAndOrigin (DirectionType &imageDir, PointType &origin);





int main( int argc, char * argv[] )
{
  itk::Brains2MaskImageIOFactory::RegisterOneFactory( );

  /** Handle the Command line arguments */
  PARSE_ARGS;
  bool debug = false;
 
  if ( debug )
    {
    // Summarize the command line arguements for the user
    std::cout << "Voxel Meshing Parameters" << std::endl;
    std::cout << "------------------------------------------------------" << std::endl;
    std::cout << "\tInput image filename: " << imageFileName << std::endl;
    std::cout << "\tMask filename: " << maskFileName << std::endl;
    std::cout << "\tOutput Abaqus filename: " << abaqusFileName << std::endl;
    std::cout << "\tAbaqus Title: " << abaqusTitle << std::endl;
    std::cout << "\tOutput VTK filename: " << vtkFilename << std::endl;
    std::cout << "\tOutput VTK Polydata filename: " << vtkPolyDataFilename << std::endl;
    std::cout << "\tModulus bin filename: " << histogramBinFilename << std::endl;
    std::cout << "\tNumber of Bins: " << numberOfBins << std::endl;
    std::cout << "\tBin Upper Bound: " << histogramUpperBound << std::endl;
    std::cout << "\tBin Lower Bound: " << histogramLowerBound << std::endl;
    std::cout << "\tResample Factor: " << resampleSize << std::endl;
    std::cout << "\tMesh Offset: " << meshIndexOffset << std::endl;
    std::cout << "\tPoisson Ratio: " << poissonRatio << std::endl;
    std::cout << "------------------------------------------------------" << std::endl;
  }



  /** Read the Input Image */
  typedef itk::ImageFileReader< ImageType >  ReaderType;
  ReaderType::Pointer imageReader = ReaderType::New( );
  itk::PluginFilterWatcher watchLoadImage(imageReader, 
                                          "Load Image", 
                                          CLPProcessInformation, 
                                          1.0/10.0, 0.0/10.0);
  imageReader->SetFileName( imageFileName.c_str( ) );
  imageReader->Update( );
  

  /**
   * The image must be in RAS Orientation  - 
   *      This will adjust the Direction Cosines accordingly
   */
  ImageType::Pointer inputImage = imageReader->GetOutput( ) ;
  if ( debug ) std::cout << "Original Image: " << inputImage << std::endl;
  
  DirectionType imageDirection = inputImage->GetDirection( );
  PointType imageOrigin = inputImage->GetOrigin( );
  FixDirectionsAndOrigin( imageDirection, imageOrigin );
  inputImage->SetDirection( imageDirection );
  inputImage->SetOrigin( imageOrigin );
  if ( debug ) std::cout << "RAS Image: " << inputImage << std::endl;
  
  /** Reading the Input Mask */
  ReaderType::Pointer maskReader = ReaderType::New( );
  ImageType::Pointer inputMask;
  
  if ( ! maskFileName.empty() )
    {
    itk::PluginFilterWatcher watchLoadMask(maskReader, 
                                           "Load Mask", 
                                           CLPProcessInformation, 
                                           1.0/10.0, 1.0/10.0);
    maskReader->SetFileName( maskFileName.c_str( ) );
    maskReader->Update( );
    inputMask = maskReader->GetOutput( );
    
    if ( debug ) std::cout << "Input Mask :" << inputMask << std::endl;
    DirectionType maskDirection = inputMask->GetDirection( );
    PointType maskOrigin = inputMask->GetOrigin( );
    FixDirectionsAndOrigin( maskDirection, maskOrigin );
    inputMask->SetDirection( maskDirection );
    inputMask->SetOrigin( maskOrigin );
    if ( debug ) std::cout << "RAS Mask: " << inputMask << std::endl;
    }

  /** Now Generate the Mesh */
  typedef itk::MimxImageToVtkUnstructuredGridFilter< ImageType, 
                                         ImageType> ImageToMeshFilterType;

  ImageToMeshFilterType::Pointer imageToHexMeshFilter = 
                                                ImageToMeshFilterType::New( );

  itk::PluginFilterWatcher watchMeshFilter(imageToHexMeshFilter, 
                                           "Create Mesh", 
                                           CLPProcessInformation, 
                                           5.0/10.0, 2.0/10.0);
  imageToHexMeshFilter->SetInput( inputImage );
  if ( ! maskFileName.empty() )
    {
    imageToHexMeshFilter->SetMaskImage( inputMask );
    }
  imageToHexMeshFilter->SetMeshIndexOffset( meshIndexOffset );
  imageToHexMeshFilter->SetComputeMeshPropertiesOn( );
  imageToHexMeshFilter->SetUseCtPropertiesOn( );
  imageToHexMeshFilter->SetComputeMeshNodeNumberingOn( );  
  imageToHexMeshFilter->SetMeshResampleSize( resampleSize );
  imageToHexMeshFilter->Update( );

  vtkUnstructuredGrid* finalMesh = imageToHexMeshFilter->GetOutput( );
 
  /* Rebin Mesh Material Properties */
  if ( numberOfBins != 0 || ! histogramBinFilename.empty() )
  {
    vtkMimxRebinMaterialProperty *rebinMeshMaterialProperties = vtkMimxRebinMaterialProperty::New();
    vtkPluginFilterWatcher watchRebinFilter(rebinMeshMaterialProperties, 
                                            "Rebin Material Properties", 
                                            CLPProcessInformation, 
                                            1.0/10.0, 7.0/10.0);
    rebinMeshMaterialProperties->SetInput( imageToHexMeshFilter->GetOutput( ) );
    if ( ! histogramBinFilename.empty() )
      {
      //vtkTableReader *histogramReader = vtkTableReader::New();
      //histogramReader->SetFileName( histogramBinFilename.c_str() );
      //histogramReader->Update();
      //rebinMeshMaterialProperties->SetPropertyTable( histogramReader->GetOutput( ) );
      vtkDataReader *histogramReader = vtkDataReader::New();
      histogramReader->SetFileName( histogramBinFilename.c_str() );
      histogramReader->OpenVTKFile( );
      histogramReader->ReadHeader( );
      vtkFieldData *histogramData = histogramReader->ReadFieldData( );
      histogramReader->CloseVTKFile();
      //rebinMeshMaterialProperties->SetPropertyTable( histogramReader->GetOutput( )->GetFieldData() );
      rebinMeshMaterialProperties->SetPropertyTable( histogramData );
      }
    else
      {
      rebinMeshMaterialProperties->SetNumberOfHistogramBins( numberOfBins );
      if (histogramLowerBound != -1)
        {
        rebinMeshMaterialProperties->SetComputeMaxBin( false );
        rebinMeshMaterialProperties->SetBinUpperBound( histogramUpperBound );
        }
      if (histogramLowerBound != -1)
        {
          rebinMeshMaterialProperties->SetComputeMinBin( false );
          rebinMeshMaterialProperties->SetBinLowerBound( histogramLowerBound );
        }
      }
    rebinMeshMaterialProperties->Update( );
    finalMesh = rebinMeshMaterialProperties->GetOutput( );
  }
  
  /* Write out the Mesh in Abaqus file format */
  if ( ! abaqusFileName.empty() )
    {
    time_t cseconds = time( NULL );
    std::string header = "";
    header += "****************************************************************************\n"; 
    header += "**                                                                        **\n"; 
    header += "**                                                                        **\n"; 
    header += "**                                                                        **\n"; 
    header += "**  Slicer 3 - Voxel Meshing Model                                        **\n";
    header += "**  MIMX Laboratory                                                       **\n"; 
    header += "**  Center for Computer Aided Design                                      **\n"; 
    header += "**  The University of Iowa                                                **\n"; 
    header += "**  Iowa City, IA  52242                                                  **\n"; 
    header += "**                                                                        **\n"; 
    header += "**                                                                        **\n"; 
    header += "**  Project Title: " + abaqusTitle + "\n"; 
    header += "**  Date: ";
    header += ctime( &cseconds );
    header += "\n";
    header += "**                                                                        **\n"; 
    header += "****************************************************************************\n";
      
    vtkAbaqusFileWriter *writer = vtkAbaqusFileWriter::New();
    vtkPluginFilterWatcher watchRebinFilter(writer, 
                                              "Write Abaqus Mesh File", 
                                              CLPProcessInformation, 
                                             1.0/10.0, 8.0/10.0);
    writer->SetInput( finalMesh );
    writer->SetAbaqusHeader( header );
    writer->SetFileName( abaqusFileName.c_str() ); /* .inp file */
    writer->Update();
    }
  
  /* Write out the Mesh in VTK file format */
  if ( ! vtkFilename.empty() )
    {
    vtkXMLUnstructuredGridWriter *writer = vtkXMLUnstructuredGridWriter::New();
    vtkPluginFilterWatcher watchRebinFilter(writer, 
                                            "Write VTK File", 
                                            CLPProcessInformation, 
                                            1.0/10.0, 9.0/10.0);
    writer->SetInput( finalMesh );
    writer->SetFileName( vtkFilename.c_str() ); /* .vtu file */
    writer->SetDataModeToAscii( );
    writer->Update();
    }
  
  /* Write out the Mesh in VTK file format - Surface nodes only Temporary Slicer3 Hack */
  if ( ! vtkPolyDataFilename.empty() )
    {
    vtkGeometryFilter *polyDataFilter = vtkGeometryFilter::New();
    polyDataFilter->SetInput( finalMesh );
    
    vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
    writer->SetInput( polyDataFilter->GetOutput( ) );
    writer->SetFileName( vtkPolyDataFilename.c_str() );
    writer->SetFileTypeToASCII( );
    writer->Update();
    }  
  
  return EXIT_SUCCESS;
}

void FixDirectionsAndOrigin (DirectionType &imageDir, PointType &origin)
{
  
  int dominantAxisRL = itk::Function::Max3(imageDir[0][0],imageDir[1][0],imageDir[2][0]);
  int signRL = itk::Function::Sign(imageDir[dominantAxisRL][0]);
  int dominantAxisAP = itk::Function::Max3(imageDir[0][1],imageDir[1][1],imageDir[2][1]);
  int signAP = itk::Function::Sign(imageDir[dominantAxisAP][1]);
  int dominantAxisSI = itk::Function::Max3(imageDir[0][2],imageDir[1][2],imageDir[2][2]);
  int signSI = itk::Function::Sign(imageDir[dominantAxisSI][2]);
  
  bool debug = false;
  
  if ( debug )
    {
    std::cout << "Dominant RL Axis: " << dominantAxisRL << std::endl;
    std::cout << "Dominant AP Axis: " << dominantAxisAP << std::endl;
    std::cout << "Dominant SI Axis: " << dominantAxisSI << std::endl;
    std::cout << "Sign RL Axis: " << signRL << std::endl;
    std::cout << "Sign AP Axis: " << signAP << std::endl;
    std::cout << "Sign SI Axis: " << signSI << std::endl;
    }
  
  DirectionType DirectionToRAS;
  DirectionToRAS.SetIdentity( );
  
  if (signRL == 1)
    {
      DirectionToRAS[dominantAxisRL][dominantAxisRL] = -1.0; 
      origin[dominantAxisRL] *= -1.0;
    }
  if (signAP == -1)
    {
      DirectionToRAS[dominantAxisAP][dominantAxisAP] = -1.0; 
      origin[dominantAxisAP] *= -1.0;
    }
  if (signSI == 1)
    {
      DirectionToRAS[dominantAxisSI][dominantAxisSI] = -1.0; 
      origin[dominantAxisSI] *= -1.0;
    }
  
  imageDir *= DirectionToRAS;
  
  
}
 
