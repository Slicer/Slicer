/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLColorNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.0 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLColorNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"

// VTK includes
#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// STD includes
#include <cassert>
#include <sstream>
#include <algorithm>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLColorNode);

//----------------------------------------------------------------------------
vtkMRMLColorNode::vtkMRMLColorNode()
{
  this->SetName("");
  this->FileName = nullptr;
  this->Type = -1;
  this->HideFromEditors = 1;

  this->NoName = nullptr;
  this->SetNoName("(none)");

  this->NamesInitialised = 0;
}

//----------------------------------------------------------------------------
vtkMRMLColorNode::~vtkMRMLColorNode()
{
  if (this->FileName)
    {
    delete [] this->FileName;
    this->FileName = nullptr;
    }

  if (this->NoName)
    {
    delete [] this->NoName;
    this->NoName = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  of << " type=\"" << this->GetType() << "\"";

  if (this->FileName != nullptr)
    {
    // don't write it out, it's handled by the storage node
    }
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "name"))
      {
      this->SetName(attValue);
      }
    else if (!strcmp(attName, "type"))
      {
      int type;
      std::stringstream ss;
      ss << attValue;
      ss >> type;
      this->SetType(type);
      }
    else if (!strcmp(attName, "filename"))
      {
      this->SetFileName(attValue);
      // don't read in the file with the colors, it's handled by the storage
      // node
      if (this->GetStorageNode() == nullptr)
        {
        vtkWarningMacro("A color node has a file name, but no storage node, trying to create one");
        this->AddDefaultStorageNode(attValue);
        }
      }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
vtkLookupTable * vtkMRMLColorNode::GetLookupTable()
{
  vtkDebugMacro("Subclass has not implemented GetLookupTable, returning NULL");
  return nullptr;
}

//----------------------------------------------------------------------------
vtkScalarsToColors * vtkMRMLColorNode::GetScalarsToColors()
{
  return this->GetLookupTable();
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLColorNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLColorNode *node = (vtkMRMLColorNode *) anode;

  if (node->Type != -1)
    {
    // not using SetType, as that will basically recreate a new color node,
    // very slow
    this->Type = node->Type;
    }
  this->SetFileName(node->FileName);
  this->SetNoName(node->NoName);

  // copy names
  this->Names = node->Names;

  this->NamesInitialised = node->NamesInitialised;

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os,indent);

  os << indent << "Name: " <<
      (this->Name ? this->Name : "(none)") << "\n";


  os << indent << "Type: (" << this->GetTypeAsString() << ")\n";

  os << indent << "NoName = " <<
    (this->NoName ? this->NoName : "(not set)") <<  "\n";

  os << indent << "Names array initialised: " << (this->GetNamesInitialised() ? "true" : "false") << "\n";

  if (this->Names.size() > 0)
    {
    os << indent << "Color Names:\n";
    for (unsigned int i = 0; i < this->Names.size(); i++)
      {
      double color[4];
      this->GetColor(i, color);
      os << indent << indent << i << " " << this->GetColorName(i)
         << " (" << color[0] << ", " << color[1] << ", " << color[2]
         << ", " << color[3] << ")"
         << std::endl;
      if ( i >= 10 )
        {
        os << indent << indent << "..." << endl;
        break;
        }
      }
    }
}

//-----------------------------------------------------------

void vtkMRMLColorNode::UpdateScene(vtkMRMLScene *scene)
{
    Superclass::UpdateScene(scene);
}


//---------------------------------------------------------------------------
void vtkMRMLColorNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
/*
  vtkMRMLColorDisplayNode *dnode = this->GetDisplayNode();
  if (dnode != nullptr && dnode == vtkMRMLColorDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
        this->InvokeEvent(vtkMRMLColorNode::DisplayModifiedEvent, nullptr);
    }
*/
  return;
}

//---------------------------------------------------------------------------
const char * vtkMRMLColorNode::GetTypeAsString()
{
  if (this->Type == this->User)
    {
    return "UserDefined";
    }
  if (this->Type == this->File)
    {
    return "File";
    }
  return "(unknown)";
}

//---------------------------------------------------------------------------
void vtkMRMLColorNode::SetTypeToUser()
{
  this->SetType(this->User);
}

//---------------------------------------------------------------------------
void vtkMRMLColorNode::SetTypeToFile()
{
  this->SetType(this->File);
}

//---------------------------------------------------------------------------
void vtkMRMLColorNode::SetType(int type)
{
  if (this->Type == type)
    {
    vtkDebugMacro("SetType: type is already set to " << type);
    return;
    }

  this->Type = type;

  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Type to " << type);

  // subclass should over ride this and define colors according to the node
  // type

  // invoke a modified event
  this->Modified();

  // invoke a type  modified event
  this->InvokeEvent(vtkMRMLColorNode::TypeModifiedEvent);
}

//---------------------------------------------------------------------------
void vtkMRMLColorNode::SetNamesFromColors()
{
  const int numPoints = this->GetNumberOfColors();
  // reset the names
  this->Names.resize(numPoints);

  for (int i = 0; i < numPoints; ++i)
    {
#ifndef NDEBUG
    bool res =
#endif
      this->SetNameFromColor(i);
    // There is no reason why SetNameFromColor would fail because we control
    // the array size.
    assert(res);
    }
  this->NamesInitialisedOn();
}

//---------------------------------------------------------------------------
bool vtkMRMLColorNode::SetNameFromColor(int index)
{
  double rgba[4] = {-1.,-1.,-1.,-1.};
  bool res = this->GetColor(index, rgba);
  std::stringstream ss;
  ss.precision(3);
  ss.setf(std::ios::fixed, std::ios::floatfield);
  ss << "R=";
  ss << rgba[0];
  ss << " G=";
  ss << rgba[1];
  ss << " B=";
  ss << rgba[2];
  ss << " A=";
  ss << rgba[3];
  vtkDebugMacro("SetNamesFromColors: " << index << " Name = " << ss.str().c_str());
  if (this->SetColorName(index, ss.str().c_str()) == 0)
    {
    vtkErrorMacro("SetNamesFromColors: Error setting color name " << index << " Name = " << ss.str().c_str());
    return false;
    }
  return res;
}

//---------------------------------------------------------------------------
bool vtkMRMLColorNode::HasNameFromColor(int index)
{
  const char* colorName = this->GetColorName(index);
  if (colorName && strcmp(colorName, this->GetNoName()) == 0)
    {
    return false;
    }
  std::stringstream ss;
  ss << colorName;
  std::string token;
  ss >> token;
  if (token.compare(0,2,"R=") != 0)
    {
    return false;
    }
  ss >> token;
  if (token.compare(0,2,"G=") != 0)
    {
    return false;
    }
  ss >> token;
  if (token.compare(0,2,"B=") != 0)
    {
    return false;
    }
  ss >> token;
  if (token.compare(0,2,"A=") != 0)
    {
    return false;
    }
  return true;
}

//---------------------------------------------------------------------------
const char *vtkMRMLColorNode::GetColorName(int ind)
{
  if (!this->GetNamesInitialised())
    {
    this->SetNamesFromColors();
    }
  if (ind < 0 || ind >= (int)this->Names.size())
    {
    vtkDebugMacro("vtkMRMLColorNode::GetColorName: index " << ind << " is out of range 0 - " << this->Names.size());
    return "invalid";
    }
  if (this->Names[ind].empty())
    {
    return this->NoName;
    }
  return this->Names[ind].c_str();
}

//---------------------------------------------------------------------------
int vtkMRMLColorNode::GetColorIndexByName(const char *name)
{
  if (name == nullptr)
    {
    vtkErrorMacro("vtkMRMLColorNode::GetColorIndexByName: need a non-null name as argument");
    return -1;
    }

  if (!this->GetNamesInitialised())
    {
    this->SetNamesFromColors();
    }

  std::string strName = name;
  for (int i = 0; i < this->GetNumberOfColors(); ++i)
    {
    if (strName == this->GetColorName(i))
      {
      return i;
      }
    }
  return -1;
}

//---------------------------------------------------------------------------
std::string vtkMRMLColorNode::GetColorNameWithoutSpaces(int ind, const char *subst)
{
  std::string name = std::string(this->GetColorName(ind));
  if (strstr(name.c_str(), " ") != nullptr)
    {
    std::string::size_type spaceIndex = name.find( " ", 0 );
    while (spaceIndex != std::string::npos)
      {
      name.replace(spaceIndex, 1, subst, 0, strlen(subst));
      spaceIndex = name.find( " ", spaceIndex );
      }
    }

  return name;
}

//---------------------------------------------------------------------------
std::string vtkMRMLColorNode::GetColorNameAsFileName(int colorIndex, const char *subst)
{
  std::string fileName(this->GetColorName(colorIndex));
  std::string validCharacters =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabscdefghijklmnopqrstuvwxyz"
    "0123456789"
    "-_.()$!~#'%^{}";
  std::string::size_type pos = 0;
  size_t substLength = strlen(subst);
  while ((pos = fileName.find_first_not_of(validCharacters, pos)) != std::string::npos)
    {
    fileName.replace(pos, 1, subst, substLength);
    pos += substLength;
    if (pos > 255)
      {
      break;
      }
    }
  // Truncate to 256 chars
  return fileName.substr(0, 256);
}

//---------------------------------------------------------------------------
int vtkMRMLColorNode::SetColorName(int ind, const char *name)
{
  if (ind >= static_cast<int>(this->Names.size()) || ind < 0)
    {
    vtkErrorMacro("ERROR: SetColorName, index was out of bounds: "<< ind << ", current size is " << this->Names.size() << ", table name = " << (this->GetName() == nullptr ? "null" : this->GetName()));
    return 0;
    }
  std::string newName(name);
  if (this->Names[ind] != newName)
    {
    this->Names[ind] = newName;
    this->StorableModifiedTime.Modified();
    this->Modified();
    }
  return 1;
}

//---------------------------------------------------------------------------
int vtkMRMLColorNode::SetColorNameWithSpaces(int ind, const char *name, const char *subst)
{

  std::string nameString = std::string(name);
  std::string substString = std::string(subst);
   // does the input name have the subst character in it?
  if (strstr(name, substString.c_str()) != nullptr)
    {
    std::replace(nameString.begin(), nameString.end(), *subst, ' ');
    return this->SetColorName(ind, nameString.c_str());
    }
  else
    {
    // no substitutions necessary
    return this->SetColorName(ind, name);
    }
}

//---------------------------------------------------------------------------
int vtkMRMLColorNode::GetNumberOfColors()
{
  return static_cast<int>(this->Names.size());
}

//---------------------------------------------------------------------------
bool vtkMRMLColorNode::GetColor(int vtkNotUsed(index), double vtkNotUsed(color)[4])
{
  return false;
}

//---------------------------------------------------------------------------
void vtkMRMLColorNode::Reset(vtkMRMLNode* vtkNotUsed(defaultNode))
{
  // don't need to call reset on color nodes, as all but the User color table
  // node are static, and that's taken care of in the vtkMRMLColorTableNode
  //Superclass::Reset(defaultNode);
}

//---------------------------------------------------------------------------
bool vtkMRMLColorNode::GetModifiedSinceRead()
{
  return this->Superclass::GetModifiedSinceRead() ||
    (this->GetScalarsToColors() &&
     this->GetScalarsToColors()->GetMTime() > this->GetStoredTime());
}

//---------------------------------------------------------------------------
vtkLookupTable* vtkMRMLColorNode::CreateLookupTableCopy()
{
  vtkLookupTable* copiedLut = vtkLookupTable::New();
  copiedLut->DeepCopy(this->GetLookupTable());

  // Workaround for VTK bug in vtkLookupTable::DeepCopy
  // (special colors are not copied)
  copiedLut->BuildSpecialColors();

  return copiedLut;
}
