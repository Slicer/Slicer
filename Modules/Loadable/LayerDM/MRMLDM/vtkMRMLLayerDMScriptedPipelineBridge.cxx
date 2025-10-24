#include "vtkMRMLLayerDMScriptedPipelineBridge.h"

// Layer DM includes
#include "vtkMRMLLayerDMPipelineManager.h"

// Slicer includes
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLLayerDMPythonUtil.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCamera.h>
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkPythonUtil.h>
#include <vtkRenderer.h>

vtkStandardNewMacro(vtkMRMLLayerDMScriptedPipelineBridge);

void vtkMRMLLayerDMScriptedPipelineBridge::UpdatePipeline()
{
  if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
  {
    return;
  }

  this->CallPythonMethod({}, __func__, true);
}

PyObject* vtkMRMLLayerDMScriptedPipelineBridge::CastCallData(PyObject* object, int vtkType)
{
  return vtkMRMLLayerDMPythonUtil::CastCallData(object, vtkType);
}

vtkMRMLLayerDMScriptedPipelineBridge::vtkMRMLLayerDMScriptedPipelineBridge()
  : m_object{ nullptr }
{
}

vtkMRMLLayerDMScriptedPipelineBridge::~vtkMRMLLayerDMScriptedPipelineBridge()
{
  vtkMRMLLayerDMPythonUtil::DeletePythonObject(&this->m_object);
}

bool vtkMRMLLayerDMScriptedPipelineBridge::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)
{
  if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
  {
    return false;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (auto result = this->CallPythonMethod(vtkMRMLLayerDMPythonUtil::ToPyArgs(eventData), __func__, false))
  {
    int canProcess;
    if (PyTuple_Check(result) && PyArg_ParseTuple(result, "pd", &canProcess, &distance2))
    {
      Py_DECREF(result);
      return canProcess;
    }

    Py_DECREF(result);
    // Unpack error or unexpected return type
    PyErr_SetString(PyExc_TypeError, "Expected a tuple[bool, float] return type");
  }

  return false;
}

vtkCamera* vtkMRMLLayerDMScriptedPipelineBridge::GetCamera() const
{
  if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
  {
    return Superclass::GetCamera();
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  auto result = this->CallPythonMethod({}, __func__, false);
  if (result)
  {
    if (result != Py_None)
    {
      return vtkCamera::SafeDownCast(vtkPythonUtil::GetPointerFromObject(result, "vtkCamera"));
    }
    Py_DECREF(result);
  }
  return Superclass::GetCamera();
}

int vtkMRMLLayerDMScriptedPipelineBridge::GetMouseCursor() const
{
  if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
  {
    return Superclass::GetMouseCursor();
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (auto result = this->CallPythonMethod({}, __func__, false))
  {
    return CastToIntAndDecrement(result);
  }
  return Superclass::GetMouseCursor();
}

unsigned int vtkMRMLLayerDMScriptedPipelineBridge::GetRenderOrder() const
{
  if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
  {
    return Superclass::GetRenderOrder();
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (auto result = this->CallPythonMethod({}, __func__, false))
  {
    return CastToIntAndDecrement(result);
  }
  return Superclass::GetRenderOrder();
}

int vtkMRMLLayerDMScriptedPipelineBridge::GetWidgetState() const
{
  if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
  {
    return Superclass::GetWidgetState();
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (auto result = this->CallPythonMethod({}, __func__, false))
  {
    return CastToIntAndDecrement(result);
  }
  return Superclass::GetWidgetState();
}

void vtkMRMLLayerDMScriptedPipelineBridge::LoseFocus(vtkMRMLInteractionEventData* eventData)
{
  if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(vtkMRMLLayerDMPythonUtil::ToPyArgs(eventData), __func__, true);
}

void vtkMRMLLayerDMScriptedPipelineBridge::OnDefaultCameraModified(vtkCamera* camera)
{
  if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(vtkMRMLLayerDMPythonUtil::ToPyArgs(camera), __func__, true);
}

void vtkMRMLLayerDMScriptedPipelineBridge::OnRendererAdded(vtkRenderer* renderer)
{
  if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(vtkMRMLLayerDMPythonUtil::ToPyArgs(renderer), __func__, true);
}

void vtkMRMLLayerDMScriptedPipelineBridge::OnRendererRemoved(vtkRenderer* renderer)
{
  if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(vtkMRMLLayerDMPythonUtil::ToPyArgs(renderer), __func__, true);
}

bool vtkMRMLLayerDMScriptedPipelineBridge::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
  {
    return false;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  if (auto result = this->CallPythonMethod(vtkMRMLLayerDMPythonUtil::ToPyArgs(eventData), __func__, false))
  {
    bool wasProcessed = result == Py_True;
    Py_DECREF(result);
    return wasProcessed;
  }
  return false;
}

void vtkMRMLLayerDMScriptedPipelineBridge::SetDisplayNode(vtkMRMLNode* displayNode)
{
  if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(vtkMRMLLayerDMPythonUtil::ToPyArgs(displayNode), __func__, true);
}

void vtkMRMLLayerDMScriptedPipelineBridge::SetViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(vtkMRMLLayerDMPythonUtil::ToPyArgs(viewNode), __func__, true);
}

void vtkMRMLLayerDMScriptedPipelineBridge::SetScene(vtkMRMLScene* scene)
{
  if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(vtkMRMLLayerDMPythonUtil::ToPyArgs(scene), __func__, true);
}

void vtkMRMLLayerDMScriptedPipelineBridge::SetPipelineManager(vtkMRMLLayerDMPipelineManager* pipelineManager)
{
  if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(vtkMRMLLayerDMPythonUtil::ToPyArgs(pipelineManager), __func__, true);
}

void vtkMRMLLayerDMScriptedPipelineBridge::SetPythonObject(PyObject* object)
{
  vtkMRMLLayerDMPythonUtil::SetPythonObject(&this->m_object, object);
}

void vtkMRMLLayerDMScriptedPipelineBridge::OnUpdate(vtkObject* obj, unsigned long eventId, void* callData)
{
  if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
  {
    return;
  }

  vtkPythonScopeGilEnsurer gilEnsurer;
  this->CallPythonMethod(vtkMRMLLayerDMPythonUtil::ToPyArgs(obj, eventId, callData), __func__, true);
}

PyObject* vtkMRMLLayerDMScriptedPipelineBridge::CallPythonMethod(const vtkSmartPyObject& pyArgs, const std::string& fName, bool decrementResult) const
{
  auto result = vtkMRMLLayerDMPythonUtil::CallPythonMethod(this->m_object, pyArgs, fName);

  if (!result)
  {
    std::string errorMsg = "Failed to call : " + fName + " : of object : " + vtkMRMLLayerDMPythonUtil::GetObjectStr(this->m_object) + ":";
    vtkMRMLLayerDMPythonUtil::PrintErrorTraceback(this, errorMsg);
    return nullptr;
  }
  else if (decrementResult)
  {
    Py_DECREF(result);
    return nullptr;
  }

  return result;
}

int vtkMRMLLayerDMScriptedPipelineBridge::CastToIntAndDecrement(PyObject* result) const
{
  if (!result)
  {
    return {};
  }
  auto value = PyLong_AsLong(result);
  Py_DECREF(result);
  return value;
}