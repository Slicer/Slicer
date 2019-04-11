/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Python includes
#include <vtkPython.h>

// MRMLDisplayableManager includes
#include "vtkMRMLScriptedDisplayableManager.h"

// MRML includes
#include <vtkMRMLViewNode.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkPythonUtil.h>
#include <vtkSmartPointer.h>

// STD includes

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLScriptedDisplayableManager );

//---------------------------------------------------------------------------
class vtkMRMLScriptedDisplayableManager::vtkInternal
{
public:
  vtkInternal();
  ~vtkInternal();

  enum {
    CreateMethod = 0,
    GetMRMLSceneEventsToObserveMethod,
    ProcessMRMLSceneEventsMethod,
    ProcessMRMLNodesEventsMethod,
    RemoveMRMLObserversMethod,
    UpdateFromMRMLMethod,
    OnMRMLDisplayableNodeModifiedEventMethod,
    OnInteractorStyleEventMethod,
    };

  static int          APIMethodCount;
  static const char * APIMethodNames[8];

  std::string  PythonSource;
  PyObject *   PythonSelf;
  PyObject *   PythonAPIMethods[8];
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
int vtkMRMLScriptedDisplayableManager::vtkInternal::APIMethodCount = 8;

//---------------------------------------------------------------------------
const char* vtkMRMLScriptedDisplayableManager::vtkInternal::APIMethodNames[8] =
{
  "Create",
  "GetMRMLSceneEventsToObserve",
  "ProcessMRMLSceneEvents",
  "ProcessMRMLNodesEvents",
  "RemoveMRMLObservers",
  "UpdateFromMRML",
  "OnMRMLDisplayableNodeModifiedEvent",
  "OnInteractorStyleEvent",
};

//---------------------------------------------------------------------------
vtkMRMLScriptedDisplayableManager::vtkInternal::vtkInternal()
{
  this->PythonSelf = nullptr;
  for (int i = 0; i < vtkInternal::APIMethodCount; ++i)
    {
    this->PythonAPIMethods[i] = nullptr;
    }
}

//---------------------------------------------------------------------------
vtkMRMLScriptedDisplayableManager::vtkInternal::~vtkInternal()
{
  if (this->PythonSelf)
    {
    for (int i = 0; i < vtkInternal::APIMethodCount; ++i)
      {
      Py_XDECREF(this->PythonAPIMethods[i]);
      }
    Py_DECREF(this->PythonSelf);
    }
}

//---------------------------------------------------------------------------
// vtkMRMLScriptedDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLScriptedDisplayableManager::vtkMRMLScriptedDisplayableManager()
{
  this->Internal = new vtkInternal;
}

//---------------------------------------------------------------------------
vtkMRMLScriptedDisplayableManager::~vtkMRMLScriptedDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLScriptedDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLScriptedDisplayableManager::Create()
{
  PyObject * method = this->Internal->PythonAPIMethods[vtkInternal::CreateMethod];
  if (!method)
    {
    return;
    }
  PyObject_CallObject(method, nullptr);
  PyErr_Print();
}

//---------------------------------------------------------------------------
void vtkMRMLScriptedDisplayableManager::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkIntArray * sceneEventsAsPointer = nullptr;

  // Obtain list of event to listen
  PyObject* method =
      this->Internal->PythonAPIMethods[vtkInternal::GetMRMLSceneEventsToObserveMethod];
  if (method)
    {
    sceneEventsAsPointer = vtkIntArray::SafeDownCast(
        vtkPythonUtil::GetPointerFromObject(PyObject_CallObject(method, nullptr), "vtkIntArray"));
    PyErr_Print();
    }
  vtkSmartPointer<vtkIntArray> sceneEvents;
  sceneEvents.TakeReference(sceneEventsAsPointer);
  //for(int i = 0; i < sceneEvents->GetNumberOfTuples(); i++)
  //  {
  //  std::cout << "eventid:" << sceneEvents->GetValue(i) << std::endl;
  //  }
  this->SetAndObserveMRMLSceneEventsInternal(newScene, sceneEvents);
}

//---------------------------------------------------------------------------
void vtkMRMLScriptedDisplayableManager
::ProcessMRMLSceneEvents(vtkObject *caller,
                         unsigned long event,
                         void *callData)
{
  PyObject* method = this->Internal->PythonAPIMethods[vtkInternal::ProcessMRMLSceneEventsMethod];
  if (!method)
    {
    return;
    }

  PyObject * arguments = PyTuple_New(3);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(caller));
  PyTuple_SET_ITEM(arguments, 1, PyInt_FromLong(event));
  PyTuple_SET_ITEM(arguments, 2,
                   vtkPythonUtil::GetObjectFromPointer(reinterpret_cast<vtkMRMLNode*>(callData)));

  PyObject_CallObject(method, arguments);
  PyErr_Print();

  Py_DECREF(arguments);
}

//---------------------------------------------------------------------------
void vtkMRMLScriptedDisplayableManager::ProcessMRMLNodesEvents(vtkObject *caller,
                                                        unsigned long event,
                                                        void *vtkNotUsed(callData))
{
  PyObject* method = this->Internal->PythonAPIMethods[vtkInternal::ProcessMRMLNodesEventsMethod];
  if (!method)
    {
    return;
    }

  PyObject * arguments = PyTuple_New(3);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(caller));
  PyTuple_SET_ITEM(arguments, 1, PyInt_FromLong(event));
  PyTuple_SET_ITEM(arguments, 2, vtkPythonUtil::GetObjectFromPointer(nullptr));

  PyObject_CallObject(method, arguments);
  PyErr_Print();

  Py_DECREF(arguments);
}

//---------------------------------------------------------------------------
void vtkMRMLScriptedDisplayableManager::RemoveMRMLObservers()
{
  PyObject* method = this->Internal->PythonAPIMethods[vtkInternal::RemoveMRMLObserversMethod];
  if (!method)
    {
    return;
    }
  PyObject_CallObject(method, nullptr);
  PyErr_Print();

  this->Superclass::RemoveMRMLObservers();
}

//---------------------------------------------------------------------------
void vtkMRMLScriptedDisplayableManager::UpdateFromMRML()
{
  PyObject* method = this->Internal->PythonAPIMethods[vtkInternal::UpdateFromMRMLMethod];
  if (!method)
    {
    return;
    }
  PyObject_CallObject(method, nullptr);
  PyErr_Print();
}

//---------------------------------------------------------------------------
void vtkMRMLScriptedDisplayableManager::OnInteractorStyleEvent(int eventid)
{
  PyObject* method = this->Internal->PythonAPIMethods[vtkInternal::OnInteractorStyleEventMethod];
  if (!method)
    {
    return;
    }

  PyObject * arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, PyInt_FromLong(eventid));

  PyObject_CallObject(method, arguments);
  PyErr_Print();

  Py_DECREF(arguments);
}

//---------------------------------------------------------------------------
void vtkMRMLScriptedDisplayableManager::OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller)
{
  PyObject* method =
      this->Internal->PythonAPIMethods[vtkInternal::OnMRMLDisplayableNodeModifiedEventMethod];
  if (!method)
    {
    return;
    }

  PyObject * arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(caller));

  PyObject_CallObject(method, arguments);
  PyErr_Print();

  Py_DECREF(arguments);
}

//---------------------------------------------------------------------------
void vtkMRMLScriptedDisplayableManager::SetPythonSource(const std::string& pythonSource)
{
  if(pythonSource.find(".py") == std::string::npos &&
     pythonSource.find(".pyc") == std::string::npos)
    {
    return;
    }

  // Extract filename - It should match the associated python class
  std::string className = vtksys::SystemTools::GetFilenameWithoutExtension(pythonSource);
  //std::cout << "SetPythonSource - className:" << className << std::endl;

  // Get a reference to the main module and global dictionary
  PyObject * main_module = PyImport_AddModule("__main__");
  PyObject * global_dict = PyModule_GetDict(main_module);

  // Load class definition if needed
  PyObject * classToInstantiate = PyDict_GetItemString(global_dict, className.c_str());
  if (!classToInstantiate)
    {
    PyObject * pyRes = nullptr;
    if (pythonSource.find(".py") != std::string::npos)
      {
      std::string pyRunStr = std::string("execfile('") + pythonSource + std::string("')");
      pyRes = PyRun_String(pyRunStr.c_str(),
                           Py_file_input, global_dict, global_dict);
      }
    else if (pythonSource.find(".pyc") != std::string::npos)
      {
      std::string pyRunStr = std::string("with open('") + pythonSource +
          std::string("', 'rb') as f:import imp;imp.load_module('__main__', f, '") + pythonSource +
          std::string("', ('.pyc', 'rb', 2))");
      pyRes = PyRun_String(
            pyRunStr.c_str(),
            Py_file_input, global_dict, global_dict);
      }
    if (!pyRes)
      {
      vtkErrorMacro(<< "setPythonSource - Failed to execute file" << pythonSource << "!");
      return;
      }
    Py_DECREF(pyRes);
    classToInstantiate = PyDict_GetItemString(global_dict, className.c_str());
    }
  if (!classToInstantiate)
    {
    vtkErrorMacro(<< "SetPythonSource - Failed to load displayable manager class definition from "
                  << pythonSource);
    return;
    }

  //std::cout << "classToInstantiate:" << classToInstantiate << std::endl;

  PyObject * arguments = PyTuple_New(1);
  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(this));

  // Attempt to instantiate the associated python class
  PyObject * self = PyObject_CallObject(classToInstantiate, arguments);
  Py_DECREF(arguments);
  if (!self)
    {
    vtkErrorMacro(<< "SetPythonSource - Failed to instantiate displayable manager:"
                  << classToInstantiate);
    return;
    }

  // Retrieve API methods
  for (int i = 0; i < vtkInternal::APIMethodCount; ++i)
    {
    assert(vtkInternal::APIMethodNames[i]);
    if (!PyObject_HasAttrString(self, vtkInternal::APIMethodNames[i]))
      {
      continue;
      }
    PyObject * method = PyObject_GetAttrString(self, vtkInternal::APIMethodNames[i]);
    //std::cout << "method:" << method << std::endl;
    this->Internal->PythonAPIMethods[i] = method;
    }

  //std::cout << "self (" << className << ", instance:" << self << ")" << std::endl;

  this->Internal->PythonSource = pythonSource;
  this->Internal->PythonSelf = self;
}
