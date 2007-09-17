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
#include "ROISelectCLP.h"

#include "vtkPolyData.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkProbeFilter.h"
#include "vtkXMLPolyDataWriter.h"


int main( int argc, const char * argv[] )
{

  PARSE_ARGS;

  // Read in ROI volume inputs
  vtkNRRDReader *readerROI_A = vtkNRRDReader::New();
  readerROI_A->SetFileName(InputROI_A.c_str());
  readerROI_A->Update();
  
  //if ( readerROI_A->GetOutput()->GetPointData()->GetTensors() == NULL )
  //  {
  //  std::cerr << argv[0] << ": No tensor data" << std::endl;
  //  return EXIT_FAILURE;
  //  }

  // Read in fiber bundle input to be selected.
  vtkXMLPolyDataReader *readerPD = vtkXMLPolyDataReader::New();
  readerPD->SetFileName(InputFibers.c_str());
  readerPD->Update();

  // These probe filters will sample the ROI volume(s) at each point on the fibers.
  vtkProbeFilter *probe = vtkProbeFilter::New();
  
  //1. Probe's source (volume to sample) is region of interest volume
  probe->SetSource(readerROI_A->GetOutput());
  
  //2. Set up matrices to put fibers into ijk space of volume
  // This assumes fibers are in RAS space of volume (i.e. RAS==world)
  vtkMatrix4x4 *ROI_A_RASToIJK = vtkMatrix4x4::New();
  ROI_A_RASToIJK->DeepCopy(readerROI_A->GetRasToIjkMatrix());
  
  //the volume we're probing knows its spacing so take this out of the matrix
  double sp[3];
  readerROI_A->GetOutput()->GetSpacing(sp);
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

  //5. Do the probing. The output of probe is new polydata with scalars
  // from the probing.
  probe->Update();

  //6. Transform the model back into original space
  vtkTransform *trans2 = vtkTransform::New();
  trans2->DeepCopy( trans );
  trans2->Inverse();
  vtkTransformPolyDataFilter *transformer2 = vtkTransformPolyDataFilter::New();
  transformer2->SetTransform( trans2 );
  transformer2->SetInput( probe->GetOutput() );
  transformer2->Update();

  
  // TO DO
  // Select the fibers based on the scalars from the probing.  The ROI has
  // some positive values (say for example the value 20).  When this is
  // probed there will be various numbers on the polyline's scalars.  The
  // probe can't do nearest neighbor interpolation so if these values are
  // nonzero assume the fiber has passed near enough to an ROI voxel.
  // The quick way was to check the scalar range of the polydata but here a loop
  // is needed to go through the scalars belonging to each line.
  
  // TO DO
  // Handle multiple ROIs and also AND/NOT Boolean operations.
  
  
  //7. Save the output
  vtkXMLPolyDataWriter *writer = vtkXMLPolyDataWriter::New();
  writer->SetFileName(OutputFibers.c_str());
  writer->SetInput( transformer2->GetOutput() );
  writer->Write();

  //8. Delete everything
  writer->Delete();
  probe->Delete();
  transformer->Delete();
  transformer2->Delete();
  trans->Delete();
  ROI_A_RASToIJK->Delete();
  readerPD->Delete();
  readerROI_A->Delete();

  return EXIT_SUCCESS;
}
