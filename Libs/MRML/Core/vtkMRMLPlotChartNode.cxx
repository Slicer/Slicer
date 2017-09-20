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

#include <sstream>
#include <map>
#include <string>

// VTK includes
#include <vtkCollection.h>
#include <vtkCommand.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlot.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTable.h>

// MRML includes
#include "vtkMRMLPlotChartNode.h"
#include "vtkMRMLPlotDataNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTableNode.h"

const char* vtkMRMLPlotChartNode::PlotDataNodeReferenceRole = "plotData";
const char* vtkMRMLPlotChartNode::PlotDataNodeReferenceMRMLAttributeName = "plotDataNodeRef";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLPlotChartNode);

//----------------------------------------------------------------------------
vtkMRMLPlotChartNode::vtkMRMLPlotChartNode()
{
  this->HideFromEditors = 0;

  // default properties
  this->SetAttribute("Type", "Line");

  this->SetAttribute("ShowGrid", "on");
  this->SetAttribute("ShowLegend", "on");

  this->SetAttribute("ShowTitle", "on");
  this->SetAttribute("ShowXAxisLabel", "on");
  this->SetAttribute("ShowYAxisLabel", "on");

  this->SetAttribute("TitleName", "");
  this->SetAttribute("XAxisLabelName", "");
  this->SetAttribute("YAxisLabelName", "");

  this->SetAttribute("ClickAndDragAlongX", "on");
  this->SetAttribute("ClickAndDragAlongY", "on");

  this->SetAttribute("FontType", "Arial");
  this->SetAttribute("TitleFontSize", "20");
  this->SetAttribute("AxisTitleFontSize", "16");
  this->SetAttribute("AxisLabelFontSize", "12");

  this->SetAttribute("LookupTable", "");

  vtkIntArray  *events = vtkIntArray::New();
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLPlotChartNode::PlotModifiedEvent);
  events->InsertNextValue(vtkMRMLPlotDataNode::TableModifiedEvent);

  this->AddNodeReferenceRole(this->GetPlotDataNodeReferenceRole(),
                             this->GetPlotDataNodeReferenceMRMLAttributeName(),
                             events);
  events->Delete();
}


//----------------------------------------------------------------------------
vtkMRMLPlotChartNode::~vtkMRMLPlotChartNode()
{
}

//----------------------------------------------------------------------------
const char* vtkMRMLPlotChartNode::GetPlotDataNodeReferenceRole()
{
  return vtkMRMLPlotChartNode::PlotDataNodeReferenceRole;
}

//----------------------------------------------------------------------------
const char* vtkMRMLPlotChartNode::GetPlotDataNodeReferenceMRMLAttributeName()
{
  return vtkMRMLPlotChartNode::PlotDataNodeReferenceMRMLAttributeName;
}

//----------------------------------------------------------------------------
void vtkMRMLPlotChartNode::OnNodeReferenceAdded(vtkMRMLNodeReference *reference)
{
  this->Superclass::OnNodeReferenceAdded(reference);
  if (std::string(reference->GetReferenceRole()) == this->PlotDataNodeReferenceRole)
    {
    this->InvokeEvent(vtkMRMLPlotChartNode::PlotModifiedEvent, reference->GetReferencedNode());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLPlotChartNode::OnNodeReferenceModified(vtkMRMLNodeReference *reference)
{
  this->Superclass::OnNodeReferenceModified(reference);
  if (std::string(reference->GetReferenceRole()) == this->PlotDataNodeReferenceRole)
    {
    this->InvokeEvent(vtkMRMLPlotChartNode::PlotModifiedEvent, reference->GetReferencedNode());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLPlotChartNode::OnNodeReferenceRemoved(vtkMRMLNodeReference *reference)
{
  this->Superclass::OnNodeReferenceRemoved(reference);
  if (std::string(reference->GetReferenceRole()) == this->PlotDataNodeReferenceRole)
    {
    this->InvokeEvent(vtkMRMLPlotChartNode::PlotModifiedEvent, reference->GetReferencedNode());
    }
}

//----------------------------------------------------------------------------
void vtkMRMLPlotChartNode::WriteXML(ostream& of, int nIndent)
{
  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLPlotChartNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  vtkMRMLNode::ReadXMLAttributes(atts);

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLPlotChartNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLPlotChartNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLPlotChartNode::SetAndObservePlotDataNodeID(const char *plotDataNodeID)
{
  this->SetAndObserveNodeReferenceID(this->GetPlotDataNodeReferenceRole(), plotDataNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLPlotChartNode::AddAndObservePlotDataNodeID(const char *plotDataNodeID)
{
  this->AddAndObserveNodeReferenceID(this->GetPlotDataNodeReferenceRole(), plotDataNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLPlotChartNode::RemovePlotDataNodeID(const char *plotDataNodeID)
{
  if (!plotDataNodeID)
    {
    return;
    }

  this->RemoveNthPlotDataNodeID(this->GetNthPlotIdexFromID(plotDataNodeID));
}

//----------------------------------------------------------------------------
void vtkMRMLPlotChartNode::RemoveNthPlotDataNodeID(int n)
{
  this->RemoveNthNodeReferenceID(this->GetPlotDataNodeReferenceRole(), n);
}

//----------------------------------------------------------------------------
void vtkMRMLPlotChartNode::RemoveAllPlotDataNodeIDs()
{
  this->RemoveNodeReferenceIDs(this->GetPlotDataNodeReferenceRole());
}

//----------------------------------------------------------------------------
void vtkMRMLPlotChartNode::SetAndObserveNthPlotDataNodeID(int n, const char *plotDataNodeID)
{
  this->SetAndObserveNthNodeReferenceID(this->GetPlotDataNodeReferenceRole(), n, plotDataNodeID);
}

//----------------------------------------------------------------------------
bool vtkMRMLPlotChartNode::HasPlotDataNodeID(const char* plotDataNodeID)
{
  return this->HasNodeReferenceID(this->GetPlotDataNodeReferenceRole(), plotDataNodeID);
}

//----------------------------------------------------------------------------
int vtkMRMLPlotChartNode::GetNumberOfPlotDataNodes()
{
  return this->GetNumberOfNodeReferences(this->GetPlotDataNodeReferenceRole());
}

//----------------------------------------------------------------------------
const char* vtkMRMLPlotChartNode::GetNthPlotDataNodeID(int n)
{
    return this->GetNthNodeReferenceID(this->GetPlotDataNodeReferenceRole(), n);
}

//----------------------------------------------------------------------------
int vtkMRMLPlotChartNode::GetNthPlotIdexFromID(const char *plotDataNodeID)
{
  if (!plotDataNodeID)
    {
    return -1;
    }

  int numPlotDataNodes = this->GetNumberOfNodeReferences(
    this->GetPlotDataNodeReferenceRole());

  for (int plotIndex = 0; plotIndex < numPlotDataNodes; plotIndex++)
    {
    const char* id = this->GetNthNodeReferenceID(
      this->GetPlotDataNodeReferenceRole(), plotIndex);
    if (!strcmp(plotDataNodeID, id))
      {
      return plotIndex;
      break;
      }
    }

  return -1;
}

//----------------------------------------------------------------------------
vtkIdType vtkMRMLPlotChartNode::GetColorPlotIdexFromID(const char *plotDataNodeID)
{
  std::string tempPlotDataNodeID(plotDataNodeID);

  vtkMRMLPlotDataNode* plotDataNode = this->GetNthPlotDataNode
    (this->GetNthPlotIdexFromID(plotDataNodeID));
  if (!plotDataNode)
    {
    return -1;
    }
  std::string namePlotDataNode = plotDataNode->GetName();
  std::size_t found = namePlotDataNode.find("Markups");
  if (found != std::string::npos)
    {
    vtkMRMLPlotDataNode* markupsPlotDataNode = vtkMRMLPlotDataNode::SafeDownCast
      (plotDataNode->GetNodeReference("Markups"));
    if (!markupsPlotDataNode)
      {
      return -1;
      }
    tempPlotDataNodeID = markupsPlotDataNode->GetID();
    }

  return this->GetNthPlotIdexFromID(tempPlotDataNodeID.c_str());
}

//----------------------------------------------------------------------------
const char* vtkMRMLPlotChartNode::GetPlotDataNodeID()
{
  return this->GetNthPlotDataNodeID(0);
}

//----------------------------------------------------------------------------
vtkMRMLPlotDataNode* vtkMRMLPlotChartNode::GetNthPlotDataNode(int n)
{
  return vtkMRMLPlotDataNode::SafeDownCast(
    this->GetNthNodeReference(this->GetPlotDataNodeReferenceRole(), n));
}

//----------------------------------------------------------------------------
vtkMRMLPlotDataNode* vtkMRMLPlotChartNode::GetPlotDataNode()
{
  return this->GetNthPlotDataNode(0);
}

//----------------------------------------------------------------------------
void vtkMRMLPlotChartNode::ProcessMRMLEvents(vtkObject *caller,
                                              unsigned long event,
                                              void *callData)
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  int numPlotDataNodes = this->GetNumberOfNodeReferences(this->GetPlotDataNodeReferenceRole());
  for (int plotIndex = 0; plotIndex < numPlotDataNodes; plotIndex++)
    {
    vtkMRMLPlotDataNode *pnode = this->GetNthPlotDataNode(plotIndex);
    if (pnode != NULL && pnode == vtkMRMLPlotDataNode::SafeDownCast(caller) &&
       (event ==  vtkCommand::ModifiedEvent || event == vtkMRMLPlotDataNode::TableModifiedEvent))
      {
      this->InvokeEvent(vtkMRMLPlotChartNode::PlotModifiedEvent, pnode);
      this->Modified();
      }
    }

  return;
}

//----------------------------------------------------------------------------
int vtkMRMLPlotChartNode::GetPlotNames(std::vector<std::string> &plotDataNodeNames)
{
  plotDataNodeNames.clear();
  int numPlotDataNodes = this->GetNumberOfNodeReferences(this->GetPlotDataNodeReferenceRole());
  for (int plotIndex = 0; plotIndex < numPlotDataNodes; plotIndex++)
    {
    vtkMRMLPlotDataNode *pnode = this->GetNthPlotDataNode(plotIndex);
    if (!pnode)
      {
      continue;
      }
    plotDataNodeNames.push_back(pnode->GetName());
    }

  return static_cast<int>(plotDataNodeNames.size());
}

//----------------------------------------------------------------------------
int vtkMRMLPlotChartNode::GetPlotIDs(std::vector<std::string> &plotDataNodeIDs)
{
  plotDataNodeIDs.clear();
  int numPlotDataNodes = this->GetNumberOfNodeReferences(this->GetPlotDataNodeReferenceRole());
  for (int plotIndex = 0; plotIndex < numPlotDataNodes; plotIndex++)
    {
    plotDataNodeIDs.push_back(this->GetNthPlotDataNodeID(plotIndex));
    }

  return static_cast<int>(plotDataNodeIDs.size());
}

//----------------------------------------------------------------------------
void vtkMRMLPlotChartNode::SetPlotType(const char *Type)
{
    if (!this->GetScene() || !strcmp(Type, this->GetAttribute("Type")))
      {
      return;
      }

    int wasModifying = this->StartModify();
    std::vector<std::string> plotDataNodesIDs;
    this->GetPlotIDs(plotDataNodesIDs);

    std::vector<std::string>::iterator it = plotDataNodesIDs.begin();
    for (; it != plotDataNodesIDs.end(); ++it)
      {
      vtkMRMLPlotDataNode* plotDataNode = vtkMRMLPlotDataNode::SafeDownCast
        (this->GetScene()->GetNodeByID((*it).c_str()));
      if (!plotDataNode)
        {
        continue;
        }

      std::string namePlotDataNode = plotDataNode->GetName();
      std::size_t found = namePlotDataNode.find("Markups");
      if (found != std::string::npos &&
          (!strcmp(Type,"Line") || !strcmp(Type,"Scatter") || !strcmp(Type,"Bar")))
        {
        this->RemovePlotDataNodeID(plotDataNode->GetID());
        plotDataNode->GetNodeReference("Markups")->RemoveNodeReferenceIDs("Markups");
        this->GetScene()->RemoveNode(plotDataNode);
        continue;
        }

      if (!strcmp(Type,"Line"))
        {
        plotDataNode->SetType(vtkMRMLPlotDataNode::LINE);
        }
      else if (!strcmp(Type,"Scatter"))
        {
        plotDataNode->SetType(vtkMRMLPlotDataNode::POINTS);
        }
      else if (!strcmp(Type,"Line and Scatter"))
        {
        plotDataNode->SetType(vtkMRMLPlotDataNode::LINE);

        vtkMRMLPlotDataNode* plotDataNodeCopy = vtkMRMLPlotDataNode::SafeDownCast
          (plotDataNode->GetNodeReference("Markups"));

        if (plotDataNodeCopy)
          {
          plotDataNodeCopy->SetType(vtkMRMLPlotDataNode::POINTS);
          }
        else
          {
          vtkSmartPointer<vtkMRMLNode> node = vtkSmartPointer<vtkMRMLNode>::Take
            (this->GetScene()->CreateNodeByClass("vtkMRMLPlotDataNode"));
          plotDataNodeCopy = vtkMRMLPlotDataNode::SafeDownCast(node);
          std::string namePlotDataNodeCopy = namePlotDataNode + " Markups";
          plotDataNodeCopy->CopyWithScene(plotDataNode);
          plotDataNodeCopy->SetName(namePlotDataNodeCopy.c_str());
          plotDataNodeCopy->SetType(vtkMRMLPlotDataNode::POINTS);
          this->GetScene()->AddNode(plotDataNodeCopy);
          plotDataNode->AddNodeReferenceID("Markups", plotDataNodeCopy->GetID());
          plotDataNodeCopy->AddNodeReferenceID("Markups", plotDataNode->GetID());
          }

        this->AddAndObservePlotDataNodeID(plotDataNodeCopy->GetID());
        }
      else if (!strcmp(Type,"Bar"))
        {
        plotDataNode->SetType(vtkMRMLPlotDataNode::BAR);
        }
      else
        {
        vtkErrorWithObjectMacro(this, "vtkMRMLPlotChartNode::SetPlotType: Unknown PlotType"<< Type);
        this->EndModify(wasModifying);
        return;
        }
      }

    this->SetAttribute("Type", Type);

    this->EndModify(wasModifying);
}
