/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLModelStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLScene.h"

#include "vtkMatrix4x4.h"
#include "vtkPolyDataNormals.h"
#include "vtkStripper.h"

#include "vtkBYUReader.h" 
#include "vtkPolyDataReader.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkSTLReader.h"
// to read in free surfer files
#include "vtkFSSurfaceReader.h"
#include "vtkFSSurfaceWFileReader.h"
#include "vtkFSSurfaceScalarReader.h"
#include "vtkFSSurfaceAnnotationReader.h"
#include "vtkMRMLFreeSurferProceduralColorNode.h"
#include "vtkMRMLColorTableNode.h"

#include "vtkPolyDataWriter.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkSTLWriter.h"

#include "vtkMRMLFreeSurferProceduralColorNode.h"
#include "vtkPointData.h"

#include "itksys/SystemTools.hxx"

// Initialize static member that controls resampling -- 
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Slicer 
// development meeting, to move ijk coordinates to voxel centers."


//------------------------------------------------------------------------------
vtkMRMLModelStorageNode* vtkMRMLModelStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLModelStorageNode");
  if(ret)
    {
    return (vtkMRMLModelStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLModelStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLModelStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLModelStorageNode");
  if(ret)
    {
    return (vtkMRMLModelStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLModelStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLModelStorageNode::vtkMRMLModelStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLModelStorageNode::~vtkMRMLModelStorageNode()
{
}

void vtkMRMLModelStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLStorageNode::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLModelStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLStorageNode::PrintSelf(os,indent);

  os << indent << "FileName: " <<
    (this->FileName ? this->FileName : "(none)") << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLModelStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------
int vtkMRMLModelStorageNode::ReadData(vtkMRMLNode *refNode)
{
  if (!refNode->IsA("vtkMRMLModelNode") ) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLModelNode");
    return 0;
    }

  vtkMRMLModelNode *modelNode = dynamic_cast <vtkMRMLModelNode *> (refNode);

  std::string fullName;
  if (this->SceneRootDir != NULL && this->Scene->IsFilePathRelative(this->GetFileName())) 
    {
    fullName = std::string(this->SceneRootDir) + std::string(this->GetFileName());
    }
  else 
    {
    fullName = std::string(this->GetFileName());
    }
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  // compute file prefix
  std::string name(fullName);
  std::string::size_type loc = name.find(".");
  if( loc == std::string::npos ) 
    {
    vtkErrorMacro("ReadData: no file extension specified");
    }
  std::string extension = name.substr(loc);

  vtkDebugMacro("ReadData: extension = " << extension.c_str());
  // don't delete the polydata if reading in a scalar overlay
  if ( extension != std::string(".w") &&
       extension != std::string(".thickness") &&
       extension != std::string(".curv") &&
       extension != std::string(".avg_curv") &&
       extension != std::string(".sulc") &&
       extension != std::string(".area") &&
       extension != std::string(".aparc.annot") &&
       extension != std::string(".cma_aparc.annot") &&
       extension != std::string(".ad_aparc.annot"))
    {
    if (modelNode->GetPolyData()) 
      {
      modelNode->SetAndObservePolyData (NULL);
      }
    }

  int result = 1;
  try
    {
    if ( extension == std::string(".g")) 
      {
      vtkBYUReader *reader = vtkBYUReader::New();
      reader->SetGeometryFileName(fullName.c_str());
      reader->Update();
      modelNode->SetAndObservePolyData(reader->GetOutput());
      reader->Delete();
      }
    else if (extension == std::string(".vtk")) 
      {
      vtkPolyDataReader *reader = vtkPolyDataReader::New();
      reader->SetFileName(fullName.c_str());
      if (!reader->IsFilePolyData())
        {
        vtkErrorMacro("File " << fullName.c_str() << " is not polydata, cannot be read with this reader");
        result = 0;
        }
      else
        {
        reader->Update();
        if (reader->GetOutput() == NULL)
          {
          vtkErrorMacro("Unable to read file " << fullName.c_str());
          result = 0;
          }
        else
          {
          modelNode->SetAndObservePolyData(reader->GetOutput());
          }
        }
      reader->Delete();
      }  
    else if (extension == std::string(".vtp")) 
      {
      vtkXMLPolyDataReader *reader = vtkXMLPolyDataReader::New();
      reader->SetFileName(fullName.c_str());
      reader->Update();
      modelNode->SetAndObservePolyData(reader->GetOutput());
      reader->Delete();
      }  
    else if ( extension == std::string(".orig") ||
              extension == std::string(".inflated") ||
              extension == std::string(".sphere") ||
              extension == std::string(".white") ||
              extension == std::string(".smoothwm") ||
              extension == std::string(".pial") ) 
      {
      //read in a free surfer file
      // -- create normals and triangle strips also
      vtkFSSurfaceReader *reader = vtkFSSurfaceReader::New();
      vtkPolyDataNormals *normals = vtkPolyDataNormals::New();
      vtkStripper *stripper = vtkStripper::New();

      reader->SetFileName(fullName.c_str());
      normals->SetSplitting(0);
      normals->SetInput( reader->GetOutput() );
      stripper->SetInput( normals->GetOutput() );
      stripper->Update();
      modelNode->SetAndObservePolyData(stripper->GetOutput());

      reader->Delete();
      normals->Delete();
      stripper->Delete();
      }
    else if (extension == std::string(".thickness") ||
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
        int numVertices = modelNode->GetPolyData()->GetPointData()->GetNumberOfTuples();

        vtkFSSurfaceScalarReader *reader = vtkFSSurfaceScalarReader::New();
        reader->SetFileName(fullName.c_str());
        // the array to read into
        vtkFloatArray *floatArray = vtkFloatArray::New();
        std::string::size_type ptr = name.find_last_of(std::string("/"));
        std::string scalarName;
        if (ptr != std::string::npos)
          {
          scalarName = name.substr(++ptr);
          }
        else
          {
          scalarName = name;
          }
        floatArray->SetName(scalarName.c_str());
        reader->SetOutput(floatArray);

        reader->ReadFSScalars();

        std::cout << "Finished reading model overlay file " << fullName.c_str() << "\n\tscalars called " << scalarName.c_str() << ", adding point scalars to model node " << modelNode->GetName() << endl;
        modelNode->AddPointScalars(floatArray);
        modelNode->GetDisplayNode()->SetActiveScalarName(scalarName.c_str());
        // make sure scalars are visible
        modelNode->GetDisplayNode()->SetScalarVisibility(1);
        // set the colour look up table
        vtkMRMLFreeSurferProceduralColorNode *colorNode = vtkMRMLFreeSurferProceduralColorNode::New();
        if (extension == std::string(".thickness"))
          {
          colorNode->SetTypeToGreenRed();
          }
        else if (extension == std::string(".curv") ||
                 extension == std::string(".avg_curv") ||
                 extension == std::string(".sulc"))
          {
          colorNode->SetTypeToGreenRed();
          }
        else if (extension == std::string(".area"))
          {
          colorNode->SetTypeToRedGreen();
          }
        else if (extension == std::string(".fs"))
          {
          colorNode->SetTypeToBlueRed();
          }
        else if (extension == std::string(".retinotopy"))
          {
          vtkWarningMacro("Retinotopy color wheel not implemented yet, using default of Heat.");
          //colorNode->SetTypeToColorWheel();
          }
        vtkDebugMacro("Using color node " << colorNode->GetTypeAsIDString() << " for scalar " << scalarName.c_str());
        modelNode->GetDisplayNode()->SetAndObserveColorNodeID(colorNode->GetTypeAsIDString());
        colorNode->Delete();
        colorNode  = NULL;

        reader->Delete();
        floatArray->Delete();
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
        std::string::size_type ptr = name.find_last_of(std::string("/"));
        std::string scalarName;
        if (ptr != std::string::npos)
          {
          scalarName = name.substr(++ptr);
          }
        else
          {
          scalarName = name;
          }
        floatArray->SetName(scalarName.c_str());
        reader->SetOutput(floatArray);
        
        reader->SetNumberOfVertices(numVertices);
        int retval = reader->ReadWFile();
        if (retval != 0)
          {
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
          std::cout << "Finished reading model overlay file " << fullName.c_str() << "\n\tscalars called " << scalarName.c_str() << ", adding point scalars to model node" << endl;
          modelNode->AddPointScalars(floatArray);
          // set the active array
          modelNode->GetDisplayNode()->SetActiveScalarName(scalarName.c_str());
          // make sure scalars are visible
          modelNode->GetDisplayNode()->SetScalarVisibility(1);
          // set the colour look up table
          vtkMRMLFreeSurferProceduralColorNode *colorNode = vtkMRMLFreeSurferProceduralColorNode::New();
          colorNode->SetTypeToHeat();
          modelNode->GetDisplayNode()->SetAndObserveColorNodeID(colorNode->GetTypeAsIDString());
          colorNode->Delete();
          colorNode = NULL;
          }
        reader->Delete();
        floatArray->Delete();
        }
      else
        {
        vtkErrorMacro("Cannot read scalar overlay file '" << name.c_str() << "', as there are no points in the model " << modelNode->GetID() << " to associate it with.");
        }
      }
    else if (extension == std::string(".aparc.annot") ||
             extension == std::string(".cma_aparc.annot") ||
             extension == std::string(".ad_aparc.annot"))
      {
      // read in a FreeSurfer annotation overlay
      
      // does the model node have point data?
      if (modelNode->GetPolyData() != NULL &&
          modelNode->GetPolyData()->GetPointData() != NULL)
        {
        vtkIntArray *scalars;
        int scalaridx = modelNode->GetPolyData()->GetPointData()->SetActiveScalars("labels");
        if (scalaridx == -1)
          {
          // make a new array
          scalars = vtkIntArray::New();
          scalars->SetName("labels");
          modelNode->AddPointScalars(scalars);
          scalaridx = modelNode->GetPolyData()->GetPointData()->SetActiveScalars("labels");
          modelNode->GetDisplayNode()->SetActiveScalarName("labels");
          modelNode->GetDisplayNode()->SetScalarVisibility(1);
          scalars->Delete();
          }
        scalars = vtkIntArray::SafeDownCast(modelNode->GetPolyData()->GetPointData()->GetArray(scalaridx));

        // set up a look up table
        vtkMRMLColorTableNode *lutNode = vtkMRMLColorTableNode::New();
        lutNode->SetTypeToUser();
        this->Scene->AddNode(lutNode);
        
        vtkFSSurfaceAnnotationReader *reader = vtkFSSurfaceAnnotationReader::New();
        reader->SetFileName(fullName.c_str());
        reader->SetOutput(scalars);
        reader->SetColorTableOutput(lutNode->GetLookupTable());
        //try reading an internal colour table first
        reader->UseExternalColorTableFileOff();
        int retval = reader->ReadFSAnnotation();
        if (retval == 6)
          {
          vtkDebugMacro("No Internal Color Table in " << fullName.c_str() << ", trying the default colours");
          // use the default annotation colours
          // colorLogic->GetDefaultFreeSurferSurfaceLabelsColorNodeID()
          
          vtkCollection *labelNodes = this->Scene->GetNodesByName("FSSurfaceLabels");
          if (labelNodes->GetNumberOfItems() > 0)
            {
            labelNodes->InitTraversal();
            vtkMRMLColorTableNode *cnode = vtkMRMLColorTableNode::SafeDownCast(labelNodes->GetNextItemAsObject());
            if (cnode != NULL)
              {
              vtkWarningMacro("Could not find an internal colour table in " << fullName.c_str() << ", using default colour node " << cnode->GetName());
              modelNode->GetDisplayNode()->SetAndObserveColorNodeID(cnode->GetID());
              cnode = NULL;
              }
            }
          else
            {
            vtkErrorMacro("Unable to find an internal nor an external colour look up table for " << fullName.c_str());
            }
          }
        else
          {          
          // the color names are formatted as 'index {name} '
          char *colorNames = reader->GetColorTableNames();
          int numColours = lutNode->GetNumberOfColors();
          // set the number of colours so that can use add call to set the names
          lutNode->SetNumberOfColors(numColours);
          modelNode->GetDisplayNode()->SetScalarRange(0, numColours);
          vtkDebugMacro("Got color table names " << colorNames << ", number of colours = " << numColours << endl);
          std::string colorString = std::string(colorNames);
          std::string::size_type startBracketIndex = colorString.find("{", 0);
          std::string::size_type endBracketIndex = colorString.find( "}", 0 );
          std::string colorIndexString = colorString.substr(0, startBracketIndex - 1);
          int numNames = 0;
          while (endBracketIndex != std::string::npos && numNames < reader->GetNumColorTableEntries())
            {
            numNames++;
            std::string colorName = colorString.substr(startBracketIndex+1, endBracketIndex - startBracketIndex - 1);
            vtkDebugMacro("Adding color name = " << colorName.c_str() << " at index \"" << colorIndexString.c_str() << "\"" << ", as int: " << atoi(colorIndexString.c_str()) <<  endl);
            lutNode->SetColorName(atoi(colorIndexString.c_str()), colorName.c_str());
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
          modelNode->GetDisplayNode()->SetAndObserveColorNodeID(lutNode->GetID());
          }
        lutNode->Delete();
        reader->Delete();
        }
      }
    else if (extension == std::string(".stl")) 
      {
      vtkSTLReader *reader = vtkSTLReader::New();
      reader->SetFileName(fullName.c_str());
      modelNode->SetAndObservePolyData(reader->GetOutput());
      reader->Update();
      reader->Delete();
      }
    else 
      {
      vtkErrorMacro("Cannot read model file '" << name.c_str() << "' (extension = " << extension.c_str() << ")");
      return 0;
      }
    }
  catch (vtkstd::exception &e)
    {
    result = 0;
    }

  if (modelNode->GetPolyData() != NULL) 
    {
    modelNode->GetPolyData()->Modified();
    }
  modelNode->SetModifiedSinceRead(0);
  return result;
}

//----------------------------------------------------------------------------
int vtkMRMLModelStorageNode::WriteData(vtkMRMLNode *refNode)
{
  // test whether refNode is a valid node to hold a model
  if (!refNode->IsA("vtkMRMLModelNode") ) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLModelNode");
    return 0;
    }
  
  vtkMRMLModelNode *modelNode = vtkMRMLModelNode::SafeDownCast(refNode);
  
  std::string fullName;
  if (this->SceneRootDir != NULL && this->Scene->IsFilePathRelative(this->GetFileName())) 
    {
    fullName = std::string(this->SceneRootDir) + std::string(this->GetFileName());
    }
  else 
    {
    fullName = std::string(this->GetFileName());
    }  
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLModelNode: File name not specified");
    return 0;
    }

  std::string extension = itksys::SystemTools::GetFilenameLastExtension(fullName);

  int result = 1;
  if (extension == ".vtk")
    {
    vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
    writer->SetFileName(fullName.c_str());
    writer->SetInput( modelNode->GetPolyData() );
    try
      {
      writer->Write();
      }
    catch (vtkstd::exception &e)
      {
      result = 0;
      }
    writer->Delete();    
    }
  else if (extension == ".vtp")
    {
    vtkXMLPolyDataWriter *writer = vtkXMLPolyDataWriter::New();
    writer->SetFileName(fullName.c_str());
    writer->SetInput( modelNode->GetPolyData() );
    try
      {
      writer->Write();
      }
    catch (vtkstd::exception &e)
      {
      result = 0;
      }
    writer->Delete();    
    }
  else if (extension == ".stl")
    {
    vtkSTLWriter *writer = vtkSTLWriter::New();
    writer->SetFileName(fullName.c_str());
    writer->SetInput( modelNode->GetPolyData() );
    try
      {
      writer->Write();
      }
    catch (vtkstd::exception &e)
      {
      result = 0;
      }
    writer->Delete();    
    }
  else
    {
    result = 0;
    vtkErrorMacro( << "No file extension recognized: " << fullName.c_str() );
    }

  
  return result;
}
