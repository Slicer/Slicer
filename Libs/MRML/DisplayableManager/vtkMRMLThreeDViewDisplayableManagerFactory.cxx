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
#include "vtkMRMLThreeDViewDisplayableManagerFactory.h"

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Needed when we don't use the vtkStandardNewMacro.
vtkInstantiatorNewMacro(vtkMRMLThreeDViewDisplayableManagerFactory);

//----------------------------------------------------------------------------
// vtkMRMLThreeDViewDisplayableManagerFactory methods

//----------------------------------------------------------------------------
// Up the reference count so it behaves like New
vtkMRMLThreeDViewDisplayableManagerFactory* vtkMRMLThreeDViewDisplayableManagerFactory::New()
{
  vtkMRMLThreeDViewDisplayableManagerFactory* instance = Self::GetInstance();
  instance->Register(0);
  return instance;
}

//----------------------------------------------------------------------------
vtkMRMLThreeDViewDisplayableManagerFactory* vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()
{
  if(!Self::Instance)
    {
    // Try the factory first
    Self::Instance = (vtkMRMLThreeDViewDisplayableManagerFactory*)
                     vtkObjectFactory::CreateInstance("vtkMRMLThreeDViewDisplayableManagerFactory");

    // if the factory did not provide one, then create it here
    if(!Self::Instance)
      {
      Self::Instance = new vtkMRMLThreeDViewDisplayableManagerFactory;
      }
    }
  // return the instance
  return Self::Instance;
}

//----------------------------------------------------------------------------
vtkMRMLThreeDViewDisplayableManagerFactory::
    vtkMRMLThreeDViewDisplayableManagerFactory():Superclass()
{
}

//----------------------------------------------------------------------------
vtkMRMLThreeDViewDisplayableManagerFactory::~vtkMRMLThreeDViewDisplayableManagerFactory()
{
}

//----------------------------------------------------------------------------
void vtkMRMLThreeDViewDisplayableManagerFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

VTK_SINGLETON_CXX(vtkMRMLThreeDViewDisplayableManagerFactory);

