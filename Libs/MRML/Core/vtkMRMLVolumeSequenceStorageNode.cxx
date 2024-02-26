/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#include <algorithm>

#include <vtkAddonMathUtilities.h>

#include "vtkMRMLI18N.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLVolumeSequenceStorageNode.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLVectorVolumeNode.h"

#include "vtkTeemNRRDReader.h"
#include "vtkTeemNRRDWriter.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#ifndef NRRD_CHUNK_IO_AVAILABLE
# include "vtkImageAppendComponents.h"
#endif
#include "vtkImageExtractComponents.h"
#include "vtkNew.h"
#include "vtkStringArray.h"
#include "vtksys/SystemTools.hxx"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLVolumeSequenceStorageNode);

//----------------------------------------------------------------------------
vtkMRMLVolumeSequenceStorageNode::vtkMRMLVolumeSequenceStorageNode() = default;

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

  vtkNew<vtkTeemNRRDReader> reader;
  reader->SetFileName(fullName.c_str());

  // Check if this is a NRRD file that we can read
  if (!reader->CanReadFile(fullName.c_str()))
  {
    vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode: This is not a nrrd file");
    return 0;
  }

#ifdef NRRD_CHUNK_IO_AVAILABLE
  // Set Read Multiple Images on.
  // If the data are compressed, this will return false and
  // the data will be read as a single multi-component image
  bool readAsMultipleImagesOn = reader->ReadImageListAsMultipleImagesOn();
#endif

  // Set up reader
  if (this->CenterImage)
  {
    reader->SetUseNativeOriginOff();
  }
  else
  {
    reader->SetUseNativeOriginOn();
  }

  // Read the header to see if the NRRD file corresponds to the
  // MRML Node
  reader->UpdateInformation();

  // Read index information and custom attributes
  std::vector<std::string> indexValues;
  typedef std::vector<std::string> KeyVector;
  KeyVector keys = reader->GetHeaderKeysVector();
  int frameAxis = 0;
  for (KeyVector::iterator kit = keys.begin(); kit != keys.end(); ++kit)
  {
#ifdef NRRD_CHUNK_IO_AVAILABLE
    if (*kit == "axis 0 index type")
    {
      volSequenceNode->SetIndexTypeFromString(reader->GetHeaderValue(kit->c_str()));
      frameAxis = 0;
    }
    else if (*kit == "axis 3 index type")
    {
      volSequenceNode->SetIndexTypeFromString(reader->GetHeaderValue(kit->c_str()));
      frameAxis = 3;
    }
    else if (*kit == "axis 0 index values" || *kit == "axis 3 index values")
    {
      std::string indexValue;
      for (std::istringstream indexValueList(reader->GetHeaderValue(kit->c_str())); indexValueList >> indexValue;)
      {
        // Encode string to make sure there are no spaces in the serialized index value (space is used as separator)
        indexValues.push_back(vtkMRMLNode::URLDecodeString(indexValue.c_str()));
      }
#else
    if (*kit == "axis 0 index type")
    {
      volSequenceNode->SetIndexTypeFromString(reader->GetHeaderValue(kit->c_str()));
    }
    else if (*kit == "axis 0 index values")
    {
      std::string indexValue;
      for (std::istringstream indexValueList(reader->GetHeaderValue(kit->c_str())); indexValueList >> indexValue;)
      {
        // Encode string to make sure there are no spaces in the serialized index value (space is used as separator)
        indexValues.push_back(vtkMRMLNode::URLDecodeString(indexValue.c_str()));
      }
#endif
    }
    else
    {
      volSequenceNode->SetAttribute(kit->c_str(), reader->GetHeaderValue(kit->c_str()));
    }
  }

  const char* sequenceAxisLabel = reader->GetAxisLabel(frameAxis);
  volSequenceNode->SetIndexName(sequenceAxisLabel ? sequenceAxisLabel : "frame");
  const char* sequenceAxisUnit = reader->GetAxisUnit(frameAxis);
  volSequenceNode->SetIndexUnit(sequenceAxisUnit ? sequenceAxisUnit : "");

  // Read and copy the data to sequence of volume nodes
#ifdef NRRD_CHUNK_IO_AVAILABLE
  int numberOfFrames = reader->GetNumberOfImages();
  vtkImageData* imageData = nullptr;
  vtkNew<vtkImageExtractComponents> extractComponents;
  if (!readAsMultipleImagesOn)
  {
    reader->Update();
    // Copy image data to sequence of volume nodes
    imageData = reader->GetOutput();
    if (imageData == nullptr || imageData->GetPointData() == nullptr
        || imageData->GetPointData()->GetScalars() == nullptr)
    {
      vtkErrorMacro("vtkMRMLVolumeSequenceStorageNode::ReadDataInternal: invalid image data");
      return 0;
    }
    numberOfFrames = imageData->GetNumberOfScalarComponents();
    extractComponents->SetInputConnection(reader->GetOutputPort());
  }
#else
  reader->Update();
  // Copy image data to sequence of volume nodes
  vtkImageData* imageData = reader->GetOutput();
  if (imageData == nullptr || imageData->GetPointData() == nullptr
      || imageData->GetPointData()->GetScalars() == nullptr)
  {
    vtkErrorMacro("vtkMRMLVolumeSequenceStorageNode::ReadDataInternal: invalid image data");
    return 0;
  }
  int numberOfFrames = imageData->GetNumberOfScalarComponents();
  vtkNew<vtkImageExtractComponents> extractComponents;
  extractComponents->SetInputConnection(reader->GetOutputPort());
#endif

  vtkDebugMacro(<< " vtkMRMLVolumeSequenceStorageNode::ReadDataInternal: Starting reading sequence. ");
  for (int frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex)
  {
    vtkDebugMacro(<< " reading frame : " << frameIndex);
#ifdef NRRD_CHUNK_IO_AVAILABLE
    vtkImageData* frameVoxels = nullptr;
    if (readAsMultipleImagesOn)
    {
      reader->SetCurrentImageIndex(frameIndex);
      reader->Update();
      // It is not necessary to deepcopy imageData here,
      // because it will be already deepcopied in volSequenceNode->SetDataNodeAtValue.
      frameVoxels = reader->GetOutput();
    }
    else
    {
      extractComponents->SetComponents(frameIndex);
      extractComponents->Update();
      frameVoxels = extractComponents->GetOutput();
    }
#else
    extractComponents->SetComponents(frameIndex);
    extractComponents->Update();
    vtkNew<vtkImageData> frameVoxels;
    frameVoxels->DeepCopy(extractComponents->GetOutput());
#endif
    // Slicer expects normalized image position and spacing
    frameVoxels->SetOrigin(0, 0, 0);
    frameVoxels->SetSpacing(1, 1, 1);
    vtkNew<vtkMRMLScalarVolumeNode> frameVolume;
#ifdef NRRD_CHUNK_IO_AVAILABLE
    frameVolume->SetAndObserveImageData(frameVoxels);
#else
    frameVolume->SetAndObserveImageData(frameVoxels.GetPointer());
#endif
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
    frameVolume->SetName(nameStr.str().c_str());
    volSequenceNode->SetDataNodeAtValue(frameVolume.GetPointer(), indexStr.str().c_str());
  }

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
  if (firstFrameVolume->GetImageData())
  {
    firstFrameVolume->GetImageData()->GetExtent(firstFrameVolumeExtent);
    firstFrameVolumeScalarType = firstFrameVolume->GetImageData()->GetScalarType();
    firstFrameVolumeNumberOfComponents = firstFrameVolume->GetImageData()->GetNumberOfScalarComponents();
    // VTK NRRD writer only supports 4D volumes (writing a 3D color volume sequence would require 5D)
    if (firstFrameVolumeNumberOfComponents != 1)
    {
      this->GetUserMessages()->AddMessage(
        vtkCommand::ErrorEvent, std::string("Only single scalar component volumes can be written in this format."));
      return false;
    }
  }
  vtkNew<vtkMatrix4x4> firstVolumeIjkToRas;
  firstFrameVolume->GetIJKToRASMatrix(firstVolumeIjkToRas.GetPointer());

  int numberOfFrameVolumes = volSequenceNode->GetNumberOfDataNodes();
  for (int frameIndex = 1; frameIndex < numberOfFrameVolumes; frameIndex++)
  {
    vtkMRMLVolumeNode* currentFrameVolume =
      vtkMRMLVolumeNode::SafeDownCast(volSequenceNode->GetNthDataNode(frameIndex));
    if (currentFrameVolume == nullptr)
    {
      vtkDebugMacro(
        "vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode: only volume nodes can be written (frame "
        << frameIndex << ")");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
                                          std::string("Only volumes can be written in this format."));
      return false;
    }
    vtkNew<vtkMatrix4x4> currentVolumeIjkToRas;
    currentFrameVolume->GetIJKToRASMatrix(currentVolumeIjkToRas.GetPointer());
    if (!vtkAddonMathUtilities::MatrixAreEqual(currentVolumeIjkToRas, firstVolumeIjkToRas))
    {
      vtkDebugMacro(
        "vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode: IJK to RAS matrix is not the same in all frames"
        << " (first frame: " << vtkAddonMathUtilities::ToString(firstVolumeIjkToRas) << ", frame " << frameIndex << ": "
        << vtkAddonMathUtilities::ToString(firstVolumeIjkToRas) << ")");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
                                          std::string("Geometry of all volumes in the sequence must be the same."));
      return false;
    }
    int currentFrameVolumeExtent[6] = { 0, -1, 0, -1, 0, -1 };
    int currentFrameVolumeScalarType = VTK_VOID;
    int currentFrameVolumeNumberOfComponents = 0;
    if (currentFrameVolume->GetImageData())
    {
      currentFrameVolume->GetImageData()->GetExtent(currentFrameVolumeExtent);
      currentFrameVolumeScalarType = currentFrameVolume->GetImageData()->GetScalarType();
      currentFrameVolumeNumberOfComponents = currentFrameVolume->GetImageData()->GetNumberOfScalarComponents();
    }
    for (int i = 0; i < 6; i++)
    {
      if (firstFrameVolumeExtent[i] != currentFrameVolumeExtent[i])
      {
        vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode: extent mismatch (frame "
                      << frameIndex << ")");
        this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
                                            std::string("Extent of all volumes in the sequence must be the same."));
        return false;
      }
    }
    if (currentFrameVolumeScalarType != firstFrameVolumeScalarType)
    {
      vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode: scalar type mismatch (frame "
                    << frameIndex << ")");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
                                          std::string("Scalar type of all volumes in the sequence must be the same."));
      return false;
    }
    if (currentFrameVolumeNumberOfComponents != firstFrameVolumeNumberOfComponents)
    {
      vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode: number of components mismatch (frame "
                    << frameIndex << ")");
      this->GetUserMessages()->AddMessage(
        vtkCommand::ErrorEvent, std::string("Number of components of all volumes in the sequence must be the same."));
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
    vtkDebugMacro(<< "vtkMRMLVolumeSequenceStorageNode::WriteDataInternal: Do not recognize node type "
                  << refNode->GetClassName());
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
                                        std::string("Only sequence nodes can be written in this format."));
    return 0;
  }

  vtkNew<vtkMatrix4x4> firstVolumeIjkToRas;
  int frameVolumeDimensions[3] = { 0 };
  int frameVolumeScalarType = VTK_VOID;
  int numberOfFrameVolumes = volSequenceNode->GetNumberOfDataNodes();
  if (numberOfFrameVolumes > 0)
  {
    vtkMRMLVolumeNode* frameVolume = vtkMRMLVolumeNode::SafeDownCast(volSequenceNode->GetNthDataNode(0));
    if (frameVolume == nullptr)
    {
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
                                          std::string("Only volume sequence can be written in this format."));
      return 0;
    }
    frameVolume->GetIJKToRASMatrix(firstVolumeIjkToRas.GetPointer());
    if (frameVolume->GetImageData())
    {
      frameVolume->GetImageData()->GetDimensions(frameVolumeDimensions);
      frameVolumeScalarType = frameVolume->GetImageData()->GetScalarType();
    }
  }

#ifndef NRRD_CHUNK_IO_AVAILABLE
  vtkNew<vtkImageAppendComponents> appender;
  for (int frameIndex = 0; frameIndex < numberOfFrameVolumes; frameIndex++)
  {
    vtkMRMLVolumeNode* frameVolume = vtkMRMLVolumeNode::SafeDownCast(volSequenceNode->GetNthDataNode(frameIndex));
    if (frameVolume == nullptr)
    {
      vtkDebugMacro(<< "vtkMRMLVolumeSequenceStorageNode::WriteDataInternal: Data node " << frameIndex
                    << " is not a volume");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
                                          std::string("Only volume sequence can be written in this format."));
      return 0;
    }
    vtkNew<vtkMatrix4x4> currentVolumeIjkToRas;
    frameVolume->GetIJKToRASMatrix(currentVolumeIjkToRas.GetPointer());
    if (!vtkAddonMathUtilities::MatrixAreEqual(currentVolumeIjkToRas, firstVolumeIjkToRas))
    {
      vtkDebugMacro(
        "vtkMRMLVolumeSequenceStorageNode::CanWriteFromReferenceNode: IJK to RAS matrix is not the same in all frames"
        << " (first frame: " << vtkAddonMathUtilities::ToString(firstVolumeIjkToRas) << ", frame " << frameIndex << ": "
        << vtkAddonMathUtilities::ToString(firstVolumeIjkToRas) << ")");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
                                          std::string("Geometry of all volumes in the sequence must be the same."));
      return 0;
    }
    int currentFrameVolumeDimensions[3] = { 0 };
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
      vtkDebugMacro(<< "vtkMRMLVolumeSequenceStorageNode::WriteDataInternal: Data node " << frameIndex
                    << " size or scalar type mismatch ("
                    << "got " << currentFrameVolumeDimensions[0] << "x" << currentFrameVolumeDimensions[1] << "x"
                    << currentFrameVolumeDimensions[2] << " "
                    << vtkImageScalarTypeNameMacro(currentFrameVolumeScalarType) << ", "
                    << "expected " << frameVolumeDimensions[0] << "x" << frameVolumeDimensions[1] << "x"
                    << frameVolumeDimensions[2] << " " << vtkImageScalarTypeNameMacro(frameVolumeScalarType));
      this->GetUserMessages()->AddMessage(
        vtkCommand::ErrorEvent, std::string("Size and scalar type of all volumes in the sequence must be the same."));
      return 0;
    }
    if (frameVolume->GetImageData())
    {
      appender->AddInputData(frameVolume->GetImageData());
    }
  }
#endif

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string(""))
  {
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("File name not specified."));
    return 0;
  }
  // Use here the NRRD Writer
  vtkNew<vtkTeemNRRDWriter> writer;
  // ForceRangeAxis needs to be enabled for the writer to correctly write image sequences that contain only a single
  // frame. Without this option the range axis would be omitted for single frame sequences, causing the first axis to be
  // a spatial dimension, rather than range. This would cause an error when attempting to set units on the first axis.
  writer->SetForceRangeAxis(true);
  writer->SetVectorAxisKind(nrrdKindList);
  writer->SetFileName(fullName.c_str());

#ifdef NRRD_CHUNK_IO_AVAILABLE
  // Set Write Multiple Images on
  writer->WriteMultipleImagesAsImageListsOn();
#endif

  writer->SetUseCompression(this->GetUseCompression());

  // Set volume attributes
  writer->SetIJKToRASMatrix(firstVolumeIjkToRas.GetPointer());
  // writer->SetMeasurementFrameMatrix(mf.GetPointer());

  // Write index information
  int axisIndex;
  std::string axisType, axisValues;
#ifdef NRRD_CHUNK_IO_AVAILABLE
  axisIndex = 3;
  axisType = "axis 3 index type";
  axisValues = "axis 3 index values";
#else
  axisIndex = 0;
  axisType = "axis 0 index type";
  axisValues = "axis 0 index values";
#endif

  if (!volSequenceNode->GetIndexName().empty())
  {
    writer->SetAxisLabel(axisIndex, volSequenceNode->GetIndexName().c_str());
  }
  if (!volSequenceNode->GetIndexUnit().empty())
  {
    writer->SetAxisUnit(axisIndex, volSequenceNode->GetIndexUnit().c_str());
  }
  if (!volSequenceNode->GetIndexTypeAsString().empty())
  {
    writer->SetAttribute(axisType, volSequenceNode->GetIndexTypeAsString());
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
    writer->SetAttribute(axisValues, ssIndexValues.str());
  }

  // pass down all MRML attributes to NRRD
  std::vector<std::string> attributeNames = volSequenceNode->GetAttributeNames();
  std::vector<std::string>::iterator ait = attributeNames.begin();
  for (; ait != attributeNames.end(); ++ait)
  {
    writer->SetAttribute((*ait), volSequenceNode->GetAttribute(ait->c_str()));
  }

#ifdef NRRD_CHUNK_IO_AVAILABLE
  writer->SetNumberOfImages(numberOfFrameVolumes);
  int writeFlag = 1;
  vtkDebugMacro(<< " vtkMRMLVolumeSequenceStorageNode::WriteDataInternal: Starting writing sequence. ");
  for (int frameIndex = 0; frameIndex < numberOfFrameVolumes; ++frameIndex)
  {
    vtkDebugMacro(<< " writing frame : " << frameIndex);
    vtkMRMLVolumeNode* frameVolume = vtkMRMLVolumeNode::SafeDownCast(volSequenceNode->GetNthDataNode(frameIndex));
    if (frameVolume == nullptr)
    {
      vtkDebugMacro(<< "vtkMRMLVolumeSequenceStorageNode::WriteDataInternal: Data node " << frameIndex
                    << " is not a volume");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
                                          std::string("Only volume nodes can be saved in this format."));
      return 0;
    }
    vtkNew<vtkMatrix4x4> currentVolumeIjkToRas;
    frameVolume->GetIJKToRASMatrix(currentVolumeIjkToRas.GetPointer());
    if (!vtkAddonMathUtilities::MatrixAreEqual(currentVolumeIjkToRas, firstVolumeIjkToRas))
    {
      vtkDebugMacro("vtkMRMLVolumeSequenceStorageNode::vtkMRMLVolumeSequenceStorageNode: IJK to RAS matrix is not the "
                    "same in all frames"
                    << " (first frame: " << vtkAddonMathUtilities::ToString(firstVolumeIjkToRas) << ", frame "
                    << frameIndex << ": " << vtkAddonMathUtilities::ToString(firstVolumeIjkToRas) << ")");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
                                          std::string("Geometry of all volumes in the sequence must be the same."));
      return 0;
    }
    int currentFrameVolumeDimensions[3] = { 0 };
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
      vtkDebugMacro(<< "vtkMRMLVolumeSequenceStorageNode::WriteDataInternal: Data node " << frameIndex
                    << " size or scalar type mismatch ("
                    << "got " << currentFrameVolumeDimensions[0] << "x" << currentFrameVolumeDimensions[1] << "x"
                    << currentFrameVolumeDimensions[2] << " "
                    << vtkImageScalarTypeNameMacro(currentFrameVolumeScalarType) << ", "
                    << "expected " << frameVolumeDimensions[0] << "x" << frameVolumeDimensions[1] << "x"
                    << frameVolumeDimensions[2] << " " << vtkImageScalarTypeNameMacro(frameVolumeScalarType));
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
                                          std::string("All volumes must be of the same type and geometry."));
      return 0;
    }
    if (frameVolume->GetImageData() == nullptr)
    {
      vtkDebugMacro(<< "vtkMRMLVolumeSequenceStorageNode::WriteDataInternal: "
                       "image data of Data node "
                    << frameIndex << " not found.");
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent,
                                          std::string("One of the volumes in the sequence does not have image data."));
      return 0;
    }

    writer->SetInputDataObject(frameVolume->GetImageData());
    writer->SetCurrentImageIndex(frameIndex);

    writer->Write();
    if (writer->GetWriteError())
    {
      vtkDebugMacro(<< "vtkMRMLVolumeSequenceStorageNode::WriteDataInternal: "
                       "ERROR writing NRRD file "
                    << (writer->GetFileName() == nullptr ? "null" : writer->GetFileName()));
      this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Failed to write NRRD file."));
      writeFlag = 0;
    }
    this->StageWriteData(refNode);
  }

  Nrrd* nrrd = writer->GetNRRDTeem();
  nrrd = nrrdNix(nrrd);
  NrrdIoState* nio = writer->GetNRRDIoTeem();
  nio = nrrdIoStateNix(nio);

#else
  appender->Update();
  writer->SetInputConnection(appender->GetOutputPort());

  writer->Write();
  int writeFlag = 1;
  if (writer->GetWriteError())
  {
    vtkDebugMacro("ERROR writing NRRD file " << (writer->GetFileName() == nullptr ? "null" : writer->GetFileName()));
    this->GetUserMessages()->AddMessage(vtkCommand::ErrorEvent, std::string("Failed to write NRRD file."));
    writeFlag = 0;
  }

  this->StageWriteData(refNode);
#endif

  vtkDebugMacro(<< " vtkMRMLVolumeSequenceStorageNode::WriteDataInternal: sequence successfully written. ");
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
