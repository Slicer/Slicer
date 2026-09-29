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

==============================================================================*/

#include "vtkSlicerSegmentationsNodeWriter.h"

// Logic includes
#include <vtkMRMLFileIOManager.h>
#include <vtkMRMLIOOptionsDescription.h>
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLI18N.h>
#include <vtkMRMLSegmentationStorageNode.h>
#include <vtkMRMLStorableNode.h>

// VTK includes
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkSlicerSegmentationsNodeWriter);

//----------------------------------------------------------------------------
vtkSlicerSegmentationsNodeWriter::vtkSlicerSegmentationsNodeWriter()
{
  this->SetDescription("Segmentation");
  this->SetFileType("SegmentationFile");
  this->SetNodeClassName("vtkMRMLSegmentationNode");
  this->SetSupportUseCompression(true);
}

//----------------------------------------------------------------------------
vtkSlicerSegmentationsNodeWriter::~vtkSlicerSegmentationsNodeWriter() = default;

//----------------------------------------------------------------------------
bool vtkSlicerSegmentationsNodeWriter::Write(vtkMRMLIOProperties* properties)
{
  if (!properties)
  {
    return false;
  }
  vtkMRMLStorableNode* node = vtkMRMLStorableNode::SafeDownCast(this->GetNodeByID(properties->GetStringProperty("nodeID")));
  if (this->CanWriteObjectConfidence(node) <= 0.0)
  {
    return false;
  }
  vtkMRMLSegmentationStorageNode* storageNode = vtkMRMLSegmentationStorageNode::SafeDownCast(vtkMRMLFileIOManager::CreateAndAddDefaultStorageNode(node));
  if (!storageNode)
  {
    vtkDebugMacro("No storage node for node " << properties->GetStringProperty("nodeID"));
    return false;
  }
  storageNode->SetCropToMinimumExtent(properties->GetBoolProperty("cropToMinimumExtent"));
  return this->Superclass::Write(properties);
}

//----------------------------------------------------------------------------
void vtkSlicerSegmentationsNodeWriter::GetOptionsDescription(vtkMRMLIOOptionsDescription* description)
{
  this->Superclass::GetOptionsDescription(description);
  if (!description)
  {
    return;
  }
  bool cropToMinimumExtent = false;
  vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetNodeByID(description->GetProperties()->GetStringProperty("nodeID")));
  vtkMRMLSegmentationStorageNode* storageNode = storableNode ? vtkMRMLSegmentationStorageNode::SafeDownCast(storableNode->GetStorageNode()) : nullptr;
  if (storageNode)
  {
    cropToMinimumExtent = storageNode->GetCropToMinimumExtent();
  }
  description->AddBoolOption("cropToMinimumExtent",
                             vtkMRMLTr("qSlicerSegmentationsNodeWriterOptionsWidget", "Crop to minimum extent"),
                             vtkMRMLTr("qSlicerSegmentationsNodeWriterOptionsWidget",
                                       "If enabled then segmentation labelmap representation is"
                                       " cropped to the minimum necessary size. This saves storage space but changes voxel coordinate system"
                                       " (physical coordinate system is not affected)."),
                             cropToMinimumExtent);
}
