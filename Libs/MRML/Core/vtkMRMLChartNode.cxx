/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLChartNode.cxx,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.18 $

=========================================================================auto=*/


#include "vtkObjectFactory.h"

#include <sstream>
#include <map>
#include <string>

#include <vtkStringArray.h>

#include "vtkMRMLChartNode.h"
#include "vtkMRMLScene.h"

class DoubleArrayIDMap : public std::map<std::string, std::string> {} ;
class ArrayPropertyMap : public std::map<std::string, std::string> {} ;
class ChartPropertyMap : public std::map<std::string, ArrayPropertyMap> {} ;


//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLChartNode);

//----------------------------------------------------------------------------
vtkMRMLChartNode::vtkMRMLChartNode()
{
  this->HideFromEditors = 0;

  this->DoubleArrayIDs = new DoubleArrayIDMap;
  this->ArrayNames = vtkStringArray::New();
  this->Arrays = vtkStringArray::New();
  this->Properties = new ChartPropertyMap;

  // default properties
  this->SetProperty("default", "showLines", "on");
  this->SetProperty("default", "showMarkers", "off");
  this->SetProperty("default", "showGrid", "on");
  this->SetProperty("default", "showLegend", "on");

  this->SetProperty("default", "showTitle", "on");
  this->SetProperty("default", "showXAxisLabel", "on");
  this->SetProperty("default", "showYAxisLabel", "on");
}


//----------------------------------------------------------------------------
vtkMRMLChartNode::~vtkMRMLChartNode()
{
  // No need to call ClearArrays() and ClearProperties(). These
  // containers will be cleared up automatically.  We apparently do
  // not need to worry about cleaning up the Scene's ReferencedNodeIDs
  // at this time. If you follow the logic in the destructor to
  // MRMLSliceCompositeNode down to the
  // vtkSetReferencedStringBodyMacro, the reference link for the old
  // id is only updated if we are setting a valid new id.

  delete this->DoubleArrayIDs;
  delete this->Properties;

  this->ArrayNames->Delete();
  this->Arrays->Delete();
}


//----------------------------------------------------------------------------
void vtkMRMLChartNode::WriteXML(ostream& of, int nIndent)
{
  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);

  // Write all the IDs
  of << " arrays=\"";
  DoubleArrayIDMap::iterator it;
  for (it = this->DoubleArrayIDs->begin(); it != this->DoubleArrayIDs->end(); ++it)
    {
    if (it != this->DoubleArrayIDs->begin())
      {
      of << " ";
      }
    of << "'" << (*it).first << "':'" << (*it).second << "'";
    }
  of << "\"";

  // Write out the properties
  of << " properties=\"";
  ChartPropertyMap::iterator pit;
  bool first = true;
  for (pit = this->Properties->begin(); pit != this->Properties->end(); ++pit)
    {
    for (ArrayPropertyMap::iterator ait = (*pit).second.begin();
         ait != (*pit).second.end(); ++ait)
      {
      if (!first)
        {
        of << ",";
        }
      first = false;
      of << "'" << (*pit).first << "','" << (*ait).first << "','" << (*ait).second
         << "'";
      }
    }
  of << "\"";
}


//----------------------------------------------------------------------------
void vtkMRMLChartNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "arrays"))
      {
      // format is 'name':'id'
      // Search for 4 single quotes and pull out the pieces.
      std::string text(attValue);
      const std::string::size_type n = text.length();
      std::string::size_type first=0, second, third, fourth;
      first = text.find_first_of("'");
      while (first < n)
       {
        second = text.find_first_of("'", first+1);
        third = text.find_first_of("'", second+1);
        fourth = text.find_first_of("'", third+1);

        this->AddArray(text.substr(first+1, second-first-1).c_str(),
                       text.substr(third+1, fourth-third-1).c_str());

        first = text.find_first_of("'",fourth+1);
        }
      }
    else if (!strcmp(attName, "properties"))
      {
      // format is 'arrayname','propertyname','value'
      // Search for 6 single quotes and pull out the pieces
      std::string text(attValue);
      const std::string::size_type n = text.length();
      std::string::size_type first=0, second, third, fourth, fifth, sixth;
      first = text.find_first_of("'");
      while (first < n)
        {
        second = text.find_first_of("'", first+1);
        third = text.find_first_of("'", second+1);
        fourth = text.find_first_of("'", third+1);
        fifth = text.find_first_of("'", fourth+1);
        sixth = text.find_first_of("'", fifth+1);

        this->SetProperty(text.substr(first+1, second-first-1).c_str(),
                          text.substr(third+1, fourth-third-1).c_str(),
                          text.substr(fifth+1, sixth-fifth-1).c_str());

        first = text.find_first_of("'",sixth+1);
        }

      }
    }

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLChartNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLChartNode *achartnode = vtkMRMLChartNode::SafeDownCast(anode);

  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  // Need to manage references to other nodes.  Unregister this node's
  // current references (done implictly when clearing the arrays and properties).
  this->ClearArrays();
  this->ClearProperties();

  // copy the array list and properties from the other node. Don't
  // bother copying the ivars ArrayNames and Arrays as they are
  // constructed upon request
  if (achartnode)
    {
    *(this->DoubleArrayIDs) = *(achartnode->DoubleArrayIDs);
    *(this->Properties) = *(achartnode->Properties);
    // Add new references
    if (this->Scene)
      {
      this->SetSceneReferences();
      }
    }

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
void vtkMRMLChartNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  return;
}


//----------------------------------------------------------------------------
void vtkMRMLChartNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLChartNode::AddArray(const char *name, const char *id)
{
  if (!name || !id)
    {
    return;
    }

  DoubleArrayIDMap::iterator it = (*this->DoubleArrayIDs).find(name);
  if (it != (*this->DoubleArrayIDs).end())
    {
    if ((*it).second == id)
      {
      return;
      }
    }
  (*this->DoubleArrayIDs)[name] = id;
  this->Modified();
  if (this->Scene)
    {
    this->Scene->AddReferencedNodeID(id, this);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLChartNode::RemoveArray(const char *name)
{
  if (!name)
    {
    return;
    }

  DoubleArrayIDMap::iterator it = (*this->DoubleArrayIDs).find(name);
  if (it == (*this->DoubleArrayIDs).end())
    {
    return;
    }

  this->DoubleArrayIDs->erase(name);
  this->Modified();
  if (this->Scene)
    {
    this->Scene->RemoveReferencedNodeID(name, this);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLChartNode::ClearArrays()
{
  DoubleArrayIDMap::iterator it;
  for (it = this->DoubleArrayIDs->begin(); it != this->DoubleArrayIDs->end(); ++it)
    {
    if (this->Scene)
      {
      this->Scene->RemoveReferencedNodeID((*it).second.c_str(), this);
      }
    }
  this->DoubleArrayIDs->clear();
  this->Modified();
}


//----------------------------------------------------------------------------
const char* vtkMRMLChartNode::GetArray(const char *name)
{
  if (!name)
    {
    return nullptr;
    }

  DoubleArrayIDMap::iterator it = (*this->DoubleArrayIDs).find(name);

  if (it == this->DoubleArrayIDs->end())
    {
    return nullptr;
    }

  return (*it).second.c_str();
}


//----------------------------------------------------------------------------
vtkStringArray* vtkMRMLChartNode::GetArrayNames()
{
  DoubleArrayIDMap::iterator it;

  this->ArrayNames->Initialize();
  for (it = this->DoubleArrayIDs->begin(); it != this->DoubleArrayIDs->end(); ++it)
    {
    this->ArrayNames->InsertNextValue((*it).first);
    }

  return this->ArrayNames;
}


//----------------------------------------------------------------------------
vtkStringArray* vtkMRMLChartNode::GetArrays()
{
  DoubleArrayIDMap::iterator it;

  this->Arrays->Initialize();
  for (it = this->DoubleArrayIDs->begin(); it != this->DoubleArrayIDs->end(); ++it)
    {
    this->Arrays->InsertNextValue((*it).second);
    }

  return this->Arrays;
}



//----------------------------------------------------------------------------
void vtkMRMLChartNode::SetProperty(const char *name,
                                   const char *property,
                                   const char *value)
{
  bool found = true;
  ChartPropertyMap::iterator it;
  ArrayPropertyMap::iterator ait;

  //std::cout << "Setting property " << property << " on " << name << " to " << value << std::endl;

  // check whether this name and property exist
  it = this->Properties->find(name);
  if (it == this->Properties->end())
    {
    found = false;
    }
  else
    {
    ait = (*it).second.find(property);
    if (ait == (*it).second.end())
      {
      found = false;
      }
    }

  // if the name and property exist, check whether we are changing the value
  std::string oldValue;
  if (found)
    {
    if ((*ait).second == value)
      {
      return;
      }
    oldValue = (*ait).second;
    }

  // new name, property or value. set it and mark modified
  //std::cout << "Set the property" << std::endl;
  (*this->Properties)[name][property] = value;
  this->Modified();

  // A ColorNode id can be store as property of the chart or an
  // array. Need to manage the references.
  if (this->Scene && !strcmp(property, "lookupTable"))
    {
    if (found)
      {
      // overwriting the ColorNode ID at this level
      this->Scene->RemoveReferencedNodeID(oldValue.c_str(), this);
      }
    this->Scene->AddReferencedNodeID(value, this);
    }
}

//----------------------------------------------------------------------------
const char *vtkMRMLChartNode::GetProperty(const char *name,
                                          const char *property)
{
  ChartPropertyMap::iterator it = this->Properties->find(name);
  if (it == this->Properties->end())
    {
    return nullptr;
    }

  ArrayPropertyMap::iterator ait = (*it).second.find(property);
  if (ait == (*it).second.end())
    {
    return nullptr;
    }

  return (*ait).second.c_str();
}

//----------------------------------------------------------------------------
void vtkMRMLChartNode::ClearProperty(const char *name,
                                     const char *property)
{
  ChartPropertyMap::iterator it = this->Properties->find(name);
  if (it == this->Properties->end())
    {
    return;
    }

  ArrayPropertyMap::iterator ait = (*it).second.find(property);
  if (ait == (*it).second.end())
    {
    return;
    }

  // erase the property from that array
  std::string oldValue = (*ait).second;
  (*it).second.erase(ait);

  this->Modified();

  // A ColorNode id can be store as property of the chart or an
  // array. Need to manage the references.
  if (this->Scene && !strcmp(name, "lookupTable"))
    {
    this->Scene->RemoveReferencedNodeID(oldValue.c_str(), this);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLChartNode::ClearProperties(const char *name)
{
  ChartPropertyMap::iterator it = this->Properties->find(name);
  if (it == this->Properties->end())
    {
    return;
    }

  // manage any references
  ArrayPropertyMap::iterator ait = (*it).second.find("lookupTable");
  if (ait != (*it).second.end())
    {
    if (this->Scene)
      {
      this->Scene->RemoveReferencedNodeID((*ait).second.c_str(), this);
      }
    }

  // clear the whole property map for this array
  (*it).second.clear();

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLChartNode::ClearProperties()
{
  // manage any references
  ChartPropertyMap::iterator it;
  for (it = this->Properties->begin(); it != this->Properties->end(); ++it)
    {
    ArrayPropertyMap::iterator ait = (*it).second.find("lookupTable");
    if (ait != (*it).second.end())
      {
      if (this->Scene)
        {
        this->Scene->RemoveReferencedNodeID((*ait).second.c_str(), this);
        }
      }
    }

  // clear the entire property map
  this->Properties->clear();

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLChartNode::SetSceneReferences()
{
  this->Superclass::SetSceneReferences();
  // references in the array list
  DoubleArrayIDMap::iterator it;
  for (it = this->DoubleArrayIDs->begin(); it != this->DoubleArrayIDs->end(); ++it)
    {
    this->Scene->AddReferencedNodeID((*it).second.c_str(), this);
    }

    // references in the properties
  ChartPropertyMap::iterator pit;
  for (pit = this->Properties->begin(); pit != this->Properties->end(); ++pit)
    {
    ArrayPropertyMap::iterator ait = (*pit).second.find("lookupTable");
    if (ait != (*pit).second.end())
      {
      this->Scene->AddReferencedNodeID((*ait).second.c_str(), this);
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLChartNode::UpdateReferences()
{
   Superclass::UpdateReferences();

   // Check to see if the referenced nodes are still in the scene
   //
   //

   // arrays
   DoubleArrayIDMap::iterator it;

   // create a separate array for removal of dangling references,
   // cannot remove inside the loop - inavlidates iterators.
   DoubleArrayIDMap doubleArrayIDsRemove;
   for (it = this->DoubleArrayIDs->begin(); it != this->DoubleArrayIDs->end();++it)
     {
     if (this->Scene && !this->Scene->GetNodeByID((*it).second.c_str()))
       {
       doubleArrayIDsRemove[(*it).first.c_str()] = (*it).second.c_str();
       }
     }
   // now remove dangling references
   for (it = doubleArrayIDsRemove.begin(); it != doubleArrayIDsRemove.end();++it)
     {
     this->RemoveArray((*it).first.c_str());
     }

   // properties
   ChartPropertyMap::iterator pit;
   for (pit = this->Properties->begin(); pit != this->Properties->end(); ++pit)
     {
     ArrayPropertyMap::iterator ait = (*pit).second.find("lookupTable");
     if (ait != (*pit).second.end())
       {
       if (this->Scene && !this->Scene->GetNodeByID((*ait).second.c_str()))
         {
         this->ClearProperty((*pit).first.c_str(), "lookupTable");
         }
       }
     }
}

//----------------------------------------------------------------------------
void vtkMRMLChartNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);

  // Check to see if the old id is an array
   DoubleArrayIDMap::iterator it;
   for (it = this->DoubleArrayIDs->begin(); it != this->DoubleArrayIDs->end();++it)
     {
     if (!strcmp((*it).second.c_str(), oldID))
       {
       if (newID)
         {
         //std::cerr << "UpdateReferenceID " << oldID <<" -> "<< newID <<std::endl;
         (*it).second = std::string(newID);
         this->Scene->RemoveReferencedNodeID(oldID, this);
         this->Scene->AddReferencedNodeID(newID, this);
         }
       else
         {
         //std::cerr << "UpdateReferenceID (RemoveArray) " << oldID << " -> " << newID << std::endl;

         this->RemoveArray((*it).first.c_str());
         }
       }
     }

   // Check to see if the old id is a lookupTable
   ChartPropertyMap::iterator pit;
   for (pit = this->Properties->begin(); pit != this->Properties->end(); ++pit)
     {
     ArrayPropertyMap::iterator ait = (*pit).second.find("lookupTable");
     if (ait != (*pit).second.end())
       {
       if (!strcmp((*ait).second.c_str(), oldID))
         {
         if (newID)
           {
           //std::cerr << "UpdateReferenceID "<< oldID <<" -> "<< newID<<std::endl;
           (*ait).second = std::string(newID);
           this->Scene->RemoveReferencedNodeID(oldID, this);
           this->Scene->AddReferencedNodeID(newID, this);
           }
         else
           {
           //std::cerr << "UpdateReferenceID (ClearProperty)" << oldID << " -> " << newID << std::endl;
           this->ClearProperty((*pit).first.c_str(), "lookupTable");
           }
         }
       }
     }
}
