#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include <iostream>
#include <algorithm>
#include <string>

#include "itkPluginFilterWatcher.h"
#include "itkPluginUtilities.h"

#include "vtkSmartPointer.h"
#include "vtkTimerLog.h"

#include "vtkNRRDReader.h"
#include "vtkNRRDWriter.h"
#include "vtkImageData.h"
#include "ImageLabelCombineCLP.h"

#include "vtkImageLabelCombine.h"


int main( int argc, const char * argv[] ){

  PARSE_ARGS;

  // Read in label volume inputs
  vtkNRRDReader *reader_A = vtkNRRDReader::New();
  reader_A->SetFileName(InputLabelMap_A.c_str());
  reader_A->Update();
 
  vtkNRRDReader *reader_B = vtkNRRDReader::New();
  reader_B->SetFileName(InputLabelMap_B.c_str());
  reader_B->Update();
  
  // combine labels
  vtkImageLabelCombine *labelCombine = vtkImageLabelCombine::New();
  labelCombine->SetInput1(reader_A->GetOutput());
  labelCombine->SetInput2(reader_A->GetOutput());
  labelCombine->Update();

  // Output
  vtkNRRDWriter *writer = vtkNRRDWriter::New();
  writer->SetFileName(OutputLabelMap.c_str());
  writer->SetInput( labelCombine->GetOutput() );
  writer->Write();

  //Delete everything
  reader_A->Delete();
  reader_B->Delete();
  labelCombine->Delete();
  writer->Delete();

  return EXIT_SUCCESS;
}
