#include "vtkMRMLLayerDMPipelineScriptedCreator.h"

// Layer DM includes
#include "vtkMRMLLayerDMPipelineI.h"

// Slicer includes
#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLNode.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkPythonUtil.h>
#include <vtkSmartPointer.h>
#include <vtkSmartPyObject.h>

vtkStandardNewMacro(vtkMRMLLayerDMPipelineScriptedCreator);

vtkMRMLLayerDMPipelineScriptedCreator::vtkMRMLLayerDMPipelineScriptedCreator()
  : m_object(nullptr)
{
  this->SetCallback(
    [this](vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node) -> vtkSmartPointer<vtkMRMLLayerDMPipelineI>
    {
      if (!Py_IsInitialized() || !this->m_object || !PyCallable_Check(this->m_object))
      {
        return nullptr;
      }

      vtkPythonScopeGilEnsurer gilEnsurer;

      PyObject* pyViewNode = vtkPythonUtil::GetObjectFromPointer(viewNode);
      PyObject* pyNode = vtkPythonUtil::GetObjectFromPointer(node);
      vtkSmartPyObject pyArgs(PyTuple_Pack(2, pyViewNode, pyNode));

      Py_XDECREF(pyViewNode);
      Py_XDECREF(pyNode);
      PyObject* result = PyObject_CallObject(this->m_object, pyArgs);
      if (!result)
      {
        PyErr_Print();
        return nullptr;
      }
      return vtkMRMLLayerDMPipelineI::SafeDownCast(vtkPythonUtil::GetPointerFromObject(result, "vtkMRMLLayerDMPipelineI"));
    });
}

vtkMRMLLayerDMPipelineScriptedCreator::~vtkMRMLLayerDMPipelineScriptedCreator()
{
  if (Py_IsInitialized())
  {
    vtkPythonScopeGilEnsurer gilEnsurer;
    Py_XDECREF(this->m_object);
  }
}

void vtkMRMLLayerDMPipelineScriptedCreator::SetPythonCallback(PyObject* object)
{
  if (!Py_IsInitialized())
  {
    vtkErrorMacro("" << __func__ << "Python environment is not initialized correctly. Failed to set callback : " << object);
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (this->m_object == object)
  {
    return;
  }

  // Set the new python lambda
  Py_XDECREF(this->m_object);
  this->m_object = object;
  Py_INCREF(this->m_object);
}
