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

// MRMLDisplayableManager includes
#include "vtkMRMLDisplayableManagerFactory.h"
#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"
#include "vtkMRMLDisplayableManagerGroup.h"
#ifdef MRMLDisplayableManager_USE_PYTHON
#include "vtkMRMLScriptedDisplayableManager.h"
#endif

// MRML includes
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkInstantiator.h>

// STD includes
#include <vector>
#include <map>
#include <algorithm>
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLDisplayableManagerFactory);
vtkCxxRevisionMacro(vtkMRMLDisplayableManagerFactory, "$Revision: 13859 $");

//----------------------------------------------------------------------------
class vtkMRMLDisplayableManagerFactory::vtkInternal
{
public:
  vtkInternal();

  // Collection of Displayable Manager classNames
  std::vector<std::string> DisplayableManagerClassNames;

  // .. and its associated convenient typedef
  typedef std::vector<std::string>::iterator DisplayableManagerClassNamesIt;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerFactory::vtkInternal::vtkInternal()
{
}

//----------------------------------------------------------------------------
// vtkMRMLDisplayableManagerFactory methods

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerFactory::vtkMRMLDisplayableManagerFactory()
{
  this->DisplayableManagerGroup = NULL;
  this->Internal = new vtkInternal;
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerFactory::~vtkMRMLDisplayableManagerFactory()
{
  if (this->DisplayableManagerGroup)
    {
    this->DisplayableManagerGroup->Delete();
    }

  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLDisplayableManagerFactory::IsDisplayableManagerRegistered(const char* vtkClassName)
{
  // Sanity checks
  if (!vtkClassName)
    {
    vtkWarningMacro(<<"IsDisplayableManagerRegistered - vtkClassName is NULL");
    return false;
    }

  // Check if the DisplayableManager has already been registered
  vtkInternal::DisplayableManagerClassNamesIt it = std::find(
      this->Internal->DisplayableManagerClassNames.begin(),
      this->Internal->DisplayableManagerClassNames.end(),
      vtkClassName);

  if ( it == this->Internal->DisplayableManagerClassNames.end())
    {
    return false;
    }
  else
    {
    return true;
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLDisplayableManagerFactory::RegisterDisplayableManager(const char* vtkClassOrScriptName)
{
  // Sanity checks
  if (!vtkClassOrScriptName)
    {
    vtkWarningMacro(<<"RegisterDisplayableManager - vtkClassOrScriptName is NULL");
    return false;
    }

  // Check if the DisplayableManager has already been registered
  vtkInternal::DisplayableManagerClassNamesIt it = std::find(
      this->Internal->DisplayableManagerClassNames.begin(),
      this->Internal->DisplayableManagerClassNames.end(),
      vtkClassOrScriptName);

  if ( it != this->Internal->DisplayableManagerClassNames.end())
    {
    vtkWarningMacro(<<"RegisterDisplayableManager - " << vtkClassOrScriptName << " already registered");
    return false;
    }

  // Check if vtkClassOrScriptName is a valid vtk className
  vtkSmartPointer<vtkObject> objectSmartPointer;
  objectSmartPointer.TakeReference(vtkInstantiator::CreateInstance(vtkClassOrScriptName));
  if (!objectSmartPointer || !objectSmartPointer->IsA("vtkMRMLAbstractDisplayableManager"))
    {
#ifdef MRMLDisplayableManager_USE_PYTHON
    // Check if vtkClassOrScriptName is a python script
    std::string str(vtkClassOrScriptName);
    if (str.find(".py") == std::string::npos)
      {
      vtkWarningMacro(<<"RegisterDisplayableManager - Failed to register " << vtkClassOrScriptName);
      return false;
      }
#else
    vtkWarningMacro(<<"RegisterDisplayableManager - Failed to register " << vtkClassOrScriptName);
    return false;
#endif
    }

  // Register it
  this->Internal->DisplayableManagerClassNames.push_back(vtkClassOrScriptName);

  this->InvokeEvent(Self::DisplayableManagerFactoryRegisteredEvent,
                    const_cast<char*>(vtkClassOrScriptName));

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLDisplayableManagerFactory::UnRegisterDisplayableManager(const char* vtkClassOrScriptName)
{
  // Sanity checks
  if (!vtkClassOrScriptName)
    {
    vtkWarningMacro(<<"UnRegisterDisplayableManager - vtkClassOrScriptName is NULL");
    return false;
    }
  
  // Check if the DisplayableManager is registered
  vtkInternal::DisplayableManagerClassNamesIt it = std::find(
      this->Internal->DisplayableManagerClassNames.begin(),
      this->Internal->DisplayableManagerClassNames.end(),
      vtkClassOrScriptName);

  if ( it == this->Internal->DisplayableManagerClassNames.end())
    {
    vtkWarningMacro(<<"UnRegisterDisplayableManager - " << vtkClassOrScriptName << " is NOT registered");
    return false;
    }

  this->Internal->DisplayableManagerClassNames.erase(it);

  this->InvokeEvent(Self::DisplayableManagerFactoryUnRegisteredEvent,
                    const_cast<char*>(vtkClassOrScriptName));

  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLDisplayableManagerFactory::GetRegisteredDisplayableManagerCount()
{
  return static_cast<int>(this->Internal->DisplayableManagerClassNames.size());
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerGroup* vtkMRMLDisplayableManagerFactory::InstantiateDisplayableManagers(
    vtkRenderer * newRenderer)
{
  // Sanity checks
  if (!newRenderer)
    {
    vtkWarningMacro(<<"InstanciateDisplayableManagers - newRenderer is NULL");
    return 0;
    }

  if (this->DisplayableManagerGroup)
    {
    this->DisplayableManagerGroup->Delete();
    }

  this->DisplayableManagerGroup = vtkMRMLDisplayableManagerGroup::New();

  // A Group observes the factory and eventually instantiates new DisplayableManager
  // when they are registered in the factory
  this->DisplayableManagerGroup->SetAndObserveDisplayableManagerFactory(this);

  for(std::size_t i=0; i < this->Internal->DisplayableManagerClassNames.size(); ++i)
    {
    const char* classOrScriptName = this->Internal->DisplayableManagerClassNames[i].c_str();

#ifdef MRMLDisplayableManager_USE_PYTHON
    // Are we dealing with a python scripted displayable manager
    std::string str(classOrScriptName);
    if (str.find(".py") != std::string::npos)
      {
      // TODO Make sure the file exists ...
      vtkSmartPointer<vtkMRMLScriptedDisplayableManager> scriptedDisplayableManager =
          vtkSmartPointer<vtkMRMLScriptedDisplayableManager>::New();
      scriptedDisplayableManager->SetPythonSource(classOrScriptName);

      // Note that DisplayableManagerGroup will take ownership of the object
      this->DisplayableManagerGroup->AddAndInitialize(scriptedDisplayableManager);
      }
    else
      {
#endif
      // Object will be unregistered when the SmartPointer will go out-of-scope
      vtkSmartPointer<vtkObject> objectSmartPointer;
      objectSmartPointer.TakeReference(vtkInstantiator::CreateInstance(classOrScriptName));
      vtkMRMLAbstractDisplayableManager* displayableManager =
          vtkMRMLAbstractDisplayableManager::SafeDownCast(objectSmartPointer);
      if (!displayableManager)
        {
        vtkErrorMacro(<<"InstantiateDisplayableManagers - Failed to instantiate " << classOrScriptName);
        continue;
        }

      // Note that DisplayableManagerGroup will take ownership of the object
      this->DisplayableManagerGroup->AddAndInitialize(displayableManager);
#ifdef MRMLDisplayableManager_USE_PYTHON
      }
#endif

    }

  this->DisplayableManagerGroup->Initialize(newRenderer);

  return this->DisplayableManagerGroup;
}


