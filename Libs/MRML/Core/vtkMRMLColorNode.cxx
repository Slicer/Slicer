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
#include <vtkCodedEntry.h>
#include "vtkMRMLColorNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"

// VTK includes
#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <cassert>
#include <sstream>
#include <algorithm>

//----------------------------------------------------------------------------
vtkMRMLColorNode::vtkMRMLColorNode()
{
  this->Type = -1;
  this->HideFromEditors = 1;
}

//----------------------------------------------------------------------------
vtkMRMLColorNode::~vtkMRMLColorNode() {}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  of << " type=\"" << this->GetType() << "\"";
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
vtkLookupTable* vtkMRMLColorNode::GetLookupTable()
{
  vtkDebugMacro("Subclass has not implemented GetLookupTable, returning NULL");
  return nullptr;
}

//----------------------------------------------------------------------------
vtkScalarsToColors* vtkMRMLColorNode::GetScalarsToColors()
{
  return this->GetLookupTable();
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::CopyContent(vtkMRMLNode* anode, bool deepCopy /*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLColorNode* node = vtkMRMLColorNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  if (node->Type != -1)
  {
    // not using SetType, as that will basically recreate a new color node, very slow
    this->Type = node->Type;
  }

  if (deepCopy)
  {
    // Deep copy
    int size = node->Properties.size();
    this->Properties.resize(size);
    for (int index = 0; index < size; index++)
    {
      this->Properties[index].DeepCopy(node->Properties[index]);
    }
  }
  else
  {
    // Shallow copy
    this->Properties = node->Properties;
  }
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Name: " << (this->Name ? this->Name : "(none)") << "\n";
  os << indent << "Type: (" << this->GetTypeAsString() << ")\n";

  if (this->Properties.size() > 0)
  {
    os << indent << "Color properties:\n";
    for (unsigned int i = 0; i < this->Properties.size(); i++)
    {
      PropertyType& prop = this->Properties[i];
      if (!prop.Defined)
      {
        continue;
      }
      os << indent << indent << i << " ";
      os << this->GetColorName(i);
      if (i < static_cast<unsigned int>(this->GetNumberOfColors()))
      {
        double color[4];
        this->GetColor(i, color);
        os << " (" << color[0] << ", " << color[1] << ", " << color[2] << ", " << color[3] << ")";
      }
      else
      {
        os << " (undefined)";
      }
      if (prop.Category)
      {
        os << " Category: " << prop.Category->GetAsPrintableString();
      }
      if (prop.Type)
      {
        os << " Type: " << prop.Type->GetAsPrintableString();
      }
      if (prop.TypeModifier)
      {
        os << " Type modifier: " << prop.TypeModifier->GetAsPrintableString();
      }
      if (prop.Region)
      {
        os << " Region: " << prop.Region->GetAsPrintableString();
      }
      if (prop.RegionModifier)
      {
        os << " Region modifier: " << prop.RegionModifier->GetAsPrintableString();
      }
      os << std::endl;
    }
  }
}

//-----------------------------------------------------------
void vtkMRMLColorNode::UpdateScene(vtkMRMLScene* scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void vtkMRMLColorNode::ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData)
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//---------------------------------------------------------------------------
const char* vtkMRMLColorNode::GetTypeAsString()
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

  // subclass should override this and define colors according to the node type

  this->StorableModified();

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
  this->Properties.resize(numPoints);

  for (int i = 0; i < numPoints; ++i)
  {
    this->SetNameFromColor(i);
  }
}

//---------------------------------------------------------------------------
bool vtkMRMLColorNode::SetNameFromColor(int index)
{
  double rgba[4] = { -1., -1., -1., -1. };
  bool res = this->GetColor(index, rgba);
  std::stringstream ss;
  ss.precision(3);
  ss.setf(std::ios::fixed, std::ios::floatfield);
  ss << "R=" << rgba[0] << " G=" << rgba[1] << " B=" << rgba[2] << " A=" << rgba[3];
  vtkDebugMacro("SetNamesFromColors: " << index << " Name = " << ss.str().c_str());
  if (!this->SetColorName(index, ss.str().c_str()))
  {
    vtkErrorMacro("SetNamesFromColors: Error setting color name " << index << " Name = " << ss.str().c_str());
    return false;
  }

  return res;
}

//---------------------------------------------------------------------------
bool vtkMRMLColorNode::GetProperty(int ind, vtkMRMLColorNode::PropertyType& prop)
{
  if (ind < 0 || ind >= (int)this->Properties.size())
  {
    return false;
  }
  prop = this->Properties[ind];
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLColorNode::GetColorDefined(int index)
{
  if (index < 0 || index >= (int)this->Properties.size())
  {
    return false;
  }
  if (!this->Properties[index].Defined)
  {
    return false;
  }
  return true;
}

//---------------------------------------------------------------------------
void vtkMRMLColorNode::SetColorDefined(int ind, bool defined)
{
  if (ind < 0 || ind >= (int)this->Properties.size())
  {
    vtkErrorMacro("SetColorDefined failed: invalid index " << ind);
    return;
  }
  if (this->Properties[ind].Defined == defined)
  {
    return;
  }
  this->Properties[ind].Defined = defined;
  this->StorableModified();
  this->Modified();
}

//---------------------------------------------------------------------------
const char* vtkMRMLColorNode::GetColorName(int index)
{
  // Do not use GetProperty because the content of the locally copied property's std::string does not survive the return
  if (index < 0 || index >= (int)this->Properties.size())
  {
    return "";
  }
  if (!this->Properties[index].Defined)
  {
    return "";
  }
  return this->Properties[index].Name.c_str();
}

//---------------------------------------------------------------------------
vtkCodedEntry* vtkMRMLColorNode::GetTerminologyCategory(int ind)
{
  PropertyType prop;
  if (!this->GetProperty(ind, prop))
  {
    return nullptr;
  }
  return prop.Category;
}

//---------------------------------------------------------------------------
vtkCodedEntry* vtkMRMLColorNode::GetTerminologyType(int ind)
{
  PropertyType prop;
  if (!this->GetProperty(ind, prop))
  {
    return nullptr;
  }
  return prop.Type;
}

//---------------------------------------------------------------------------
vtkCodedEntry* vtkMRMLColorNode::GetTerminologyTypeModifier(int ind)
{
  PropertyType prop;
  if (!this->GetProperty(ind, prop))
  {
    return nullptr;
  }
  return prop.TypeModifier;
}

//---------------------------------------------------------------------------
vtkCodedEntry* vtkMRMLColorNode::GetTerminologyRegion(int ind)
{
  PropertyType prop;
  if (!this->GetProperty(ind, prop))
  {
    return nullptr;
  }
  return prop.Region;
}

//---------------------------------------------------------------------------
vtkCodedEntry* vtkMRMLColorNode::GetTerminologyRegionModifier(int ind)
{
  PropertyType prop;
  if (!this->GetProperty(ind, prop))
  {
    return nullptr;
  }
  return prop.RegionModifier;
}

//---------------------------------------------------------------------------
std::string vtkMRMLColorNode::GetTerminologyAsString(int ind)
{
  PropertyType prop;
  if (!this->GetProperty(ind, prop))
  {
    return std::string();
  }

  return vtkMRMLColorNode::GetTerminologyAsString(
    "", prop.Category, prop.Type, prop.TypeModifier, "", prop.Region, prop.RegionModifier);
}

//---------------------------------------------------------------------------
std::string vtkMRMLColorNode::GetTerminologyAsString(std::string terminologyContextName,
                                                     vtkCodedEntry* category,
                                                     vtkCodedEntry* type,
                                                     vtkCodedEntry* typeModifier,
                                                     std::string regionContextName,
                                                     vtkCodedEntry* region /*=nullptr*/,
                                                     vtkCodedEntry* regionModifier /*=nullptr*/)
{
  std::string serializedEntry("");
  serializedEntry += terminologyContextName + "~";
  if (category)
  {
    serializedEntry += (category->GetCodingSchemeDesignator() ? std::string(category->GetCodingSchemeDesignator()) : "")
                       + "^" + (category->GetCodeValue() ? std::string(category->GetCodeValue()) : "") + "^"
                       + (category->GetCodeMeaning() ? std::string(category->GetCodeMeaning()) : "") + "~";
  }
  else
  {
    serializedEntry += "^^~";
  }
  if (type)
  {
    serializedEntry += (type->GetCodingSchemeDesignator() ? std::string(type->GetCodingSchemeDesignator()) : "") + "^"
                       + (type->GetCodeValue() ? std::string(type->GetCodeValue()) : "") + "^"
                       + (type->GetCodeMeaning() ? std::string(type->GetCodeMeaning()) : "") + "~";
  }
  else
  {
    serializedEntry += "^^~";
  }
  if (typeModifier)
  {
    serializedEntry +=
      (typeModifier->GetCodingSchemeDesignator() ? std::string(typeModifier->GetCodingSchemeDesignator()) : "") + "^"
      + (typeModifier->GetCodeValue() ? std::string(typeModifier->GetCodeValue()) : "") + "^"
      + (typeModifier->GetCodeMeaning() ? std::string(typeModifier->GetCodeMeaning()) : "") + "~";
  }
  else
  {
    serializedEntry += "^^~";
  }

  serializedEntry += regionContextName + "~";
  if (region)
  {
    serializedEntry += (region->GetCodingSchemeDesignator() ? std::string(region->GetCodingSchemeDesignator()) : "")
                       + "^" + (region->GetCodeValue() ? std::string(region->GetCodeValue()) : "") + "^"
                       + (region->GetCodeMeaning() ? std::string(region->GetCodeMeaning()) : "") + "~";
  }
  else
  {
    serializedEntry += "^^~";
  }
  if (regionModifier)
  {
    serializedEntry +=
      (regionModifier->GetCodingSchemeDesignator() ? std::string(regionModifier->GetCodingSchemeDesignator()) : "")
      + "^" + (regionModifier->GetCodeValue() ? std::string(regionModifier->GetCodeValue()) : "") + "^"
      + (regionModifier->GetCodeMeaning() ? std::string(regionModifier->GetCodeMeaning()) : "") + "~";
  }
  else
  {
    serializedEntry += "^^~";
  }

  return serializedEntry;
}

//---------------------------------------------------------------------------
bool vtkMRMLColorNode::SetTerminologyFromString(int ind, std::string terminologyString)
{
  if (ind < 0 || ind >= (int)this->Properties.size())
  {
    vtkDebugMacro("vtkMRMLColorNode::GetProperty: index " << ind << " is out of range 0 - " << this->Properties.size());
    return false;
  }
  PropertyType& prop = this->Properties[ind];

  // Serialized terminology entry consists of the following: terminologyContextName, category (codingScheme,
  // codeValue, codeMeaning triple), type, typeModifier, regionContextName, region, regionModifier
  std::vector<std::string> entryComponents;
  vtksys::SystemTools::Split(terminologyString, entryComponents, '~');
  if (entryComponents.size() != 7)
  {
    return false;
  }
  if (!entryComponents[1].compare("^^"))
  {
    return false; // Empty category (none selection)
  }

  // Category
  std::vector<std::string> categoryIds = vtksys::SystemTools::SplitString(entryComponents[1], '^');
  if (categoryIds.size() != 3)
  {
    vtkErrorMacro("DeserializeTerminologyEntry: Invalid category component");
    return false;
  }
  if (prop.Category == nullptr)
  {
    prop.Category = vtkSmartPointer<vtkCodedEntry>::New();
  }
  prop.Category->SetCodeMeaning(categoryIds[2].c_str());
  prop.Category->SetCodeValue(categoryIds[1].c_str());
  prop.Category->SetCodingSchemeDesignator(categoryIds[0].c_str());

  // Type
  std::vector<std::string> typeIds = vtksys::SystemTools::SplitString(entryComponents[2], '^');

  if (typeIds.size() != 3)
  {
    vtkErrorMacro("DeserializeTerminologyEntry: Invalid type component");
    return false;
  }
  if (prop.Type == nullptr)
  {
    prop.Type = vtkSmartPointer<vtkCodedEntry>::New();
  }
  prop.Type->SetCodeMeaning(typeIds[2].c_str());
  prop.Type->SetCodeValue(typeIds[1].c_str());
  prop.Type->SetCodingSchemeDesignator(typeIds[0].c_str());

  // Type modifier (optional)
  std::vector<std::string> typeModifierIds = vtksys::SystemTools::SplitString(entryComponents[3], '^');
  if (typeModifierIds.size() == 3)
  {
    if (prop.TypeModifier == nullptr)
    {
      prop.TypeModifier = vtkSmartPointer<vtkCodedEntry>::New();
    }
    prop.TypeModifier->SetCodeMeaning(typeModifierIds[2].c_str());
    prop.TypeModifier->SetCodeValue(typeModifierIds[1].c_str());
    prop.TypeModifier->SetCodingSchemeDesignator(typeModifierIds[0].c_str());
  }

  // Region (optional)
  std::vector<std::string> regionIds = vtksys::SystemTools::SplitString(entryComponents[5], '^');
  if (regionIds.size() == 3)
  {
    if (prop.Region == nullptr)
    {
      prop.Region = vtkSmartPointer<vtkCodedEntry>::New();
    }
    prop.Region->SetCodeMeaning(regionIds[2].c_str());
    prop.Region->SetCodeValue(regionIds[1].c_str());
    prop.Region->SetCodingSchemeDesignator(regionIds[0].c_str());

    // Region modifier (optional)
    std::vector<std::string> regionModifierIds = vtksys::SystemTools::SplitString(entryComponents[6], '^');
    if (regionModifierIds.size() == 3)
    {
      if (prop.RegionModifier == nullptr)
      {
        prop.RegionModifier = vtkSmartPointer<vtkCodedEntry>::New();
      }
      prop.RegionModifier->SetCodeMeaning(regionModifierIds[2].c_str());
      prop.RegionModifier->SetCodeValue(regionModifierIds[1].c_str());
      prop.RegionModifier->SetCodingSchemeDesignator(regionModifierIds[0].c_str());
    }
  }

  if (this->Properties[ind] != prop)
  {
    this->Properties[ind] = prop;
    this->StorableModified();
    this->Modified();
  }

  // Set attribute indicating that the color table contains terminology
  this->SetContainsTerminology(true);

  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLColorNode::SetTerminology(int ind,
                                      std::string categoryCodingScheme,
                                      std::string categoryCodeValue,
                                      std::string categoryCodeMeaning,
                                      std::string typeCodingScheme,
                                      std::string typeCodeValue,
                                      std::string typeCodeMeaning,
                                      std::string typeModifierCodingScheme /*=""*/,
                                      std::string typeModifierCodeValue /*=""*/,
                                      std::string typeModifierCodeMeaning /*=""*/,
                                      std::string regionCodingScheme /*=""*/,
                                      std::string regionCodeValue /*=""*/,
                                      std::string regionCodeMeaning /*=""*/,
                                      std::string regionModifierCodingScheme /*=""*/,
                                      std::string regionModifierCodeValue /*=""*/,
                                      std::string regionModifierCodeMeaning /*=""*/)
{
  vtkNew<vtkCodedEntry> category;
  if (!categoryCodeValue.empty())
  {
    category->SetValueSchemeMeaning(
      categoryCodeValue.c_str(), categoryCodingScheme.c_str(), categoryCodeMeaning.c_str());
  }
  vtkNew<vtkCodedEntry> type;
  if (!typeCodeValue.empty())
  {
    type->SetValueSchemeMeaning(typeCodeValue.c_str(), typeCodingScheme.c_str(), typeCodeMeaning.c_str());
  }
  vtkNew<vtkCodedEntry> typeModifier;
  if (!typeModifierCodeValue.empty())
  {
    typeModifier->SetValueSchemeMeaning(
      typeModifierCodeValue.c_str(), typeModifierCodingScheme.c_str(), typeModifierCodeMeaning.c_str());
  }
  vtkNew<vtkCodedEntry> region;
  if (!regionCodeValue.empty())
  {
    region->SetValueSchemeMeaning(regionCodeValue.c_str(), regionCodingScheme.c_str(), regionCodeMeaning.c_str());
  }
  vtkNew<vtkCodedEntry> regionModifier;
  if (!regionModifierCodeValue.empty())
  {
    regionModifier->SetValueSchemeMeaning(
      regionModifierCodeValue.c_str(), regionModifierCodingScheme.c_str(), regionModifierCodeMeaning.c_str());
  }

  std::string terminologyString =
    vtkMRMLColorNode::GetTerminologyAsString("", category, type, typeModifier, "", region, regionModifier);
  return this->SetTerminologyFromString(ind, terminologyString);
}

//---------------------------------------------------------------------------
int vtkMRMLColorNode::GetColorIndexByName(const char* name)
{
  if (name == nullptr)
  {
    vtkErrorMacro("GetColorIndexByName: need a non-null name as argument");
    return -1;
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
std::string vtkMRMLColorNode::GetColorNameWithoutSpaces(int ind, const char* subst)
{
  std::string name = std::string(this->GetColorName(ind));
  if (strstr(name.c_str(), " ") != nullptr)
  {
    std::string::size_type spaceIndex = name.find(" ", 0);
    while (spaceIndex != std::string::npos)
    {
      name.replace(spaceIndex, 1, subst, 0, strlen(subst));
      spaceIndex = name.find(" ", spaceIndex);
    }
  }

  return name;
}

//---------------------------------------------------------------------------
std::string vtkMRMLColorNode::GetColorNameAsFileName(int colorIndex, const char* subst)
{
  std::string fileName(this->GetColorName(colorIndex));
  std::string validCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabscdefghijklmnopqrstuvwxyz"
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
int vtkMRMLColorNode::SetColorName(int ind, const char* name)
{
  if (ind >= static_cast<int>(this->Properties.size()) || ind < 0)
  {
    vtkErrorMacro("SetColorName: Index was out of bounds: " << ind << ", current size is " << this->Properties.size()
                                                            << ", table name = "
                                                            << (this->GetName() == nullptr ? "null" : this->GetName()));
    return 0;
  }
  PropertyType& prop = this->Properties[ind];
  if (prop.Name != name || !prop.Defined)
  {
    prop.Name = name;
    prop.Defined = true;
    this->StorableModified();
    this->Modified();
  }
  return 1;
}

//---------------------------------------------------------------------------
int vtkMRMLColorNode::SetColorNameWithSpaces(int ind, const char* name, const char* subst)
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
void vtkMRMLColorNode::Reset(vtkMRMLNode* vtkNotUsed(defaultNode))
{
  // don't need to call reset on color nodes, as all but the User color table
  // node are static, and that's taken care of in the vtkMRMLColorTableNode
  // Superclass::Reset(defaultNode);
}

//---------------------------------------------------------------------------
bool vtkMRMLColorNode::GetModifiedSinceRead()
{
  return this->Superclass::GetModifiedSinceRead()
         || (this->GetScalarsToColors() && this->GetScalarsToColors()->GetMTime() > this->GetStoredTime());
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

//---------------------------------------------------------------------------
bool vtkMRMLColorNode::GetContainsTerminology()
{
  return (this->GetAttribute(this->GetContainsTerminologyAttributeName()) != nullptr);
}

//---------------------------------------------------------------------------
void vtkMRMLColorNode::SetContainsTerminology(bool containsTerminology)
{
  if (containsTerminology)
  {
    this->SetAttribute(this->GetContainsTerminologyAttributeName(), "true");
  }
  else
  {
    this->RemoveAttribute(this->GetContainsTerminologyAttributeName());
  }
}

//----------------------------------------------------------------------------
void vtkMRMLColorNode::SetAllColorsDefined()
{
  unsigned int numberOfColors = static_cast<unsigned int>(this->GetNumberOfColors());
  bool modified = false;
  if (this->Properties.size() < static_cast<size_t>(numberOfColors))
  {
    this->Properties.resize(numberOfColors);
    modified = true;
  }
  for (unsigned int index = 0; index < numberOfColors; index++)
  {
    if (this->Properties[index].Defined == true)
    {
      // no change
      continue;
    }
    this->Properties[index].Defined = true;
    modified = true;
  }
  if (modified)
  {
    this->StorableModified();
    this->Modified();
  }
}
