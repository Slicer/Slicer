#include "vtkMRMLLayerDMPipelineCallbackCreator.h"

// Layer DM includes
#include "vtkMRMLLayerDMPipeline.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

vtkStandardNewMacro(vtkMRMLLayerDMPipelineCallbackCreator);

vtkSmartPointer<vtkMRMLLayerDMPipeline> vtkMRMLLayerDMPipelineCallbackCreator::CreatePipeline(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node) const
{
  return this->Callback(viewNode, node);
}

void vtkMRMLLayerDMPipelineCallbackCreator::SetCallback(const std::function<vtkSmartPointer<vtkMRMLLayerDMPipeline>(vtkMRMLAbstractViewNode*, vtkMRMLNode*)>& callback)
{
  this->Callback = callback;
}

vtkMRMLLayerDMPipelineCallbackCreator::vtkMRMLLayerDMPipelineCallbackCreator()
  : Callback([](vtkMRMLAbstractViewNode*, vtkMRMLNode*) { return nullptr; }) {};
