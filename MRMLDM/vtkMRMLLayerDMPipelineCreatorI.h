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

protected:
  vtkMRMLLayerDMPipelineCreatorI() = default;
  ~vtkMRMLLayerDMPipelineCreatorI() override = default;
};
