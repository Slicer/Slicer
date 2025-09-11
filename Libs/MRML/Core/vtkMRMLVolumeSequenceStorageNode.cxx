/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Children's Hospital of Philadelphia, USA. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, Ebatinca, funded
  by the grant GRT-00000485 of Children's Hospital of Philadelphia, USA.

==============================================================================*/

// MRML includes
#include "vtkMRMLI18N.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLVolumeSequenceStorageNode.h"

// vtkAddon includes
#include <vtkAddonMathUtilities.h>

// vtkITK includes
#include "vtkITKImageSequenceReader.h"
#include "vtkITKImageSequenceWriter.h"

// VTK includes
#include <vtkErrorCode.h>
#include <vtkImageData.h>
#include <vtkMatrix3x3.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkStringArray.h>

// VTKsys includes
#include "vtksys/SystemTools.hxx"

// STD includes
#include <algorithm>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLVolumeSequenceStorageNode);

//----------------------------------------------------------------------------
vtkMRMLVolumeSequenceStorageNode::vtkMRMLVolumeSequenceStorageNode()
{
  this->TypeDisplayName = vtkMRMLTr("vtkMRMLVolumeSequenceStorageNode", "Volume Sequence Storage");
}

//----------------------------------------------------------------------------
vtkMRMLVolumeSequenceStorageNode::~vtkMRMLVolumeSequenceStorageNode() = default;

//----------------------------------------------------------------------------
bool vtkMRMLVolumeSequenceStorageNode::CanReadInReferenceNode(vtkMRMLNode* refNode)
{
  return refNode->IsA("vtkMRMLSequenceNode");
}

//----------------------------------------------------------------------------
int vtkMRMLVolumeSequenceStorageNode::ReadDataInternal(vtkMRMLNode* refNode)
{
  if (!this->CanReadInReferenceNode(refNode))
  {
    return 0;
  }

  vtkMRMLSequenceNode* volSequenceNode = dynamic_cast<vtkMRMLSequenceNode*>(refNode);
  if (!volSequenceNode)
  {
    vtkErrorMacro(<< "vtkMRMLVolumeSequenceStorageNode::ReadDataInternal : not a Sequence node.");
    return 0;
  }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string(""))
  {
    vtkErrorMacro(<< "vtkMRMLVolumeSequenceStorageNode::ReadDataInternal : File name not specified");
    return 0;
  }

  // Read first frame and check success
  vtkNew<vtkITKImageSequenceReader> reader;
  reader->SetFileName(fullName.c_str());
  reader->Update(); // This will read all the frames into the cache
  if (reader->GetErrorCode() != vtkErrorCode::NoError)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLVolumeSequenceStorageNode::ReadDataInternal", "Error reading file.");
    return 0;
  }

  // Read custom attributes
  std::vector<std::string> indexValues;
  typedef std::vector<std::string> KeyVector;
  KeyVector keys = reader->GetHeaderKeysVector();
  std::string dataNodeClassName;
  for (KeyVector::iterator kit = keys.begin(); kit != keys.end(); ++kit)
  {
    if (*kit == "axis 0 index type" || *kit == "axis 3 index type")
    {
      volSequenceNode->SetIndexTypeFromString(reader->GetHeaderValue(kit->c_str()));
    }
    else if (*kit == "axis 0 index values" || *kit == "axis 3 index values")
    {
      std::string indexValue;
      for (std::istringstream indexValueList(reader->GetHeaderValue(kit->c_str())); indexValueList >> indexValue;)
      {
        // Encode string to make sure there are no spaces in the serialized index value (space is used as separator)
        indexValues.push_back(vtkMRMLNode::URLDecodeString(indexValue.c_str()));
      }
    }
    else if (*kit == "DataNodeClassName")
    {
      dataNodeClassName = reader->GetHeaderValue(kit->c_str());
    }
    else
    {
      volSequenceNode->SetAttribute(kit->c_str(), reader->GetHeaderValue(kit->c_str()));
    }
  }

  for (int frameIndex = 0; frameIndex < reader->GetNumberOfCachedImages(); ++frameIndex)
  {
    vtkImageData* frameImage = reader->GetCachedImage(frameIndex);
    if (frameImage == nullptr || frameImage->GetPointData() == nullptr || frameImage->GetPointData()->GetScalars() == nullptr)
    {
      vtkErrorMacro("vtkMRMLVolumeSequenceStorageNode::ReadDataInternal: invalid image data");
      return 0;
    }

    // Create appropriate volume node based on hint in the file or number of components
    vtkSmartPointer<vtkMRMLVolumeNode> frameVolume;
    if (dataNodeClassName.empty())
    {
      if (frameImage->GetNumberOfScalarComponents() > 1)
      {
        dataNodeClassName = "vtkMRMLVectorVolumeNode";
      }
      else
      {
        dataNodeClassName = "vtkMRMLScalarVolumeNode";
      }
    }
    if (this->GetScene())
    {
      vtkMRMLNode* newNode = this->GetScene()->CreateNodeByClass(dataNodeClassName.c_str());
      frameVolume = vtkSmartPointer<vtkMRMLVolumeNode>::Take(vtkMRMLVolumeNode::SafeDownCast(newNode));
    }
    else
    {
      vtkWarningMacro("vtkMRMLVolumeSequenceStorageNode::ReadDataInternal: Scene is not set.");
    }
    if (!frameVolume)
    {
      if (dataNodeClassName != "vtkMRMLScalarVolumeNode")
      {
        vtkErrorMacro("Requested DataNodeClass is " << dataNodeClassName << " but volume sequence will be read into vtkMRMLScalarVolumeNode.");
      }
      frameVolume = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
    }

    // Copy origin and spacing from image data to volume node
    double origin[3] = { 0.0, 0.0, 0.0 };
    double spacing[3] = { 1.0, 1.0, 1.0 };
    frameImage->GetOrigin(origin);
    frameImage->GetSpacing(spacing);

    // Clear origin, spacing, and directions from image data since they are now in the volume node
    frameImage->SetOrigin(0.0, 0.0, 0.0);
    frameImage->SetSpacing(1.0, 1.0, 1.0);
    vtkNew<vtkMatrix3x3> identityDirections;
    frameImage->SetDirectionMatrix(identityDirections);

    // Set up the volume node
    frameVolume->SetAndObserveImageData(frameImage);
    frameVolume->SetRASToIJKMatrix(reader->GetRasToIjkMatrix());

    frameVolume->SetVoxelVectorType(vtkMRMLVolumeArchetypeStorageNode::ConvertVoxelVectorTypeVTKITKToMRML(reader->GetVoxelVectorType()));

    std::ostringstream indexStr;
    if (static_cast<int>(indexValues.size()) > frameIndex)
    {
      indexStr << indexValues[frameIndex];
    }
    else
    {
      indexStr << frameIndex;
    }

    std::ostringstream nameStr;
    nameStr << (refNode->GetName() ? refNode->GetName() : "Node") << "_" << std::setw(4) << std::setfill('0') << frameIndex;
    frameVolume->SetName(nameStr.str().c_str());
    volSequenceNode->SetDataNodeAtValue(frameVolume, indexStr.str().c_str());
  }

  // Read axis label and unit
  volSequenceNode->SetIndexName(reader->GetSequenceAxisLabel().empty() ? "frame" : reader->GetSequenceAxisLabel());
  volSequenceNode->SetIndexUnit(reader->GetSequenceAxisUnit().empty() ? "" : reader->GetSequenceAxisUnit());

  vtkDebugMacro(<< " vtkMRMLVolumeSequenceStorageNode::ReadDataInternal: sequence successfully read. ");

  // success
  return 1;
}

//----------------------------------------------------------------------------
bool vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode(vtkMRMLNode* refNode)
{
  vtkMRMLSequenceNode* volSequenceNode = vtkMRMLSequenceNode::SafeDownCast(refNode);
  if (volSequenceNode == nullptr)
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Data node must be a sequence node."));
    return false;
  }
  vtkMRMLVolumeNode* firstFrameVolume = vtkMRMLVolumeNode::SafeDownCast(volSequenceNode->GetNthDataNode(0));
  if (firstFrameVolume == nullptr)
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only volume nodes can be written."));
    return false;
  }

  int firstFrameVolumeExtent[6] = { 0, -1, 0, -1, 0, -1 };
  int firstFrameVolumeScalarType = VTK_VOID;
  int firstFrameVolumeNumberOfComponents = 0;
  int firstFrameVolumeVoxelVectorType = vtkMRMLVolumeNode::VoxelVectorTypeUndefined;
  if (firstFrameVolume->GetImageData())
  {
    firstFrameVolume->GetImageData()->GetExtent(firstFrameVolumeExtent);
    firstFrameVolumeScalarType = firstFrameVolume->GetImageData()->GetScalarType();
    firstFrameVolumeNumberOfComponents = firstFrameVolume->GetImageData()->GetNumberOfScalarComponents();
    firstFrameVolumeVoxelVectorType = firstFrameVolume->GetVoxelVectorType();
  }
  vtkNew<vtkMatrix4x4> firstVolumeRasToIjk;
  firstFrameVolume->GetRASToIJKMatrix(firstVolumeRasToIjk.GetPointer());

  int numberOfFrameVolumes = volSequenceNode->GetNumberOfDataNodes();
  for (int frameIndex = 1; frameIndex < numberOfFrameVolumes; frameIndex++)
  {
    vtkMRMLVolumeNode* currentFrameVolume = vtkMRMLVolumeNode::SafeDownCast(volSequenceNode->GetNthDataNode(frameIndex));
    if (currentFrameVolume == nullptr)
    {
      vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode: only volume nodes can be written (frame " << frameIndex << ")");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only volumes can be written in this format."));
      return false;
    }
    vtkNew<vtkMatrix4x4> currentVolumeRasToIjk;
    currentFrameVolume->GetRASToIJKMatrix(currentVolumeRasToIjk.GetPointer());
    if (!vtkAddonMathUtilities::MatrixAreEqual(currentVolumeRasToIjk, firstVolumeRasToIjk))
    {
      vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode: IJK to RAS matrix is not the same in all frames"
                    << " (first frame: " << vtkAddonMathUtilities::ToString(firstVolumeRasToIjk) << ", frame " << frameIndex << ": "
                    << vtkAddonMathUtilities::ToString(firstVolumeRasToIjk) << ")");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Geometry of all volumes in the sequence must be the same."));
      return false;
    }
    int currentFrameVolumeExtent[6] = { 0, -1, 0, -1, 0, -1 };
    int currentFrameVolumeScalarType = VTK_VOID;
    int currentFrameVolumeNumberOfComponents = 0;
    int currentFrameVolumeVoxelVectorType = vtkMRMLVolumeNode::VoxelVectorTypeUndefined;
    if (currentFrameVolume->GetImageData())
    {
      currentFrameVolume->GetImageData()->GetExtent(currentFrameVolumeExtent);
      currentFrameVolumeScalarType = currentFrameVolume->GetImageData()->GetScalarType();
      currentFrameVolumeNumberOfComponents = currentFrameVolume->GetImageData()->GetNumberOfScalarComponents();
      currentFrameVolumeVoxelVectorType = currentFrameVolume->GetVoxelVectorType();
    }
    for (int i = 0; i < 6; i++)
    {
      if (firstFrameVolumeExtent[i] != currentFrameVolumeExtent[i])
      {
        vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode: extent mismatch (frame " << frameIndex << ")");
        this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Extent of all volumes in the sequence must be the same."));
        return false;
      }
    }
    if (currentFrameVolumeScalarType != firstFrameVolumeScalarType)
    {
      vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode: scalar type mismatch (frame " << frameIndex << ")");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Scalar type of all volumes in the sequence must be the same."));
      return false;
    }
    if (currentFrameVolumeNumberOfComponents != firstFrameVolumeNumberOfComponents)
    {
      vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode: number of components mismatch (frame " << frameIndex << ")");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Number of components of all volumes in the sequence must be the same."));
      return false;
    }
    if (currentFrameVolumeVoxelVectorType != firstFrameVolumeVoxelVectorType)
    {
      vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode: voxel vector type mismatch (frame " << frameIndex << ")");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Voxel vector type of all volumes in the sequence must be the same."));
      return false;
    }
  }

  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLVolumeSequenceStorageNode::WriteDataInternal(vtkMRMLNode* refNode)
{
  vtkMRMLSequenceNode* volSequenceNode = vtkMRMLSequenceNode::SafeDownCast(refNode);
  if (volSequenceNode == nullptr)
  {
    vtkDebugMacro(<< "vtkMRMLVolumeSequenceStorageNode::WriteDataInternal: Do not recognize node type " << refNode->GetClassName());
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only sequence nodes can be written in this format."));
    return 0;
  }

  vtkNew<vtkMatrix4x4> firstVolumeRasToIjk;
  int frameVolumeDimensions[3] = { 0 };
  int frameVolumeScalarType = VTK_VOID;
  int frameVolumeNumberOfComponents = 0;
  int frameVolumeVoxelVectorType = vtkMRMLVolumeNode::VoxelVectorTypeUndefined;
  int numberOfFrameVolumes = volSequenceNode->GetNumberOfDataNodes();
  if (numberOfFrameVolumes > 0)
  {
    vtkMRMLVolumeNode* frameVolume = vtkMRMLVolumeNode::SafeDownCast(volSequenceNode->GetNthDataNode(0));
    if (frameVolume == nullptr)
    {
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only volume sequence can be written in this format."));
      return 0;
    }
    frameVolume->GetRASToIJKMatrix(firstVolumeRasToIjk.GetPointer());
    if (frameVolume->GetImageData())
    {
      frameVolume->GetImageData()->GetDimensions(frameVolumeDimensions);
      frameVolumeScalarType = frameVolume->GetImageData()->GetScalarType();
      frameVolumeNumberOfComponents = frameVolume->GetImageData()->GetNumberOfScalarComponents();
      frameVolumeVoxelVectorType = frameVolume->GetVoxelVectorType();
    }
  }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string(""))
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("File name not specified."));
    return 0;
  }

  vtkNew<vtkITKImageSequenceWriter> writer;
  writer->SetFileName(fullName.c_str());
  writer->SetUseCompression(this->GetUseCompression());
  writer->SetRasToIJKMatrix(firstVolumeRasToIjk.GetPointer());

  // Pass on voxel type to the writer (NRRD kind of first axis)
  writer->SetVoxelVectorType(vtkMRMLVolumeArchetypeStorageNode::ConvertVoxelVectorTypeMRMLToVTKITK(frameVolumeVoxelVectorType));

  // Pass down all MRML attributes to the file writer, including "DataNodeClassName", which is used to determine the type of the data node
  // when reading the sequence from file
  std::vector<std::string> attributeNames = volSequenceNode->GetAttributeNames();
  for (const std::string& attributeName : attributeNames)
  {
    const char* attributeValue = volSequenceNode->GetAttribute(attributeName.c_str());
    if (attributeValue)
    {
      writer->SetAttribute(attributeName.c_str(), attributeValue);
    }
  }
  // Set sequence axis label and unit
  const unsigned int sequenceAxisIndex = 3; // The fourth NRRD axis regardless the components, because the component axis does not count as real axis
  if (!volSequenceNode->GetIndexName().empty())
  {
    writer->SetAxisLabel(sequenceAxisIndex, volSequenceNode->GetIndexName().c_str());
  }
  if (!volSequenceNode->GetIndexUnit().empty())
  {
    writer->SetAxisUnit(sequenceAxisIndex, volSequenceNode->GetIndexUnit().c_str());
  }
  // Set index information
  if (!volSequenceNode->GetIndexTypeAsString().empty())
  {
    std::stringstream ssAttributeName;
    ssAttributeName << "axis " << sequenceAxisIndex << " index type";
    writer->SetAttribute(ssAttributeName.str(), volSequenceNode->GetIndexTypeAsString());
  }
  if (numberOfFrameVolumes > 0)
  {
    std::stringstream ssIndexValues;
    for (int frameIndex = 0; frameIndex < numberOfFrameVolumes; frameIndex++)
    {
      if (frameIndex > 0)
      {
        ssIndexValues << " ";
      }
      // Encode string to make sure there are no spaces in the serialized index value (space is used as separator)
      ssIndexValues << vtkMRMLNode::URLEncodeString(volSequenceNode->GetNthIndexValue(frameIndex).c_str());
    }
    std::stringstream ssAttributeName;
    ssAttributeName << "axis " << sequenceAxisIndex << " index values";
    writer->SetAttribute(ssAttributeName.str(), ssIndexValues.str());
  }

  // Setup writer
  for (int frameIndex = 0; frameIndex < numberOfFrameVolumes; frameIndex++)
  {
    vtkMRMLVolumeNode* frameVolume = vtkMRMLVolumeNode::SafeDownCast(volSequenceNode->GetNthDataNode(frameIndex));
    if (frameVolume == nullptr)
    {
      vtkDebugMacro(<< "vtkMRMLVolumeSequenceStorageNode::WriteDataInternal: Data node " << frameIndex << " is not a volume");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only volume sequence can be written in this format."));
      return 0;
    }
    vtkNew<vtkMatrix4x4> currentVolumeRasToIjk;
    frameVolume->GetRASToIJKMatrix(currentVolumeRasToIjk.GetPointer());
    if (!vtkAddonMathUtilities::MatrixAreEqual(currentVolumeRasToIjk, firstVolumeRasToIjk))
    {
      vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode: IJK to RAS matrix is not the same in all frames"
                    << " (first frame: " << vtkAddonMathUtilities::ToString(firstVolumeRasToIjk) << ", frame " << frameIndex << ": "
                    << vtkAddonMathUtilities::ToString(firstVolumeRasToIjk) << ")");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Geometry of all volumes in the sequence must be the same."));
      return 0;
    }
    int currentFrameVolumeDimensions[3] = { 0 };
    int currentFrameVolumeScalarType = VTK_VOID;
    if (frameVolume->GetImageData())
    {
      frameVolume->GetImageData()->GetDimensions(currentFrameVolumeDimensions);
      currentFrameVolumeScalarType = frameVolume->GetImageData()->GetScalarType();
    }
    if (currentFrameVolumeDimensions[0] != frameVolumeDimensions[0] || currentFrameVolumeDimensions[1] != frameVolumeDimensions[1]
        || currentFrameVolumeDimensions[2] != frameVolumeDimensions[2] || currentFrameVolumeScalarType != frameVolumeScalarType)
    {
      vtkDebugMacro(<< "vtkMRMLVolumeSequenceStorageNode::WriteDataInternal: Data node " << frameIndex << " size or scalar type mismatch ("
                    << "got " << currentFrameVolumeDimensions[0] << "x" << currentFrameVolumeDimensions[1] << "x" << currentFrameVolumeDimensions[2] << " "
                    << vtkImageScalarTypeNameMacro(currentFrameVolumeScalarType) << ", "
                    << "expected " << frameVolumeDimensions[0] << "x" << frameVolumeDimensions[1] << "x" << frameVolumeDimensions[2] << " "
                    << vtkImageScalarTypeNameMacro(frameVolumeScalarType));
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Size and scalar type of all volumes in the sequence must be the same."));
      return 0;
    }
    if (frameVolume->GetImageData())
    {
      writer->AddInputConnection(frameVolume->GetImageDataConnection());
    }
  }

  // Write image
  writer->Write();
  int writeFlag = 1;
  if (writer->GetErrorCode())
  {
    vtkDebugMacro("ERROR writing NRRD file " << (writer->GetFileName() == nullptr ? "null" : writer->GetFileName()));
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Failed to write NRRD file."));
    writeFlag = 0;
  }

  this->StageWriteData(refNode);

  vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode::WriteDataInternal: sequence successfully written.");
  return writeFlag;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeSequenceStorageNode::InitializeSupportedReadFileTypes()
{
  //: File format name
  std::string fileType = vtkMRMLTr("vtkMRMLVolumeSequenceStorageNode", "Volume Sequence");
  this->SupportedReadFileTypes->InsertNextValue(fileType + " (.seq.nrrd)");
  this->SupportedReadFileTypes->InsertNextValue(fileType + " (.seq.nhdr)");
  this->SupportedReadFileTypes->InsertNextValue(fileType + " (.nrrd)");
  this->SupportedReadFileTypes->InsertNextValue(fileType + " (.nhdr)");
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeSequenceStorageNode::InitializeSupportedWriteFileTypes()
{
  //: File format name
  std::string fileType = vtkMRMLTr("vtkMRMLVolumeSequenceStorageNode", "Volume Sequence");
  this->SupportedWriteFileTypes->InsertNextValue(fileType + " (.seq.nrrd)");
  this->SupportedWriteFileTypes->InsertNextValue(fileType + " (.seq.nhdr)");
  this->SupportedWriteFileTypes->InsertNextValue(fileType + " (.nrrd)");
  this->SupportedWriteFileTypes->InsertNextValue(fileType + " (.nhdr)");
}

//----------------------------------------------------------------------------
const char* vtkMRMLVolumeSequenceStorageNode::GetDefaultWriteFileExtension()
{
  return "seq.nrrd";
}
