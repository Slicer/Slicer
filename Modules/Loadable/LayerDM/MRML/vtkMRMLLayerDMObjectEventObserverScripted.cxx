#include "vtkMRMLLayerDMObjectEventObserverScripted.h"

#include "vtkMRMLLayerDMPythonUtil.h"
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkMRMLLayerDMObjectEventObserverScripted);

vtkMRMLLayerDMObjectEventObserverScripted::vtkMRMLLayerDMObjectEventObserverScripted()
  : Object(nullptr)
{
  this->SetUpdateCallback(
    [this](vtkObject* node, unsigned long eventId, void* callData) -> void
    {
      if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
      {
        return;
      }

      vtkPythonScopeGilEnsurer gilEnsurer;
      if (auto result = vtkMRMLLayerDMPythonUtil::CallPythonObject(this->Object, vtkMRMLLayerDMPythonUtil::ToPyArgs(node, eventId, callData)))
      {
        Py_DECREF(result);
      }
      else
      {
        auto errorMsg = std::string(__func__) + ": Failed to call : " + vtkMRMLLayerDMPythonUtil::GetObjectStr(this->Object) + ":";
        vtkMRMLLayerDMPythonUtil::PrintErrorTraceback(this, errorMsg);
      }
    });
}

vtkMRMLLayerDMObjectEventObserverScripted::~vtkMRMLLayerDMObjectEventObserverScripted()
{
  vtkMRMLLayerDMPythonUtil::DeletePythonObject(&this->Object);
}

void vtkMRMLLayerDMObjectEventObserverScripted::SetPythonCallback(PyObject* object)
{
  vtkMRMLLayerDMPythonUtil::SetPythonObject(&this->Object, object);
}

PyObject* vtkMRMLLayerDMObjectEventObserverScripted::CastCallData(PyObject* object, int vtkType)
{
  return vtkMRMLLayerDMPythonUtil::CastCallData(object, vtkType);
}
