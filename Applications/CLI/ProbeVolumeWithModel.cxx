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


#include "vtkNRRDReader.h"
#include "vtkNRRDWriter.h"
#include "vtkImageData.h"
#include "ProbeVolumeWithModelCLP.h"

#include "vtkPolyData.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkProbeFilter.h"
#include "vtkXMLPolyDataWriter.h"


int main( int argc, const char * argv[] )
{

  PARSE_ARGS;

  vtkNRRDReader *readerVol = vtkNRRDReader::New();
  readerVol->SetFileName(InputVolume.c_str());
  readerVol->Update();
  
  //if ( readerVol->GetOutput()->GetPointData()->GetTensors() == NULL )
  //  {
  //  std::cerr << argv[0] << ": No tensor data" << std::endl;
  //  return EXIT_FAILURE;
  //  }

  vtkXMLPolyDataReader *readerPD = vtkXMLPolyDataReader::New();
  readerPD->SetFileName(InputModel.c_str());
  readerPD->Update();

    
  vtkProbeFilter *probe = vtkProbeFilter::New();
  
  //1. Probe's source is region of interest volume
  probe->SetSource(readerVol->GetOutput());
  
  //2. Set up matrices to put fibers into ijk space of volume
  // This assumes fibers are in RAS space of volume (i.e. RAS==world)
  vtkMatrix4x4 *ROI_A_RASToIJK = vtkMatrix4x4::New();
  ROI_A_RASToIJK->DeepCopy(readerVol->GetRasToIjkMatrix());
  
  //the volume we're probing knows its spacing so take this out of the matrix
  double sp[3];
  readerVol->GetOutput()->GetSpacing(sp);
  vtkTransform *trans = vtkTransform::New();
  trans->Identity();
  trans->PreMultiply();
  trans->SetMatrix(ROI_A_RASToIJK);
  // Trans from IJK to RAS
  trans->Inverse();
  // Take into account spacing to compute Scaled IJK
  trans->Scale(1/sp[0],1/sp[1],1/sp[2]);
  // now it's RAS to scaled IJK
  trans->Inverse();

  //3. Transform fibers
  vtkTransformPolyDataFilter *transformer = vtkTransformPolyDataFilter::New();
  transformer->SetTransform( trans );
  transformer->SetInput( readerPD->GetOutput() );
  transformer->Update();

  //4. Probe's input data is polydata (fibers)
  probe->SetInput( transformer->GetOutput() );

  //5. Do the probing
  probe->Update();

  //6. Transform the model back into original space
  vtkTransform *trans2 = vtkTransform::New();
  trans2->DeepCopy( trans );
  trans2->Inverse();
  vtkTransformPolyDataFilter *transformer2 = vtkTransformPolyDataFilter::New();
  transformer2->SetTransform( trans2 );
  transformer2->SetInput( probe->GetOutput() );
  transformer2->Update();
  
  //7. Save the output
  vtkXMLPolyDataWriter *writer = vtkXMLPolyDataWriter::New();
  writer->SetFileName(OutputModel.c_str());
  writer->SetInput( transformer2->GetOutput() );
  writer->Write();

  //8. Delete everything
  writer->Delete();
  probe->Delete();
  transformer->Delete();
  transformer2->Delete();
  trans->Delete();
  trans2->Delete();
  ROI_A_RASToIJK->Delete();
  readerPD->Delete();
  readerVol->Delete();

  return EXIT_SUCCESS;
}
