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
vtkMRMLFreeSurferModelOverlayStorageNode::vtkMRMLFreeSurferModelOverlayStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLFreeSurferModelOverlayStorageNode::~vtkMRMLFreeSurferModelOverlayStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelOverlayStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
int vtkMRMLFreeSurferModelOverlayStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(refNode);

  if (modelNode == NULL ||
      modelNode->GetPolyData() == NULL ||
      modelNode->GetPolyData()->GetPointData() == NULL)
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

  // don't delete the polydata if reading in a scalar overlay

  // reset this to 0 if have an error
  int result = 1;
  try
    {
    if (extension == std::string(".thickness") ||
        extension == std::string(".curv") ||
        extension == std::string(".avg_curv") ||
        extension == std::string(".sulc") ||
        extension == std::string(".area"))
      {
      // read in a freesurfer scalar overlay
      // does the model node have point data?
      if (modelNode->GetPolyData() != NULL &&
          modelNode->GetPolyData()->GetPointData() != NULL)
        {
        vtkMRMLModelDisplayNode *displayNode = modelNode->GetModelDisplayNode();
        //int numVertices = modelNode->GetPolyData()->GetPointData()->GetNumberOfTuples();

        vtkFSSurfaceScalarReader *reader = vtkFSSurfaceScalarReader::New();
        reader->SetFileName(fullName.c_str());
        // the array to read into
        vtkFloatArray *floatArray = vtkFloatArray::New();
        std::string::size_type ptr = fullName.find_last_of(std::string("/"));
        std::string scalarName;
        if (ptr != std::string::npos)
        {
            // find the dir name above
            std::string::size_type ptrNext = ptr;
            std::string::size_type dirptr = fullName.find_last_of(std::string("/"), --ptrNext);
            if (dirptr != std::string::npos)
            {
                scalarName = fullName.substr(++dirptr);
                vtkDebugMacro("Using dir name in scalar name " << scalarName.c_str());
            }
            else
            {
                scalarName = fullName.substr(++ptr);
                vtkDebugMacro("Not using the dir name in the scalar name " << scalarName.c_str());
            }
        }
        else
        {
            scalarName = fullName;
        }
        floatArray->SetName(scalarName.c_str());
        reader->SetOutput(floatArray);

        if (reader->ReadFSScalars() == 0)
          {
          vtkDebugMacro("ReadData: error reading scalar overlay file " << fullName.c_str());
          reader->SetOutput(NULL);
          reader->Delete();
          floatArray->Delete();
          floatArray = NULL;
          return 0;
          }

        vtkDebugMacro("Finished reading model overlay file " << fullName.c_str()
                      << "\n\tscalars called " << scalarName.c_str()
                      << ", adding point scalars to model node " << modelNode->GetName());
        modelNode->AddPointScalars(floatArray);
        if (displayNode)
          {
          displayNode->SetActiveScalarName(scalarName.c_str());
          // make sure scalars are visible
          displayNode->SetScalarVisibility(1);
          }

        // set the colour look up table
        std::string colorNodeID = this->GetColorNodeIDFromExtension(extension);
        if (displayNode && !colorNodeID.empty())
          {
          vtkDebugMacro("Using color node " << colorNodeID
                        << " for scalar " << scalarName.c_str());
          displayNode->SetAndObserveColorNodeID(colorNodeID.c_str());
          }

        reader->SetOutput(NULL);
        reader->Delete();
        floatArray->Delete();
        floatArray = NULL;
        }
      }
    else if (extension == std::string(".w"))
      {
      // read in freesurfer .W file scalar overlay
      // does the model node have point data?
      if (modelNode->GetPolyData() != NULL &&
          modelNode->GetPolyData()->GetPointData() != NULL)
        {
        int numVertices = modelNode->GetPolyData()->GetPointData()->GetNumberOfTuples();

        // read in a freesurfer W scalar overlay file
        vtkFSSurfaceWFileReader *reader = vtkFSSurfaceWFileReader::New();
        reader->SetFileName(fullName.c_str());

        // the array to read into
        vtkFloatArray *floatArray = vtkFloatArray::New();
        std::string::size_type ptr = fullName.find_last_of(std::string("/"));
        std::string scalarName;
        if (ptr != std::string::npos)
          {
          scalarName = fullName.substr(++ptr);
          }
        else
          {
          scalarName = fullName;
          }
        floatArray->SetName(scalarName.c_str());
        reader->SetOutput(floatArray);

        reader->SetNumberOfVertices(numVertices);
        int retval = reader->ReadWFile();
        result = retval;
        vtkDebugMacro("Result from reading w file = " << result);
        if (retval != 0)
          {
          result = 0;
          vtkErrorMacro ("Error reading FreeSurfer W overlay file " << fullName.c_str() << ": ");
          if (retval == 1)
            {
            vtkErrorMacro ("Output is null\n");
            }
          if (retval == 2)
            {
            vtkErrorMacro ("FileName not specified\n");
            }
          if (retval == 3)
            {
            vtkErrorMacro ("Could not open file\n");
            }
          if (retval == 4)
            {
            vtkErrorMacro ("Number of values in the file is 0 or negative, or greater than number of vertices in the associated scalar file\n");
            }
          if (retval == 5)
            {
            vtkErrorMacro (" Error allocating the array of floats\n");
            }
          if (retval == 6)
            {
            vtkErrorMacro ("Unexpected EOF\n");
            }
          }
        else
          {
          result = 1;
          std::cout << "Finished reading model overlay file " << fullName.c_str() << "\n\tscalars called " << scalarName.c_str() << ", adding point scalars to model node" << endl;
          modelNode->AddPointScalars(floatArray);
          vtkMRMLModelDisplayNode *displayNode = modelNode->GetModelDisplayNode();
          // set the colour look up table
          if (displayNode)
            {
            displayNode->SetActiveScalarName(scalarName.c_str());
            // make sure scalars are visible
            displayNode->SetScalarVisibility(1);
            std::string colorNodeID = this->GetColorNodeIDFromType(
              vtkMRMLFreeSurferProceduralColorNode::Heat);
            displayNode->SetAndObserveColorNodeID(colorNodeID.c_str());
            }
          }
        reader->Delete();
        floatArray->Delete();
        }
      else
        {
        vtkErrorMacro("Cannot read scalar overlay file '" << fullName.c_str()
                      << "', as there are no points in the model " << modelNode->GetID() << " to associate it with.");
        result = 1;
        }
      }
    else if (extension == std::string(".label"))
      {
      // read in freesurfer label file scalar overlay
      // does the model node have point data?
      if (modelNode->GetPolyData() != NULL &&
          modelNode->GetPolyData()->GetPointData() != NULL)
        {
        int numVertices = modelNode->GetPolyData()->GetPointData()->GetNumberOfTuples();

        // read in a freesurfer label scalar overlay file
        vtkFSSurfaceLabelReader *reader = vtkFSSurfaceLabelReader::New();
        reader->SetFileName(fullName.c_str());

        // the array to read into
        vtkFloatArray *floatArray = vtkFloatArray::New();
        std::string::size_type ptr = fullName.find_last_of(std::string("/"));
        std::string scalarName;
        if (ptr != std::string::npos)
          {
          scalarName = fullName.substr(++ptr);
          }
        else
          {
          scalarName = fullName;
          }
        floatArray->SetName(scalarName.c_str());
        reader->SetOutput(floatArray);

        reader->SetNumberOfVertices(numVertices);
        // set the scalar values for the label overlay being read in, unknown
        // for off and cerebral cortex for on, from the freesurfer labels
        // colour file
        reader->SetLabelOff(1000.0);
        reader->SetLabelOn(3.0);

        int retval = reader->ReadLabel();
        result = retval;
        if (retval != 0)
          {
          result = 0;
          vtkErrorMacro ("Error reading FreeSurfer label overlay file " << fullName.c_str() << ": ");
          if (retval == 1)
            {
            vtkErrorMacro ("Output is null\n");
            }
          if (retval == 2)
            {
            vtkErrorMacro ("FileName not specified\n");
            }
          if (retval == 3)
            {
            vtkErrorMacro ("Could not open file\n");
            }
          if (retval == 4)
            {
            vtkErrorMacro ("Number of values in the file is 0 or negative, or greater than number of vertices in the associated scalar file\n");
            }
          if (retval == 5)
            {
            vtkErrorMacro (" Error allocating the array of floats\n");
            }
          if (retval == 6)
            {
            vtkErrorMacro ("Unexpected EOF\n");
            }
          }
        else
          {
          std::cout << "Finished reading model overlay file " << fullName.c_str() << "\n\tscalars called " << scalarName.c_str() << ", adding point scalars to model node" << endl;
          modelNode->AddPointScalars(floatArray);
          vtkMRMLModelDisplayNode *displayNode = modelNode->GetModelDisplayNode();
          // set the colour look up table
          if (displayNode)
            {
            displayNode->SetActiveScalarName(scalarName.c_str());
            // make sure scalars are visible
            displayNode->SetScalarVisibility(1);
            std::string colorNodeID = this->GetColorNodeIDFromType(
              vtkMRMLFreeSurferProceduralColorNode::Labels);
            displayNode->SetAndObserveColorNodeID(colorNodeID.c_str());
            }
          }
        reader->Delete();
        floatArray->Delete();
        }
      }
    else if (extension == std::string(".annot"))
      {
      // read in a FreeSurfer annotation overlay

      // does the model node have point data?
      if (modelNode->GetPolyData() != NULL &&
          modelNode->GetPolyData()->GetPointData() != NULL)
        {
        vtkMRMLModelDisplayNode *displayNode = modelNode->GetModelDisplayNode();
        vtkIntArray *scalars;
        int scalaridx = modelNode->GetPolyData()->GetPointData()->SetActiveScalars("labels");
        if (scalaridx == -1)
          {
          // make a new array
          scalars = vtkIntArray::New();
          scalars->SetName("labels");
          modelNode->AddPointScalars(scalars);
          scalars->Delete();
          scalaridx = modelNode->GetPolyData()->GetPointData()->SetActiveScalars("labels");
          if (displayNode)
            {
            displayNode->SetActiveScalarName("labels");
            displayNode->SetScalarVisibility(1);
            }
          }
        scalars = vtkIntArray::SafeDownCast(modelNode->GetPolyData()->GetPointData()->GetArray(scalaridx));

        // set up a look up table
        vtkMRMLColorTableNode *lutNode = vtkMRMLColorTableNode::New();
        lutNode->SetTypeToUser();

        vtkFSSurfaceAnnotationReader *reader = vtkFSSurfaceAnnotationReader::New();
        reader->SetFileName(fullName.c_str());
        reader->SetOutput(scalars);
        reader->SetColorTableOutput(lutNode->GetLookupTable());
        //try reading an internal colour table first
        reader->UseExternalColorTableFileOff();
        if (this->GetDebug())
          {
          reader->DebugOn();
          }
        int retval = reader->ReadFSAnnotation();
        if (retval != 0 && retval != 5)
          {
          result = 0;
          }

        if (retval == 0 || retval == 5)
          {
          // no error, or a warning about unassigned labels
          // the color names are formatted as 'index {name} '
          char *colorNames = reader->GetColorTableNames();
          int numColours = lutNode->GetNumberOfColors();
          // set the number of colours so that can use add call to set the names
          lutNode->SetNumberOfColors(numColours);
          modelNode->GetModelDisplayNode()->SetScalarRange(0, numColours);
          vtkDebugMacro("Got color table names " << colorNames << ", number of colours = " << numColours << endl);
          std::string colorString = std::string(colorNames);
          std::string::size_type startBracketIndex = colorString.find("{", 0);
          std::string::size_type endBracketIndex = colorString.find( "}", 0 );
          std::string colorIndexString = colorString.substr(0, startBracketIndex - 1);
          int numNames = 0;
          bool errorCondition = false;
          while (endBracketIndex != std::string::npos && numNames < reader->GetNumColorTableEntries())
            {
            numNames++;
            std::string colorName = colorString.substr(startBracketIndex+1, endBracketIndex - startBracketIndex - 1);
            vtkDebugMacro("Adding color name = " << colorName.c_str() << " at index \"" << colorIndexString.c_str() << "\"" << ", as int: " << atoi(colorIndexString.c_str()) <<  endl);
            if (lutNode->SetColorName(atoi(colorIndexString.c_str()), colorName.c_str()) == 0)
              {
              vtkErrorMacro("ReadData: error setting annotation color name " << colorName.c_str() << " at index \"" << colorIndexString.c_str() << "\"" << ", as int: " << atoi(colorIndexString.c_str()) <<", breaking the loop over " << reader->GetNumColorTableEntries() << " entries");
              errorCondition = true;
              break;
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
          if (!errorCondition)
            {
            this->Scene->AddNode(lutNode);
            modelNode->GetModelDisplayNode()->SetAndObserveColorNodeID(lutNode->GetID());
            }
          }
        else if (retval == 6)
          {
          vtkDebugMacro("No Internal Color Table in " << fullName.c_str() << ", trying the default colours");

          // clear up the internal one
          lutNode->Delete();
          lutNode = NULL;

          // use the default annotation colours
          // colorLogic->GetDefaultFreeSurferSurfaceLabelsColorNodeID()

          vtkCollection *labelNodes = this->Scene->GetNodesByName("FSLabels");
          if (labelNodes->GetNumberOfItems() > 0)
            {
            labelNodes->InitTraversal();
            vtkMRMLColorTableNode *cnode = vtkMRMLColorTableNode::SafeDownCast(labelNodes->GetNextItemAsObject());
            if (cnode != NULL)
              {
              vtkWarningMacro("Could not find an internal colour table in " << fullName.c_str() << ", using default colour node " << cnode->GetName());
              modelNode->GetModelDisplayNode()->SetAndObserveColorNodeID(cnode->GetID());
              cnode = NULL;
              // recovered, so no error condition
              result = 1;
              }
            }
          else
            {
            vtkErrorMacro("Unable to find an internal nor an external colour look up table for " << fullName.c_str());
            result = 0;
            }
          if (labelNodes)
            {
            labelNodes->RemoveAllItems();
            labelNodes->Delete();
            labelNodes = NULL;
            }
          } // no internal colour table
        else
          {
          if (retval == -1)
            {
            vtkErrorMacro("ReadFSAnnotation: memory allocation error or file name not specified");
            }
          else if (retval == 1 || retval == 3)
            {
            vtkErrorMacro("ReadFSAnnotation: error loading or parsing color table.");
            }
          else if (retval == 2)
            {
            vtkErrorMacro("ReadFSAnnotation: error opening file " << fullName.c_str());
            }
          else if (retval == 4)
            {
            vtkErrorMacro("Error parsing the annotation file");
            }
          }
        if (lutNode)
          {
          lutNode->Delete();
          }
        reader->Delete();
        }
      }
    else if (extension == std::string(".mgz") ||
             extension == std::string(".mgh"))
      {
      // read in a freesurfer scalar overlay held in a volume file
      // does the model node have point data?
      if (modelNode->GetPolyData() != NULL &&
          modelNode->GetPolyData()->GetPointData() != NULL)
        {
        int numVertices = modelNode->GetPolyData()->GetPointData()->GetNumberOfTuples();
        // read the volume
        vtkITKArchetypeImageSeriesScalarReader *reader = vtkITKArchetypeImageSeriesScalarReader::New();
        reader->SetArchetype(fullName.c_str());
        reader->SetOutputScalarTypeToNative();
        reader->SetDesiredCoordinateOrientationToNative();
        try
          {
          reader->Update();
          }
        catch (...)
          {
          vtkErrorMacro("vtkMRMLFreeSurferModelOverlayStorageNode::ReadData Cannot read scalar overlay volume file "
                        << fullName.c_str());
          reader->Delete();
          return 0;
          }
        vtkImageData *imageData = reader->GetOutput();
        if (imageData == NULL)
          {
          vtkErrorMacro("vtkMRMLFreeSurferModelOverlayStorageNode::ReadData Unable to get image data out of scalar overlay volume file "
                        << fullName.c_str());
          reader->Delete();
          return 0;
          }
        int numPoints = imageData->GetNumberOfPoints();
        vtkDebugMacro("Testing volume file for scalar overlay, num vertices = "
                      << numVertices << ", image data number of points = " << numPoints );
        // only valid if volume w*h*d == num vertices
        if (numPoints != numVertices)
          {
          vtkErrorMacro("vtkMRMLFreeSurferModelOverlayStorageNode::ReadData : volume file data size "
                        << numPoints << " not the same as model vertices " << numVertices);
          reader->Delete();
          return 0;
          }

        // put it into an array
        vtkFloatArray *floatArray = vtkFloatArray::New();
        std::string::size_type ptr = fullName.find_last_of(std::string("/"));
        std::string scalarName;
        if (ptr != std::string::npos)
          {
          // find the dir name above
          std::string::size_type ptrNext = ptr;
          std::string::size_type dirptr = fullName.find_last_of(std::string("/"), --ptrNext);
          if (dirptr != std::string::npos)
            {
            scalarName = fullName.substr(++dirptr);
            vtkDebugMacro("Using dir name in scalar name " << scalarName.c_str());
            }
          else
            {
            scalarName = fullName.substr(++ptr);
            vtkDebugMacro("Not using the dir name in the scalar name " << scalarName.c_str());
            }

          }
        else
          {
          scalarName = fullName;
          }
        floatArray->SetName(scalarName.c_str());

        // transfer the volume values to the float array
        floatArray->SetNumberOfValues(numVertices);
        float minValue = imageData->GetPointData()->GetScalars()->GetTuple1(0);
        float maxValue = imageData->GetPointData()->GetScalars()->GetTuple1(0);
        float pointValue;
        for (int i = 0; i < numVertices; i++)
          {
          pointValue = imageData->GetPointData()->GetScalars()->GetTuple1(i);
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
        modelNode->GetModelDisplayNode()->SetScalarRange(minValue, maxValue);

        // add the scalars to the model
        modelNode->AddPointScalars(floatArray);
        // set the active array
        modelNode->GetModelDisplayNode()->SetActiveScalarName(scalarName.c_str());
        // make sure scalars are visible
        modelNode->GetModelDisplayNode()->SetScalarVisibility(1);
        // use the heat colour scale
        std::string colorNodeID = this->GetColorNodeIDFromType(
          vtkMRMLFreeSurferProceduralColorNode::Heat);
        modelNode->GetModelDisplayNode()->SetAndObserveColorNodeID(colorNodeID.c_str());

        reader->Delete();
        floatArray->Delete();
        }  // end of have poly data
      }
    else
      {
      vtkErrorMacro("MRML FreeSurfer ModelStorage Node: Cannot read model file '" << fullName.c_str() << "' (extension = " << extension.c_str() << ")");
      return 0;
      }
    }
  catch (...)
    {
    result = 0;
    }

  if (modelNode->GetPolyData() != NULL)
    {
    //modelNode->GetPolyData()->Modified();
    }

  vtkDebugMacro("ReadData: Returning " << result);
  return result;
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
int vtkMRMLFreeSurferModelOverlayStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  vtkErrorMacro("Model Writing not supported for FreeSurfer models. For RemoteIO, please see the CopyData method as a possible workaround.");
  return 0;

  // test whether refNode is a valid node to hold a model
  if (!refNode->IsA("vtkMRMLModelNode") )
    {
    vtkErrorMacro("Reference node is not a vtkMRMLModelNode");
    return 0;
    }

  //vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(refNode);

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("WriteData: File name not specified");
    return 0;
    }

  std::string extension = itksys::SystemTools::GetFilenameLastExtension(fullName);

  int result = 1;

  result = 0;
  vtkErrorMacro("No Writer for file extension: " << fullName.c_str() );

  return result;
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
  copyOK = itksys::SystemTools::CopyAFile ( fullName.c_str(), newName.c_str(), 1 );
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
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Thickness (.thickness)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Curvature (.curv)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Avg. Curvature (.avg_curv)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Sulcus (.sulc)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Area (.area)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Annotation (.annot)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer MGZ (.mgz)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer MGH GZ (.mgh.gz)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer MGH (.mgh)");
  this->SupportedReadFileTypes->InsertNextValue("FreeSurfer Label (.label)");
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelOverlayStorageNode::InitializeSupportedWriteFileTypes()
{
  // Look at WriteData()
  // Not supporting any writing currently
}
