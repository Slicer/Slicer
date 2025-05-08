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
bool vtkMRMLTransformSequenceStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
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
  reader->Update();  // Read first frame. This will also set NumberOfFrames
  if (reader->GetErrorCode() != vtkErrorCode::NoError)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(), "vtkMRMLTransformSequenceStorageNode::ReadDataInternal",
      "Error reading file.");
    return 0;
  }

  const char* sequenceAxisLabel = "frame";
  const char* sequenceAxisUnit = "";

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
      vtkErrorMacro("vtkMRMLTransformSequenceStorageNode::ReadDataInternal: invalid image data");
      return 0;
    }

    // Create a grid transform node
    vtkNew<vtkMRMLGridTransformNode> frameTransform;
    vtkNew<vtkOrientedGridTransform> gridTransform;

    // Set up the grid transform with the image data
    gridTransform->SetDisplacementGridData(frameImage);

    // Set the transform in the transform node
    frameTransform->SetAndObserveTransformToParent(gridTransform.GetPointer());

    std::ostringstream indexStr;
    indexStr << frameIndex << std::ends;

    std::ostringstream nameStr;
    nameStr << refNode->GetName() << "_" << std::setw(4) << std::setfill('0') << frameIndex << std::ends;
    frameTransform->SetName(nameStr.str().c_str());
    seqNode->SetDataNodeAtValue(frameTransform.GetPointer(), indexStr.str().c_str());
  }

  vtkDebugMacro(<< " vtkMRMLTransformSequenceStorageNode::ReadDataInternal: sequence successfully read. ");

  // success
  return 1;
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformSequenceStorageNode::CanWriteFromReferenceNode(vtkMRMLNode *refNode)
{
  vtkMRMLSequenceNode* seqNode = vtkMRMLSequenceNode::SafeDownCast(refNode);
  if (seqNode == nullptr)
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only sequence nodes can be written in this format."));
    return false;
  }

  // Get the first frame to determine geometry
  vtkMRMLTransformNode* firstFrameTransform = vtkMRMLTransformNode::SafeDownCast(seqNode->GetNthDataNode(0));
  if (firstFrameTransform == nullptr)
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only transform nodes can be written in this format."));
    return false;
  }

  // Convert first transform to grid transform to get geometry
  vtkOrientedGridTransform* firstGridTransform = vtkOrientedGridTransform::SafeDownCast(
    firstFrameTransform->GetTransformToParentAs("vtkOrientedGridTransform",
    false /* don't report conversion error */,
    true /* we would like to modify the transform */));
  if (firstGridTransform == nullptr)
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only grid transforms can be written in this format."));
    return false;
  }

  // Get the displacement field from the grid transform
  vtkImageData* firstDisplacementField = firstGridTransform->GetDisplacementGrid();
  if (firstDisplacementField == nullptr)
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Invalid grid transform: missing displacement field."));
    return false;
  }

  // Get geometry from first frame
  vtkMatrix4x4* firstGridDirection = firstGridTransform->GetGridDirectionMatrix();
  int firstDimensions[3] = {0};
  firstDisplacementField->GetDimensions(firstDimensions);
  int firstScalarType = firstDisplacementField->GetScalarType();
  unsigned int numberOfNonIdentityGridTransforms = 0;

  // Check all frames
  int numberOfFrames = seqNode->GetNumberOfDataNodes();
  for (int frameIndex = 1; frameIndex < numberOfFrames; frameIndex++)
  {
    vtkMRMLTransformNode* frameTransform = vtkMRMLTransformNode::SafeDownCast(seqNode->GetNthDataNode(frameIndex));
    if (frameTransform == nullptr)
    {
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only transform nodes can be written in this format."));
      return false;
    }

    // Convert transform to grid transform
    vtkOrientedGridTransform* frameGridTransform = vtkOrientedGridTransform::SafeDownCast(
      frameTransform->GetTransformToParentAs("vtkOrientedGridTransform",
      false /* don't report conversion error */,
      true /* we would like to modify the transform */));
    if (frameGridTransform == nullptr)
    {
      // If the transform is linear, it does not prevent the sequence from saving, but will require
      // creating an identity grid transform volume for saving it into a single volumetric file.
      // Note: we do not support the case when the first transform in the sequence is linear, because
      // we need a reference grid transform to copy the geometry from when writing.
      if (frameTransform->IsA("vtkMRMLLinearTransformNode"))
      {
        continue;
      }
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only grid transforms can be written in this format."));
      return false;
    }

    // Get the displacement field
    vtkImageData* frameDisplacementField = frameGridTransform->GetDisplacementGrid();
    if (frameDisplacementField == nullptr)
    {
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Invalid grid transform: missing displacement field."));
      return false;
    }
    else
    {
      numberOfNonIdentityGridTransforms++;
    }

    // Check geometry matches first frame
    vtkMatrix4x4* frameGridDirection = frameGridTransform->GetGridDirectionMatrix();
    if (frameGridDirection != nullptr && firstGridDirection != nullptr
      && !vtkAddonMathUtilities::MatrixAreEqual(frameGridDirection, firstGridDirection))
    {
      vtkDebugMacro("vtkMRMLTransformSequenceStorageNode::CanWriteFromReferenceNode: Grid direction matrix is not the same in all frames"
        << " (first frame: " << vtkAddonMathUtilities::ToString(firstGridDirection)
        << ", frame " << frameIndex << ": " << vtkAddonMathUtilities::ToString(frameGridDirection) << ")");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Geometry of all transforms in the sequence must be the same."));
      return false;
    }

    int frameDimensions[3] = {0};
    frameDisplacementField->GetDimensions(frameDimensions);
    if (frameDimensions[0] != firstDimensions[0] ||
        frameDimensions[1] != firstDimensions[1] ||
        frameDimensions[2] != firstDimensions[2] ||
        frameDisplacementField->GetScalarType() != firstScalarType)
    {
      vtkDebugMacro("vtkMRMLTransformSequenceStorageNode::CanWriteFromReferenceNode: Size or scalar type mismatch (frame " << frameIndex << ")");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Size and scalar type of all transforms in the sequence must be the same."));
      return false;
    }
  } // for all frames

  // Sanity check to confirm that some of the transforms in the sequence contain non-identity grid transforms
  if (numberOfNonIdentityGridTransforms == 0)
  {
    this->GetUserMessages()->AddMessage(vtkCommand::WarningEvent, std::string("Sequence only contains linear transforms."));
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

  // Get the first frame to determine geometry
  vtkMRMLTransformNode* firstFrameTransform = vtkMRMLTransformNode::SafeDownCast(seqNode->GetNthDataNode(0));
  if (firstFrameTransform == nullptr)
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only transform nodes can be written in this format."));
    return 0;
  }

  // Convert first transform to grid transform to get geometry
  vtkNew<vtkOrientedGridTransform> firstGridTransform;
  vtkOrientedGridTransform* gridTransform = vtkOrientedGridTransform::SafeDownCast(
    firstFrameTransform->GetTransformToParentAs("vtkOrientedGridTransform",
    false /* don't report conversion error */,
    true /* we would like to modify the transform */));
  if (gridTransform == nullptr)
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
  vtkNew<vtkMatrix4x4> rasToIjkMatrixWithOriginAndSpacing;
  if (gridTransform->GetGridDirectionMatrix() != nullptr)
  {
    rasToIjkMatrixWithOriginAndSpacing->DeepCopy(gridTransform->GetGridDirectionMatrix());
  }
  double spacing[3];
  firstDisplacementField->GetSpacing(spacing);
  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      rasToIjkMatrixWithOriginAndSpacing->SetElement(i, j, rasToIjkMatrixWithOriginAndSpacing->GetElement(i, j) / spacing[j]);
    }
  }
  double origin[3];
  firstDisplacementField->GetOrigin(origin);
  for (int i = 0; i < 3; ++i)
  {
    rasToIjkMatrixWithOriginAndSpacing->SetElement(i, 3, rasToIjkMatrixWithOriginAndSpacing->GetElement(i, 3) - origin[i] / spacing[i]);
  }
  writer->SetRasToIJKMatrix(rasToIjkMatrixWithOriginAndSpacing);

  writer->SetVoxelVectorType(vtkITKImageSequenceWriter::VoxelVectorTypeSpatial);
  writer->SetIntentCode("1006"); // Set intent code indicating this is a transform (comes from Nifti heritage as a de facto standard)

  // Process each frame
  int numberOfFrames = seqNode->GetNumberOfDataNodes();
  for (int frameIndex = 0; frameIndex < numberOfFrames; frameIndex++)
  {
    vtkMRMLTransformNode* frameTransform = vtkMRMLTransformNode::SafeDownCast(seqNode->GetNthDataNode(frameIndex));
    if (frameTransform == nullptr)
    {
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Only transform nodes can be written in this format."));
      return 0;
    }

    // Convert transform to grid transform
    vtkOrientedGridTransform* frameGridTransform = vtkOrientedGridTransform::SafeDownCast(
      frameTransform->GetTransformToParentAs("vtkOrientedGridTransform", false, true));
    if (frameGridTransform == nullptr)
    {
      if (frameIndex == 0)
      {
        this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("First frame in the sequence needs to contain a valid grid transform."));
        return 0;
      }

      // Generate an identity displacement field to use as a placeholder for the linear transform
      std::ostringstream warningMessage;
      warningMessage << "Frame " << frameIndex << " contains linear transform. Generating identity displacement field placeholder.";
      this->GetUserMessages()->AddMessage(vtkCommand::WarningEvent, warningMessage.str());
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
int vtkMRMLTransformSequenceStorageNode::SupportedFileType(const char *fileName)
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
