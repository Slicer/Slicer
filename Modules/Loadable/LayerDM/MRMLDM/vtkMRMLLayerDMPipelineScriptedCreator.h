#pragma once

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

#include "vtkMRMLLayerDMPipelineCallbackCreator.h"

// VTK includes
#include <vtkPython.h>

/// Python lambda implementation of \sa vtkMRMLLayerDMPipelineCallbackCreator
/// Delegates callback to underlying Python callable object.
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDMPipelineScriptedCreator : public vtkMRMLLayerDMPipelineCallbackCreator
{
public:
  static vtkMRMLLayerDMPipelineScriptedCreator* New();

  vtkTypeMacro(vtkMRMLLayerDMPipelineScriptedCreator, vtkMRMLLayerDMPipelineCallbackCreator);
  void SetPythonCallback(PyObject* object);

protected:
  vtkMRMLLayerDMPipelineScriptedCreator();
  ~vtkMRMLLayerDMPipelineScriptedCreator() override;

private:
  PyObject* m_object;
};
