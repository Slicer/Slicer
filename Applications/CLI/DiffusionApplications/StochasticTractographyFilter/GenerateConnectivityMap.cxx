#include "itkVectorImage.h"
#include "itkImageSeriesReader.h"  //this is needed for itk::ExposeMetaData()
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMetaDataDictionary.h"
#include <iostream>
#include <vector>
#include "itkImageRegionConstIterator.h"
#include "CalculateStatisticsCLP.h"
#include "itkTensorFractionalAnisotropyImageFilter.h"
#include "itkPathIterator.h"
#include <string>
#include "itkShiftScaleImageFilter.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "vtkPoints.h" 

int main(int argc, char* argv[]){
  PARSE_ARGS;
  //define the input/output types
  typedef itk::VectorImage< unsigned short int, 3 > DWIVectorImageType;
  typedef itk::Image< unsigned int, 3 > CImageType;
  typedef itk::Image< bool, 3 > BooleanImageType;
  typedef itk::Image< float, 3 > NormalizedCImageType;

  //define reader and writer
  typedef itk::ImageFileReader< DWIVectorImageType > DWIVectorImageReaderType;
  typedef itk::ImageFileWriter< CImageType > CImageWriterType;
  typedef itk::ImageFileWriter< NormalizedCImageType > NormalizedCImageWriterType;
  
  //define a filter to normalize the connectivity map
  typedef itk::ShiftScaleImageFilter< CImageType, NormalizedCImageType >
    NormalizeCImageFilterType;

  //read in the DWI image
  DWIVectorImageReaderType::Pointer dwireaderPtr = DWIVectorImageReaderType::New();
  dwireaderPtr->SetFileName(dwifilename);
  dwireaderPtr->Update();
  
  //Load in the vtk tracts
  vtkXMLPolyDataReader* tractsreader = vtkXMLPolyDataReader::New();
  tractsreader->SetFileName( tractsfilename.c_str() );
  tractsreader->Update();
  vtkCellArray* loadedtracts = tractsreader->GetOutput()->GetLines();

  //Create a Connectivity Image
  BooleanImageType::Pointer bimagePtr = BooleanImageType::New();
  bimagePtr->CopyInformation( dwireaderPtr->GetOutput() );
  bimagePtr->SetBufferedRegion( dwireaderPtr->GetOutput()->GetBufferedRegion() );
  bimagePtr->SetRequestedRegion( dwireaderPtr->GetOutput()->GetRequestedRegion() );
  bimagePtr->Allocate();
  bimagePtr->FillBuffer(false);
   
  //Create a zeroed accumulated Connectivity Image
  CImageType::Pointer cimagePtr = CImageType::New();
  cimagePtr->CopyInformation( dwireaderPtr->GetOutput() );
  cimagePtr->SetBufferedRegion( dwireaderPtr->GetOutput()->GetBufferedRegion() );
  cimagePtr->SetRequestedRegion( dwireaderPtr->GetOutput()->GetRequestedRegion() );
  cimagePtr->Allocate();
  cimagePtr->FillBuffer(0);
      
   
  //write tracts to connectivity image
  CImageType::IndexType index;  //preallocate for efficiency
  std::cout<<"Writing Tracts to Image, Total Tracts: "<<loadedtracts->GetNumberOfCells()<<std::endl;
  
  vtkIdType npts;
  vtkIdType* pts;
  vtkPoints* points = tractsreader->GetOutput()->GetPoints();
  
  loadedtracts->InitTraversal();
  while( loadedtracts->GetNextCell( npts, pts ) ){
    for( int i=0; i<npts; i++ ){
      double* vertex = points->GetPoint( pts[i] );
                
      index[0]=static_cast<long int>(vertex[0]);
      index[1]=static_cast<long int>(vertex[1]);
      index[2]=static_cast<long int>(vertex[2]);
     
      BooleanImageType::PixelType& bimagepix = bimagePtr->GetPixel( index );
      CImageType::PixelType& cimagepix = cimagePtr->GetPixel( index );
      if(bimagepix == false){ 
        bimagepix=true;
        cimagepix++;
      }
    }
    //clear boolean image
    for( int i=0; i<npts; i++ ){
      double* vertex = points->GetPoint( pts[i] );
     
      index[0]=static_cast<long int>(vertex[0]);
      index[1]=static_cast<long int>(vertex[1]);
      index[2]=static_cast<long int>(vertex[2]);
     
      BooleanImageType::PixelType& bimagepix = bimagePtr->GetPixel( index );
      bimagepix = false;
    }
  }

  //Write the normalized connectivity map
  NormalizeCImageFilterType::Pointer ncifilterPtr = NormalizeCImageFilterType::New();
  ncifilterPtr->SetInput( cimagePtr );
  ncifilterPtr->SetScale( 1.0f/static_cast< double >(loadedtracts->GetNumberOfCells()) );

  NormalizedCImageWriterType::Pointer nciwriterPtr = NormalizedCImageWriterType::New();
  nciwriterPtr->SetInput( ncifilterPtr->GetOutput() );
  nciwriterPtr->SetFileName( ncmapfilename.c_str() );
  nciwriterPtr->SetUseCompression( true );
  nciwriterPtr->Update();

  //clean up vtk stuff
  tractsreader->Delete();
  
  return EXIT_SUCCESS;
}
