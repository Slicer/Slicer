#include "vtkMRMLLayerDMPipelineScriptedCreator.h"

// Layer DM includes
#include "vtkMRMLLayerDMPipelineI.h"
#include "vtkMRMLLayerDMPythonUtil.h"

// Slicer includes
#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLNode.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkPythonUtil.h>

vtkStandardNewMacro(vtkMRMLLayerDMPipelineScriptedCreator);

vtkMRMLLayerDMPipelineScriptedCreator::vtkMRMLLayerDMPipelineScriptedCreator()
  : m_object(nullptr)
{
  this->SetCallback(
    [this](vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node) -> vtkSmartPointer<vtkMRMLLayerDMPipelineI>
    {
      if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
      {
        return nullptr;
      }

      vtkPythonScopeGilEnsurer gilEnsurer;
      PyObject* result = vtkMRMLLayerDMPythonUtil::CallPythonObject(
        this->m_object, vtkMRMLLayerDMPythonUtil::ToPyArgs({ vtkMRMLLayerDMPythonUtil::ToPyObject(viewNode), vtkMRMLLayerDMPythonUtil::ToPyObject(node) }));
      if (!result)
      {
        auto errorMsg = std::string(__func__) + ": Failed to call : " + vtkMRMLLayerDMPythonUtil::GetObjectStr(this->m_object) + ":";
        vtkMRMLLayerDMPythonUtil::PrintErrorTraceback(this, errorMsg);
        return nullptr;
      }
      return vtkMRMLLayerDMPipelineI::SafeDownCast(vtkPythonUtil::GetPointerFromObject(result, "vtkMRMLLayerDMPipelineI"));
    });
}

vtkMRMLLayerDMPipelineScriptedCreator::~vtkMRMLLayerDMPipelineScriptedCreator()
{
  vtkMRMLLayerDMPythonUtil::DeletePythonObject(&this->m_object);
}

void vtkMRMLLayerDMPipelineScriptedCreator::SetPythonCallback(PyObject* object)
{
  vtkMRMLLayerDMPythonUtil::SetPythonObject(&this->m_object, object);
}
