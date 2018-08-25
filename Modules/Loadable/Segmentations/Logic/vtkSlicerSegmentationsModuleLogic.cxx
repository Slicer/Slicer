/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Segmentations includes
#include "vtkSlicerSegmentationsModuleLogic.h"

// SegmentationCore includes
#include "vtkBinaryLabelmapToClosedSurfaceConversionRule.h"
#include "vtkClosedSurfaceToBinaryLabelmapConversionRule.h"
#include "vtkClosedSurfaceToFractionalLabelmapConversionRule.h"
#include "vtkFractionalLabelmapToClosedSurfaceConversionRule.h"
#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkSegmentationConverterFactory.h"

// Terminologies includes
#include "vtkSlicerTerminologiesModuleLogic.h"
#include "vtkSlicerTerminologyEntry.h"

// VTK includes
#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkCallbackCommand.h>
#include <vtkDataObject.h>
#include <vtkGeneralTransform.h>
#include <vtkImageAccumulate.h>
#include <vtkImageConstantPad.h>
#include <vtkImageMathematics.h>
#include <vtkImageThreshold.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkOBJExporter.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSTLWriter.h>
#include <vtkStringArray.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkTrivialProducer.h>
#include <vtksys/SystemTools.hxx>

// MRML includes
#include <vtkMRMLScene.h>
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkMRMLSegmentationStorageNode.h"
#include "vtkMRMLSegmentEditorNode.h"
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkEventBroker.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerSegmentationsModuleLogic);
vtkCxxSetObjectMacro(vtkSlicerSegmentationsModuleLogic, TerminologiesLogic, vtkSlicerTerminologiesModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerSegmentationsModuleLogic::vtkSlicerSegmentationsModuleLogic()
 : TerminologiesLogic(NULL)
{
  this->SubjectHierarchyUIDCallbackCommand = vtkCallbackCommand::New();
  this->SubjectHierarchyUIDCallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->SubjectHierarchyUIDCallbackCommand->SetCallback( vtkSlicerSegmentationsModuleLogic::OnSubjectHierarchyUIDAdded );
}

//----------------------------------------------------------------------------
vtkSlicerSegmentationsModuleLogic::~vtkSlicerSegmentationsModuleLogic()
{
  if (this->SubjectHierarchyUIDCallbackCommand)
    {
    this->SubjectHierarchyUIDCallbackCommand->SetClientData(NULL);
    this->SubjectHierarchyUIDCallbackCommand->Delete();
    this->SubjectHierarchyUIDCallbackCommand = NULL;
    }
  this->SetTerminologiesLogic(NULL);
}

//----------------------------------------------------------------------------
void vtkSlicerSegmentationsModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerSegmentationsModuleLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  this->SetAndObserveMRMLSceneEvents(newScene, events.GetPointer());

  // Observe subject hierarchy UID events
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(newScene);
  if (shNode)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      shNode, vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemUIDAddedEvent, this, this->SubjectHierarchyUIDCallbackCommand );
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentationsModuleLogic::RegisterNodes()
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("RegisterNodes: Invalid MRML scene");
    return;
    }

  // vtkMRMLSegmentationNode, vtkMRMLSegmentationDisplayNode, and
  // vtkMRMLSegmentationStorageNode nodes are registered in vtkMRMLScene.
  this->GetMRMLScene()->RegisterNodeClass(vtkSmartPointer<vtkMRMLSegmentEditorNode>::New());

  // Register converter rules
  vtkSegmentationConverterFactory::GetInstance()->RegisterConverterRule(
    vtkSmartPointer<vtkBinaryLabelmapToClosedSurfaceConversionRule>::New() );
  vtkSegmentationConverterFactory::GetInstance()->RegisterConverterRule(
    vtkSmartPointer<vtkClosedSurfaceToBinaryLabelmapConversionRule>::New() );
  vtkSegmentationConverterFactory::GetInstance()->RegisterConverterRule(
    vtkSmartPointer<vtkClosedSurfaceToFractionalLabelmapConversionRule>::New() );
  vtkSegmentationConverterFactory::GetInstance()->RegisterConverterRule(
    vtkSmartPointer<vtkFractionalLabelmapToClosedSurfaceConversionRule>::New() );
}

//---------------------------------------------------------------------------
void vtkSlicerSegmentationsModuleLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
    {
    vtkErrorMacro("OnMRMLSceneNodeAdded: Invalid MRML scene or input node");
    return;
    }

  if (node->IsA("vtkMRMLSubjectHierarchyNode"))
    {
    vtkEventBroker::GetInstance()->AddObservation(
      node, vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemUIDAddedEvent, this, this->SubjectHierarchyUIDCallbackCommand );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSegmentationsModuleLogic::OnSubjectHierarchyUIDAdded(vtkObject* caller,
                                                                   unsigned long vtkNotUsed(eid),
                                                                   void* clientData,
                                                                   void* callData)
{
  vtkSlicerSegmentationsModuleLogic* self = reinterpret_cast<vtkSlicerSegmentationsModuleLogic*>(clientData);
  vtkMRMLSubjectHierarchyNode* shNode = reinterpret_cast<vtkMRMLSubjectHierarchyNode*>(caller);
  vtkIdType* itemWithNewUID = reinterpret_cast<vtkIdType*>(callData);
  if (!self || !shNode || !itemWithNewUID)
    {
    return;
    }

  // Call callback function in all segmentation nodes. The callback function establishes the right
  // connection between loaded DICOM volumes and segmentations (related to reference image geometry)
  std::vector<vtkMRMLNode*> segmentationNodes;
  unsigned int numberOfNodes = self->GetMRMLScene()->GetNodesByClass("vtkMRMLSegmentationNode", segmentationNodes);
  for (unsigned int nodeIndex=0; nodeIndex<numberOfNodes; nodeIndex++)
    {
    vtkMRMLSegmentationNode* node = vtkMRMLSegmentationNode::SafeDownCast(segmentationNodes[nodeIndex]);
    if (node)
      {
      node->OnSubjectHierarchyUIDAdded(shNode, *itemWithNewUID);
      }
    }
}

//-----------------------------------------------------------------------------
vtkMRMLSegmentationNode* vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentation(vtkMRMLScene* scene, vtkSegmentation* segmentation)
{
  if (!scene || !segmentation)
    {
    return NULL;
    }

  std::vector<vtkMRMLNode*> segmentationNodes;
  unsigned int numberOfNodes = scene->GetNodesByClass("vtkMRMLSegmentationNode", segmentationNodes);
  for (unsigned int nodeIndex=0; nodeIndex<numberOfNodes; nodeIndex++)
    {
    vtkMRMLSegmentationNode* node = vtkMRMLSegmentationNode::SafeDownCast(segmentationNodes[nodeIndex]);
    if (node && node->GetSegmentation() == segmentation)
      {
      return node;
      }
    }

  return NULL;
}

//-----------------------------------------------------------------------------
vtkMRMLSegmentationNode* vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegment(vtkMRMLScene* scene, vtkSegment* segment, std::string& segmentId)
{
  segmentId = "";
  if (!scene || !segment)
    {
    return NULL;
    }

  std::vector<vtkMRMLNode*> segmentationNodes;
  unsigned int numberOfNodes = scene->GetNodesByClass("vtkMRMLSegmentationNode", segmentationNodes);
  for (unsigned int nodeIndex=0; nodeIndex<numberOfNodes; nodeIndex++)
    {
    vtkMRMLSegmentationNode* node = vtkMRMLSegmentationNode::SafeDownCast(segmentationNodes[nodeIndex]);
    segmentId = node->GetSegmentation()->GetSegmentIdBySegment(segment);
    if (!segmentId.empty())
      {
      return node;
      }
    }
  return NULL;
}

//-----------------------------------------------------------------------------
vtkMRMLSegmentationNode* vtkSlicerSegmentationsModuleLogic::LoadSegmentationFromFile(const char* fileName, bool autoOpacities/*=true*/)
{
  if (this->GetMRMLScene() == NULL || fileName == NULL)
    {
    return NULL;
    }
  vtkSmartPointer<vtkMRMLSegmentationNode> segmentationNode = vtkSmartPointer<vtkMRMLSegmentationNode>::New();
  vtkSmartPointer<vtkMRMLSegmentationStorageNode> storageNode = vtkSmartPointer<vtkMRMLSegmentationStorageNode>::New();
  storageNode->SetFileName(fileName);

  // Check to see which node can read this type of file
  if (!storageNode->SupportedFileType(fileName))
    {
    vtkErrorMacro("LoadSegmentationFromFile: Segmentation storage node unable to load segmentation file.");
    return NULL;
    }

  std::string baseName = vtksys::SystemTools::GetFilenameWithoutExtension(fileName);
  std::string uname( this->GetMRMLScene()->GetUniqueNameByString(baseName.c_str()));
  segmentationNode->SetName(uname.c_str());
  std::string storageUName = uname + "_Storage";
  storageNode->SetName(storageUName.c_str());
  this->GetMRMLScene()->SaveStateForUndo();
  this->GetMRMLScene()->AddNode(storageNode.GetPointer());

  segmentationNode->SetScene(this->GetMRMLScene());
  segmentationNode->SetAndObserveStorageNodeID(storageNode->GetID());

  this->GetMRMLScene()->AddNode(segmentationNode);

  // Read file
  vtkDebugMacro("LoadSegmentationFromFile: calling read on the storage node");
  int success = storageNode->ReadData(segmentationNode);
  if (success != 1)
    {
    vtkErrorMacro("LoadSegmentationFromFile: Error reading " << fileName);
    this->GetMRMLScene()->RemoveNode(segmentationNode);
    return NULL;
    }

  // Show closed surface poly data if it exist. By default the preferred representation is shown,
  // but we do not have a display node for the segmentation here. In its absence the master representation
  // is shown if it's poly data, but closed surface model is specifically for 3D visualization)
  if (segmentationNode->GetSegmentation()->ContainsRepresentation(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName()) )
    {
    if (!segmentationNode->GetDisplayNode())
      {
      segmentationNode->CreateDefaultDisplayNodes();
      }
    vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
    if (displayNode)
      {
      displayNode->SetPreferredDisplayRepresentationName2D(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
      displayNode->SetPreferredDisplayRepresentationName3D(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());

      // If not loading segmentation from a scene (where display information is available),
      // then calculate and set auto-opacity for the displayed poly data for better visualization
      if (autoOpacities)
        {
        displayNode->CalculateAutoOpacitiesForSegments();
        }
      }
    }

  return segmentationNode.GetPointer();
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::CreateLabelmapVolumeFromOrientedImageData(
  vtkOrientedImageData* orientedImageData, vtkMRMLLabelMapVolumeNode* labelmapVolumeNode)
{
  if (!vtkSlicerSegmentationsModuleLogic::CopyOrientedImageDataToVolumeNode(orientedImageData, labelmapVolumeNode, true, true))
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::CreateLabelmapVolumeFromOrientedImageData: failed to copy into volume node");
    return false;
    }

  // Create default display node if it does not have one
  if (labelmapVolumeNode->GetScene())
    {
    vtkSmartPointer<vtkMRMLLabelMapVolumeDisplayNode> labelmapVolumeDisplayNode = vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(
      labelmapVolumeNode->GetDisplayNode() );
    if (!labelmapVolumeDisplayNode.GetPointer())
      {
      labelmapVolumeDisplayNode = vtkSmartPointer<vtkMRMLLabelMapVolumeDisplayNode>::New();
      labelmapVolumeNode->GetScene()->AddNode(labelmapVolumeDisplayNode);
      labelmapVolumeNode->SetAndObserveDisplayNodeID(labelmapVolumeDisplayNode->GetID());
      labelmapVolumeDisplayNode->SetDefaultColorMap();
      }
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::CopyOrientedImageDataToVolumeNode(
  vtkOrientedImageData* orientedImageData, vtkMRMLVolumeNode* volumeNode, bool shallowCopy /*=true*/, bool shiftImageDataExtentToZeroStart /*=true*/)
{
  if (!orientedImageData)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::CopyOrientedImageDataToVolumeNode: Invalid input image data");
    return false;
    }
  if (!volumeNode)
    {
    vtkErrorWithObjectMacro(orientedImageData, "CopyOrientedImageDataToVolumeNode: Invalid input volume node");
    return false;
    }

  // Create an identity (zero origin, unit spacing, identity orientation) vtkImageData that can be stored in vtkMRMLVolumeNode
  vtkSmartPointer<vtkImageData> identityImageData = vtkSmartPointer<vtkImageData>::New();
  if (shallowCopy)
    {
    identityImageData->ShallowCopy(orientedImageData);
    }
  else
    {
    identityImageData->DeepCopy(orientedImageData);
    }
  identityImageData->SetOrigin(0,0,0);
  identityImageData->SetSpacing(1,1,1);
  volumeNode->SetAndObserveImageData(identityImageData);

  vtkSmartPointer<vtkMatrix4x4> labelmapImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  orientedImageData->GetImageToWorldMatrix(labelmapImageToWorldMatrix);
  volumeNode->SetIJKToRASMatrix(labelmapImageToWorldMatrix);

  if (shiftImageDataExtentToZeroStart)
    {
    volumeNode->ShiftImageDataExtentToZeroStart();
    }

  return true;
}

//-----------------------------------------------------------------------------
vtkOrientedImageData* vtkSlicerSegmentationsModuleLogic::CreateOrientedImageDataFromVolumeNode(vtkMRMLScalarVolumeNode* volumeNode, vtkMRMLTransformNode* outputParentTransformNode /* = NULL */)
{
  if (!volumeNode || !volumeNode->GetImageData())
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::CreateOrientedImageDataFromVolumeNode: Invalid volume node");
    return NULL;
    }

  vtkOrientedImageData* orientedImageData = vtkOrientedImageData::New();
  orientedImageData->vtkImageData::DeepCopy(volumeNode->GetImageData());

  vtkSmartPointer<vtkMatrix4x4> ijkToRasMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  volumeNode->GetIJKToRASMatrix(ijkToRasMatrix);
  orientedImageData->SetGeometryFromImageToWorldMatrix(ijkToRasMatrix);

  // Apply parent transform of the volume node if any
  if (volumeNode->GetParentTransformNode() != outputParentTransformNode)
    {
    vtkSmartPointer<vtkGeneralTransform> nodeToOutputTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    vtkMRMLTransformNode::GetTransformBetweenNodes(volumeNode->GetParentTransformNode(), outputParentTransformNode, nodeToOutputTransform);
    vtkOrientedImageDataResample::TransformOrientedImage(orientedImageData, nodeToOutputTransform);
    }

  return orientedImageData;
}

//-----------------------------------------------------------------------------
int vtkSlicerSegmentationsModuleLogic::DoesLabelmapContainSingleLabel(vtkMRMLLabelMapVolumeNode* labelmapVolumeNode)
{
  if (!labelmapVolumeNode || !labelmapVolumeNode->GetImageData())
  {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::DoesLabelmapContainSingleLabel: Invalid labelmap volume MRML node");
    return 0;
  }
  int highLabel = (int)(ceil(labelmapVolumeNode->GetImageData()->GetScalarRange()[1]));
  if (highLabel == 0)
  {
    return 0;
  }
  vtkSmartPointer<vtkImageAccumulate> imageAccumulate = vtkSmartPointer<vtkImageAccumulate>::New();
  imageAccumulate->SetInputConnection(labelmapVolumeNode->GetImageDataConnection());
  imageAccumulate->IgnoreZeroOn();
  imageAccumulate->Update();
  int lowLabel = (int)imageAccumulate->GetMin()[0];
  highLabel = (int)imageAccumulate->GetMax()[0];
  if (lowLabel != highLabel)
    {
    return 0;
    }

  return lowLabel;
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentationsModuleLogic::GetAllLabelValues(vtkIntArray* labels, vtkImageData* labelmap)
{
  if (!labels)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::GetAllLabelValues: Invalid labels");
    return;
    }
  labels->Reset();
  if (!labelmap)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::GetAllLabelValues: Invalid labelmap");
    return;
   }
  double* scalarRange = labelmap->GetScalarRange();
  int lowLabel = (int)(floor(scalarRange[0]));
  int highLabel = (int)(ceil(scalarRange[1]));
  vtkSmartPointer<vtkImageAccumulate> imageAccumulate = vtkSmartPointer<vtkImageAccumulate>::New();
  imageAccumulate->SetInputData(labelmap);
  imageAccumulate->IgnoreZeroOn(); // Do not create segment from background
  imageAccumulate->SetComponentExtent(lowLabel, highLabel, 0, 0, 0, 0);
  imageAccumulate->SetComponentOrigin(0, 0, 0);
  imageAccumulate->SetComponentSpacing(1, 1, 1);
  imageAccumulate->Update();

  for (int label = lowLabel; label <= highLabel; ++label)
    {
    if (label == 0)
      {
      continue;
      }
    double frequency = imageAccumulate->GetOutput()->GetPointData()->GetScalars()->GetTuple1(label - lowLabel);
    if (frequency == 0.0)
      {
      continue;
      }
    labels->InsertNextValue(label);
    }
}


//-----------------------------------------------------------------------------
vtkSegment* vtkSlicerSegmentationsModuleLogic::CreateSegmentFromLabelmapVolumeNode(vtkMRMLLabelMapVolumeNode* labelmapVolumeNode, vtkMRMLSegmentationNode* segmentationNode/*=NULL*/)
{
  if (!labelmapVolumeNode)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::CreateSegmentFromLabelmapVolumeNode: Invalid labelmap volume MRML node");
    return NULL;
    }

  // Cannot create single segment from labelmap node if it contains more than one segment
  int label = vtkSlicerSegmentationsModuleLogic::DoesLabelmapContainSingleLabel(labelmapVolumeNode);
  if (!label)
    {
    vtkErrorWithObjectMacro(labelmapVolumeNode, "CreateSegmentFromLabelmapVolumeNode: Unable to create single segment from labelmap volume node, as labelmap contains more than one label");
    return NULL;
    }

  // Create segment
  vtkSegment* segment = vtkSegment::New();
  segment->SetName(labelmapVolumeNode->GetName());

  // Set segment color
  double color[4] = { vtkSegment::SEGMENT_COLOR_INVALID[0],
                      vtkSegment::SEGMENT_COLOR_INVALID[1],
                      vtkSegment::SEGMENT_COLOR_INVALID[2], 1.0 };
  vtkMRMLColorTableNode* colorNode = NULL;
  if (labelmapVolumeNode->GetDisplayNode())
    {
    colorNode = vtkMRMLColorTableNode::SafeDownCast(labelmapVolumeNode->GetDisplayNode()->GetColorNode());
    if (colorNode)
      {
      colorNode->GetColor(label, color);
      }
    }
  segment->SetColor(color[0], color[1], color[2]);

  // Create oriented image data from labelmap
  vtkSmartPointer<vtkOrientedImageData> orientedImageData = vtkSmartPointer<vtkOrientedImageData>::Take(
    vtkSlicerSegmentationsModuleLogic::CreateOrientedImageDataFromVolumeNode(labelmapVolumeNode) );

  // Apply parent transforms if any
  if (labelmapVolumeNode->GetParentTransformNode() || (segmentationNode && segmentationNode->GetParentTransformNode()))
    {
    vtkSmartPointer<vtkGeneralTransform> labelmapToSegmentationTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    if (segmentationNode)
      {
      vtkSlicerSegmentationsModuleLogic::GetTransformBetweenRepresentationAndSegmentation(labelmapVolumeNode, segmentationNode, labelmapToSegmentationTransform);
      }
    else
      {
      // Get parent transform which is the representation to world transform in absence of a segmentation node
      vtkMRMLTransformNode* representationParentTransformNode = labelmapVolumeNode->GetParentTransformNode();
      if (representationParentTransformNode)
        {
        representationParentTransformNode->GetTransformToWorld(labelmapToSegmentationTransform);
        }
      }
    vtkOrientedImageDataResample::TransformOrientedImage(orientedImageData, labelmapToSegmentationTransform);
    }

  // Add oriented image data as binary labelmap representation
  segment->AddRepresentation(
    vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName(),
    orientedImageData );

  return segment;
}

//-----------------------------------------------------------------------------
vtkSegment* vtkSlicerSegmentationsModuleLogic::CreateSegmentFromModelNode(vtkMRMLModelNode* modelNode, vtkMRMLSegmentationNode* segmentationNode/*=NULL*/)
{
  if (!modelNode)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::CreateSegmentFromModelNode: Invalid model MRML node");
    return NULL;
    }
  if (!modelNode->GetPolyData())
    {
    vtkErrorWithObjectMacro(modelNode, "CreateSegmentFromModelNode: Model node does not contain poly data");
    return NULL;
    }

  double color[3] = { vtkSegment::SEGMENT_COLOR_INVALID[0],
                      vtkSegment::SEGMENT_COLOR_INVALID[1],
                      vtkSegment::SEGMENT_COLOR_INVALID[2] };

  // Create oriented image data from labelmap volume node
  vtkSegment* segment = vtkSegment::New();
    segment->SetName(modelNode->GetName());

  // Color from display node
  vtkMRMLDisplayNode* modelDisplayNode = modelNode->GetDisplayNode();
  if (modelDisplayNode)
    {
    modelDisplayNode->GetColor(color);
    segment->SetColor(color);
    }

  // Make a copy of the model's poly data to set it in the segment
  vtkSmartPointer<vtkPolyData> polyDataCopy = vtkSmartPointer<vtkPolyData>::New();

  // Apply parent transforms if any
  if (modelNode->GetParentTransformNode() || (segmentationNode && segmentationNode->GetParentTransformNode()))
    {
    vtkSmartPointer<vtkGeneralTransform> modelToSegmentationTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    if (segmentationNode)
      {
      vtkSlicerSegmentationsModuleLogic::GetTransformBetweenRepresentationAndSegmentation(modelNode, segmentationNode, modelToSegmentationTransform);
      }
    else
      {
      // Get parent transform which is the representation to world transform in absence of a segmentation node
      vtkMRMLTransformNode* representationParentTransformNode = modelNode->GetParentTransformNode();
      if (representationParentTransformNode)
        {
        representationParentTransformNode->GetTransformToWorld(modelToSegmentationTransform);
        }
      }

    vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilter->SetInputData(modelNode->GetPolyData());
    transformFilter->SetTransform(modelToSegmentationTransform);
    transformFilter->Update();
    polyDataCopy->DeepCopy(transformFilter->GetOutput());
    }
  else
    {
    polyDataCopy->DeepCopy(modelNode->GetPolyData());
    }

  // Add model poly data as closed surface representation
  segment->AddRepresentation(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(),
    polyDataCopy );

  return segment;
}

//-----------------------------------------------------------------------------
vtkMRMLSegmentationNode* vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem(
  vtkIdType segmentShItemID, vtkMRMLScene* scene )
{
  if (!scene)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem: Invalid MRML scene");
    return NULL;
    }
  if (segmentShItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    vtkErrorWithObjectMacro(scene, "vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem: Invalid subject hierarchy item");
    return NULL;
    }

  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
  if (!shNode)
    {
    vtkErrorWithObjectMacro(scene, "vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem: Failed to access subject hierarchy");
    return NULL;
    }

  vtkIdType parentShItem = shNode->GetItemParent(segmentShItemID);
  if (parentShItem == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    vtkErrorWithObjectMacro(scene, "vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem:"
      << " Segment subject hierarchy item has no segmentation parent");
    return NULL;
    }
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
    shNode->GetItemDataNode(parentShItem) );
  if (!segmentationNode)
    {
    vtkErrorWithObjectMacro(scene, "vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem:"
      << " Segment subject hierarchy item's parent has no associated segmentation node");
    return NULL;
    }

  return segmentationNode;
}

//-----------------------------------------------------------------------------
vtkSegment* vtkSlicerSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyItem(vtkIdType segmentShItemID, vtkMRMLScene* scene)
{
  if (!scene)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyItem: Invalid MRML scene");
    return NULL;
    }
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(scene);
  if (!shNode)
    {
    vtkErrorWithObjectMacro(scene, "vtkSlicerSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyItem: Failed to access subject hierarchy");
    return NULL;
    }

  vtkMRMLSegmentationNode* segmentationNode =
    vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem(segmentShItemID, scene);
  if (!segmentationNode)
    {
    return NULL;
    }

  std::string segmentId = shNode->GetItemAttribute(segmentShItemID, vtkMRMLSegmentationNode::GetSegmentIDAttributeName());
  if (segmentId.empty())
    {
    vtkErrorWithObjectMacro(scene, "vtkSlicerSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyItem:"
      << " Segment subject hierarchy item " << shNode->GetItemName(segmentShItemID) << " does not contain segment ID");
    return NULL;
    }

  vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(segmentId);
  if (!segment)
    {
    vtkErrorWithObjectMacro(scene, "vtkSlicerSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyItem: "
      "Segmentation does not contain segment with given ID: " << (segmentId.empty() ? "(empty)" : segmentId.c_str()));
    }

  return segment;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ExportSegmentToRepresentationNode(vtkSegment* segment, vtkMRMLNode* representationNode)
{
  if (!segment)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ExportSegmentToRepresentationNode: Invalid segment");
    return false;
    }
  if (!representationNode)
    {
    vtkErrorWithObjectMacro(segment, "ExportSegmentToRepresentationNode: Invalid representation MRML node");
    return false;
    }
  vtkMRMLLabelMapVolumeNode* labelmapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(representationNode);
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(representationNode);
  if (!labelmapNode && !modelNode)
    {
    vtkErrorWithObjectMacro(representationNode, "ExportSegmentToRepresentationNode: Representation MRML node should be either labelmap volume node or model node");
    return false;
    }

  // Determine segment ID and set it as name of the representation node if found
  std::string segmentId("");
  vtkMRMLSegmentationNode* segmentationNode = vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegment(
    representationNode->GetScene(), segment, segmentId);
  if (segmentationNode)
    {
    representationNode->SetName(segment->GetName());
    }
  vtkMRMLTransformNode* parentTransformNode = segmentationNode->GetParentTransformNode();

  if (labelmapNode)
    {
    // Make sure binary labelmap representation exists in segment
    bool binaryLabelmapPresent = segment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
    if (!binaryLabelmapPresent && !segmentationNode)
      {
      vtkErrorWithObjectMacro(representationNode, "ExportSegmentToRepresentationNode: Segment does not contain binary labelmap representation and cannot convert, because it is not in a segmentation");
      return false;
      }
    binaryLabelmapPresent = segmentationNode->GetSegmentation()->CreateRepresentation(
      vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
    if (!binaryLabelmapPresent)
      {
      vtkErrorWithObjectMacro(representationNode, "ExportSegmentToRepresentationNode: Unable to convert segment to binary labelmap representation");
      return false;
      }

    // Export binary labelmap representation into labelmap volume node
    vtkOrientedImageData* orientedImageData = vtkOrientedImageData::SafeDownCast(
      segment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()) );
    bool success = vtkSlicerSegmentationsModuleLogic::CreateLabelmapVolumeFromOrientedImageData(orientedImageData, labelmapNode);
    if (!success)
      {
      return false;
      }

    // Set segmentation's parent transform to exported node
    if (parentTransformNode)
      {
      labelmapNode->SetAndObserveTransformNodeID(parentTransformNode->GetID());
      }

    return true;
    }
  else if (modelNode)
    {
    // Make sure closed surface representation exists in segment
    bool closedSurfacePresent = segmentationNode->GetSegmentation()->CreateRepresentation(
      vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
    if (!closedSurfacePresent)
      {
      vtkErrorWithObjectMacro(representationNode, "ExportSegmentToRepresentationNode: Unable to convert segment to closed surface representation");
      return false;
      }

    // Export binary labelmap representation into labelmap volume node
    vtkPolyData* polyData = vtkPolyData::SafeDownCast(
      segment->GetRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName()) );
    vtkSmartPointer<vtkPolyData> polyDataCopy = vtkSmartPointer<vtkPolyData>::New();
    polyDataCopy->DeepCopy(polyData); // Make copy of poly data so that the model node does not change if segment changes
    modelNode->SetAndObservePolyData(polyDataCopy);

    // Set color of the exported model
    vtkMRMLSegmentationDisplayNode* segmentationDisplayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
    vtkMRMLDisplayNode* modelDisplayNode = modelNode->GetDisplayNode();
    if (!modelDisplayNode)
      {
      // Create display node
      vtkSmartPointer<vtkMRMLModelDisplayNode> displayNode = vtkSmartPointer<vtkMRMLModelDisplayNode>::New();
      displayNode = vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetScene()->AddNode(displayNode));
      displayNode->VisibilityOn();
      modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());
      modelDisplayNode = displayNode.GetPointer();
      }
    if (segmentationDisplayNode && modelDisplayNode)
      {
      modelDisplayNode->SetColor(segment->GetColor());
      }

    // Set segmentation's parent transform to exported node
    if (parentTransformNode)
      {
      modelNode->SetAndObserveTransformNodeID(parentTransformNode->GetID());
      }
      else
      {
      modelNode->SetAndObserveTransformNodeID(NULL);
      }

    return true;
    }

  // Representation node is neither labelmap, nor model
  return false;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ExportSegmentsToModelHierarchy(vtkMRMLSegmentationNode* segmentationNode,
  std::vector<std::string>& segmentIDs, vtkMRMLModelHierarchyNode* modelHierarchyNode)
{
  if (!segmentationNode || !segmentationNode->GetScene())
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ExportSegmentsToModelHierarchy: Invalid segmentation node");
    return false;
    }
  if (!modelHierarchyNode || !modelHierarchyNode->GetScene() || modelHierarchyNode->GetScene() != segmentationNode->GetScene())
    {
    vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsToModelHierarchy: Invalid model hierarchy node");
    return false;
    }

  // Make sure closed surface representation exists in segment
  bool closedSurfacePresent = segmentationNode->GetSegmentation()->CreateRepresentation(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName() );
  if (!closedSurfacePresent)
    {
    vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsToModelHierarchy: Unable to convert segment to closed surface representation");
    return false;
    }

  // Create a map that can be used for quickly looking up existing models in a hierarchy
  vtkNew<vtkCollection> existingModels;
  modelHierarchyNode->GetChildrenModelNodes(existingModels.GetPointer());
  std::map< std::string, vtkMRMLModelNode* > existingModelNamesToModels;
  vtkObject* object = NULL;
  vtkCollectionSimpleIterator it;
  for (existingModels->InitTraversal(it); (object = existingModels->GetNextItemAsObject(it));)
    {
    vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(object);
    if (!modelNode)
      {
      continue;
      }
    existingModelNamesToModels[modelNode->GetName()] = modelNode;
    }

  std::vector<std::string> exportedSegmentIDs;
  if (segmentIDs.empty())
    {
    segmentationNode->GetSegmentation()->GetSegmentIDs(exportedSegmentIDs);
    }
  else
    {
    exportedSegmentIDs = segmentIDs;
    }

  // Export each segment into a model
  for (std::vector<std::string>::iterator segmentIdIt = exportedSegmentIDs.begin(); segmentIdIt != exportedSegmentIDs.end(); ++segmentIdIt)
    {
    // Export segment into model node
    vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(*segmentIdIt);
    vtkMRMLModelNode* modelNode = NULL;
    if (existingModelNamesToModels.find(segment->GetName()) != existingModelNamesToModels.end())
      {
      // Model by the same name exists in the selected hierarchy, overwrite that model
      modelNode = existingModelNamesToModels[segment->GetName()];
      }
    else
      {
      // Create new model node
      vtkNew<vtkMRMLModelNode> newModelNode;
      modelHierarchyNode->GetScene()->AddNode(newModelNode.GetPointer());
      newModelNode->CreateDefaultDisplayNodes();
      modelNode = newModelNode.GetPointer();
      // Add to model hierarchy
      vtkNew<vtkMRMLModelHierarchyNode> newModelHierarchyNode;
      newModelHierarchyNode->SetHideFromEditors(true);
      modelHierarchyNode->GetScene()->AddNode(newModelHierarchyNode.GetPointer());
      newModelHierarchyNode->SetAssociatedNodeID(modelNode->GetID());
      newModelHierarchyNode->SetParentNodeID(modelHierarchyNode->GetID());
      }

    // Export segment into model node
    if (!vtkSlicerSegmentationsModuleLogic::ExportSegmentToRepresentationNode(segment, modelNode))
      {
      vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsToModelHierarchy: Failed to export segmentation into model hierarchy");
      return false;
      }
    }

  // Move exported representation under same parent as segmentation
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(segmentationNode->GetScene());
  if (shNode)
    {
    shNode->SetItemParent(shNode->GetItemByDataNode(modelHierarchyNode),
      shNode->GetItemParent(shNode->GetItemByDataNode(segmentationNode)) );
    }
  else
    {
    vtkWarningWithObjectMacro(segmentationNode, "ExportSegmentsToModelHierarchy: Failed to access subject hierarchy node");
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ExportSegmentsToModelHierarchy(vtkMRMLSegmentationNode* segmentationNode,
  vtkStringArray* segmentIds, vtkMRMLModelHierarchyNode* modelHierarchyNode)
{
  std::vector<std::string> segmentIdsVector;
  if (segmentIds == NULL)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ExportSegmentsToModelHierarchy failed: invalid segmentIDs");
    return false;
    }
  for (int segmentIndex = 0; segmentIndex < segmentIds->GetNumberOfValues(); ++segmentIndex)
    {
    segmentIdsVector.push_back(segmentIds->GetValue(segmentIndex));
    }
  return vtkSlicerSegmentationsModuleLogic::ExportSegmentsToModelHierarchy(segmentationNode, segmentIdsVector, modelHierarchyNode);
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ExportVisibleSegmentsToModelHierarchy(vtkMRMLSegmentationNode* segmentationNode, vtkMRMLModelHierarchyNode* modelHierarchyNode)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ExportVisibleSegmentsToModelHierarchy: Invalid segmentation node");
    return false;
    }

  std::vector<std::string> visibleSegmentIDs;
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
  displayNode->GetVisibleSegmentIDs(visibleSegmentIDs);

  return vtkSlicerSegmentationsModuleLogic::ExportSegmentsToModelHierarchy(segmentationNode, visibleSegmentIDs, modelHierarchyNode);
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ExportAllSegmentsToModelHierarchy(vtkMRMLSegmentationNode* segmentationNode, vtkMRMLModelHierarchyNode* modelHierarchyNode)
{
  std::vector<std::string> segmentIDs;
  return vtkSlicerSegmentationsModuleLogic::ExportSegmentsToModelHierarchy(segmentationNode, segmentIDs, modelHierarchyNode);
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ExportSegmentsToLabelmapNode(vtkMRMLSegmentationNode* segmentationNode,
  std::vector<std::string>& segmentIDs, vtkMRMLLabelMapVolumeNode* labelmapNode, vtkMRMLVolumeNode* referenceVolumeNode /*=NULL*/)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ExportSegmentsToLabelmapNode: Invalid segmentation node");
    return false;
    }
  if (!labelmapNode)
    {
    vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsToLabelmapNode: Invalid labelmap volume node");
    return false;
    }

  // Make sure binary labelmap representation exists in segment
  bool binaryLabelmapPresent = segmentationNode->GetSegmentation()->CreateRepresentation(
    vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
  if (!binaryLabelmapPresent)
    {
    vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsToLabelmapNode: Unable to convert segment to binary labelmap representation");
    return false;
    }

  // Use reference volume's parent transform if available, otherwise put under the same transform as segmentation node
  vtkMRMLTransformNode* parentTransformNode = NULL;
  if (referenceVolumeNode)
    {
    parentTransformNode = referenceVolumeNode->GetParentTransformNode();
    }
  else
    {
    parentTransformNode = segmentationNode->GetParentTransformNode();
    }
  labelmapNode->SetAndObserveTransformNodeID(parentTransformNode ? parentTransformNode->GetID() : "");

  // Get reference geometry in the segmentation node's coordinate system
  vtkSmartPointer<vtkOrientedImageData> referenceGeometry_Reference; // reference geometry in reference node coordinate system
  vtkSmartPointer<vtkOrientedImageData> referenceGeometry_Segmentation; // reference geometry in segmentation coordinate system
  vtkSmartPointer<vtkGeneralTransform> referenceGeometryToSegmentationTransform;
  if (referenceVolumeNode && referenceVolumeNode->GetImageData())
    {
    // Create (non-allocated) image data that matches reference geometry
    referenceGeometry_Reference = vtkSmartPointer<vtkOrientedImageData>::New();
    referenceGeometry_Reference->SetExtent(referenceVolumeNode->GetImageData()->GetExtent());
    vtkSmartPointer<vtkMatrix4x4> ijkToRasMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    referenceVolumeNode->GetIJKToRASMatrix(ijkToRasMatrix);
    referenceGeometry_Reference->SetGeometryFromImageToWorldMatrix(ijkToRasMatrix);

    // Transform it to the segmentation node coordinate system
    referenceGeometry_Segmentation = vtkSmartPointer<vtkOrientedImageData>::New();
    referenceGeometry_Segmentation->DeepCopy(referenceGeometry_Reference);
    // Apply parent transform of the volume node if any
    if (referenceVolumeNode->GetParentTransformNode() != segmentationNode->GetParentTransformNode())
      {
      referenceGeometryToSegmentationTransform = vtkSmartPointer<vtkGeneralTransform>::New();
      vtkMRMLTransformNode::GetTransformBetweenNodes(referenceVolumeNode->GetParentTransformNode(),
        segmentationNode->GetParentTransformNode(), referenceGeometryToSegmentationTransform);
      vtkOrientedImageDataResample::TransformOrientedImage(referenceGeometry_Segmentation, referenceGeometryToSegmentationTransform, true /* geometry only */);
      }
    }

  // Generate merged labelmap for the exported segments
  vtkSmartPointer<vtkOrientedImageData> mergedImage_Segmentation = vtkSmartPointer<vtkOrientedImageData>::New();
  if (!segmentationNode->GenerateMergedLabelmap(mergedImage_Segmentation, vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS,
    referenceGeometry_Segmentation, segmentIDs))
    {
    vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsToLabelmapNode: Failed to generate merged labelmap");
    return false;
    }

  // Transform merged labelmap to reference geometry coordinate system
  vtkSmartPointer<vtkOrientedImageData> mergedImage_Reference;
  if (referenceGeometryToSegmentationTransform)
    {
    mergedImage_Reference = vtkSmartPointer<vtkOrientedImageData>::New();
    vtkAbstractTransform* segmentationToReferenceGeometryTransform = referenceGeometryToSegmentationTransform->GetInverse();
    segmentationToReferenceGeometryTransform->Update();
    vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(mergedImage_Segmentation, referenceGeometry_Reference, mergedImage_Reference,
      false /* nearest neighbor interpolation*/, false /* no padding */, segmentationToReferenceGeometryTransform);
    }
  else
    {
    mergedImage_Reference = mergedImage_Segmentation;
    }
  mergedImage_Segmentation = NULL; // free up memory

  // Export merged labelmap to the output node
  if (!vtkSlicerSegmentationsModuleLogic::CreateLabelmapVolumeFromOrientedImageData(mergedImage_Reference, labelmapNode))
    {
    vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsToLabelmapNode: Failed to create labelmap from merged segments image");
    return false;
    }

  // Create/update color table to labelmap so that the labels appear in the same color
  if (!labelmapNode->GetDisplayNode())
    {
    labelmapNode->CreateDefaultDisplayNodes();
    }
  if (!segmentationNode->GetDisplayNode())
    {
    segmentationNode->CreateDefaultDisplayNodes();
    }
  if (!labelmapNode->GetDisplayNode()->GetColorNode() || labelmapNode->GetDisplayNode()->GetColorNode()->GetType() != vtkMRMLColorNode::User)
    {
    // Create new color table node if labelmap node doesn't have a color node or if the existing one is not user type
    vtkSmartPointer<vtkMRMLColorTableNode> newColorTable = vtkSmartPointer<vtkMRMLColorTableNode>::New();
    // Need to make the color table node visible because only non-hidden storable nodes are offered to be saved
    newColorTable->SetHideFromEditors(false);
    std::string colorTableNodeName(labelmapNode->GetName());
    colorTableNodeName.append("_ColorTable");
    newColorTable->SetName(colorTableNodeName.c_str());
    newColorTable->SetTypeToUser();
    newColorTable->NamesInitialisedOn();
    newColorTable->SetAttribute("Category", "Segmentations");
    labelmapNode->GetScene()->AddNode(newColorTable);
    labelmapNode->GetDisplayNode()->SetAndObserveColorNodeID(newColorTable->GetID());
    }
  // Copy segment colors to color table node
  vtkMRMLColorTableNode* colorTableNode = vtkMRMLColorTableNode::SafeDownCast(
    labelmapNode->GetDisplayNode()->GetColorNode() ); // Always valid, as it was created just above if was missing
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
    segmentationNode->GetDisplayNode() );
  std::vector<std::string> exportedSegmentIDs;
  if (segmentIDs.empty())
    {
    segmentationNode->GetSegmentation()->GetSegmentIDs(exportedSegmentIDs);
    }
  else
    {
    exportedSegmentIDs = segmentIDs;
    }
  colorTableNode->SetNumberOfColors(exportedSegmentIDs.size() + 1);
  colorTableNode->GetLookupTable()->SetNumberOfTableValues(exportedSegmentIDs.size() + 1);
  colorTableNode->SetColor(0, "Background", 0.0, 0.0, 0.0, 0.0);
  short colorIndex = 1;
  for (std::vector<std::string>::iterator segmentIt = exportedSegmentIDs.begin(); segmentIt != exportedSegmentIDs.end(); ++segmentIt, ++colorIndex)
    {
    const char* segmentName = segmentationNode->GetSegmentation()->GetSegment(*segmentIt)->GetName();
    vtkVector3d color = displayNode->GetSegmentColor(*segmentIt);
    colorTableNode->SetColor(colorIndex, segmentName, color.GetX(), color.GetY(), color.GetZ());
    }

  // Move exported labelmap node under same parent as segmentation
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(segmentationNode->GetScene());
  if (shNode)
    {
    shNode->SetItemParent(shNode->GetItemByDataNode(labelmapNode),
      shNode->GetItemParent(shNode->GetItemByDataNode(segmentationNode)) );
    }
  else
    {
    vtkWarningWithObjectMacro(segmentationNode, "ExportSegmentsToLabelmapNode: Failed to access subject hierarchy node");
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ExportSegmentsToLabelmapNode(vtkMRMLSegmentationNode* segmentationNode,
  vtkStringArray* segmentIds, vtkMRMLLabelMapVolumeNode* labelmapNode, vtkMRMLVolumeNode* referenceVolumeNode /*=NULL*/)
{
  std::vector<std::string> segmentIdsVector;
  if (segmentIds == NULL)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ExportSegmentsToLabelmapNode failed: invalid segmentIDs");
    return false;
    }
  for (int segmentIndex = 0; segmentIndex < segmentIds->GetNumberOfValues(); ++segmentIndex)
    {
    segmentIdsVector.push_back(segmentIds->GetValue(segmentIndex));
    }
  return vtkSlicerSegmentationsModuleLogic::ExportSegmentsToLabelmapNode(segmentationNode, segmentIdsVector, labelmapNode, referenceVolumeNode);
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ExportVisibleSegmentsToLabelmapNode(vtkMRMLSegmentationNode* segmentationNode,
  vtkMRMLLabelMapVolumeNode* labelmapNode, vtkMRMLVolumeNode* referenceVolumeNode /*=NULL*/)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ExportVisibleSegmentsToLabelmapNode: Invalid segmentation node");
    return false;
    }

  std::vector<std::string> visibleSegmentIDs;
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
  displayNode->GetVisibleSegmentIDs(visibleSegmentIDs);

  return vtkSlicerSegmentationsModuleLogic::ExportSegmentsToLabelmapNode(segmentationNode, visibleSegmentIDs, labelmapNode, referenceVolumeNode);
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ExportAllSegmentsToLabelmapNode(vtkMRMLSegmentationNode* segmentationNode, vtkMRMLLabelMapVolumeNode* labelmapNode)
{
  std::vector<std::string> segmentIDs;
  return vtkSlicerSegmentationsModuleLogic::ExportSegmentsToLabelmapNode(segmentationNode, segmentIDs, labelmapNode);
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ImportModelToSegmentationNode(vtkMRMLModelNode* modelNode,
  vtkMRMLSegmentationNode* segmentationNode, std::string insertBeforeSegmentId/*=""*/)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ImportModelToSegmentationNode: Invalid segmentation node");
    return false;
    }
  if (!modelNode || !modelNode->GetPolyData())
    {
    vtkErrorWithObjectMacro(segmentationNode, "ImportModelToSegmentationNode: Invalid model node");
    return false;
    }
  vtkSmartPointer<vtkSegment> segment = vtkSmartPointer<vtkSegment>::Take(
    vtkSlicerSegmentationsModuleLogic::CreateSegmentFromModelNode(modelNode, segmentationNode));
  if (!segment.GetPointer())
    {
    return false;
    }

  if (!segmentationNode->GetDisplayNode())
    {
    segmentationNode->CreateDefaultDisplayNodes();
    }

  // Add segment to current segmentation
  segmentationNode->GetSegmentation()->AddSegment(segment, "", insertBeforeSegmentId);

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ImportModelHierarchyToSegmentationNode(vtkMRMLModelHierarchyNode* modelHierarchyNode,
  vtkMRMLSegmentationNode* segmentationNode, std::string vtkNotUsed(insertBeforeSegmentId)/*=""*/)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ImportModelHierarchyToSegmentationNode: Invalid segmentation node");
    return false;
    }
  if (!modelHierarchyNode)
    {
    vtkErrorWithObjectMacro(segmentationNode, "ImportModelHierarchyToSegmentationNode: Invalid model hierarchy node");
    return false;
    }

  // Get model nodes
  bool returnValue = true;
  vtkMRMLModelNode* modelNode = NULL;
  vtkNew<vtkCollection> modelNodes;
  modelHierarchyNode->GetChildrenModelNodes(modelNodes.GetPointer());
  vtkObject* object = NULL;
  vtkCollectionSimpleIterator it;
  for (modelNodes->InitTraversal(it); (object = modelNodes->GetNextItemAsObject(it));)
    {
    modelNode = vtkMRMLModelNode::SafeDownCast(object);
    if (!modelNode)
      {
      continue;
      }
    // TODO: look up segment with matching name and overwrite that
    if (!vtkSlicerSegmentationsModuleLogic::ImportModelToSegmentationNode(modelNode, segmentationNode))
      {
      vtkErrorWithObjectMacro(segmentationNode, "ImportModelHierarchyToSegmentationNode: Failed to import model node "
        << modelNode->GetName() << " to segmentation " << segmentationNode->GetName());
      returnValue = false;
      }
    }

  return returnValue;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(vtkMRMLLabelMapVolumeNode* labelmapNode,
  vtkMRMLSegmentationNode* segmentationNode, std::string insertBeforeSegmentId/*=""*/)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode: Invalid segmentation node");
    return false;
    }
  if (!labelmapNode || !labelmapNode->GetImageData())
    {
    vtkErrorWithObjectMacro(segmentationNode, "ImportLabelmapToSegmentationNode: Invalid labelmap volume node");
    return false;
    }

  // Get labelmap geometry
  vtkSmartPointer<vtkMatrix4x4> labelmapIjkToRasMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  labelmapNode->GetIJKToRASMatrix(labelmapIjkToRasMatrix);

  // Note: Splitting code ported from EditorLib/HelperBox.py:split

  // Get color node
  vtkMRMLColorTableNode* colorNode = NULL;
  if (labelmapNode->GetDisplayNode())
    {
    colorNode = vtkMRMLColorTableNode::SafeDownCast(labelmapNode->GetDisplayNode()->GetColorNode());
    }

  if (!segmentationNode->GetDisplayNode())
    {
    segmentationNode->CreateDefaultDisplayNodes();
    }

  // Split labelmap node into per-label image data

  vtkNew<vtkIntArray> labelValues;
  vtkSlicerSegmentationsModuleLogic::GetAllLabelValues(labelValues.GetPointer(), labelmapNode->GetImageData());

  vtkSmartPointer<vtkImageThreshold> threshold = vtkSmartPointer<vtkImageThreshold>::New();
  threshold->SetInputConnection(labelmapNode->GetImageDataConnection());
  threshold->SetInValue(1);
  threshold->SetOutValue(0);
  threshold->ReplaceInOn();
  threshold->ReplaceOutOn();
  threshold->SetOutputScalarType(labelmapNode->GetImageData()->GetScalarType());
  //TODO: pending resolution of bug http://www.na-mic.org/Bug/view.php?id=1822,
  //   run the thresholding in single threaded mode to avoid data corruption observed on mac release builds
  //threshold->SetNumberOfThreads(1);

  int segmentationNodeWasModified = segmentationNode->StartModify();
  for (int labelIndex = 0; labelIndex < labelValues->GetNumberOfValues(); ++labelIndex)
    {
    int label = labelValues->GetValue(labelIndex);
    threshold->ThresholdBetween(label, label);
    threshold->Update();

    // Create oriented image data for label
    vtkSmartPointer<vtkOrientedImageData> labelOrientedImageData = vtkSmartPointer<vtkOrientedImageData>::New();
    labelOrientedImageData->vtkImageData::DeepCopy(threshold->GetOutput());
    labelOrientedImageData->SetGeometryFromImageToWorldMatrix(labelmapIjkToRasMatrix);

    vtkSmartPointer<vtkSegment> segment = vtkSmartPointer<vtkSegment>::New();

    // Set segment color
    double color[4] = { vtkSegment::SEGMENT_COLOR_INVALID[0],
                        vtkSegment::SEGMENT_COLOR_INVALID[1],
                        vtkSegment::SEGMENT_COLOR_INVALID[2], 1.0 };
    const char* labelName = NULL;
    if (colorNode)
      {
      labelName = colorNode->GetColorName(label);
      colorNode->GetColor(label, color);
      }
    segment->SetColor(color[0], color[1], color[2]);

    // If there is only one label, then the (only) segment name will be the labelmap name
    if (labelValues->GetNumberOfValues() == 1)
      {
      labelName = labelmapNode->GetName();
      }

    // Set segment name
    if (!labelName)
      {
      std::stringstream ss;
      ss << "Label_" << label;
      labelName = ss.str().c_str();
      }
    segment->SetName(labelName);

    // Apply parent transforms if any
    if (labelmapNode->GetParentTransformNode() || segmentationNode->GetParentTransformNode())
      {
      vtkSmartPointer<vtkGeneralTransform> labelmapToSegmentationTransform = vtkSmartPointer<vtkGeneralTransform>::New();
      vtkSlicerSegmentationsModuleLogic::GetTransformBetweenRepresentationAndSegmentation(labelmapNode, segmentationNode, labelmapToSegmentationTransform);
      vtkOrientedImageDataResample::TransformOrientedImage(labelOrientedImageData, labelmapToSegmentationTransform);
      }

    // Clip to effective extent
    int labelOrientedImageDataEffectiveExtent[6] = { 0, -1, 0, -1, 0, -1 };
    vtkOrientedImageDataResample::CalculateEffectiveExtent(labelOrientedImageData, labelOrientedImageDataEffectiveExtent);
    vtkSmartPointer<vtkImageConstantPad> padder = vtkSmartPointer<vtkImageConstantPad>::New();
    padder->SetInputData(labelOrientedImageData);
    padder->SetOutputWholeExtent(labelOrientedImageDataEffectiveExtent);
    padder->Update();
    labelOrientedImageData->DeepCopy(padder->GetOutput());

    // Add oriented image data as binary labelmap representation
    segment->AddRepresentation(
      vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName(),
      labelOrientedImageData );

    segmentationNode->GetSegmentation()->AddSegment(segment, "", insertBeforeSegmentId);
    } // for each label

  segmentationNode->EndModify(segmentationNodeWasModified);

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(vtkOrientedImageData* labelmapImage,
  vtkMRMLSegmentationNode* segmentationNode, std::string baseSegmentName/*=""*/, std::string insertBeforeSegmentId/*=""*/)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode: Invalid segmentation node");
    return false;
    }
  if (!labelmapImage)
    {
    vtkErrorWithObjectMacro(labelmapImage, "ImportLabelmapToSegmentationNode: Invalid labelmap image");
    return false;
    }

  if (!segmentationNode->GetDisplayNode())
    {
    segmentationNode->CreateDefaultDisplayNodes();
    }

  // Note: Splitting code ported from EditorLib/HelperBox.py:split

  // Split labelmap node into per-label image data

  vtkNew<vtkIntArray> labelValues;
  vtkSlicerSegmentationsModuleLogic::GetAllLabelValues(labelValues.GetPointer(), labelmapImage);

  int segmentationNodeWasModified = segmentationNode->StartModify();

  vtkSmartPointer<vtkImageThreshold> threshold = vtkSmartPointer<vtkImageThreshold>::New();
  threshold->SetInputData(labelmapImage);
  threshold->SetInValue(1);
  threshold->SetOutValue(0);
  threshold->ReplaceInOn();
  threshold->ReplaceOutOn();
  threshold->SetOutputScalarType(VTK_UNSIGNED_CHAR);

  for (int labelIndex = 0; labelIndex < labelValues->GetNumberOfValues(); ++labelIndex)
    {
    int label = labelValues->GetValue(labelIndex);

    threshold->ThresholdBetween(label, label);
    threshold->Update();

    // Create oriented image data for label
    vtkSmartPointer<vtkOrientedImageData> labelOrientedImageData = vtkSmartPointer<vtkOrientedImageData>::New();
    labelOrientedImageData->ShallowCopy(threshold->GetOutput());

    // Clip to effective extent
    int labelOrientedImageDataEffectiveExtent[6] = { 0, -1, 0, -1, 0, -1 };
    vtkOrientedImageDataResample::CalculateEffectiveExtent(labelOrientedImageData, labelOrientedImageDataEffectiveExtent);
    vtkSmartPointer<vtkImageConstantPad> padder = vtkSmartPointer<vtkImageConstantPad>::New();
    padder->SetInputData(labelOrientedImageData);
    padder->SetOutputWholeExtent(labelOrientedImageDataEffectiveExtent);
    padder->Update();

    labelOrientedImageData->ShallowCopy(padder->GetOutput());
    vtkSmartPointer<vtkMatrix4x4> labelmapImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    labelmapImage->GetImageToWorldMatrix(labelmapImageToWorldMatrix);
    labelOrientedImageData->SetGeometryFromImageToWorldMatrix(labelmapImageToWorldMatrix);

    vtkSmartPointer<vtkSegment> segment = vtkSmartPointer<vtkSegment>::New();

    // Set segment name
    std::stringstream ss;
    ss << (baseSegmentName.empty() ? "Label" : baseSegmentName) << "_" << labelIndex+1;
    segment->SetName(ss.str().c_str());

    // Add oriented image data as binary labelmap representation
    segment->AddRepresentation(
      vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName(),
      labelOrientedImageData );

    segmentationNode->GetSegmentation()->AddSegment(segment, "", insertBeforeSegmentId);
    } // for each label

  segmentationNode->EndModify(segmentationNodeWasModified);

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(
  vtkMRMLLabelMapVolumeNode* labelmapNode, vtkMRMLSegmentationNode* segmentationNode, vtkStringArray* updatedSegmentIDs)
{
  if (!segmentationNode && !segmentationNode->GetSegmentation())
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode: Invalid segmentation node");
    return false;
    }
  if (!labelmapNode || !labelmapNode->GetImageData())
    {
    vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode: Invalid labelmap volume node");
    return false;
    }

  // Get oriented image data from labelmap volume node
  vtkOrientedImageData* labelOrientedImageData = vtkOrientedImageData::New();
  labelOrientedImageData->vtkImageData::ShallowCopy(labelmapNode->GetImageData());
  vtkNew<vtkMatrix4x4> ijkToRasMatrix;
  labelmapNode->GetIJKToRASMatrix(ijkToRasMatrix.GetPointer());
  labelOrientedImageData->SetGeometryFromImageToWorldMatrix(ijkToRasMatrix.GetPointer());

  // Apply transforms if segmentation and labelmap are not in the same coordinate system
  vtkSmartPointer<vtkGeneralTransform> labelmapToSegmentationTransform;
  if (labelmapNode->GetParentTransformNode() != segmentationNode->GetParentTransformNode())
    {
    labelmapToSegmentationTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    vtkSlicerSegmentationsModuleLogic::GetTransformBetweenRepresentationAndSegmentation(labelmapNode, segmentationNode, labelmapToSegmentationTransform);
    vtkOrientedImageDataResample::TransformOrientedImage(labelOrientedImageData, labelmapToSegmentationTransform);
    }

  return vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(
    labelOrientedImageData, segmentationNode, updatedSegmentIDs,
    labelmapToSegmentationTransform);
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(
  vtkOrientedImageData* labelmapImage, vtkMRMLSegmentationNode* segmentationNode, vtkStringArray* updatedSegmentIDs,
  vtkGeneralTransform* labelmapToSegmentationTransform /*=NULL*/)
{
  if (!segmentationNode && !segmentationNode->GetSegmentation())
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode: Invalid segmentation node");
    return false;
    }
  if (!labelmapImage)
    {
    vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode: Invalid labelmap volume");
    return false;
    }
  if (!updatedSegmentIDs)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode: Invalid updatedSegmentIDs");
    return false;
    }

  // If master representation is not binary labelmap, then cannot add
  // (this should have been done by the UI classes, notifying the users about hazards of changing the master representation)
  if (segmentationNode->GetSegmentation()->GetMasterRepresentationName() != vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName())
  {
    vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode:"
      "Master representation of the target segmentation node "
      << (segmentationNode->GetName() ? segmentationNode->GetName() : "NULL") << " is not binary labelmap");
    return false;
  }

  if (!segmentationNode->GetDisplayNode())
    {
    segmentationNode->CreateDefaultDisplayNodes();
    }

  // Get labelmap geometry
  vtkSmartPointer<vtkMatrix4x4> labelmapIjkToRasMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  labelmapImage->GetImageToWorldMatrix(labelmapIjkToRasMatrix);

  vtkSmartPointer<vtkImageThreshold> threshold = vtkSmartPointer<vtkImageThreshold>::New();
  threshold->SetInputData(labelmapImage);
  threshold->SetInValue(1);
  threshold->SetOutValue(0);
  threshold->ReplaceInOn();
  threshold->ReplaceOutOn();
  threshold->SetOutputScalarType(labelmapImage->GetScalarType());

  int segmentationNodeWasModified = segmentationNode->StartModify();
  for (int segmentIndex = 0; segmentIndex < updatedSegmentIDs->GetNumberOfValues(); ++segmentIndex)
  {
    std::string segmentId = updatedSegmentIDs->GetValue(segmentIndex);
    if (segmentId.empty())
    {
      continue;
    }
    vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(segmentId);
    if (!segment)
    {
      continue;
    }

    int label = segmentIndex + 1;
    threshold->ThresholdBetween(label, label);
    threshold->Update();

    // Create oriented image data for label
    vtkSmartPointer<vtkOrientedImageData> labelOrientedImageData = vtkSmartPointer<vtkOrientedImageData>::New();
    labelOrientedImageData->ShallowCopy(threshold->GetOutput());
    labelOrientedImageData->SetGeometryFromImageToWorldMatrix(labelmapIjkToRasMatrix);

    // Apply parent transforms if any
    if (labelmapToSegmentationTransform)
      {
      vtkOrientedImageDataResample::TransformOrientedImage(labelOrientedImageData, labelmapToSegmentationTransform);
      }

    // Update segment
    vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment(labelOrientedImageData, segmentationNode, segmentId);

  } // for each label

  segmentationNode->EndModify(segmentationNodeWasModified);

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNodeWithTerminology(vtkMRMLLabelMapVolumeNode* labelmapNode,
  vtkMRMLSegmentationNode* segmentationNode, std::string terminologyContextName, std::string insertBeforeSegmentId/*=""*/)
{
  // Import labelmap to segmentation
  if (! vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(
        labelmapNode, segmentationNode, insertBeforeSegmentId ) )
    {
    vtkErrorMacro("ImportLabelmapToSegmentationNodeWithTerminology: Invalid labelmap volume");
    return false;
    }

  // Assign terminology to segments in the populated segmentation based on the labels of the imported labelmap
  return this->SetTerminologyToSegmentationFromLabelmapNode(segmentationNode, labelmapNode, terminologyContextName);
}

//-----------------------------------------------------------------------------
vtkDataObject* vtkSlicerSegmentationsModuleLogic::CreateRepresentationForOneSegment(
  vtkSegmentation* segmentation, std::string segmentID, std::string representationName )
{
  if (!segmentation)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::CreateRepresentationForOneSegment: Invalid segmentation");
    return NULL;
    }

  // Temporarily duplicate selected segment to only convert them, not the whole segmentation (to save time)
  vtkSmartPointer<vtkSegmentation> segmentationCopy = vtkSmartPointer<vtkSegmentation>::New();
  segmentationCopy->SetMasterRepresentationName(segmentation->GetMasterRepresentationName());
  segmentationCopy->CopyConversionParameters(segmentation);
  segmentationCopy->CopySegmentFromSegmentation(segmentation, segmentID);
  if (!segmentationCopy->CreateRepresentation(representationName, true))
    {
    vtkErrorWithObjectMacro(segmentation, "CreateRepresentationForOneSegment: Failed to convert segment " << segmentID << " to " << representationName);
    return NULL;
    }

  // If conversion succeeded,
  vtkDataObject* segmentTempRepresentation = vtkDataObject::SafeDownCast(
    segmentationCopy->GetSegment(segmentID)->GetRepresentation(representationName) );
  if (!segmentTempRepresentation)
    {
    vtkErrorWithObjectMacro(segmentation, "CreateRepresentationForOneSegment: Failed to get representation "
      << representationName << " from segment " << segmentID);
    return NULL;
    }

  // Copy representation into new data object (the representation will be deleted when segmentation copy gets out of scope)
  vtkDataObject* representationCopy =
    vtkSegmentationConverterFactory::GetInstance()->ConstructRepresentationObjectByClass(segmentTempRepresentation->GetClassName());
  representationCopy->ShallowCopy(segmentTempRepresentation);
  return representationCopy;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ApplyParentTransformToOrientedImageData(
  vtkMRMLTransformableNode* transformableNode, vtkOrientedImageData* orientedImageData, bool linearInterpolation/*=false*/, double backgroundColor[4]/*=NULL*/ )
{
  if (!transformableNode || !orientedImageData)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ApplyParentTransformToOrientedImageData: Invalid inputs");
    return false;
    }

  // Get world to reference RAS transform
  vtkSmartPointer<vtkGeneralTransform> nodeToWorldTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  vtkMRMLTransformNode* parentTransformNode = transformableNode->GetParentTransformNode();
  if (!parentTransformNode)
    {
    // There is no parent transform for segmentation, nothing to apply
    return true;
    }

  // Transform oriented image data
  parentTransformNode->GetTransformToWorld(nodeToWorldTransform);
  vtkOrientedImageDataResample::TransformOrientedImage(orientedImageData, nodeToWorldTransform, false, false, linearInterpolation, backgroundColor);

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ApplyParentTransformToPolyData(vtkMRMLTransformableNode* transformableNode, vtkPolyData* polyData)
{
  if (!transformableNode || !polyData)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ApplyParentTransformToPolyData: Invalid inputs");
    return false;
    }

  // Get world to reference RAS transform
  vtkSmartPointer<vtkGeneralTransform> nodeToWorldTransform = vtkSmartPointer<vtkGeneralTransform>::New();
  vtkMRMLTransformNode* parentTransformNode = transformableNode->GetParentTransformNode();
  if (!parentTransformNode)
    {
    // There is no parent transform for segmentation, nothing to apply
    return true;
    }

  // Transform oriented image data
  parentTransformNode->GetTransformToWorld(nodeToWorldTransform);

  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformFilter->SetInputData(polyData);
  transformFilter->SetTransform(nodeToWorldTransform);
  transformFilter->Update();
  polyData->DeepCopy(transformFilter->GetOutput());

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::GetTransformBetweenRepresentationAndSegmentation(
  vtkMRMLTransformableNode* representationNode, vtkMRMLSegmentationNode* segmentationNode, vtkGeneralTransform* representationToSegmentationTransform )
{
  if (!representationNode || !segmentationNode || !representationToSegmentationTransform)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::GetTransformBetweenRepresentationAndSegmentation: Invalid inputs");
    return false;
    }
  vtkMRMLTransformNode::GetTransformBetweenNodes(
    representationNode->GetParentTransformNode(), segmentationNode->GetParentTransformNode(), representationToSegmentationTransform);

  return true;
}


//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation(
  vtkMRMLSegmentationNode* segmentationNode, std::string segmentID, std::string representationName,
  vtkDataObject* segmentRepresentation, bool applyParentTransform/*=true*/ )
{
  if (!segmentationNode || segmentID.empty() || representationName.empty() || !segmentRepresentation)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation: Invalid inputs");
    return false;
    }

  // Get requested segment
  vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(segmentID);
  if (!segment)
    {
    vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation: Unable to find segment with ID "
      << segmentID << " in segmentation " << segmentationNode->GetName());
    return false;
    }

  if (segmentationNode->GetSegmentation()->ContainsRepresentation(representationName))
    {
    // Get and copy representation into output data object
    vtkDataObject* representationObject = segment->GetRepresentation(representationName);
    if (!representationObject)
      {
      vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation: Unable to get '" << representationName
        << "' representation from segment with ID " << segmentID << " in segmentation " << segmentationNode->GetName());
      return false;
      }
    segmentRepresentation->DeepCopy(representationObject);
    }
  else // Need to convert
    {
    // Temporarily duplicate selected segment to only convert them, not the whole segmentation (to save time)
    vtkSmartPointer<vtkDataObject> representationObject = vtkSmartPointer<vtkDataObject>::Take(
      vtkSlicerSegmentationsModuleLogic::CreateRepresentationForOneSegment(segmentationNode->GetSegmentation(), segmentID, representationName) );
    if (!representationObject.GetPointer())
      {
      vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation: Unable to convert segment with ID "
        << segmentID << " to '" << representationName << "' representation in segmentation " << segmentationNode->GetName());
      return false;
      }
    segmentRepresentation->DeepCopy(representationObject);
    }

  // Apply parent transformation nodes if necessary
  if (applyParentTransform && segmentationNode->GetParentTransformNode())
    {
    vtkOrientedImageData* segmentRepresentationOrientedImageData = vtkOrientedImageData::SafeDownCast(segmentRepresentation);
    vtkPolyData* segmentRepresentationPolyData = vtkPolyData::SafeDownCast(segmentRepresentation);
    if (segmentRepresentationOrientedImageData)
      {
      if (!vtkSlicerSegmentationsModuleLogic::ApplyParentTransformToOrientedImageData(segmentationNode, segmentRepresentationOrientedImageData))
        {
        vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation: Failed to apply parent transform of "
          << "segmentation " << segmentationNode->GetName() << " on representation oriented image data");
        return false;
        }
      }
    else if (segmentRepresentationPolyData)
      {
        if (!vtkSlicerSegmentationsModuleLogic::ApplyParentTransformToPolyData(segmentationNode, segmentRepresentationPolyData))
        {
        vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation: Failed to apply parent transform of "
          << "segmentation " << segmentationNode->GetName() << " on representation poly data");
        return false;
        }
      }
    else
      {
      vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation: Failed to apply parent transform of "
        << "segmentation " << segmentationNode->GetName() << " due to unsupported representation with class name " << segmentRepresentation->GetClassName());
      return false;
      }
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::GetSegmentBinaryLabelmapRepresentation(
  vtkMRMLSegmentationNode* segmentationNode, std::string segmentID, vtkOrientedImageData* imageData, bool applyParentTransform/*=true*/ )
{
  if (!segmentationNode || segmentID.empty() || !imageData)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::GetSegmentBinaryLabelmapRepresentation: Invalid inputs");
    return false;
    }

  return vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation(segmentationNode, segmentID,
    vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName(), imageData, applyParentTransform );
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::GetSegmentClosedSurfaceRepresentation(vtkMRMLSegmentationNode* segmentationNode,
  std::string segmentID, vtkPolyData* polyData, bool applyParentTransform/*=true*/)
{
  if (!segmentationNode || segmentID.empty() || !polyData)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::GetSegmentBinaryLabelmapRepresentation: Invalid inputs");
    return false;
    }

  return vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation(segmentationNode, segmentID,
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(), polyData, applyParentTransform);
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment(
  vtkOrientedImageData* labelmap, vtkMRMLSegmentationNode* segmentationNode, std::string segmentID, int mergeMode/*=MODE_REPLACE*/, const int extent[6]/*=0*/ )
{
  if (!segmentationNode || segmentID.empty() || !labelmap)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment: Invalid inputs");
    return false;
    }
  if (labelmap->GetPointData()->GetScalars() == NULL)
    {
    vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment: Invalid input labelmap");
    return false;
    }

  // Get binary labelmap representation of selected segment
  vtkSegment* selectedSegment = segmentationNode->GetSegmentation()->GetSegment(segmentID);
  if (!selectedSegment)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment: Invalid selected segment");
    return false;
    }
  vtkOrientedImageData* segmentLabelmap = vtkOrientedImageData::SafeDownCast(
    selectedSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()) );
  if (!segmentLabelmap)
    {
    vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment: Failed to get binary labelmap representation in "
      << "segmentation " << segmentationNode->GetName());
    return false;
    }

  // 1. Append input labelmap to the segment labelmap if requested
  vtkSmartPointer<vtkOrientedImageData> newSegmentLabelmap = vtkSmartPointer<vtkOrientedImageData>::New();
  bool segmentLabelmapModified = true;

  int* segmentLabelmapExtent = segmentLabelmap->GetExtent();
  bool segmentLabelmapEmpty = (segmentLabelmapExtent[0] > segmentLabelmapExtent[1] ||
    segmentLabelmapExtent[2] > segmentLabelmapExtent[3] ||
    segmentLabelmapExtent[4] > segmentLabelmapExtent[5]);
  if (segmentLabelmapEmpty)
    {
    if (mergeMode == MODE_MERGE_MIN)
      {
      // empty image is assumed to have minimum value everywhere, combining it with MAX operation
      // results an empty image, so we don't need to do anything.
      return true;
      }
    // Replace the empty image with the modifier image
    mergeMode = MODE_REPLACE;
    }

  if (mergeMode == MODE_REPLACE)
    {
    if (!vtkOrientedImageDataResample::CopyImage(labelmap, newSegmentLabelmap, extent))
      {
      vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment: Failed to copy labelmap");
      return false;
      }
    }
  else
    {
    int operation = (mergeMode==MODE_MERGE_MAX ? vtkOrientedImageDataResample::OPERATION_MAXIMUM : vtkOrientedImageDataResample::OPERATION_MINIMUM);

    if (!vtkOrientedImageDataResample::DoGeometriesMatch(segmentLabelmap, labelmap))
      {
      // Make sure appended image has the same lattice as the input image
      vtkSmartPointer<vtkOrientedImageData> resampledSegmentLabelmap = vtkSmartPointer<vtkOrientedImageData>::New();
      vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(
        segmentLabelmap, labelmap, resampledSegmentLabelmap, false /*interpolate*/, true /*pad*/);
      if (!vtkOrientedImageDataResample::MergeImage(resampledSegmentLabelmap, labelmap, newSegmentLabelmap, operation, extent, 0, 1, &segmentLabelmapModified))
        {
        vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment: Failed to merge labelmap (max)");
        return false;
        }
      }
    else
      {
        if (!vtkOrientedImageDataResample::MergeImage(segmentLabelmap, labelmap, newSegmentLabelmap, operation, extent, 0, 1, &segmentLabelmapModified))
        {
        vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment: Failed to merge labelmap (max)");
        return false;
        }
      }
    }

  if (!segmentLabelmapModified)
    {
    // segment labelmap not modified, there is no need to update representations
    return true;
    }

  // 2. Copy the temporary padded modifier labelmap to the segment.
  //    Disable modified event so that the consequently emitted MasterRepresentationModified event that causes
  //    removal of all other representations in all segments does not get activated. Instead, explicitly create
  //    representations for the edited segment that the other segments have.
  bool wasMasterRepresentationModifiedEnabled = segmentationNode->GetSegmentation()->SetMasterRepresentationModifiedEnabled(false);
  segmentLabelmap->ShallowCopy(newSegmentLabelmap);

  // 3. Shrink the image data extent to only contain the effective data (extent of non-zero voxels)
  int effectiveExtent[6] = {0,-1,0,-1,0,-1};
  vtkOrientedImageDataResample::CalculateEffectiveExtent(segmentLabelmap, effectiveExtent); // TODO: use the update extent? maybe crop when changing segment?
  if (effectiveExtent[0] > effectiveExtent[1] || effectiveExtent[2] > effectiveExtent[3] || effectiveExtent[4] > effectiveExtent[5])
    {
    vtkDebugWithObjectMacro(segmentationNode,
      "vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment: effective extent of the labelmap to set is invalid (labelmap is empty)");
    }
  else
    {
    vtkSmartPointer<vtkImageConstantPad> padder = vtkSmartPointer<vtkImageConstantPad>::New();
    padder->SetInputData(segmentLabelmap);
    padder->SetOutputWholeExtent(effectiveExtent);
    padder->Update();
    segmentLabelmap->DeepCopy(padder->GetOutput());
    }
  // 4. Re-convert all other representations
  std::vector<std::string> representationNames;
  selectedSegment->GetContainedRepresentationNames(representationNames);
  bool conversionHappened = false;
  for (std::vector<std::string>::iterator reprIt = representationNames.begin();
    reprIt != representationNames.end(); ++reprIt)
    {
    std::string targetRepresentationName = (*reprIt);
    if (targetRepresentationName.compare(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()))
      {
      conversionHappened |= segmentationNode->GetSegmentation()->ConvertSingleSegment(
        segmentID, targetRepresentationName );
      }
    }

  // Re-enable master representation modified event
  segmentationNode->GetSegmentation()->SetMasterRepresentationModifiedEnabled(wasMasterRepresentationModifiedEnabled);
  const char* segmentIdChar = segmentID.c_str();
  segmentationNode->GetSegmentation()->InvokeEvent(vtkSegmentation::MasterRepresentationModified, (void*)segmentIdChar);
  segmentationNode->GetSegmentation()->InvokeEvent(vtkSegmentation::RepresentationModified, (void*)segmentIdChar);

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::SetTerminologyToSegmentationFromLabelmapNode(vtkMRMLSegmentationNode* segmentationNode,
  vtkMRMLLabelMapVolumeNode* labelmapNode, std::string terminologyContextName)
{
  if (terminologyContextName.empty())
    {
    return true;
    }
  if (!this->TerminologiesLogic)
    {
    vtkErrorMacro("SetTerminologyToSegmentationFromLabelmapNode: Terminology logic cannot be accessed");
    return false;
    }
  if (!segmentationNode)
    {
    vtkErrorMacro("SetTerminologyToSegmentationFromLabelmapNode: Invalid segmentation node");
    return false;
    }
  if (!labelmapNode || !labelmapNode->GetImageData())
    {
    vtkErrorMacro("SetTerminologyToSegmentationFromLabelmapNode: Invalid labelmap volume node");
    return false;
    }

  // Get color node
  if (!labelmapNode->GetDisplayNode())
    {
    vtkErrorMacro("SetTerminologyToSegmentationFromLabelmapNode: Segmentation node " << segmentationNode->GetName() << " has no display node");
    return false;
    }
  vtkMRMLColorTableNode* colorNode = vtkMRMLColorTableNode::SafeDownCast(labelmapNode->GetDisplayNode()->GetColorNode());
  if (!colorNode)
    {
    vtkErrorMacro("SetTerminologyToSegmentationFromLabelmapNode: Segmentation node " << segmentationNode->GetName() << " has no associated color table node");
    return false;
    }

  // Get first terminology entry. This is set to segments that cannot be matched to labels, and when
  // label names are not found in 3dSlicerLabel attributes in terminology types within the context.
  std::vector<vtkSlicerTerminologiesModuleLogic::CodeIdentifier> categories;
  this->TerminologiesLogic->GetCategoriesInTerminology(terminologyContextName, categories);
  if (categories.empty())
    {
    vtkErrorMacro("SetTerminologyToSegmentationFromLabelmapNode: Terminology context " << terminologyContextName << " is empty");
    return false;
    }
  std::vector<vtkSlicerTerminologiesModuleLogic::CodeIdentifier> typesInFirstCategory;
  int firstNonEmptyCategoryIndex = -1;
  do
    {
    this->TerminologiesLogic->GetTypesInTerminologyCategory(terminologyContextName, categories[++firstNonEmptyCategoryIndex], typesInFirstCategory);
    }
  while (typesInFirstCategory.empty() && firstNonEmptyCategoryIndex < static_cast<int>(categories.size()));
  if (typesInFirstCategory.empty())
    {
    vtkErrorMacro("SetTerminologyToSegmentationFromLabelmapNode: All categories in terminology context " << terminologyContextName << " are empty");
    return false;
    }

  vtkSmartPointer<vtkSlicerTerminologyEntry> firstTerminologyEntry = vtkSmartPointer<vtkSlicerTerminologyEntry>::New();
  firstTerminologyEntry->SetTerminologyContextName(terminologyContextName.c_str());
  vtkSmartPointer<vtkSlicerTerminologyCategory> firstCategory = vtkSmartPointer<vtkSlicerTerminologyCategory>::New();
  this->TerminologiesLogic->GetCategoryInTerminology(
    terminologyContextName, categories[firstNonEmptyCategoryIndex], firstCategory );
  firstTerminologyEntry->GetCategoryObject()->Copy(firstCategory);
  vtkSmartPointer<vtkSlicerTerminologyType> firstType = vtkSmartPointer<vtkSlicerTerminologyType>::New();
  this->TerminologiesLogic->GetTypeInTerminologyCategory(
    terminologyContextName, categories[firstNonEmptyCategoryIndex], typesInFirstCategory[0], firstType );
  firstTerminologyEntry->GetTypeObject()->Copy(firstType);
  std::string firstTerminologyString = this->TerminologiesLogic->SerializeTerminologyEntry(firstTerminologyEntry);

  // Assign terminology entry to each segment in the segmentation
  std::vector<std::string> segmentIDs;
  segmentationNode->GetSegmentation()->GetSegmentIDs(segmentIDs);
  vtkSmartPointer<vtkSlicerTerminologyEntry> foundTerminologyEntry = vtkSmartPointer<vtkSlicerTerminologyEntry>::New();
  for (std::vector<std::string>::iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
    {
    vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(*segmentIdIt);

    // Check if label for the current segment exists in labelmap. If there were segments in the segmentation when
    // importing, then the label and so terminology will not be found. In this case terminology tag is left as is
    // (which may be the default GeneralAnatomy/Tissue/Tissue, or the one the user manually specified)
    int label = colorNode->GetColorIndexByName(segment->GetName());
    if (label == -1)
      {
      continue;
      }

    // Search for the 3dSlicerLabel attribute in the specified terminology context
    if (this->TerminologiesLogic->FindTypeInTerminologyBy3dSlicerLabel(terminologyContextName, segment->GetName(), foundTerminologyEntry))
      {
      std::string foundTerminologyString = this->TerminologiesLogic->SerializeTerminologyEntry(foundTerminologyEntry);
      segment->SetTag(vtkSegment::GetTerminologyEntryTagName(), foundTerminologyString);
      }
    else
      {
      // Set first entry if 3dSlicerLabel is not found
      segment->SetTag(vtkSegment::GetTerminologyEntryTagName(), firstTerminologyString);
      }
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ExportSegmentsClosedSurfaceRepresentationToFiles(std::string destinationFolder,
  vtkMRMLSegmentationNode* segmentationNode, vtkStringArray* segmentIds /*=NULL*/,
  std::string fileFormat /*="STL"*/, bool lps /*=true*/, double sizeScale /*=1.0*/, bool merge /*=false*/)
{
  if (!segmentationNode || !segmentationNode->GetSegmentation())
    {
    vtkGenericWarningMacro("ExportSegmentsClosedSurfaceRepresentationToFiles failed: invalid segmentationNode");
    return false;
    }

  std::vector<std::string> segmentIdsVector;
  if (segmentIds == NULL)
    {
    segmentationNode->GetSegmentation()->GetSegmentIDs(segmentIdsVector);
    }
  else
    {
    for (int segmentIndex = 0; segmentIndex < segmentIds->GetNumberOfValues(); ++segmentIndex)
      {
      segmentIdsVector.push_back(segmentIds->GetValue(segmentIndex));
      }
    }

  std::string extension = vtksys::SystemTools::LowerCase(fileFormat);
  if (extension == "obj")
    {
    return ExportSegmentsClosedSurfaceRepresentationToObjFile(destinationFolder, segmentationNode, segmentIdsVector, lps, sizeScale);
    }
  if (extension != "stl")
    {
    vtkGenericWarningMacro("ExportSegmentsClosedSurfaceRepresentationToFiles: fileFormat "
      << fileFormat << " is unknown. Using STL.");
    }
  return ExportSegmentsClosedSurfaceRepresentationToStlFiles(destinationFolder, segmentationNode, segmentIdsVector, lps, sizeScale, merge);
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ExportSegmentsClosedSurfaceRepresentationToStlFiles(std::string destinationFolder,
  vtkMRMLSegmentationNode* segmentationNode, std::vector<std::string>& segmentIDs, bool lps, double sizeScale, bool merge)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("ExportSegmentsClosedSurfaceRepresentationToFiles failed: invalid segmentationNode");
    return false;
    }

  // We explicitly write the coordinate system into the file header.
  // See vtkMRMLModelStorageNode::WriteDataInternal.
  const std::string coordinateSystemValue = (lps ? "LPS" : "RAS");
  const std::string coordinateSytemSpecification = "SPACE=" + coordinateSystemValue;

  vtkNew<vtkTriangleFilter> triangulator;
  vtkNew<vtkSTLWriter> writer;
  writer->SetFileType(VTK_BINARY);
  writer->SetInputConnection(triangulator->GetOutputPort());
  std::string header = std::string("3D Slicer output. ") + coordinateSytemSpecification;
  if (sizeScale != 1.0)
    {
    std::ostringstream strs;
    strs << sizeScale;
    header += ";SCALE=" + strs.str();
    }
  writer->SetHeader(header.c_str());

  if (merge)
    {
    vtkNew<vtkAppendPolyData> appendPolyData;

    for (std::vector<std::string>::iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
      {
      vtkNew<vtkPolyData> segmentPolyData;
      bool polyDataAvailable = vtkSlicerSegmentationsModuleLogic::GetSegmentClosedSurfaceRepresentation(
        segmentationNode, *segmentIdIt, segmentPolyData.GetPointer());
      if (!polyDataAvailable || segmentPolyData.GetPointer() == NULL)
        {
        vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsClosedSurfaceRepresentationToFiles: Unable to convert segment "
          << (*segmentIdIt) << " to closed surface representation");
        continue;
        }
      appendPolyData->AddInputData(segmentPolyData.GetPointer());
      }
    vtkNew<vtkTransform> transformRasToLps;
    if (sizeScale != 1.0)
      {
      transformRasToLps->Scale(sizeScale, sizeScale, sizeScale);
      }
    if (lps)
      {
      transformRasToLps->Scale(-1, -1, 1);
      }
    vtkNew<vtkTransformPolyDataFilter> transformPolyDataToOutput;
    transformPolyDataToOutput->SetTransform(transformRasToLps.GetPointer());
    transformPolyDataToOutput->SetInputConnection(appendPolyData->GetOutputPort());
    std::string filePath = destinationFolder + "/" + segmentationNode->GetName() + ".stl";
    triangulator->SetInputConnection(transformPolyDataToOutput->GetOutputPort());
    writer->SetFileName(filePath.c_str());
    try
      {
      writer->Write();
      }
    catch (...)
      {
      vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsClosedSurfaceRepresentationToFiles:"
        " Unable to write segmentation to " << filePath);
      return false;
      }
    }
  else
    {
    for (std::vector<std::string>::iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
      {
      vtkNew<vtkPolyData> segmentPolyData;
      bool polyDataAvailable = vtkSlicerSegmentationsModuleLogic::GetSegmentClosedSurfaceRepresentation(
        segmentationNode, *segmentIdIt, segmentPolyData.GetPointer());
      if (!polyDataAvailable || segmentPolyData.GetPointer() == NULL)
        {
        vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsClosedSurfaceRepresentationToFiles: Unable to convert segment "
          << (*segmentIdIt) << " to closed surface representation");
        continue;
        }
      vtkNew<vtkTransform> transformRasToLps;
      if (sizeScale != 1.0)
        {
        transformRasToLps->Scale(sizeScale, sizeScale, sizeScale);
        }
      if (lps)
        {
        transformRasToLps->Scale(-1, -1, 1);
        }
      vtkNew<vtkTransformPolyDataFilter> transformPolyDataToOutput;
      transformPolyDataToOutput->SetTransform(transformRasToLps.GetPointer());
      transformPolyDataToOutput->SetInputData(segmentPolyData.GetPointer());
      std::string segmentName = segmentationNode->GetSegmentation()->GetSegment(*segmentIdIt)->GetName();
      std::string filePath = destinationFolder + "/" + segmentationNode->GetName() + "_" + segmentName + ".stl";
      triangulator->SetInputConnection(transformPolyDataToOutput->GetOutputPort());
      writer->SetFileName(filePath.c_str());
      try
        {
        writer->Write();
        }
      catch (...)
        {
        vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsClosedSurfaceRepresentationToFiles:"
          " Unable to write segmentation to " << filePath);
        return false;
        }
      }
    }
  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ExportSegmentsClosedSurfaceRepresentationToObjFile(std::string destinationFolder,
  vtkMRMLSegmentationNode* segmentationNode, std::vector<std::string>& segmentIDs, bool lps, double sizeScale)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("ExportSegmentsClosedSurfaceRepresentationToFiles failed: invalid segmentationNode");
    return false;
    }

  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());

  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer.GetPointer());

  for (std::vector<std::string>::iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
    {
    vtkNew<vtkPolyData> segmentPolyData;
    bool polyDataAvailable = vtkSlicerSegmentationsModuleLogic::GetSegmentClosedSurfaceRepresentation(
      segmentationNode, *segmentIdIt, segmentPolyData.GetPointer());
    if (!polyDataAvailable || segmentPolyData.GetPointer() == NULL)
      {
      vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsClosedSurfaceRepresentationToObjFile: Unable to convert segment "
        << (*segmentIdIt) << " to closed surface representation");
      continue;
      }
    vtkNew<vtkTransform> transformRasToLps;
    if (sizeScale != 1.0)
      {
      transformRasToLps->Scale(sizeScale, sizeScale, sizeScale);
      }
    if (lps)
      {
      transformRasToLps->Scale(-1, -1, 1);
      }
    vtkNew<vtkTransformPolyDataFilter> transformPolyDataToOutput;
    transformPolyDataToOutput->SetTransform(transformRasToLps.GetPointer());
    transformPolyDataToOutput->SetInputData(segmentPolyData.GetPointer());
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(transformPolyDataToOutput->GetOutputPort());
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper.GetPointer());

    if (displayNode)
      {
      double color[3] = { 0.5, 0.5, 0.5 };
      displayNode->GetSegmentColor(*segmentIdIt, color);
      // OBJ exporter sets the same color for ambient, diffuse, specular
      // so we scale it by 1/3 to avoid having too bright material.
      double colorScale = 1.0 / 3.0;
      actor->GetProperty()->SetColor(color[0] * colorScale, color[1] * colorScale, color[2] * colorScale);
      actor->GetProperty()->SetSpecularPower(3.0);
      actor->GetProperty()->SetOpacity(displayNode->GetSegmentOpacity3D(*segmentIdIt));
      }
    renderer->AddActor(actor.GetPointer());
    }

  vtkNew<vtkOBJExporter> exporter;
  exporter->SetRenderWindow(renderWindow.GetPointer());
  std::string fullNameWithoutExtension = destinationFolder + "/" + segmentationNode->GetName();
  exporter->SetFilePrefix(fullNameWithoutExtension.c_str());

#if VTK_MAJOR_VERSION >= 9
  // We explicitly write the coordinate system into the file header.
  // See vtkMRMLModelStorageNode::WriteDataInternal.
  const std::string coordinateSystemValue = (lps ? "LPS" : "RAS");
  const std::string coordinateSytemSpecification = "SPACE=" + coordinateSystemValue;
  std::string header = std::string("3D Slicer output. ") + coordinateSytemSpecification;
  if (sizeScale != 1.0)
    {
    std::ostringstream strs;
    strs << sizeScale;
    header += ";SCALE=" + strs.str();
    }
  exporter->SetOBJFileComment(header.c_str());
#endif

  try
    {
    exporter->Write();
    }
  catch (...)
    {
    vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsClosedSurfaceRepresentationToObjFile:"
      " Unable to write segmentation to " << fullNameWithoutExtension << ".obj");
    return false;
    }

  return true;
}
