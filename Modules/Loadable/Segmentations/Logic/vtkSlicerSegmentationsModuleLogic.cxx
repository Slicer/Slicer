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

#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkMRMLSegmentationStorageNode.h"
#include "vtkMRMLSegmentEditorNode.h"

// SegmentationCore includes
#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkSegmentationConverterFactory.h"
#include "vtkBinaryLabelmapToClosedSurfaceConversionRule.h"
#include "vtkClosedSurfaceToBinaryLabelmapConversionRule.h"

// Subject Hierarchy includes
#include <vtkMRMLSubjectHierarchyNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkTrivialProducer.h>
#include <vtkMatrix4x4.h>
#include <vtkCallbackCommand.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkImageAccumulate.h>
#include <vtkImageThreshold.h>
#include <vtkDataObject.h>
#include <vtkTransform.h>
#include <vtksys/SystemTools.hxx>
#include <vtkGeneralTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkImageMathematics.h>
#include <vtkImageConstantPad.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkEventBroker.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLTransformNode.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerSegmentationsModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerSegmentationsModuleLogic::vtkSlicerSegmentationsModuleLogic()
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
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentationsModuleLogic::RegisterNodes()
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("RegisterNodes: Invalid MRML scene!");
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
}

//---------------------------------------------------------------------------
void vtkSlicerSegmentationsModuleLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
    {
    vtkErrorMacro("OnMRMLSceneNodeAdded: Invalid MRML scene or input node!");
    return;
    }

  if (node->IsA("vtkMRMLSubjectHierarchyNode"))
    {
    vtkEventBroker::GetInstance()->AddObservation(
      node, vtkMRMLSubjectHierarchyNode::SubjectHierarchyUIDAddedEvent, this, this->SubjectHierarchyUIDCallbackCommand );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSegmentationsModuleLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
    {
    vtkErrorMacro("OnMRMLSceneNodeRemoved: Invalid MRML scene or input node!");
    return;
    }

  if (node->IsA("vtkMRMLSegmentationNode"))
    {
    vtkEventBroker::GetInstance()->RemoveObservations(
      node, vtkMRMLSubjectHierarchyNode::SubjectHierarchyUIDAddedEvent, this, this->SubjectHierarchyUIDCallbackCommand );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSegmentationsModuleLogic::OnSubjectHierarchyUIDAdded(vtkObject* caller,
                                                                   unsigned long vtkNotUsed(eid),
                                                                   void* clientData,
                                                                   void* vtkNotUsed(callData))
{
  vtkSlicerSegmentationsModuleLogic* self = reinterpret_cast<vtkSlicerSegmentationsModuleLogic*>(clientData);
  if (!self)
    {
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNodeWithNewUID = reinterpret_cast<vtkMRMLSubjectHierarchyNode*>(caller);
  if (!shNodeWithNewUID)
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
      node->OnSubjectHierarchyUIDAdded(shNodeWithNewUID);
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
    vtkSegmentation::SegmentMap segmentMap = node->GetSegmentation()->GetSegments();
    for (vtkSegmentation::SegmentMap::iterator segmentIt = segmentMap.begin(); segmentIt != segmentMap.end(); ++segmentIt)
      {
      if (segmentIt->second.GetPointer() == segment)
        {
        segmentId = segmentIt->first;
        return node;
        }
      }
    }

  return NULL;
}

//-----------------------------------------------------------------------------
vtkMRMLSegmentationNode* vtkSlicerSegmentationsModuleLogic::LoadSegmentationFromFile(const char* filename)
{
  if (this->GetMRMLScene() == NULL || filename == NULL)
    {
    return NULL;
    }
  vtkSmartPointer<vtkMRMLSegmentationNode> segmentationNode = vtkSmartPointer<vtkMRMLSegmentationNode>::New();
  vtkSmartPointer<vtkMRMLSegmentationStorageNode> storageNode = vtkSmartPointer<vtkMRMLSegmentationStorageNode>::New();
  storageNode->SetFileName(filename);

  // Check to see which node can read this type of file
  if (!storageNode->SupportedFileType(filename))
    {
    vtkErrorMacro("LoadSegmentationFromFile: Segmentation storage node unable to load segmentation file.");
    return NULL;
    }

  std::string baseName = vtksys::SystemTools::GetFilenameWithoutExtension(filename);
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
    vtkErrorMacro("LoadSegmentationFromFile: Error reading " << filename);
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
      displayNode->CalculateAutoOpacitiesForSegments();
      }
    }

  return segmentationNode.GetPointer();
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::CreateLabelmapVolumeFromOrientedImageData(vtkOrientedImageData* orientedImageData, vtkMRMLLabelMapVolumeNode* labelmapVolumeNode)
{
  if (!orientedImageData)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::CreateLabelmapVolumeFromOrientedImageData: Invalid input image data");
    return false;
    }
  if (!labelmapVolumeNode)
    {
    vtkErrorWithObjectMacro(orientedImageData, "CreateLabelmapVolumeFromOrientedImageData: Invalid labelmap volume node");
    return false;
    }

  // Create an identity (zero origin, unit spacing, identity orientation) vtkImageData that can be stored in vtkMRMLVolumeNode
  vtkSmartPointer<vtkImageData> identityImageData = vtkSmartPointer<vtkImageData>::New();
  identityImageData->ShallowCopy(orientedImageData);
  identityImageData->SetOrigin(0,0,0);
  identityImageData->SetSpacing(1,1,1);
  labelmapVolumeNode->SetAndObserveImageData(identityImageData);

  vtkSmartPointer<vtkMatrix4x4> labelmapImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  orientedImageData->GetImageToWorldMatrix(labelmapImageToWorldMatrix);
  labelmapVolumeNode->SetIJKToRASMatrix(labelmapImageToWorldMatrix);

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
      }
    labelmapVolumeDisplayNode->SetDefaultColorMap();
    }

  // Make sure merged labelmap extents starts at zeros for compatibility reasons
  labelmapVolumeNode->ShiftImageDataExtentToZeroStart();

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

  //TODO: This method leaks from python. The function SlicerRtCommon::ConvertVolumeNodeToVtkOrientedImageData can be used istead.
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
    vtkErrorWithObjectMacro(labelmapVolumeNode, "CreateSegmentFromLabelmapVolumeNode: Unable to create single segment from labelmap volume node, as labelmap contains more than one label!");
    return NULL;
    }

  // Create segment
  vtkSegment* segment = vtkSegment::New();
  segment->SetName(labelmapVolumeNode->GetName());

  // Set segment color
  double color[4] = { vtkSegment::SEGMENT_COLOR_VALUE_INVALID[0],
                      vtkSegment::SEGMENT_COLOR_VALUE_INVALID[1],
                      vtkSegment::SEGMENT_COLOR_VALUE_INVALID[2], 1.0 };
  vtkMRMLColorTableNode* colorNode = NULL;
  if (labelmapVolumeNode->GetDisplayNode())
    {
    colorNode = vtkMRMLColorTableNode::SafeDownCast(labelmapVolumeNode->GetDisplayNode()->GetColorNode());
    if (colorNode)
      {
      colorNode->GetColor(label, color);
      }
    }
  segment->SetDefaultColor(color[0], color[1], color[2]);

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
    vtkErrorWithObjectMacro(modelNode, "CreateSegmentFromModelNode: Model node does not contain poly data!");
    return NULL;
    }

  double color[3] = { vtkSegment::SEGMENT_COLOR_VALUE_INVALID[0],
                      vtkSegment::SEGMENT_COLOR_VALUE_INVALID[1],
                      vtkSegment::SEGMENT_COLOR_VALUE_INVALID[2] };

  // Create oriented image data from labelmap volume node
  vtkSegment* segment = vtkSegment::New();
    segment->SetName(modelNode->GetName());

  // Color from display node
  vtkMRMLDisplayNode* modelDisplayNode = modelNode->GetDisplayNode();
  if (modelDisplayNode)
    {
    modelDisplayNode->GetColor(color);
    segment->SetDefaultColor(color);
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
vtkMRMLSegmentationNode* vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* segmentShNode)
{
  if (!segmentShNode)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyNode: input subject hierarchy node is invalid");
    return NULL;
    }

  vtkMRMLSubjectHierarchyNode* parentShNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(
    segmentShNode->GetParentNode() );
  if (!parentShNode)
    {
    vtkWarningWithObjectMacro(segmentShNode, "vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyNode: Segment subject hierarchy node has no segmentation parent!");
    return NULL;
    }
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(parentShNode->GetAssociatedNode());
  if (!segmentationNode)
    {
    vtkWarningWithObjectMacro(segmentShNode, "vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyNode: Segment subject hierarchy node's parent has no associated segmentation node!");
    return NULL;
    }

  return segmentationNode;
}

//-----------------------------------------------------------------------------
vtkSegment* vtkSlicerSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* segmentShNode)
{
  vtkMRMLSegmentationNode* segmentationNode =
    vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyNode(segmentShNode);
  if (!segmentShNode || !segmentationNode)
    {
    return NULL;
    }

  const char* segmentId = segmentShNode->GetAttribute(vtkMRMLSegmentationNode::GetSegmentIDAttributeName());
  if (!segmentId)
    {
    vtkWarningWithObjectMacro(segmentShNode, "vtkSlicerSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyNode: Segment subject hierarchy node "
      << (segmentShNode->GetID() ? segmentShNode->GetID() : "(unknown)") << " does not contain segment ID");
    return NULL;
    }

  vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(segmentId);
  if (!segment)
    {
    vtkErrorWithObjectMacro(segmentShNode, "vtkSlicerSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyNode: "
      "Segmentation does not contain segment with given ID: " << (segmentId ? segmentId : "(empty)"));
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
    vtkErrorWithObjectMacro(segment, "ExportSegmentToRepresentationNode: Invalid representation MRML node!");
    return false;
    }
  vtkMRMLLabelMapVolumeNode* labelmapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(representationNode);
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(representationNode);
  if (!labelmapNode && !modelNode)
    {
    vtkErrorWithObjectMacro(representationNode, "ExportSegmentToRepresentationNode: Representation MRML node should be either labelmap volume node or model node!");
    return false;
    }

  // Determine segment ID and set it as name of the representation node if found
  std::string segmentId("");
  vtkMRMLSegmentationNode* segmentationNode = vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegment(
    representationNode->GetScene(), segment, segmentId);
  if (segmentationNode)
    {
    representationNode->SetName(segmentId.c_str());
    }
  vtkMRMLTransformNode* parentTransformNode = segmentationNode->GetParentTransformNode();

  if (labelmapNode)
    {
    // Make sure binary labelmap representation exists in segment
    bool binaryLabelmapPresent = segment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
    if (!binaryLabelmapPresent && !segmentationNode)
      {
      vtkErrorWithObjectMacro(representationNode, "ExportSegmentToRepresentationNode: Segment does not contain binary labelmap representation and cannot convert, because it is not in a segmentation!");
      return false;
      }
    binaryLabelmapPresent = segmentationNode->GetSegmentation()->CreateRepresentation(
      vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
    if (!binaryLabelmapPresent)
      {
      vtkErrorWithObjectMacro(representationNode, "ExportSegmentToRepresentationNode: Unable to convert segment to binary labelmap representation!");
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
      vtkErrorWithObjectMacro(representationNode, "ExportSegmentToRepresentationNode: Unable to convert segment to closed surface representation!");
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
      vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
      segmentationDisplayNode->GetSegmentDisplayProperties(segmentId, properties);
      modelDisplayNode->SetColor(properties.Color);
      }

    // Set segmentation's parent transform to exported node
    if (parentTransformNode)
      {
      modelNode->SetAndObserveTransformNodeID(parentTransformNode->GetID());
      }

    return true;
    }

  // Representation node is neither labelmap, nor model
  return false;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ExportSegmentsToLabelmapNode(vtkMRMLSegmentationNode* segmentationNode, std::vector<std::string>& segmentIDs, vtkMRMLLabelMapVolumeNode* labelmapNode)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ExportSegmentsToLabelmapNode: Invalid segmentation node");
    return false;
    }
  if (!labelmapNode)
    {
    vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsToLabelmapNode: Invalid labelmap volume node!");
    return false;
    }

  // Make sure binary labelmap representation exists in segment
  bool binaryLabelmapPresent = segmentationNode->GetSegmentation()->CreateRepresentation(
    vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
  if (!binaryLabelmapPresent)
    {
    vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsToLabelmapNode: Unable to convert segment to binary labelmap representation!");
    return false;
    }

  // Generate merged labelmap for the exported segments
  vtkSmartPointer<vtkOrientedImageData> mergedImage = vtkSmartPointer<vtkOrientedImageData>::New();
  if (!segmentationNode->GenerateMergedLabelmap(mergedImage, vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS, NULL, segmentIDs))
    {
    vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsToLabelmapNode: Failed to generate merged labelmap!");
    return false;
    }

  // Export merged labelmap to the output node
  if (!vtkSlicerSegmentationsModuleLogic::CreateLabelmapVolumeFromOrientedImageData(mergedImage, labelmapNode))
    {
    vtkErrorWithObjectMacro(segmentationNode, "ExportSegmentsToLabelmapNode: Failed to create labelmap from merged segments image!");
    return false;
    }

  // Set segmentation's color table to labelmap so that the labels appear in the same color
  if (labelmapNode->GetDisplayNode())
    {
    if (segmentationNode->GetDisplayNode() && segmentationNode->GetDisplayNode()->GetColorNode())
      {
      labelmapNode->GetDisplayNode()->SetAndObserveColorNodeID(segmentationNode->GetDisplayNode()->GetColorNodeID());
      }
    }

  // Set segmentation's parent transform to exported node
  vtkMRMLTransformNode* parentTransformNode = segmentationNode->GetParentTransformNode();
  if (parentTransformNode)
    {
    labelmapNode->SetAndObserveTransformNodeID(parentTransformNode->GetID());
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ExportAllSegmentsToLabelmapNode(vtkMRMLSegmentationNode* segmentationNode, vtkMRMLLabelMapVolumeNode* labelmapNode)
{
  std::vector<std::string> segmentIDs;
  return vtkSlicerSegmentationsModuleLogic::ExportSegmentsToLabelmapNode(segmentationNode, segmentIDs, labelmapNode);
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ImportModelToSegmentationNode(vtkMRMLModelNode* modelNode, vtkMRMLSegmentationNode* segmentationNode)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ImportModelToSegmentationNode: Invalid segmentation node");
    return false;
    }
  if (!modelNode || !modelNode->GetPolyData())
    {
    vtkErrorWithObjectMacro(segmentationNode, "ImportLabelmapToSegmentationNode: Invalid model node");
    return false;
    }
  vtkSmartPointer<vtkSegment> segment = vtkSmartPointer<vtkSegment>::Take(
    vtkSlicerSegmentationsModuleLogic::CreateSegmentFromModelNode(modelNode, segmentationNode));
  if (!segment.GetPointer())
    {
    return false;
    }
  // Add segment to current segmentation
  segmentationNode->GetSegmentation()->AddSegment(segment);
  segmentationNode->CreateDefaultDisplayNodes();
  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(vtkMRMLLabelMapVolumeNode* labelmapNode, vtkMRMLSegmentationNode* segmentationNode)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode: Invalid segmentation node!");
    return false;
    }
  if (!labelmapNode || !labelmapNode->GetImageData())
    {
    vtkErrorWithObjectMacro(segmentationNode, "ImportLabelmapToSegmentationNode: Invalid labelmap volume node!");
    return false;
    }

  // If master representation is not binary labelmap, then cannot add
  // (this should have been done by the UI classes, notifying the users about hazards of changing the master representation)
  if ( segmentationNode->GetSegmentation()->GetMasterRepresentationName() != vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName())
    {
    vtkErrorWithObjectMacro(segmentationNode, "ImportLabelmapToSegmentationNode: Master representation of the target segmentation node "
      << (segmentationNode->GetName()?segmentationNode->GetName():"NULL") << " is not binary labelmap!");
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

  // Split labelmap node into per-label image data

  vtkNew<vtkIntArray> labelValues;
  vtkSlicerSegmentationsModuleLogic::GetAllLabelValues(labelValues.GetPointer(), labelmapNode->GetImageData());

  // Set master representation to binary labelmap
  segmentationNode->GetSegmentation()->SetMasterRepresentationName(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());

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
    double color[4] = { vtkSegment::SEGMENT_COLOR_VALUE_INVALID[0],
                        vtkSegment::SEGMENT_COLOR_VALUE_INVALID[1],
                        vtkSegment::SEGMENT_COLOR_VALUE_INVALID[2], 1.0 };
    const char* labelName = NULL;
    if (colorNode)
      {
      labelName = colorNode->GetColorName(label);
      colorNode->GetColor(label, color);
      }
    segment->SetDefaultColor(color[0], color[1], color[2]);

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

    segmentationNode->GetSegmentation()->AddSegment(segment);
    } // for each label

  segmentationNode->CreateDefaultDisplayNodes();
  segmentationNode->EndModify(segmentationNodeWasModified);
  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(vtkOrientedImageData* labelmapImage, vtkMRMLSegmentationNode* segmentationNode, std::string baseSegmentName/*=""*/)
{
  if (!segmentationNode)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode: Invalid segmentation node!");
    return false;
    }
  if (!labelmapImage)
    {
    vtkErrorWithObjectMacro(labelmapImage, "ImportLabelmapToSegmentationNode: Invalid labelmap image!");
    return false;
    }

  // If master representation is not binary labelmap, then cannot add
  // (this should have been done by the UI classes, notifying the users about hazards of changing the master representation)
  if (segmentationNode->GetSegmentation()->GetMasterRepresentationName() != vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName())
    {
    vtkErrorWithObjectMacro(segmentationNode, "ImportLabelmapToSegmentationNode: Master representation of the target segmentation node "
      << (segmentationNode->GetName()?segmentationNode->GetName():"NULL") << " is not binary labelmap!");
    return false;
    }

  // Note: Splitting code ported from EditorLib/HelperBox.py:split

  // Split labelmap node into per-label image data

  vtkNew<vtkIntArray> labelValues;
  vtkSlicerSegmentationsModuleLogic::GetAllLabelValues(labelValues.GetPointer(), labelmapImage);

  int segmentationNodeWasModified = segmentationNode->StartModify();

  // Set master representation to binary labelmap
  segmentationNode->GetSegmentation()->SetMasterRepresentationName(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());

  vtkSmartPointer<vtkImageThreshold> threshold = vtkSmartPointer<vtkImageThreshold>::New();
  threshold->SetInputData(labelmapImage);
  threshold->SetInValue(1);
  threshold->SetOutValue(0);
  threshold->ReplaceInOn();
  threshold->ReplaceOutOn();
  threshold->SetOutputScalarType(labelmapImage->GetScalarType());
  //TODO: pending resolution of bug http://www.na-mic.org/Bug/view.php?id=1822,
  //   run the thresholding in single threaded mode to avoid data corruption observed on mac release builds
  //threshold->SetNumberOfThreads(1);
  for (int labelIndex = 0; labelIndex < labelValues->GetNumberOfValues(); ++labelIndex)
  {
    int label = labelValues->GetValue(labelIndex);

    threshold->ThresholdBetween(label, label);
    threshold->Update();

    // Create oriented image data for label
    vtkSmartPointer<vtkOrientedImageData> labelOrientedImageData = vtkSmartPointer<vtkOrientedImageData>::New();
    labelOrientedImageData->vtkImageData::DeepCopy(threshold->GetOutput());
    vtkSmartPointer<vtkMatrix4x4> labelmapImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    labelmapImage->GetImageToWorldMatrix(labelmapImageToWorldMatrix);
    labelOrientedImageData->SetGeometryFromImageToWorldMatrix(labelmapImageToWorldMatrix);

    vtkSmartPointer<vtkSegment> segment = vtkSmartPointer<vtkSegment>::New();

    // Set segment name
    std::stringstream ss;
    ss << (baseSegmentName.empty() ? "Label" : baseSegmentName) << "_" << label;
    segment->SetName(ss.str().c_str());

    // Apply parent transforms if any
    vtkMRMLTransformNode* segmentationParentTransformNode = segmentationNode->GetParentTransformNode();
    if (segmentationParentTransformNode)
      {
      vtkSmartPointer<vtkGeneralTransform> worldToSegmentationTransform = vtkSmartPointer<vtkGeneralTransform>::New();
      if (segmentationParentTransformNode)
        {
        segmentationParentTransformNode->GetTransformFromWorld(worldToSegmentationTransform);
        }

      vtkOrientedImageDataResample::TransformOrientedImage(labelOrientedImageData, worldToSegmentationTransform);
      }

    // Add oriented image data as binary labelmap representation
    segment->AddRepresentation(
      vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName(),
      labelOrientedImageData );

    segmentationNode->GetSegmentation()->AddSegment(segment);
    } // for each label

  segmentationNode->CreateDefaultDisplayNodes();
  segmentationNode->EndModify(segmentationNodeWasModified);
  return true;
}

//-----------------------------------------------------------------------------
vtkDataObject* vtkSlicerSegmentationsModuleLogic::CreateRepresentationForOneSegment(vtkSegmentation* segmentation, std::string segmentID, std::string representationName)
{
  if (!segmentation)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::CreateRepresentationForOneSegment: Invalid segmentation!");
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
    vtkErrorWithObjectMacro(segmentation, "CreateRepresentationForOneSegment: Failed to get representation " << representationName << " from segment " << segmentID);
    return NULL;
    }

  // Copy representation into new data object (the representation will be deleted when segmentation copy gets out of scope)
  vtkDataObject* representationCopy =
    vtkSegmentationConverterFactory::GetInstance()->ConstructRepresentationObjectByClass(segmentTempRepresentation->GetClassName());
  representationCopy->ShallowCopy(segmentTempRepresentation);
  return representationCopy;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ApplyParentTransformToOrientedImageData(vtkMRMLTransformableNode* transformableNode, vtkOrientedImageData* orientedImageData)
{
  if (!transformableNode || !orientedImageData)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ApplyParentTransformToOrientedImageData: Invalid inputs!");
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
  vtkOrientedImageDataResample::TransformOrientedImage(orientedImageData, nodeToWorldTransform);

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::ApplyParentTransformToPolyData(vtkMRMLTransformableNode* transformableNode, vtkPolyData* polyData)
{
  if (!transformableNode || !polyData)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::ApplyParentTransformToPolyData: Invalid inputs!");
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
bool vtkSlicerSegmentationsModuleLogic::GetTransformBetweenRepresentationAndSegmentation(vtkMRMLTransformableNode* representationNode, vtkMRMLSegmentationNode* segmentationNode, vtkGeneralTransform* representationToSegmentationTransform)
{
  if (!representationNode || !segmentationNode || !representationToSegmentationTransform)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::GetTransformBetweenRepresentationAndSegmentation: Invalid inputs");
    return false;
    }
  vtkMRMLTransformNode::GetTransformBetweenNodes(representationNode->GetParentTransformNode(), segmentationNode->GetParentTransformNode(), representationToSegmentationTransform);

  return true;
}


//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation(vtkMRMLSegmentationNode* segmentationNode, std::string segmentID, std::string representationName, vtkDataObject* segmentRepresentation, bool applyParentTransform/*=true*/)
{
  if (!segmentationNode || segmentID.empty() || representationName.empty() || !segmentRepresentation)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation: Invalid inputs!");
    return false;
    }

  // Get requested segment
  vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(segmentID);
  if (!segment)
    {
    vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation: Unable to find segment with ID " << segmentID << " in segmentation " << segmentationNode->GetName());
    return false;
    }

  if (segmentationNode->GetSegmentation()->ContainsRepresentation(representationName))
    {
    // Get and copy representation into output data object
    vtkDataObject* representationObject = segment->GetRepresentation(representationName);
    if (!representationObject)
      {
      vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation: Unable to get '" << representationName << "' representation from segment with ID " << segmentID << " in segmentation " << segmentationNode->GetName());
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
      vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation: Unable to convert segment with ID " << segmentID << " to '" << representationName << "' representation in segmentation " << segmentationNode->GetName());
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
        vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation: Failed to apply parent transform of segmentation " << segmentationNode->GetName() << " on representation oriented image data");
        return false;
        }
      }
    else if (segmentRepresentationPolyData)
      {
        if (!vtkSlicerSegmentationsModuleLogic::ApplyParentTransformToPolyData(segmentationNode, segmentRepresentationPolyData))
        {
        vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation: Failed to apply parent transform of segmentation " << segmentationNode->GetName() << " on representation poly data");
        return false;
        }
      }
    else
      {
      vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation: Failed to apply parent transform of segmentation " << segmentationNode->GetName() << " due to unsupported representation with class name " << segmentRepresentation->GetClassName());
      return false;
      }
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::GetSegmentBinaryLabelmapRepresentation(vtkMRMLSegmentationNode* segmentationNode, std::string segmentID, vtkOrientedImageData* imageData, bool applyParentTransform/*=true*/)
{
  if (!segmentationNode || segmentID.empty() || !imageData)
    {
    vtkGenericWarningMacro("vtkSlicerSegmentationsModuleLogic::GetSegmentBinaryLabelmapRepresentation: Invalid inputs!");
    return false;
    }

  return vtkSlicerSegmentationsModuleLogic::GetSegmentRepresentation(segmentationNode, segmentID,
    vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName(), imageData, applyParentTransform );
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment(vtkOrientedImageData* labelmap, vtkMRMLSegmentationNode* segmentationNode, std::string segmentID, int mergeMode/*=MODE_REPLACE*/, const int extent[6]/*=0*/)
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
  vtkOrientedImageData* segmentLabelmap = vtkOrientedImageData::SafeDownCast(
    selectedSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()) );
  if (!segmentLabelmap)
    {
    vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment: Failed to get binary labelmap representation in segmentation " << segmentationNode->GetName());
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
      vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(segmentLabelmap, labelmap, resampledSegmentLabelmap);
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
    vtkErrorWithObjectMacro(segmentationNode, "vtkSlicerSegmentationsModuleLogic::SetBinaryLabelmapToSegment: Effective extent of the labelmap to set is invalid!");
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
