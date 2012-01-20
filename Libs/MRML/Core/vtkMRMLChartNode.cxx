

#include "vtkObjectFactory.h"

#include <sstream>
#include <map>
#include <string>

#include <vtkStringArray.h>

#include "vtkMRMLChartNode.h"

class DoubleArrayIDMap : public std::map<std::string, std::string> {} ;



//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLChartNode);

//----------------------------------------------------------------------------
vtkMRMLChartNode::vtkMRMLChartNode()
{
  this->HideFromEditors = 0;

  this->DoubleArrayIDs = new DoubleArrayIDMap;
  this->ArrayNames = vtkStringArray::New();
  this->Arrays = vtkStringArray::New();
}


//----------------------------------------------------------------------------
vtkMRMLChartNode::~vtkMRMLChartNode()
{
  delete this->DoubleArrayIDs;
  
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
  (*this->DoubleArrayIDs)[name] = id;
}

//----------------------------------------------------------------------------
void vtkMRMLChartNode::RemoveArray(const char *name)
{
  this->DoubleArrayIDs->erase(name);
}

//----------------------------------------------------------------------------
void vtkMRMLChartNode::ClearArrays()
{
  this->DoubleArrayIDs->clear();
}

//----------------------------------------------------------------------------
void vtkMRMLChartNode::SetProperty(const char *vtkNotUsed(name),
                                   const char *vtkNotUsed(property),
                                   const char *vtkNotUsed(value))
{

}

//----------------------------------------------------------------------------
const char* vtkMRMLChartNode::GetArray(const char *name)
{
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


