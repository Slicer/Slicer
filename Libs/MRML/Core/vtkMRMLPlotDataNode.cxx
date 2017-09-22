/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

// MRML includes
#include "vtkMRMLPlotDataNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLTableNode.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkAssignAttribute.h>
#include <vtkBrush.h>
#include <vtkCallbackCommand.h>
#include <vtkColorSeries.h>
#include <vtkCommand.h>
#include <vtkContextMapper2D.h>
#include <vtkEventBroker.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>
#include <vtkPen.h>
#include <vtkPlot.h>
#include <vtkPlotBar.h>
#include <vtkPlotLine.h>
#include <vtkPlotPoints.h>
#include <vtkTable.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// STD includes
#include <algorithm>
#include <sstream>

const char* vtkMRMLPlotDataNode::TableNodeReferenceRole = "table";
const char* vtkMRMLPlotDataNode::TableNodeReferenceMRMLAttributeName = "tableNodeRef";

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLPlotDataNode);

namespace
{
//----------------------------------------------------------------------------
template <typename T> T StringToNumber(const char* num)
{
  std::stringstream ss;
  ss << num;
  T result;
  return ss >> result ? result : 0;
}

//----------------------------------------------------------------------------
int StringToInt(const char* str)
{
  return StringToNumber<int>(str);
}

}// end namespace

//----------------------------------------------------------------------------
vtkMRMLPlotDataNode::vtkMRMLPlotDataNode()
{
  this->Plot = NULL;
  this->Type = -1;
  this->XColumnName = "(none)";
  this->YColumnName = "(none)";
  this->HideFromEditorsOff();

  this->SetType(LINE);

  vtkIntArray  *events = vtkIntArray::New();
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLPlotDataNode::TableModifiedEvent);
  this->AddNodeReferenceRole(this->GetTableNodeReferenceRole(),
                             this->GetTableNodeReferenceMRMLAttributeName(),
                             events);
  events->Delete();
}

//----------------------------------------------------------------------------
vtkMRMLPlotDataNode::~vtkMRMLPlotDataNode()
{
  if (this->Plot)
    {
    this->Plot->Delete();
    this->Plot = NULL;
    }
}

//----------------------------------------------------------------------------
const char *vtkMRMLPlotDataNode::GetTableNodeReferenceRole()
{
  return vtkMRMLPlotDataNode::TableNodeReferenceRole;
}

//----------------------------------------------------------------------------
const char *vtkMRMLPlotDataNode::GetTableNodeReferenceMRMLAttributeName()
{
  return vtkMRMLPlotDataNode::TableNodeReferenceMRMLAttributeName;
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::WriteXML(ostream& of, int nIndent)
{
  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);
  of << " Type=\"" << this->GetType() << "\"";
  of << " XColumnName=\"" << this->GetXColumnName() << "\"";
  of << " YColumnName=\"" << this->GetYColumnName() << "\"";
  of << " ";
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "Type"))
      {
      this->SetType(StringToInt(attValue));
      }
    else if (!strcmp(attName, "XColumnName"))
      {
      this->SetXColumnName(attValue);
      }
    else if (!strcmp(attName, "YColumnName"))
      {
      this->SetYColumnName(attValue);
      }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
//
void vtkMRMLPlotDataNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLPlotDataNode *node = vtkMRMLPlotDataNode::SafeDownCast(anode);
  if (!node)
    {
    vtkErrorMacro("vtkMRMLPlotDataNode::Copy failed: invalid or incompatible source node");
    return;
    }

  this->SetType(node->GetType());
  this->SetXColumnName(node->GetXColumnName());
  this->SetYColumnName(node->GetYColumnName());

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  if (caller == NULL ||
      (event != vtkCommand::ModifiedEvent &&
       event != vtkMRMLPlotDataNode::TableModifiedEvent))
    {
    return;
    }

  vtkPlot* callerPlot = vtkPlot::SafeDownCast(caller);
  if (callerPlot != NULL && this->Plot != NULL && this->Plot == callerPlot)
    {
    // this indicates that data stored in the node is changed (either the Plot or other
    // data members are changed)
    this->Modified();
    return;
    }

  vtkMRMLTableNode *tnode = this->GetTableNode();
  vtkMRMLTableNode *callerTable = vtkMRMLTableNode::SafeDownCast(caller);
  if (callerTable != NULL && tnode != NULL && tnode == callerTable &&
      event == vtkCommand::ModifiedEvent)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLPlotDataNode::TableModifiedEvent, callerTable);
    }

  return;
}

//----------------------------------------------------------------------------
const char *vtkMRMLPlotDataNode::GetTableNodeID()
{
  return this->GetNodeReferenceID(this->GetTableNodeReferenceRole());
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);

  this->SetInputData(this->GetTableNode());
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::OnNodeAddedToScene()
{
  Superclass::OnNodeAddedToScene();

  this->SetInputData(this->GetTableNode());
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);

  this->SetInputData(this->GetTableNode());
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::SetSceneReferences()
{
  Superclass::SetSceneReferences();

  this->SetInputData(this->GetTableNode());
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "\nType: " << this->Type;
  os << indent << "\nXColumnName: " << this->XColumnName;
  os << indent << "\nYColumnName: " << this->YColumnName;
  os << indent << "\nvtkPlot: " <<
    (this->Plot ? this->Plot->GetClassName() : "(none)");
  if (this->Plot)
    {
    this->Plot->PrintSelf(os,indent);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::SetAndObservePlot(vtkPlot* plot)
{
  if (plot == this->Plot)
    {
    return;
    }

  vtkSetAndObserveMRMLObjectMacro(this->Plot, plot);
  // Set the connection between the vktTable and the vtkPlot
  this->SetInputData(this->GetTableNode());
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::SetInputData(vtkMRMLTableNode *tableNode,
                                       vtkStdString xColumnName,
                                       vtkStdString yColumnName)
{
  if (tableNode == NULL ||
      tableNode->GetTable() == NULL ||
      tableNode->GetNumberOfColumns() < 2 ||
      this->GetPlot() == NULL ||
      !xColumnName.compare("(none)") ||
      !yColumnName.compare("(none)"))
    {
    return;
    }

  this->GetPlot()->SetInputData(tableNode->GetTable(), xColumnName, yColumnName);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::SetInputData(vtkMRMLTableNode *tableNode)
{
  this->SetInputData(tableNode, this->GetXColumnName(), this->GetYColumnName());
}

//----------------------------------------------------------------------------
bool vtkMRMLPlotDataNode::SetAndObserveTableNodeID(const char *TableNodeID)
{
  if (!TableNodeID)
    {
    return false;
    }

  // Set and Observe the MRMLTable reference
  this->SetAndObserveNodeReferenceID(this->GetTableNodeReferenceRole(), TableNodeID);

  // Set the connection between the vktTable and the vtkPlot
  this->SetInputData(this->GetTableNode());

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLPlotDataNode::SetAndObserveTableNodeID(const std::string &TableNodeID)
{
  return this->SetAndObserveTableNodeID(TableNodeID.c_str());
}

//----------------------------------------------------------------------------
vtkMRMLTableNode *vtkMRMLPlotDataNode::GetTableNode()
{
  return vtkMRMLTableNode::SafeDownCast(
    this->GetNodeReference(this->GetTableNodeReferenceRole()));
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::SetType(int type)
{
  if (this->Type == type)
    {
    return;
    }

  this->Type = type;

  int wasModifyingNode = this->StartModify();
  switch (this->Type)
  {
  case LINE:
    {
    vtkSmartPointer<vtkPlotLine> line = vtkSmartPointer<vtkPlotLine>::New();
    line->SetWidth(4.0);
    line->GetSelectionPen()->SetColor(137., 0., 13.);
    this->SetAndObservePlot(line);
    break;
    }
  case POINTS:
    {
    vtkSmartPointer<vtkPlotPoints> points = vtkSmartPointer<vtkPlotPoints>::New();
    points->SetMarkerSize(10.0);
    points->GetSelectionPen()->SetColor(137., 0., 13.);
    this->SetAndObservePlot(points);
    break;
    }
  case BAR:
    {
    vtkSmartPointer<vtkPlotBar> bar = vtkSmartPointer<vtkPlotBar>::New();
    bar->GetSelectionPen()->SetColor(137., 0., 13.);
    this->SetAndObservePlot(bar);
    break;
    }
  default:
    vtkWarningMacro(<< "vtkMRMLPlotDataNode::SetType : Type not known, "
                       "no vtkPlot has been instantiate.");
    this->Plot = NULL;
    this->Type = -1;
  }

  this->EndModify(wasModifyingNode);
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::SetXColumnName(vtkStdString xColumnName)
{
  this->XColumnName = xColumnName;
  // Set the connection between the vktTable and the vtkPlot
  this->SetInputData(this->GetTableNode());
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::SetYColumnName(vtkStdString yColumnName)
{
  this->YColumnName = yColumnName;
  // Set the connection between the vktTable and the vtkPlot
  this->SetInputData(this->GetTableNode());
}
