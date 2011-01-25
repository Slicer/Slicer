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
#include "vtkMRMLTestCustomDisplayableManager.h"
#include "vtkMRMLDisplayableManagerGroup.h"

// MRML includes
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>

// STD includes
#include <cassert>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLTestCustomDisplayableManager );
vtkCxxRevisionMacro(vtkMRMLTestCustomDisplayableManager, "$Revision: 13525 $");

//---------------------------------------------------------------------------
int vtkMRMLTestCustomDisplayableManager::NodeAddedCountThreeDView = 0;
int vtkMRMLTestCustomDisplayableManager::NodeAddedCountSliceView = 0;

//---------------------------------------------------------------------------
class vtkMRMLTestCustomDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkMRMLTestCustomDisplayableManager * external);
  ~vtkInternal();

  vtkMRMLTestCustomDisplayableManager*             External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLTestCustomDisplayableManager::vtkInternal::vtkInternal(vtkMRMLTestCustomDisplayableManager * external)
{
  this->External = external;
}

//---------------------------------------------------------------------------
vtkMRMLTestCustomDisplayableManager::vtkInternal::~vtkInternal()
{
}

//---------------------------------------------------------------------------
// vtkMRMLTestCustomDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLTestCustomDisplayableManager::vtkMRMLTestCustomDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLTestCustomDisplayableManager::~vtkMRMLTestCustomDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLTestCustomDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLTestCustomDisplayableManager::AdditionnalInitializeStep()
{
}

//---------------------------------------------------------------------------
void vtkMRMLTestCustomDisplayableManager::Create()
{
  assert(this->GetRenderer());
  assert(this->GetMRMLDisplayableNode());
}

//---------------------------------------------------------------------------
void vtkMRMLTestCustomDisplayableManager::OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node)
{
  vtkMRMLCameraNode * cameraNode = vtkMRMLCameraNode::SafeDownCast(node);
  if (!cameraNode)
    {
    return;
    }
  if (vtkMRMLViewNode::SafeDownCast(this->GetMRMLDisplayableNode()))
    {
    vtkMRMLTestCustomDisplayableManager::NodeAddedCountThreeDView++;
    //std::cout << "vtkMRMLTestCustomDisplayableManager[vtkMRMLViewNode] - NodeAdded - "
    //          << (node ? node->GetName() : "None")<< std::endl;
    }
  if (vtkMRMLSliceNode::SafeDownCast(this->GetMRMLDisplayableNode()))
    {
    vtkMRMLTestCustomDisplayableManager::NodeAddedCountSliceView++;
    //std::cout << "vtkMRMLTestCustomDisplayableManager[vtkMRMLSliceNode] - NodeAdded - "
    //          << (node ? node->GetName() : "None")<< std::endl;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLTestCustomDisplayableManager::ProcessMRMLEvents(vtkObject * caller,
                                                      unsigned long event,
                                                      void *callData)
{
  //if (vtkMRMLCameraDisplayableManager::SafeDownCast(caller))
  //  {
  //  
  //  }
  //else
  //  {
    this->Superclass::ProcessMRMLEvents(caller, event, callData);
  //  }
}

