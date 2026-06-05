#include "vtkMRMLModelDMPipelineCreatorLogic.h"

// Module includes
#include "vtkMRMLModelPickingNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDMPipeline.h"
#include "vtkMRMLModelPickingDMPipeline.h"

// LayerDM includes
#include "vtkMRMLLayerDMPipelineCreateHelper.h"
#include "vtkMRMLLayerDMPipelineCreatorI.h"
#include "vtkMRMLLayerDMPipelineFactory.h"
#include "vtkSlicerLayerDMLogic.h"

// MRML includes
#include "vtkMRMLViewNode.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

vtkStandardNewMacro(vtkMRMLModelDMPipelineCreatorLogic);

void vtkMRMLModelDMPipelineCreatorLogic::RegisterPipelines()
{
  static vtkSmartPointer<vtkMRMLLayerDMPipelineCreatorI> creator{ nullptr };
  if (creator)
  {
    return;
  }

  creator = vtkMRMLLayerDMPipelineFactory::GetInstance()->AddPipelineCreator(
    [](vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* displayNode)
    {
      return layer_dm::TryCreateForView< //
        vtkMRMLViewNode,
        vtkMRMLModelDisplayNode,
        vtkMRMLModelDMPipeline,
        vtkMRMLModelPickingNode,
        vtkMRMLModelPickingDMPipeline>(viewNode, displayNode);
    });
}
