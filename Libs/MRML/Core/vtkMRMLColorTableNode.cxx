/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLColorTableNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.0 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLColorTableStorageNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <random>
#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLColorTableNode);

//----------------------------------------------------------------------------
vtkMRMLColorTableNode::vtkMRMLColorTableNode()
{
  this->SetName("");
  this->SetDescription("Color Table");
  this->LookupTable = nullptr;
  this->LastAddedColor = -1;
}

//----------------------------------------------------------------------------
vtkMRMLColorTableNode::~vtkMRMLColorTableNode()
{
  this->SetAndObserveLookupTable(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their FullRainbows

  Superclass::WriteXML(of, nIndent);

  // only print out the look up table size so that the table can be
  // initialized properly
  if (this->LookupTable != nullptr)
    {
    of << " numcolors=\"" << this->LookupTable->GetNumberOfTableValues() << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  int numColours;
  while (*atts != nullptr)
  {
      attName = *(atts++);
      attValue = *(atts++);
      if (!strcmp(attName, "numcolors"))
        {
        std::stringstream ss;
        ss << attValue;
        ss >> numColours;
        vtkDebugMacro("Setting the look up table size to " << numColours << "\n");
        this->SetNumberOfColors(numColours);
        // init the table to black/opacity 0 with no name, just in case we're missing values
        const char *noName = this->GetNoName();
        if (!noName)
          {
          noName = "(none)";
          }
        for (int i = 0; i < numColours; i++)
          {
          this->SetColor(i, noName, 0.0, 0.0, 0.0, 0.0);
          }
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
        // might have a version of a mrml file that has tick marks around
        // the name
        const char *tickPtr = strstr(name.c_str(), "'");
        if (tickPtr)
          {
          size_t firstValidChar = name.find_first_not_of("'");
          size_t lastValidChar = name.find_last_not_of("'");
          name = name.substr(firstValidChar, 1 + lastValidChar - firstValidChar);
          }
        vtkDebugMacro("Adding colour at index " << index << ", r = " << r << ", g = " << g << ", b = " << b << ", a = " << a << " and then setting name to " << name.c_str() << endl);

        if (this->SetColorNameWithSpaces(index, name.c_str(), "_") != 0)
          {
          this->LookupTable->SetTableValue(index, r, g, b, a);
          }
        }
      // set the table range
      if ( this->LookupTable->GetNumberOfTableValues() > 0 )
        {
          this->LookupTable->SetRange(0, 255);
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
      else
      {
          vtkDebugMacro ("Unknown attribute name " << attName << endl);
      }
  }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLColorTableNode::Copy(vtkMRMLNode *anode)
{
  /// BUG 3992: when custom color tables appear in scene views,
  /// the color information is saved in a file on disk and not
  /// read into the scene view copy of the node. Continuing
  /// with the copy will remove the color information from the
  /// node in the main scene, so return to preserve it.
  /// See also vtkMRMLMarkupsNode::Copy.
  /// TBD: if scene view node reading xml triggers reading the data from
  // storage nodes, this should no longer be necessary.
  if (this->Scene &&
      this->Scene->IsRestoring())
    {
#ifndef _NDEBUG
    vtkWarningMacro("ColorTable Copy inside scene view restore: "
                    << "colors to restore are missing, skipping.");
#endif
    return;
    }
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLColorTableNode *node = (vtkMRMLColorTableNode *) anode;

  // Deep copy LookupTable
  if (node->GetLookupTable() != nullptr)
    {
    if (this->LookupTable == nullptr)
      {
      vtkNew<vtkLookupTable> lut;
      this->SetAndObserveLookupTable(lut.GetPointer());
      }
    if (this->LookupTable != node->GetLookupTable())
      {
      this->LookupTable->DeepCopy(node->GetLookupTable());
      }
    }
  else
    {
    this->SetAndObserveLookupTable(nullptr);
    }

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  if (this->LookupTable != nullptr)
    {
    os << indent << "Look up table:\n";
    this->LookupTable->PrintSelf(os, indent.GetNextIndent());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToFullRainbow()
{
    this->SetType(this->FullRainbow);
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
void vtkMRMLColorTableNode::SetTypeToRandom()
{
  this->SetType(this->Random);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToRed()
{
  this->SetType(this->Red);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToGreen()
{
    this->SetType(this->Green);
}
//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToBlue()
{
    this->SetType(this->Blue);
}
//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToCyan()
{
    this->SetType(this->Cyan);
}
//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToMagenta()
{
    this->SetType(this->Magenta);
}
//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToYellow()
{
    this->SetType(this->Yellow);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToWarm1()
{
    this->SetType(this->Warm1);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToWarm2()
{
    this->SetType(this->Warm2);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToWarm3()
{
    this->SetType(this->Warm3);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToCool1()
{
    this->SetType(this->Cool1);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToCool2()
{
    this->SetType(this->Cool2);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToCool3()
{
    this->SetType(this->Cool3);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToWarmShade1()
{
    this->SetType(this->WarmShade1);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToWarmShade2()
{
    this->SetType(this->WarmShade2);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToWarmShade3()
{
    this->SetType(this->WarmShade3);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToCoolShade1()
{
    this->SetType(this->CoolShade1);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToCoolShade2()
{
    this->SetType(this->CoolShade2);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToCoolShade3()
{
    this->SetType(this->CoolShade3);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToWarmTint1()
{
    this->SetType(this->WarmTint1);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToWarmTint2()
{
    this->SetType(this->WarmTint2);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToWarmTint3()
{
    this->SetType(this->WarmTint3);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToCoolTint1()
{
    this->SetType(this->CoolTint1);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToCoolTint2()
{
    this->SetType(this->CoolTint2);
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetTypeToCoolTint3()
{
    this->SetType(this->CoolTint3);
}

//----------------------------------------------------------------------------
const char* vtkMRMLColorTableNode::GetTypeAsString()
{
  const char *type = Superclass::GetTypeAsString();
  if (type && strcmp(type,"(unknown)") != 0)
    {
      return type;
    }
  if (this->Type == this->FullRainbow)
    {
    return "FullRainbow";
    }
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
  if (this->Type == this->Red)
    {
    return "Red";
    }
  if (this->Type == this->Green)
    {
    return "Green";
    }
  if (this->Type == this->Blue)
    {
    return "Blue";
    }
  if (this->Type == this->Cyan)
    {
    return "Cyan";
    }
  if (this->Type == this->Magenta)
    {
    return "Magenta";
    }
  if (this->Type == this->Yellow)
    {
    return "Yellow";
    }
  if (this->Type == this->Warm1)
    {
    return "Warm1";
    }
  if (this->Type == this->Warm2)
    {
    return "Warm2";
    }
  if (this->Type == this->Warm3)
    {
    return "Warm3";
    }
  if (this->Type == this->Cool1)
    {
    return "Cool1";
    }
  if (this->Type == this->Cool2)
    {
    return "Cool2";
    }
  if (this->Type == this->Cool3)
    {
    return "Cool3";
    }
  if (this->Type == this->WarmShade1)
    {
    return "WarmShade1";
    }
  if (this->Type == this->WarmShade2)
    {
    return "WarmShade2";
    }
  if (this->Type == this->WarmShade3)
    {
    return "WarmShade3";
    }
  if (this->Type == this->CoolShade1)
    {
    return "CoolShade1";
    }
  if (this->Type == this->CoolShade2)
    {
    return "CoolShade2";
    }
  if (this->Type == this->CoolShade3)
    {
    return "CoolShade3";
    }
  if (this->Type == this->WarmTint1)
    {
    return "WarmTint1";
    }
  if (this->Type == this->WarmTint2)
    {
    return "WarmTint2";
    }
  if (this->Type == this->WarmTint3)
    {
    return "WarmTint3";
    }
  if (this->Type == this->CoolTint1)
    {
    return "CoolTint1";
    }
  if (this->Type == this->CoolTint2)
    {
    return "CoolTint2";
    }
  if (this->Type == this->CoolTint3)
    {
    return "CoolTint3";
    }
  return "(unknown)";
}

//---------------------------------------------------------------------------
void vtkMRMLColorTableNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  // Emit a node modified event if the lookup table object is modified
  if (caller != nullptr && caller == this->LookupTable && event == vtkCommand::ModifiedEvent)
    {
    Modified();
    }

  return;
}

//---------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetType(int type)
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
      vtkDebugMacro("vtkMRMLColorTableNode::SetType Creating a new lookup table (was null) of type " << this->GetTypeAsString() << "\n");
      vtkLookupTable *table = vtkLookupTable::New();
      this->SetLookupTable(table);
      table->Delete();
      // as a FullRainbow, set the table range to 255
      this->GetLookupTable()->SetTableRange(0, 255);
      }

    // delay setting names from colours until asked for one
    if (this->Type == this->FullRainbow)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0, 1);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(1, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A full rainbow of 256 colors, goes from red to red with all rainbow colors in between. Useful for colourful display of a label map");
      }
    else if (this->Type == this->Grey)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0, 0);
      this->GetLookupTable()->SetSaturationRange(0, 0);
      this->GetLookupTable()->SetValueRange(0, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A grey scale ranging from black at 0 to white at 255. Useful for displaying MRI volumes.");
      }
    else if (this->Type == this->Red)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0, 0);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(0, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A red scale of 256 values. Useful for layering with Cyan");
      }
    else if (this->Type == this->Green)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0.333, 0.333);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(0, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A green scale of 256 values, useful for layering with Magenta");
      }
    else if (this->Type == this->Blue)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0.667, 0.667);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(0, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A blue scale of 256 values from black to pure blue, useful for layering with Yellow");
      }
    else if (this->Type == this->Yellow)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0.167, 0.167);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(0, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A yellow scale of 256 values, from black to pure yellow, useful for layering with blue (it's complementary, layering yields gray)");
      }
    else if (this->Type == this->Cyan)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0.5, 0.5);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(0, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A cyan ramp of 256 values, from black to cyan, complementary ramp to red, layering yields gray");
      }
    else if (this->Type == this->Magenta)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0.833, 0.833);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(0, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A magenta scale of 256 colors from black to magenta, complementary ramp to green, layering yields gray ");
      }
    else if (this->Type == this->WarmShade1)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0.167, 0.0);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(0, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from black to red, of 256 colors, ramp of warm colors with variation in value that's complementary to CoolShade1 ");
      }
    else if (this->Type == this->WarmShade2)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(.333, 0.167);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(0, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from black to yellow, through green, of 256 colors, ramp of warm colors with variation in value that's complementary to CoolShade2 ");
      }
    else if (this->Type == this->WarmShade3)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0.5, 0.333);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(0, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from black to green, of 256 colours, ramp of warm colors with variation in value that's complementary to CoolShade3 ");
      }
    else if (this->Type == this->CoolShade1)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0.667, 0.5);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(0, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from black to cyan, 256 colours, ramp of cool colours with variation in value that is complementary to WarmShade1 ");
      }
    else if (this->Type == this->CoolShade2)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0.833, 0.667);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(0, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from black to blue through purple, 256 colors, ramp of cool colours with variation in value that is complementary to WarmShade2 ");
      }
    else if (this->Type == this->CoolShade3)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(1.0, 0.833);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(0, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from black to magenta, 256 colours, ramp of cool colours with variation in value that is complementary to WarmShade3");
      }
    else if (this->Type == this->WarmTint1)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0.167, 0.0);
      this->GetLookupTable()->SetSaturationRange(0, 1);
      this->GetLookupTable()->SetValueRange(1, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from white to red, 256 colours, ramp of warm colours with variation in saturation that's complementary to CoolTint1");
      }
    else if (this->Type == this->WarmTint2)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(.333, 0.167);
      this->GetLookupTable()->SetSaturationRange(0, 1);
      this->GetLookupTable()->SetValueRange(1, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from white to yellow, 256 colours, ramp of warm colours with variation in saturation that's complementary to CoolTint2");
      }
    else if (this->Type == this->WarmTint3)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0.5, 0.333);
      this->GetLookupTable()->SetSaturationRange(0, 1);
      this->GetLookupTable()->SetValueRange(1, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from white to green, 256 colours, ramp of warm colours with variation in saturation that's complementary to CoolTint3");
      }
    else if (this->Type == this->CoolTint1)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0.667, 0.5);
      this->GetLookupTable()->SetSaturationRange(0, 1);
      this->GetLookupTable()->SetValueRange(1, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from white to cyan, 256 colours, ramp of cool colours with variations in saturation that's complementary to WarmTint1");
      }
    else if (this->Type == this->CoolTint2)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0.833, 0.667);
      this->GetLookupTable()->SetSaturationRange(0, 1);
      this->GetLookupTable()->SetValueRange(1, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from white to blue, 256 colours, ramp of cool colours with variations in saturation that's complementary to WarmTint2");
      }
    else if (this->Type == this->CoolTint3)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(1.0, 0.833);
      this->GetLookupTable()->SetSaturationRange(0, 1);
      this->GetLookupTable()->SetValueRange(1, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from white to magenta, 256 colours, ramp of cool colours with variations in saturation that's complementary to WarmTint3");
      }
    else if (this->Type == this->Warm1)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0.167, 0.0);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(1, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from yellow to red, of 256 colors, ramp of warm colours that's complementary to Cool1");
      }
    else if (this->Type == this->Warm2)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(.333, 0.167);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(1, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from green to yellow, 256 colours, ramp of warm colours that's complementary to Cool2");
      }
    else if (this->Type == this->Warm3)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0.5, 0.333);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(1, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from cyan to green, 256 colours, ramp of warm colours that's complementary to Cool3");
      }
    else if (this->Type == this->Cool1)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0.667, 0.5);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(1, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from blue to cyan, 256 colours, ramp of cool colours that's complementary to Warm1");
      }
    else if (this->Type == this->Cool2)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0.833, 0.667);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(1, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from magenta to blue, 256 colours, ramp of cool colours that's complementary to Warm2");
      }
    else if (this->Type == this->Cool3)
      {
      // from vtkMRMLSliceLayerLogic.cxx
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(1.0, 0.833);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(1, 1);
      this->GetLookupTable()->SetAlphaRange(1, 1); // not used
      this->GetLookupTable()->Build();
      this->SetNamesFromColors();
      this->SetDescription("A scale from red to magenta, ramp of cool colours that's complementary to Warm3");
      }
    else if (this->Type == this->Iron)
      {
      this->GetLookupTable()->SetNumberOfTableValues(156);
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetHueRange(0, 0.15);
      this->GetLookupTable()->SetSaturationRange(1,1);
      this->GetLookupTable()->SetValueRange(1,1);
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->ForceBuild();
      this->SetNamesFromColors();
      this->SetDescription("A scale from red to  yellow, 157 colours, useful for ");
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
      this->SetDescription("Goes from red to purple, passing through the colors of the rainbow in between. Useful for a colorful display of a label map");
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
      this->SetDescription("A lighter blue scale of 256 values, useful for showing registration results.");
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
      this->SetDescription("Red to yellow/orange scale, 256 colours, useful for ");
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
      this->SetDescription("A white to black scale, 256 colours, useful to highlight negative versions, or to flip intensities of signal values.");
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
      this->SetDescription("A colourful display option, 256 colours going from purple to red");
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
      this->GetLookupTable()->SetTableRange(0,255);
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
      this->SetDescription("A combination of Ocean (0-22) and Desert (23-42), useful for displaying functional MRI volumes (highlights activation)");
      }

    else if (this->Type == this->FMRIPA)
      {
      int size = 20;
      this->GetLookupTable()->SetNumberOfTableValues(size);
      this->GetLookupTable()->SetTableRange(0,255);
      this->GetLookupTable()->SetHueRange(0, 0.16667);
      this->GetLookupTable()->SetSaturationRange(1, 1);
      this->GetLookupTable()->SetValueRange(1, 1);
      this->GetLookupTable()->SetRampToLinear();
      this->GetLookupTable()->ForceBuild();
      this->SetNamesFromColors();
      this->SetDescription("A small fMRI positive activation scale going from red to yellow from 0-19, useful for displaying functional MRI volumes when don't need the blue of the fMRI scale.");
      }

    else if (this->Type == this->Labels)
      {
      // from Slicer2's Colors.xml
      this->GetLookupTable()->SetNumberOfTableValues(257);
      this->GetLookupTable()->SetTableRange(0,255);
      this->Names.clear();
      this->Names.resize(this->GetLookupTable()->GetNumberOfTableValues());

      if (this->SetColorName(0, "Black") != 0)
        {
        this->GetLookupTable()->SetTableValue(0, 0, 0, 0, 0.0);
        }

      if (this->SetColorName(1, "jake") != 0)
        {
        this->GetLookupTable()->SetTableValue(1, 0.2, 0.5, 0.8, 1.0);
        }
      if (this->SetColorName(2, "Peach") != 0)
        {
        this->GetLookupTable()->SetTableValue(2, 1.0, 0.8, 0.7, 1.0);
        }
      if (this->SetColorName(3, "Brain") != 0)
        {
        this->GetLookupTable()->SetTableValue(3, 1.0, 1.0, 1.0, 1.0);
        }
      if (this->SetColorName(4, "Ventricles") != 0)
        {
        this->GetLookupTable()->SetTableValue(4, 0.4, 0.7, 1.0, 1.0);
        }

      if (this->SetColorName(5, "Vessels") != 0)
        {
        this->GetLookupTable()->SetTableValue(5, 0.9, 0.5, 0.5, 1.0);
        }
      if (this->SetColorName(6, "Tumor") != 0)
        {
        this->GetLookupTable()->SetTableValue(6, 0.5, 0.9, 0.5, 1.0);
        }
      if (this->SetColorName(7, "fMRI-high") != 0)
        {
        this->GetLookupTable()->SetTableValue(7, 0.5, 0.9, 0.9, 1.0);
        }
      if (this->SetColorName(8, "fMRI-low") != 0)
        {
        this->GetLookupTable()->SetTableValue(8, 0.9, 0.9, 0.5, 1.0);
        }
      if (this->SetColorName(9, "Pre-Gyrus") != 0)
        {
        this->GetLookupTable()->SetTableValue(9, 0.9, 0.7, 0.9, 1.0);
        }
        if (this->SetColorName(10, "Post-Gyrus") != 0)
        {
        this->GetLookupTable()->SetTableValue(10, 0.9, 0.9, 0.5, 1.0);
        }
      for (int offset = 0; offset <= 240; offset += 10)
        {
        if (this->SetColorName(offset + 11, "jake") != 0)
          {
          this->GetLookupTable()->SetTableValue(offset + 11, 0.2, 0.5, 0.8, 1.0);
          }
        if (this->SetColorName(offset + 12, "elwood") != 0)
          {
          this->GetLookupTable()->SetTableValue(offset + 12, 0.2, 0.8, 0.5, 1.0);
          }
        if (this->SetColorName(offset + 13, "gato") != 0)
          {
          this->GetLookupTable()->SetTableValue(offset + 13, 0.8, 0.5, 0.2, 1.0);
          }
        if (this->SetColorName(offset + 14, "avery") != 0)
          {
          this->GetLookupTable()->SetTableValue(offset + 14, 0.8, 0.2, 0.5, 1.0);
          }
        if (this->SetColorName(offset + 15, "mambazo") != 0)
          {
          this->GetLookupTable()->SetTableValue(offset + 15, 0.5, 0.2, 0.8, 1.0);
          }
        if (this->SetColorName(offset + 16, "domino") != 0)
          {
          this->GetLookupTable()->SetTableValue(offset + 16, 0.5, 0.8, 0.2, 1.0);
          }
        if (offset <= 230)
          {
          // these ones don't go past 256
          if (this->SetColorName(offset + 17, "monk") != 0)
            {
            this->GetLookupTable()->SetTableValue(offset + 17, 0.2, 0.2, 0.8, 1.0);
            }
          if (this->SetColorName(offset + 18, "forest") != 0)
            {
            this->GetLookupTable()->SetTableValue(offset + 18, 0.8, 0.8, 0.2, 1.0);
            }
          if (this->SetColorName(offset + 19, "dylan") != 0)
            {
            this->GetLookupTable()->SetTableValue(offset + 19, 0.2, 0.8, 0.8, 1.0);
            }
          if (this->SetColorName(offset + 20, "kales") != 0)
            {
            this->GetLookupTable()->SetTableValue(offset + 20, 0.5, 0.5, 0.5, 1.0);
            }
          }
        }
      /*
      this->SetColorName(300, "fMRI-neg");
      this->GetLookupTable()->SetTableValue(300, 0.0, 0.8, 1.0, 1.0);

      this->SetColorName(301, "fMRI-pos");
      this->GetLookupTable()->SetTableValue(301, 1.0, 1.0, 0.0, 1.0);
      */
      this->NamesInitialisedOn();
      this->SetDescription("A legacy colour table that contains some anatomical mapping");
      }
    else if (this->Type == this->Random)
      {
      int size = 255;
      std::default_random_engine randomGenerator(std::random_device{}());

      this->GetLookupTable()->SetNumberOfTableValues(size + 1);
      this->GetLookupTable()->SetTableRange(0, 255);
      this->GetLookupTable()->SetTableValue(0, 0, 0, 0, 0);
      for (int i = 1; i <= size; i++)
        {
        // table values have to be 0-1
        double r = static_cast<double>(randomGenerator()) / randomGenerator.max();
        double g = static_cast<double>(randomGenerator()) / randomGenerator.max();
        double b = static_cast<double>(randomGenerator()) / randomGenerator.max();

        this->GetLookupTable()->SetTableValue(i, r, g, b, 1.0);
        }
      this->SetNamesFromColors();
      this->SetDescription("A random selection of 256 rgb colours, useful to distinguish between a small number of labeled regions (especially outside of the brain)");
      }

    else if (this->Type == this->User)
      {
      this->LookupTable->SetNumberOfTableValues(0);
      this->LastAddedColor = -1;
      vtkDebugMacro("Set type to user, call SetNumberOfColors, then AddColor..");
      this->SetDescription("A user defined colour table, use the editor to specify it");
      }

    else if (this->Type == this->File)
      {
      vtkDebugMacro("Set type to file, set up a storage node, set it's FileName and call ReadData on it...");
      this->SetDescription("A color table read in from a text file, each line of the format: IntegerLabel  Name  R  G  B  Alpha");
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
void vtkMRMLColorTableNode::SetNumberOfColors(int n)
{
  if (this->GetLookupTable() == nullptr)
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

  int numberOfTableValues = this->GetLookupTable()->GetNumberOfTableValues();
  if (numberOfTableValues != n)
    {
    this->GetLookupTable()->SetNumberOfTableValues(n);
    for (int indx = numberOfTableValues; indx < n; indx++)
      {
      this->GetLookupTable()->SetTableValue(indx, 0.0, 0.0, 0.0);
      }
    }

  if (this->Names.size() != (unsigned int)n)
    {
    this->Names.resize(n);
    }

  this->Modified();
}

//---------------------------------------------------------------------------
int vtkMRMLColorTableNode::GetNumberOfColors()
{
  if (this->GetLookupTable() != nullptr)
    {
    return this->GetLookupTable()->GetNumberOfTableValues();
    }
  else
    {
    return 0;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLColorTableNode::AddColor(const char *name, double r, double g, double b, double a)
{
 if (this->GetType() != this->User &&
     this->GetType() != this->File)
    {
      vtkErrorMacro("vtkMRMLColorTableNode::AddColor: ERROR: can't add a colour if not a user defined colour table, reset the type first to User or File\n");
      return;
    }
 this->LastAddedColor++;
 this->SetColor(this->LastAddedColor, name, r, g, b, a);
}

//---------------------------------------------------------------------------
int vtkMRMLColorTableNode::SetColor(int entry, const char *name, double r, double g, double b, double a)
{
  if (this->GetType() != this->User &&
      this->GetType() != this->File)
    {
      vtkErrorMacro( "vtkMRMLColorTableNode::SetColor: ERROR: can't set a colour if not a user defined colour table, reset the type first to User or File\n");
      return 0;
    }
  if (entry < 0 ||
      entry >= this->GetLookupTable()->GetNumberOfTableValues())
    {
    vtkErrorMacro( "vtkMRMLColorTableNode::SetColor: requested entry " << entry << " is out of table range: 0 - " << this->GetLookupTable()->GetNumberOfTableValues() << ", call SetNumberOfColors" << endl);
      return 0;
    }

  this->GetLookupTable()->SetTableValue(entry, r, g, b, a);
  if (this->SetColorName(entry, name) == 0)
    {
    vtkWarningMacro("SetColor: error setting color name " << name << " for entry " << entry);
    return 0;
    }

  // trigger a modified event
  this->Modified();
  return 1;
}

//---------------------------------------------------------------------------
int vtkMRMLColorTableNode::SetColor(int entry, double r, double g, double b, double a)
{
  if (this->GetType() != this->User &&
      this->GetType() != this->File)
    {
      vtkErrorMacro( "vtkMRMLColorTableNode::SetColor: ERROR: can't set a colour if not a user defined colour table, reset the type first to User or File\n");
      return 0;
    }
  if (entry < 0 ||
      entry >= this->GetLookupTable()->GetNumberOfTableValues())
    {
    vtkErrorMacro( "vtkMRMLColorTableNode::SetColor: requested entry " << entry << " is out of table range: 0 - " << this->GetLookupTable()->GetNumberOfTableValues() << ", call SetNumberOfColors" << endl);
      return 0;
    }
  double* rgba = this->GetLookupTable()->GetTableValue(entry);
  if (rgba[0] == r && rgba[1] == g && rgba[2] == b && rgba[3] == a)
    {
    return 1;
    }
  this->GetLookupTable()->SetTableValue(entry, r, g, b, a);
  if (this->HasNameFromColor(entry))
    {
    this->SetNameFromColor(entry);
    }

  // trigger a modified event
  this->Modified();
  return 1;
}

//---------------------------------------------------------------------------
int vtkMRMLColorTableNode::SetColor(int entry, double r, double g, double b)
{
  if (entry < 0 ||
      entry >= this->GetLookupTable()->GetNumberOfTableValues())
    {
    vtkErrorMacro( "vtkMRMLColorTableNode::SetColor: requested entry " << entry << " is out of table range: 0 - " << this->GetLookupTable()->GetNumberOfTableValues() << ", call SetNumberOfColors" << endl);
      return 0;
    }
  double* rgba = this->GetLookupTable()->GetTableValue(entry);
  return this->SetColor(entry, r,g,b,rgba[3]);
}

//---------------------------------------------------------------------------
int vtkMRMLColorTableNode::SetOpacity(int entry, double opacity)
{
  if (entry < 0 ||
      entry >= this->GetLookupTable()->GetNumberOfTableValues())
    {
    vtkErrorMacro( "vtkMRMLColorTableNode::SetColor: requested entry " << entry << " is out of table range: 0 - " << this->GetLookupTable()->GetNumberOfTableValues() << ", call SetNumberOfColors" << endl);
      return 0;
    }
  double* rgba = this->GetLookupTable()->GetTableValue(entry);
  return this->SetColor(entry, rgba[0], rgba[1], rgba[2], opacity);
}

//---------------------------------------------------------------------------
bool vtkMRMLColorTableNode::GetColor(int entry, double color[4])
{
  if (entry < 0 || entry >= this->GetNumberOfColors())
    {
    vtkErrorMacro( "vtkMRMLColorTableNode::SetColor: requested entry " << entry << " is out of table range: 0 - " << this->GetLookupTable()->GetNumberOfTableValues() << ", call SetNumberOfColors" << endl);
    return false;
    }
  this->GetLookupTable()->GetTableValue(entry, color);
  return true;
}

//---------------------------------------------------------------------------
void vtkMRMLColorTableNode::ClearNames()
{
  this->Names.clear();
  this->NamesInitialisedOff();
}

//---------------------------------------------------------------------------
void vtkMRMLColorTableNode::Reset(vtkMRMLNode* defaultNode)
{
  int disabledModify = this->StartModify();

  // only call reset if this is a user node
  if (this->GetType() == vtkMRMLColorTableNode::User)
    {
    int type = this->GetType();
    Superclass::Reset(defaultNode);
    this->SetType(type);
    }

  this->EndModify(disabledModify);
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLColorTableNode::CreateDefaultStorageNode()
{
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(scene->CreateNodeByClass("vtkMRMLColorTableStorageNode"));
}

//----------------------------------------------------------------------------
vtkLookupTable* vtkMRMLColorTableNode::GetLookupTable()
{
  return this->LookupTable;
}

//----------------------------------------------------------------------------
void vtkMRMLColorTableNode::SetAndObserveLookupTable(vtkLookupTable *lut)
{
  if (lut == this->LookupTable)
    {
    return;
    }
  vtkSetAndObserveMRMLObjectMacro(this->LookupTable, lut);
  this->Modified();
}
