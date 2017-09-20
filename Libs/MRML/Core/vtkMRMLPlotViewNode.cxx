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
: DoPropagatePlotChartSelection(true)
{
  vtkIntArray  *events = vtkIntArray::New();
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLPlotViewNode::PlotChartNodeChangedEvent);
  events->InsertNextValue(vtkMRMLPlotChartNode::PlotModifiedEvent);

  this->AddNodeReferenceRole(this->GetPlotChartNodeReferenceRole(),
                             this->GetPlotChartNodeReferenceMRMLAttributeName(),
                             events);
  events->Delete();
}

//----------------------------------------------------------------------------
vtkMRMLPlotViewNode::~vtkMRMLPlotViewNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLPlotViewNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  of << " doPropagatePlotChartSelection=\"" << (int)this->DoPropagatePlotChartSelection << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLPlotViewNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if(!strcmp(attName, "doPropagatePlotChartSelection"))
      {
      this->SetDoPropagatePlotChartSelection(atoi(attValue)?true:false);
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLPlotViewNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLPlotViewNode *aPlotviewnode = vtkMRMLPlotViewNode::SafeDownCast(anode);

  int disabledModify = this->StartModify();

  this->Superclass::Copy(anode);

  this->SetDoPropagatePlotChartSelection(aPlotviewnode->GetDoPropagatePlotChartSelection());

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLPlotViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "DoPropagatePlotChartSelection: " << this->DoPropagatePlotChartSelection << "\n";
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
  if (pnode != NULL && pnode == vtkMRMLPlotChartNode::SafeDownCast(caller) &&
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
