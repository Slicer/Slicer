
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
#include <vtkImageChangeInformation.h>

#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelStorageNode.h"

#include <vtksys/SystemTools.hxx>

int main(int argc, char* argv[])
{
  PARSE_ARGS;

  // Read the file
  // Use vtkTeemNRRDReader because it supports both scalar and vector volumes.
  vtkNew<vtkTeemNRRDReader> readerVol;
  readerVol->SetFileName(InputVolume.c_str());
  readerVol->Update();
  vtkImageData* volume = readerVol->GetOutput();
  int* extent = volume->GetExtent();
  if (extent[0]>extent[1] || extent[2] > extent[3] || extent[4] > extent[5])
    {
    std::cerr << "Input image file is empty: " << InputVolume << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Done reading the file " << InputVolume << endl;

  vtkNew<vtkImageChangeInformation> ici;
  ici->SetInputConnection(readerVol->GetOutputPort());
  ici->SetOutputSpacing(1, 1, 1);
  ici->SetOutputOrigin(0, 0, 0);
  ici->Update();
  vtkImageData* volume_Ijk = ici->GetOutput();

  vtkNew<vtkMRMLModelStorageNode> modelStorageNode;
  vtkNew<vtkMRMLModelNode> modelNode;
  modelStorageNode->SetFileName(InputModel.c_str());
  if (!modelStorageNode->ReadData(modelNode))
    {
    std::cerr << "Failed to read input model file " << InputModel << std::endl;
    return EXIT_FAILURE;
    }

  // Transform the model into the volume's IJK space
  vtkNew<vtkTransformFilter> modelTransformerRasToIjk;
  vtkNew<vtkTransform> transformRasToIjk;
  transformRasToIjk->SetMatrix(readerVol->GetRasToIjkMatrix());
  modelTransformerRasToIjk->SetTransform(transformRasToIjk);
  modelTransformerRasToIjk->SetInputConnection(modelNode->GetMeshConnection());

  vtkNew<vtkProbeFilter> probe;
  probe->SetSourceData(volume_Ijk);
  probe->SetInputConnection(modelTransformerRasToIjk->GetOutputPort());

  // Transform the model back into RAS space
  vtkNew<vtkTransformFilter> modelTransformerIjkToRas;
  modelTransformerIjkToRas->SetTransform(transformRasToIjk->GetInverse());
  modelTransformerIjkToRas->SetInputConnection(probe->GetOutputPort());
  modelTransformerIjkToRas->Update();

  // Save the output
  modelNode->SetAndObserveMesh(modelTransformerIjkToRas->GetOutput());
  modelStorageNode->SetFileName(OutputModel.c_str());
  if (!modelStorageNode->WriteData(modelNode))
    {
    std::cerr << "Failed to write output model file " << OutputModel << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
