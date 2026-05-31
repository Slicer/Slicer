#pragma once

// Export includes
#include "vtkSlicerModelsModuleMRMLDisplayableManagerExport.h"

// LayerDM includes
#include "vtkMRMLLayerDMPipelineI.h"

// STL includes
#include <map>

class vtkProp;
class vtkCellPicker;
class vtkMRMLModelDisplayNode;
class vtkMRMLModelPickingNode;
class vtkPropPicker;

/// \brief Pipeline for handling model picking in a view.
class VTK_SLICER_MODELS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLModelPickingDMPipeline : public vtkMRMLLayerDMPipelineI
{
public:
  static vtkMRMLModelPickingDMPipeline* New();
  vtkTypeMacro(vtkMRMLModelPickingDMPipeline, vtkMRMLLayerDMPipelineI);

  void SetPipelineManager(vtkMRMLLayerDMPipelineManager* pipelineManager) override;
  bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2) override;

protected:
  vtkMRMLModelPickingDMPipeline();
  ~vtkMRMLModelPickingDMPipeline() override;

  void OnUpdate(vtkObject* obj, unsigned long eventId, void* callData) override;

private:
  vtkMRMLModelPickingDMPipeline(const vtkMRMLModelPickingDMPipeline&) = delete;
  void operator=(const vtkMRMLModelPickingDMPipeline&) = delete;

  vtkMRMLModelPickingNode* GetPickingNode() const;
  void UpdateActorToDisplayNodeMap();

  vtkSmartPointer<vtkPropPicker> PropPicker;
  vtkSmartPointer<vtkCellPicker> CellPicker;
  std::map<vtkProp*, vtkMRMLModelDisplayNode*> ActorToDisplayNodeMap;
};
