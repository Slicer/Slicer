#include "vtkMRMLLayerDMPipelineCallbackCreator.h"

// Layer DM includes
#include "vtkMRMLLayerDMPipelineI.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

vtkStandardNewMacro(vtkMRMLLayerDMPipelineCallbackCreator);

vtkSmartPointer<vtkMRMLLayerDMPipelineI> vtkMRMLLayerDMPipelineCallbackCreator::CreatePipeline(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node) const
{
  return this->m_callback(viewNode, node);
}

void vtkMRMLLayerDMPipelineCallbackCreator::SetCallback(const std::function<vtkSmartPointer<vtkMRMLLayerDMPipelineI>(vtkMRMLAbstractViewNode*, vtkMRMLNode*)>& callback)
{
  this->m_callback = callback;
}

vtkMRMLLayerDMPipelineCallbackCreator::vtkMRMLLayerDMPipelineCallbackCreator()
  : m_callback([](vtkMRMLAbstractViewNode*, vtkMRMLNode*) { return nullptr; }) {};
