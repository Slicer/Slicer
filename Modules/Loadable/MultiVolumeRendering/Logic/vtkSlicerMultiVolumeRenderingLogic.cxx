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

// MultiVolumeRendering includes
#include "vtkSlicerMultiVolumeRenderingLogic.h"
#include "vtkMRMLMultiVolumeRenderingDisplayNode.h"

// MRML includes
#include <vtkCacheManager.h>
#include <vtkMRMLColorNode.h>
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLVectorVolumeDisplayNode.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLVolumePropertyNode.h>
#include <vtkMRMLVolumePropertyStorageNode.h>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkVolumeProperty.h>

// VTKSYS includes
#include <itksys/SystemTools.hxx>

// STD includes
#include <algorithm>
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerMultiVolumeRenderingLogic);

//----------------------------------------------------------------------------
vtkSlicerMultiVolumeRenderingLogic::vtkSlicerMultiVolumeRenderingLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerMultiVolumeRenderingLogic::~vtkSlicerMultiVolumeRenderingLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerMultiVolumeRenderingLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerMultiVolumeRenderingLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerMultiVolumeRenderingLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerMultiVolumeRenderingLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerMultiVolumeRenderingLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerMultiVolumeRenderingLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

bool vtkSlicerMultiVolumeRenderingLogic::IsDisplayNodeMatch(vtkMRMLMultiVolumeRenderingDisplayNode *dnode,
                                                            vtkMRMLVolumeNode *bg, vtkMRMLVolumeNode *fg, vtkMRMLVolumeNode *label)
{
  if (!dnode)
    return false;

  vtkMRMLVolumeNode *b, *f, *l;

  b = dnode->GetBgVolumeNode();
  f = dnode->GetFgVolumeNode();
  l = dnode->GetLabelmapVolumeNode();

  if (b == bg && f == fg && l == label)
    return true;
  else
    return false;
}

vtkMRMLMultiVolumeRenderingDisplayNode* vtkSlicerMultiVolumeRenderingLogic
::FindFirstMatchedDisplayNode(vtkMRMLVolumeNode *bg, vtkMRMLVolumeNode *fg, vtkMRMLVolumeNode *label)
{
  if (bg == NULL && fg == NULL && label == NULL)
    return NULL;

  int ndnodes = 0;

  if (bg)
    bg->GetNumberOfDisplayNodes();

  for (int i=0; i<ndnodes; i++)
  {
    vtkMRMLMultiVolumeRenderingDisplayNode *dnode = vtkMRMLMultiVolumeRenderingDisplayNode::SafeDownCast(
                                                          bg->GetNthDisplayNode(i));
    if (dnode && this->IsDisplayNodeMatch(dnode, bg, fg, label) )
      return dnode;
  }

  ndnodes = 0;

  if (fg)
    ndnodes = fg->GetNumberOfDisplayNodes();

  for (int i=0; i<ndnodes; i++)
  {
    vtkMRMLMultiVolumeRenderingDisplayNode *dnode = vtkMRMLMultiVolumeRenderingDisplayNode::SafeDownCast(
                                                          fg->GetNthDisplayNode(i));
    if (dnode && this->IsDisplayNodeMatch(dnode, bg, fg, label) )
      return dnode;
  }

  ndnodes = 0;

  if (label)
    ndnodes = label->GetNumberOfDisplayNodes();

  for (int i=0; i<ndnodes; i++)
  {
    vtkMRMLMultiVolumeRenderingDisplayNode *dnode = vtkMRMLMultiVolumeRenderingDisplayNode::SafeDownCast(
                                                          label->GetNthDisplayNode(i));
    if (dnode && this->IsDisplayNodeMatch(dnode, bg, fg, label) )
      return dnode;
  }

  return NULL;
}

vtkMRMLMultiVolumeRenderingDisplayNode* vtkSlicerMultiVolumeRenderingLogic::CreateDisplayNode()
{
  vtkMRMLMultiVolumeRenderingDisplayNode *node = NULL;

  if (this->GetMRMLScene() == 0)
  {
    return node;
  }

  node = vtkMRMLMultiVolumeRenderingDisplayNode::New();

  this->GetMRMLScene()->AddNode(node);
  node->Delete();

  return node;
}

// Description:
// Find volume rendering display node reference in the volume
//----------------------------------------------------------------------------
vtkMRMLMultiVolumeRenderingDisplayNode* vtkSlicerMultiVolumeRenderingLogic
                                        ::GetDisplayNodeByID(vtkMRMLVolumeNode *volumeNode, char *displayNodeID)
{
  if (displayNodeID == NULL || volumeNode == NULL)
  {
    return NULL;
  }

  int ndnodes = volumeNode->GetNumberOfDisplayNodes();

  for (int i = 0; i < ndnodes; i++)
  {
    vtkMRMLMultiVolumeRenderingDisplayNode *dnode = vtkMRMLMultiVolumeRenderingDisplayNode::SafeDownCast(
                                                    volumeNode->GetNthDisplayNode(i));

    if (dnode && !strcmp(displayNodeID, dnode->GetID()))
    {
      return dnode;
    }
  }

  return NULL;
}
