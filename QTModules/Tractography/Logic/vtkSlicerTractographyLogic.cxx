/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Tractography includes
#include "vtkSlicerTractographyLogic.h"

// MRML includes
#include <vtkMRMLConfigure.h>
#ifdef MRML_USE_vtkTeem
# include "vtkMRMLFiberBundleGlyphDisplayNode.h"
# include "vtkMRMLFiberBundleLineDisplayNode.h"
# include "vtkMRMLFiberBundleNode.h"
# include "vtkMRMLFiberBundleStorageNode.h"
# include "vtkMRMLFiberBundleTubeDisplayNode.h"
#endif

// VTK includes
#include <vtkNew.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTractographyLogic);

//----------------------------------------------------------------------------
vtkSlicerTractographyLogic::vtkSlicerTractographyLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerTractographyLogic::~vtkSlicerTractographyLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerTractographyLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerTractographyLogic::InitializeEventListeners()
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(this->GetMRMLScene(), events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerTractographyLogic::ProcessMRMLEvents(
  vtkObject* vtkNotUsed(caller), unsigned long vtkNotUsed(event), void * vtkNotUsed(callData))
{
}

//-----------------------------------------------------------------------------
void vtkSlicerTractographyLogic::RegisterNodes()
{
  if(!this->GetMRMLScene())
    {
    return;
    }
#ifdef MRML_USE_vtkTeem
  this->GetMRMLScene()->RegisterNodeClass(vtkNew<vtkMRMLFiberBundleNode>().GetPointer());
  this->GetMRMLScene()->RegisterNodeClass(vtkNew<vtkMRMLFiberBundleLineDisplayNode>().GetPointer());
  this->GetMRMLScene()->RegisterNodeClass(vtkNew<vtkMRMLFiberBundleTubeDisplayNode>().GetPointer());
  this->GetMRMLScene()->RegisterNodeClass(vtkNew<vtkMRMLFiberBundleGlyphDisplayNode>().GetPointer());
  this->GetMRMLScene()->RegisterNodeClass(vtkNew<vtkMRMLFiberBundleStorageNode>().GetPointer());
#endif
}
