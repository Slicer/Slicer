/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

/// Factory where displayable manager classe should be registered with
/// 
/// A displayable manager class is responsible to represente a 
/// MRMLDisplayable node in a renderer.
/// 

#ifndef __vtkMRMLDisplayableManagerFactory_h
#define __vtkMRMLDisplayableManagerFactory_h

// VTK includes
#include <vtkObject.h>

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkRenderer;
class vtkMRMLDisplayableManagerGroup;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLDisplayableManagerFactory : public vtkObject
{
public:

  static vtkMRMLDisplayableManagerFactory* New();
  vtkTypeRevisionMacro(vtkMRMLDisplayableManagerFactory,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //BTX
  enum
  {
    DisplayableManagerFactoryRegisteredEvent     = 30100,
    DisplayableManagerFactoryUnRegisteredEvent   = 30101
  };
  //ETX

  ///
  /// Return True if Displayable Manager identified by \a vtkClassName
  /// is already registered.
  bool IsDisplayableManagerRegistered(const char* vtkClassName);

  ///
  /// Register Displayable Manager identified by \a vtkClassOrScriptName
  /// \a vtkClassOrScriptName should be either:
  /// a VTK class registered using the CMake macro VTK_MAKE_INSTANTIATOR3
  /// a python script having the extension .py
  /// Internally for each registered python script, a scripted displayable manager
  /// will be instanciated.
  void RegisterDisplayableManager(const char* vtkClassOrScriptName);

  ///
  /// UnRegister Displayable Manager identified by \a vtkClassOrScriptName
  /// \a vtkClassOrScriptName should be either a VTK class registered using the
  /// CMake macro VTK_MAKE_INSTANTIATOR3 or a python script name.
  void UnRegisterDisplayableManager(const char* vtkClassOrScriptName);

  ///
  /// Return number of registered displayable managers
  int GetRegisteredDisplayableManagerCount();
  
  /// Instantiate registrered DisplayableManagers
  /// It returns a vtkMRMLDisplayableManagerGroup representing a list of DisplayableManager
  /// Internally, the factory keep track of all the Group and will invoke the ModifiedEvent
  /// of each group.
  vtkMRMLDisplayableManagerGroup* InstantiateDisplayableManagers(vtkRenderer * newRenderer);

protected:

  vtkMRMLDisplayableManagerFactory();
  virtual ~vtkMRMLDisplayableManagerFactory();

  //BTX
  class vtkInternal;
  vtkInternal* Internal;
  //ETX

  //BTX
  typedef vtkMRMLDisplayableManagerFactory Self;
  //ETX

private:
  
  vtkMRMLDisplayableManagerFactory(const vtkMRMLDisplayableManagerFactory&);
  void operator=(const vtkMRMLDisplayableManagerFactory&);

};

#endif


