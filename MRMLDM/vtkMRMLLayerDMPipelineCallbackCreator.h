#pragma once

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

#include "vtkMRMLLayerDMPipelineCreatorI.h"

// STL includes
#include <functional>

/// \brief Lambda pipeline creator.
/// Delegates to its internal lambda when CreatePipeline is called.
///
/// \sa vtkMRMLLayerDMPipelineScriptedCreator
/// \sa vtkMRMLLayerDMPipelineFactory::AddPipelineCreator
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDMPipelineCallbackCreator : public vtkMRMLLayerDMPipelineCreatorI
{
public:
  static vtkMRMLLayerDMPipelineCallbackCreator* New();
  vtkTypeMacro(vtkMRMLLayerDMPipelineCallbackCreator, vtkMRMLLayerDMPipelineCreatorI);

  vtkSmartPointer<vtkMRMLLayerDMPipelineI> CreatePipeline(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node) const override;
  void SetCallback(const std::function<vtkSmartPointer<vtkMRMLLayerDMPipelineI>(vtkMRMLAbstractViewNode*, vtkMRMLNode*)>& callback);

protected:
  vtkMRMLLayerDMPipelineCallbackCreator();
  ~vtkMRMLLayerDMPipelineCallbackCreator() override = default;

private:
  std::function<vtkSmartPointer<vtkMRMLLayerDMPipelineI>(vtkMRMLAbstractViewNode*, vtkMRMLNode*)> m_callback;
};
