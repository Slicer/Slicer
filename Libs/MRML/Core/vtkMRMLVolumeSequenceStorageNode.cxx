/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLI18N.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLVolumeSequenceStorageNode.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSequenceNode.h"

// vtkAddon includes
#include <vtkAddonMathUtilities.h>

// vtkITK includes
#include "vtkITKImageSequenceReader.h"
#include "vtkITKImageSequenceWriter.h"

// VTK includes
#include "vtkImageAppendComponents.h"
#include "vtkImageData.h"
#include "vtkImageExtractComponents.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include "vtkTeemNRRDReader.h"

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

  vtkNew<vtkITKImageSequenceReader> reader;
  reader->SetFileName(fullName.c_str());
  reader->Update();  // Read first frame. This will also set NumberOfFrames

  for (int frameIndex = 0; frameIndex < reader->GetNumberOfFrames(); ++frameIndex)
  {
    if (frameIndex > 0)
    {
      reader->SetCurrentFrameIndex(frameIndex);
      reader->Update();
    }
    vtkImageData* frameImage = reader->GetOutput();
    vtkNew<vtkMRMLVectorVolumeNode> frameVolume;
    frameVolume->SetAndObserveImageData(frameImage);
    //frameVolume->SetRASToIJKMatrix(reader->GetRasToIjkMatrix());  //TODO:!!!

    std::ostringstream indexStr;
    //if (static_cast<int>(indexValues.size()) > frameIndex)
    //{
    //  indexStr << indexValues[frameIndex] << std::ends;
    //}
    //else
    //{
      indexStr << frameIndex << std::ends;
    //}

    std::ostringstream nameStr;
    nameStr << refNode->GetName() << "_" << std::setw(4) << std::setfill('0') << frameIndex << std::ends;
    frameVolume->SetName( nameStr.str().c_str() );
    volSequenceNode->SetDataNodeAtValue(frameVolume.GetPointer(), indexStr.str().c_str() );
  }

  /*
  vtkNew<vtkTeemNRRDReader> reader;
  reader->SetFileName(fullName.c_str());

  // Check if this is a NRRD file that we can read
  if (!reader->CanReadFile(fullName.c_str()))
  {
    vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode: This is not a nrrd file");
    return 0;
  }

  // Set up reader
  if (this->CenterImage)
  {
    reader->SetUseNativeOriginOff();
  }
  else
  {
    reader->SetUseNativeOriginOn();
  }

  // Read the header to see if the NRRD file corresponds to the MRML Node
  reader->UpdateInformation();

  //// Read index information and custom attributes
  //std::vector< std::string > indexValues;
  //typedef std::vector<std::string> KeyVector;
  //KeyVector keys = reader->GetHeaderKeysVector();
  //int frameAxis = 0;
  //for ( KeyVector::iterator kit = keys.begin(); kit != keys.end(); ++kit)
  //{
  //  if (*kit == "axis 0 index type")
  //  {
  //    volSequenceNode->SetIndexTypeFromString(reader->GetHeaderValue(kit->c_str()));
  //  }
  //  else if (*kit == "axis 0 index values")
  //  {
  //    std::string indexValue;
  //    for (std::istringstream indexValueList(reader->GetHeaderValue(kit->c_str()));
  //      indexValueList >> indexValue;)
  //    {
  //      // Encode string to make sure there are no spaces in the serialized index value (space is used as separator)
  //      indexValues.push_back(vtkMRMLNode::URLDecodeString(indexValue.c_str()));
  //    }
  //  }
  //  else
  //  {
  //    volSequenceNode->SetAttribute(kit->c_str(), reader->GetHeaderValue(kit->c_str()));
  //  }
  //}

  const char* sequenceAxisLabel = "frame";
  //const char* sequenceAxisLabel = reader->GetAxisLabel(frameAxis);
  //volSequenceNode->SetIndexName(sequenceAxisLabel ? sequenceAxisLabel : "frame");
  const char* sequenceAxisUnit = "";
  //const char* sequenceAxisUnit = reader->GetAxisUnit(frameAxis);
  //volSequenceNode->SetIndexUnit(sequenceAxisUnit ? sequenceAxisUnit : "");

  // Read and copy the data to sequence of volume nodes
  reader->Update();
  // Copy image data to sequence of volume nodes
  vtkImageData* imageData = reader->GetOutput();
  if (imageData == nullptr || imageData->GetPointData()==nullptr || imageData->GetPointData()->GetScalars() == nullptr)
  {
    vtkErrorMacro("vtkMRMLVolumeSequenceStorageNode::ReadDataInternal: invalid image data");
    return 0;
  }
  int numberOfFrames = imageData->GetNumberOfScalarComponents();
  vtkNew<vtkImageExtractComponents> extractComponents;
  extractComponents->SetInputConnection(reader->GetOutputPort());

  vtkDebugMacro(<< " vtkMRMLVolumeSequenceStorageNode::ReadDataInternal: Starting reading sequence. ");
  for (int frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex)
  {
    vtkDebugMacro(<< " reading frame : "<<frameIndex);
    extractComponents->SetComponents(frameIndex);
    extractComponents->Update();
    vtkNew<vtkImageData> frameVoxels;
    frameVoxels->DeepCopy(extractComponents->GetOutput());

    // Slicer expects normalized image position and spacing
    frameVoxels->SetOrigin(0, 0, 0);
    frameVoxels->SetSpacing(1, 1, 1);
    vtkSmartPointer<vtkMRMLVolumeNode> frameVolume;
    if (dataNodeClassName.empty())
    {
      dataNodeClassName = "vtkMRMLScalarVolumeNode";
    }
    if (this->GetScene())
    {
      frameVolume = vtkSmartPointer<vtkMRMLVolumeNode>::Take(vtkMRMLVolumeNode::SafeDownCast(this->GetScene()->CreateNodeByClass(dataNodeClassName.c_str())));
    }
    else
    {
      vtkWarningMacro("vtkMRMLVolumeSequenceStorageNode::ReadDataInternal: Scene is not set.");
    }
    if (frameVolume == nullptr)
    {
      if (dataNodeClassName != "vtkMRMLScalarVolumeNode")
      {
        vtkErrorMacro("Requested DataNodeClass is " << dataNodeClassName << " but volume sequence will be read into vtkMRMLScalarVolumeNode.");
      }
      frameVolume = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
    }
    frameVolume->SetAndObserveImageData(frameVoxels.GetPointer());
    frameVolume->SetRASToIJKMatrix(reader->GetRasToIjkMatrix());

    std::ostringstream indexStr;
    if (static_cast<int>(indexValues.size()) > frameIndex)
    {
      indexStr << indexValues[frameIndex] << std::ends;
    }
    else
    {
      indexStr << frameIndex << std::ends;
    }

    std::ostringstream nameStr;
    nameStr << refNode->GetName() << "_" << std::setw(4) << std::setfill('0') << frameIndex << std::ends;
    frameVolume->SetName( nameStr.str().c_str() );
    volSequenceNode->SetDataNodeAtValue(frameVolume.GetPointer(), indexStr.str().c_str() );
  }

  vtkDebugMacro(<< " vtkMRMLVolumeSequenceStorageNode::ReadDataInternal: sequence successfully read. ");
  */

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
