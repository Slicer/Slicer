

#include "vtkObjectFactory.h"

#include <sstream>
#include <map>
#include <string>

#include <vtkStringArray.h>

#include "vtkMRMLChartNode.h"

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
}


//----------------------------------------------------------------------------
void vtkMRMLChartNode::ReadXMLAttributes(const char** atts)
{
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read in the IDs
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLChartNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);

  vtkMRMLChartNode *achartnode = vtkMRMLChartNode::SafeDownCast(anode);
  if (achartnode)
    {
    *(this->DoubleArrayIDs) = *(achartnode->DoubleArrayIDs);
    *(this->Properties) = *(achartnode->Properties);
    }
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
}

//----------------------------------------------------------------------------
void vtkMRMLChartNode::ClearArrays()
{
  this->DoubleArrayIDs->clear();
  this->Modified();
}


//----------------------------------------------------------------------------
const char* vtkMRMLChartNode::GetArray(const char *name)
{
  if (!name)
    {
    return 0;
    }

  DoubleArrayIDMap::iterator it = (*this->DoubleArrayIDs).find(name);
  
  if (it == this->DoubleArrayIDs->end())
    {
    return 0;
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
  if (found)
    {
    if ((*ait).second == value)
      {
      return;
      }
    }
  
  // new name, property or value. set it and mark modified
  //std::cout << "Set the property" << std::endl;
  (*this->Properties)[name][property] = value;
  this->Modified();
}

//----------------------------------------------------------------------------
const char *vtkMRMLChartNode::GetProperty(const char *name,
                                          const char *property)
{
  ChartPropertyMap::iterator it = this->Properties->find(name);
  if (it == this->Properties->end())
    {
    return 0;
    }

  ArrayPropertyMap::iterator ait = (*it).second.find(property);
  if (ait == (*it).second.end())
    {
    return 0;
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
  (*it).second.erase(ait);

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLChartNode::ClearProperties(const char *name)
{
  ChartPropertyMap::iterator it = this->Properties->find(name);
  if (it == this->Properties->end())
    {
    return;
    }

  // clear the whole property map for this array
  (*it).second.clear();

  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLChartNode::ClearProperties()
{
  this->Properties->clear();

  this->Modified();
}
