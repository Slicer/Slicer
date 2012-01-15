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
#include "vtkMRMLChartViewDisplayableManagerFactory.h"

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkMRMLChartViewDisplayableManagerFactory, "$Revision: 13859 $");

//----------------------------------------------------------------------------
// Needed when we don't use the vtkStandardNewMacro.
vtkInstantiatorNewMacro(vtkMRMLChartViewDisplayableManagerFactory);

//----------------------------------------------------------------------------
// vtkMRMLChartViewDisplayableManagerFactory methods

//----------------------------------------------------------------------------
// Up the reference count so it behaves like New
vtkMRMLChartViewDisplayableManagerFactory* vtkMRMLChartViewDisplayableManagerFactory::New()
{
  vtkMRMLChartViewDisplayableManagerFactory* instance = Self::GetInstance();
  instance->Register(0);
  return instance;
}

//----------------------------------------------------------------------------
vtkMRMLChartViewDisplayableManagerFactory* vtkMRMLChartViewDisplayableManagerFactory::GetInstance()
{
  if(!Self::Instance)
    {
    // Try the factory first
    Self::Instance = (vtkMRMLChartViewDisplayableManagerFactory*)
                     vtkObjectFactory::CreateInstance("vtkMRMLChartViewDisplayableManagerFactory");

    // if the factory did not provide one, then create it here
    if(!Self::Instance)
      {
      Self::Instance = new vtkMRMLChartViewDisplayableManagerFactory;
      }
    }
  // return the instance
  return Self::Instance;
}

//----------------------------------------------------------------------------
vtkMRMLChartViewDisplayableManagerFactory::
    vtkMRMLChartViewDisplayableManagerFactory():Superclass()
{
}

//----------------------------------------------------------------------------
vtkMRMLChartViewDisplayableManagerFactory::~vtkMRMLChartViewDisplayableManagerFactory()
{
}

//----------------------------------------------------------------------------
void vtkMRMLChartViewDisplayableManagerFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

VTK_SINGLETON_CXX(vtkMRMLChartViewDisplayableManagerFactory);

