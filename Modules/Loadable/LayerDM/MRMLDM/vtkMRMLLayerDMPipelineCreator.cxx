#include "vtkMRMLLayerDMPipelineCreator.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

vtkStandardNewMacro(vtkMRMLLayerDMPipelineCreator);

vtkSmartPointer<vtkMRMLLayerDMPipeline> vtkMRMLLayerDMPipelineCreator::CreatePipeline(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node) const
{
  return {};
}
