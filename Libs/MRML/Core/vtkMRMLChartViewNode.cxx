
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLChartViewNode.h"
#include "vtkMRMLScene.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLChartViewNode);

//vtkCxxSetReferenceStringMacro(vtkMRMLChartViewNode, ChartNodeID);

//----------------------------------------------------------------------------
vtkMRMLChartViewNode::vtkMRMLChartViewNode() : vtkMRMLViewNode()
{
  this->ChartNodeID = 0;
}

//----------------------------------------------------------------------------
vtkMRMLChartViewNode::~vtkMRMLChartViewNode()
{
  if (this->ChartNodeID)
    {
    this->SetChartNodeID(0);
    }
}

//----------------------------------------------------------------------------
const char* vtkMRMLChartViewNode::GetNodeTagName() 
{
  return "ChartView"; 
}

//----------------------------------------------------------------------------
void vtkMRMLChartViewNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->ChartNodeID)
    {
    of << " chart=\"" << this->ChartNodeID << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLChartViewNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "chart")) 
      {
      this->SetChartNodeID(attValue);
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLChartViewNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLChartViewNode *achartviewnode = vtkMRMLChartViewNode::SafeDownCast(anode);

  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  this->SetChartNodeID(achartviewnode->GetChartNodeID());

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLChartViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLChartViewNode::SetChartNodeID(const char* _arg)
{
  vtkSetReferenceStringBodyMacro(ChartNodeID);

  this->InvokeEvent(vtkMRMLChartViewNode::ChartNodeChangedEvent);
}

//----------------------------------------------------------------------------
void vtkMRMLChartViewNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->ChartNodeID != NULL && this->Scene->GetNodeByID(this->ChartNodeID) == NULL)
    {
    this->SetChartNodeID(NULL);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLChartViewNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);

  if (this->ChartNodeID && !strcmp(oldID, this->ChartNodeID))
    {
    this->SetChartNodeID(newID);
    }
}
