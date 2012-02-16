
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRMLChartViewNode.h"
#include "vtkMRMLScene.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLChartViewNode);

//vtkCxxSetReferenceStringMacro(vtkMRMLChartViewNode, ChartNodeID);

//----------------------------------------------------------------------------
vtkMRMLChartViewNode::vtkMRMLChartViewNode() : vtkMRMLNode()
{
  this->ChartNodeID = 0;
  this->ViewLabel = new char[2];
  strcpy(this->ViewLabel, "1");
}

//----------------------------------------------------------------------------
vtkMRMLChartViewNode::~vtkMRMLChartViewNode()
{
  if (this->ChartNodeID)
    {
    this->SetChartNodeID(0);
    }
  if ( this->ViewLabel )
    {
    delete [] this->ViewLabel;
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
  if (this->GetViewLabel())
    {
    of << indent << " layoutLabel=\"" << this->GetViewLabel() << "\"";
    }
  if (this->GetLayoutName() != NULL)
    {
    of << indent << " layoutName=\"" << this->GetLayoutName() << "\"";
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
    if (!strcmp(attName, "layoutLabel")) 
      {
      this->SetViewLabel( attValue );
      }
    else if (!strcmp(attName, "layoutName")) 
      {
      this->SetLayoutName( attValue );
      }
    else if (!strcmp(attName, "chart")) 
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

  this->SetViewLabel(achartviewnode->GetViewLabel());
  this->SetChartNodeID(achartviewnode->GetChartNodeID());

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLChartViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

  os << indent << "ViewLabel: " << (this->ViewLabel ? this->ViewLabel : "(null)") << std::endl;
  os << indent << "ChartNodeID: " << 
   (this->ChartNodeID ? this->ChartNodeID : "(none)") << "\n";
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
