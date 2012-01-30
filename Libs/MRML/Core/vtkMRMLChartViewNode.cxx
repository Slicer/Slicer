
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
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLChartViewNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLChartViewNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
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
