/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFreeSurferModelStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLFreeSurferModelStorageNode.h"
#include "vtkMRMLScene.h"

#include "vtkMatrix4x4.h"
#include "vtkPolyDataNormals.h"
#include "vtkStripper.h"

#include "vtkFSSurfaceReader.h"
#include "vtkFSSurfaceWFileReader.h"
#include "vtkFSSurfaceScalarReader.h"
#include "vtkFSSurfaceAnnotationReader.h"
#include "vtkMRMLFreeSurferProceduralColorNode.h"
#include "vtkMRMLColorTableNode.h"

#include "vtkPolyDataWriter.h"
#include "vtkXMLPolyDataWriter.h"

#include "vtkMRMLFreeSurferProceduralColorNode.h"
#include "vtkPointData.h"

#include "vtkITKArchetypeImageSeriesScalarReader.h"

#include "itksys/SystemTools.hxx"

#include "vtkCollection.h"
#include "vtkStdString.h"

// Initialize static member that controls resampling -- 
// old comment: "This offset will be changed to 0.5 from 0.0 per 2/8/2002 Slicer 
// development meeting, to move ijk coordinates to voxel centers."


//------------------------------------------------------------------------------
vtkMRMLFreeSurferModelStorageNode* vtkMRMLFreeSurferModelStorageNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFreeSurferModelStorageNode");
  if(ret)
    {
    return (vtkMRMLFreeSurferModelStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFreeSurferModelStorageNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLFreeSurferModelStorageNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFreeSurferModelStorageNode");
  if(ret)
    {
    return (vtkMRMLFreeSurferModelStorageNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFreeSurferModelStorageNode;
}

//----------------------------------------------------------------------------
vtkMRMLFreeSurferModelStorageNode::vtkMRMLFreeSurferModelStorageNode()
{
  this->SurfaceFileName = NULL;
  this->UseStripper = 1;
  // set up the list of known surface file extensions
  this->AddFileExtension(std::string(".orig"));
  this->AddFileExtension(std::string(".inflated"));
  this->AddFileExtension(std::string(".sphere"));
  this->AddFileExtension(std::string(".white"));
  this->AddFileExtension(std::string(".smoothwm"));
  this->AddFileExtension(std::string(".pial"));
  // set up the list of known surface overlay file extensions
  this->AddOverlayFileExtension(std::string(".w"));
  this->AddOverlayFileExtension(std::string(".thickness"));
  this->AddOverlayFileExtension(std::string(".curv"));
  this->AddOverlayFileExtension(std::string(".avg_curv"));
  this->AddOverlayFileExtension(std::string(".sulc"));
  this->AddOverlayFileExtension(std::string(".area"));
  this->AddOverlayFileExtension(std::string(".annot"));
  this->AddOverlayFileExtension(std::string(".mgz"));
  this->AddOverlayFileExtension(std::string(".mgh"));
}

//----------------------------------------------------------------------------
vtkMRMLFreeSurferModelStorageNode::~vtkMRMLFreeSurferModelStorageNode()
{
  if (this->SurfaceFileName)
    {
    delete [] this->SurfaceFileName;
    this->SurfaceFileName = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelStorageNode::WriteXML(ostream& of, int indent)
{
  // make sure that FileName is the surface file name
  if (this->SurfaceFileName != NULL &&
      this->FileName != NULL &&
      strcmp(this->SurfaceFileName, this->FileName) != 0)
    {
    vtkWarningMacro("WriteXML: resetting FileName to SurfaceFileName " << this->SurfaceFileName);
    this->SetFileName(this->SurfaceFileName);
    }
  Superclass::WriteXML(of, indent);
  if (this->SurfaceFileName != NULL)
    {
    of << " surfaceFileName=\"" << vtkMRMLNode::URLEncodeString(this->SurfaceFileName) << "\"";    
    }
  
  if (this->GetNumberOfOverlayFiles() > 0)
    {
    of << " overlays=\"";
    // URL encoded so can use spaces to separate the filenames
    for ( int i = 0; i < this->GetNumberOfOverlayFiles(); i++)
      {      
      of << vtkMRMLNode::URLEncodeString(this->GetOverlayFileName(i));
      if (i < this->GetNumberOfOverlayFiles() - 1)
        {
        // don't put a space after the last filename
        of << " ";
        }
      }
    of << "\"" << endl;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelStorageNode::ReadXMLAttributes(const char** atts)
{
  vtkDebugMacro("ReadXMLAttributes called... calling superclass");
  
  vtkMRMLStorageNode::ReadXMLAttributes(atts);
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "surfaceFileName")) 
      {
      vtkDebugMacro("Got surface file name: " << attValue);
      this->SetSurfaceFileName(vtkMRMLNode::URLDecodeString(attValue));
      }
    if (!strcmp(attName, "overlays"))
      {
      
      // one per line
      char *fileNames = (char*)attValue;
      vtkDebugMacro("Have scalar overlay file names: " << fileNames);
      char *ptr = strtok(fileNames, " ");
      vtkDebugMacro("Got file name " << ptr);
      while (ptr != NULL)
        {
        this->AddOverlayFileName(vtkMRMLNode::URLDecodeString(ptr));
        ptr = strtok(NULL, " ");
        vtkDebugMacro("\tfile name = " << ptr);
        }
      }
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, StorageID
void vtkMRMLFreeSurferModelStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);

  vtkMRMLFreeSurferModelStorageNode *node = (vtkMRMLFreeSurferModelStorageNode*)anode;
  this->SetSurfaceFileName(node->SurfaceFileName);

  for (int i = 0; i < node->GetNumberOfOverlayFiles(); i++)
    {
    this->AddOverlayFileName(node->GetOverlayFileName(i));
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLStorageNode::PrintSelf(os,indent);

  os << indent << "SurfaceFileName: " <<
    (this->SurfaceFileName ? this->SurfaceFileName : "(none)") << "\n";

  os << indent << "Overlay File Names: " << endl;
  if (this->GetNumberOfOverlayFiles() > 0)
    {    
    for ( int i = 0; i < this->GetNumberOfOverlayFiles(); i++)
      {
      os << indent << indent << this->GetOverlayFileName(i) << endl;
      }
    }
  else
    {
    os << indent << indent << "(none)" << endl;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelStorageNode::ProcessParentNode(vtkMRMLNode *parentNode)
{
  this->ReadData(parentNode);
}

//----------------------------------------------------------------------------
int vtkMRMLFreeSurferModelStorageNode::ReadData(vtkMRMLNode *refNode)
{
  if (refNode == NULL)
    {
    vtkErrorMacro("vtkMRMLFreeSurferModelStorageNode::ReadData: Reference node is null.");
    return 0;
    }
  
  if (!refNode->IsA("vtkMRMLModelNode") ) 
    {
    vtkErrorMacro("Reference node is not a vtkMRMLModelNode");
    return 0;
    }

  Superclass::StageReadData(refNode);
  if ( this->GetReadState() == this->Pending )
    {
    // remote file download hasn't finished
    return 0;
    }
  
  vtkMRMLModelNode *modelNode = dynamic_cast <vtkMRMLModelNode *> (refNode);
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string("")) 
    {
    vtkErrorMacro("ReadData: File name not specified");
    return 0;
    }

  // try to figure out if we're just reading a single file, or if it's called
  // from a SceneUpdate with geometry and scalar files set
  // if it's a geometry file and the scalar overlay list isn't empty, re-read
  // those
  int isSurfaceFile = 0;
  
  vtkDebugMacro("ReadData: reading " << fullName.c_str());
  
  // compute file prefix
  std::string name(fullName);
  std::string::size_type loc = name.find_last_of(".");
  if( loc == std::string::npos ) 
    {
    vtkErrorMacro("ReadData: no file extension specified");
    }
  std::string extension = name.substr(loc);

  vtkDebugMacro("ReadData: extension = " << extension.c_str());
  // don't delete the polydata if reading in a scalar overlay
  if ( !IsKnownOverlayFileExtension(extension) )
    {
    if (modelNode->GetPolyData()) 
      {
      modelNode->SetAndObservePolyData (NULL);
      }
    }

  int result = 1;
  try
    {
    if ( this->IsKnownFileExtension(extension))
      {
      vtkDebugMacro("Reading in a freesurfer surface file, extension = " << extension.c_str());

      //read in a free surfer file
      // -- create normals and triangle strips also
      vtkFSSurfaceReader *reader = vtkFSSurfaceReader::New();
      vtkPolyDataNormals *normals = vtkPolyDataNormals::New();
      vtkStripper *stripper = vtkStripper::New();

      reader->SetFileName(fullName.c_str());
      normals->SetSplitting(0);
      normals->SetInput( reader->GetOutput() );
      if ( this->GetUseStripper() )
        {
        stripper->SetInput( normals->GetOutput() );
        stripper->Update();
        modelNode->SetAndObservePolyData(stripper->GetOutput());
        }
      else
        {
        normals->Update();
        modelNode->SetAndObservePolyData(normals->GetOutput());
        }

      // save the surface file name, as the model storage node superclass
      // variable file name gets overwritten if we load in a scalar overlay
      this->SetSurfaceFileName(fullName.c_str());
      
      reader->Delete();
      normals->Delete();
      stripper->Delete();

      isSurfaceFile = 1;
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
        vtkMRMLModelDisplayNode *displayNode = modelNode->GetModelDisplayNode();
        int numVertices = modelNode->GetPolyData()->GetPointData()->GetNumberOfTuples();

        vtkFSSurfaceScalarReader *reader = vtkFSSurfaceScalarReader::New();
        reader->SetFileName(fullName.c_str());
        // the array to read into
        vtkFloatArray *floatArray = vtkFloatArray::New();
        std::string::size_type ptr = name.find_last_of(std::string("/"));
        std::string scalarName;
        if (ptr != std::string::npos)
        {
            // find the dir name above
            std::string::size_type ptrNext = ptr;
            std::string::size_type dirptr = name.find_last_of(std::string("/"), --ptrNext);
            if (dirptr != std::string::npos)
            {
                scalarName = name.substr(++dirptr);
                vtkDebugMacro("Using dir name in scalar name " << scalarName.c_str());
            }
            else
            {
                scalarName = name.substr(++ptr);
                vtkDebugMacro("Not using the dir name in the scalar name " << scalarName.c_str());
            }
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
        if (displayNode)
          {
          displayNode->SetActiveScalarName(scalarName.c_str());
          // make sure scalars are visible
          displayNode->SetScalarVisibility(1);
          }
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
        if (displayNode)
          {
          displayNode->SetAndObserveColorNodeID(colorNode->GetTypeAsIDString());
          }
        colorNode->Delete();
        colorNode  = NULL;

        reader->Delete();
        floatArray->Delete();
        floatArray = NULL;
        this->AddOverlayFileName(fullName.c_str());
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
        result = retval;
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
          vtkMRMLModelDisplayNode *displayNode = modelNode->GetModelDisplayNode();
          // set the colour look up table
          vtkMRMLFreeSurferProceduralColorNode *colorNode = vtkMRMLFreeSurferProceduralColorNode::New();
          colorNode->SetTypeToHeat();
          if (displayNode)
            {
            displayNode->SetActiveScalarName(scalarName.c_str());
            // make sure scalars are visible
            displayNode->SetScalarVisibility(1);
            displayNode->SetAndObserveColorNodeID(colorNode->GetTypeAsIDString());
            }
          colorNode->Delete();
          colorNode = NULL;
          }
        reader->Delete();
        floatArray->Delete();
        this->AddOverlayFileName(fullName.c_str());
        }
      else
        {
        vtkErrorMacro("Cannot read scalar overlay file '" << name.c_str() << "', as there are no points in the model " << modelNode->GetID() << " to associate it with.");
        result = 1;
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
        this->Scene->AddNode(lutNode);
        
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
        if (retval == 6)
          {
          vtkDebugMacro("No Internal Color Table in " << fullName.c_str() << ", trying the default colours");
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
              }
            }
          else
            {
            vtkErrorMacro("Unable to find an internal nor an external colour look up table for " << fullName.c_str());
            result = 6;
            }
          if (labelNodes)
            {
            labelNodes->RemoveAllItems();
            labelNodes->Delete();
            labelNodes = NULL;
            }
          }
        else
          {          
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
          modelNode->GetModelDisplayNode()->SetAndObserveColorNodeID(lutNode->GetID());
          }
        lutNode->Delete();
        reader->Delete();
        this->AddOverlayFileName(fullName.c_str());
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
        int result = 1;
        try
          {
          reader->Update();
          }
        catch (...)
          {
          vtkErrorMacro("vtkMRMLFreeSurferModelStorageNode::ReadData Cannot read scalar overlay volume file " << fullName.c_str());
          reader->Delete();
          return 0;
          }
        if (reader->GetOutput() == NULL) 
          {
          vtkErrorMacro("vtkMRMLFreeSurferModelStorageNode::ReadData Cannot read scalar overlay volume file " << fullName.c_str());
          reader->Delete();
          return 0;
          }
        vtkImageData *imageData = reader->GetOutput();
        if (imageData == NULL)
          {
          vtkErrorMacro("vtkMRMLFreeSurferModelStorageNode::ReadData Unable to get image data out of scalar overlay volume file " << fullName.c_str());
          reader->Delete();
          return 0;
          }
        int numPoints = imageData->GetNumberOfPoints();
        vtkDebugMacro("Testing volume file for scalar overlay, num vertices = " << numVertices << ", image data number of points = " << numPoints );      
        // only valid if volume w*h*d == num vertices
        if (numPoints != numVertices)
          {
          vtkErrorMacro("vtkMRMLFreeSurferModelStorageNode::ReadData : volume file data size " << numPoints << " not the same as model vertices " << numVertices);
          reader->Delete();
          return 0;
          }
        
        // put it into an array        
        vtkFloatArray *floatArray = vtkFloatArray::New();
        std::string::size_type ptr = name.find_last_of(std::string("/"));
        std::string scalarName;
        if (ptr != std::string::npos)
        {
            // find the dir name above
            std::string::size_type ptrNext = ptr;
            std::string::size_type dirptr = name.find_last_of(std::string("/"), --ptrNext);
            if (dirptr != std::string::npos)
            {
                scalarName = name.substr(++dirptr);
                vtkDebugMacro("Using dir name in scalar name " << scalarName.c_str());
            }
            else
            {
                scalarName = name.substr(++ptr);
                vtkDebugMacro("Not using the dir name in the scalar name " << scalarName.c_str());
            }

        }
        else
          {
          scalarName = name;
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
        vtkMRMLFreeSurferProceduralColorNode *colorNode = vtkMRMLFreeSurferProceduralColorNode::New();
        colorNode->SetTypeToHeat();
        modelNode->GetModelDisplayNode()->SetAndObserveColorNodeID(colorNode->GetTypeAsIDString());
        colorNode->Delete();
        colorNode = NULL;         
       
        reader->Delete();
        floatArray->Delete();
        this->AddOverlayFileName(fullName.c_str());
        }  // end of have poly data
      }
    else 
      {
      vtkErrorMacro("MRML FreeSurfer ModelStorage Node: Cannot read model file '" << name.c_str() << "' (extension = " << extension.c_str() << ")");
      return 0;
      }
    }
  catch (...)
    {
    result = 0;
    }
  
  if (modelNode->GetPolyData() != NULL) 
    {
    modelNode->GetPolyData()->Modified();
    }

  if (isSurfaceFile && this->GetNumberOfOverlayFiles() > 0)
    {
    vtkWarningMacro("Loaded a new freesurfer surface file, reloading " << this->GetNumberOfOverlayFiles() << " scalar overlays");
    int numLoaded = this->ReloadOverlayFiles(refNode);
    if (numLoaded != this->GetNumberOfOverlayFiles())
      {
      vtkErrorMacro("ReadData: only reloaded " << numLoaded << " scalars, expected " << this->GetNumberOfOverlayFiles());
      }
    }

   
  modelNode->SetModifiedSinceRead(0);
  return result;
}


//----------------------------------------------------------------------------
int vtkMRMLFreeSurferModelStorageNode::WriteData(vtkMRMLNode *refNode)
{
  vtkErrorMacro("Model Writing not supported for FreeSurfer models");
  return 0;
  
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
 
  result = 0;
  vtkErrorMacro( << "No Writer for file extension: " << fullName.c_str() );


  
  return result;
}


//----------------------------------------------------------------------------
int vtkMRMLFreeSurferModelStorageNode::GetNumberOfOverlayFiles()
{
  return this->OverlayFiles.size();
}

//----------------------------------------------------------------------------
const char *vtkMRMLFreeSurferModelStorageNode::GetOverlayFileName(int i)
{
  if (this->GetNumberOfOverlayFiles() == 0)
    {
    return NULL;
    }

  if (i < 0 || i >= this->GetNumberOfOverlayFiles())
    {
    vtkErrorMacro("GetOverlayFileName: cannot get file name " << i << ", valid range = 0-" << this->GetNumberOfOverlayFiles() - 1);
    return NULL;
    }
  return (this->OverlayFiles[i]).c_str();
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelStorageNode::AddOverlayFileName (const char *fileName)
{
  if (fileName == NULL)
    {
    return;
    }
  std::string fname = std::string(fileName);
  // is it already there?
  std::vector< std::string >::iterator iter;
  for (iter = this->OverlayFiles.begin(); iter != this->OverlayFiles.end(); ++iter)
    {
    if ((*iter) == fname)
      {
      return;
      }
    }
  vtkDebugMacro("Adding overlay file name " << fileName);
  this->OverlayFiles.push_back(fname);
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelStorageNode::RemoveOverlayFileName (const char *fileName)
{
  if (fileName == NULL)
    {
    return;
    }
  vtkDebugMacro("Removing overlay file name " << fileName);
  std::string fname = std::string(fileName);
  std::vector< std::string >::iterator iter;
  for (iter =  this->OverlayFiles.begin(); iter != this->OverlayFiles.end(); ++iter)
    {
    if ((*iter) == fname)
      {
      vtkDebugMacro("RemoveOverlayFileName: removing " << fileName);
      this->OverlayFiles.erase(iter);
      }
    }
}

//----------------------------------------------------------------------------
int vtkMRMLFreeSurferModelStorageNode::ReloadOverlayFiles(vtkMRMLNode *refNode)
{
  int numLoaded = 0;
  for (int i = 0; i < this->GetNumberOfOverlayFiles(); i++)
    {
    // reset the filename
    this->SetFileName(this->GetOverlayFileName(i));
    numLoaded += this->ReadData(refNode);
    }
  return numLoaded;
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelStorageNode::AddOverlayFileExtension(std::string ext)
{
  if (!IsKnownOverlayFileExtension(ext))
    {
    this->KnownOverlayFileExtensions.push_back(ext);
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLFreeSurferModelStorageNode::IsKnownOverlayFileExtension(std::string ext)
{
  std::vector< std::string >::iterator iter;
  for (iter = this->KnownOverlayFileExtensions.begin(); iter != this->KnownOverlayFileExtensions.end(); ++iter)
    {
    if ((*iter) == ext)
      {
      return true;
      }
    }
  return false;  
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferModelStorageNode::AddFileExtension(std::string ext)
{
  if (!IsKnownFileExtension(ext))
    {
    this->KnownFileExtensions.push_back(ext);
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLFreeSurferModelStorageNode::IsKnownFileExtension(std::string ext)
{
  std::vector< std::string >::iterator iter;
  for (iter = this->KnownFileExtensions.begin(); iter != this->KnownFileExtensions.end(); ++iter)
    {
    if ((*iter) == ext)
      {
      return true;
      }
    }
  return false;  
}


//----------------------------------------------------------------------------
int vtkMRMLFreeSurferModelStorageNode::SupportedFileType(const char *fileName)
{
  // check to see which file name we need to check
  std::string name;
  if (fileName)
    {
    name = std::string(fileName);
    }
  else if (this->FileName != NULL)
    {
    name = std::string(this->FileName);
    }
  else if (this->URI != NULL)
    {
    name = std::string(this->URI);
    }
  else
    {
    vtkWarningMacro("SupportedFileType: no file name to check");
    return 0;
    }
  
  std::string::size_type loc = name.find_last_of(".");
  if( loc == std::string::npos ) 
    {
    vtkErrorMacro("SupportedFileType: no file extension specified");
    return 0;
    }
  std::string extension = name.substr(loc);

  vtkDebugMacro("SupportedFileType: extension = " << extension.c_str());
  if ( this->IsKnownOverlayFileExtension(extension) ||
       this->IsKnownFileExtension(extension))
    {
    return 1;
    }
  else
    {
    return 0;
    }
}
