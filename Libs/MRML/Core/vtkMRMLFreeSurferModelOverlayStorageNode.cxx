/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFreeSurferModelOverlayStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// FreeSurfer includes
#include <vtkFSSurfaceWFileReader.h>
#include <vtkFSSurfaceLabelReader.h>
#include <vtkFSSurfaceScalarReader.h>
#include <vtkFSSurfaceAnnotationReader.h>

// MRML includes
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLFreeSurferModelOverlayStorageNode.h"
#include "vtkMRMLFreeSurferProceduralColorNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLScene.h"

// VTK ITK includes
#include "vtkITKArchetypeImageSeriesScalarReader.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkStringArray.h>
#include <vtksys/SystemTools.hxx>

// ITKSys includes

// STD includes

// Initialize static member that controls resampling --
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Slicer
// development meeting, to move ijk coordinates to voxel centers."


//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLFreeSurferModelOverlayStorageNode);

//----------------------------------------------------------------------------
vtkMRMLFreeSurferModelOverlayStorageNode::vtkMRMLFreeSurferModelOverlayStorageNode() = default;

//----------------------------------------------------------------------------
vtkMRMLFreeSurferModelOverlayStorageNode::~vtkMRMLFreeSurferModelOverlayStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelOverlayStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLFreeSurferModelOverlayStorageNode::ReadScalarOverlay(const std::string& fullName, vtkMRMLModelNode* modelNode)
{
  // the array to read into
  vtkNew<vtkFloatArray> floatArray;
  std::string scalarName = vtksys::SystemTools::GetFilenameName(fullName);
  floatArray->SetName(scalarName.c_str());
  int numVertices = modelNode->GetPolyData()->GetPointData()->GetNumberOfTuples();

  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  int errorCode = -1;
  if (extension == std::string(".w"))
    {
    vtkNew<vtkFSSurfaceWFileReader> reader;
    reader->SetFileName(fullName.c_str());
    reader->SetNumberOfVertices(numVertices);
    reader->SetOutput(floatArray.GetPointer());
    errorCode = reader->ReadWFile();
    }
  else if (extension == std::string(".label"))
    {
    vtkNew<vtkFSSurfaceLabelReader> reader;
    reader->SetFileName(fullName.c_str());
    reader->SetNumberOfVertices(numVertices);
    reader->SetOutput(floatArray.GetPointer());

    // set the scalar values for the label overlay being read in, unknown
    // for off and cerebral cortex for on, from the freesurfer labels
    // colour file
    reader->SetLabelOff(1000.0);
    reader->SetLabelOn(3.0);

    errorCode = reader->ReadLabel();
    }
  else // .thickness, .curv, .avg_curv, .sulc, .area, or anything else
    {
    vtkNew<vtkFSSurfaceScalarReader> reader;
    reader->SetFileName(fullName.c_str());
    reader->SetOutput(floatArray.GetPointer());
    // reader->ReadFSScalars() returns 0 on error, 1 on success
    errorCode = (reader->ReadFSScalars() == 0 ? -1 : 0);
    }

  if (errorCode != 0)
    {
    // Reading failed
    std::string errorDetail;
    switch (errorCode)
      {
      case 1: errorDetail = "Output is null"; break;
      case 2: errorDetail = "FileName not specified"; break;
      case 3: errorDetail = "Could not open file"; break;
      case 4: errorDetail = "Number of values in the file is 0 or negative, or greater than number of vertices in the associated scalar file"; break;
      case 5: errorDetail = "Error allocating the array of floats"; break;
      case 6: errorDetail = "Unexpected EOF"; break;
      default: errorDetail = "Unknown error"; break;
      }
    vtkErrorMacro("Error reading FreeSurfer scalar overlay file " << fullName.c_str() << ": " << errorDetail << " (" << errorCode << ")");
    return false;
    }

  // Reading was successful
  vtkDebugMacro("Finished reading FreeSurfer model scalar overlay file " << fullName.c_str()
    << "\n\tscalars called " << scalarName.c_str()
    << ", adding point scalars to model node " << modelNode->GetName());
  modelNode->AddPointScalars(floatArray.GetPointer());

  // make sure scalars are visible
  vtkMRMLModelDisplayNode *displayNode = modelNode->GetModelDisplayNode();
  if (displayNode)
    {
    std::string colorNodeID = this->GetColorNodeIDFromType(vtkMRMLFreeSurferProceduralColorNode::RedGreen);
    if (extension == std::string(".w"))
      {
      colorNodeID = this->GetColorNodeIDFromType(vtkMRMLFreeSurferProceduralColorNode::Heat);
      }
    else if (extension == std::string(".label"))
      {
      colorNodeID = this->GetColorNodeIDFromType(vtkMRMLFreeSurferProceduralColorNode::Labels);
      }
    else // .thickness, .curv, .avg_curv, .sulc, .area, or anything else
      {
      // set the colour look up table
      std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
      std::string colorNodeID = this->GetColorNodeIDFromExtension(extension);
      if (!colorNodeID.empty())
        {
        vtkDebugMacro("Using color node " << colorNodeID << " for scalar " << scalarName.c_str());
        }
      }
    displayNode->SetAndObserveColorNodeID(colorNodeID.c_str());
    displayNode->SetActiveScalarName(scalarName.c_str());
    displayNode->SetScalarVisibility(1);
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLFreeSurferModelOverlayStorageNode::ReadScalarOverlayAnnot(const std::string& fullName, vtkMRMLModelNode* modelNode)
{
  int scalaridx = modelNode->GetPolyData()->GetPointData()->SetActiveScalars("labels");
  if (scalaridx == -1)
    {
    // make a new array
    vtkNew<vtkIntArray> newScalars;
    newScalars->SetName("labels");
    modelNode->AddPointScalars(newScalars.GetPointer());
    scalaridx = modelNode->GetPolyData()->GetPointData()->SetActiveScalars("labels");
    }
  vtkIntArray* scalars = vtkIntArray::SafeDownCast(modelNode->GetPolyData()->GetPointData()->GetArray(scalaridx));
  if (scalars == nullptr)
    {
    vtkErrorMacro("vtkMRMLFreeSurferModelOverlayStorageNode::ReadScalarOverlayAnnot failed: invalid scalars array");
    return false;
    }

  // set up a look up table
  vtkNew<vtkMRMLColorTableNode> lutNode;
  lutNode->SetTypeToUser();

  vtkNew<vtkFSSurfaceAnnotationReader> reader;
  reader->SetFileName(fullName.c_str());
  reader->SetOutput(scalars);
  reader->SetColorTableOutput(lutNode->GetLookupTable());
  //try reading an internal colour table first
  reader->UseExternalColorTableFileOff();
  reader->SetDebug(this->GetDebug());
  int errorCode = reader->ReadFSAnnotation();

  if (errorCode != 0 && errorCode != 5 /* unassigned labels warning */ && errorCode != 6 /* no internal color table */)
    {
    // Reading failed
    std::string errorDetail;
    switch (errorCode)
      {
      case -1: errorDetail = "memory allocation error or file name not specified"; break;
      case 1: errorDetail = "error loading or parsing color table."; break;
      case 2: errorDetail = "error opening file"; break;
      case 3: errorDetail = "error loading or parsing color table."; break;
      case 4: errorDetail = "error parsing the annotation file"; break;
      default: errorDetail = "Unknown error"; break;
      }
    vtkErrorMacro("Error reading FreeSurfer annot file " << fullName.c_str() << ": " << errorDetail << " (" << errorCode << ")");
    return false;
    }

  std::string colorNodeId;

  if (errorCode == 6)
    {
    vtkDebugMacro("No Internal Color Table in " << fullName.c_str() << ", trying the default colours");
    vtkMRMLColorTableNode *cnode = nullptr;
    vtkSmartPointer<vtkCollection> labelNodes = vtkSmartPointer<vtkCollection>::Take(this->Scene->GetNodesByClassByName("vtkMRMLColorTableNode", "FSLabels"));
    if (labelNodes->GetNumberOfItems() > 0)
      {
      cnode = vtkMRMLColorTableNode::SafeDownCast(labelNodes->GetItemAsObject(0));
      }
    if (cnode == nullptr)
      {
      vtkErrorMacro("Unable to find an internal nor an external colour look up table for " << fullName.c_str());
      return false;
      }
    vtkWarningMacro("Could not find an internal colour table in " << fullName.c_str() << ", using default colour node " << cnode->GetName());
    colorNodeId = cnode->GetID();
    }
  else
    {
    // no error, or just a warning about unassigned labels

    // set the number of colours so that can use add call to set the names
    int numColours = lutNode->GetNumberOfColors();
    lutNode->SetNumberOfColors(numColours);

    std::string colorString = reader->GetColorTableNames();
    vtkDebugMacro("Got color table names " << colorString << ", number of colours = " << numColours << endl);

    // color names are formatted as 'index {name} '
    std::string::size_type startBracketIndex = colorString.find("{", 0);
    std::string::size_type endBracketIndex = colorString.find( "}", 0 );
    std::string colorIndexString = colorString.substr(0, startBracketIndex - 1);
    int numNames = 0;
    while (endBracketIndex != std::string::npos && numNames < reader->GetNumColorTableEntries())
      {
      numNames++;
      std::string colorName = colorString.substr(startBracketIndex+1, endBracketIndex - startBracketIndex - 1);
      vtkDebugMacro("Adding color name = " << colorName.c_str() << " at index \""
        << colorIndexString.c_str() << "\"" << ", as int: " << atoi(colorIndexString.c_str()) <<  endl);
      if (lutNode->SetColorName(atoi(colorIndexString.c_str()), colorName.c_str()) == 0)
        {
        vtkErrorMacro("ReadData: error setting annotation color name " << colorName.c_str()
          << " at index \"" << colorIndexString.c_str() << "\"" << ", as int: "
          << atoi(colorIndexString.c_str()) <<", breaking the loop over " << reader->GetNumColorTableEntries() << " entries");
        return false;
        }
      startBracketIndex = colorString.find("{", endBracketIndex);
      if (startBracketIndex != std::string::npos)
        {
        colorIndexString = colorString.substr(endBracketIndex + 1, startBracketIndex - endBracketIndex - 1);
        }
      else
        {
        colorIndexString = colorString.substr(endBracketIndex + 1);
        }
      endBracketIndex = colorString.find( "}", startBracketIndex);
      }
    lutNode->SetNamesInitialised(true);

    if (modelNode->GetModelDisplayNode())
      {
      modelNode->GetModelDisplayNode()->SetScalarRange(0, numColours);
      }

    // Set color node as the file name, including extension (.annot)
    std::string lutNodeName = vtksys::SystemTools::GetFilenameName(fullName);
    lutNode->SetName(lutNodeName.c_str());
    this->Scene->AddNode(lutNode.GetPointer());
    colorNodeId = lutNode->GetID();
    }

  vtkMRMLModelDisplayNode *displayNode = modelNode->GetModelDisplayNode();
  if (displayNode)
    {
    displayNode->SetAndObserveColorNodeID(colorNodeId);
    displayNode->SetActiveScalarName("labels");
    displayNode->SetScalarVisibility(1);
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLFreeSurferModelOverlayStorageNode::ReadScalarOverlayVolume(const std::string& fullName, vtkMRMLModelNode* modelNode)
{
  // read the volume
  vtkNew<vtkITKArchetypeImageSeriesScalarReader> reader;
  reader->SetArchetype(fullName.c_str());
  reader->SetOutputScalarTypeToNative();
  reader->SetDesiredCoordinateOrientationToNative();
  try
    {
    reader->Update();
    }
  catch (...)
    {
    vtkErrorMacro("vtkMRMLFreeSurferModelOverlayStorageNode::ReadScalarOverlayVolume Cannot read scalar overlay volume file "
                  << fullName.c_str());
    return false;
    }
  vtkImageData *imageData = reader->GetOutput();
  if (imageData == nullptr)
    {
    vtkErrorMacro("vtkMRMLFreeSurferModelOverlayStorageNode::ReadScalarOverlayVolume Unable to get image data out of scalar overlay volume file "
                  << fullName.c_str());
    return false;
    }

  // only valid if volume w*h*d == num vertices
  int numPoints = imageData->GetNumberOfPoints();
  int numVertices = modelNode->GetPolyData()->GetPointData()->GetNumberOfTuples();
  vtkDebugMacro("Testing volume file for scalar overlay, num vertices = " << numVertices << ", image data number of points = " << numPoints );
  if (numPoints != numVertices)
    {
    vtkErrorMacro("vtkMRMLFreeSurferModelOverlayStorageNode::ReadData : volume file data size "
                  << numPoints << " not the same as model vertices " << numVertices);
    return false;
    }

  // transfer the volume values to the float array
  vtkNew<vtkFloatArray> floatArray;
  std::string scalarName = vtksys::SystemTools::GetFilenameName(fullName);
  floatArray->SetName(scalarName.c_str());
  floatArray->SetNumberOfValues(numVertices);
  float minValue = imageData->GetPointData()->GetScalars()->GetTuple1(0);
  float maxValue = imageData->GetPointData()->GetScalars()->GetTuple1(0);
  for (int i = 0; i < numVertices; i++)
    {
    float pointValue = imageData->GetPointData()->GetScalars()->GetTuple1(i);
    floatArray->SetValue(i, pointValue);
    if (pointValue < minValue)
      {
      minValue = pointValue;
      }
    if (pointValue > maxValue)
      {
      maxValue = pointValue;
      }
    }
  vtkDebugMacro("ReadData: Setting scalar range, using min value = " << minValue << ", max value = " << maxValue);
  modelNode->AddPointScalars(floatArray.GetPointer());

  vtkMRMLModelDisplayNode *displayNode = modelNode->GetModelDisplayNode();
  if (displayNode)
    {
    std::string colorNodeID = this->GetColorNodeIDFromType(vtkMRMLFreeSurferProceduralColorNode::Heat);
    displayNode->SetAndObserveColorNodeID(colorNodeID);
    displayNode->SetActiveScalarName(scalarName.c_str());
    displayNode->SetScalarVisibility(1);
    displayNode->SetScalarRange(minValue, maxValue);
    }

  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLFreeSurferModelOverlayStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(refNode);

  if (modelNode == nullptr ||
      modelNode->GetPolyData() == nullptr ||
      modelNode->GetPolyData()->GetPointData() == nullptr)
    {
    vtkErrorMacro("ReadData: the model node doesn't have poly data yet, resetting the overlay read state to pending, try again later.");
    this->SetReadStatePending();
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  // try to figure out if we're just reading a single file, or if it's called
  // from a SceneUpdate with geometry and scalar files set
  // if it's a geometry file and the scalar overlay list isn't empty, re-read
  // those
  //int isSurfaceFile = 0;

  vtkDebugMacro("ReadData: reading " << fullName.c_str());

  // compute file prefix
  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  vtkDebugMacro("ReadData: extension = " << extension.c_str());

  bool success = false;
  try
    {
    if (extension == std::string(".annot"))
      {
      // read in a FreeSurfer annotation overlay
      success = this->ReadScalarOverlayAnnot(fullName, modelNode);
      }
    else if (extension == std::string(".mgz") ||
             extension == std::string(".mgh"))
      {
      // read in a freesurfer scalar overlay held in a volume file
      success = this->ReadScalarOverlayVolume(fullName, modelNode);
      }
    else
      {
      // read in as a freesurfer scalar overlay
      success = this->ReadScalarOverlay(fullName, modelNode);
      }
    }
  catch (...)
    {
    return 0;
    }

  vtkDebugMacro("ReadData: success");
  return (success ? 1 : 0);
}

//----------------------------------------------------------------------------
std::string vtkMRMLFreeSurferModelOverlayStorageNode
::GetColorNodeIDFromExtension(const std::string& extension)
{
  std::string lowercaseExtension = vtksys::SystemTools::LowerCase(extension);
  int type = vtkMRMLFreeSurferProceduralColorNode::Heat;
  if (lowercaseExtension == std::string(".thickness"))
    {
    type = vtkMRMLFreeSurferProceduralColorNode::GreenRed;
    }
  else if (lowercaseExtension == std::string(".curv") ||
           lowercaseExtension == std::string(".avg_curv") ||
           lowercaseExtension == std::string(".sulc"))
    {
    type = vtkMRMLFreeSurferProceduralColorNode::GreenRed;
    }
  else if (lowercaseExtension == std::string(".area"))
    {
    type = vtkMRMLFreeSurferProceduralColorNode::RedGreen;
    }
  else if (lowercaseExtension == std::string(".fs"))
    {
    type = vtkMRMLFreeSurferProceduralColorNode::BlueRed;
    }
  else if (lowercaseExtension == std::string(".retinotopy"))
    {
    vtkWarningMacro("Retinotopy color wheel not implemented yet, using default of Heat.");
    //type = type = vtkMRMLFreeSurferProceduralColorNode::ColorWheel();
    }
  return this->GetColorNodeIDFromType(type);
}

//----------------------------------------------------------------------------
std::string vtkMRMLFreeSurferModelOverlayStorageNode
::GetColorNodeIDFromType(int type)
{
  if (!this->GetScene())
    {
    return std::string();
    }
  std::vector<vtkMRMLNode *> colorNodes;
  this->GetScene()->GetNodesByClass("vtkMRMLFreeSurferProceduralColorNode",  colorNodes);
  for (unsigned int i = 0; i < colorNodes.size(); ++i)
    {
    vtkMRMLFreeSurferProceduralColorNode* colorNode =
      vtkMRMLFreeSurferProceduralColorNode::SafeDownCast(colorNodes[i]);
    if (colorNode->GetType() == type)
      {
      return std::string(colorNode->GetID());
      }
    }
  vtkErrorMacro( << "Can't find free surfer color node of type: " << type);
  return std::string();
}

//----------------------------------------------------------------------------
bool vtkMRMLFreeSurferModelOverlayStorageNode
::CanWriteFromReferenceNode(vtkMRMLNode *vtkNotUsed(refNode))
{
  return false;
}

//----------------------------------------------------------------------------
int vtkMRMLFreeSurferModelOverlayStorageNode::WriteDataInternal(vtkMRMLNode * vtkNotUsed(refNode))
{
  vtkErrorMacro("Model Writing not supported for FreeSurfer models. For RemoteIO, please see the CopyData method as a possible workaround.");
  return 0;
}

//----------------------------------------------------------------------------
int vtkMRMLFreeSurferModelOverlayStorageNode::CopyData(vtkMRMLNode *refNode,
                                                       const char *newFileName)
{
  bool copyOK;

  // test whether refNode is a valid node to hold a model
  if (!refNode->IsA("vtkMRMLModelNode") )
    {
    vtkErrorMacro("Reference node is not a vtkMRMLModelNode");
    return 0;
    }

//  vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(refNode);
  std::string newName = newFileName;
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLFreeSurferModelOverlayNode: File name not specified");
    return 0;
    }
  if ( newName.empty())
    {
    vtkErrorMacro("vtkMRMLFreeSurferModelOverlayNode: Copy-to file name not specified");
    return 0;
    }
  if (fullName == newName )
    {
    vtkWarningMacro("vtkMRMLFreeSurferModelOverlayNode: Copy-to file name and Copy-from file names are identical");
    //return 1;
    }

  //--- try copying to destination always
  copyOK = itksys::SystemTools::CopyAFile ( fullName.c_str(), newName.c_str(), true );
  //--- try copying to destination if different
  //tst = itksys::SystemTools::CopyAFile ( fullName.c_str(), newName.c_str(), 0 );

  if ( !copyOK )
    {
    return ( 0 );
    }

  //--- if copy worked, change filename, then upload.
  this->SetFileName ( newName.c_str() );
  this->StageWriteData(refNode);
  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelOverlayStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer W file (.w)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Label (.label)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Thickness (.thickness)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Curvature (.curv)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Avg. Curvature (.avg_curv)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Sulcus (.sulc)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Area (.area)");

  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Annotation (.annot)");

  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer MGZ (.mgz)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer MGH GZ (.mgh.gz)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer MGH (.mgh)");

}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelOverlayStorageNode::InitializeSupportedWriteFileTypes()
{
  // Look at WriteData()
  // Not supporting any writing currently.
  // Created color table files are saved by their own storage node, in Slicer format.
}
