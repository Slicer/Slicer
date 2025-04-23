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
#include "vtkMRMLVolumeSequenceStorageNode.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLVectorVolumeNode.h"

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
vtkMRMLVolumeSequenceStorageNode::vtkMRMLVolumeSequenceStorageNode() = default;

//----------------------------------------------------------------------------
vtkMRMLVolumeSequenceStorageNode::~vtkMRMLVolumeSequenceStorageNode() = default;

//----------------------------------------------------------------------------
int vtkMRMLVolumeSequenceStorageNode::ConvertVoxelVectorTypeMRMLToVTKITK(int mrmlType)
{
  switch (mrmlType)
  {
    case vtkMRMLVolumeNode::VoxelVectorTypeUndefined: return vtkITKImageSequenceWriter::VoxelVectorTypeUndefined;
    case vtkMRMLVolumeNode::VoxelVectorTypeSpatial: return vtkITKImageSequenceWriter::VoxelVectorTypeSpatial;
    case vtkMRMLVolumeNode::VoxelVectorTypeColorRGB: return vtkITKImageSequenceWriter::VoxelVectorTypeColorRGB;
    case vtkMRMLVolumeNode::VoxelVectorTypeColorRGBA: return vtkITKImageSequenceWriter::VoxelVectorTypeColorRGBA;
    default:
      return vtkITKImageSequenceWriter::VoxelVectorTypeUndefined;
  }
}

//----------------------------------------------------------------------------
int vtkMRMLVolumeSequenceStorageNode::ConvertVoxelVectorTypeVTKITKToMRML(int vtkitkType)
{
  switch (vtkitkType)
  {
    case vtkITKImageSequenceWriter::VoxelVectorTypeUndefined: return vtkMRMLVolumeNode::VoxelVectorTypeUndefined;
    case vtkITKImageSequenceWriter::VoxelVectorTypeSpatial: return vtkMRMLVolumeNode::VoxelVectorTypeSpatial;
    case vtkITKImageSequenceWriter::VoxelVectorTypeColorRGB: return vtkMRMLVolumeNode::VoxelVectorTypeColorRGB;
    case vtkITKImageSequenceWriter::VoxelVectorTypeColorRGBA: return vtkMRMLVolumeNode::VoxelVectorTypeColorRGBA;
    default:
      return vtkMRMLVolumeNode::VoxelVectorTypeUndefined;
  }
}

//----------------------------------------------------------------------------
bool vtkMRMLVolumeSequenceStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
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
  reader->Update(); // This will set NumberOfFrames
  if (reader->GetErrorCode() != vtkErrorCode::NoError)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLVolumeSequenceStorageNode::ReadDataInternal",
      "Error reading file.");
    return 0;
  }

  // Read all frames
  for (int frameIndex = 0; frameIndex < reader->GetNumberOfFrames(); ++frameIndex)
  {
    if (frameIndex > 0)
    {
      reader->SetCurrentFrameIndex(frameIndex);
      reader->Update();
    }
    vtkImageData* frameImage = reader->GetOutput();
    if (frameImage == nullptr || frameImage->GetPointData() == nullptr || frameImage->GetPointData()->GetScalars() == nullptr)
    {
      vtkErrorMacro("vtkMRMLVolumeSequenceStorageNode::ReadDataInternal: invalid image data");
      return 0;
    }

    // Create appropriate volume node based on number of components
    vtkMRMLVolumeNode* frameVolume = nullptr;
    if (frameImage->GetNumberOfScalarComponents() > 1)
    {
      frameVolume = vtkMRMLVectorVolumeNode::New();
    }
    else
    {
      frameVolume = vtkMRMLScalarVolumeNode::New();
    }

    // Copy origin and spacing from image data to volume node
    double origin[3], spacing[3];
    frameImage->GetOrigin(origin);
    frameImage->GetSpacing(spacing);
    frameVolume->SetOrigin(origin);
    frameVolume->SetSpacing(spacing);

    // Clear origin, spacing, and directions from image data since they are now in the volume node
    frameImage->SetOrigin(0.0, 0.0, 0.0);
    frameImage->SetSpacing(1.0, 1.0, 1.0);
    vtkNew<vtkMatrix3x3> identityDirections;
    frameImage->SetDirectionMatrix(identityDirections);

    // Set up the volume node
    frameVolume->SetAndObserveImageData(frameImage);
    frameVolume->SetRASToIJKMatrix(reader->GetRasToIjkMatrix());

    std::ostringstream indexStr;
    indexStr << frameIndex << std::ends;

    std::ostringstream nameStr;
    nameStr << refNode->GetName() << "_" << std::setw(4) << std::setfill('0') << frameIndex << std::ends;
    frameVolume->SetName(nameStr.str().c_str());
    volSequenceNode->SetDataNodeAtValue(frameVolume, indexStr.str().c_str());
    frameVolume->Delete();
  }

  // Read axis label and unit
  volSequenceNode->SetIndexName(reader->GetSequenceAxisLabel().empty() ? "frame" : reader->GetSequenceAxisLabel());
  volSequenceNode->SetIndexUnit(reader->GetSequenceAxisUnit().empty() ? "" : reader->GetSequenceAxisUnit());

  // Read custom attributes
  std::vector<std::string> indexValues;
  typedef std::vector<std::string> KeyVector;
  KeyVector keys = reader->GetHeaderKeysVector();
  int frameAxis = 0;
  for (KeyVector::iterator kit = keys.begin(); kit != keys.end(); ++kit)
  {
    volSequenceNode->SetAttribute(kit->c_str(), reader->GetHeaderValue(kit->c_str()));
  }

  vtkDebugMacro(<< " vtkMRMLVolumeSequenceStorageNode::ReadDataInternal: sequence successfully read. ");

  // success
  return 1;
}

//----------------------------------------------------------------------------
bool vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode(vtkMRMLNode *refNode)
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
  for (int frameIndex = 1; frameIndex<numberOfFrameVolumes; frameIndex++)
  {
    vtkMRMLVolumeNode* currentFrameVolume = vtkMRMLVolumeNode::SafeDownCast(volSequenceNode->GetNthDataNode(frameIndex));
    if (currentFrameVolume == nullptr)
    {
      vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode: only volume nodes can be written (frame "<<frameIndex<<")");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only volumes can be written in this format."));
      return false;
    }
    vtkNew<vtkMatrix4x4> currentVolumeRasToIjk;
    currentFrameVolume->GetRASToIJKMatrix(currentVolumeRasToIjk.GetPointer());
    if (!vtkAddonMathUtilities::MatrixAreEqual(currentVolumeRasToIjk, firstVolumeRasToIjk))
    {
      vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode: IJK to RAS matrix is not the same in all frames"
        << " (first frame: " << vtkAddonMathUtilities::ToString(firstVolumeRasToIjk)
        << ", frame "<<frameIndex<<": " << vtkAddonMathUtilities::ToString(firstVolumeRasToIjk) << ")");
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
    if (currentFrameVolumeVoxelVectorType  != firstFrameVolumeVoxelVectorType )
      {
      vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode: voxel vector type mismatch (frame " << frameIndex << ")");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Voxel vector type of all volumes in the sequence must be the same."));
      return false;
      }
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
  int frameVolumeDimensions[3] = {0};
  int frameVolumeScalarType = VTK_VOID;
  int frameVolumeNumberOfComponents = 0;
  int frameVolumeVoxelVectorType = vtkMRMLVolumeNode::VoxelVectorTypeUndefined;
  int numberOfFrameVolumes = volSequenceNode->GetNumberOfDataNodes();
  if (numberOfFrameVolumes > 0)
  {
    vtkMRMLVolumeNode* frameVolume = vtkMRMLVolumeNode::SafeDownCast(volSequenceNode->GetNthDataNode(0));
    if (frameVolume==nullptr)
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
  writer->SetVoxelVectorType(this->ConvertVoxelVectorTypeMRMLToVTKITK(frameVolumeVoxelVectorType));

  // Set attributes from sequence node
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
  writer->SetAxisLabel(sequenceAxisIndex, volSequenceNode->GetIndexName().c_str());
  writer->SetAxisUnit(sequenceAxisIndex, volSequenceNode->GetIndexUnit().c_str());

  // Setup writer
  for (int frameIndex=0; frameIndex<numberOfFrameVolumes; frameIndex++)
  {
    vtkMRMLVolumeNode* frameVolume = vtkMRMLVolumeNode::SafeDownCast(volSequenceNode->GetNthDataNode(frameIndex));
    if (frameVolume==nullptr)
    {
      vtkDebugMacro(<< "vtkMRMLVolumeSequenceStorageNode::WriteDataInternal: Data node "<<frameIndex<<" is not a volume");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only volume sequence can be written in this format."));
      return 0;
    }
    vtkNew<vtkMatrix4x4> currentVolumeRasToIjk;
    frameVolume->GetRASToIJKMatrix(currentVolumeRasToIjk.GetPointer());
    if (!vtkAddonMathUtilities::MatrixAreEqual(currentVolumeRasToIjk, firstVolumeRasToIjk))
    {
      vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode: IJK to RAS matrix is not the same in all frames"
        << " (first frame: " << vtkAddonMathUtilities::ToString(firstVolumeRasToIjk)
        << ", frame " << frameIndex << ": " << vtkAddonMathUtilities::ToString(firstVolumeRasToIjk) << ")");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Geometry of all volumes in the sequence must be the same."));
      return 0;
    }
    int currentFrameVolumeDimensions[3] = {0};
    int currentFrameVolumeScalarType = VTK_VOID;
    if (frameVolume->GetImageData())
    {
      frameVolume->GetImageData()->GetDimensions(currentFrameVolumeDimensions);
      currentFrameVolumeScalarType = frameVolume->GetImageData()->GetScalarType();
    }
    if (currentFrameVolumeDimensions[0] != frameVolumeDimensions[0]
      || currentFrameVolumeDimensions[1] != frameVolumeDimensions[1]
      || currentFrameVolumeDimensions[2] != frameVolumeDimensions[2]
      || currentFrameVolumeScalarType != frameVolumeScalarType)
    {
      vtkDebugMacro(<< "vtkMRMLVolumeSequenceStorageNode::WriteDataInternal: Data node "<<frameIndex<<" size or scalar type mismatch ("
        << "got " << currentFrameVolumeDimensions[0]
          << "x" << currentFrameVolumeDimensions[1]
          << "x" << currentFrameVolumeDimensions[2]
          << " " << vtkImageScalarTypeNameMacro(currentFrameVolumeScalarType) << ", "
        << "expected " << frameVolumeDimensions[0]
          << "x" << frameVolumeDimensions[1]
          << "x" << frameVolumeDimensions[2]
          << " " << vtkImageScalarTypeNameMacro(frameVolumeScalarType) );
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
