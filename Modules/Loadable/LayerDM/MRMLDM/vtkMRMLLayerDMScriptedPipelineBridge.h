#pragma once

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

#include "vtkMRMLLayerDMPipelineI.h"

// VTK includes
#include <vtkPython.h>

class vtkSmartPyObject;

/// \brief Python bridge for vtkMRMLLayerDMPipelineI.
/// Delegates calls to the pipeline to its underlying python object.
///
/// \sa vtkMRMLLayerDMPipelineI
/// \sa vtkMRMLLayerDMScriptedPipeline
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDMScriptedPipelineBridge : public vtkMRMLLayerDMPipelineI
{
public:
  static vtkMRMLLayerDMScriptedPipelineBridge* New();
  vtkTypeMacro(vtkMRMLLayerDMScriptedPipelineBridge, vtkMRMLLayerDMPipelineI);

  bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2) override;
  vtkCamera* GetCamera() const override;
  int GetMouseCursor() const override;
  unsigned int GetRenderOrder() const override;
  int GetWidgetState() const override;
  void LoseFocus(vtkMRMLInteractionEventData* eventData) override;
  void OnDefaultCameraModified(vtkCamera* camera) override;
  void OnRendererAdded(vtkRenderer* renderer) override;
  void OnRendererRemoved(vtkRenderer* renderer) override;
  bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) override;
  void SetDisplayNode(vtkMRMLNode* displayNode) override;
  void SetViewNode(vtkMRMLAbstractViewNode* viewNode) override;
  void SetScene(vtkMRMLScene* scene) override;
  void SetPipelineManager(vtkMRMLLayerDMPipelineManager* pipelineManager) override;
  void SetPythonObject(PyObject* object);
  void UpdatePipeline() override;
  static PyObject* CastCallData(PyObject* object, int vtkType);

protected:
  vtkMRMLLayerDMScriptedPipelineBridge();
  ~vtkMRMLLayerDMScriptedPipelineBridge() override;

  void OnUpdate(vtkObject* obj, unsigned long eventId, void* callData) override;

private:
  PyObject* CallPythonMethod(const vtkSmartPyObject& pyArgs, const std::string& fName, bool decrementResult) const;
  int CastToIntAndDecrement(PyObject* result) const;

  PyObject* m_object;
};
