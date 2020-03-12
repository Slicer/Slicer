/*=auto=========================================================================

Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLFreeSurferProceduralColorNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.0 $

=========================================================================auto=*/
#include <sstream>
#include <vtksys/SystemTools.hxx>

#include "vtkObjectFactory.h"

#include "vtkMRMLFreeSurferProceduralColorNode.h"
#include "vtkMRMLLogic.h"

#include "vtkFSLookupTable.h"

//------------------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkMRMLFreeSurferProceduralColorNode, LookupTable, vtkFSLookupTable);

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLFreeSurferProceduralColorNode);

//----------------------------------------------------------------------------
vtkMRMLFreeSurferProceduralColorNode::vtkMRMLFreeSurferProceduralColorNode()
{
  this->LookupTable = nullptr;
  this->HideFromEditors = 1;
  this->LabelsFileName = nullptr;

  // get the colour file in the freesurfer share dir
  std::vector<std::string> filesVector;
  filesVector.emplace_back(""); // for relative path
  filesVector.push_back(vtkMRMLLogic::GetApplicationHomeDirectory());
  filesVector.emplace_back("share/FreeSurfer/FreeSurferColorLUT20120827.txt");
  std::string colorFileName = vtksys::SystemTools::JoinPath(filesVector);
  this->SetLabelsFileName(colorFileName.c_str());

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
}

//----------------------------------------------------------------------------
void vtkMRMLFreeSurferProceduralColorNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  // only print out the look up table if ?
  if (this->LookupTable != nullptr)
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
  while (*atts != nullptr)
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
      bool errorCondition = false;
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
        if (this->SetColorNameWithSpaces(index, name.c_str(), "_") == 0)
          {
              vtkErrorMacro("ReadXMLAttributes: error setting color " << index << " to name " << name.c_str());
          errorCondition = true;
          break;
          }
        }
      if (!errorCondition)
        {
        this->NamesInitialisedOn();
        }
      }
    }
  vtkDebugMacro("Finished reading in xml attributes, list id = " << this->GetID() << " and name = " << this->GetName() << endl);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLFreeSurferProceduralColorNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLFreeSurferProceduralColorNode *node = (vtkMRMLFreeSurferProceduralColorNode *) anode;

  if (node != nullptr)
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

  if (this->LookupTable != nullptr)
    {
    os << indent << "Look up table:\n";
    this->LookupTable->PrintSelf(os, indent.GetNextIndent());
    }
  if (this->LabelsFileName)
    {
    os << indent << "Volume label map color file: " << this->GetLabelsFileName() << endl;
    }
}

//-----------------------------------------------------------

void vtkMRMLFreeSurferProceduralColorNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
  /*
    if (this->GetStorageNodeID() == nullptr)
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
  if (this->Type == this->Custom)
    {
    return "Custom";
    }
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
  if (dnode != nullptr && dnode == vtkMRMLFreeSurferProceduralColorDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
        this->InvokeEvent(vtkMRMLFreeSurferProceduralColorNode::DisplayModifiedEvent, nullptr);
    }
*/
  return;
}

//---------------------------------------------------------------------------
void vtkMRMLFreeSurferProceduralColorNode::SetType(int type)
{
  if (this->GetLookupTable() != nullptr &&
      this->Type == type)
    {
    vtkDebugMacro("SetType: type is already set to " << type <<  " = " << this->GetTypeAsString());
    return;
    }

    this->Type = type;

    vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Type to " << type << " = " << this->GetTypeAsString());

    //this->LookupTable->Delete();
    if (this->GetLookupTable() == nullptr)
      {
      vtkDebugMacro("vtkMRMLFreeSurferProceduralColorNode::SetType Creating a new lookup table (was null) of type " << this->GetTypeAsString() << "\n");
      vtkFSLookupTable * table = vtkFSLookupTable::New();
      if (table != nullptr)
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
      this->SetDescription("The Heat FreeSurfer colour table, shows hot spots with high activation");
      }
    else if (this->Type == this->BlueRed)
      {
      this->GetFSLookupTable()->SetLutTypeToBlueRed();
      this->SetNamesFromColors();
      this->SetDescription("A FreeSurfer color scale, 256 colours, from blue to red");
      }

    else if (this->Type == this->RedBlue)
      {
      this->GetFSLookupTable()->SetLutTypeToRedBlue();
      this->SetNamesFromColors();
      this->SetDescription("A FreeSurfer color scale, 256 colours, from red to blue");
      }

    else if (this->Type == this->RedGreen)
      {
      this->GetFSLookupTable()->SetLutTypeToRedGreen();
      this->SetNamesFromColors();
      this->SetDescription("A FreeSurfer color scale, 256 colours, from red to green, used to highlight sulcal curvature");
      }
    else if (this->Type == this->GreenRed)
      {
      this->GetFSLookupTable()->SetLutTypeToGreenRed();
      this->SetNamesFromColors();
      this->SetDescription("A FreeSurfer color scale, 256 colours, from green to red, used to highlight sulcal curvature");
      }
    else if (this->Type == this->Labels ||
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
bool vtkMRMLFreeSurferProceduralColorNode::SetNameFromColor(int index)
{
  return vtkMRMLColorNode::SetNameFromColor(index);
}

//---------------------------------------------------------------------------
vtkFSLookupTable *vtkMRMLFreeSurferProceduralColorNode::GetFSLookupTable()
{
  return this->LookupTable;
}

//---------------------------------------------------------------------------
vtkLookupTable * vtkMRMLFreeSurferProceduralColorNode::GetLookupTable()
{
  if (this->LookupTable == nullptr)
    {
    return nullptr;
    }

  // otherwise have to cast it
  // return (vtkLookupTable *)(this->LookupTable);
  if (vtkLookupTable::SafeDownCast(this->LookupTable) == nullptr)
    {
    vtkErrorMacro("GetLookupTable: error converting fs lookup table to vtk look up table.\n");
    }
  return vtkLookupTable::SafeDownCast(this->LookupTable);
}

//---------------------------------------------------------------------------
vtkScalarsToColors* vtkMRMLFreeSurferProceduralColorNode::GetScalarsToColors()
{
  return this->GetLookupTable();
}

//---------------------------------------------------------------------------
int vtkMRMLFreeSurferProceduralColorNode::GetNumberOfColors()
{
  double *range = this->GetLookupTable()->GetRange();
  if (!range)
    {
    return 0;
    }
  int numPoints = static_cast<int>(floor(range[1] - range[0]));
  if (range[0] < 0 && range[1] >= 0)
    {
    // add one for zero
    numPoints++;
    }
  return numPoints;
}

//---------------------------------------------------------------------------
bool vtkMRMLFreeSurferProceduralColorNode::GetColor(int entry, double color[4])
{
  if (entry < 0 || entry >= this->GetNumberOfColors())
    {
    vtkErrorMacro( "vtkMRMLColorTableNode::SetColor: requested entry " << entry << " is out of table range: 0 - " << this->GetLookupTable()->GetNumberOfTableValues() << ", call SetNumberOfColors" << endl);
    return false;
    }
  double *range = this->GetFSLookupTable()->GetRange();
  if (!range)
    {
    return false;
    }
  this->GetFSLookupTable()->GetColor(range[0] + entry, color);
  color[3] = 1.;
  return true;
}
