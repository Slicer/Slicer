#pragma once

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

#include "vtkMRMLLayerDMPipelineI.h"

// VTK includes
#include <vtkObject.h>

class vtkMRMLAbstractViewNode;
class vtkMRMLNode;

/// \brief Interface responsible for creating new pipelines given input pairs of viewNode and node.
///
/// \sa vtkMRMLLayerDMPipelineCallbackCreator
/// \sa vtkMRMLLayerDMPipelineFactory::AddPipelineCreator
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDMPipelineCreatorI : public vtkObject
{
public:
  static vtkMRMLLayerDMPipelineCreatorI* New();
  vtkTypeMacro(vtkMRMLLayerDMPipelineCreatorI, vtkObject);

  virtual vtkSmartPointer<vtkMRMLLayerDMPipelineI> CreatePipeline(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node) const;

  /// @{
  /// \brief Get/Set the priority of creation of the pipeline creator.
  /// Pipelines with higher priority will be allowed to handle creation first (allowing to supplant existing creators).
  vtkGetMacro(Priority, int);
  vtkSetMacro(Priority, int);
  /// @}
protected:
  vtkMRMLLayerDMPipelineCreatorI() = default;
  ~vtkMRMLLayerDMPipelineCreatorI() override = default;

private:
  int Priority = 0;
};
