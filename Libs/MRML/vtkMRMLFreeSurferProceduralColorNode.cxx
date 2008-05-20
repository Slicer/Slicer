/*=auto=========================================================================

Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFreeSurferProceduralColorNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.0 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLFreeSurferProceduralColorNode.h"
#include "vtkMRMLScene.h"

#include "vtkFSLookupTable.h"

//------------------------------------------------------------------------------
vtkMRMLFreeSurferProceduralColorNode* vtkMRMLFreeSurferProceduralColorNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFreeSurferProceduralColorNode");
  if(ret)
    {
    return (vtkMRMLFreeSurferProceduralColorNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFreeSurferProceduralColorNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLFreeSurferProceduralColorNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFreeSurferProceduralColorNode");
  if(ret)
    {
    return (vtkMRMLFreeSurferProceduralColorNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFreeSurferProceduralColorNode;
}


//----------------------------------------------------------------------------
vtkMRMLFreeSurferProceduralColorNode::vtkMRMLFreeSurferProceduralColorNode()
{
  this->LookupTable = NULL;
  this->HideFromEditors = 1;
  this->LabelsFileName = NULL;
  this->SurfaceLabelsFileName = NULL;

  // get the home directory and the colour file in the freesurfer lib dir
  vtksys_stl::string slicerHome;
  if (vtksys::SystemTools::GetEnv("Slicer3_HOME") == NULL)
    {
    if (vtksys::SystemTools::GetEnv("PWD") != NULL)
      {
      slicerHome =  vtksys_stl::string(vtksys::SystemTools::GetEnv("PWD"));
      }
    else
      {
      slicerHome =  vtksys_stl::string("");
      }
    }
  else
    {
    slicerHome = vtksys_stl::string(vtksys::SystemTools::GetEnv("Slicer3_HOME"));
    }
  // check to see if slicer home was set
  vtksys_stl::vector<vtksys_stl::string> filesVector;
  filesVector.push_back(""); // for relative path
  filesVector.push_back(slicerHome);
  filesVector.push_back(vtksys_stl::string("share/FreeSurfer/FreeSurferColorLUT.txt"));
  vtksys_stl::string colorFileName = vtksys::SystemTools::JoinPath(filesVector);
  this->SetLabelsFileName(colorFileName.c_str());

  filesVector.pop_back();
  filesVector.push_back("share/FreeSurfer/Simple_surface_labels2002.txt");
  colorFileName = vtksys::SystemTools::JoinPath(filesVector);
  this->SetSurfaceLabelsFileName(colorFileName.c_str());
  
  //this->DebugOn();
}

//----------------------------------------------------------------------------
vtkMRMLFreeSurferProceduralColorNode::~vtkMRMLFreeSurferProceduralColorNode()
{
  if (this->LookupTable)
    {
    this->LookupTable->Delete();
    }

  if (this->LabelsFileName)
    {
    delete [] this->LabelsFileName;
    }
  if (this->SurfaceLabelsFileName)
    {
    delete [] this->SurfaceLabelsFileName;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferProceduralColorNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);
  
  // only print out the look up table if ?
  if (this->LookupTable != NULL)
    {
    of << " numcolors=\"" << this->LookupTable->GetNumberOfColors() << "\"";
    of << " colors=\"";
    for (int i = 0; i < this->LookupTable->GetNumberOfColors(); i++)
      {
      double rgb[3];
      this->LookupTable->GetColor(i, rgb);
      of <<  i << " '" << this->GetColorNameWithoutSpaces(i, "_") << "' " << rgb[0] << " " << rgb[1] << " " << rgb[2] << " 1.0 ";
      }
    of << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferProceduralColorNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  int numColors;
  while (*atts != NULL) 
  {
      attName = *(atts++);
      attValue = *(atts++);
      if (!strcmp(attName, "numcolors"))
        {
        std::stringstream ss;
        ss << attValue;
        ss >> numColors;
        vtkDebugMacro("Setting the look up table size to " << numColors << "\n");
        this->LookupTable->SetNumberOfColors(numColors);
        this->Names.clear();
        this->Names.resize(numColors);
        }
      else if (!strcmp(attName, "colors")) 
        {
        std::stringstream ss;
        for (int i = 0; i < this->LookupTable->GetNumberOfColors(); i++)
          {
          vtkDebugMacro("Reading colour " << i << " of " << this->LookupTable->GetNumberOfColors() << endl);
          ss << attValue;
          // index name r g b a
          int index;
          std::string name;
          double r, g, b, a;
          ss >> index;
          ss >> name;          
          ss >> r;
          ss >> g;
          ss >> b;
          ss >> a;
//        vtkDebugMacro("Adding colour at index " << index << ", r = " << r << ", g = " << g << ", b = " << b << ", a = " << a << " and then setting name to " << name.c_str() << endl);
//        this->LookupTable->SetTableValue(index, r, g, b, a);
          this->SetColorNameWithSpaces(index, name.c_str(), "_");
          }
        this->NamesInitialisedOn();
        }      
      else
        {
        vtkErrorMacro ("Unknown attribute name " << attName << endl);
        }
  }
  vtkDebugMacro("Finished reading in xml attributes, list id = " << this->GetID() << " and name = " << this->GetName() << endl);
}

/*
  Done via the vtkMRMLColorTableNode::ReadFile method
//----------------------------------------------------------------------------
void vtkMRMLFreeSurferProceduralColorNode::ReadFile ()
{

  // open the file for reading input
  fstream fstr;

  fstr.open(this->FileName, fstream::in);

  if (fstr.is_open())
    {
    // clear out the table
    this->SetTypeToFile();
    this->LookupTable->SetNumberOfColors(0);
    this->Names.clear();
    char line[1024];
    // save the valid lines in a vector, parse them once know the max id
    std::vector<std::string>lines;
    int maxID = 0;
    while (fstr.good())
      {
      fstr.getline(line, 1024);
      
      // does it start with a #?
      if (line[0] == '#')
        {
        vtkDebugMacro("Comment line, skipping:\n\"" << line << "\"");
        }
      else
        {
        // is it empty?
        if (line[0] == '\0')
          {
          vtkDebugMacro("Empty line, skipping:\n\"" << line << "\"");
          }
        else
          {
          vtkDebugMacro("got a line: \n\"" << line << "\"");
          lines.push_back(std::string(line));
          std::stringstream ss;
          ss << line;
          int id;
          ss >> id;
          if (id > maxID)
            {
            maxID = id;
            }
          }
        }
      }
    fstr.close();
    // now parse out the valid lines and set up the colour lookup table
    vtkDebugMacro("The largest id is " << maxID);
    // for zero
    this->LookupTable->SetNumberOfColors(maxID + 1);
    this->LookupTable->SetRange(0, maxID);
    // init the table to black/opactity 0, just in case we're missing values
    for (int i = 0; i < maxID+1; i++)
      {
//      this->LookupTable->SetTableValue(i, 0.0, 0.0, 0.0, 0.0);
      } 
    this->Names.resize(maxID + 1);
    for (unsigned int i = 0; i < lines.size(); i++)
      {
      std::stringstream ss;
      ss << lines[i];
      int id;
      std::string name;
      double r, g, b, a;
      ss >> id;
      ss >> name;
      ss >> r;
      ss >> g;
      ss >> b;
      ss >> a;
      vtkDebugMacro("id " << id << ", name = " << name.c_str() << ", r = " << r << ", g = " << g << ", b = " << b << ", a = " << a);
      // the file values are 0-255, colour look up table needs 0-1
      r = r / 255.0;
      g = g / 255.0;
      b = b / 255.0;
      a = a / 255.0;
      vtkDebugMacro("Adding colour at id " << id << " and then pushing name " << name.c_str());
      //this->LookupTable->SetTableValue(id, r, g, b, a);
      this->SetColorName(id, name.c_str());
      }
    }
  else
    {
    vtkErrorMacro ("ERROR opening colour file " << this->FileName << endl);
    }
}
*/

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFreeSurferProceduralColorNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLFreeSurferProceduralColorNode *node = (vtkMRMLFreeSurferProceduralColorNode *) anode;

  if (node != NULL)
    {
    this->SetName(node->Name);
    this->SetLookupTable(node->LookupTable);
    this->SetType(node->Type);
    this->SetFileName(node->FileName);
    }
  else
    {
    vtkErrorMacro("Copy: unable to cast a vtkMRMLNode to a vtkMRMLFreeSurferProceduralColorNode for node id = " << anode->GetID());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferProceduralColorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

  if (this->LookupTable != NULL)
    {
    os << indent << "Look up table:\n";
    this->LookupTable->PrintSelf(os, indent.GetNextIndent());
    }
  if (this->LabelsFileName)
    {
    os << indent << "Volume label map color file: " << this->GetLabelsFileName() << endl;
    }
  if (this->SurfaceLabelsFileName)
    {
    os << indent << "Surface label map color file: " << this->GetSurfaceLabelsFileName() << endl;
    }
}

//-----------------------------------------------------------

void vtkMRMLFreeSurferProceduralColorNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
  /*
    if (this->GetStorageNodeID() == NULL) 
    {
    //vtkErrorMacro("No reference StorageNodeID found");
    return;
    }
    
    vtkMRMLNode* mnode = scene->GetNodeByID(this->StorageNodeID);
    if (mnode) 
    {
    vtkMRMLStorageNode *node  = dynamic_cast < vtkMRMLStorageNode *>(mnode);
    node->ReadData(this);
    //this->SetAndObservePolyData(this->GetPolyData());
    }
  */
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferProceduralColorNode::SetTypeToHeat()
{
  this->SetType(this->Heat);
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferProceduralColorNode::SetTypeToBlueRed()
{
  this->SetType(this->BlueRed);
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferProceduralColorNode::SetTypeToRedBlue()
{
  this->SetType(this->RedBlue);
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferProceduralColorNode::SetTypeToRedGreen()
{
  this->SetType(this->RedGreen);
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferProceduralColorNode::SetTypeToGreenRed()
{
  this->SetType(this->GreenRed);
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferProceduralColorNode::SetTypeToLabels()
{
  this->SetType(this->Labels);
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferProceduralColorNode::SetTypeToSurfaceLabels()
{
  this->SetType(this->SurfaceLabels);
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferProceduralColorNode::SetTypeToCustom()
{
  this->SetType(this->Custom);
}

//----------------------------------------------------------------------------
const char* vtkMRMLFreeSurferProceduralColorNode::GetTypeAsString()
{
  if (this->Type == this->Heat)
    {
    return "Heat";
    }
  if (this->Type == this->BlueRed)
    {
    return "BlueRed";
    }
  if (this->Type == this->RedBlue)
    {
    return "RedBlue";
    }
  if (this->Type == this->RedGreen)
    {
    return "RedGreen";
    }
  if (this->Type == this->GreenRed)
    {
    return "GreenRed";
    }
  if (this->Type == this->Labels)
    {
    return "Labels";
    }
  if (this->Type == this->SurfaceLabels)
    {
    return "SurfaceLabels";
    }
  if (this->Type == this->Custom)
    {
    return "Custom";
    }
  return "(unknown)";
}

//----------------------------------------------------------------------------
const char* vtkMRMLFreeSurferProceduralColorNode::GetTypeAsIDString()
{
  if (this->Type == this->Heat)
    {
    return "vtkMRMLFreeSurferProceduralColorNodeHeat";
    }
  if (this->Type == this->BlueRed)
    {
    return "vtkMRMLFreeSurferProceduralColorNodeBlueRed";
    }
  if (this->Type == this->RedBlue)
    {
    return "vtkMRMLFreeSurferProceduralColorNodeRedBlue";
    }
  if (this->Type == this->RedGreen)
    {
    return "vtkMRMLFreeSurferProceduralColorNodeRedGreen";
    }
  if (this->Type == this->GreenRed)
    {
    return "vtkMRMLFreeSurferProceduralColorNodeGreenRed";
    }
  // these two are not held in this node, but use this node to define constants
  if (this->Type == this->Labels)
    {
    return "vtkMRMLFreeSurferColorNodeLabels";
    }
  if (this->Type == this->SurfaceLabels)
    {
    return "vtkMRMLFreeSurferColorNodeSurfaceLabels";
    }
  // custom will have a unique id
  return "(unknown)";
}

//---------------------------------------------------------------------------
void vtkMRMLFreeSurferProceduralColorNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
/*
  vtkMRMLFreeSurferProceduralColorDisplayNode *dnode = this->GetDisplayNode();
  if (dnode != NULL && dnode == vtkMRMLFreeSurferProceduralColorDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
        this->InvokeEvent(vtkMRMLFreeSurferProceduralColorNode::DisplayModifiedEvent, NULL);
    }
*/
  return;
}

//---------------------------------------------------------------------------
void vtkMRMLFreeSurferProceduralColorNode::SetType(int type)
{
  if (this->GetLookupTable() != NULL &&
      this->Type == type)
    {
    vtkDebugMacro("SetType: type is already set to " << type <<  " = " << this->GetTypeAsString());
    return;
    }
    
    this->Type = type;

    vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Type to " << type << " = " << this->GetTypeAsString());

    //this->LookupTable->Delete();
    if (this->GetLookupTable() == NULL)
      {
      vtkDebugMacro("vtkMRMLFreeSurferProceduralColorNode::SetType Creating a new lookup table (was null) of type " << this->GetTypeAsString() << "\n");
      vtkFSLookupTable * table = vtkFSLookupTable::New();
      if (table != NULL)
        {
        this->SetLookupTable(table);
        table->Delete();
        // as a default, set the table range to 255
        this->GetFSLookupTable()->SetRange(0, 255);
        }
      else
        {
        vtkErrorMacro("SetType: Failed to make a new vtkFSLookupTable!");
        return;
        }
      }
    
    if (this->Type == this->Heat)
      {
      this->GetFSLookupTable()->SetLutTypeToHeat();    
      this->SetNamesFromColors();
      }
    else if (this->Type == this->BlueRed)
      {
      this->GetFSLookupTable()->SetLutTypeToBlueRed();
      this->SetNamesFromColors();
      }

    else if (this->Type == this->RedBlue)
      {
      this->GetFSLookupTable()->SetLutTypeToRedBlue();     
      this->SetNamesFromColors();
      }

    else if (this->Type == this->RedGreen)
      {
      this->GetFSLookupTable()->SetLutTypeToRedGreen();
      this->SetNamesFromColors();
      }
    else if (this->Type == this->GreenRed)
      {
      this->GetFSLookupTable()->SetLutTypeToGreenRed();
      this->SetNamesFromColors();
      }
    else if (this->Type == this->Labels ||
             this->Type == this->SurfaceLabels ||
             this->Type == this->Custom)
      {
      // do nothing
      }
    else
      {
      vtkErrorMacro ("vtkMRMLFreeSurferProceduralColorNode: SetType ERROR, unknown type " << type << endl);
      return;
      }
    // invoke a modified event
    this->Modified();
    
    // invoke a type  modified event
    this->InvokeEvent(vtkMRMLFreeSurferProceduralColorNode::TypeModifiedEvent);
}

//---------------------------------------------------------------------------
void vtkMRMLFreeSurferProceduralColorNode::SetNamesFromColors()
{
  int size = this->GetFSLookupTable()->GetNumberOfColors();
  double rgb[3];
  // reset the names
  this->Names.clear();
  this->Names.resize(size);
  for (int i = 0; i < size; i++)
    {
    this->GetFSLookupTable()->GetColor((double)i, rgb);
    std::stringstream ss;
    ss << "R=";
    ss << rgb[0];
    ss << " G=";
    ss << rgb[1];
    ss << " B=";
    ss << rgb[2];
    ss << " A=";
    ss << 1.0;
    vtkDebugMacro("SetNamesFromColors: " << i << " Name = " << ss.str().c_str());
    this->SetColorName(i, ss.str().c_str());
    }
  this->NamesInitialisedOn();
}

//---------------------------------------------------------------------------
vtkFSLookupTable *vtkMRMLFreeSurferProceduralColorNode::GetFSLookupTable()
{
  return this->LookupTable;
}

//---------------------------------------------------------------------------
vtkLookupTable * vtkMRMLFreeSurferProceduralColorNode::GetLookupTable()
{
  if (this->LookupTable == NULL)
    {
    return NULL;
    }

  // otherwise have to cast it
  // return (vtkLookupTable *)(this->LookupTable);
  if (vtkLookupTable::SafeDownCast(this->LookupTable) == NULL)
    {
    vtkErrorMacro("GetLookupTable: error converting fs lookup table to vtk look up table.\n");
    }
  return vtkLookupTable::SafeDownCast(this->LookupTable);
}
