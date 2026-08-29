#pragma once

#include "vtkSlicerLayerDMModuleMRMLDisplayableManagerExport.h"

#include "vtkMRMLLayerDMPipeline.h"

// VTK includes
#include <vtkObject.h>

class vtkMRMLAbstractViewNode;
class vtkMRMLNode;

/// \brief Interface responsible for creating new pipelines given input pairs of viewNode and node.
///
/// \sa vtkMRMLLayerDMPipelineCallbackCreator
/// \sa vtkMRMLLayerDMPipelineFactory::AddPipelineCreator
class VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLLayerDMPipelineCreator : public vtkObject
{
public:
  static vtkMRMLLayerDMPipelineCreator* New();
  vtkTypeMacro(vtkMRMLLayerDMPipelineCreator, vtkObject);

  virtual vtkSmartPointer<vtkMRMLLayerDMPipeline> CreatePipeline(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node) const;

  /// @{
  /// \brief Get/Set the priority of creation of the pipeline creator.
  /// Pipelines with higher priority will be allowed to handle creation first (allowing to supplant existing creators).
  vtkGetMacro(Priority, int);
  vtkSetMacro(Priority, int);
  /// @}
protected:
  vtkMRMLLayerDMPipelineCreator() = default;
  ~vtkMRMLLayerDMPipelineCreator() override = default;

private:
  int Priority = 0;
};
