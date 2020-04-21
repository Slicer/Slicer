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

// MRMLDisplayableManager includes
#include "vtkMRMLDisplayableManagerFactory.h"
#include "vtkMRMLDisplayableManagerGroup.h"
#ifdef MRMLDisplayableManager_USE_PYTHON
#include "vtkMRMLScriptedDisplayableManager.h"
#endif

// MRMLLogic includes
#include "vtkMRMLApplicationLogic.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <algorithm>
#include <string>
#include <vector>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLDisplayableManagerFactory);

//----------------------------------------------------------------------------
class vtkMRMLDisplayableManagerFactory::vtkInternal
{
public:
  vtkInternal();

  // Collection of Displayable Manager classNames
  std::vector<std::string> DisplayableManagerClassNames;

  // .. and its associated convenient typedef
  typedef std::vector<std::string>::iterator DisplayableManagerClassNamesIt;

  // The application logic (can be a vtkSlicerApplicationLogic
  vtkSmartPointer<vtkMRMLApplicationLogic> ApplicationLogic;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerFactory::vtkInternal::vtkInternal() = default;

//----------------------------------------------------------------------------
// vtkMRMLDisplayableManagerFactory methods

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerFactory::vtkMRMLDisplayableManagerFactory()
{
  this->Internal = new vtkInternal;
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerFactory::~vtkMRMLDisplayableManagerFactory()
{
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

  if (!vtkMRMLDisplayableManagerGroup::IsADisplayableManager(vtkClassOrScriptName))
    {
    vtkWarningMacro(<<"RegisterDisplayableManager - " << vtkClassOrScriptName
                    << " is not a displayable manager. Failed to register");
    return false;
    }
  // Register it
  this->Internal->DisplayableManagerClassNames.emplace_back(vtkClassOrScriptName);

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
std::string vtkMRMLDisplayableManagerFactory::GetRegisteredDisplayableManagerName(int n)
{
  if (n < 0 || n >= this->GetRegisteredDisplayableManagerCount())
    {
    vtkWarningMacro(<<"GetNthRegisteredDisplayableManagerName - "
                    "n " << n << " is invalid. A valid value for n should be >= 0 and < " <<
                    this->GetRegisteredDisplayableManagerCount());
    return std::string();
    }
  return this->Internal->DisplayableManagerClassNames.at(n);
}

//----------------------------------------------------------------------------
vtkMRMLDisplayableManagerGroup* vtkMRMLDisplayableManagerFactory::InstantiateDisplayableManagers(
    vtkRenderer * newRenderer)
{
  // Sanity checks
  if (!newRenderer)
    {
    vtkWarningMacro(<<"InstanciateDisplayableManagers - newRenderer is NULL");
    return nullptr;
    }

  vtkMRMLDisplayableManagerGroup * displayableManagerGroup = vtkMRMLDisplayableManagerGroup::New();
  displayableManagerGroup->Initialize(this, newRenderer);
  return displayableManagerGroup;
}

//----------------------------------------------------------------------------
vtkMRMLApplicationLogic* vtkMRMLDisplayableManagerFactory
::GetMRMLApplicationLogic()const
{
  return this->Internal->ApplicationLogic.GetPointer();
}

//----------------------------------------------------------------------------
void vtkMRMLDisplayableManagerFactory
::SetMRMLApplicationLogic(vtkMRMLApplicationLogic* logic)
{
  this->Internal->ApplicationLogic = logic;
}
