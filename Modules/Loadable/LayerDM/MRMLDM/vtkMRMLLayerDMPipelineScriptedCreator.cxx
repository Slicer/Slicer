#include "vtkMRMLLayerDMPipelineScriptedCreator.h"

// Layer DM includes
#include "vtkMRMLLayerDMPipeline.h"
#include "vtkMRMLLayerDMPythonUtil.h"

// Slicer includes
#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLNode.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkPythonUtil.h>

vtkStandardNewMacro(vtkMRMLLayerDMPipelineScriptedCreator);

vtkMRMLLayerDMPipelineScriptedCreator::vtkMRMLLayerDMPipelineScriptedCreator()
  : Object(nullptr)
{
  this->SetCallback(
    [this](vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node) -> vtkSmartPointer<vtkMRMLLayerDMPipeline>
    {
      if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
      {
        return nullptr;
      }

      vtkPythonScopeGilEnsurer gilEnsurer;
      PyObject* result = vtkMRMLLayerDMPythonUtil::CallPythonObject(
        this->Object, vtkMRMLLayerDMPythonUtil::ToPyArgs({ vtkMRMLLayerDMPythonUtil::ToPyObject(viewNode), vtkMRMLLayerDMPythonUtil::ToPyObject(node) }));
      if (!result)
      {
        auto errorMsg = std::string(__func__) + ": Failed to call : " + vtkMRMLLayerDMPythonUtil::GetObjectStr(this->Object) + ":";
        vtkMRMLLayerDMPythonUtil::PrintErrorTraceback(this, errorMsg);
        return nullptr;
      }
      return vtkMRMLLayerDMPipeline::SafeDownCast(vtkPythonUtil::GetPointerFromObject(result, "vtkMRMLLayerDMPipeline"));
    });
}

vtkMRMLLayerDMPipelineScriptedCreator::~vtkMRMLLayerDMPipelineScriptedCreator()
{
  vtkMRMLLayerDMPythonUtil::DeletePythonObject(&this->Object);
}

void vtkMRMLLayerDMPipelineScriptedCreator::SetPythonCallback(PyObject* object)
{
  vtkMRMLLayerDMPythonUtil::SetPythonObject(&this->Object, object);
}
