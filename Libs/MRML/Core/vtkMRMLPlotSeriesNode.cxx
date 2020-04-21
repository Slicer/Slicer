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

#include "vtkMRMLPlotSeriesNode.h"

// MRML includes
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLTableNode.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkAssignAttribute.h>
#include <vtkBrush.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkContextMapper2D.h>
#include <vtkEventBroker.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>
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

const char* vtkMRMLPlotSeriesNode::TableNodeReferenceRole = "table";
const char* vtkMRMLPlotSeriesNode::TableNodeReferenceMRMLAttributeName = "tableNodeRef";

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLPlotSeriesNode);

//----------------------------------------------------------------------------
vtkMRMLPlotSeriesNode::vtkMRMLPlotSeriesNode()
{
  this->HideFromEditors = 0;
  this->Color[0] = 0.0;
  this->Color[1] = 0.0;
  this->Color[2] = 0.0;

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLPlotSeriesNode::TableModifiedEvent);
  this->AddNodeReferenceRole(this->GetTableNodeReferenceRole(),
                             this->GetTableNodeReferenceMRMLAttributeName(),
                             events.GetPointer());
}

//----------------------------------------------------------------------------
vtkMRMLPlotSeriesNode::~vtkMRMLPlotSeriesNode() = default;

//----------------------------------------------------------------------------
const char *vtkMRMLPlotSeriesNode::GetTableNodeReferenceRole()
{
  return vtkMRMLPlotSeriesNode::TableNodeReferenceRole;
}

//----------------------------------------------------------------------------
const char *vtkMRMLPlotSeriesNode::GetTableNodeReferenceMRMLAttributeName()
{
  return vtkMRMLPlotSeriesNode::TableNodeReferenceMRMLAttributeName;
}

//----------------------------------------------------------------------------
void vtkMRMLPlotSeriesNode::WriteXML(ostream& of, int nIndent)
{
  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLEnumMacro(plotType, PlotType);
  vtkMRMLWriteXMLStdStringMacro(xColumnName, XColumnName);
  vtkMRMLWriteXMLStdStringMacro(yColumnName, YColumnName);
  vtkMRMLWriteXMLStdStringMacro(labelColumnName, LabelColumnName);
  vtkMRMLWriteXMLEnumMacro(markerStyle, MarkerStyle);
  vtkMRMLWriteXMLFloatMacro(markerSize, MarkerSize);
  vtkMRMLWriteXMLEnumMacro(lineStyle, LineStyle);
  vtkMRMLWriteXMLFloatMacro(lineWidth, LineWidth);
  vtkMRMLWriteXMLVectorMacro(color, Color, double, 3);
  vtkMRMLWriteXMLFloatMacro(opacity, Opacity);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLPlotSeriesNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLEnumMacro(plotType, PlotType);
  vtkMRMLReadXMLStdStringMacro(xColumnName, XColumnName);
  vtkMRMLReadXMLStdStringMacro(yColumnName, YColumnName);
  vtkMRMLReadXMLStdStringMacro(labelColumnName, LabelColumnName);
  vtkMRMLReadXMLEnumMacro(markerStyle, MarkerStyle);
  vtkMRMLReadXMLFloatMacro(markerSize, MarkerSize);
  vtkMRMLReadXMLEnumMacro(lineStyle, LineStyle);
  vtkMRMLReadXMLFloatMacro(lineWidth, LineWidth);
  vtkMRMLReadXMLVectorMacro(color, Color, double, 3);
  vtkMRMLReadXMLFloatMacro(opacity, Opacity);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLPlotSeriesNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyEnumMacro(PlotType);
  vtkMRMLCopyStdStringMacro(XColumnName);
  vtkMRMLCopyStdStringMacro(YColumnName);
  vtkMRMLCopyStdStringMacro(LabelColumnName);
  vtkMRMLCopyEnumMacro(MarkerStyle);
  vtkMRMLCopyFloatMacro(MarkerSize);
  vtkMRMLCopyEnumMacro(LineStyle);
  vtkMRMLCopyFloatMacro(LineWidth);
  vtkMRMLCopyVectorMacro(Color, double, 3);
  vtkMRMLCopyFloatMacro(Opacity);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLPlotSeriesNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(PlotType);
  vtkMRMLPrintStdStringMacro(XColumnName);
  vtkMRMLPrintStdStringMacro(YColumnName);
  vtkMRMLPrintStdStringMacro(LabelColumnName);
  vtkMRMLPrintEnumMacro(MarkerStyle);
  vtkMRMLPrintFloatMacro(MarkerSize);
  vtkMRMLPrintEnumMacro(LineStyle);
  vtkMRMLPrintFloatMacro(LineWidth);
  vtkMRMLPrintVectorMacro(Color, double, 3);
  vtkMRMLPrintFloatMacro(Opacity);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLPlotSeriesNode::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  if (caller == nullptr ||
      (event != vtkCommand::ModifiedEvent &&
       event != vtkMRMLPlotSeriesNode::TableModifiedEvent))
    {
    return;
    }

  vtkMRMLTableNode *tnode = this->GetTableNode();
  vtkMRMLTableNode *callerTable = vtkMRMLTableNode::SafeDownCast(caller);
  if (callerTable != nullptr && tnode != nullptr && tnode == callerTable &&
      event == vtkCommand::ModifiedEvent)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLPlotSeriesNode::TableModifiedEvent, callerTable);
    }

  return;
}

//----------------------------------------------------------------------------
const char *vtkMRMLPlotSeriesNode::GetTableNodeID()
{
  return this->GetNodeReferenceID(this->GetTableNodeReferenceRole());
}

//----------------------------------------------------------------------------
void vtkMRMLPlotSeriesNode::SetAndObserveTableNodeID(const char *tableNodeID)
{
  // Set and Observe the MRMLTable reference
  this->SetAndObserveNodeReferenceID(this->GetTableNodeReferenceRole(), tableNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLPlotSeriesNode::SetAndObserveTableNodeID(const std::string &tableNodeID)
{
  return this->SetAndObserveTableNodeID(tableNodeID.c_str());
}

//----------------------------------------------------------------------------
vtkMRMLTableNode *vtkMRMLPlotSeriesNode::GetTableNode()
{
  return vtkMRMLTableNode::SafeDownCast(
    this->GetNodeReference(this->GetTableNodeReferenceRole()));
}

//----------------------------------------------------------------------------
void vtkMRMLPlotSeriesNode::SetPlotType(const char *type)
{
  this->SetPlotType(this->GetPlotTypeFromString(type));
}


//-----------------------------------------------------------
const char* vtkMRMLPlotSeriesNode::GetPlotTypeAsString(int id)
{
  switch (id)
    {
    case PlotTypeLine: return "Line";
    case PlotTypeBar: return "Bar";
    case PlotTypeScatter: return "Scatter";
    case PlotTypeScatterBar: return "ScatterBar";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLPlotSeriesNode::GetPlotTypeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < PlotType_Last; ii++)
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
const char* vtkMRMLPlotSeriesNode::GetMarkerStyleAsString(int id)
{
  switch (id)
    {
    case MarkerStyleNone: return "none";
    case MarkerStyleCross: return "cross";
    case MarkerStylePlus: return "plus";
    case MarkerStyleSquare: return "square";
    case MarkerStyleCircle: return "circle";
    case MarkerStyleDiamond: return "diamond";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLPlotSeriesNode::GetMarkerStyleFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < MarkerStyle_Last; ii++)
    {
    if (strcmp(name, GetMarkerStyleAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
const char* vtkMRMLPlotSeriesNode::GetLineStyleAsString(int id)
{
  switch (id)
    {
    case LineStyleNone: return "none";
    case LineStyleSolid: return "solid";
    case LineStyleDash: return "dash";
    case LineStyleDot: return "dot";
    case LineStyleDashDot: return "dash-dot";
    case LineStyleDashDotDot: return "dash-dot-dot";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLPlotSeriesNode::GetLineStyleFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < LineStyle_Last; ii++)
    {
    if (strcmp(name, GetLineStyleAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
bool vtkMRMLPlotSeriesNode::IsXColumnRequired()
{
  return (this->PlotType == PlotTypeScatter || this->PlotType == PlotTypeScatterBar);
}

//-----------------------------------------------------------
void vtkMRMLPlotSeriesNode::SetUniqueColor(const char* colorTableNodeID)
{
  if (this->GetScene() == nullptr)
    {
    vtkGenericWarningMacro("vtkMRMLPlotSeriesNode::GenerateUniqueColor failed: node is not added to scene");
    return;
    }
  if (colorTableNodeID == nullptr)
    {
    colorTableNodeID = "vtkMRMLColorTableNodeRandom";
    }
  vtkMRMLColorTableNode* colorTableNode = vtkMRMLColorTableNode::SafeDownCast(this->GetScene()->GetNodeByID(colorTableNodeID));
  if (colorTableNode == nullptr)
    {
    vtkGenericWarningMacro("vtkMRMLPlotSeriesNode::GenerateUniqueColor failed: color table node by ID "
      << (colorTableNodeID ? colorTableNodeID : "(none)") << " not found in scene");
    return;
    }
  std::vector< vtkMRMLNode* > seriesNodes;
  this->GetScene()->GetNodesByClass("vtkMRMLPlotSeriesNode", seriesNodes);
  int numberOfColors = colorTableNode->GetNumberOfColors();
  if (numberOfColors < 1)
    {
    vtkGenericWarningMacro("vtkMRMLPlotSeriesNode::GenerateUniqueColor failed: color table node "
      << (colorTableNodeID ? colorTableNodeID : "(none)") << " is empty");
    return;
    }
  double color[4] = { 0,0,0,0 };
  bool isColorUnique = false;
  for (int colorIndex = 0; colorIndex < numberOfColors; colorIndex++)
    {
    colorTableNode->GetColor(colorIndex, color);
    isColorUnique = true;
    for (std::vector< vtkMRMLNode* >::iterator seriesNodeIt = seriesNodes.begin(); seriesNodeIt != seriesNodes.end(); ++seriesNodeIt)
      {
      vtkMRMLPlotSeriesNode* seriesNode = vtkMRMLPlotSeriesNode::SafeDownCast(*seriesNodeIt);
      if (!seriesNode)
        {
        continue;
        }
      if (seriesNode == this)
        {
        continue;
        }
      double* foundColor = seriesNode->GetColor();
      if (fabs(foundColor[0] - color[0]) < 0.1
        && fabs(foundColor[1] - color[1]) < 0.1
        && fabs(foundColor[2] - color[2]) < 0.1)
        {
        isColorUnique = false;
        break;
        }
      }
    if (isColorUnique)
      {
      break;
      }
    }
  if (!isColorUnique)
    {
    // Run out of colors, which means that there are more series than entries
    // in the color table. Use sequential indices to have approximately
    // uniform distribution.
    colorTableNode->GetColor(seriesNodes.size() % numberOfColors, color);
    }
  this->SetColor(color);
}
