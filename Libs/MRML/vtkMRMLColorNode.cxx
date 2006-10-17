/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLColorNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.0 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLColorNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLColorNode* vtkMRMLColorNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLColorNode");
  if(ret)
    {
    return (vtkMRMLColorNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLColorNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLColorNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLColorNode");
  if(ret)
    {
    return (vtkMRMLColorNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLColorNode;
}


//----------------------------------------------------------------------------
vtkMRMLColorNode::vtkMRMLColorNode()
{

  this->Name = NULL;
  this->SetName("");
  this->LookupTable = vtkLookupTable::New();
  this->FileName = NULL;
  this->SetTypeToGrey();
  
}

//----------------------------------------------------------------------------
vtkMRMLColorNode::~vtkMRMLColorNode()
{
    if (this->LookupTable)
    {
        this->LookupTable->Delete();
    }
  if (this->FileName) {

      delete [] this->FileName;
      this->FileName = NULL;
  }
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);
  
  of << " type=\"" << this->GetType() << "\"";

  if (this->Type == this->File && this->FileName != NULL)
    {
    of << " filename=\"" << this->FileName << "\"";
    }
  // only print out the look up table if ?
  if (this->LookupTable != NULL) // && this->Type != this->File
    {
    of << " numcolors=\"" << this->LookupTable->GetNumberOfTableValues() << "\"";
    of << " colors=\"";
    for (int i = 0; i < this->LookupTable->GetNumberOfTableValues(); i++)
      {
      double *rgba;
      rgba = this->LookupTable->GetTableValue(i);
      of <<  i << " '" << this->GetColorNameWithoutSpaces(i, "_") << "' " << rgba[0] << " " << rgba[1] << " " << rgba[2] << " " << rgba[3] << " ";
      }
    of << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  int numColours;
  while (*atts != NULL) 
  {
      attName = *(atts++);
      attValue = *(atts++);
      if (!strcmp(attName, "name"))
      {
          this->SetName(attValue);
      }
      else if (!strcmp(attName, "id"))
      {
          // handled at the vtkMRMLNode level
      }
      else if (!strcmp(attName, "numcolors"))
        {
        std::stringstream ss;
        ss << attValue;
        ss >> numColours;
        vtkDebugMacro("Setting the look up table size to " << numColours << "\n");
        this->LookupTable->SetNumberOfTableValues(numColours);
        this->Names.clear();
        this->Names.resize(numColours);
        }
      else  if (!strcmp(attName, "colors")) 
      {
      std::stringstream ss;
      for (int i = 0; i < this->LookupTable->GetNumberOfTableValues(); i++)
        {
        vtkDebugMacro("Reading colour " << i << " of " << this->LookupTable->GetNumberOfTableValues() << endl);
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
        vtkDebugMacro("Adding colour at index " << index << ", r = " << r << ", g = " << g << ", b = " << b << ", a = " << a << " and then setting name to " << name.c_str() << endl);
        this->LookupTable->SetTableValue(index, r, g, b, a);
        this->SetColorNameWithSpaces(index, name.c_str(), "_");
        }
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
        // read in the file with the colours
        std::cout << "Reading file " << this->FileName << endl;
        this->ReadFile();
        }
      else
      {
          std::cerr << "Unknown attribute name " << attName << endl;
      }
  }
  vtkDebugMacro("Finished reading in xml attributes, list id = " << this->GetID() << " and name = " << this->GetName() << endl);
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::ReadFile ()
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
    this->LookupTable->SetNumberOfColors(maxID + 1); // for zero
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
      this->LookupTable->SetTableValue(id, r, g, b, a);
      this->SetColorName(id, name.c_str());
      }
    }
  else
    {
    std::cerr << "ERROR opening colour file " << this->FileName << endl;
    }
}
//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLColorNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLColorNode *node = (vtkMRMLColorNode *) anode;

  this->SetName(node->Name);
  this->SetLookupTable(node->LookupTable);
  this->SetType(node->Type);
  this->SetFileName(node->FileName);
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

  os << indent << "Name: " <<
      (this->Name ? this->Name : "(none)") << "\n";
  

  os << indent << "Type: (" << this->GetTypeAsString() << ")\n";

  if (this->LookupTable != NULL)
    {
    os << indent << "Look up table:\n";
    this->LookupTable->PrintSelf(os, indent.GetNextIndent());
    }
  
  if (this->Names.size() > 0)
    {
    os << indent << "Color Names:\n";
    for (int i = 0; (int)i < this->Names.size(); i++)
      {
      os << indent << indent << i << " " << this->GetColorName(i) << endl;
      }
    }
}

//-----------------------------------------------------------

void vtkMRMLColorNode::UpdateScene(vtkMRMLScene *scene)
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
void vtkMRMLColorNode::SetTypeToGrey()
{
    this->SetType(this->Grey);
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::SetTypeToIron()
{
    this->SetType(this->Iron);
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::SetTypeToRainbow()
{
    this->SetType(this->Rainbow);
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::SetTypeToOcean()
{
    this->SetType(this->Ocean);
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::SetTypeToDesert()
{
    this->SetType(this->Desert);
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::SetTypeToInvGrey()
{
    this->SetType(this->InvGrey);
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::SetTypeToReverseRainbow()
{
    this->SetType(this->ReverseRainbow);
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::SetTypeToFMRI()
{
    this->SetType(this->FMRI);
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::SetTypeToFMRIPA()
{
    this->SetType(this->FMRIPA);
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::SetTypeToLabels()
{
    this->SetType(this->Labels);
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::SetTypeToRandom()
{
  
  this->SetType(this->Random);
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::SetTypeToFile()
{
    this->SetType(this->File);
}

//----------------------------------------------------------------------------
const char* vtkMRMLColorNode::GetTypeAsString()
{
  if (this->Type == this->Grey)
    {
    return "Grey";
    }
  if (this->Type == this->Iron)
    {
    return "Iron";
    }
  if (this->Type == this->Rainbow)
    {
    return "Rainbow";
    }
  if (this->Type == this->Ocean)
    {
    return "Ocean";
    }
  if (this->Type == this->Desert)
    {
    return "Desert";
    }
  if (this->Type == this->InvGrey)
    {
    return "InvertedGrey";
    }
  if (this->Type == this->ReverseRainbow)
    {
    return "ReverseRainbow";
    }
  if (this->Type == this->FMRI)
    {
    return "fMRI";
    }
  if (this->Type == this->FMRIPA)
    {
    return "fMRIPA";
    }
  if (this->Type == this->Labels)
    {
    return "Labels";
    }
  if (this->Type == this->Random)
    {
    return "Random";
    }
  if (this->Type == this->File)
    {
    return "File";
    }
  return "(unknown)";
}

//---------------------------------------------------------------------------
void vtkMRMLColorNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
/*
  vtkMRMLColorDisplayNode *dnode = this->GetDisplayNode();
  if (dnode != NULL && dnode == vtkMRMLColorDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
        this->InvokeEvent(vtkMRMLColorNode::DisplayModifiedEvent, NULL);
    }
*/
  return;
}

//---------------------------------------------------------------------------
void vtkMRMLColorNode::SetType(int type)
{
    if (this->Type == type)
      {
      vtkDebugMacro("SetType: type is already set to " << type <<  " = " << this->GetTypeAsString());
      return;
      }
    
    this->Type = type;
    //this->SetName(this->GetTypeAsString());

    vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Type to " << type << " = " << this->GetTypeAsString());

    this->LookupTable->Delete();
    this->LookupTable = vtkLookupTable::New();
    
    if (this->Type == this->Grey)
      {
      // from vtkSlicerSliceLayerLogic.cxx
      this->LookupTable->SetRampToLinear();
      this->LookupTable->SetTableRange(0, 255);
      this->LookupTable->SetHueRange(0, 0);
      this->LookupTable->SetSaturationRange(0, 0);
      this->LookupTable->SetValueRange(0, 1);
      this->LookupTable->SetAlphaRange(1, 1); // not used
      this->LookupTable->Build();
      this->SetNamesFromColors();
      }
    else if (this->Type == this->Iron)
      {
      this->LookupTable->SetNumberOfTableValues(156);
      this->LookupTable->SetHueRange(0, 0.15);
      this->LookupTable->SetSaturationRange(1,1);
      this->LookupTable->SetValueRange(1,1);
      this->LookupTable->SetRampToLinear();
      this->LookupTable->Build();
      this->SetNamesFromColors();
      }

    else if (this->Type == this->Rainbow)
      {
      this->LookupTable->SetNumberOfTableValues(256);
      this->LookupTable->SetHueRange(0, 0.8);
      this->LookupTable->SetSaturationRange(1,1);
      this->LookupTable->SetValueRange(1,1);
      this->LookupTable->SetRampToLinear();
      this->LookupTable->Build();
      this->SetNamesFromColors();
      }

    else if (this->Type == this->Ocean)
      {
      this->LookupTable->SetNumberOfTableValues(256);
      this->LookupTable->SetHueRange(0.666667, 0.5);
      this->LookupTable->SetSaturationRange(1,1);
      this->LookupTable->SetValueRange(1,1);
      this->LookupTable->SetRampToLinear();
      this->LookupTable->Build();
      this->SetNamesFromColors();
      }
    else if (this->Type == this->Desert)
      {
      this->LookupTable->SetNumberOfTableValues(256);
      this->LookupTable->SetHueRange(0, 0.1);
      this->LookupTable->SetSaturationRange(1,1);
      this->LookupTable->SetValueRange(1,1);
      this->LookupTable->SetRampToLinear();
      this->LookupTable->Build();
      this->SetNamesFromColors();
      }
    
    else if (this->Type == this->InvGrey)
      {
      this->LookupTable->SetNumberOfTableValues(256);
      this->LookupTable->SetHueRange(0,0);
      this->LookupTable->SetSaturationRange(0,0);
      this->LookupTable->SetValueRange(1,0);
      this->LookupTable->SetRampToLinear();
      this->LookupTable->Build();
      this->SetNamesFromColors();
      }

    else if (this->Type == this->ReverseRainbow)
      {
      this->LookupTable->SetNumberOfTableValues(256);
      this->LookupTable->SetHueRange(0.8, 1);
      this->LookupTable->SetSaturationRange(1,1);
      this->LookupTable->SetValueRange(1,1);
      this->LookupTable->SetRampToLinear();
      this->LookupTable->Build();
      this->SetNamesFromColors();
      }
    
    else if (this->Type == this->FMRI)
      {
      // Use different numbers of table values for neg and pos
      // to make sure -1 is represented by blue

      // From green to blue
      vtkLookupTable *neg = vtkLookupTable::New();
      neg->SetNumberOfTableValues(23);
      neg->SetHueRange(0.5, 0.66667);
      neg->SetSaturationRange( 1, 1);
      neg->SetValueRange(1, 1);
      neg->SetRampToLinear();
      neg->Build();

      // From red to yellow
      vtkLookupTable *pos = vtkLookupTable::New();
      pos->SetNumberOfTableValues(20);
      pos->SetHueRange(0,0.16667);
      pos->SetSaturationRange(1,1);
      pos->SetValueRange(1,1);
      pos->SetRampToLinear();
      pos->Build();

      this->LookupTable->SetNumberOfTableValues(43);
      this->LookupTable->SetRampToLinear();
      this->LookupTable->Build();

      for (int i = 0; i < 23; i++)
        {
        this->LookupTable->SetTableValue(i, neg->GetTableValue(i));
        }
      for (int i = 0; i < 20; i++)
        {
        this->LookupTable->SetTableValue(i+23, pos->GetTableValue(i));
        }
      
      pos->Delete();
      neg->Delete();
      }
    
    else if (this->Type == this->FMRIPA)
      {
      int size = 20;
      this->LookupTable->SetNumberOfTableValues(size);
      this->LookupTable->SetHueRange(0, 0.16667);
      this->LookupTable->SetSaturationRange(1, 1);
      this->LookupTable->SetValueRange(1, 1);
      this->LookupTable->SetRampToLinear();
      this->LookupTable->Build();
      this->SetNamesFromColors();
      }

    else if (this->Type == this->Labels)
      {
      // from Slicer2's Colors.xml
      this->LookupTable->SetNumberOfTableValues(21);
      this->Names.clear();
      this->Names.resize(this->LookupTable->GetNumberOfTableValues());
      
      this->SetColorName(0, "Black");
      this->LookupTable->SetTableValue(0, 0, 0, 0, 1.0);

      this->SetColorName(1, "jake");
      this->LookupTable->SetTableValue(1, 0.2, 0.5, 0.8, 1.0);

      this->SetColorName(2, "Skin");
      this->LookupTable->SetTableValue(2, 1.0, 0.8, 0.7, 1.0);

      this->SetColorName(3, "Brain");
      this->LookupTable->SetTableValue(3, 1.0, 1.0, 1.0, 1.0);

      this->SetColorName(4, "Ventricles");
      this->LookupTable->SetTableValue(4, 0.4, 0.7, 1.0, 1.0);

      this->SetColorName(5, "Vessels");
      this->LookupTable->SetTableValue(5, 0.9, 0.5, 0.5, 1.0);

      this->SetColorName(6, "Tumor");
      this->LookupTable->SetTableValue(6, 0.5, 0.9, 0.5, 1.0);

      this->SetColorName(7, "fMRI-high");
      this->LookupTable->SetTableValue(7, 0.5, 0.9, 0.9, 1.0);

      this->SetColorName(8, "fMRI-low");
      this->LookupTable->SetTableValue(8, 0.9, 0.9, 0.5, 1.0);

      this->SetColorName(9, "Pre-Gyrus");
      this->LookupTable->SetTableValue(9, 0.9, 0.7, 0.9, 1.0);

      this->SetColorName(10, "Post-Gyrus");
      this->LookupTable->SetTableValue(10, 0.9, 0.9, 0.5, 1.0);

      this->SetColorName(11, "jake");
      this->LookupTable->SetTableValue(11, 0.2, 0.5, 0.8, 1.0);

      this->SetColorName(12, "elwood");
      this->LookupTable->SetTableValue(12, 0.2, 0.8, 0.5, 1.0);

      this->SetColorName(13, "gato");
      this->LookupTable->SetTableValue(13, 0.8, 0.5, 0.2, 1.0);

      this->SetColorName(14, "avery");
      this->LookupTable->SetTableValue(14, 0.8, 0.2, 0.5, 1.0);

      this->SetColorName(15, "mambazo");
      this->LookupTable->SetTableValue(15, 0.5, 0.2, 0.8, 1.0);

      this->SetColorName(16, "domino");
      this->LookupTable->SetTableValue(16, 0.5, 0.8, 0.2, 1.0);

      this->SetColorName(17, "monk");
      this->LookupTable->SetTableValue(17, 0.2, 0.2, 0.8, 1.0);

      this->SetColorName(18, "forest");
      this->LookupTable->SetTableValue(18, 0.8, 0.8, 0.2, 1.0);

      this->SetColorName(19, "dylan");
      this->LookupTable->SetTableValue(19, 0.2, 0.8, 0.8, 1.0);

      this->SetColorName(20, "kales");
      this->LookupTable->SetTableValue(20, 0.5, 0.5, 0.5, 1.0);
      /*
      this->SetColorName(300, "fMRI-neg");
      this->LookupTable->SetTableValue(300, 0.0, 0.8, 1.0, 1.0);

      this->SetColorName(301, "fMRI-pos");
      this->LookupTable->SetTableValue(301, 1.0, 1.0, 0.0, 1.0);
      */
      
      }
    
    else if (this->Type == this->Random)
      {
      int size = 255;
      
      this->LookupTable->SetTableValue(0, 0, 0, 0, 0);
      this->LookupTable->SetRange(0, size);
      this->LookupTable->SetNumberOfColors(size + 1);
      for (int i = 1; i <= size; i++)
        {
        // table values have to be 0-1
        double r = (rand()%255)/255.0;
        double g = (rand()%255)/255.0;
        double b = (rand()%255)/255.0;
       
        this->LookupTable->SetTableValue(i, r, g, b, 1.0);
        }
      this->SetNamesFromColors();      
      }
    
    else if (this->Type == this->File)
      {
      std::cout << "Set type to file, call SetFileName and ReadFile next...\n";
      }

    else
      {
      std::cerr << "vtkMRMLColorNode: SetType ERROR, unknown type " << type << endl;
      return;
      }
    // invoke a modified event
    this->Modified();
    
    // invoke a type  modified event
    this->InvokeEvent(vtkMRMLColorNode::TypeModifiedEvent);
}

//---------------------------------------------------------------------------
void vtkMRMLColorNode::SetNamesFromColors()
{
  int size = this->LookupTable->GetNumberOfColors();
  double *rgba;
  // reset the names
  this->Names.clear();
  this->Names.resize(size);
  for (int i = 0; i < size; i++)
    {
    rgba = this->LookupTable->GetTableValue(i);
    std::stringstream ss;
    ss << "R=";
    ss << rgba[0];
    ss << " G=";
    ss << rgba[1];
    ss << " B=";
    ss << rgba[2];
    ss << " A=";
    ss << rgba[3];
    vtkDebugMacro("SetNamesFromColors: " << i << " Name = " << ss.str().c_str());
    this->SetColorName(i, ss.str().c_str());
    }
}

//---------------------------------------------------------------------------
const char *vtkMRMLColorNode::GetColorName(int ind)
{
    if (ind < this->Names.size() && ind >= 0)
    {
    if (strcmp(this->Names[ind].c_str(), "") == 0)
      {
      return "(none)";
      }
    else
      {
      return this->Names[ind].c_str();
      }
    }
  else
    {
    return "invalid";
    }
}

//---------------------------------------------------------------------------
const char *vtkMRMLColorNode::GetColorNameWithoutSpaces(int ind, const char *subst)
{
  std::string name = std::string(this->GetColorName(ind));
  if (strstr(name.c_str(), " ") != NULL)
    {
    std::string::size_type spaceIndex = name.find( " ", 0 );
    while (spaceIndex != std::string::npos)
      {
      name.replace(spaceIndex, 1, subst, 0, strlen(subst));
      spaceIndex = name.find( " ", spaceIndex );
      }
    return name.c_str();
    }
  else
    {
    // no spaces, return it as is
    return name.c_str();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLColorNode::SetColorName(int ind, const char *name)
{
    if (ind < this->Names.size() && ind >= 0)
    {
    this->Names[ind] = std::string(name);
    }
  else
    {
    std::cerr << "ERROR: SetColorName, index was out of bounds: " << ind << ", current size is " << this->Names.size() << endl;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLColorNode::SetColorNameWithSpaces(int ind, const char *name, const char *subst)
{
 
  std::string nameString = std::string(name);
  std::string substString = std::string(subst);
   // does the input name have the subst character in it?
  if (strstr(name, substString.c_str()) != NULL)
    {
    this->SetColorName(ind, nameString.c_str());
    }
  else
    {
    // no substitutions necessary
    this->SetColorName(ind, name);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLColorNode::AddColorName(const char *name)
{
  this->Names.push_back(std::string(name));
}
