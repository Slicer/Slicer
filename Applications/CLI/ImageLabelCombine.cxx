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
  vtkNRRDReader *readerA = vtkNRRDReader::New();
  readerA->SetFileName(InputLabelMap_A.c_str());
  readerA->Update();
 
  vtkNRRDReader *readerB = vtkNRRDReader::New();
  readerB->SetFileName(InputLabelMap_B.c_str());
  readerB->Update();
  
  // combine labels
  vtkImageLabelCombine *labelCombine = vtkImageLabelCombine::New();
  labelCombine->SetOverwriteInput(!FirstOverwrites);
  labelCombine->SetInput1( readerA->GetOutput() );
  labelCombine->SetInput2( readerB->GetOutput() );
  labelCombine->Update();

  // Output
  vtkNRRDWriter *writer = vtkNRRDWriter::New();
  writer->SetFileName(OutputLabelMap.c_str());
  writer->SetInput( labelCombine->GetOutput() );
  
  readerA->GetRasToIjkMatrix()->Invert();
  writer->SetIJKToRASMatrix( readerA->GetRasToIjkMatrix() );
  writer->Write();
  writer->Write();

  //Delete everything
  readerA->Delete();
  readerB->Delete();
  labelCombine->Delete();
  writer->Delete();

  return EXIT_SUCCESS;
}
