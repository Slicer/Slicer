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

// Slicer includes
#include "vtkSlicerScriptedLoadableModuleLogic.h"

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// VTK includes
#include <vtkPythonUtil.h>
#include <vtkStdString.h>

// STD includes
#include <cstdlib>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerScriptedLoadableModuleLogic);

//---------------------------------------------------------------------------
class vtkSlicerScriptedLoadableModuleLogic::vtkInternal
{
public:
  vtkInternal();
  ~vtkInternal();

//  enum {
//    GetMRMLSceneEventsToObserveMethod = 0,
//    ProcessMRMLSceneEventsMethod,
//    };

//  static int          APIMethodCount;
//  static const char * APIMethodNames[2];

  std::string  PythonSource;
  PyObject *   PythonSelf;
//  PyObject *   PythonAPIMethods[2];
};

//----------------------------------------------------------------------------
// vtkInternal methods

////---------------------------------------------------------------------------
//int vtkSlicerScriptedLoadableModuleLogic::vtkInternal::APIMethodCount = 2;

////---------------------------------------------------------------------------
//const char* vtkSlicerScriptedLoadableModuleLogic::vtkInternal::APIMethodNames[2] =
//{
//  "GetMRMLSceneEventsToObserve",
//  "ProcessMRMLSceneEvents",
//};

//---------------------------------------------------------------------------
vtkSlicerScriptedLoadableModuleLogic::vtkInternal::vtkInternal()
{
  this->PythonSelf = nullptr;
//  for (int i = 0; i < vtkInternal::APIMethodCount; ++i)
//    {
//    this->PythonAPIMethods[i] = 0;
//    }
}

//---------------------------------------------------------------------------
vtkSlicerScriptedLoadableModuleLogic::vtkInternal::~vtkInternal()
{
  if (this->PythonSelf)
    {
    Py_DECREF(this->PythonSelf);
    }
}

//----------------------------------------------------------------------------
// vtkSlicerScriptedLoadableModuleLogic methods

//----------------------------------------------------------------------------
vtkSlicerScriptedLoadableModuleLogic::vtkSlicerScriptedLoadableModuleLogic()
{
  this->Internal = new vtkInternal;
}

//----------------------------------------------------------------------------
vtkSlicerScriptedLoadableModuleLogic::~vtkSlicerScriptedLoadableModuleLogic()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkSlicerScriptedLoadableModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
}

////---------------------------------------------------------------------------
//void vtkSlicerScriptedLoadableModuleLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
//{
//  vtkIntArray * sceneEventsAsPointer = 0;

//  // Obtain list of event to listen
//  PyObject* method =
//      this->Internal->PythonAPIMethods[vtkInternal::GetMRMLSceneEventsToObserveMethod];
//  if (method)
//    {
//    sceneEventsAsPointer = vtkIntArray::SafeDownCast(
//        vtkPythonUtil::GetPointerFromObject(PyObject_CallObject(method, 0), "vtkIntArray"));
//    }
//  vtkSmartPointer<vtkIntArray> sceneEvents;
//  sceneEvents.TakeReference(sceneEventsAsPointer);
//  //for(int i = 0; i < sceneEvents->GetNumberOfTuples(); i++)
//  //  {
//  //  std::cout << "eventid:" << sceneEvents->GetValue(i) << std::endl;
//  //  }
//  this->SetAndObserveMRMLSceneEventsInternal(newScene, sceneEvents);
//}

////---------------------------------------------------------------------------
//void vtkSlicerScriptedLoadableModuleLogic::ProcessMRMLSceneEvents(vtkObject *caller,
//                                                             unsigned long event,
//                                                             void *callData)
//{
//  PyObject* method = this->Internal->PythonAPIMethods[vtkInternal::ProcessMRMLSceneEventsMethod];
//  if (!method)
//    {
//    return;
//    }

//  PyObject * arguments = PyTuple_New(3);
//  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(caller));
//  PyTuple_SET_ITEM(arguments, 1, PyInt_FromLong(event));
//  PyTuple_SET_ITEM(arguments, 2,
//                   vtkPythonUtil::GetObjectFromPointer(reinterpret_cast<vtkMRMLNode*>(callData)));

//  PyObject_CallObject(method, arguments);

//  Py_DECREF(arguments);
//}

////---------------------------------------------------------------------------
//void vtkSlicerScriptedLoadableModuleLogic::ProcessMRMLNodesEvents(vtkObject *caller,
//                                                              unsigned long event,
//                                                              void *callData)
//{
//  PyObject* method = this->Internal->PythonAPIMethods[vtkInternal::ProcessMRMLNodesEventsMethod];
//  if (!method)
//    {
//    return;
//    }

//  PyObject * arguments = PyTuple_New(3);
//  PyTuple_SET_ITEM(arguments, 0, vtkPythonUtil::GetObjectFromPointer(caller));
//  PyTuple_SET_ITEM(arguments, 1, PyInt_FromLong(event));
//  PyTuple_SET_ITEM(arguments, 2,
//                   vtkPythonUtil::GetObjectFromPointer(reinterpret_cast<vtkMRMLNode*>(callData)));

//  PyObject_CallObject(method, arguments);

//  Py_DECREF(arguments);
//}

//---------------------------------------------------------------------------
bool vtkSlicerScriptedLoadableModuleLogic::SetPythonSource(const std::string& pythonSource)
{
  if(pythonSource.find(".py") == std::string::npos &&
     pythonSource.find(".pyc") == std::string::npos)
    {
    return false;
    }

  // Extract filename - It should match the associated python class
  std::string className = vtksys::SystemTools::GetFilenameWithoutExtension(pythonSource);
  className+= "Logic";
  //std::cout << "SetPythonSource - className:" << className << std::endl;

  // Get a reference to the main module and global dictionary
  PyObject * main_module = PyImport_AddModule("__main__");
  PyObject * global_dict = PyModule_GetDict(main_module);

  // Load class definition if needed
  PyObject * classToInstantiate = PyDict_GetItemString(global_dict, className.c_str());
  if (!classToInstantiate)
    {
    PyObject * pyRes = nullptr;
    if (pythonSource.find(".pyc") != std::string::npos)
      {
      std::string pyRunStr = std::string("with open('") + pythonSource +
          std::string("', 'rb') as f:import imp;imp.load_module('__main__', f, '") + pythonSource +
          std::string("', ('.pyc', 'rb', 2))");
      pyRes = PyRun_String(
            pyRunStr.c_str(),
            Py_file_input, global_dict, global_dict);
      }
    else if (pythonSource.find(".py") != std::string::npos)
      {
      std::string pyRunStr = std::string("execfile('") + pythonSource + std::string("')");
      pyRes = PyRun_String(pyRunStr.c_str(),
        Py_file_input, global_dict, global_dict);
      }
    if (!pyRes)
      {
      vtkErrorMacro(<< "setPythonSource - Failed to execute file" << pythonSource << "!");
      return false;
      }
    Py_DECREF(pyRes);
    classToInstantiate = PyDict_GetItemString(global_dict, className.c_str());
    }
  if (!classToInstantiate)
    {
    vtkErrorMacro(<< "SetPythonSource - Failed to load displayable manager class definition from "
                  << pythonSource);
    return false;
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
    return false;
    }

//  // Retrieve API methods
//  for (int i = 0; i < vtkInternal::APIMethodCount; ++i)
//    {
//    assert(vtkInternal::APIMethodNames[i]);
//    PyObject * method = PyObject_GetAttrString(self, vtkInternal::APIMethodNames[i]);
//    //std::cout << "method:" << method << std::endl;
//    this->Internal->PythonAPIMethods[i] = method;
//    }

  //std::cout << "self (" << className << ", instance:" << self << ")" << std::endl;

  this->Internal->PythonSource = pythonSource;
  this->Internal->PythonSelf = self;

  return true;
}
