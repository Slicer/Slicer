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
  of << " Type=\"" << this->GetPlotTypeAsString(this->GetType()) << "\"";
  of << " XColumnName=\"" << this->GetXColumnName() << "\"";
  of << " YColumnName=\"" << this->GetYColumnName() << "\"";
  of << " MarkersStyle=\"" << this->GetMarkersStyleAsString(GetMarkerStyle()) << "\"";
  of << " MarkersSize=\"" << this->GetMarkerSize() << "\"";
  of << " LineWidth=\"" << this->GetLineWidth() << "\"";
  double rgba[4];
  this->GetPlotColor(rgba);
  of << " ColorRed=\"" << rgba[0] << "\"";
  of << " ColorGreen=\"" << rgba[1] << "\"";
  of << " ColorBlue=\"" << rgba[2] << "\"";
  of << " ColorAlpha=\"" << rgba[3] << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  double rgba[4];

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "Type"))
      {
      this->SetType(GetPlotTypeFromString(attValue));
      }
    else if (!strcmp(attName, "XColumnName"))
      {
      this->SetXColumnName(attValue);
      }
    else if (!strcmp(attName, "YColumnName"))
      {
      this->SetYColumnName(attValue);
      }
    else if (!strcmp(attName, "MarkersStyle"))
      {
      this->SetMarkerStyle(GetMarkersStyleFromString(attValue));
      }
    else if (!strcmp(attName, "MarkersSize"))
      {
      this->SetMarkerSize(atof(attValue));
      }
    else if (!strcmp(attName, "LineWidth"))
      {
      this->SetLineWidth(atof(attValue));
      }
    else if (!strcmp(attName, "ColorRed"))
      {
      rgba[0] = atof(attValue);
      }
    else if (!strcmp(attName, "ColorGreen"))
      {
      rgba[1] = atof(attValue);
      }
    else if (!strcmp(attName, "ColorBlue"))
      {
      rgba[2] = atof(attValue);
      }
    else if (!strcmp(attName, "ColorAlpha"))
      {
      rgba[3] = atof(attValue);
      }
   }

  this->SetPlotColor(rgba);

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
  this->SetMarkerStyle(node->GetMarkerStyle());
  this->SetMarkerSize(node->GetMarkerSize());
  this->SetLineWidth(node->GetLineWidth());
  unsigned char rgba[4];
  node->GetPlotColor(rgba);
  this->SetPlotColor(rgba);

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
  os << indent << "\nPlot Type: " << this->GetPlotTypeAsString(this->Type);
  os << indent << "\nXColumnName: " << this->XColumnName;
  os << indent << "\nYColumnName: " << this->YColumnName;
  os << indent << "\nMarkersStyle: " <<
        this->GetMarkersStyleAsString(this->GetMarkerStyle());
  os << indent << "\nMarkersSize: " << this->GetMarkerSize();
  os << indent << "\nLineWidth: " << this->GetLineWidth();
  unsigned char rgba[4];
  this->GetPlotColor(rgba);
  os << indent << "\nColorRed: " << rgba[0];
  os << indent << "\nColorGreen: " << rgba[1];
  os << indent << "\nColorBlue: " << rgba[2];
  os << indent << "\nColorAlpha: " << rgba[3];
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
      tableNode->GetNumberOfColumns() < 1 ||
      this->Plot == NULL ||
      !xColumnName.compare("(none)") ||
      !yColumnName.compare("(none)"))
    {
    return;
    }

  vtkTable* table = tableNode->GetTable();
  vtkAbstractArray* xColumn = table->GetColumnByName(xColumnName);
  int XColumnDataType = VTK_VOID;
  if (xColumn != NULL)
    {
    XColumnDataType = xColumn->GetDataType();
    }
  vtkAbstractArray* yColumn = table->GetColumnByName(yColumnName);
  int YColumnDataType = VTK_VOID;
  if (yColumn != NULL)
    {
    YColumnDataType = yColumn->GetDataType();
    }

  if (XColumnDataType == VTK_STRING || XColumnDataType == VTK_BIT ||
      YColumnDataType == VTK_STRING || YColumnDataType == VTK_BIT)
    {
    vtkErrorMacro("vtkMRMLPlotDataNode::SetInputData error : input Columns "
                  "with dataType 'string' and 'bit' are not accepted.")
    return;
    }

  // In the case of Indexes, SetInputData still needs a proper Column.
  if (!xColumnName.compare("Indexes"))
    {
    xColumnName = table->GetColumnName(0);
    }

  this->Plot->SetInputData(table, xColumnName, yColumnName);
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

  this->Modified();

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

  int wasModifyingNode = this->StartModify();

  unsigned char rgba[4] = {0, 0, 0, 255};
  if (this->Plot)
    {
    this->GetPlotColor(rgba);
    }

  switch (type)
  {
  case LINE:
    {
    vtkSmartPointer<vtkPlotLine> line = vtkSmartPointer<vtkPlotLine>::New();
    line->SetWidth(4.0);
    line->SetMarkerSize(10.0);
    line->SetColor(rgba[0], rgba[1], rgba[2], rgba[3]);
    this->SetAndObservePlot(line);
    break;
    }
  case POINTS:
    {
    vtkSmartPointer<vtkPlotPoints> points = vtkSmartPointer<vtkPlotPoints>::New();
    points->SetMarkerSize(10.0);
    points->SetColor(rgba[0], rgba[1], rgba[2], rgba[3]);
    this->SetAndObservePlot(points);
    break;
    }
  case BAR:
    {
    vtkSmartPointer<vtkPlotBar> bar = vtkSmartPointer<vtkPlotBar>::New();
    bar->SetColor(rgba[0], rgba[1], rgba[2], rgba[3]);
    this->SetAndObservePlot(bar);
    break;
    }
  default:
    vtkWarningMacro(<< "vtkMRMLPlotDataNode::SetType : Type not known, "
                       "no vtkPlot has been instantiate.");
    this->Plot = NULL;
    type = -1;
    this->Modified();
  }

  if (!this->XColumnName.compare("Indexes") && this->Plot)
    {
    this->Plot->SetUseIndexForXSeries(true);
    }

  this->Type = type;

  this->EndModify(wasModifyingNode);
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::SetType(const char *type)
{
  this->SetType(this->GetPlotTypeFromString(type));
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::SetXColumnName(vtkStdString xColumnName)
{
  if (this->XColumnName == xColumnName)
    {
    return;
    }

  if (this->Plot)
    {
    if (!xColumnName.compare("Indexes"))
      {
      this->Plot->SetUseIndexForXSeries(true);
      }
    else
      {
      this->Plot->SetUseIndexForXSeries(false);
      }
    }

  this->XColumnName = xColumnName;
  // Set the connection between the vktTable and the vtkPlot
  this->SetInputData(this->GetTableNode());
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLPlotDataNode::SetYColumnName(vtkStdString yColumnName)
{
  if (this->YColumnName == yColumnName)
    {
    return;
    }

  this->YColumnName = yColumnName;
  // Set the connection between the vktTable and the vtkPlot
  this->SetInputData(this->GetTableNode());
  this->Modified();
}

//-----------------------------------------------------------
const char* vtkMRMLPlotDataNode::GetPlotTypeAsString(int id)
{
  switch (id)
    {
    case LINE: return "Line";
    case POINTS: return "Scatter";
    case BAR: return "Bar";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLPlotDataNode::GetPlotTypeFromString(const char* name)
{
  if (name == NULL)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < 3; ii++)
    {
    if (strcmp(name, GetPlotTypeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
void vtkMRMLPlotDataNode::SetMarkerStyle(int style)
{
  vtkPlotPoints* plotPoints = vtkPlotPoints::SafeDownCast(this->Plot);
  if (!plotPoints)
    {
    return;
    }

  plotPoints->SetMarkerStyle(style);
}

//-----------------------------------------------------------
int vtkMRMLPlotDataNode::GetMarkerStyle()
{
  vtkPlotPoints* plotPoints = vtkPlotPoints::SafeDownCast(this->Plot);
  if (!plotPoints)
    {
    return VTK_MARKER_UNKNOWN;
    }

  return plotPoints->GetMarkerStyle();
}

//-----------------------------------------------------------
const char* vtkMRMLPlotDataNode::GetMarkersStyleAsString(int id)
{
  switch (id)
    {
    case VTK_MARKER_NONE: return "None";
    case VTK_MARKER_CROSS: return "Cross";
    case VTK_MARKER_PLUS: return "Plus";
    case VTK_MARKER_SQUARE: return "Square";
    case VTK_MARKER_CIRCLE: return "Circle";
    case VTK_MARKER_DIAMOND: return "Diamond";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLPlotDataNode::GetMarkersStyleFromString(const char* name)
{
  if (name == NULL)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < 6; ii++)
    {
    if (strcmp(name, GetMarkersStyleAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
void vtkMRMLPlotDataNode::SetMarkerSize(float size)
{
  vtkPlotPoints* plotPoints = vtkPlotPoints::SafeDownCast(this->Plot);
  if (!plotPoints)
    {
    return;
    }

  plotPoints->SetMarkerSize(size);
}

//-----------------------------------------------------------
float vtkMRMLPlotDataNode::GetMarkerSize()
{
  vtkPlotPoints* plotPoints = vtkPlotPoints::SafeDownCast(this->Plot);
  if (!plotPoints)
    {
    return 10.;
    }

  return plotPoints->GetMarkerSize();
}

//-----------------------------------------------------------
void vtkMRMLPlotDataNode::SetLineWidth(float width)
{
  vtkPlotLine* plotLine = vtkPlotLine::SafeDownCast(this->Plot);
  if (!plotLine)
    {
    return;
    }

  plotLine->SetWidth(width);
  plotLine->Modified();
}

//-----------------------------------------------------------
float vtkMRMLPlotDataNode::GetLineWidth()
{
  vtkPlotLine* plotLine = vtkPlotLine::SafeDownCast(this->Plot);
  if (!plotLine)
    {
    return 4.;
    }

  return plotLine->GetWidth();
}

//-----------------------------------------------------------
void vtkMRMLPlotDataNode::SetPlotColor(double color[4])
{
  if (!this->Plot)
    {
    return;
    }
  this->Plot->SetColor(color[0], color[1], color[2]);
  if (this->Plot->GetPen())
    {
    this->Plot->GetPen()->SetOpacityF(color[3]);
    }
  this->Modified();
}

//-----------------------------------------------------------
void vtkMRMLPlotDataNode::SetPlotColor(unsigned char color[4])
{
  if (!this->Plot)
    {
    return;
    }
  this->Plot->SetColor(color[0], color[1], color[2], color[3]);
  this->Modified();
}

//-----------------------------------------------------------
void vtkMRMLPlotDataNode::GetPlotColor(double color[4])
{
  if (!this->Plot)
    {
    return;
    }
  double rgb[3];
  this->Plot->GetColor(rgb);
  color[0] = rgb[0];
  color[1] = rgb[1];
  color[2] = rgb[2];
  if (this->Plot->GetPen())
    {
    color[3] = this->Plot->GetPen()->GetOpacity() / 255.0;
    }
}

//-----------------------------------------------------------
void vtkMRMLPlotDataNode::GetPlotColor(unsigned char color[4])
{
  if (!this->Plot)
    {
    return;
    }
  unsigned char rgb[3];
  this->Plot->GetColor(rgb);
  color[0] = rgb[0];
  color[1] = rgb[1];
  color[2] = rgb[2];
  if (this->Plot->GetPen())
    {
    color[3] = this->Plot->GetPen()->GetOpacity();
    }
}
