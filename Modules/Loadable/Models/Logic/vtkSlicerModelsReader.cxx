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

#include "vtkSlicerModelsReader.h"

// Logic includes
#include "vtkSlicerModelsLogic.h"
#include <vtkSlicerApplicationLogic.h>
#include <vtkMRMLIOOptionsDescription.h>
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLI18N.h>
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLStorageNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkSlicerModelsReader);

//----------------------------------------------------------------------------
vtkSlicerModelsReader::vtkSlicerModelsReader()
{
  this->SetFileType("ModelFile");
  this->SetDescription("Model");
  this->SetNameFilters(std::vector<std::string>{ "Model (*.vtk *.vtp *.vtu *.g *.byu *.stl *.ply *.obj *.ucd)" });
}

//----------------------------------------------------------------------------
vtkSlicerModelsReader::~vtkSlicerModelsReader() = default;

//----------------------------------------------------------------------------
void vtkSlicerModelsReader::SetModelsLogic(vtkSlicerModelsLogic* logic)
{
  this->ModelsLogic = logic;
}

//----------------------------------------------------------------------------
vtkSlicerModelsLogic* vtkSlicerModelsReader::GetModelsLogic()
{
  return this->ModelsLogic;
}

//----------------------------------------------------------------------------
double vtkSlicerModelsReader::CanLoadFileConfidence(const std::string& filePath)
{
  double confidence = this->Superclass::CanLoadFileConfidence(filePath);
  // .vtk files can store either an image (DATASET STRUCTURED_POINTS)
  // or a mesh (DATASET POLYDATA). Images are read by the volume reader with default
  // confidence of 0.54 (4 characters in the .vtk file extension matched).
  // Therefore, we set confidence here to 0.6 for meshes and 0.0 for images.
  if (confidence > 0 && EndsWithNoCase(filePath, ".VTK"))
  {
    // .vtk image file header contains DATASET STRUCTURED_POINTS at around
    // around position 100, read a bit further to account for slight variations in the header.
    std::string header = ReadFileHeader(filePath, 200);
    if (!header.empty())
    {
      // If dataset is structured points then it is an image, which this reader cannot read.
      confidence = (header.find("STRUCTURED_POINTS") != std::string::npos ? 0.0 : 0.6);
    }
  }
  return confidence;
}

//----------------------------------------------------------------------------
bool vtkSlicerModelsReader::Load(vtkMRMLIOProperties* properties)
{
  this->ClearLoadedNodeIDs();
  if (!this->ModelsLogic)
  {
    vtkErrorMacro("Load failed: Models logic is invalid.");
    return false;
  }
  vtkMRMLScene* scene = this->GetScene();
  if (!properties || !scene)
  {
    vtkErrorMacro("Load failed: invalid properties or scene");
    return false;
  }
  std::string fileName = properties->GetStringProperty("fileName");
  int coordinateSystem = properties->GetIntProperty("coordinateSystem", vtkMRMLStorageNode::CoordinateSystemLPS);
  if (coordinateSystem < 0)
  {
    // "Default" option: use the coordinate system specified in the file, and LPS if the file does not specify it
    coordinateSystem = vtkMRMLStorageNode::CoordinateSystemLPS;
  }

  this->GetUserMessages()->ClearMessages();
  vtkMRMLModelNode* node = this->ModelsLogic->AddModel(fileName.c_str(), coordinateSystem, this->GetUserMessages());
  if (!node)
  {
    // errors are already logged and userMessages contain details that can be displayed to users
    return false;
  }
  this->AddLoadedNodeID(node->GetID());

  if (properties->HasProperty("name"))
  {
    std::string uname = scene->GetUniqueNameByString(properties->GetStringProperty("name").c_str());
    node->SetName(uname.c_str());
  }

  // If no other nodes are displayed then reset the field of view
  bool otherNodesAreAlreadyVisible = false;
  vtkSmartPointer<vtkCollection> displayNodes = vtkSmartPointer<vtkCollection>::Take(scene->GetNodesByClass("vtkMRMLDisplayNode"));
  for (int displayNodeIndex = 0; displayNodeIndex < displayNodes->GetNumberOfItems(); ++displayNodeIndex)
  {
    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(displayNodes->GetItemAsObject(displayNodeIndex));
    if (displayNode->GetDisplayableNode() && displayNode->GetVisibility() && displayNode->GetDisplayableNode() != node)
    {
      otherNodesAreAlreadyVisible = true;
      break;
    }
  }
  if (!otherNodesAreAlreadyVisible && this->ModelsLogic->GetApplicationLogic())
  {
    this->ModelsLogic->GetApplicationLogic()->RequestResetThreeDViews();
  }

  return true;
}

//----------------------------------------------------------------------------
void vtkSlicerModelsReader::GetOptionsDescription(vtkMRMLIOOptionsDescription* description)
{
  if (!description)
  {
    return;
  }
  description->AddEnumOption("coordinateSystem",
                             vtkMRMLTr("qSlicerModelsIOOptionsWidget", "Coordinate system:"),
                             vtkMRMLTr("qSlicerModelsIOOptionsWidget",
                                       "Use LPS (left-posterior-superior) for better compatibility with most software (this is the default)."
                                       " Use RAS (right-anterior-superior) for better compatibility with earlier Slicer versions."
                                       " If coordinate system is defined in the file then that is used and this choice is ignored."),
                             vtkVariant(-1));
  description->AddEnumChoice("coordinateSystem", vtkVariant(-1), vtkMRMLTr("qSlicerModelsIOOptionsWidget", "Default"));
  description->AddEnumChoice("coordinateSystem", vtkVariant(static_cast<int>(vtkMRMLStorageNode::CoordinateSystemLPS)), vtkMRMLTr("qSlicerModelsIOOptionsWidget", "LPS"));
  description->AddEnumChoice("coordinateSystem", vtkVariant(static_cast<int>(vtkMRMLStorageNode::CoordinateSystemRAS)), vtkMRMLTr("qSlicerModelsIOOptionsWidget", "RAS"));
}
