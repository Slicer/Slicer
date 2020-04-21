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
#include "vtkMRMLTestThreeDViewDisplayableManager.h"

// MRML includes
#include <vtkMRMLCameraNode.h>

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLTestThreeDViewDisplayableManager );

//---------------------------------------------------------------------------
int vtkMRMLTestThreeDViewDisplayableManager::NodeAddedCount = 0;

//---------------------------------------------------------------------------
class vtkMRMLTestThreeDViewDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkMRMLTestThreeDViewDisplayableManager * external);
  ~vtkInternal();

  vtkMRMLTestThreeDViewDisplayableManager*             External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLTestThreeDViewDisplayableManager::vtkInternal::vtkInternal(vtkMRMLTestThreeDViewDisplayableManager * external)
{
  this->External = external;
}

//---------------------------------------------------------------------------
vtkMRMLTestThreeDViewDisplayableManager::vtkInternal::~vtkInternal() = default;

//---------------------------------------------------------------------------
// vtkMRMLTestThreeDViewDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLTestThreeDViewDisplayableManager::vtkMRMLTestThreeDViewDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLTestThreeDViewDisplayableManager::~vtkMRMLTestThreeDViewDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLTestThreeDViewDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLTestThreeDViewDisplayableManager::AdditionnalInitializeStep()
{
}

//---------------------------------------------------------------------------
void vtkMRMLTestThreeDViewDisplayableManager::Create()
{
  assert(this->GetRenderer());
  assert(this->GetMRMLViewNode());
}

//---------------------------------------------------------------------------
void vtkMRMLTestThreeDViewDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  vtkMRMLCameraNode * cameraNode = vtkMRMLCameraNode::SafeDownCast(node);
  if (!cameraNode)
    {
    return;
    }
  vtkMRMLTestThreeDViewDisplayableManager::NodeAddedCount++;
  //std::cout << "vtkMRMLTestThreeDViewDisplayableManager - NodeAdded - "
  //          << (node ? node->GetName() : "None")<< std::endl;
}

