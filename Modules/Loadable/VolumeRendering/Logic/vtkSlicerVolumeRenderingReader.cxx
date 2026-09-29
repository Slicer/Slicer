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

#include "vtkSlicerVolumeRenderingReader.h"

// Logic includes
#include "vtkSlicerVolumeRenderingLogic.h"
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLI18N.h>
#include <vtkMRMLVolumePropertyNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkSlicerVolumeRenderingReader);

//----------------------------------------------------------------------------
vtkSlicerVolumeRenderingReader::vtkSlicerVolumeRenderingReader()
{
  this->SetFileType("TransferFunctionFile");
  this->SetDescription(vtkMRMLTr("qSlicerVolumeRenderingReader", "Transfer Function"));
  this->SetNameFilters(std::vector<std::string>{ vtkMRMLTr("qSlicerVolumeRenderingReader", "Transfer Function") + " (*.vp)",
                                                 vtkMRMLTr("qSlicerVolumeRenderingReader", "Volume Property") + " (*.vp.json)" });
}

//----------------------------------------------------------------------------
vtkSlicerVolumeRenderingReader::~vtkSlicerVolumeRenderingReader() = default;

//----------------------------------------------------------------------------
void vtkSlicerVolumeRenderingReader::SetVolumeRenderingLogic(vtkSlicerVolumeRenderingLogic* logic)
{
  this->VolumeRenderingLogic = logic;
}

//----------------------------------------------------------------------------
vtkSlicerVolumeRenderingLogic* vtkSlicerVolumeRenderingReader::GetVolumeRenderingLogic()
{
  return this->VolumeRenderingLogic;
}

//----------------------------------------------------------------------------
bool vtkSlicerVolumeRenderingReader::Load(vtkMRMLIOProperties* properties)
{
  this->ClearLoadedNodeIDs();
  if (!properties || !this->VolumeRenderingLogic)
  {
    return false;
  }
  std::string fileName = properties->GetStringProperty("fileName");
  vtkNew<vtkCollection> volumePropertyNodes;
  bool success = this->VolumeRenderingLogic->AddVolumePropertiesFromFile(fileName.c_str(), volumePropertyNodes);
  for (int i = 0; i < volumePropertyNodes->GetNumberOfItems(); ++i)
  {
    vtkMRMLVolumePropertyNode* node = vtkMRMLVolumePropertyNode::SafeDownCast(volumePropertyNodes->GetItemAsObject(i));
    if (node)
    {
      this->AddLoadedNodeID(node->GetID());
    }
  }
  return success;
}
