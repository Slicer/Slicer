
#include "ProbeVolumeWithModelCLP.h"

// vtkTeem includes
#include <vtkTeemNRRDReader.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkProbeFilter.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridWriter.h>

#include <vtksys/SystemTools.hxx>

int main( int argc, char * argv[] )
{

  PARSE_ARGS;

  vtkNew<vtkTeemNRRDReader> readerVol;
  readerVol->SetFileName(InputVolume.c_str() );
  readerVol->Update();

  vtkNew<vtkTransformFilter> transformer;

  vtkNew<vtkXMLPolyDataReader> readerXMLPD;
  readerXMLPD->SetFileName(InputModel.c_str());
  vtkNew<vtkPolyDataReader> readerVTKPD;
  readerVTKPD->SetFileName(InputModel.c_str());
  vtkNew<vtkXMLUnstructuredGridReader> readerXMLUG;
  readerXMLUG->SetFileName(InputModel.c_str());
  vtkNew<vtkUnstructuredGridReader> readerVTKUG;
  readerVTKUG->SetFileName(InputModel.c_str());

  bool isPolyData = true; // polydata or unstructured grid

  if (readerXMLPD->CanReadFile(InputModel.c_str()))
    {
    readerXMLPD->Update();
    transformer->SetInputConnection(readerXMLPD->GetOutputPort());
    }
  else if (readerVTKPD->IsFileValid("polydata"))
    {
    readerVTKPD->Update();
    transformer->SetInputConnection(readerVTKPD->GetOutputPort());
    }
  else if (readerXMLUG->CanReadFile(InputModel.c_str()))
    {
    readerXMLUG->Update();
    transformer->SetInputConnection(readerXMLUG->GetOutputPort());
    isPolyData = false;
    }
  else if (readerVTKUG->IsFileValid("unstructured_grid"))
    {
    readerVTKUG->Update();
    transformer->SetInputConnection(readerVTKUG->GetOutputPort());
    isPolyData = false;
    }
  else
    {
    std::cerr << "Failed to find suitable reader for " << InputModel.c_str() << std::endl;
    return -1;
    }

  // Set up matrices to put model points into ijk space of volume
  // This assumes points are in RAS space of volume (i.e. RAS==world)
  vtkNew<vtkMatrix4x4> ROI_A_RASToIJK;
  ROI_A_RASToIJK->DeepCopy(readerVol->GetRasToIjkMatrix());

  // the volume we're probing knows its spacing so take this out of the matrix
  double sp[3] = { 1.0, 1.0, 1.0 };
  readerVol->GetOutput()->GetSpacing(sp);
  vtkNew<vtkTransform> trans;
  trans->Identity();
  trans->PreMultiply();
  trans->SetMatrix(ROI_A_RASToIJK.GetPointer());
  // Trans from IJK to RAS
  trans->Inverse();
  // Take into account spacing to compute Scaled IJK
  trans->Scale(1 / sp[0], 1 / sp[1], 1 / sp[2]);
  // now it's RAS to scaled IJK
  trans->Inverse();

  transformer->SetTransform(trans.GetPointer());
  transformer->Update();

  vtkNew<vtkProbeFilter> probe;
  probe->SetSourceConnection(readerVol->GetOutputPort());
  probe->SetInputConnection( transformer->GetOutputPort() );
  probe->Update();

  // Transform the model back into original space
  vtkNew<vtkTransform> trans2;
  trans2->DeepCopy(trans.GetPointer());
  trans2->Inverse();
  vtkNew<vtkTransformFilter> transformer2;
  transformer2->SetTransform(trans2.GetPointer());
  transformer2->SetInputConnection(probe->GetOutputPort());
  transformer2->Update();

  // Save the output
  std::string ext = vtksys::SystemTools::LowerCase(vtksys::SystemTools::GetFilenameLastExtension(OutputModel));
  bool isLegacyFileFormat = (ext.compare(".vtk") == 0);
  if (isPolyData)
    {
    if (isLegacyFileFormat)
      {
      vtkNew<vtkPolyDataWriter> dataWriter;
      dataWriter->SetFileName(OutputModel.c_str());
      dataWriter->SetInputConnection(transformer2->GetOutputPort());
      dataWriter->Write();
      }
    else
      {
      vtkNew<vtkXMLPolyDataWriter> xmlWriter;
      xmlWriter->SetFileName(OutputModel.c_str());
      xmlWriter->SetInputConnection(transformer2->GetOutputPort());
      xmlWriter->Write();
      }
    }
  else
    {
    if (isLegacyFileFormat)
      {
      vtkNew<vtkUnstructuredGridWriter> dataWriter;
      dataWriter->SetFileName(OutputModel.c_str());
      dataWriter->SetInputConnection(transformer2->GetOutputPort());
      dataWriter->Write();
      }
    else
      {
      vtkNew<vtkXMLUnstructuredGridWriter> xmlWriter;
      xmlWriter->SetFileName(OutputModel.c_str());
      xmlWriter->SetInputConnection(transformer2->GetOutputPort());
      xmlWriter->Write();
      }
    }

  return EXIT_SUCCESS;
}
