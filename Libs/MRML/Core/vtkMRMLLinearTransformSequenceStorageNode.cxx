/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

// STD includes
#include <algorithm>
#include <sstream>

#include "vtkMRMLLinearTransformSequenceStorageNode.h"
#include "vtkMRMLSequenceStorageNode.h"

#include "vtkAddonMathUtilities.h"

#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSequenceNode.h"

#include "vtkObjectFactory.h"
#include "vtkImageAppendComponents.h"
#include "vtkImageData.h"
#include "vtkImageExtractComponents.h"
#include "vtkMatrix4x4.h"
#include "vtkNew.h"
#include "vtkStringArray.h"

#include "vtksys/SystemTools.hxx"

// Constants for reading sequence metafiles
static const int MAX_LINE_LENGTH = 1000;
static std::string SEQMETA_FIELD_FRAME_FIELD_PREFIX = "Seq_Frame";
static std::string SEQMETA_FIELD_IMG_STATUS = "ImageStatus";

// Constants for creating nodes
static const char NODE_BASE_NAME_SEPARATOR[] = "-";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLLinearTransformSequenceStorageNode);

//----------------------------------------------------------------------------
vtkMRMLLinearTransformSequenceStorageNode::vtkMRMLLinearTransformSequenceStorageNode() = default;

//----------------------------------------------------------------------------
vtkMRMLLinearTransformSequenceStorageNode::~vtkMRMLLinearTransformSequenceStorageNode() = default;

//----------------------------------------------------------------------------
bool vtkMRMLLinearTransformSequenceStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLSequenceNode");
}

// Add the helper functions

//-------------------------------------------------------
inline void Trim(std::string &str)
{
  str.erase(str.find_last_not_of(" \t\r\n") + 1);
  str.erase(0, str.find_first_not_of(" \t\r\n"));
}

//----------------------------------------------------------------------------
/*! Quick and robust string to int conversion */
template<class T>
void StringToInt(const char* strPtr, T &result)
{
  if (strPtr == nullptr || strlen(strPtr) == 0)
    {
    return;
    }
  char * pEnd = nullptr;
  result = static_cast<int>(strtol(strPtr, &pEnd, 10));
  if (pEnd != strPtr + strlen(strPtr))
    {
    return;
    }
  return;
}

//----------------------------------------------------------------------------
int vtkMRMLLinearTransformSequenceStorageNode::ReadSequenceFileTransforms(const std::string& fileName, vtkMRMLScene *scene,
  std::deque< vtkSmartPointer<vtkMRMLSequenceNode> > &createdNodes, std::map< int, std::string >& frameNumberToIndexValueMap,
  std::map< std::string, std::string > &imageMetaData, SequenceFileType fileType/*=METAIMAGE_SEQUENCE_FILE */)
{
  int numberOfCreatedNodes = 0;
  // Open in binary mode because we determine the start of the image buffer also during this read
  const char* flags = "rb";
  FILE* stream = fopen(fileName.c_str(), flags); // TODO: Removed error
  if (stream == nullptr)
    {
    vtkGenericWarningMacro("Failed to open file for transform reading: " << fileName);
    return numberOfCreatedNodes;
    }

  char line[MAX_LINE_LENGTH + 1] = { 0 };

  frameNumberToIndexValueMap.clear();

  // This structure contains all the transform nodes that are read from the file.
  // The nodes are not added immediately to the scene to allow them properly named, using the timestamp index value.
  // Maps the frame number to a vector of transform nodes that belong to that frame.
  std::map<int, std::vector<vtkMRMLLinearTransformNode*> > importedTransformNodes;

  // It contains the largest frame number. It will be used to iterate through all the frame numbers from 0 to lastFrameNumber
  int lastFrameNumber = -1;

  while (fgets(line, MAX_LINE_LENGTH, stream))
    {
    std::string lineStr = line;

    // Split line into name and value
    size_t separatorFound = 0;
    if (fileType == NRRD_SEQUENCE_FILE)
      {
      separatorFound = lineStr.find_first_of("#");
      if (separatorFound != std::string::npos || lineStr.find("NRRD") == 0)
        {
        // Header definition or comment found, skip
        continue;
        }

      separatorFound = lineStr.find_first_of(":");
      }
    else
      {
      separatorFound = lineStr.find_first_of("=");
      }

    if (separatorFound == std::string::npos)
      {
      if (fileType == NRRD_SEQUENCE_FILE)
        {
        // End of NRRD header
        // There are no more transforms to read
        break;
        }

      vtkGenericWarningMacro("Parsing line failed, equal sign is missing (" << lineStr << ")");
      continue;
      }

    std::string name = lineStr.substr(0, separatorFound);
    std::string value = lineStr.substr(separatorFound + 1);

    if (fileType == NRRD_SEQUENCE_FILE)
      {
      if (lineStr[separatorFound + 1] == '=')
        {
        value = lineStr.substr(separatorFound + 2);
        }
      }

    // Trim spaces from the left and right
    Trim(name);
    Trim(value);

    if (name.compare("ElementDataFile") == 0)
      {
      // this is the last field of the header
      break;
      }

    // Only consider the Seq_Frame
    if (name.compare(0, SEQMETA_FIELD_FRAME_FIELD_PREFIX.size(), SEQMETA_FIELD_FRAME_FIELD_PREFIX) != 0)
      {
      // not a frame field
      if (name.compare("UltrasoundImageOrientation") == 0)
        {
        imageMetaData["UltrasoundImageOrientation"] = value;
        }
      else if (name.compare("UltrasoundImageType") == 0)
        {
        imageMetaData["UltrasoundImageType"] = value;
        }
      else if (name.compare("NDims") == 0)
        {
        imageMetaData["NDims"] = value;
        }
      continue;
      }

    // frame field
    // name: Seq_Frame0000_CustomTransform
    name.erase(0, SEQMETA_FIELD_FRAME_FIELD_PREFIX.size()); // 0000_CustomTransform

    // Split line into name and value
    size_t underscoreFound;
    underscoreFound = name.find_first_of("_");
    if (underscoreFound == std::string::npos)
      {
      vtkGenericWarningMacro("Parsing line failed, underscore is missing from frame field name (" << lineStr << ")");
      continue;
      }

    std::string frameNumberStr = name.substr(0, underscoreFound); // 0000
    std::string frameFieldName = name.substr(underscoreFound + 1); // CustomTransform

    int frameNumber = 0;
    StringToInt(frameNumberStr.c_str(), frameNumber); // TODO: Removed warning
    if (frameNumber > lastFrameNumber)
      {
      lastFrameNumber = frameNumber;
      }

    // Convert the string to transform and add transform to hierarchy
    if (frameFieldName.find("Transform") != std::string::npos && frameFieldName.find("Status") == std::string::npos)
      {
      vtkNew<vtkMatrix4x4> matrix;
      bool success = vtkAddonMathUtilities::FromString(matrix.GetPointer(), value);
      if (!success)
        {
        continue;
        }
      vtkMRMLLinearTransformNode* currentTransform = vtkMRMLLinearTransformNode::New(); // will be deleted when added to the scene
      currentTransform->SetMatrixTransformToParent(matrix.GetPointer());
      // Generating a unique name is important because that will be used to generate the filename by default
      currentTransform->SetName(frameFieldName.c_str());
      importedTransformNodes[frameNumber].push_back(currentTransform);
      }

    if (frameFieldName.compare("Timestamp") == 0)
      {
      double timestampSec = atof(value.c_str());
      // round timestamp to 3 decimal digits, as timestamp is included in node names and having lots of decimal digits would
      // sometimes lead to extremely long node names
      std::ostringstream timestampSecStr;
      timestampSecStr << std::fixed << std::setprecision(3) << timestampSec << std::ends;
      frameNumberToIndexValueMap[frameNumber] = timestampSecStr.str();
      }

    if (ferror(stream))
      {
      vtkGenericWarningMacro("Error reading the file " << fileName.c_str());
      break;
      }
    if (feof(stream))
      {
      break;
      }

    }
  fclose(stream);

  // Now add all the nodes to the scene

  std::map< std::string, vtkMRMLSequenceNode* > transformSequenceNodes;

  for (int currentFrameNumber = 0; currentFrameNumber <= lastFrameNumber; currentFrameNumber++)
    {
    std::map<int, std::vector<vtkMRMLLinearTransformNode*> >::iterator transformsForCurrentFrame = importedTransformNodes.find(currentFrameNumber);
    if (transformsForCurrentFrame == importedTransformNodes.end())
      {
      // no transforms for this frame
      continue;
      }
    std::string paramValueString = frameNumberToIndexValueMap[currentFrameNumber];
    for (std::vector<vtkMRMLLinearTransformNode*>::iterator transformIt = transformsForCurrentFrame->second.begin();
      transformIt != transformsForCurrentFrame->second.end(); ++transformIt)
      {
      vtkMRMLLinearTransformNode* transform = (*transformIt);
      vtkMRMLSequenceNode* transformsSequenceNode = nullptr;
      if (transformSequenceNodes.find(transform->GetName()) == transformSequenceNodes.end())
        {
        // Setup hierarchy structure
        vtkSmartPointer<vtkMRMLSequenceNode> newTransformsSequenceNode;
        if (numberOfCreatedNodes < static_cast<int>(createdNodes.size()))
          {
          // reuse supplied sequence node
          newTransformsSequenceNode = createdNodes[numberOfCreatedNodes];
          newTransformsSequenceNode->RemoveAllDataNodes();
          }
        else
          {
          // Create new sequence node
          newTransformsSequenceNode = vtkSmartPointer<vtkMRMLSequenceNode>::New();
          createdNodes.push_back(newTransformsSequenceNode);
          }
        numberOfCreatedNodes++;
        transformsSequenceNode = newTransformsSequenceNode;
        transformsSequenceNode->SetIndexName("time");
        transformsSequenceNode->SetIndexUnit("s");
        std::string transformName = transform->GetName();
        // Strip "Transform" from the end of the transform name
        std::string transformPostfix = "Transform";
        if (transformName.length() > transformPostfix.length() &&
          transformName.compare(transformName.length() - transformPostfix.length(),
          transformPostfix.length(), transformPostfix) == 0)
          {
          // ends with "Transform" (SomethingToSomethingElseTransform),
          // remove it (to have SomethingToSomethingElse)
          transformName.erase(transformName.length() - transformPostfix.length(), transformPostfix.length());
          }
        // Save transform name to Sequences.Source attribute so that modules can
        // find a transform by matching the original the transform name.
        transformsSequenceNode->SetAttribute("Sequences.Source", transformName.c_str());

        transformSequenceNodes[transform->GetName()] = transformsSequenceNode;
        }
      else
        {
        transformsSequenceNode = transformSequenceNodes[transform->GetName()];
        }
      transform->SetHideFromEditors(false);
      // Generating a unique name is important because that will be used to generate the filename by default
      std::ostringstream nameStr;
      nameStr << transform->GetName() << "_" << std::setw(4) << std::setfill('0') << currentFrameNumber << std::ends;
      transform->SetName(nameStr.str().c_str());
      transformsSequenceNode->SetDataNodeAtValue(transform, paramValueString.c_str());
      transform->Delete(); // ownership transferred to the sequence node
      }
    }

  // Add to scene and set name and storage node
  std::string fileNameName = vtksys::SystemTools::GetFilenameName(fileName);
  std::string shortestBaseNodeName;
  int transformNodeIndex = 0;
  for (std::deque< vtkSmartPointer<vtkMRMLSequenceNode> >::iterator createdTransformNodeIt = createdNodes.begin();
    createdTransformNodeIt != createdNodes.end() && transformNodeIndex < numberOfCreatedNodes; ++createdTransformNodeIt, transformNodeIndex++)
    {
    // strip known file extensions from filename to get base name
    std::string transformName = (*createdTransformNodeIt)->GetAttribute("Sequences.Source") ?
      (*createdTransformNodeIt)->GetAttribute("Sequences.Source") : "";
    std::string baseNodeName = vtkMRMLSequenceStorageNode::GetSequenceBaseName(fileNameName, transformName);
    if (shortestBaseNodeName.empty() || baseNodeName.size() < shortestBaseNodeName.size())
      {
      shortestBaseNodeName = baseNodeName;
      }
    std::string transformsSequenceName = vtkMRMLSequenceStorageNode::GetSequenceNodeName(baseNodeName, transformName);
    (*createdTransformNodeIt)->SetName(transformsSequenceName.c_str());
    if (scene && (*createdTransformNodeIt)->GetScene() == nullptr)
      {
      scene->AddNode(*createdTransformNodeIt);
      }
    if ((*createdTransformNodeIt)->GetScene())
      {
      // Add/initialize storage node
      if (!(*createdTransformNodeIt)->GetStorageNode())
        {
        (*createdTransformNodeIt)->AddDefaultStorageNode();
        }
      if (numberOfCreatedNodes == 1)
        {
        // Only one transform is stored in this file. Update stored time to mark the file as not modified since read.
        vtkMRMLLinearTransformSequenceStorageNode* storageNode =
          vtkMRMLLinearTransformSequenceStorageNode::SafeDownCast((*createdTransformNodeIt)->GetStorageNode());
        if (storageNode)
          {
          // Only one transform is stored in this file. Update stored time to mark the file as not modified since read.
          storageNode->StoredTime->Modified();
          }
        }
      }
    }
  return numberOfCreatedNodes;
}

//----------------------------------------------------------------------------
bool vtkMRMLLinearTransformSequenceStorageNode::WriteSequenceMetafileTransforms(
  const std::string& fileName, std::deque< vtkMRMLSequenceNode* > &transformSequenceNodes,
  std::deque< std::string > &transformNames, vtkMRMLSequenceNode* masterNode, vtkMRMLSequenceNode* imageNode)
{
  vtkMRMLSequenceNode* masterSequenceNode = vtkMRMLSequenceNode::SafeDownCast(masterNode);
  if (masterSequenceNode == nullptr)
    {
    return false;
    }
  vtkMRMLSequenceNode* imageSequenceNode = vtkMRMLSequenceNode::SafeDownCast(imageNode);

  std::stringstream defaultHeaderOutStream;
  std::string elementDataFileLine = "ElementDataFile = LOCAL";

  // If header file exists then append transform info before element data file line
  if (vtksys::SystemTools::FileExists(fileName.c_str()))
    {
    // Read the file back in
    std::ifstream headerInStream(fileName.c_str(), std::ios_base::binary);
    std::string line;
    while (std::getline(headerInStream, line))
      {
      if (line.find("ElementDataFile") == std::string::npos) // Ignore this line (since this must be last)
        {
        defaultHeaderOutStream << line << std::endl;
        }
      else
        {
        elementDataFileLine = line;
        }
      }
    headerInStream.close();
    }
  else
    {
    defaultHeaderOutStream
      << "ObjectType = Image" << std::endl
      << "NDims = 3" << std::endl
      << "AnatomicalOrientation = RAI" << std::endl
      << "BinaryData = True" << std::endl
      << "CompressedData = False" << std::endl
      << "DimSize = 0 0 " << masterSequenceNode->GetNumberOfDataNodes() << std::endl
      << "ElementSpacing = 1 1 1" << std::endl
      << "Offset = 0 0 0" << std::endl
      << "TransformMatrix = 1 0 0 0 1 0 0 0 1" << std::endl
      << "ElementType = MET_UCHAR" << std::endl
      << "Kinds = domain domain list" << std::endl;
    }

  // Append the transform information to the end of the file
  std::ofstream headerOutStream(fileName.c_str(), std::ios_base::binary);
  headerOutStream << defaultHeaderOutStream.str();

  // Add the necessary image metadata to header.
  // Other fields are already taken care of by the vtkMetaImageWriter.
  if (imageNode)
    {
    std::string ultrasoundImageOrientation = "??";
    std::string ultrasoundImageType = "BRIGHTNESS";
    if (imageNode->GetAttribute("Sequences.UltrasoundImageOrientation"))
      {
      ultrasoundImageOrientation = imageNode->GetAttribute("Sequences.UltrasoundImageOrientation");
      }
    if (imageNode->GetAttribute("Sequences.UltrasoundImageType"))
      {
      ultrasoundImageType = imageNode->GetAttribute("Sequences.UltrasoundImageType");
      }
    headerOutStream << "UltrasoundImageOrientation = " << ultrasoundImageOrientation << std::endl;
    headerOutStream << "UltrasoundImageType = BRIGHTNESS" << ultrasoundImageType << std::endl;
    }

  headerOutStream << std::setfill('0');
  // Iterate over everything in the master sequence node
  int numberOfTransforms = transformSequenceNodes.size();
  for (int frameNumber = 0; frameNumber < masterSequenceNode->GetNumberOfDataNodes(); frameNumber++)
    {
    std::string indexValue = masterSequenceNode->GetNthIndexValue(frameNumber);
    // Put all the transforms in the header
    for (int transformIndex = 0; transformIndex < numberOfTransforms; transformIndex++)
      {
      vtkMRMLSequenceNode* currSequenceNode = vtkMRMLSequenceNode::SafeDownCast(transformSequenceNodes[transformIndex]);
      std::string currTransformName = transformNames[transformIndex];

      std::string transformValue = "1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1"; // Identity
      std::string transformStatus = "INVALID";
      vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(currSequenceNode->GetDataNodeAtValue(indexValue.c_str()));
      if (transformNode != nullptr && transformNode->IsLinear())
        {
        vtkNew<vtkMatrix4x4> matrix;
        transformNode->GetMatrixTransformToParent(matrix.GetPointer());
        transformValue = vtkAddonMathUtilities::ToString(matrix.GetPointer());
        transformStatus = "OK";
        }

      headerOutStream << SEQMETA_FIELD_FRAME_FIELD_PREFIX << std::setw(4) << frameNumber << std::setw(0);
      headerOutStream << "_" << currTransformName << "Transform =" << transformValue << std::endl;
      headerOutStream << SEQMETA_FIELD_FRAME_FIELD_PREFIX << std::setw(4) << frameNumber << std::setw(0);
      headerOutStream << "_" << currTransformName << "TransformStatus = " << transformStatus << std::endl;
      }

    // The timestamp information
    headerOutStream << SEQMETA_FIELD_FRAME_FIELD_PREFIX << std::setw(4) << frameNumber << std::setw(0);
    headerOutStream << "_Timestamp = " << masterSequenceNode->GetNthIndexValue(frameNumber) << std::endl;

    // Put the image information
    if (imageNode)
      {
      std::string imageStatus = "INVALID";
      if (imageSequenceNode != NULL && imageSequenceNode->GetDataNodeAtValue(indexValue.c_str()) != NULL)
        {
        imageStatus = "OK";
        }
      headerOutStream << SEQMETA_FIELD_FRAME_FIELD_PREFIX << std::setw(4) << frameNumber << std::setw(0);
      headerOutStream << "_ImageStatus = " << imageStatus << std::endl; // TODO: Find the image status in a better way
      }
    }

  // Finally, append the element data file line at the end
  headerOutStream << elementDataFileLine;

  headerOutStream.close();
  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLLinearTransformSequenceStorageNode::ReadDataInternal(vtkMRMLNode* refNode)
{
  if (!this->CanReadInReferenceNode(refNode))
    {
    return 0;
    }

  vtkMRMLSequenceNode* seqNode = dynamic_cast<vtkMRMLSequenceNode*>(refNode);
  if (!seqNode)
    {
    vtkErrorMacro("ReadDataInternal: not a Sequence node.");
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string(""))
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  std::deque< vtkSmartPointer<vtkMRMLSequenceNode> > createdTransformNodes;
  createdTransformNodes.push_back(seqNode);
  std::map< int, std::string > frameNumberToIndexValueMap;
  std::map< std::string, std::string > imageMetaData;
  if (vtkMRMLLinearTransformSequenceStorageNode::ReadSequenceFileTransforms(fullName,
    NULL, /* additional nodes will not be added to the scene */
    createdTransformNodes,
    frameNumberToIndexValueMap, imageMetaData) == 0)
    {
    // error is logged in ReadTransforms
    return false;
    }

  /*
  std::string baseNodeName = vtksys::SystemTools::GetFilenameName(fullName);
  std::string transformsSequenceName = baseNodeName
  //+ NODE_BASE_NAME_SEPARATOR + (*createdTransformNodeIt)->GetAttribute("Sequences.Source")
  + NODE_BASE_NAME_SEPARATOR + "Seq";
  (*createdTransformNodeIt)->SetName(transformsSequenceName.c_str());
  */

  // success
  return 1;
}

//----------------------------------------------------------------------------
bool vtkMRMLLinearTransformSequenceStorageNode::CanWriteFromReferenceNode(vtkMRMLNode *refNode)
{
  vtkMRMLSequenceNode* sequenceNode = vtkMRMLSequenceNode::SafeDownCast(refNode);
  if (sequenceNode == NULL)
    {
    vtkErrorMacro("vtkMRMLLinearTransformSequenceStorageNode::CanWriteFromReferenceNode: input is not a sequence node");
    return false;
    }
  int numberOfFrameVolumes = sequenceNode->GetNumberOfDataNodes();
  for (int frameIndex = 0; frameIndex < numberOfFrameVolumes; frameIndex++)
    {
    vtkMRMLTransformNode* transform = vtkMRMLTransformNode::SafeDownCast(sequenceNode->GetNthDataNode(frameIndex));
    if (transform == NULL || !transform->IsLinear())
      {
      vtkErrorMacro("vtkMRMLLinearTransformSequenceStorageNode::CanWriteFromReferenceNode:"
        << " only linear transform nodes can be written (frame " << frameIndex << ")");
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLLinearTransformSequenceStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLSequenceNode* sequenceNode = vtkMRMLSequenceNode::SafeDownCast(refNode);
  if (sequenceNode == NULL)
    {
    vtkErrorMacro(<< "vtkMRMLLinearTransformSequenceStorageNode::WriteDataInternal: Do not recognize node type " << refNode->GetClassName());
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string(""))
    {
    vtkErrorMacro("WriteData: File name not specified");
    return 0;
    }

  std::deque< vtkMRMLSequenceNode* > transformSequenceNodes;
  transformSequenceNodes.push_back(sequenceNode);
  std::deque< std::string > transformNames;
  std::string transformName = "Unknown1ToUnknown2";
  if (refNode->GetAttribute("Sequences.Source"))
    {
    transformName = refNode->GetAttribute("Sequences.Source");
    }
  else if (refNode->GetName())
    {
    transformName = refNode->GetName();
    }
  transformNames.push_back(transformName);
  if (!vtkMRMLLinearTransformSequenceStorageNode::WriteSequenceMetafileTransforms(fullName, transformSequenceNodes, transformNames, sequenceNode, NULL))
    {
    vtkErrorMacro(<< "vtkMRMLLinearTransformSequenceStorageNode::WriteDataInternal failed");
    return 0;
    }

  this->StageWriteData(refNode);
  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLLinearTransformSequenceStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Linear transform sequence (.seq.mhd)");
  this->SupportedReadFileTypes->InsertNextValue("Linear transform sequence (.seq.mha)");
  this->SupportedReadFileTypes->InsertNextValue("Linear transform sequence (.mha)");
  this->SupportedReadFileTypes->InsertNextValue("Linear transform sequence (.mhd)");
}

//----------------------------------------------------------------------------
void vtkMRMLLinearTransformSequenceStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Linear transform sequence (.seq.mhd)");
  this->SupportedWriteFileTypes->InsertNextValue("Linear transform sequence (.seq.mha)");
  this->SupportedWriteFileTypes->InsertNextValue("Linear transform sequence (.mhd)");
  this->SupportedWriteFileTypes->InsertNextValue("Linear transform sequence (.mha)");
}

//----------------------------------------------------------------------------
const char* vtkMRMLLinearTransformSequenceStorageNode::GetDefaultWriteFileExtension()
{
  return "seq.mha";
}
