#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include <iostream>
#include "ROITractFilterCLP.h"
#include <string>
#include <vector>
#include "itkShiftScaleImageFilter.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "vtkPoints.h" 
#include "vtkZLibDataCompressor.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkCleanPolyData.h"

int main(int argc, char* argv[]){
  PARSE_ARGS;
  //define the input/output types
  typedef itk::Image< short int, 3 > ROIImageType;

  //define reader and writer
  typedef itk::ImageFileReader< ROIImageType > ROIImageReaderType;

  //setup the ROI image reader
  ROIImageReaderType::Pointer roireaderPtr = ROIImageReaderType::New();
  roireaderPtr->SetFileName(roifilename);
  roireaderPtr->Update();
  
  //Load in the vtk tracts
  vtkXMLPolyDataReader* tractsreader = vtkXMLPolyDataReader::New();
  tractsreader->SetFileName( inputtractsfilename.c_str() );
  tractsreader->Update();
  vtkCellArray* loadedtracts = tractsreader->GetOutput()->GetLines();
  std::cout<<"Total Input Tracts: "<<loadedtracts->GetNumberOfCells()<<std::endl;
  
  //allocate new VTK Polydata to output the filtered tracts
  vtkPolyData* filteredtracts = vtkPolyData::New();
  //vtkPoints* filteredpoints = vtkPoints::New();
  vtkCellArray* filteredtractarray = vtkCellArray::New();
  
  ROIImageType::IndexType index;  //preallocate for efficiency
  
  vtkIdType npts;
  vtkIdType* pts;
  vtkPoints* points = tractsreader->GetOutput()->GetPoints();
  loadedtracts->InitTraversal();

  while( loadedtracts->GetNextCell( npts, pts ) ){
    //std::cout<<std::endl;
    int state=0;
    int firstoutsidepointID=0;
    int currentlabel=0;
    for(int currentpointIDindex=0; currentpointIDindex<npts; currentpointIDindex++){
      double* vertex = points->GetPoint( pts[currentpointIDindex] );
      index[0]=static_cast<long int>(vertex[0]);
      index[1]=static_cast<long int>(vertex[1]); 
      index[2]=static_cast<long int>(vertex[2]);
      ROIImageType::PixelType& roiimagepix = roireaderPtr->GetOutput()->GetPixel( index );
      switch(state){
      case 0: //have not found first ROI
        if(roiimagepix==roilabels[0]){
          state=1;
          currentlabel++;
        }
        break;
      case 1: //found first ROI but have not left it yet
        if(roiimagepix!=roilabels[0]){
          state=2;
          firstoutsidepointID = currentpointIDindex;
        }
        //don't break because it is possible that 1st outside pixel is last ROI
      case 2: //left first ROI, looking for remaining ROI's in order
        if(roiimagepix==roilabels[currentlabel]){
          if( currentlabel==roilabels.size()-1 ){
            state=3;
          } 
          else currentlabel++;
        }
        break;
      default:
        break;
      }
      //std::cout<<state;
      if(state==3){
        if(cuttractsswitch){
          vtkIdType* cutpts = pts+firstoutsidepointID;
          filteredtractarray->InsertNextCell( currentpointIDindex-firstoutsidepointID, cutpts );
        }
        else filteredtractarray->InsertNextCell( npts, pts );
        
        break;
      }
    }
  }    
  
  //finish up the vtk polydata
  filteredtracts->SetPoints( tractsreader->GetOutput()->GetPoints() );
  filteredtracts->SetLines( filteredtractarray );
  
  //clean up the poly data to remove redundant points
  vtkCleanPolyData* cleaner = vtkCleanPolyData::New();
  cleaner->SetInput( filteredtracts );
  cleaner->SetAbsoluteTolerance( 0.0 );
  cleaner->Update();
  
  std::cout<<"Total Output Tracts: "<<filteredtracts->GetNumberOfCells()<<std::endl;
  //output the vtk tract container
  vtkZLibDataCompressor* compressor = vtkZLibDataCompressor::New();
  vtkXMLPolyDataWriter* tractswriter = vtkXMLPolyDataWriter::New();
  tractswriter->SetCompressor( compressor );
  //tractswriter->SetDataModeToBinary();
  tractswriter->SetInput( cleaner->GetOutput() );
  tractswriter->SetFileName( outputtractsfilename.c_str() );
  tractswriter->Write();
  
  //cleanup vtk stuff
  tractsreader->Delete();
  filteredtracts->Delete();
  cleaner->Delete();
  //filteredpoints->Delete();
  filteredtractarray->Delete();
  tractswriter->Delete();
  compressor->Delete();

  return EXIT_SUCCESS;
}
