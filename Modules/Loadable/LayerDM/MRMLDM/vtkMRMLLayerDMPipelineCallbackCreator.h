#pragma once

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

#include "vtkMRMLLayerDMPipelineCreator.h"

// STL includes
#include <functional>

/// \brief Lambda pipeline creator.
/// Delegates to its internal lambda when CreatePipeline is called.
///
/// \sa vtkMRMLLayerDMPipelineScriptedCreator
/// \sa vtkMRMLLayerDMPipelineFactory::AddPipelineCreator
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDMPipelineCallbackCreator : public vtkMRMLLayerDMPipelineCreator
{
public:
  static vtkMRMLLayerDMPipelineCallbackCreator* New();
  vtkTypeMacro(vtkMRMLLayerDMPipelineCallbackCreator, vtkMRMLLayerDMPipelineCreator);

  vtkSmartPointer<vtkMRMLLayerDMPipeline> CreatePipeline(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node) const override;
  void SetCallback(const std::function<vtkSmartPointer<vtkMRMLLayerDMPipeline>(vtkMRMLAbstractViewNode*, vtkMRMLNode*)>& callback);

protected:
  vtkMRMLLayerDMPipelineCallbackCreator();
  ~vtkMRMLLayerDMPipelineCallbackCreator() override = default;

private:
  std::function<vtkSmartPointer<vtkMRMLLayerDMPipeline>(vtkMRMLAbstractViewNode*, vtkMRMLNode*)> m_callback;
};
