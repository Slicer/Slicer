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

#include "vtkSlicerSegmentationsReader.h"

// Logic includes
#include "vtkSlicerSegmentationsModuleLogic.h"
#include <vtkMRMLIOOptionsDescription.h>
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLI18N.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelStorageNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSegmentationDisplayNode.h>
#include <vtkMRMLSegmentationNode.h>
#include <vtkMRMLStorageNode.h>

// Segmentations includes
#include <vtkSegment.h>
#include <vtkSegmentationConverter.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkSlicerSegmentationsReader);

//----------------------------------------------------------------------------
vtkSlicerSegmentationsReader::vtkSlicerSegmentationsReader()
{
  this->SetFileType("SegmentationFile");
  this->SetDescription(vtkMRMLTr("qSlicerSegmentationsReader", "Segmentation"));
  const std::string extensionText = vtkMRMLTr("qSlicerSegmentationsReader", "Segmentation");
  this->SetNameFilters(std::vector<std::string>{ extensionText + " (*.seg.nrrd)",
                                                 extensionText + " (*.seg.nhdr)",
                                                 extensionText + " (*.seg.vtm)",
                                                 extensionText + " (*.nrrd)",
                                                 extensionText + " (*.nhdr)",
                                                 extensionText + " (*.vtm)",
                                                 extensionText + " (*.nii.gz)",
                                                 extensionText + " (*.nii)",
                                                 extensionText + " (*.hdr)",
                                                 extensionText + " (*.stl)",
                                                 extensionText + " (*.obj)" });
}

//----------------------------------------------------------------------------
vtkSlicerSegmentationsReader::~vtkSlicerSegmentationsReader() = default;

//----------------------------------------------------------------------------
void vtkSlicerSegmentationsReader::SetSegmentationsLogic(vtkSlicerSegmentationsModuleLogic* logic)
{
  this->SegmentationsLogic = logic;
}

//----------------------------------------------------------------------------
vtkSlicerSegmentationsModuleLogic* vtkSlicerSegmentationsReader::GetSegmentationsLogic()
{
  return this->SegmentationsLogic;
}

//----------------------------------------------------------------------------
double vtkSlicerSegmentationsReader::CanLoadFileConfidence(const std::string& filePath)
{
  double confidence = this->Superclass::CanLoadFileConfidence(filePath);
  // Confidence for .nrrd file is 0.55 (5 characters in the file extension matched),
  // .vtm is 0.54; for composite file extensions (.seg.nrrd, .seg.vtm) it would be >0.58.
  // Therefore, confidence below 0.56 means that we got a generic file extension
  // that we need to inspect further.
  if (confidence > 0 && confidence < 0.56 && (EndsWithNoCase(filePath, "NRRD") || EndsWithNoCase(filePath, "NHDR")))
  {
    // Segmentation NRRD files contain ID for each segment (such as Segment0_ID:=...)
    // or common segmentation information (such as Segmentation_ContainedRepresentations:=...).
    // around position 500, read a bit further to account for slight variations in the header.
    std::string header = ReadFileHeader(filePath, 800);
    if (!header.empty())
    {
      // If this appears in the file header then declare higher confidence value.
      confidence = (header.find("Segment0_ID:=") != std::string::npos || header.find("Segmentation_") != std::string::npos ? 0.6 : 0.4);
    }
  }
  return confidence;
}

//----------------------------------------------------------------------------
bool vtkSlicerSegmentationsReader::Load(vtkMRMLIOProperties* properties)
{
  this->ClearLoadedNodeIDs();
  vtkMRMLScene* scene = this->GetScene();
  if (!properties || !this->SegmentationsLogic || !scene)
  {
    return false;
  }
  std::string fileName = properties->GetStringProperty("fileName");
  std::string name = properties->GetStringProperty("name");

  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fileName);
  if (extension == ".stl" || extension == ".obj")
  {
    vtkNew<vtkMRMLModelStorageNode> modelStorageNode;
    modelStorageNode->SetFileName(fileName.c_str());
    vtkNew<vtkMRMLModelNode> modelNode;
    if (!modelStorageNode->ReadData(modelNode))
    {
      return false;
    }
    vtkSmartPointer<vtkPolyData> closedSurfaceRepresentation = modelNode->GetPolyData();
    if (closedSurfaceRepresentation == nullptr)
    {
      return false;
    }
    // Remove all arrays, because they could slow down all further processing
    // and consume significant amount of memory.
    vtkPointData* pointData = closedSurfaceRepresentation->GetPointData();
    while (pointData && pointData->GetNumberOfArrays() > 0)
    {
      pointData->RemoveArray(0);
    }

    if (name.empty())
    {
      // complete base name: file name without the last extension
      name = vtksys::SystemTools::GetFilenameWithoutLastExtension(vtksys::SystemTools::GetFilenameName(fileName));
    }
    vtkNew<vtkSegment> segment;
    segment->SetName(name.c_str());
    segment->AddRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(), closedSurfaceRepresentation);

    vtkMRMLSegmentationNode* segmentationNode =
      vtkMRMLSegmentationNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSegmentationNode", scene->GetUniqueNameByString(name.c_str())));
    segmentationNode->SetSourceRepresentationToClosedSurface();
    segmentationNode->CreateDefaultDisplayNodes();
    vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
    if (displayNode)
    {
      // Show slice intersections using closed surface representation (don't create binary labelmap for display)
      displayNode->SetPreferredDisplayRepresentationName2D(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
    }
    segmentationNode->GetSegmentation()->AddSegment(segment);
    this->AddLoadedNodeID(segmentationNode->GetID());
    return true;
  }

  // Non-STL file, load using segmentation storage node
  bool autoOpacities = properties->GetBoolProperty("autoOpacities", true);
  vtkMRMLColorTableNode* colorTableNode = nullptr;
  if (properties->HasProperty("colorNodeID"))
  {
    colorTableNode = vtkMRMLColorTableNode::SafeDownCast(scene->GetNodeByID(properties->GetStringProperty("colorNodeID")));
  }
  vtkMRMLSegmentationNode* node = this->SegmentationsLogic->LoadSegmentationFromFile(fileName.c_str(), autoOpacities, name.c_str(), colorTableNode, this->GetUserMessages());
  if (!node)
  {
    return false;
  }
  this->AddLoadedNodeID(node->GetID());
  return true;
}

//----------------------------------------------------------------------------
void vtkSlicerSegmentationsReader::GetOptionsDescription(vtkMRMLIOOptionsDescription* description)
{
  if (!description)
  {
    return;
  }
  description->AddBoolOption(
    "autoOpacities",
    vtkMRMLTr("qSlicerSegmentationsIOOptionsWidget", "Automatic Segment Opacities"),
    vtkMRMLTr("qSlicerSegmentationsIOOptionsWidget", "Automatically set opacities of the segments based on which contains which, so that no segment obscures another"),
    true);
  description->AddNodeOption("colorNodeID",
                             vtkMRMLTr("qSlicerSegmentationsIOOptionsWidget", "Color node:"),
                             vtkMRMLTr("qSlicerSegmentationsIOOptionsWidget", "Color table node used to display this volume."),
                             "",
                             std::vector<std::string>{ "vtkMRMLColorTableNode" },
                             true);
  description->SetOptionShowHidden("colorNodeID", true);
  description->SetOptionWidget("colorNodeID", "colorTable");
}
