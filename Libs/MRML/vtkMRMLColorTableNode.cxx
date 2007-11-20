/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLColorTableNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.0 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLScene.h"

#include "vtkLookupTable.h"
//#include "vtkFSLookupTable.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLColorTableStorageNode.h"
//------------------------------------------------------------------------------
vtkMRMLColorTableNode* vtkMRMLColorTableNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLColorTableNode");
  if(ret)
    {
    return (vtkMRMLColorTableNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLColorTableNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLColorTableNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLColorTableNode");
  if(ret)
    {
    return (vtkMRMLColorTableNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLColorTableNode;
}


//----------------------------------------------------------------------------
vtkMRMLColorTableNode::vtkMRMLColorTableNode()
{

  this->Name = NULL;
  this->SetName("");
  this->LookupTable = NULL;
  this->FileName = NULL;
  this->LastAddedColor = -1;
  this->StorageNodeID = NULL;
}

//----------------------------------------------------------------------------
vtkMRMLColorTableNode::~vtkMRMLColorTableNode()
{
  if (this->LookupTable)
    {
    this->LookupTable->Delete();
    }
  if (this->FileName)
    {  
    delete [] this->FileName;
    this->FileName = NULL;
    }
  if (this->StorageNodeID)
    {
    delete [] this->StorageNodeID;
    this->StorageNodeID = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);
  
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
void vtkMRMLColorTableNode::ReadXMLAttributes(const char** atts)
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
      this->NamesInitialisedOn();
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
          vtkErrorMacro ("Unknown attribute name " << attName << endl);
      }
  }
  vtkDebugMacro("Finished reading in xml attributes, list id = " << this->GetID() << " and name = " << this->GetName() << endl);
}

//----------------------------------------------------------------------------
int vtkMRMLColorTableNode::ReadFile ()
{

  vtkMRMLStorageNode *storageNode = this->GetStorageNode();
  if (!storageNode)
    {
    vtkErrorMacro("ReadFile: unable to get storage node to read file");
    return 0;
    }
  storageNode->SetFileName(this->FileName);
  return storageNode->ReadData(this);
  

}
//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLColorTableNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLColorTableNode *node = (vtkMRMLColorTableNode *) anode;
  if (node->LookupTable)
    {
    this->SetLookupTable(node->LookupTable);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::PrintSelf(ostream& os, vtkIndent indent)
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
/*  if (this->FSLookupTable != NULL)
    {
    os << indent << "FreeSurfer look up table:\n";
    this->FSLookupTable->PrintSelf(os, indent.GetNextIndent());
    }
*/
  if (this->Names.size() > 0)
    {
    os << indent << "Color Names:\n";
    for (unsigned int i = 0; (int)i < this->Names.size(); i++)
      {
      os << indent << indent << i << " " << this->GetColorName(i) << endl;
      if ( i > 10 )
        {
        os << indent << indent << "..." << endl;
        break;
        }
      }
    }
}

//-----------------------------------------------------------

void vtkMRMLColorTableNode::UpdateScene(vtkMRMLScene *scene)
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
void vtkMRMLColorTableNode::SetTypeToGrey()
{
    this->SetType(this->Grey);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToIron()
{
    this->SetType(this->Iron);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToRainbow()
{
    this->SetType(this->Rainbow);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToOcean()
{
    this->SetType(this->Ocean);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToDesert()
{
    this->SetType(this->Desert);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToInvGrey()
{
    this->SetType(this->InvGrey);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToReverseRainbow()
{
    this->SetType(this->ReverseRainbow);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToFMRI()
{
    this->SetType(this->FMRI);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToFMRIPA()
{
    this->SetType(this->FMRIPA);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToLabels()
{
    this->SetType(this->Labels);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToSPLBrainAtlas()
{
    this->SetType(this->SPLBrainAtlas);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToRandom()
{
  
  this->SetType(this->Random);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToUser()
{
  this->SetType(this->User);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToFile()
{
    this->SetType(this->File);
}

//----------------------------------------------------------------------------
const char* vtkMRMLColorTableNode::GetTypeAsIDString()
{
  if (this->Type == this->Grey)
    {
    return "vtkMRMLColorTableNodeGrey";
    }
  if (this->Type == this->Iron)
    {
    return "vtkMRMLColorTableNodeIron";
    }
  if (this->Type == this->Rainbow)
    {
    return "vtkMRMLColorTableNodeRainbow";
    }
  if (this->Type == this->Ocean)
    {
    return "vtkMRMLColorTableNodeOcean";
    }
  if (this->Type == this->Desert)
    {
    return "vtkMRMLColorTableNodeDesert";
    }
  if (this->Type == this->InvGrey)
    {
    return "vtkMRMLColorTableNodeInvertedGrey";
    }
  if (this->Type == this->ReverseRainbow)
    {
    return "vtkMRMLColorTableNodeReverseRainbow";
    }
  if (this->Type == this->FMRI)
    {
    return "vtkMRMLColorTableNodefMRI";
    }
  if (this->Type == this->FMRIPA)
    {
    return "vtkMRMLColorTableNodefMRIPA";
    }
  if (this->Type == this->Labels)
    {
    return "vtkMRMLColorTableNodeLabels";
    }
  if (this->Type == this->SPLBrainAtlas)
    {
    return "vtkMRMLColorTableNodeSPLBrainAtlas";
    }
  if (this->Type == this->Random)
    {
    return "vtkMRMLColorTableNodeRandom";
    }
  if (this->Type == this->User)
    {
      return "vtkMRMLColorTableNodeUser";
    }
  if (this->Type == this->File)
    {
    return "vtkMRMLColorTableNodeFile";
    }
  return "(unknown)";
}

//----------------------------------------------------------------------------
const char* vtkMRMLColorTableNode::GetTypeAsString()
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
  if (this->Type == this->SPLBrainAtlas)
    {
    return "SPLBrainAtlas";
    }
  if (this->Type == this->Random)
    {
    return "Random";
    }
  if (this->Type == this->User)
    {
      return "User";
    }
  if (this->Type == this->File)
    {
    return "File";
    }
  return "(unknown)";
}

//---------------------------------------------------------------------------
void vtkMRMLColorTableNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
/*
  vtkMRMLColorTableDisplayNode *dnode = this->GetDisplayNode();
  if (dnode != NULL && dnode == vtkMRMLColorTableDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
        this->InvokeEvent(vtkMRMLColorTableNode::DisplayModifiedEvent, NULL);
    }
*/
  return;
}

//---------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetType(int type)
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
      vtkDebugMacro("vtkMRMLColorTableNode::SetType Creating a new lookup table (was null) of type " << this->GetTypeAsString() << "\n");
      vtkLookupTable *table = vtkLookupTable::New();
      this->SetLookupTable(table);
      table->Delete();
      // as a default, set the table range to 255
      this->GetLookupTable()->SetTableRange(0, 255);
      }

    // delay setting names from colours until asked for one
    if (this->Type == this->Grey)
      {
      // from vtkSlicerSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0, 0);
      this->GetLookupTable()->SetSaturationRange(0, 0);
      this->GetLookupTable()->SetValueRange(0, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      }
    else if (this->Type == this->Iron)
      {
      this->GetLookupTable()->SetNumberOfTableValues(156);
      this->GetLookupTable()->SetTableRange(0, 156);
      this->GetLookupTable()->SetHueRange(0, 0.15);
      this->GetLookupTable()->SetSaturationRange(1,1);
      this->GetLookupTable()->SetValueRange(1,1);
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->ForceBuild();
      this->SetNamesFromColors();
      }

    else if (this->Type == this->Rainbow)
      {
      this->GetLookupTable()->SetNumberOfTableValues(256);
      this->GetLookupTable()->SetHueRange(0, 0.8);
      this->GetLookupTable()->SetSaturationRange(1,1);
      this->GetLookupTable()->SetValueRange(1,1);
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->ForceBuild();
      this->SetNamesFromColors();
      }

    else if (this->Type == this->Ocean)
      {
      this->GetLookupTable()->SetNumberOfTableValues(256);
      this->GetLookupTable()->SetHueRange(0.666667, 0.5);
      this->GetLookupTable()->SetSaturationRange(1,1);
      this->GetLookupTable()->SetValueRange(1,1);
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->ForceBuild();
      this->SetNamesFromColors();
      }
    else if (this->Type == this->Desert)
      {
      this->GetLookupTable()->SetNumberOfTableValues(256);
      this->GetLookupTable()->SetHueRange(0, 0.1);
      this->GetLookupTable()->SetSaturationRange(1,1);
      this->GetLookupTable()->SetValueRange(1,1);
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->ForceBuild();
      this->SetNamesFromColors();
      }
    
    else if (this->Type == this->InvGrey)
      {
      this->GetLookupTable()->SetNumberOfTableValues(256);
      this->GetLookupTable()->SetHueRange(0,0);
      this->GetLookupTable()->SetSaturationRange(0,0);
      this->GetLookupTable()->SetValueRange(1,0);
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->ForceBuild();
      this->SetNamesFromColors();
      }

    else if (this->Type == this->ReverseRainbow)
      {
      this->GetLookupTable()->SetNumberOfTableValues(256);
      this->GetLookupTable()->SetHueRange(0.8, 1);
      this->GetLookupTable()->SetSaturationRange(1,1);
      this->GetLookupTable()->SetValueRange(1,1);
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->ForceBuild();
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

      this->GetLookupTable()->SetNumberOfTableValues(43);
      this->GetLookupTable()->SetTableRange(0,43);
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->ForceBuild();

      for (int i = 0; i < 23; i++)
        {
        this->GetLookupTable()->SetTableValue(i, neg->GetTableValue(i));
        }
      for (int i = 0; i < 20; i++)
        {
        this->GetLookupTable()->SetTableValue(i+23, pos->GetTableValue(i));
        }
      
      pos->Delete();
      neg->Delete();
      this->SetNamesFromColors();
      }
    
    else if (this->Type == this->FMRIPA)
      {
      int size = 20;
      this->GetLookupTable()->SetNumberOfTableValues(size);
      this->GetLookupTable()->SetTableRange(0,size);
      this->GetLookupTable()->SetHueRange(0, 0.16667);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(1, 1);
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->ForceBuild();
      this->SetNamesFromColors();
      }

    else if (this->Type == this->Labels)
      {
      // from Slicer2's Colors.xml
      this->GetLookupTable()->SetNumberOfTableValues(257);
      this->GetLookupTable()->SetTableRange(0,257);
      this->Names.clear();
      this->Names.resize(this->GetLookupTable()->GetNumberOfTableValues());
      
      this->SetColorName(0, "Black");
      this->GetLookupTable()->SetTableValue(0, 0, 0, 0, 0.0);

      this->SetColorName(1, "jake");
      this->GetLookupTable()->SetTableValue(1, 0.2, 0.5, 0.8, 1.0);

      this->SetColorName(2, "Peach");
      this->GetLookupTable()->SetTableValue(2, 1.0, 0.8, 0.7, 1.0);

      this->SetColorName(3, "Brain");
      this->GetLookupTable()->SetTableValue(3, 1.0, 1.0, 1.0, 1.0);

      this->SetColorName(4, "Ventricles");
      this->GetLookupTable()->SetTableValue(4, 0.4, 0.7, 1.0, 1.0);

      this->SetColorName(5, "Vessels");
      this->GetLookupTable()->SetTableValue(5, 0.9, 0.5, 0.5, 1.0);

      this->SetColorName(6, "Tumor");
      this->GetLookupTable()->SetTableValue(6, 0.5, 0.9, 0.5, 1.0);

      this->SetColorName(7, "fMRI-high");
      this->GetLookupTable()->SetTableValue(7, 0.5, 0.9, 0.9, 1.0);

      this->SetColorName(8, "fMRI-low");
      this->GetLookupTable()->SetTableValue(8, 0.9, 0.9, 0.5, 1.0);

      this->SetColorName(9, "Pre-Gyrus");
      this->GetLookupTable()->SetTableValue(9, 0.9, 0.7, 0.9, 1.0);

      this->SetColorName(10, "Post-Gyrus");
      this->GetLookupTable()->SetTableValue(10, 0.9, 0.9, 0.5, 1.0);

      for (int offset = 0; offset <= 240; offset += 10)
        {
        this->SetColorName(offset + 11, "jake");
        this->GetLookupTable()->SetTableValue(offset + 11, 0.2, 0.5, 0.8, 1.0);
        
        this->SetColorName(offset + 12, "elwood");
        this->GetLookupTable()->SetTableValue(offset + 12, 0.2, 0.8, 0.5, 1.0);
        
        this->SetColorName(offset + 13, "gato");
        this->GetLookupTable()->SetTableValue(offset + 13, 0.8, 0.5, 0.2, 1.0);
        
        this->SetColorName(offset + 14, "avery");
        this->GetLookupTable()->SetTableValue(offset + 14, 0.8, 0.2, 0.5, 1.0);
        
        this->SetColorName(offset + 15, "mambazo");
        this->GetLookupTable()->SetTableValue(offset + 15, 0.5, 0.2, 0.8, 1.0);
        
        this->SetColorName(offset + 16, "domino");
        this->GetLookupTable()->SetTableValue(offset + 16, 0.5, 0.8, 0.2, 1.0);

        if (offset <= 230)
          {
          // these ones don't go past 256
          this->SetColorName(offset + 17, "monk");
          this->GetLookupTable()->SetTableValue(offset + 17, 0.2, 0.2, 0.8, 1.0);
          
          this->SetColorName(offset + 18, "forest");
          this->GetLookupTable()->SetTableValue(offset + 18, 0.8, 0.8, 0.2, 1.0);
          
          this->SetColorName(offset + 19, "dylan");
          this->GetLookupTable()->SetTableValue(offset + 19, 0.2, 0.8, 0.8, 1.0);
          
          this->SetColorName(offset + 20, "kales");
          this->GetLookupTable()->SetTableValue(offset + 20, 0.5, 0.5, 0.5, 1.0);
          }
        }
      /*
      this->SetColorName(300, "fMRI-neg");
      this->GetLookupTable()->SetTableValue(300, 0.0, 0.8, 1.0, 1.0);

      this->SetColorName(301, "fMRI-pos");
      this->GetLookupTable()->SetTableValue(301, 1.0, 1.0, 0.0, 1.0);
      */
      this->NamesInitialisedOn();
      }
    else if (this->Type == this->SPLBrainAtlas)
      {
      // From the spl_pnl_brain_atlas_2006 atlas2006.xml
      this->GetLookupTable()->SetNumberOfTableValues(714);
      this->GetLookupTable()->SetTableRange(0,714);
      // there are lots of skipped labels, so set the full table to opaque black
      for (int t = 0; t < 714; t++)
        {
        this->GetLookupTable()->SetTableValue(t, 0, 0, 0, 1.0);
        }
      this->Names.clear();
      this->Names.resize(this->GetLookupTable()->GetNumberOfTableValues());
      
      this->SetColorName(0, "Black");
      this->GetLookupTable()->SetTableValue(0, 0, 0, 0, 0.0);
      // add in the slicer2 colours first, they may get over written by the
      // custom colours
      this->SetColorName(0, "Black");
      this->GetLookupTable()->SetTableValue(0, 0, 0, 0, 0.0);

      this->SetColorName(1, "jake");
      this->GetLookupTable()->SetTableValue(1, 0.2, 0.5, 0.8, 1.0);

      this->SetColorName(2, "Peach");
      this->GetLookupTable()->SetTableValue(2, 1.0, 0.8, 0.7, 1.0);

      this->SetColorName(3, "Brain");
      this->GetLookupTable()->SetTableValue(3, 1.0, 1.0, 1.0, 1.0);

      this->SetColorName(4, "Ventricles");
      this->GetLookupTable()->SetTableValue(4, 0.4, 0.7, 1.0, 1.0);

      this->SetColorName(5, "Vessels");
      this->GetLookupTable()->SetTableValue(5, 0.9, 0.5, 0.5, 1.0);

      this->SetColorName(6, "Tumor");
      this->GetLookupTable()->SetTableValue(6, 0.5, 0.9, 0.5, 1.0);

      this->SetColorName(7, "fMRI-high");
      this->GetLookupTable()->SetTableValue(7, 0.5, 0.9, 0.9, 1.0);

      this->SetColorName(8, "fMRI-low");
      this->GetLookupTable()->SetTableValue(8, 0.9, 0.9, 0.5, 1.0);

      this->SetColorName(9, "Pre-Gyrus");
      this->GetLookupTable()->SetTableValue(9, 0.9, 0.7, 0.9, 1.0);

      this->SetColorName(10, "Post-Gyrus");
      this->GetLookupTable()->SetTableValue(10, 0.9, 0.9, 0.5, 1.0);

      for (int offset = 0; offset <= 240; offset += 10)
        {
        this->SetColorName(offset + 11, "jake");
        this->GetLookupTable()->SetTableValue(offset + 11, 0.2, 0.5, 0.8, 1.0);
        
        this->SetColorName(offset + 12, "elwood");
        this->GetLookupTable()->SetTableValue(offset + 12, 0.2, 0.8, 0.5, 1.0);
        
        this->SetColorName(offset + 13, "gato");
        this->GetLookupTable()->SetTableValue(offset + 13, 0.8, 0.5, 0.2, 1.0);
        
        this->SetColorName(offset + 14, "avery");
        this->GetLookupTable()->SetTableValue(offset + 14, 0.8, 0.2, 0.5, 1.0);
        
        this->SetColorName(offset + 15, "mambazo");
        this->GetLookupTable()->SetTableValue(offset + 15, 0.5, 0.2, 0.8, 1.0);
        
        this->SetColorName(offset + 16, "domino");
        this->GetLookupTable()->SetTableValue(offset + 16, 0.5, 0.8, 0.2, 1.0);

        if (offset <= 230)
          {
          // these ones don't go past 256
          this->SetColorName(offset + 17, "monk");
          this->GetLookupTable()->SetTableValue(offset + 17, 0.2, 0.2, 0.8, 1.0);
          
          this->SetColorName(offset + 18, "forest");
          this->GetLookupTable()->SetTableValue(offset + 18, 0.8, 0.8, 0.2, 1.0);
          
          this->SetColorName(offset + 19, "dylan");
          this->GetLookupTable()->SetTableValue(offset + 19, 0.2, 0.8, 0.8, 1.0);
          
          this->SetColorName(offset + 20, "kales");
          this->GetLookupTable()->SetTableValue(offset + 20, 0.5, 0.5, 0.5, 1.0);
          }
        }
      this->SetColorName(628, "C0"); this->GetLookupTable()->SetTableValue(628, 0.6, 0.8, 0.3, 1.0);
      this->SetColorName(629, "C0"); this->GetLookupTable()->SetTableValue(629, 0.6, 0.8, 0.3, 1.0);
      
      this->SetColorName(101, "C2"); this->GetLookupTable()->SetTableValue(101, 0.996094, 0.746094, 0.792969, 1.0);
      this->SetColorName(632, "C3"); this->GetLookupTable()->SetTableValue(632, 0.7, 0.7, 0, 1.0);
      this->SetColorName(633, "C3"); this->GetLookupTable()->SetTableValue(633, 0.7, 0.7, 0, 1.0);
      this->SetColorName(680, "C4"); this->GetLookupTable()->SetTableValue(680, 1, 0.65, 0.64, 1.0);
      this->SetColorName(681, "C4"); this->GetLookupTable()->SetTableValue(681, 1, 0.65, 0.64, 1.0);
      this->SetColorName(688, "C5"); this->GetLookupTable()->SetTableValue(688, 0.9, 0.4, 0.4, 1.0);
      this->SetColorName(689, "C5"); this->GetLookupTable()->SetTableValue(689, 0.9, 0.4, 0.4, 1.0);
      this->SetColorName(140, "C6"); this->GetLookupTable()->SetTableValue(140, 0.597656, 0.796875, 0.195312, 1.0);
      this->SetColorName(672, "C7"); this->GetLookupTable()->SetTableValue(672, 0.9, 0.6, 0.35, 1.0);
      this->SetColorName(673, "C7"); this->GetLookupTable()->SetTableValue(673, 0.9, 0.6, 0.35, 1.0);
      this->SetColorName(712, "C8"); this->GetLookupTable()->SetTableValue(712, 0.39, 0.85, 0.27, 1.0);
      this->SetColorName(713, "C8"); this->GetLookupTable()->SetTableValue(713, 0.39, 0.85, 0.27, 1.0);
      this->SetColorName(102, "C9"); this->GetLookupTable()->SetTableValue(102, 0.25, 0.410156, 0.878906, 1.0);
      this->SetColorName(97, "C10"); this->GetLookupTable()->SetTableValue(97, 0.25, 0.871094, 0.816406, 1.0);
      this->SetColorName(710, "C11"); this->GetLookupTable()->SetTableValue(710, 0.39, 0.85, 0.27, 1.0);
      this->SetColorName(614, "C12"); this->GetLookupTable()->SetTableValue(614, 0.8, 0.8, 0.3, 1.0);
      this->SetColorName(612, "C13"); this->GetLookupTable()->SetTableValue(612, 0.8, 0.8, 0, 1.0);
      this->SetColorName(600, "C14"); this->GetLookupTable()->SetTableValue(600, 0.8, 0.5, 0.15, 1.0);
      this->SetColorName(602, "C15"); this->GetLookupTable()->SetTableValue(602, 0.8, 0.9, 0.15, 1.0);
      this->SetColorName(622, "C16"); this->GetLookupTable()->SetTableValue(622, 0.7, 0.7, 0.3, 1.0);
      this->SetColorName(33, "C17"); this->GetLookupTable()->SetTableValue(33, 0.605469, 0.398438, 0.117188, 1.0);
      this->SetColorName(442, "C18"); this->GetLookupTable()->SetTableValue(442, 0.238281, 0.566406, 0.246094, 1.0);
      this->SetColorName(508, "C19"); this->GetLookupTable()->SetTableValue(508, 0.179688, 0.542969, 0.339844, 1.0);
      this->SetColorName(81, "C20"); this->GetLookupTable()->SetTableValue(81, 0.996094, 0.484375, 0.246094, 1.0);
      this->SetColorName(512, "C21"); this->GetLookupTable()->SetTableValue(512, 0.953125, 0.636719, 0.375, 1.0);
      this->SetColorName(71, "C22"); this->GetLookupTable()->SetTableValue(71, 0.199219, 0.625, 0.785156, 1.0);
      this->SetColorName(141, "C23"); this->GetLookupTable()->SetTableValue(141, 0.882812, 0.804688, 0.335938, 1.0);
      this->SetColorName(510, "C24"); this->GetLookupTable()->SetTableValue(510, 0.738281, 0.710938, 0.414062, 1.0);
      this->SetColorName(300, "C25"); this->GetLookupTable()->SetTableValue(300, 0.304688, 0.8125, 0.796875, 1.0);
      this->SetColorName(86, "C26"); this->GetLookupTable()->SetTableValue(86, 0, 0.742188, 0.996094, 1.0);
      this->SetColorName(562, "C27"); this->GetLookupTable()->SetTableValue(562, 0.996094, 0.269531, 0, 1.0);
      this->SetColorName(35, "C28"); this->GetLookupTable()->SetTableValue(35, 0.492188, 0.996094, 0, 1.0);
      this->SetColorName(500, "C29"); this->GetLookupTable()->SetTableValue(500, 0.496094, 0.160156, 0.160156, 1.0);
      this->SetColorName(520, "C30"); this->GetLookupTable()->SetTableValue(520, 0.238281, 0.347656, 0.664062, 1.0);
      this->SetColorName(711, "C11"); this->GetLookupTable()->SetTableValue(711, 0.39, 0.85, 0.27, 1.0);
      this->SetColorName(615, "C12"); this->GetLookupTable()->SetTableValue(615, 0.8, 0.8, 0.3, 1.0);
      this->SetColorName(613, "C13"); this->GetLookupTable()->SetTableValue(613, 0.8, 0.8, 0, 1.0);
      this->SetColorName(601, "C14"); this->GetLookupTable()->SetTableValue(601, 0.8, 0.5, 0.15, 1.0);
      this->SetColorName(603, "C15"); this->GetLookupTable()->SetTableValue(603, 0.8, 0.9, 0.15, 1.0);
      this->SetColorName(623, "C16"); this->GetLookupTable()->SetTableValue(623, 0.7, 0.7, 0.3, 1.0);
      this->SetColorName(34, "C17"); this->GetLookupTable()->SetTableValue(34, 0.605469, 0.398438, 0.117188, 1.0);
      this->SetColorName(443, "C18"); this->GetLookupTable()->SetTableValue(443, 0.238281, 0.566406, 0.246094, 1.0);
      this->SetColorName(509, "C19"); this->GetLookupTable()->SetTableValue(509, 0.179688, 0.542969, 0.339844, 1.0);
      this->SetColorName(82, "C20"); this->GetLookupTable()->SetTableValue(82, 0.996094, 0.484375, 0.246094, 1.0);
      this->SetColorName(513, "C21"); this->GetLookupTable()->SetTableValue(513, 0.953125, 0.636719, 0.375, 1.0);
      this->SetColorName(72, "C22"); this->GetLookupTable()->SetTableValue(72, 0.199219, 0.625, 0.785156, 1.0);
      this->SetColorName(142, "C23"); this->GetLookupTable()->SetTableValue(142, 0.882812, 0.804688, 0.335938, 1.0);
      this->SetColorName(511, "C24"); this->GetLookupTable()->SetTableValue(511, 0.738281, 0.710938, 0.414062, 1.0);
      this->SetColorName(301, "C25"); this->GetLookupTable()->SetTableValue(301, 0.304688, 0.8125, 0.796875, 1.0);
      this->SetColorName(85, "C26"); this->GetLookupTable()->SetTableValue(85, 0, 0.742188, 0.996094, 1.0);
      this->SetColorName(563, "C27"); this->GetLookupTable()->SetTableValue(563, 0.996094, 0.269531, 0, 1.0);
      this->SetColorName(36, "C28"); this->GetLookupTable()->SetTableValue(36, 0.492188, 0.996094, 0, 1.0);
      this->SetColorName(501, "C29"); this->GetLookupTable()->SetTableValue(501, 0.496094, 0.160156, 0.160156, 1.0);
      this->SetColorName(521, "C30"); this->GetLookupTable()->SetTableValue(521, 0.238281, 0.347656, 0.664062, 1.0);
      this->SetColorName(21, "C31"); this->GetLookupTable()->SetTableValue(21, 0.953125, 0.636719, 0.375, 1.0);
      this->SetColorName(22, "C32"); this->GetLookupTable()->SetTableValue(22, 0.480469, 0.683594, 0.96875, 1.0);
      this->SetColorName(23, "C33"); this->GetLookupTable()->SetTableValue(23, 0.871094, 0.996094, 0.996094, 1.0);
      this->SetColorName(24, "C34"); this->GetLookupTable()->SetTableValue(24, 0.671875, 0.84375, 0.898438, 1.0);
      this->SetColorName(25, "C35"); this->GetLookupTable()->SetTableValue(25, 0.425781, 0.996094, 0.4375, 1.0);
      this->SetColorName(26, "C36"); this->GetLookupTable()->SetTableValue(26, 0.882812, 0.65625, 0.410156, 1.0);
      this->SetColorName(27, "C37"); this->GetLookupTable()->SetTableValue(27, 0.976562, 0.976562, 0.664062, 1.0);
      this->SetColorName(28, "C38"); this->GetLookupTable()->SetTableValue(28, 0.996094, 0.9375, 0.820312, 1.0);
      this->SetColorName(29, "C39"); this->GetLookupTable()->SetTableValue(29, 0.398438, 0.496094, 0.078125, 1.0);
      this->SetColorName(30, "C40"); this->GetLookupTable()->SetTableValue(30, 0.996094, 0.867188, 0.671875, 1.0);
      this->SetColorName(31, "C41"); this->GetLookupTable()->SetTableValue(31, 0.996094, 0.882812, 0.0078125, 1.0);
      this->SetColorName(32, "C42"); this->GetLookupTable()->SetTableValue(32, 0.492188, 0.980469, 0.0859375, 1.0);
      this->SetColorName(33, "C43"); this->GetLookupTable()->SetTableValue(33, 0.59375, 0.976562, 0.59375, 1.0);
      this->SetColorName(34, "C44"); this->GetLookupTable()->SetTableValue(34, 0.773438, 0.464844, 0.148438, 1.0);
      this->SetColorName(35, "C45"); this->GetLookupTable()->SetTableValue(35, 0.996094, 0.597656, 0.0664062, 1.0);
      this->SetColorName(5, "C46"); this->GetLookupTable()->SetTableValue(5, 0, 0.773438, 0.9375, 1.0);
      this->SetColorName(6, "C47"); this->GetLookupTable()->SetTableValue(6, 0.671875, 0.84375, 0.898438, 1.0);
      this->SetColorName(84, "C48"); this->GetLookupTable()->SetTableValue(84, 1, 0.7, 0.6, 1.0);
      this->SetColorName(115, "C49"); this->GetLookupTable()->SetTableValue(115, 0.99, 0.8, 0.3, 1.0);
      this->SetColorName(81, "C50"); this->GetLookupTable()->SetTableValue(81, 0.9, 0.4, 0.4, 1.0);
      this->SetColorName(91, "C51"); this->GetLookupTable()->SetTableValue(91, 1, 0.35, 0.7, 1.0);
      this->SetColorName(94, "C52"); this->GetLookupTable()->SetTableValue(94, 0.85, 0.5, 0.48, 1.0);
      this->SetColorName(74, "C53"); this->GetLookupTable()->SetTableValue(74, 1, 0.6, 0.55, 1.0);
      this->SetColorName(36, "C54"); this->GetLookupTable()->SetTableValue(36, 0.996094, 0.835938, 0, 1.0);
      this->SetColorName(37, "C55"); this->GetLookupTable()->SetTableValue(37, 0.6875, 0.761719, 0.867188, 1.0);
      this->SetColorName(88, "C56"); this->GetLookupTable()->SetTableValue(88, 1, 0.68, 0.38, 1.0);
      this->SetColorName(80, "C57"); this->GetLookupTable()->SetTableValue(80, 1, 0.5, 0.48, 1.0);
      this->SetColorName(86, "C58"); this->GetLookupTable()->SetTableValue(86, 1, 0.6, 0.55, 1.0);
      this->SetColorName(99, "C59"); this->GetLookupTable()->SetTableValue(99, 1, 0.5, 0.6, 1.0);
      this->SetColorName(38, "C60"); this->GetLookupTable()->SetTableValue(38, 0.9375, 0.898438, 0.542969, 1.0);
      this->SetColorName(95, "C61"); this->GetLookupTable()->SetTableValue(95, 1, 0.35, 0.45, 1.0);
      this->SetColorName(117, "C62"); this->GetLookupTable()->SetTableValue(117, 0.99, 0.8, 0, 1.0);
      this->SetColorName(39, "C63"); this->GetLookupTable()->SetTableValue(39, 0.25, 0.871094, 0.8125, 1.0);
      this->SetColorName(90, "C64"); this->GetLookupTable()->SetTableValue(90, 1, 0.5, 0.48, 1.0);
      this->SetColorName(40, "C65"); this->GetLookupTable()->SetTableValue(40, 0.128906, 0.542969, 0.128906, 1.0);
      this->SetColorName(41, "C66"); this->GetLookupTable()->SetTableValue(41, 0.824219, 0.0976562, 0.117188, 1.0);
      this->SetColorName(77, "C67"); this->GetLookupTable()->SetTableValue(77, 1, 0.65, 0.64, 1.0);
      this->SetColorName(75, "C68"); this->GetLookupTable()->SetTableValue(75, 1, 0.5, 0.6, 1.0);
      this->SetColorName(83, "C69"); this->GetLookupTable()->SetTableValue(83, 1, 0.35, 0.45, 1.0);
      this->SetColorName(87, "C70"); this->GetLookupTable()->SetTableValue(87, 1, 0.5, 0.6, 1.0);
      this->SetColorName(42, "C71"); this->GetLookupTable()->SetTableValue(42, 0.953125, 0.636719, 0.375, 1.0);
      this->SetColorName(43, "C72"); this->GetLookupTable()->SetTableValue(43, 0.851562, 0.933594, 0.996094, 1.0);
      this->SetColorName(44, "C73"); this->GetLookupTable()->SetTableValue(44, 0.304688, 0.8125, 0.796875, 1.0);
      this->SetColorName(45, "C74"); this->GetLookupTable()->SetTableValue(45, 0, 0.773438, 0.9375, 1.0);
      this->SetColorName(46, "C75"); this->GetLookupTable()->SetTableValue(46, 0.425781, 0.996094, 0.4375, 1.0);
      this->SetColorName(47, "C76"); this->GetLookupTable()->SetTableValue(47, 0.773438, 0.464844, 0.148438, 1.0);
      this->SetColorName(48, "C77"); this->GetLookupTable()->SetTableValue(48, 0.25, 0.410156, 0.878906, 1.0);
      this->SetColorName(49, "C78"); this->GetLookupTable()->SetTableValue(49, 0.882812, 0.804688, 0.339844, 1.0);
      this->SetColorName(50, "C79"); this->GetLookupTable()->SetTableValue(50, 0.496094, 0.160156, 0.160156, 1.0);
      this->SetColorName(51, "C80"); this->GetLookupTable()->SetTableValue(51, 0.683594, 0.925781, 0.925781, 1.0);
      this->SetColorName(52, "C81"); this->GetLookupTable()->SetTableValue(52, 0.816406, 0.410156, 0.113281, 1.0);
      this->SetColorName(53, "C82"); this->GetLookupTable()->SetTableValue(53, 0.9375, 0.898438, 0.542969, 1.0);
      this->SetColorName(54, "C83"); this->GetLookupTable()->SetTableValue(54, 0.238281, 0.742188, 0.511719, 1.0);
      this->SetColorName(55, "C84"); this->GetLookupTable()->SetTableValue(55, 0.671875, 0.84375, 0.898438, 1.0);
      this->SetColorName(56, "C85"); this->GetLookupTable()->SetTableValue(56, 0.996094, 0.996094, 0.605469, 1.0);
      this->SetColorName(57, "C86"); this->GetLookupTable()->SetTableValue(57, 0.882812, 0.65625, 0.410156, 1.0);
      this->SetColorName(58, "C87"); this->GetLookupTable()->SetTableValue(58, 0.996094, 0.9375, 0.820312, 1.0);
      this->SetColorName(59, "C88"); this->GetLookupTable()->SetTableValue(59, 0.179688, 0.542969, 0.339844, 1.0);
      this->SetColorName(60, "C89"); this->GetLookupTable()->SetTableValue(60, 0.996094, 0.492188, 0.308594, 1.0);
      this->SetColorName(61, "C90"); this->GetLookupTable()->SetTableValue(61, 0.738281, 0.710938, 0.414062, 1.0);
      this->SetColorName(62, "C91"); this->GetLookupTable()->SetTableValue(62, 0.492188, 0.980469, 0.0859375, 1.0);
      this->SetColorName(63, "C92"); this->GetLookupTable()->SetTableValue(63, 0.824219, 0.0976562, 0.117188, 1.0);
      this->SetColorName(64, "C93"); this->GetLookupTable()->SetTableValue(64, 0.445312, 0.238281, 0.0976562, 1.0);
      this->SetColorName(65, "C94"); this->GetLookupTable()->SetTableValue(65, 0.953125, 0.636719, 0.375, 1.0);
      this->SetColorName(66, "C95"); this->GetLookupTable()->SetTableValue(66, 0.480469, 0.683594, 0.96875, 1.0);
      this->SetColorName(67, "C96"); this->GetLookupTable()->SetTableValue(67, 0.238281, 0.347656, 0.664062, 1.0);
      this->SetColorName(68, "C97"); this->GetLookupTable()->SetTableValue(68, 0.773438, 0.464844, 0.148438, 1.0);
      this->SetColorName(69, "C98"); this->GetLookupTable()->SetTableValue(69, 0, 0.800781, 0.8125, 1.0);
      this->SetColorName(70, "C99"); this->GetLookupTable()->SetTableValue(70, 0.851562, 0.433594, 0.574219, 1.0);
      this->SetColorName(78, "C100"); this->GetLookupTable()->SetTableValue(78, 1, 0.5, 0.48, 1.0);
      this->SetColorName(120, "C101"); this->GetLookupTable()->SetTableValue(120, 0.996094, 0.886719, 0.878906, 1.0);
      this->SetColorName(119, "C102"); this->GetLookupTable()->SetTableValue(119, 0.996094, 0.996094, 0.9375, 1.0);
      this->SetColorName(71, "C103"); this->GetLookupTable()->SetTableValue(71, 0.496094, 0.496094, 0.40625, 1.0);
      this->SetColorName(79, "C104"); this->GetLookupTable()->SetTableValue(79, 1, 0.35, 0.7, 1.0);
      this->SetColorName(82, "C105"); this->GetLookupTable()->SetTableValue(82, 0.85, 0.5, 0.48, 1.0);
      this->SetColorName(96, "C106"); this->GetLookupTable()->SetTableValue(96, 1, 0.7, 0.6, 1.0);
      this->SetColorName(98, "C107"); this->GetLookupTable()->SetTableValue(98, 1, 0.6, 0.55, 1.0);
      this->SetColorName(97, "C108"); this->GetLookupTable()->SetTableValue(97, 0.9, 0.6, 0.35, 1.0);
      this->SetColorName(76, "C109"); this->GetLookupTable()->SetTableValue(76, 1, 0.68, 0.38, 1.0);
      this->SetColorName(92, "C110"); this->GetLookupTable()->SetTableValue(92, 1, 0.5, 0.48, 1.0);
      this->SetColorName(72, "C111"); this->GetLookupTable()->SetTableValue(72, 0.492188, 0.996094, 0.828125, 1.0);
      this->SetColorName(102, "C112"); this->GetLookupTable()->SetTableValue(102, 1, 0.59, 0.65, 1.0);
      this->SetColorName(105, "C113"); this->GetLookupTable()->SetTableValue(105, 0.6, 0.8, 0.15, 1.0);
      this->SetColorName(73, "C114"); this->GetLookupTable()->SetTableValue(73, 0.882812, 0.507812, 0.0859375, 1.0);
      this->SetColorName(110, "C115"); this->GetLookupTable()->SetTableValue(110, 0.6, 0.5, 0.15, 1.0);
      this->SetColorName(111, "C116"); this->GetLookupTable()->SetTableValue(111, 0.8, 0.5, 0, 1.0);
      this->SetColorName(112, "C117"); this->GetLookupTable()->SetTableValue(112, 0.99, 0.99, 0.15, 1.0);
      this->SetColorName(114, "C118"); this->GetLookupTable()->SetTableValue(114, 0.6, 0.8, 0, 1.0);
      this->SetColorName(106, "C119"); this->GetLookupTable()->SetTableValue(106, 0.8, 0.8, 0.15, 1.0);
      this->SetColorName(107, "C120"); this->GetLookupTable()->SetTableValue(107, 0.99, 0.8, 0.15, 1.0);
      this->SetColorName(108, "C121"); this->GetLookupTable()->SetTableValue(108, 0.8, 0.8, 0.8, 1.0);
      this->SetColorName(200, "Cnerves"); this->GetLookupTable()->SetTableValue(200, 0.38, 0.6, 1.0, 1.0);
      this->SetColorName(201, "Cmuscles"); this->GetLookupTable()->SetTableValue(201, 0.79, 0.79, 0.47, 1.0);
      this->SetColorName(202, "Cinsula"); this->GetLookupTable()->SetTableValue(202, 1, 1, 0.2, 1.0);
      this->SetColorName(203, "Coccip"); this->GetLookupTable()->SetTableValue(203, 0.78, 0.47, 0.15, 1.0);
      this->SetColorName(204, "Cthalamus"); this->GetLookupTable()->SetTableValue(204, 1, 0.27, 0, 1.0);
      this->SetColorName(205, "Cdienc"); this->GetLookupTable()->SetTableValue(205, 0.89, 0.51, 0.086, 1.0);
      this->SetColorName(206, "Cpari"); this->GetLookupTable()->SetTableValue(206, 0.78, 0.47, 0.88, 1.0);
      this->SetColorName(207, "Ctempor"); this->GetLookupTable()->SetTableValue(207, 0.25, 0.41, 0.88, 1.0);
      this->SetColorName(207, "Cfront"); this->GetLookupTable()->SetTableValue(207, 0.039, 0.79, 0.17, 1.0);
      this->SetColorName(208, "Cextr"); this->GetLookupTable()->SetTableValue(208, 1, 0.34, 0.13, 1.0);
      this->SetColorName(209, "Ccerebellum"); this->GetLookupTable()->SetTableValue(209, 0.85, 0.92, 1, 1.0);
      this->SetColorName(210, "Cwm"); this->GetLookupTable()->SetTableValue(210, 0.93, 0.91, 0.67, 1.0);
      this->SetColorName(211, "Cbas"); this->GetLookupTable()->SetTableValue(211, 0.25, 0.41, 0.88, 1.0);
      this->SetColorName(212, "Climb"); this->GetLookupTable()->SetTableValue(212, 0, 0, 1, 1.0);
      this->SetColorName(213, "Cvis"); this->GetLookupTable()->SetTableValue(213, 0.94, 0.9, 0.55, 1.0);
      this->SetColorName(214, "Cstem"); this->GetLookupTable()->SetTableValue(214, 0.64, 0.58, 0.5, 1.0);
      this->SetColorName(215, "Cvent"); this->GetLookupTable()->SetTableValue(215, 0.9, 0.9, 0.98, 1.0);
      this->SetColorName(216, "Cbrain"); this->GetLookupTable()->SetTableValue(216, 0.25, 0.41, 0.88, 1.0);
      this->SetColorName(217, "Cneo"); this->GetLookupTable()->SetTableValue(217, 0.86, 0.86, 0.86, 1.0);

      // from SPL-BrainAtlas-label_list.txt
      // these ones reuse the colours from above labels
      this->SetColorName(500, "R internal medullary lamina"); this->GetLookupTable()->SetTableValue(500, 0.496094, 0.160156, 0.160156, 1.0);
      this->SetColorName(508, "R anterior thalamic nucleus"); this->GetLookupTable()->SetTableValue(508, 0.179688, 0.542969, 0.339844, 1.0);
      this->SetColorName(510, "R dorsomedial thalamic nucleus"); this->GetLookupTable()->SetTableValue(510, 0.738281, 0.710938, 0.414062, 1.0);
      this->SetColorName(512, "R centromedian thalamic nucleus"); this->GetLookupTable()->SetTableValue(512, 0.953125, 0.636719, 0.375, 1.0);
      this->SetColorName(520, "R lateral dorsal thalamic nucleus"); this->GetLookupTable()->SetTableValue(520, 0.238281, 0.347656, 0.664062, 1.0);
      this->SetColorName(300, "R eyeball"); this->GetLookupTable()->SetTableValue(300, 0.5, 0.5, 0.5, 1.0);
      this->SetColorName(301, "L eyeball"); this->GetLookupTable()->SetTableValue(301, 0.5, 0.5, 0.5, 1.0);
      this->SetColorName(310, "right optic nerve"); this->GetLookupTable()->SetTableValue(310, 0.8, 0.8, 0.2, 1.0);
      this->SetColorName(311, "left optic nerve");  this->GetLookupTable()->SetTableValue(311, 0.8, 0.8, 0.2, 1.0);
      this->SetColorName(320, "R lateral geniculate body");  this->GetLookupTable()->SetTableValue(320, 0.4, 0.7, 1.0, 1.0);
      this->SetColorName(321, "L lateral geniculate body");  this->GetLookupTable()->SetTableValue(321, 0.2, 0.2, 0.8, 1.0);
      this->SetColorName(330, "R optic radiation");   this->GetLookupTable()->SetTableValue(330, 1.0, 1.0, 1.0, 1.0);
      this->SetColorName(331, "L optic radiation");   this->GetLookupTable()->SetTableValue(331, 1.0, 1.0, 1.0, 1.0);
      this->SetColorName(340, "pituitary gland");   this->GetLookupTable()->SetTableValue(340, 1, 0.5, 0.48, 1.0);
      this->SetColorName(360, "Internal capsule R (ant. limb)"); this->GetLookupTable()->SetTableValue(360, 1.0, 1.0, 1.0, 1.0);
      this->SetColorName(361, "Internal capsule L (ant. limb)"); this->GetLookupTable()->SetTableValue(361, 1.0, 1.0, 1.0, 1.0);
      this->SetColorName(440, "R internal capsule posterior limb"); this->GetLookupTable()->SetTableValue(440, 1.0, 1.0, 1.0, 1.0);
      this->SetColorName(441, "L internal capsule posterior limb"); this->GetLookupTable()->SetTableValue(441, 1.0, 1.0, 1.0, 1.0);
      this->SetColorName(501, "L internal medullary lamina"); this->GetLookupTable()->SetTableValue(501, 1.0, 1.0, 1.0, 1.0);
      this->SetColorName(502, "R medial geniculate body"); this->GetLookupTable()->SetTableValue(502, 0.492188, 0.980469, 0.0859375, 1.0);
      this->SetColorName(503, "L medial geniculate body"); this->GetLookupTable()->SetTableValue(503, 0.492188, 0.980469, 0.0859375, 1.0);
      this->SetColorName(504, "R lateral geniculate body"); this->GetLookupTable()->SetTableValue(504, 0.4, 0.7, 1.0, 1.0);
      this->SetColorName(505, "L lateral geniculate body"); this->GetLookupTable()->SetTableValue(505, 0.2, 0.2, 0.8, 1.0);
      this->SetColorName(506, "R pulvinar"); this->GetLookupTable()->SetTableValue(506, 0.25, 0.41, 0.88, 1.0);
      this->SetColorName(507, "L pulvinar"); this->GetLookupTable()->SetTableValue(507, 0.238281, 0.347656, 0.664062, 1.0);
      this->SetColorName(509, "L anterior thalamic nucleus"); this->GetLookupTable()->SetTableValue(509, 1, 0.27, 0, 1.0);
      this->SetColorName(511, "L dorsomedial thalamic nucleus"); this->GetLookupTable()->SetTableValue(511, 0.238281, 0.742188, 0.511719, 1.0);
      this->SetColorName(513, "L centromedian thalamic nucleus"); this->GetLookupTable()->SetTableValue(513, 0.8, 0.8, 0.8, 1.0);
      this->SetColorName(514, "R ventral anterior thalamic nucleus"); this->GetLookupTable()->SetTableValue(514, 0.996094, 0.835938, 0, 1.0);
      this->SetColorName(515, "L ventral anterior thalamic nucleus"); this->GetLookupTable()->SetTableValue(515, 0.996094, 0.996094, 0.605469, 1.0);
      this->SetColorName(516, "R ventral lateral thalamic nucleus"); this->GetLookupTable()->SetTableValue(516, 0.179688, 0.542969, 0.339844, 1.0);
      this->SetColorName(517, "L ventral lateral thalamic nucleus"); this->GetLookupTable()->SetTableValue(517, 0.238281, 0.742188, 0.511719, 1.0);
      this->SetColorName(518, "R lateral posterior thalamic nucleus"); this->GetLookupTable()->SetTableValue(518, 1, 0.5, 0.48, 1.0);
      this->SetColorName(519, "L lateral posterior thalamic nucleus"); this->GetLookupTable()->SetTableValue(519, 0.8, 0.5, 0, 1.0);
      this->SetColorName(521, "L lateral dorsal thalamic nucleus"); this->GetLookupTable()->SetTableValue(521, 0.25, 0.41, 0.88, 1.0);
      this->SetColorName(522, "R VPM"); this->GetLookupTable()->SetTableValue(522, 1, 0.35, 0.45, 1.0);
      this->SetColorName(523, "L VPM"); this->GetLookupTable()->SetTableValue(523, 0.882812, 0.65625, 0.410156, 1.0);
      this->SetColorName(524, "R VPL"); this->GetLookupTable()->SetTableValue(524, 0.9, 0.5, 0.5, 1.0);
      this->SetColorName(525, "L VPL"); this->GetLookupTable()->SetTableValue(525, 1, 0.35, 0.45, 1.0);

      // names are now initialised
      this->NamesInitialisedOn();
      }
    else if (this->Type == this->Random)
      {
      int size = 255;
      
      this->GetLookupTable()->SetTableValue(0, 0, 0, 0, 0);
      this->GetLookupTable()->SetRange(0, size);
      this->GetLookupTable()->SetNumberOfTableValues(size + 1);
      for (int i = 1; i <= size; i++)
        {
        // table values have to be 0-1
        double r = (rand()%255)/255.0;
        double g = (rand()%255)/255.0;
        double b = (rand()%255)/255.0;
       
        this->GetLookupTable()->SetTableValue(i, r, g, b, 1.0);
        }
      this->SetNamesFromColors();      
      }

    else if (this->Type == this->User)
      {
      this->LookupTable->SetNumberOfTableValues(0);
      this->LastAddedColor = -1;
      vtkDebugMacro("Set type to user, call SetNumberOfColors, then AddColor..");
      }

    else if (this->Type == this->File)
      {
      vtkDebugMacro("Set type to file, call SetFileName and ReadFile next...");
      }
    
    else
      {
      vtkErrorMacro("vtkMRMLColorTableNode: SetType ERROR, unknown type " << type << endl);
      return;
      }
    // invoke a modified event
    this->Modified();
    
    // invoke a type  modified event
    this->InvokeEvent(vtkMRMLColorTableNode::TypeModifiedEvent);
}

//---------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetNamesFromColors()
{
  int size = this->GetLookupTable()->GetNumberOfColors();
  double *rgba;
  // reset the names
  this->Names.clear();
  this->Names.resize(size);
  for (int i = 0; i < size; i++)
    {
    rgba = this->GetLookupTable()->GetTableValue(i);
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
  this->NamesInitialisedOn();
}

//---------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetNumberOfColors(int n)
{
  if (this->GetLookupTable() == NULL)
    {
    vtkErrorMacro("SetNumberofColors: lookup table is null, set the type first.");
    return;
    }
  if (this->GetType() != this->User &&
      this->GetType() != this->File)
    {
      vtkErrorMacro("vtkMRMLColorTableNode::SetNumberOfColors: ERROR: can't set number of colours if not a user defined colour table, reset the type first to User or File\n");
      return;
    }

  if (this->GetLookupTable()->GetNumberOfTableValues() != n)
    {
    this->GetLookupTable()->SetNumberOfTableValues(n);
    }

  if (this->Names.size() != n)
    {
    this->Names.resize(n);
    }
  
}

//---------------------------------------------------------------------------
int vtkMRMLColorTableNode::GetNumberOfColors()
{
  if (this->GetLookupTable() != NULL)
    {
      return this->GetLookupTable()->GetNumberOfTableValues();
    }
  else
    {
      return 0;
    }
}
//---------------------------------------------------------------------------
void vtkMRMLColorTableNode::AddColor(const char *name, double r, double g, double b)
{
 if (this->GetType() != this->User)
    {
      vtkErrorMacro("vtkMRMLColorTableNode::AddColor: ERROR: can't add a colour if not a user defined colour table, reset the type first to User\n");
      return;
    }
 this->LastAddedColor++;
 this->SetColor(this->LastAddedColor, name, r, g, b);
}

//---------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetColor(int entry, const char *name, double r, double g, double b)
{
  if (this->GetType() != this->User)
    {
      vtkErrorMacro( "vtkMRMLColorTableNode::SetColor: ERROR: can't set a colour if not a user defined colour table, reset the type first to User\n");
      return;
    }
  if (entry < 0 ||
      entry >= this->GetLookupTable()->GetNumberOfTableValues())
    {
    vtkErrorMacro( "vtkMRMLColorTableNode::SetColor: requested entry " << entry << " is out of table range: 0 - " << this->GetLookupTable()->GetNumberOfTableValues() << ", call SetNumberOfColors" << endl);
      return;
    }

  this->GetLookupTable()->SetTableValue(entry, r, g, b, 1.0);
  if (strcmp(this->GetColorName(entry), name) != 0)
    {
      this->SetColorName(entry, name);
    }

  // trigger a modified event
  this->InvokeEvent (vtkCommand::ModifiedEvent);
}


//---------------------------------------------------------------------------
void vtkMRMLColorTableNode::ClearNames()
{
  this->Names.clear();
  this->NamesInitialisedOff();
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLColorTableNode::GetStorageNode()
{
  vtkMRMLStorageNode* node = NULL;
  if (this->GetScene() && this->GetStorageNodeID() )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID(this->StorageNodeID);
    node = vtkMRMLStorageNode::SafeDownCast(snode);
    }
  return node;
}
