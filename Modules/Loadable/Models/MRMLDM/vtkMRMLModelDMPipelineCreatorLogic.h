#pragma once

#include "vtkSlicerModelsModuleMRMLDisplayableManagerExport.h"

// VTK includes
#include <vtkObject.h>

class VTK_SLICER_MODELS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLModelDMPipelineCreatorLogic : public vtkObject
{
public:
  static vtkMRMLModelDMPipelineCreatorLogic* New();
  vtkTypeMacro(vtkMRMLModelDMPipelineCreatorLogic, vtkObject);

  static void RegisterPipelines();

protected:
  vtkMRMLModelDMPipelineCreatorLogic() = default;
  ~vtkMRMLModelDMPipelineCreatorLogic() override = default;
};
