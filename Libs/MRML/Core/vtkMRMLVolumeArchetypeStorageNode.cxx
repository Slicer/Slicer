/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLVolumeArchetypeStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

// MRML includes
#include "vtkDataFileFormatHelper.h"
#include "vtkMRMLI18N.h"
#include "vtkDataIOManager.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLScene.h"
#ifdef MRML_USE_vtkTeem
# include "vtkMRMLVectorVolumeNode.h"
# include "vtkMRMLDiffusionTensorVolumeNode.h"
#endif
#include "vtkMRMLVolumeArchetypeStorageNode.h"

// VTK ITK includes
#include "vtkITKArchetypeImageSeriesScalarReader.h"
#include "vtkITKArchetypeDiffusionTensorImageReaderFile.h"
#include "vtkITKArchetypeImageSeriesVectorReaderFile.h"
#include "vtkITKArchetypeImageSeriesVectorReaderSeries.h"
#include "vtkITKImageWriter.h"

// VTKsys includes
#include <vtksys/SystemTools.hxx>

// VTK includes
#include <vtkAddonMathUtilities.h>
#include <vtkDataArray.h>
#include <vtkErrorCode.h>
#include <vtkImageChangeInformation.h>
#include <vtkMatrix3x3.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtksys/Directory.hxx>
#include <vtkTransform.h>

// STD includes
#include <algorithm>
#include <iterator>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLVolumeArchetypeStorageNode);

//----------------------------------------------------------------------------
vtkMRMLVolumeArchetypeStorageNode::vtkMRMLVolumeArchetypeStorageNode()
{
  this->TypeDisplayName = vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode", "Volume Archetype Storage");

  this->CenterImage = 0;
  this->SingleFile = 0;
  this->UseOrientationFromFile = 1;
  this->ForceRightHandedIJKCoordinateSystem = true;
  this->DefaultWriteFileExtension = "nrrd";
}

//----------------------------------------------------------------------------
vtkMRMLVolumeArchetypeStorageNode::~vtkMRMLVolumeArchetypeStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLVolumeArchetypeStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  {
    std::stringstream ss;
    ss << this->CenterImage;
    of << " centerImage=\"" << ss.str() << "\"";
  }
  of << " forceRightHandedIJKCoordinateSystem=\"" << (this->ForceRightHandedIJKCoordinateSystem ? "true" : "false") << "\"";
  {
    std::stringstream ss;
    ss << this->UseOrientationFromFile;
    of << " UseOrientationFromFile=\"" << ss.str() << "\"";
  }

  // SingleFile attribute is not written to file. GetNumberOfFileNames()
  // is used to determine if reader should read from single/multiple files.
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeArchetypeStorageNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
  {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "centerImage"))
    {
      int centerImage = 0;
      std::stringstream ss;
      ss << attValue;
      ss >> centerImage;
      this->SetCenterImage(centerImage);
    }
    if (!strcmp(attName, "UseOrientationFromFile"))
    {
      int useOrientationFromFile = 1;
      std::stringstream ss;
      ss << attValue;
      ss >> useOrientationFromFile;
      this->SetUseOrientationFromFile(useOrientationFromFile);
    }
    if (!strcmp(attName, "forceRightHandedIJKCoordinateSystem"))
    {
      this->SetForceRightHandedIJKCoordinateSystem(strcmp(attValue, "true") == 0);
    }
  }

  // SingleFile attribute used to be read from the scene, but often
  // its value was inconsistent with GetNumberOfFileNames() for color volumes.
  // We now initialize SingleFile based on GetNumberOfFileNames(), ignoring
  // any singleFile attribute that may be found in the XML stream.
  this->SingleFile = (this->GetNumberOfFileNames() <= 1);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLVolumeArchetypeStorageNode::Copy(vtkMRMLNode* anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLVolumeArchetypeStorageNode* node = (vtkMRMLVolumeArchetypeStorageNode*)anode;

  this->SetCenterImage(node->CenterImage);
  this->SetSingleFile(node->SingleFile);
  this->SetUseOrientationFromFile(node->UseOrientationFromFile);
  this->SetForceRightHandedIJKCoordinateSystem(node->ForceRightHandedIJKCoordinateSystem);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeArchetypeStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os, indent);
  os << indent << "CenterImage:   " << this->CenterImage << "\n";
  os << indent << "SingleFile:   " << this->SingleFile << "\n";
  os << indent << "UseOrientationFromFile:   " << this->UseOrientationFromFile << "\n";
  os << indent << "ForceRightHandedIJKCoordinateSystem:   " << (this->ForceRightHandedIJKCoordinateSystem ? "true" : "false") << "\n";
}

//----------------------------------------------------------------------------
int vtkMRMLVolumeArchetypeStorageNode::ConvertVoxelVectorTypeMRMLToVTKITK(int mrmlType)
{
  switch (mrmlType)
  {
    case vtkMRMLVolumeNode::VoxelVectorTypeUndefined: return vtkITKImageWriter::VoxelVectorTypeUndefined;
    case vtkMRMLVolumeNode::VoxelVectorTypeSpatial: return vtkITKImageWriter::VoxelVectorTypeSpatial;
    case vtkMRMLVolumeNode::VoxelVectorTypeColorRGB: return vtkITKImageWriter::VoxelVectorTypeColorRGB;
    case vtkMRMLVolumeNode::VoxelVectorTypeColorRGBA: return vtkITKImageWriter::VoxelVectorTypeColorRGBA;
    case vtkMRMLVolumeNode::VoxelVectorTypeSpatialCovariant: return vtkITKImageWriter::VoxelVectorTypeSpatialCovariant;
    default: return vtkITKImageWriter::VoxelVectorTypeUndefined;
  }
}

//----------------------------------------------------------------------------
int vtkMRMLVolumeArchetypeStorageNode::ConvertVoxelVectorTypeVTKITKToMRML(int vtkitkType)
{
  switch (vtkitkType)
  {
    case vtkITKImageWriter::VoxelVectorTypeUndefined: return vtkMRMLVolumeNode::VoxelVectorTypeUndefined;
    case vtkITKImageWriter::VoxelVectorTypeSpatial: return vtkMRMLVolumeNode::VoxelVectorTypeSpatial;
    case vtkITKImageWriter::VoxelVectorTypeColorRGB: return vtkMRMLVolumeNode::VoxelVectorTypeColorRGB;
    case vtkITKImageWriter::VoxelVectorTypeColorRGBA: return vtkMRMLVolumeNode::VoxelVectorTypeColorRGBA;
    case vtkITKImageWriter::VoxelVectorTypeSpatialCovariant: return vtkMRMLVolumeNode::VoxelVectorTypeSpatialCovariant;
    default: return vtkMRMLVolumeNode::VoxelVectorTypeUndefined;
  }
}

//----------------------------------------------------------------------------
bool vtkMRMLVolumeArchetypeStorageNode::CanReadInReferenceNode(vtkMRMLNode* refNode)
{
  return refNode->IsA("vtkMRMLScalarVolumeNode") || //
         refNode->IsA("vtkMRMLVectorVolumeNode");
}

//----------------------------------------------------------------------------
bool vtkMRMLVolumeArchetypeStorageNode::CanWriteFromReferenceNode(vtkMRMLNode* refNode)
{
  return refNode->IsA("vtkMRMLScalarVolumeNode");
}

//----------------------------------------------------------------------------
vtkITKArchetypeImageSeriesReader* vtkMRMLVolumeArchetypeStorageNode::InstantiateVectorVolumeReader(const std::string& fullName)
{
#ifdef MRML_USE_vtkTeem
  //
  // decide if we want to use a vector file reader (e.g. for multi-component nrrd)
  // or a vector series reader (e.g. for a sequence of rgb image files)
  // - note these are different classes because they are too big to compile
  //   as a single class on some systems
  //

  vtkSmartPointer<vtkITKArchetypeImageSeriesReader> reader = vtkSmartPointer<vtkITKArchetypeImageSeriesVectorReaderSeries>::New();
  reader->SetArchetype(fullName.c_str());
  reader->SetSingleFile(this->GetSingleFile());
  reader->SetUseOrientationFromFile(this->GetUseOrientationFromFile());
  try
  {
    reader->UpdateInformation();
  }
  catch (...)
  {
    return nullptr;
  }

  unsigned int numberOfFileNames = reader->GetNumberOfFileNames();

  if (numberOfFileNames == 1)
  {
    reader = vtkSmartPointer<vtkITKArchetypeImageSeriesVectorReaderFile>::New();
    reader->SetArchetype(fullName.c_str());
    reader->SetSingleFile(this->GetSingleFile());
    reader->SetUseOrientationFromFile(this->GetUseOrientationFromFile());
    try
    {
      reader->UpdateInformation();
    }
    catch (...)
    {
      return nullptr;
    }
  }

  vtkDebugMacro("ReadData: readerSeries number of file names = " << numberOfFileNames);

  if (reader->GetNumberOfComponents() < 2)
  {
    return nullptr;
  }
  reader->Register(nullptr);
  return reader;
#else
  (void)fullName;
  return false;
#endif
}

//----------------------------------------------------------------------------
namespace
{

//----------------------------------------------------------------------------
void ApplyImageSeriesReaderWorkaround(vtkMRMLVolumeArchetypeStorageNode* storageNode, vtkITKArchetypeImageSeriesReader* reader, const std::string& fullName)
{
  // TODO: this is a workaround for an issue in itk::ImageSeriesReader
  // where is assumes that all the filenames that have been passed
  // to it are a dimension smaller than the image it is asked to create
  // (i.e. a list of .jpg files that form a volume).
  // In our case though, we can have file lists that include both the
  // header and bulk data, like .hdr/.img pairs.  So we need to
  // be careful not to send extra filenames to the reader if the
  // format is multi-file for the same volume
  //
  // check for Analyze and similar format- if the archetype is
  // one of those, then don't send the rest of the list
  //
  std::string fileExt = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  if (fileExt != std::string(".hdr")    //
      && fileExt != std::string(".img") //
      && fileExt != std::string(".mhd") //
      && fileExt != std::string(".nhdr"))
  {
    for (int n = 0; n < storageNode->GetNumberOfFileNames(); n++)
    {
      std::string nthFileName = storageNode->GetFullNameFromNthFileName(n);
      vtkDebugWithObjectMacro(
        storageNode, "ReadData: got full name for " << n << "th file: " << nthFileName << ", adding it to reader, current num files on it = " << reader->GetNumberOfFileNames());
      reader->AddFileName(nthFileName.c_str());
    }
  }
}

} // end of anonymous namespace

//----------------------------------------------------------------------------
int vtkMRMLVolumeArchetypeStorageNode::ReadDataInternal(vtkMRMLNode* refNode)
{
  // Skip file loading for empty volume, for which no file was saved
  if (this->GetWriteState() == SkippedNoData)
  {
    vtkDebugMacro("ReadDataInternal: Empty volume file was not saved, ignore loading");
    return 1;
  }

  std::string fullName = this->GetFullNameFromFileName();
  vtkDebugMacro("ReadData: got full archetype name " << fullName);

  if (fullName.empty())
  {
    vtkErrorMacro("vtkMRMLVolumeArchetypeStorageNode::ReadDataInternal: File name not specified");
    return 0;
  }

  //
  // vtkMRMLVolumeNode
  //   |
  //   |--vtkMRMLScalarVolumeNode
  //         |
  //         |----vtkMRMLDiffusionWeightedVolumeNode
  //         |
  //         |----vtkMRMLTensorVolumeNode
  //                  |
  //                  |---vtkMRMLDiffusionImageVolumeNode
  //                  |       |
  //                  |       |---vtkMRMLDiffusionTensorVolumeNode
  //                  |
  //                  |---vtkMRMLVectorVolumeNode
  //

  vtkMRMLScalarVolumeNode* volNode = vtkMRMLScalarVolumeNode::SafeDownCast(refNode);
  if (volNode == nullptr)
  {
    vtkErrorMacro("vtkMRMLVolumeArchetypeStorageNode::ReadDataInternal: Reference node is expected to be a vtkMRMLScalarVolumeNode");
    return 0;
  }

  vtkSmartPointer<vtkITKArchetypeImageSeriesReader> reader;

  if (refNode->IsA("vtkMRMLVectorVolumeNode"))
  {
    reader.TakeReference(this->InstantiateVectorVolumeReader(fullName));
  }
  else if (refNode->IsA("vtkMRMLDiffusionTensorVolumeNode"))
  {
    reader = vtkSmartPointer<vtkITKArchetypeDiffusionTensorImageReaderFile>::New();
    reader->SetSingleFile(this->GetSingleFile());
    reader->SetUseOrientationFromFile(this->GetUseOrientationFromFile());
  }
  else
  {
    reader = vtkSmartPointer<vtkITKArchetypeImageSeriesScalarReader>::New();
    reader->SetSingleFile(this->GetSingleFile());
    reader->SetUseOrientationFromFile(this->GetUseOrientationFromFile());
  }

  if (reader.GetPointer() == nullptr)
  {
    vtkErrorMacro("vtkMRMLVolumeArchetypeStorageNode::ReadDataInternal: Failed to instantiate a file reader");
    return 0;
  }

  vtkObserveMRMLObjectEventMacro(reader, vtkCommand::ProgressEvent);

  if (volNode->GetImageData())
  {
    volNode->SetAndObserveImageData(nullptr);
  }

  // Set the list of file names on the reader
  reader->ResetFileNames();
  reader->SetArchetype(fullName.c_str());

  // Workaround
  ApplyImageSeriesReaderWorkaround(this, reader, fullName);

  // Center image
  reader->SetOutputScalarTypeToNative();
  reader->SetDesiredCoordinateOrientationToNative();
  if (this->CenterImage)
  {
    reader->SetUseNativeOriginOff();
  }
  else
  {
    reader->SetUseNativeOriginOn();
  }

  bool readingWorked = true;
  std::string errorMessage = "";
  try
  {
    vtkDebugMacro("ReadDataInternal: right before reader update, reader num files = " << reader->GetNumberOfFileNames());
    reader->Update();
    if (reader->GetErrorCode() != vtkErrorCode::NoError)
    {
      readingWorked = false;
      errorMessage = std::string(vtkErrorCode::GetStringFromErrorCode(reader->GetErrorCode()));
    }
  }
  catch (itk::ExceptionObject& e)
  {
    readingWorked = false;
    errorMessage = std::string("ITK exception info: error in ") + e.GetLocation() + "\n" + e.GetDescription() + "\n";
  }
  if (!readingWorked)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumeArchetypeStorageNode::ReadDataInternal",
                                     vtkMRMLI18N::Format(vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode", "Cannot read '%1' file as a volume of type '%2'. Details: %3."),
                                                         fullName.c_str(),
                                                         refNode ? refNode->GetNodeTagName() : "",
                                                         errorMessage.c_str());
                                     // Log some more details for debugging (not displayed to user)
                                     vtkErrorMacro("vtkMRMLVolumeArchetypeStorageNode::ReadDataInternal: Reading of file '"
                                                   << fullName << "' failed: " << errorMessage << " Number of files listed in the node is " << this->GetNumberOfFileNames() << "."
                                                   << " File reader says it was able to read " << reader->GetNumberOfFileNames() << " files."
                                                   << " File reader used the archetype file name of '" << reader->GetArchetype() << "' (first filename: '"
                                                   << (reader->GetFileName(0) ? reader->GetFileName(0) : "") << "')")) return 0;
  }

  if (reader->GetOutput() == nullptr || reader->GetOutput()->GetPointData() == nullptr)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumeArchetypeStorageNode::ReadDataInternal",
                                     vtkMRMLI18N::Format(vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode", "Unable to read data from file: '%1'"), fullName.c_str()));
    return 0;
  }

  vtkPointData* pointData = reader->GetOutput()->GetPointData();
  if (volNode->IsA("vtkMRMLDiffusionTensorVolumeNode"))
  {
    if (pointData->GetTensors() == nullptr || pointData->GetTensors()->GetNumberOfTuples() == 0)
    {
      vtkErrorToMessageCollectionMacro(
        this->GetUserMessages(),
        "vtkMRMLVolumeArchetypeStorageNode::ReadDataInternal",
        vtkMRMLI18N::Format(vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode", "Unable to read DiffusionTensorVolume data from file: '%1'"), fullName.c_str()));
      return 0;
    }
  }
  else
  {
    if (pointData->GetScalars() == nullptr || pointData->GetScalars()->GetNumberOfTuples() == 0)
    {
      vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                       "vtkMRMLVolumeArchetypeStorageNode::ReadDataInternal",
                                       vtkMRMLI18N::Format(vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode", "Unable to read ScalarVolume data from file: '%1'"), fullName.c_str()));
      return 0;
    }
  }

  if (!volNode->IsA("vtkMRMLVectorVolumeNode")             //
      && !volNode->IsA("vtkMRMLDiffusionTensorVolumeNode") //
      && reader->GetNumberOfComponents() != 1)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumeArchetypeStorageNode::ReadDataInternal",
                                     vtkMRMLI18N::Format(vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode", "Not a scalar volume file: '%1'"), fullName.c_str()));
    return 0;
  }

  // Set volume attributes
  vtkMRMLVolumeArchetypeStorageNode::SetMetaDataDictionaryFromReader(volNode, reader);

  // Get all the file names from the reader
  if (reader->GetNumberOfFileNames() > 1)
  {
    vtkDebugMacro("ReadDataInternal: Number of file names = " << reader->GetNumberOfFileNames() << ", number of slice location = " << reader->GetNumberOfSliceLocation());
    if (this->FileNameList.size() == 0)
    {
      // It is safe to assume that the file names in reader are unique.
      // Here we shortcut the n*log(n) unique insertion of  AddFileName().
      this->FileNameList = reader->GetFileNames();
    }
    else
    {
      // include the archetype, file 0, in the storage node's file list
      for (unsigned int n = 0; n < reader->GetNumberOfFileNames(); n++)
      {
        const char* thisFileName = reader->GetFileName(n);
#ifndef NDEBUG
        int currentSize =
#endif
          this->AddFileName(thisFileName);
        vtkDebugMacro("ReadDataInternal: After adding file " << n << ", filename = " << thisFileName << " to this storage node's list, current size of the list = " << currentSize);
      }
    }
  }

  vtkNew<vtkImageChangeInformation> ici;
  ici->SetInputConnection(reader->GetOutputPort());
  ici->SetOutputSpacing(1, 1, 1);
  ici->SetOutputOrigin(0, 0, 0);
  ici->Update();

  if (ici->GetOutput() == nullptr)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumeArchetypeStorageNode::ReadDataInternal",
                                     vtkMRMLI18N::Format(vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode", "Cannot read file: '%1'"), fullName.c_str()));
    return 0;
  }

  vtkNew<vtkImageData> outputImage;
  outputImage->ShallowCopy(ici->GetOutput());
  volNode->SetAndObserveImageData(outputImage.GetPointer());

  int voxelVectorType = this->ConvertVoxelVectorTypeVTKITKToMRML(reader->GetVoxelVectorType());
  volNode->SetVoxelVectorType(voxelVectorType);

  // Log volume size to the application log. It helps to identify potential out-of-memory issues.
  vtkDebugMacro(<< "Loaded volume from file: " << fullName << ". Dimensions: " << outputImage->GetDimensions()[0] << "x" << outputImage->GetDimensions()[1] << "x"
                << outputImage->GetDimensions()[2] << ". Number of components: " << outputImage->GetNumberOfScalarComponents()
                << ". Pixel type: " << vtkImageScalarTypeNameMacro(outputImage->GetScalarType()) << ".");

  vtkMatrix4x4* rasToIjkMatrix = reader->GetRasToIjkMatrix();
  if (rasToIjkMatrix == nullptr)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumeArchetypeStorageNode::ReadDataInternal",
                                     vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode", "Image reader provided invalid RAS to IJK matrix"));
  }
  volNode->SetRASToIJKMatrix(rasToIjkMatrix);

  // If volume is left-handed coordinates, modify it to right-handed coordinate
  // to have support for every algorithms in 3D Slicer
  if (this->ForceRightHandedIJKCoordinateSystem)
  {
    volNode->SetIJKCoordinateSystemToRightHanded();
  }

  if (volNode->IsA("vtkMRMLDiffusionTensorVolumeNode"))
  {
    vtkMRMLDiffusionTensorVolumeNode* dtvn = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(volNode);
    dtvn->SetMeasurementFrameMatrix(reader->GetMeasurementFrameMatrix());
  }

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLVolumeArchetypeStorageNode::WriteDataInternal(vtkMRMLNode* refNode)
{
  int result = 1;

  vtkMRMLVolumeNode* volNode = vtkMRMLVolumeNode::SafeDownCast(refNode);
  if (!volNode)
  {
    vtkErrorMacro("vtkMRMLVolumeArchetypeStorageNode::WriteDataInternal: File writing failed: refNode is invalid");
    return 0;
  }

  if (volNode->GetImageData() == nullptr)
  {
    this->SetWriteStateSkippedNoData();
    return 1;
  }

  // update the file list
  std::string moveFromDir = this->UpdateFileList(refNode, true);

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
  {
    vtkErrorMacro("vtkMRMLVolumeArchetypeStorageNode::WriteDataInternal: File name not specified");
    return 0;
  }

  int voxelVectorType = volNode->GetVoxelVectorType();
  if (voxelVectorType == vtkMRMLVolumeNode::VoxelVectorTypeSpatial || voxelVectorType == vtkMRMLVolumeNode::VoxelVectorTypeSpatialCovariant)
  {
    if (volNode->GetImageData()->GetNumberOfScalarComponents() != 3)
    {
      vtkWarningToMessageCollectionMacro(
        this->GetUserMessages(),
        "vtkMRMLVolumeArchetypeStorageNode::WriteDataInternal",
        vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode", "Voxel vector type is spatial but number of scalar components is not 3. Saved vector type will be non-spatial."));
    }
    else
    {
      std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
      if (extension != ".nrrd" && extension != ".nhdr")
      {
        vtkWarningToMessageCollectionMacro(this->GetUserMessages(),
                                           "vtkMRMLVolumeArchetypeStorageNode::WriteDataInternal",
                                           vtkMRMLI18N::Format(vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode",
                                                                         "Spatial vectors will be written to non-NRRD file format (%1). In this format, voxels are saved"
                                                                         " as regular vectors. If the file is imported again then vector axis directions may be flipped."
                                                                         " It is recommended to save volumes that contain spatial vectors in NRRD file format."),
                                                               extension.c_str()));
      }
    }
  }

  bool moveSucceeded = true;
  if (!moveFromDir.empty())
  {
    // the temp writing went okay, just move the files from there to where
    // they're supposed to go. It will fail if the temp dir is on a different
    // device, so fall back to a second write in that case.
    std::string targetDir = vtksys::SystemTools::GetFilenamePath(fullName);
    if (!this->MoveFilesWithLocking(moveFromDir, targetDir))
    {
      vtksys::SystemTools::RemoveADirectory(moveFromDir.c_str());
      vtkErrorToMessageCollectionMacro(
        this->GetUserMessages(), "vtkMRMLVolumeArchetypeStorageNode::WriteDataInternal", vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode", "Failed to commit temporary files."));
      return 0;
    }
  }
  else
  {
    // didn't move it
    moveSucceeded = false;
  }

  if (!moveSucceeded)
  {
    vtkDebugMacro("WriteData: writing out file with archetype " << fullName);

    vtkNew<vtkITKImageWriter> writer;
    writer->SetFileName(fullName.c_str());

    writer->SetInputConnection(volNode->GetImageDataConnection());
    writer->SetUseCompression(this->GetUseCompression());
    if (this->WriteFileFormat)
    {
      writer->SetImageIOClassName(this->GetScene()->GetDataIOManager()->GetFileFormatHelper()->GetClassNameFromFormatString(this->WriteFileFormat));
    }

    // set volume attributes
    vtkNew<vtkMatrix4x4> mat;
    volNode->GetRASToIJKMatrix(mat.GetPointer());
    writer->SetRasToIJKMatrix(mat.GetPointer());

    // Pass on voxel type to the writer
    int voxelVectorType = volNode->GetVoxelVectorType();
    writer->SetVoxelVectorType(this->ConvertVoxelVectorTypeMRMLToVTKITK(voxelVectorType));
    try
    {
      writer->Write();
    }
    catch (...)
    {
      result = 0;
    }
  }

  // Display warning if saving VTK file with non-LPS axes (VTK cannot store axis directions)
  std::string lowerCaseFileName = vtksys::SystemTools::LowerCase(fullName);
  if (vtksys::SystemTools::StringEndsWith(lowerCaseFileName, ".vtk"))
  {
    vtkNew<vtkMatrix4x4> currentIjkToRasDirection;
    volNode->GetRASToIJKMatrix(currentIjkToRasDirection.GetPointer());
    double unitScale[3] = { 1.0, 1.0, 1.0 };
    vtkAddonMathUtilities::NormalizeOrientationMatrixColumns(currentIjkToRasDirection, unitScale);

    vtkNew<vtkMatrix3x3> identityIjkToRasDirection;
    identityIjkToRasDirection->SetElement(0, 0, -1.0);
    identityIjkToRasDirection->SetElement(1, 1, -1.0);

    if (!vtkAddonMathUtilities::MatrixAreEqual(currentIjkToRasDirection, identityIjkToRasDirection))
    {
      vtkWarningToMessageCollectionMacro(this->GetUserMessages(),
                                         "vtkMRMLVolumeArchetypeStorageNode::WriteDataInternal",
                                         vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode",
                                                   "VTK file format can only store LPS axis oriented images. Orientation of the saved image may be incorrect."
                                                   " NRRD file format is recommended for storing this image."));
    }
  }

  // Display warning if saving TIFF file as 3D image
  if ((vtksys::SystemTools::StringEndsWith(lowerCaseFileName, ".tiff") || //
       vtksys::SystemTools::StringEndsWith(lowerCaseFileName, ".tif")))
  {
    // Display warning if saving TIFF file with non-LPS axes or non-zero origin
    // (TIFF cannot store axis directions and origin position in standard fields)

    vtkNew<vtkMatrix4x4> currentIjkToRas;
    volNode->GetRASToIJKMatrix(currentIjkToRas);
    double unitScale[3] = { 1.0, 1.0, 1.0 };
    vtkAddonMathUtilities::NormalizeOrientationMatrixColumns(currentIjkToRas, unitScale);

    vtkNew<vtkMatrix4x4> identityIjkToRas;
    identityIjkToRas->SetElement(0, 0, -1.0);
    identityIjkToRas->SetElement(1, 1, -1.0);

    // Display warning if saving TIFF file with non-unit K spacing (TIFF cannot store spacing along K axis)
    bool unitZSpacing = (fabs(unitScale[2] - 1.0) < 1e-3);

    if (!vtkAddonMathUtilities::MatrixAreEqual(currentIjkToRas, identityIjkToRas) //
        || !unitZSpacing)
    {
      vtkWarningToMessageCollectionMacro(
        this->GetUserMessages(),
        "vtkMRMLVolumeArchetypeStorageNode::WriteDataInternal",
        vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode",
                  "Saving this image in TIFF format will result in loss of information (image position, orientation, or spacing along the third image axis)."
                  " NRRD file format is recommended for storing this image."));
    }
  }

  // Display warnings for Analyze files
  if (vtksys::SystemTools::StringEndsWith(lowerCaseFileName, ".hdr")    //
      || vtksys::SystemTools::StringEndsWith(lowerCaseFileName, ".img") //
      || vtksys::SystemTools::StringEndsWith(lowerCaseFileName, ".img.gz"))
  {
    vtkWarningToMessageCollectionMacro(this->GetUserMessages(),
                                       "vtkMRMLVolumeArchetypeStorageNode::WriteDataInternal",
                                       vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode",
                                                 "Analyze file format is not recommended, as its image orientation specification is ambiguous."
                                                 " NRRD file format is recommended for storing this image."));
  }

  return result;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeArchetypeStorageNode::InitializeSupportedWriteFileTypes()
{
  Superclass::InitializeSupportedWriteFileTypes();
  if (this->GetScene() &&                     //
      this->GetScene()->GetDataIOManager() && //
      this->GetScene()->GetDataIOManager()->GetFileFormatHelper())
  {
    vtkStringArray* supportedFormats = this->GetScene()->GetDataIOManager()->GetFileFormatHelper()->GetITKSupportedWriteFileFormats();
    for (int i = 0; i < supportedFormats->GetNumberOfTuples(); i++)
    {
      this->SupportedWriteFileTypes->InsertNextValue(supportedFormats->GetValue(i));
    }
  }
}

//----------------------------------------------------------------------------
std::string vtkMRMLVolumeArchetypeStorageNode::UpdateFileList(vtkMRMLNode* refNode, bool move)
{
  // test whether refNode is a valid node to hold a volume
  if (!refNode->IsA("vtkMRMLScalarVolumeNode"))
  {
    vtkErrorMacro("vtkMRMLVolumeArchetypeStorageNode::UpdateFileList: Reference node is not a volume");
    return "";
  }

  vtkMRMLVolumeNode* volNode = vtkMRMLScalarVolumeNode::SafeDownCast(refNode);

  if (volNode == nullptr || volNode->GetImageData() == nullptr)
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumeArchetypeStorageNode::UpdateFileList", vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode", "Cannot write volume, image data is empty"));
    return "";
  }

  std::string oldName(this->GetFileName() ? this->GetFileName() : "");
  if (oldName.empty())
  {
    vtkErrorMacro("vtkMRMLVolumeArchetypeStorageNode::UpdateFileList failed: File name not specified");
    return "";
  }

  vtkDebugMacro("UpdateFileList: old file name = " << oldName);

  // clear out the old file list
  this->ResetFileNameList();

  std::string originalDir = vtksys::SystemTools::GetFilenamePath(oldName);
  std::string tempDir;
  std::string tempFilePath;
  if (!this->GenerateTempFilePathForWrite(oldName, tempDir, tempFilePath))
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumeArchetypeStorageNode::UpdateFileList",
                                     vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode", "Failed to create temporary file and directory for writing."));
    return "";
  }
  if (tempFilePath.empty())
  {
    vtkErrorToMessageCollectionMacro(
      this->GetUserMessages(), "vtkMRMLVolumeArchetypeStorageNode::UpdateFileList", vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode", "Failed to create temporary file for writing."));
    vtksys::SystemTools::RemoveADirectory(tempDir.c_str());
    return "";
  }

  vtkDebugMacro("UpdateFileList: new archetype file name = " << tempFilePath.c_str());

  // set up the writer and write
  vtkNew<vtkITKImageWriter> writer;
  writer->SetFileName(tempFilePath.c_str());
  writer->SetInputData(volNode->GetImageData());
  writer->SetUseCompression(this->GetUseCompression());
  if (this->WriteFileFormat)
  {
    if (this->GetScene() &&                     //
        this->GetScene()->GetDataIOManager() && //
        this->GetScene()->GetDataIOManager()->GetFileFormatHelper())
    {
      writer->SetImageIOClassName(this->GetScene()->GetDataIOManager()->GetFileFormatHelper()->GetClassNameFromFormatString(this->WriteFileFormat));
    }
  }

  // set volume attributes
  vtkNew<vtkMatrix4x4> mat;
  volNode->GetRASToIJKMatrix(mat.GetPointer());
  writer->SetRasToIJKMatrix(mat.GetPointer());

  // Pass on voxel type to the writer
  int voxelVectorType = volNode->GetVoxelVectorType();
  writer->SetVoxelVectorType(this->ConvertVoxelVectorTypeMRMLToVTKITK(voxelVectorType));
  bool success = true;
  try
  {
    writer->Write();
  }
  catch (...)
  {
    success = false;
  }
  if (!success)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumeArchetypeStorageNode::UpdateFileList",
                                     vtkMRMLI18N::Format(vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode", "Failed to write '%1'"), tempFilePath.c_str()));
    vtksys::SystemTools::RemoveADirectory(tempDir.c_str());
    return "";
  }

  // look through the new dir and populate the file list
  vtksys::Directory dir;
  success = dir.Load(tempDir.c_str()).IsSuccess();
  vtkDebugMacro("UpdateFileList: tempdir " << tempDir.c_str() << " has " << dir.GetNumberOfFiles() << " in it");
  if (!success)
  {
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumeArchetypeStorageNode::UpdateFileList",
                                     vtkMRMLI18N::Format(vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode", "Failed to open directory '%1'"), tempDir.c_str()));
    vtksys::SystemTools::RemoveADirectory(tempDir.c_str());
    return "";
  }

  // take temp dir off of the path
  std::vector<std::string> pathComponents;
  vtksys::SystemTools::SplitPath(tempDir.c_str(), pathComponents);
  pathComponents.pop_back();
  std::string localDirectory = vtksys::SystemTools::JoinPath(pathComponents);
  std::string relativePath;

  if (this->IsFilePathRelative(localDirectory.c_str()))
  {
    vtkDebugMacro("UpdateFileList: the local directory is already relative, use it " << localDirectory);
    relativePath = localDirectory;
  }
  else
  {
    if (volNode->GetScene() != nullptr && //
        strlen(volNode->GetScene()->GetRootDirectory()))
    {
      // use the scene's root dir, all the files in the list will be
      // relative to it (the relative path is how you go from the root dir to
      // the dir in which the volume is saved)
      std::string rootDir = volNode->GetScene()->GetRootDirectory();
      if (rootDir.length() != 0 && //
          rootDir.find_last_of("/") == rootDir.length() - 1)
      {
        vtkDebugMacro("UpdateFileList: found trailing slash in : " << rootDir);
        rootDir = rootDir.substr(0, rootDir.length() - 1);
      }
      vtkDebugMacro("UpdateFileList: got the scene root dir " << rootDir << ", local dir = " << localDirectory.c_str());
      // RelativePath requires two absolute paths, otherwise returns empty
      // string
      if (this->IsFilePathRelative(rootDir.c_str()))
      {
        vtkDebugMacro("UpdateFileList: have a relative directory in root dir (" << rootDir << "), using the local dir as a relative path.");
        // assume the relative local directory is relative to the root
        // directory
        relativePath = localDirectory;
      }
      else
      {
        relativePath = vtksys::SystemTools::RelativePath(rootDir.c_str(), localDirectory.c_str());
      }
    }
    else
    {
      // use the archetype's directory, so that all the files in the list will
      // be relative to it
      if (this->IsFilePathRelative(originalDir.c_str()))
      {
        relativePath = localDirectory;
      }
      else
      {
        // the RelativePath method needs two absolute paths
        relativePath = vtksys::SystemTools::RelativePath(originalDir.c_str(), localDirectory.c_str());
      }
      vtkDebugMacro("UpdateFileList: no scene root dir, using original dir = " << originalDir.c_str() << " and local dir " << localDirectory.c_str());
    }
  }
  // strip off any trailing slashes
  if (relativePath.length() != 0 &&                          //
      relativePath.find_last_of("/") != std::string::npos && //
      relativePath.find_last_of("/") == relativePath.length() - 1)
  {
    vtkDebugMacro("UpdateFileList: stripping off a trailing slash from relativePath '" << relativePath.c_str() << "'");
    relativePath = relativePath.substr(0, relativePath.length() - 1);
  }
  vtkDebugMacro("UpdateFileList: using prefix of relative path '" << relativePath.c_str() << "'");
  // now get ready to join the relative path to thisFile
  std::vector<std::string> relativePathComponents;
  vtksys::SystemTools::SplitPath(relativePath.c_str(), relativePathComponents);

  // make sure that the archetype is added first! AddFile when it gets to it
  // in the dir will not add a duplicate
  std::string newArchetype = vtksys::SystemTools::GetFilenameName(tempFilePath.c_str());
  vtkDebugMacro("Stripped archetype = " << newArchetype.c_str());
  relativePathComponents.push_back(newArchetype);
  std::string relativeArchetypeFile = vtksys::SystemTools::JoinPath(relativePathComponents);
  vtkDebugMacro("Relative archetype = " << relativeArchetypeFile.c_str());
  relativePathComponents.pop_back();
  this->AddFileName(relativeArchetypeFile.c_str());

  bool addedArchetype = false;
  // now iterate through the directory files
  for (size_t fileNum = 0; fileNum < dir.GetNumberOfFiles(); ++fileNum)
  {
    // skip the dirs
    const char* thisFile = dir.GetFile(static_cast<unsigned long>(fileNum));
    if (strcmp(thisFile, ".") && //
        strcmp(thisFile, ".."))
    {
      vtkDebugMacro("UpdateFileList: adding file number " << fileNum << ", " << thisFile);
      if (newArchetype.compare(thisFile) == 0)
      {
        addedArchetype = true;
      }
      // at this point, the file name is bare of a directory, turn it into a
      // relative path from the original archetype
      relativePathComponents.emplace_back(thisFile);
      std::string relativeFile = vtksys::SystemTools::JoinPath(relativePathComponents);
      relativePathComponents.pop_back();
      vtkDebugMacro("UpdateFileList: " << fileNum << ", using relative file name " << relativeFile.c_str());
      this->AddFileName(relativeFile.c_str());
    }
  }
  if (!addedArchetype)
  {
    std::stringstream addedFiles;
    std::copy(++this->FileNameList.begin(), this->FileNameList.end(), std::ostream_iterator<std::string>(addedFiles, ", "));
    vtkErrorToMessageCollectionMacro(this->GetUserMessages(),
                                     "vtkMRMLVolumeArchetypeStorageNode::UpdateFileList",
                                     vtkMRMLI18N::Format(vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode",
                                                                   "The archetype file '%1' wasn't written out when writing '%2' in '%3'."
                                                                   " Only these %4 file(s) have been written: %5."
                                                                   " Old name is '%6'."),
                                                         newArchetype.c_str(),
                                                         tempFilePath.c_str(),
                                                         tempDir.c_str(),
                                                         std::to_string(dir.GetNumberOfFiles() - 2).c_str(),
                                                         addedFiles.str().c_str(),
                                                         oldName.c_str()));
    vtksys::SystemTools::RemoveADirectory(tempDir.c_str());
    return "";
  }
  // restore the old file name
  vtkDebugMacro("UpdateFileList: resetting file name to " << oldName.c_str());
  this->SetFileName(oldName.c_str());

  if (move)
  {
    vtkDebugMacro("UpdateFileList: returning temp dir " << tempDir);
    return tempDir;
  }

  // clean up temp directory
  vtkDebugMacro("UpdateFileList: removing temp dir " << tempDir);
  if (!vtksys::SystemTools::RemoveADirectory(tempDir.c_str()))
  {
    vtkWarningToMessageCollectionMacro(this->GetUserMessages(),
                                       "vtkMRMLVolumeArchetypeStorageNode::UpdateFileList",
                                       vtkMRMLI18N::Format(vtkMRMLTr("vtkMRMLVolumeArchetypeStorageNode", "Failed to remove temporary directory '%1'"), tempDir.c_str()));
  }
  return "";
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeArchetypeStorageNode::ConfigureForDataExchange()
{
  this->UseCompressionOff();
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeArchetypeStorageNode::SetMetaDataDictionaryFromReader(vtkMRMLVolumeNode* volNode, vtkITKArchetypeImageSeriesReader* reader)
{
  if (volNode && reader)
  {
    volNode->SetMetaDataDictionary(reader->GetMetaDataDictionary());
  }
}
