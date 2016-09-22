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
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkMRMLSegmentationStorageNode.h"

// SegmentationCore includes
#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkCalculateOversamplingFactor.h"

// MRML includes
#include <vtkEventBroker.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLSubjectHierarchyConstants.h>
#include <vtkMRMLScalarVolumeNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkGeneralTransform.h>
#include <vtkHomogeneousTransform.h>
#include <vtkIntArray.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>

// STD includes
#include <algorithm>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSegmentationNode);

//----------------------------------------------------------------------------
vtkMRMLSegmentationNode::vtkMRMLSegmentationNode()
{
  this->SegmentationModifiedCallbackCommand = vtkSmartPointer<vtkCallbackCommand>::New();
  this->SegmentationModifiedCallbackCommand->SetClientData(reinterpret_cast<void *>(this));
  this->SegmentationModifiedCallbackCommand->SetCallback(vtkMRMLSegmentationNode::SegmentationModifiedCallback);

  // Create empty segmentations object
  this->Segmentation = NULL;
  vtkSmartPointer<vtkSegmentation> segmentation = vtkSmartPointer<vtkSegmentation>::New();
  this->SetAndObserveSegmentation(segmentation);
}

//----------------------------------------------------------------------------
vtkMRMLSegmentationNode::~vtkMRMLSegmentationNode()
{
  this->SetAndObserveSegmentation(NULL);
  // Make sure this callback cannot call this object
  this->SegmentationModifiedCallbackCommand->SetClientData(NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentationNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  if (this->Segmentation)
    {
    this->Segmentation->WriteXML(of, nIndent);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentationNode::ReadXMLAttributes(const char** atts)
{
  // Read all MRML node attributes from two arrays of names and values
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  if (!this->Segmentation)
    {
    vtkSmartPointer<vtkSegmentation> segmentation = vtkSmartPointer<vtkSegmentation>::New();
    this->SetAndObserveSegmentation(segmentation);
    }
  this->Segmentation->ReadXMLAttributes(atts);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentationNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLSegmentationNode* otherNode = vtkMRMLSegmentationNode::SafeDownCast(anode);
  if (!otherNode)
    {
    vtkErrorMacro("vtkMRMLSegmentationNode::Copy failed: invalid input node");
    return;
    }

  int wasModified = this->StartModify();

  Superclass::Copy(anode);

  if (otherNode->Segmentation)
    {
    if (!this->Segmentation)
      {
      vtkSmartPointer<vtkSegmentation> segmentation = vtkSmartPointer<vtkSegmentation>::New();
      this->SetAndObserveSegmentation(segmentation);
      }
    // Deep copy segmentation (containing the same segments from two segmentations is unstable)
    this->Segmentation->DeepCopy(otherNode->GetSegmentation());
    }
  else
    {
    this->SetAndObserveSegmentation(NULL);
    }

  this->EndModify(wasModified);
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentationNode::DeepCopy(vtkMRMLNode* aNode)
{
  Copy(aNode);
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentationNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "Segmentation:";
  if (this->Segmentation)
    {
    this->Segmentation->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << " (invalid)\n";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentationNode::SetAndObserveSegmentation(vtkSegmentation* segmentation)
{
  if (segmentation == this->Segmentation)
    {
    return;
    }

  // Remove segment event observations from previous segmentation
  if (this->Segmentation)
    {
    vtkEventBroker::GetInstance()->RemoveObservations(
      this->Segmentation, 0, this, this->SegmentationModifiedCallbackCommand);
    }

  this->SetSegmentation(segmentation);

  // Observe segment events in new segmentation
  if (this->Segmentation)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      this->Segmentation, vtkSegmentation::MasterRepresentationModified, this, this->SegmentationModifiedCallbackCommand);
    vtkEventBroker::GetInstance()->AddObservation(
      this->Segmentation, vtkSegmentation::SegmentAdded, this, this->SegmentationModifiedCallbackCommand);
    vtkEventBroker::GetInstance()->AddObservation(
      this->Segmentation, vtkSegmentation::SegmentRemoved, this, this->SegmentationModifiedCallbackCommand);
    vtkEventBroker::GetInstance()->AddObservation(
      this->Segmentation, vtkSegmentation::SegmentModified, this, this->SegmentationModifiedCallbackCommand);
    vtkEventBroker::GetInstance()->AddObservation(
      this->Segmentation, vtkSegmentation::ContainedRepresentationNamesModified, this, this->SegmentationModifiedCallbackCommand);
    vtkEventBroker::GetInstance()->AddObservation(
      this->Segmentation, vtkSegmentation::RepresentationModified, this, this->SegmentationModifiedCallbackCommand);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationNode::SegmentationModifiedCallback(vtkObject* vtkNotUsed(caller), unsigned long eid, void* clientData, void* callData)
{
  vtkMRMLSegmentationNode* self = reinterpret_cast<vtkMRMLSegmentationNode*>(clientData);
  if (!self)
    {
    return;
    }
  if (!self->Segmentation)
    {
    // this object is being deleted
    return;
    }
  switch (eid)
    {
    case vtkSegmentation::MasterRepresentationModified:
      self->OnMasterRepresentationModified();
      self->InvokeCustomModifiedEvent(eid, callData);
      break;
    case vtkSegmentation::RepresentationModified:
      self->StorableModifiedTime.Modified();
      self->InvokeCustomModifiedEvent(eid, callData);
      break;
    case vtkSegmentation::ContainedRepresentationNamesModified:
      self->StorableModifiedTime.Modified();
      self->InvokeCustomModifiedEvent(eid);
      break;
    case vtkSegmentation::SegmentAdded:
      self->StorableModifiedTime.Modified();
      self->OnSegmentAdded(reinterpret_cast<char*>(callData));
      self->InvokeCustomModifiedEvent(eid, callData);
      break;
    case vtkSegmentation::SegmentRemoved:
      self->StorableModifiedTime.Modified();
      self->OnSegmentRemoved(reinterpret_cast<char*>(callData));
      self->InvokeCustomModifiedEvent(eid, callData);
      break;
    case vtkSegmentation::SegmentModified:
      self->StorableModifiedTime.Modified();
      self->OnSegmentModified(reinterpret_cast<char*>(callData));
      self->InvokeCustomModifiedEvent(eid, callData);
      break;
    default:
      vtkErrorWithObjectMacro(self, "vtkMRMLSegmentationNode::SegmentationModifiedCallback: Unknown event id "<<eid);
      return;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationNode::OnMasterRepresentationModified()
{
  // Reset supported write file types
  vtkMRMLSegmentationStorageNode* storageNode =  vtkMRMLSegmentationStorageNode::SafeDownCast(this->GetStorageNode());
  if (storageNode)
    {
    storageNode->ResetSupportedWriteFileTypes();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationNode::OnSegmentAdded(const char* segmentId)
{
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationNode::OnSegmentRemoved(const char* segmentId)
{
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationNode::OnSegmentModified(const char* segmentId)
{
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationNode::OnSubjectHierarchyUIDAdded(vtkMRMLSubjectHierarchyNode* shNodeWithNewUID)
{
  if (!shNodeWithNewUID || !this->Segmentation)
    {
    return;
    }
  // If already has geometry, then do not look for a new one
  if (!this->Segmentation->GetConversionParameter(vtkSegmentationConverter::GetReferenceImageGeometryParameterName()).empty())
    {
    return;
    }

  // Get associated volume node from subject hierarchy node with new UID
  vtkMRMLScalarVolumeNode* referencedVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(shNodeWithNewUID->GetAssociatedNode());
  if (!referencedVolumeNode)
    {
    // If associated node is not a volume, then return
    return;
    }

  // Get associated subject hierarchy node
  vtkMRMLSubjectHierarchyNode* segmentationShNode = vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(this);
  if (!segmentationShNode)
    {
    // If segmentation is not in subject hierarchy, then we cannot find its DICOM references
    return;
    }

  // Get DICOM references from segmentation subject hierarchy node
  const char* referencedInstanceUIDsAttribute = segmentationShNode->GetAttribute(
    vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName().c_str());
  if (!referencedInstanceUIDsAttribute)
    {
    // No references
    return;
    }

  // If the subject hierarchy node that got a new UID has a DICOM instance UID referenced
  // from this segmentation, then use its geometry as image geometry conversion parameter
  std::vector<std::string> referencedSopInstanceUids;
  vtkMRMLSubjectHierarchyNode::DeserializeUIDList(referencedInstanceUIDsAttribute, referencedSopInstanceUids);
  bool referencedVolumeFound = false;
  bool warningLogged = false;
  std::vector<std::string>::iterator uidIt;
  std::string nodeUidValueStr = shNodeWithNewUID->GetUID(vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName());
  if (nodeUidValueStr.empty())
    {
    // If the new UID is empty string, then do not look further
    return;
    }
  for (uidIt = referencedSopInstanceUids.begin(); uidIt != referencedSopInstanceUids.end(); ++uidIt)
    {
    // If we find the instance UID, then we set the geometry
    if (nodeUidValueStr.find(*uidIt) != std::string::npos)
      {
      // Only set the reference once, but check all UIDs
      if (!referencedVolumeFound)
        {
        // Set reference image geometry parameter if volume node is found
        this->SetReferenceImageGeometryParameterFromVolumeNode(referencedVolumeNode);
        referencedVolumeFound = true;
        }
      }
    // If referenced UID is not contained in found node, then warn user
    else if (referencedVolumeFound && !warningLogged)
      {
      vtkWarningMacro("vtkMRMLSegmentationNode::OnSubjectHierarchyUIDAdded: Referenced volume for segmentation '"
        << this->Name << "' found (" << referencedVolumeNode->GetName() << "), but some referenced UIDs are not present in it! (maybe only partial volume was loaded?)");
      // Only log warning once for this node
      warningLogged = true;
      }
    }
}

//----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLSegmentationNode::CreateDefaultStorageNode()
{
  return vtkMRMLSegmentationStorageNode::New();
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentationNode::CreateDefaultDisplayNodes()
{
  if (vtkMRMLSegmentationDisplayNode::SafeDownCast(this->GetDisplayNode()))
    {
    // Display node already exists
    return;
    }
  if (this->GetScene() == NULL)
    {
    vtkErrorMacro("vtkMRMLSegmentationNode::CreateDefaultDisplayNodes failed: Scene is invalid");
    return;
    }
  vtkNew<vtkMRMLSegmentationDisplayNode> displayNode;
  this->GetScene()->AddNode(displayNode.GetPointer());
  this->SetAndObserveDisplayNodeID(displayNode->GetID());
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationNode::ApplyTransformMatrix(vtkMatrix4x4* transformMatrix)
{
  vtkTransform* transform = vtkTransform::New();
  transform->SetMatrix(transformMatrix);
  this->ApplyTransform(transform);
  transform->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLSegmentationNode::ApplyTransform(vtkAbstractTransform* transform)
{
  if (!this->Segmentation)
    {
    return;
    }

  // Apply transform on segmentation
  vtkSmartPointer<vtkTransform> linearTransform = vtkSmartPointer<vtkTransform>::New();
  if (vtkOrientedImageDataResample::IsTransformLinear(transform, linearTransform))
    {
    this->Segmentation->ApplyLinearTransform(transform);
    }
  else
    {
    this->Segmentation->ApplyNonLinearTransform(transform);
    }

  // Make sure preferred display representations exist after transformation
  // (it is invalidated in the process unless it is the master representation)
  char* preferredDisplayRepresentation2D = NULL;
  char* preferredDisplayRepresentation3D = NULL;
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(this->GetDisplayNode());
  if (displayNode)
  {
    preferredDisplayRepresentation2D = displayNode->GetPreferredDisplayRepresentationName2D();
    preferredDisplayRepresentation3D = displayNode->GetPreferredDisplayRepresentationName3D();
  }

  // Make sure preferred display representations exist after transformation
  // (it was invalidated in the process unless it is the master representation)
  if (displayNode)
    {
    if (preferredDisplayRepresentation2D)
      {
      this->Segmentation->CreateRepresentation(preferredDisplayRepresentation2D);
      }
    if (preferredDisplayRepresentation3D)
      {
      this->Segmentation->CreateRepresentation(preferredDisplayRepresentation3D);
      }
    // Need to set preferred representations again, as conversion sets them to the last converted one
    displayNode->SetPreferredDisplayRepresentationName2D(preferredDisplayRepresentation2D);
    displayNode->SetPreferredDisplayRepresentationName3D(preferredDisplayRepresentation3D);
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLSegmentationNode::CanApplyNonLinearTransforms() const
{
  return true;
}

//---------------------------------------------------------------------------
// Global RAS in the form (Xmin, Xmax, Ymin, Ymax, Zmin, Zmax)
//---------------------------------------------------------------------------
void vtkMRMLSegmentationNode::GetRASBounds(double bounds[6])
{
  vtkOrientedImageData::UninitializeBounds(bounds);
  if (this->Segmentation)
    {
    this->Segmentation->GetBounds(bounds);
    }
}


//---------------------------------------------------------------------------
bool vtkMRMLSegmentationNode::GenerateMergedLabelmap(
  vtkOrientedImageData* mergedImageData,
  int extentComputationMode,
  vtkOrientedImageData* mergedLabelmapGeometry/*=NULL*/,
  const std::vector<std::string>& segmentIDs/*=std::vector<std::string>()*/
  )
{
  if (!mergedImageData)
    {
    vtkErrorMacro("GenerateMergedLabelmap: Invalid image data");
    return false;
    }
  // If segmentation is missing or empty then we cannot create a merged image data
  if (!this->Segmentation)
    {
    vtkErrorMacro("GenerateMergedLabelmap: Invalid segmentation");
    return false;
    }
  if (!this->Segmentation->ContainsRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()))
    {
    vtkErrorMacro("GenerateMergedLabelmap: Segmentation does not contain binary labelmap representation");
    return false;
    }

  // If segment IDs list is empty then include all segments
  vtkSegmentation::SegmentMap segmentMap = this->Segmentation->GetSegments();
  std::vector<std::string> mergedSegmentIDs;
  if (segmentIDs.empty())
    {
    for (vtkSegmentation::SegmentMap::iterator segmentIt = segmentMap.begin(); segmentIt != segmentMap.end(); ++segmentIt)
      {
      mergedSegmentIDs.push_back(segmentIt->first);
      }
    }
  else
    {
    mergedSegmentIDs = segmentIDs;
    }

  // Determine common labelmap geometry that will be used for the merged labelmap
  vtkSmartPointer<vtkMatrix4x4> mergedImageToWorldMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  vtkSmartPointer<vtkOrientedImageData> commonGeometryImage;
  if (mergedLabelmapGeometry)
    {
    // Use merged labelmap geometry if provided
    commonGeometryImage = mergedLabelmapGeometry;
    mergedLabelmapGeometry->GetImageToWorldMatrix(mergedImageToWorldMatrix);
    }
  else
    {
    commonGeometryImage = vtkSmartPointer<vtkOrientedImageData>::New();
    std::string commonGeometryString = this->Segmentation->DetermineCommonLabelmapGeometry(extentComputationMode, mergedSegmentIDs);
    if (commonGeometryString.empty())
      {
      // This can occur if there are only empty segments in the segmentation
      mergedImageToWorldMatrix->Identity();
      return true;
      }
    vtkSegmentationConverter::DeserializeImageGeometry(commonGeometryString, commonGeometryImage);
    }
  commonGeometryImage->GetImageToWorldMatrix(mergedImageToWorldMatrix);
  int referenceDimensions[3] = {0,0,0};
  commonGeometryImage->GetDimensions(referenceDimensions);
  int referenceExtent[6] = {0,-1,0,-1,0,-1};
  commonGeometryImage->GetExtent(referenceExtent);

  // Allocate image data if empty or if reference extent changed
  int imageDataExtent[6] = {0,-1,0,-1,0,-1};
  mergedImageData->GetExtent(imageDataExtent);
  if ( mergedImageData->GetScalarType() != VTK_SHORT
    || imageDataExtent[0] != referenceExtent[0] || imageDataExtent[1] != referenceExtent[1] || imageDataExtent[2] != referenceExtent[2]
    || imageDataExtent[3] != referenceExtent[3] || imageDataExtent[4] != referenceExtent[4] || imageDataExtent[5] != referenceExtent[5] )
    {
    if (mergedImageData->GetPointData()->GetScalars() && mergedImageData->GetScalarType() != VTK_SHORT)
      {
      vtkWarningMacro("GenerateMergedLabelmap: Merged image data scalar type is not short. Allocating using short.");
      }
    mergedImageData->SetExtent(referenceExtent);
    mergedImageData->AllocateScalars(VTK_SHORT, 1);
    }
  mergedImageData->SetImageToWorldMatrix(mergedImageToWorldMatrix);

  // Paint the image data background first
  unsigned short backgroundColor = vtkMRMLSegmentationDisplayNode::GetSegmentationColorIndexBackground();
  short* mergedImagePtr = (short*)mergedImageData->GetScalarPointerForExtent(referenceExtent);
  if (!mergedImagePtr)
    {
    // Setting the extent may invoke this function again via ImageDataModified, in which case the pointer is NULL
    return false;
    }

  vtkOrientedImageDataResample::FillImage(mergedImageData, backgroundColor);

  // Skip the rest if there are no segments
  if (this->Segmentation->GetNumberOfSegments() == 0)
    {
    return true;
    }

  // Create merged labelmap
  for (vtkSegmentation::SegmentMap::iterator segmentIt = segmentMap.begin(); segmentIt != segmentMap.end(); ++segmentIt)
    {
    std::string currentSegmentId(segmentIt->first);
    vtkSegment* currentSegment = segmentIt->second;
    bool segmentIncluded = ( std::find(mergedSegmentIDs.begin(), mergedSegmentIDs.end(), std::string(currentSegmentId)) != mergedSegmentIDs.end() );
    if (!segmentIncluded)
      {
      continue;
      }
    // Get color table index for the segment
    std::string colorIndexStr;
    bool tagFound = currentSegment->GetTag(vtkMRMLSegmentationDisplayNode::GetColorIndexTag(), colorIndexStr);
    if (!tagFound)
      {
      vtkErrorMacro("GenerateMergedLabelmap: No color table index found for segment " << currentSegmentId);
      continue;
      }
    short colorIndex = vtkVariant(colorIndexStr).ToShort();

    // Get binary labelmap from segment
    vtkOrientedImageData* representationBinaryLabelmap = vtkOrientedImageData::SafeDownCast(
      currentSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()) );
    // If binary labelmap is empty then skip
    if (representationBinaryLabelmap->IsEmpty())
      {
      continue;
      }

    // Set oriented image data used for merging to the representation (may change later if resampling is needed)
    vtkOrientedImageData* binaryLabelmap = representationBinaryLabelmap;

    // If labelmap geometries (origin, spacing, and directions) do not match reference then resample temporarily
    vtkSmartPointer<vtkOrientedImageData> resampledBinaryLabelmap;
    if (!vtkOrientedImageDataResample::DoGeometriesMatch(commonGeometryImage, representationBinaryLabelmap))
      {
      resampledBinaryLabelmap = vtkSmartPointer<vtkOrientedImageData>::New();

      // Resample segment labelmap for merging
      if (!vtkOrientedImageDataResample::ResampleOrientedImageToReferenceGeometry(representationBinaryLabelmap, mergedImageToWorldMatrix, resampledBinaryLabelmap))
        {
        continue;
        }

      // Use resampled labelmap for merging
      binaryLabelmap = resampledBinaryLabelmap;
      }

    // Copy image data voxels into merged labelmap with the proper color index
    vtkOrientedImageDataResample::ModifyImage(
          mergedImageData,
          binaryLabelmap,
          vtkOrientedImageDataResample::OPERATION_MASKING,
          NULL,
          0,
          colorIndex);
    }

  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLSegmentationNode::GenerateMergedLabelmapForAllSegments(vtkOrientedImageData* mergedImageData, int extentComputationMode,
  vtkOrientedImageData* mergedLabelmapGeometry /*=NULL*/, vtkStringArray* segmentIDs /*=NULL*/)
{
  std::vector<std::string> segmentIDsVector;
  if (segmentIDs)
    {
    for (int i = 0; i < segmentIDs->GetNumberOfValues(); i++)
      {
      segmentIDsVector.push_back(segmentIDs->GetValue(i));
      }
    }
  return this->GenerateMergedLabelmap(mergedImageData, extentComputationMode, mergedLabelmapGeometry, segmentIDsVector);
}

//---------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* vtkMRMLSegmentationNode::GetSegmentSubjectHierarchyNode(std::string segmentID)
{
  vtkMRMLSubjectHierarchyNode* segmentationSubjectHierarchyNode =
    vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(this);
  if (!segmentationSubjectHierarchyNode)
    {
    return NULL;
    }

  // Find child node of segmentation subject hierarchy node that has the requested segment ID
  std::vector<vtkMRMLHierarchyNode*> children = segmentationSubjectHierarchyNode->GetChildrenNodes();
  for (std::vector<vtkMRMLHierarchyNode*>::iterator childIt=children.begin(); childIt!=children.end(); ++childIt)
    {
    vtkMRMLSubjectHierarchyNode* childNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(*childIt);
    if ( childNode && childNode->GetAttribute(vtkMRMLSegmentationNode::GetSegmentIDAttributeName())
      && !segmentID.compare(childNode->GetAttribute(vtkMRMLSegmentationNode::GetSegmentIDAttributeName())) )
      {
      return childNode;
      }
    }

  return NULL;
}

//---------------------------------------------------------------------------
void vtkMRMLSegmentationNode::SetReferenceImageGeometryParameterFromVolumeNode(vtkMRMLScalarVolumeNode* volumeNode)
{
  if (!volumeNode || !volumeNode->GetImageData())
    {
    return;
    }
  if (!this->Segmentation)
    {
    vtkSmartPointer<vtkSegmentation> segmentation = vtkSmartPointer<vtkSegmentation>::New();
    this->SetAndObserveSegmentation(segmentation);
    }

  // Get serialized geometry of selected volume
  vtkSmartPointer<vtkMatrix4x4> volumeIjkToRasMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  volumeNode->GetIJKToRASMatrix(volumeIjkToRasMatrix);

  // If there is a linear transform between the reference volume and segmentation then transform the geometry
  // to be aligned with the reference volume.
  if (volumeNode->GetParentTransformNode() != this->GetParentTransformNode())
    {
    vtkSmartPointer<vtkGeneralTransform> volumeToSegmentationTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    vtkMRMLTransformNode::GetTransformBetweenNodes(volumeNode->GetParentTransformNode(), this->GetParentTransformNode(), volumeToSegmentationTransform);
    if (vtkMRMLTransformNode::IsGeneralTransformLinear(volumeToSegmentationTransform))
      {
      vtkNew<vtkMatrix4x4> volumeToSegmentationMatrix;
      vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(volumeNode->GetParentTransformNode(), this->GetParentTransformNode(), volumeToSegmentationMatrix.GetPointer());
      vtkMatrix4x4::Multiply4x4(volumeToSegmentationMatrix.GetPointer(), volumeIjkToRasMatrix, volumeIjkToRasMatrix);
      }
    }

  std::string serializedImageGeometry = vtkSegmentationConverter::SerializeImageGeometry(
    volumeIjkToRasMatrix, volumeNode->GetImageData() );

  // Set parameter
  this->Segmentation->SetConversionParameter(
    vtkSegmentationConverter::GetReferenceImageGeometryParameterName(), serializedImageGeometry);

  // Set node reference from segmentation to reference geometry volume
  this->SetNodeReferenceID(
    vtkMRMLSegmentationNode::GetReferenceImageGeometryReferenceRole().c_str(), volumeNode->GetID() );
}
