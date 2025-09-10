#pragma once

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// STL includes
#include <vector>

class vtkMRMLLayerDMPipelineI;
class vtkMRMLInteractionEventData;
class vtkMRMLAbstractViewNode;

/// \brief Pipeline manager interaction logic class
///
/// This class is responsible for sorting the pipelines depending on their priorities and delegating interactions
/// to the pipeline the highest in the priority list.
///
/// Priorities are ordered by :
///   - Widget State if state is greater than WidgetStateOnWidget (indicates previously active display pipeline)
///   - Pipeline layer (higher = overlay on top of other renderers)
///   - Distance to interaction (min = closer to VTK event)
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDMInteractionLogic : public vtkObject
{
public:
  static vtkMRMLLayerDMInteractionLogic* New();
  vtkTypeMacro(vtkMRMLLayerDMInteractionLogic, vtkObject);

  void AddPipeline(const vtkSmartPointer<vtkMRMLLayerDMPipelineI>& pipeline);
  bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2);
  std::vector<vtkSmartPointer<vtkMRMLLayerDMPipelineI>> GetCanProcessPipelines() const;
  vtkMRMLLayerDMPipelineI* GetLastFocusedPipeline() const;
  void LoseFocus(vtkMRMLInteractionEventData* eventData);
  void LoseFocus();
  bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData);
  void RemovePipeline(const vtkSmartPointer<vtkMRMLLayerDMPipelineI>& pipeline);
  void SetViewNode(vtkMRMLAbstractViewNode* viewNode);

protected:
  vtkMRMLLayerDMInteractionLogic();
  ~vtkMRMLLayerDMInteractionLogic() override = default;

private:
  static int MinWidgetState();
  std::tuple<double, int> PrioritizeCanProcessPipelines(vtkMRMLInteractionEventData* eventData);
  void LosePreviousFocusInCannotProcess(vtkMRMLInteractionEventData* eventData);

  std::vector<vtkSmartPointer<vtkMRMLLayerDMPipelineI>> m_pipelines;
  vtkSmartPointer<vtkMRMLLayerDMPipelineI> m_prevFocusedPipeline;
  std::vector<vtkSmartPointer<vtkMRMLLayerDMPipelineI>> m_canProcess;
  vtkWeakPointer<vtkMRMLAbstractViewNode> m_viewNode;
};
