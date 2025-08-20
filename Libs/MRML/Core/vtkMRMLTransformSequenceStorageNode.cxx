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
#include "vtkMRMLTransformSequenceStorageNode.h"

#include "vtkMRMLGridTransformNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLVectorVolumeNode.h"

// vtkAddon includes
#include <vtkAddonMathUtilities.h>
#include <vtkOrientedGridTransform.h>

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
vtkMRMLNodeNewMacro(vtkMRMLTransformSequenceStorageNode);

//----------------------------------------------------------------------------
vtkMRMLTransformSequenceStorageNode::vtkMRMLTransformSequenceStorageNode() = default;

//----------------------------------------------------------------------------
vtkMRMLTransformSequenceStorageNode::~vtkMRMLTransformSequenceStorageNode() = default;

//----------------------------------------------------------------------------
bool vtkMRMLTransformSequenceStorageNode::CanReadInReferenceNode(vtkMRMLNode* refNode)
{
  return refNode->IsA("vtkMRMLSequenceNode");
}

//----------------------------------------------------------------------------
int vtkMRMLTransformSequenceStorageNode::ReadDataInternal(vtkMRMLNode* refNode)
{
  if (!this->CanReadInReferenceNode(refNode))
  {
    return 0;
  }

  vtkMRMLSequenceNode* seqNode = dynamic_cast<vtkMRMLSequenceNode*>(refNode);
  if (!seqNode)
  {
    vtkErrorMacro(<< "vtkMRMLTransformSequenceStorageNode::ReadDataInternal : not a Sequence node.");
    return 0;
  }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string(""))
  {
    vtkErrorMacro(<< "vtkMRMLTransformSequenceStorageNode::ReadDataInternal : File name not specified");
    return 0;
  }

  vtkNew<vtkITKImageSequenceReader> reader;
  reader->SetFileName(fullName.c_str());
  reader->Update(); // Read first frame. This will also set NumberOfFrames
  if (reader->GetErrorCode() != vtkErrorCode::NoError)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLTransformSequenceStorageNode::ReadDataInternal", "Error reading file.");
    return 0;
  }

  // Read custom attributes
  std::vector<std::string> indexValues;
  typedef std::vector<std::string> KeyVector;
  KeyVector keys = reader->GetHeaderKeysVector();
  for (KeyVector::iterator kit = keys.begin(); kit != keys.end(); ++kit)
  {
    if (*kit == "axis 3 index type")
    {
      seqNode->SetIndexTypeFromString(reader->GetHeaderValue(kit->c_str()));
    }
    else if (*kit == "axis 3 index values")
    {
      std::string indexValue;
      for (std::istringstream indexValueList(reader->GetHeaderValue(kit->c_str())); indexValueList >> indexValue;)
      {
        // Encode string to make sure there are no spaces in the serialized index value (space is used as separator)
        indexValues.push_back(vtkMRMLNode::URLDecodeString(indexValue.c_str()));
      }
    }
  }

  vtkNew<vtkMatrix4x4> ijkToRas;
  vtkMatrix4x4::Invert(reader->GetRasToIjkMatrix(), ijkToRas);

  double origin[3] = { ijkToRas->GetElement(0, 3), ijkToRas->GetElement(1, 3), ijkToRas->GetElement(2, 3) };
  double spacing[3] = { 1.0, 1.0, 1.0 };
  vtkNew<vtkMatrix3x3> directionMatrix3x3;
  vtkAddonMathUtilities::GetOrientationMatrix(ijkToRas, directionMatrix3x3);
  vtkAddonMathUtilities::NormalizeColumns(directionMatrix3x3, spacing);
  vtkNew<vtkMatrix4x4> directionMatrix4x4;
  vtkAddonMathUtilities::SetOrientationMatrix(directionMatrix3x3, directionMatrix4x4);

  for (int frameIndex = 0; frameIndex < reader->GetNumberOfCachedImages(); ++frameIndex)
  {
    vtkImageData* frameImage = reader->GetCachedImage(frameIndex);
    if (frameImage == nullptr || frameImage->GetPointData() == nullptr || frameImage->GetPointData()->GetScalars() == nullptr)
    {
      vtkErrorMacro("vtkMRMLTransformSequenceStorageNode::ReadDataInternal: invalid image data");
      return 0;
    }

    // Create a grid transform node
    vtkNew<vtkMRMLTransformNode> frameTransform;
    vtkNew<vtkOrientedGridTransform> gridTransform;

    // Set up the grid transform with the image data
    frameImage->SetOrigin(origin);
    frameImage->SetSpacing(spacing);
    gridTransform->SetDisplacementGridData(frameImage);
    gridTransform->SetGridDirectionMatrix(directionMatrix4x4);

    // Set the transform in the transform node
    frameTransform->SetAndObserveTransformFromParent(gridTransform.GetPointer());

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
    nameStr << refNode->GetName() << "_" << std::setw(4) << std::setfill('0') << frameIndex;

    frameTransform->SetName(nameStr.str().c_str());
    seqNode->SetDataNodeAtValue(frameTransform.GetPointer(), indexStr.str().c_str());
  }

  // Read axis label and unit
  seqNode->SetIndexName(reader->GetSequenceAxisLabel().empty() ? "frame" : reader->GetSequenceAxisLabel());
  seqNode->SetIndexUnit(reader->GetSequenceAxisUnit().empty() ? "" : reader->GetSequenceAxisUnit());

  vtkDebugMacro(<< " vtkMRMLTransformSequenceStorageNode::ReadDataInternal: sequence successfully read. ");

  // success
  return 1;
}

//----------------------------------------------------------------------------
vtkOrientedGridTransform* vtkMRMLTransformSequenceStorageNode::GetReferenceGridTransform(vtkMRMLSequenceNode* seqNode)
{
  vtkSmartPointer<vtkOrientedGridTransform> firstGridTransform;

  // Check all frames
  int numberOfFrames = seqNode->GetNumberOfDataNodes();
  for (int frameIndex = 0; frameIndex < numberOfFrames; frameIndex++)
  {
    vtkMRMLTransformNode* frameTransform = vtkMRMLTransformNode::SafeDownCast(seqNode->GetNthDataNode(frameIndex));
    if (frameTransform == nullptr)
    {
      vtkDebugMacro("GetReferenceGridTransform: Only transform nodes can be written in this format.");
      return nullptr;
    }

    // Convert transform to grid transform
    vtkOrientedGridTransform* frameGridTransform = vtkOrientedGridTransform::SafeDownCast( //
      frameTransform->GetTransformFromParentAs("vtkOrientedGridTransform",
                                               false, // don't report conversion error
                                               true   // we would like to modify the transform
                                               ));
    if (frameGridTransform == nullptr)
    {
      // If the transform is linear, it does not prevent the sequence from saving, but will require
      // creating an identity grid transform volume for saving it into a single volumetric file.
      // Note: we do not support the case when the first transform in the sequence is linear, because
      // we need a reference grid transform to copy the geometry from when writing.
      if (frameTransform->IsLinear())
      {
        vtkNew<vtkMatrix4x4> linearTransformMatrix;
        frameTransform->GetMatrixTransformFromParent(linearTransformMatrix);
        vtkNew<vtkMatrix4x4> identityMatrix;
        if (vtkAddonMathUtilities::MatrixAreEqual(linearTransformMatrix, identityMatrix))
        {
          // Identity linear transform can be written as an empty grid transform
          continue;
        }
      }
      vtkDebugMacro("GetReferenceGridTransform: Only grid or identity transform can be written in this format.");
      return nullptr;
    }

    // Get the displacement field
    vtkImageData* frameDisplacementGrid = frameGridTransform->GetDisplacementGrid();
    if (!frameDisplacementGrid)
    {
      vtkDebugMacro("GetReferenceGridTransform: Invalid grid transform: missing displacement field.");
      return nullptr;
    }

    if (!firstGridTransform.GetPointer())
    {
      // This is the first grid transform. It will be used as reference.
      firstGridTransform = frameGridTransform;
      continue;
    }

    vtkSmartPointer<vtkMatrix4x4> firstGridDirection = firstGridTransform->GetGridDirectionMatrix();
    vtkSmartPointer<vtkMatrix4x4> frameGridDirection = frameGridTransform->GetGridDirectionMatrix();
    if (!firstGridDirection)
    {
      firstGridDirection = vtkSmartPointer<vtkMatrix4x4>::New();
    }
    if (!frameGridDirection)
    {
      frameGridDirection = vtkSmartPointer<vtkMatrix4x4>::New();
    }
    if (!vtkAddonMathUtilities::MatrixAreEqual(frameGridDirection, firstGridDirection))
    {
      vtkDebugMacro("GetReferenceGridTransform: Grid direction matrix is not the same in all frames" << " (first frame: " << vtkAddonMathUtilities::ToString(firstGridDirection)
                                                                                                     << ", frame " << frameIndex << ": "
                                                                                                     << vtkAddonMathUtilities::ToString(frameGridDirection) << ")");
      return nullptr;
    }

    vtkImageData* firstDisplacementGrid = firstGridTransform->GetDisplacementGrid();

    int firstDimensions[3]{};
    firstDisplacementGrid->GetDimensions(firstDimensions);
    int frameDimensions[3]{};
    frameDisplacementGrid->GetDimensions(frameDimensions);
    if (frameDimensions[0] != firstDimensions[0]    //
        || frameDimensions[1] != firstDimensions[1] //
        || frameDimensions[2] != firstDimensions[2] //
        || frameDisplacementGrid->GetScalarType() != firstDisplacementGrid->GetScalarType())
    {
      vtkDebugMacro("GetReferenceGridTransform: Size or scalar type mismatch (frame " << frameIndex << ")");
      return nullptr;
    }

    double tolerance = 1e-6; // Tolerance for comparing floating-point numbers

    double firstOrigin[3]{};
    firstDisplacementGrid->GetOrigin(firstOrigin);
    double frameOrigin[3]{};
    frameDisplacementGrid->GetOrigin(frameOrigin);
    if (std::abs(frameOrigin[0] - firstOrigin[0]) > tolerance    //
        || std::abs(frameOrigin[1] - firstOrigin[1]) > tolerance //
        || std::abs(frameOrigin[2] - firstOrigin[2]) > tolerance)
    {
      vtkDebugMacro("GetReferenceGridTransform: Origin mismatch (frame " << frameIndex << ")");
      return nullptr;
    }

    double firstSpacing[3]{};
    firstDisplacementGrid->GetSpacing(firstSpacing);
    double frameSpacing[3]{};
    frameDisplacementGrid->GetSpacing(frameSpacing);
    if (std::abs(frameSpacing[0] - firstSpacing[0]) > tolerance    //
        || std::abs(frameSpacing[1] - firstSpacing[1]) > tolerance //
        || std::abs(frameSpacing[2] - firstSpacing[2]) > tolerance)
    {
      vtkDebugMacro("GetReferenceGridTransform: Spacing mismatch (frame " << frameIndex << ")");
      return nullptr;
    }

  } // for all frames

  return firstGridTransform;
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformSequenceStorageNode::CanWriteFromReferenceNode(vtkMRMLNode* refNode)
{
  vtkMRMLSequenceNode* seqNode = vtkMRMLSequenceNode::SafeDownCast(refNode);
  if (seqNode == nullptr)
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only sequence nodes can be written in this format."));
    return false;
  }

  if (!this->GetReferenceGridTransform(seqNode))
  {
    this->GetUserMessages()->AddMessage(vtkCommand::WarningEvent, std::string("Sequence cannot be saved with this storage node, it does not contain grid transforms."));
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformSequenceStorageNode::WriteDataInternal(vtkMRMLNode* refNode)
{
  vtkMRMLSequenceNode* seqNode = vtkMRMLSequenceNode::SafeDownCast(refNode);
  if (seqNode == nullptr)
  {
    vtkDebugMacro(<< "vtkMRMLTransformSequenceStorageNode::WriteDataInternal: Do not recognize node type " << refNode->GetClassName());
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only sequence nodes can be written in this format."));
    return 0;
  }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string(""))
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("File name is not specified."));
    return 0;
  }

  // Create a writer for the sequence
  vtkNew<vtkITKImageSequenceWriter> writer;
  writer->SetFileName(fullName.c_str());
  writer->SetUseCompression(this->GetUseCompression());

  // Convert transform to grid transform
  vtkOrientedGridTransform* gridTransform = this->GetReferenceGridTransform(seqNode);
  if (!gridTransform)
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only grid transforms can be written in this format."));
    return 0;
  }

  // Get the displacement field from the grid transform
  vtkImageData* firstDisplacementField = gridTransform->GetDisplacementGrid();
  if (firstDisplacementField == nullptr)
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Invalid grid transform: missing displacement field."));
    return 0;
  }

  // Set up the writer with the geometry from the first frame
  vtkNew<vtkMatrix4x4> ijkToRas;
  if (gridTransform->GetGridDirectionMatrix() != nullptr)
  {
    ijkToRas->DeepCopy(gridTransform->GetGridDirectionMatrix());
  }
  double spacing[3] = { 1.0, 1.0, 1.0 };
  firstDisplacementField->GetSpacing(spacing);
  double origin[3] = { 0.0, 0.0, 0.0 };
  firstDisplacementField->GetOrigin(origin);
  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      ijkToRas->SetElement(i, j, ijkToRas->GetElement(i, j) * spacing[j]);
    }
    ijkToRas->SetElement(i, 3, origin[i]);
  }
  vtkNew<vtkMatrix4x4> rasToIjk;
  vtkMatrix4x4::Invert(ijkToRas, rasToIjk);

  writer->SetRasToIJKMatrix(rasToIjk);

  writer->SetVoxelVectorType(vtkITKImageWriter::VoxelVectorTypeSpatial);
  writer->SetIntentCode("1006"); // Set intent code indicating this is a transform (comes from Nifti heritage as a de facto standard)

  // Set sequence axis label and unit
  const unsigned int sequenceAxisIndex = 3; // The fourth NRRD axis regardless the components, because the component axis does not count as real axis
  writer->SetAxisLabel(sequenceAxisIndex, seqNode->GetIndexName().c_str());
  writer->SetAxisUnit(sequenceAxisIndex, seqNode->GetIndexUnit().c_str());
  // Set index information
  if (!seqNode->GetIndexTypeAsString().empty())
  {
    std::stringstream ssAttributeName;
    ssAttributeName << "axis " << sequenceAxisIndex << " index type";
    writer->SetAttribute(ssAttributeName.str(), seqNode->GetIndexTypeAsString());
  }
  int numberOfFrames = seqNode->GetNumberOfDataNodes();
  if (numberOfFrames > 0)
  {
    std::stringstream ssIndexValues;
    for (int frameIndex = 0; frameIndex < numberOfFrames; frameIndex++)
    {
      if (frameIndex > 0)
      {
        ssIndexValues << " ";
      }
      // Encode string to make sure there are no spaces in the serialized index value (space is used as separator)
      ssIndexValues << vtkMRMLNode::URLEncodeString(seqNode->GetNthIndexValue(frameIndex).c_str());
    }
    std::stringstream ssAttributeName;
    ssAttributeName << "axis " << sequenceAxisIndex << " index values";
    writer->SetAttribute(ssAttributeName.str(), ssIndexValues.str());
  }

  // Process each frame
  for (int frameIndex = 0; frameIndex < numberOfFrames; frameIndex++)
  {
    vtkMRMLTransformNode* frameTransform = vtkMRMLTransformNode::SafeDownCast(seqNode->GetNthDataNode(frameIndex));
    if (frameTransform == nullptr)
    {
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only transform nodes can be written in this format."));
      return 0;
    }

    // Convert transform to grid transform
    vtkOrientedGridTransform* frameGridTransform = vtkOrientedGridTransform::SafeDownCast( //
      frameTransform->GetTransformFromParentAs("vtkOrientedGridTransform", false, true));
    if (frameGridTransform == nullptr)
    {
      if (frameIndex == 0)
      {
        this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("First frame in the sequence needs to contain a valid grid transform."));
        return 0;
      }

      // Generate an identity displacement field to use as a placeholder for the linear transform
      vtkWarningMacro("Frame " << frameIndex << " contains linear transform. Generating identity displacement field placeholder.");
      vtkNew<vtkImageData> identityDisplacementField;
      identityDisplacementField->CopyStructure(firstDisplacementField);
      identityDisplacementField->AllocateScalars(firstDisplacementField->GetScalarType(), firstDisplacementField->GetNumberOfScalarComponents());
      identityDisplacementField->GetPointData()->GetScalars()->Fill(0.0); // Fill all voxels with zeros
      // Add the identity displacement field to the writer
      writer->AddInputData(identityDisplacementField);
      continue;
    }

    // Get the displacement field
    vtkImageData* frameDisplacementField = frameGridTransform->GetDisplacementGrid();
    if (frameDisplacementField == nullptr)
    {
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Invalid grid transform: missing displacement field."));
      return 0;
    }

    // Add the displacement field to the writer
    writer->AddInputData(frameDisplacementField);
  }

  // Write the sequence
  writer->Write();
  int writeFlag = 1;
  if (writer->GetErrorCode())
  {
    vtkDebugMacro("ERROR writing NRRD file " << (writer->GetFileName() == nullptr ? "null" : writer->GetFileName()));
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Failed to write NRRD file."));
    writeFlag = 0;
  }

  this->StageWriteData(refNode);

  vtkDebugMacro("vtkMRMLTransformSequenceStorageNode::WriteDataInternal: sequence successfully written.");
  return writeFlag;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformSequenceStorageNode::InitializeSupportedReadFileTypes()
{
  //: File format name
  std::string fileType = vtkMRMLTr("vtkMRMLTransformSequenceStorageNode", "Grid Transform Sequence");
  this->SupportedReadFileTypes->InsertNextValue(fileType + " (.seq.nrrd)");
  this->SupportedReadFileTypes->InsertNextValue(fileType + " (.seq.nhdr)");
  this->SupportedReadFileTypes->InsertNextValue(fileType + " (.nrrd)");
  this->SupportedReadFileTypes->InsertNextValue(fileType + " (.nhdr)");
}

//----------------------------------------------------------------------------
void vtkMRMLTransformSequenceStorageNode::InitializeSupportedWriteFileTypes()
{
  //: File format name
  std::string fileType = vtkMRMLTr("vtkMRMLTransformSequenceStorageNode", "Grid Transform Sequence");
  this->SupportedWriteFileTypes->InsertNextValue(fileType + " (.seq.nrrd)");
  this->SupportedWriteFileTypes->InsertNextValue(fileType + " (.seq.nhdr)");
  this->SupportedWriteFileTypes->InsertNextValue(fileType + " (.nrrd)");
  this->SupportedWriteFileTypes->InsertNextValue(fileType + " (.nhdr)");
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransformSequenceStorageNode::GetDefaultWriteFileExtension()
{
  return "seq.nrrd";
}

//----------------------------------------------------------------------------
int vtkMRMLTransformSequenceStorageNode::SupportedFileType(const char* fileName)
{
  if (Superclass::SupportedFileType(fileName) == 0)
  {
    return 0; // The file extension is not supported so surely not acceptable
  }

  std::ifstream file(fileName, std::ios::in);
  if (!file.is_open())
  {
    // Unable to open the file, probably fileName did not contain full path, only the file name.
    // See SupportedFileType call in vtkSlicerSequencesLogic::AddSequence method after creating the
    // storage node. That call is superfluous and it is ambiguous that one time the full path
    // is passed, then only the file name.
    return 1;
  }

  // Read the first 800 characters of the file
  std::string content(800, '\0');
  file.read(&content[0], 800);
  content.resize(file.gcount()); // Resize to actual number of characters read

  // Look for "intent_code:=1006"
  if (content.find("intent_code:=1006") != std::string::npos)
  {
    return 1; // File is supported
  }

  return 0; // File is not supported
}
