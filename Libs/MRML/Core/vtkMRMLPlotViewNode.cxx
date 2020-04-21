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
#include "vtkMRMLPlotChartNode.h"
#include "vtkMRMLPlotViewNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"

// VTK includes
#include <vtkAssignAttribute.h>
#include <vtkCommand.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// STD includes
#include <sstream>

const char* vtkMRMLPlotViewNode::PlotChartNodeReferenceRole = "plotChart";
const char* vtkMRMLPlotViewNode::PlotChartNodeReferenceMRMLAttributeName = "plotChartNodeRef";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLPlotViewNode);

//----------------------------------------------------------------------------
vtkMRMLPlotViewNode::vtkMRMLPlotViewNode()
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLPlotViewNode::PlotChartNodeChangedEvent);
  events->InsertNextValue(vtkMRMLPlotChartNode::PlotModifiedEvent);

  this->AddNodeReferenceRole(this->GetPlotChartNodeReferenceRole(),
                             this->GetPlotChartNodeReferenceMRMLAttributeName(),
                             events.GetPointer());
}

//----------------------------------------------------------------------------
vtkMRMLPlotViewNode::~vtkMRMLPlotViewNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLPlotViewNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLIntMacro(doPropagatePlotChartSelection, DoPropagatePlotChartSelection);
  vtkMRMLWriteXMLEnumMacro(interactionMode, InteractionMode);
  vtkMRMLWriteXMLBooleanMacro(enablePointMoveAlongX, EnablePointMoveAlongX);
  vtkMRMLWriteXMLBooleanMacro(enablePointMoveAlongY, EnablePointMoveAlongY);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLPlotViewNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLIntMacro(doPropagatePlotChartSelection, DoPropagatePlotChartSelection);
  vtkMRMLReadXMLEnumMacro(interactionMode, InteractionMode);
  vtkMRMLReadXMLBooleanMacro(enablePointMoveAlongX, EnablePointMoveAlongX);
  vtkMRMLReadXMLBooleanMacro(enablePointMoveAlongY, EnablePointMoveAlongY);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLPlotViewNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyIntMacro(DoPropagatePlotChartSelection);
  vtkMRMLCopyEnumMacro(InteractionMode);
  vtkMRMLCopyBooleanMacro(EnablePointMoveAlongX);
  vtkMRMLCopyBooleanMacro(EnablePointMoveAlongY);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLPlotViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintIntMacro(DoPropagatePlotChartSelection);
  vtkMRMLPrintEnumMacro(InteractionMode);
  vtkMRMLPrintBooleanMacro(EnablePointMoveAlongX);
  vtkMRMLPrintBooleanMacro(EnablePointMoveAlongY);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLPlotViewNode::SetPlotChartNodeID(const char* plotChartNodeId)
{
  this->SetNodeReferenceID(this->GetPlotChartNodeReferenceRole(), plotChartNodeId);
}

//----------------------------------------------------------------------------
const char* vtkMRMLPlotViewNode::GetPlotChartNodeID()
{
  return this->GetNodeReferenceID(this->GetPlotChartNodeReferenceRole());
}

//----------------------------------------------------------------------------
vtkMRMLPlotChartNode* vtkMRMLPlotViewNode::GetPlotChartNode()
{
  return vtkMRMLPlotChartNode::SafeDownCast(this->GetNodeReference(this->GetPlotChartNodeReferenceRole()));
}

//----------------------------------------------------------------------------
void vtkMRMLPlotViewNode::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  vtkMRMLPlotChartNode *pnode = this->GetPlotChartNode();
  if (pnode != nullptr && pnode == vtkMRMLPlotChartNode::SafeDownCast(caller) &&
     (event ==  vtkCommand::ModifiedEvent || event == vtkMRMLPlotChartNode::PlotModifiedEvent))
    {
    this->InvokeEvent(vtkMRMLPlotViewNode::PlotChartNodeChangedEvent, pnode);
    }

  return;
}

//----------------------------------------------------------------------------
const char* vtkMRMLPlotViewNode::GetPlotChartNodeReferenceRole()
{
  return vtkMRMLPlotViewNode::PlotChartNodeReferenceRole;
}

//----------------------------------------------------------------------------
const char* vtkMRMLPlotViewNode::GetPlotChartNodeReferenceMRMLAttributeName()
{
    return vtkMRMLPlotViewNode::PlotChartNodeReferenceMRMLAttributeName;
}

//----------------------------------------------------------------------------
void vtkMRMLPlotViewNode::OnNodeReferenceAdded(vtkMRMLNodeReference *reference)
{
  this->Superclass::OnNodeReferenceAdded(reference);
  if (std::string(reference->GetReferenceRole()) == this->PlotChartNodeReferenceRole)
    {
    this->InvokeEvent(vtkMRMLPlotViewNode::PlotChartNodeChangedEvent, reference->GetReferencedNode());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLPlotViewNode::OnNodeReferenceModified(vtkMRMLNodeReference *reference)
{
  this->Superclass::OnNodeReferenceModified(reference);
  if (std::string(reference->GetReferenceRole()) == this->PlotChartNodeReferenceRole)
    {
    this->InvokeEvent(vtkMRMLPlotViewNode::PlotChartNodeChangedEvent, reference->GetReferencedNode());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLPlotViewNode::OnNodeReferenceRemoved(vtkMRMLNodeReference *reference)
{
  this->Superclass::OnNodeReferenceRemoved(reference);
  if (std::string(reference->GetReferenceRole()) == this->PlotChartNodeReferenceRole)
    {
    this->InvokeEvent(vtkMRMLPlotViewNode::PlotChartNodeChangedEvent, reference->GetReferencedNode());
    }
}

//-----------------------------------------------------------
const char* vtkMRMLPlotViewNode::GetInteractionModeAsString(int id)
{
  switch (id)
  {
  case InteractionModePanView: return "PanView";
  case InteractionModeSelectPoints: return "SelectPoints";
  case InteractionModeFreehandSelectPoints: return "FreehandSelectPoints";
  case InteractionModeMovePoints: return "MovePoints";
  default:
    // invalid id
    return "";
  }
}

//-----------------------------------------------------------
int vtkMRMLPlotViewNode::GetInteractionModeFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int ii = 0; ii < InteractionMode_Last; ii++)
  {
    if (strcmp(name, GetInteractionModeAsString(ii)) == 0)
    {
      // found a matching name
      return ii;
    }
  }
  // unknown name
  return -1;
}
