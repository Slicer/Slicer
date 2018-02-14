/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

// Plots Logic includes
#include "vtkSlicerPlotsLogic.h"

// Slicer includes
#include <qSlicerApplication.h>
#include <qSlicerLayoutManager.h>

// MRML includes
#include <vtkMRMLPlotChartNode.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLPlotSeriesNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>

// STD includes

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerPlotsLogic);

//----------------------------------------------------------------------------
vtkSlicerPlotsLogic::vtkSlicerPlotsLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerPlotsLogic::~vtkSlicerPlotsLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerPlotsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
int vtkSlicerPlotsLogic::GetLayoutWithPlot(int currentLayout)
{
  switch (currentLayout)
    {
    case vtkMRMLLayoutNode::SlicerLayoutFourUpPlotView:
    case vtkMRMLLayoutNode::SlicerLayoutFourUpPlotTableView:
    case vtkMRMLLayoutNode::SlicerLayoutOneUpPlotView:
    case vtkMRMLLayoutNode::SlicerLayoutConventionalPlotView:
    case vtkMRMLLayoutNode::SlicerLayoutThreeOverThreePlotView:
      // plot already shown, no need to change
      return currentLayout;
    case vtkMRMLLayoutNode::SlicerLayoutFourUpTableView:
      return vtkMRMLLayoutNode::SlicerLayoutFourUpPlotTableView;
    case vtkMRMLLayoutNode::SlicerLayoutConventionalView:
      return vtkMRMLLayoutNode::SlicerLayoutConventionalPlotView;
    default:
      return vtkMRMLLayoutNode::SlicerLayoutFourUpPlotView;
    }
}

// --------------------------------------------------------------------------
vtkMRMLPlotSeriesNode* vtkSlicerPlotsLogic::CloneSeries(vtkMRMLPlotSeriesNode* source, const char * vtkNotUsed(name))
{
  if (!source || source->GetScene() == NULL)
    {
    vtkErrorMacro("vtkSlicerPlotsLogic::CloneSeries failed: source is NULL or not added to a a scene");
    return NULL;
    }

  vtkSmartPointer<vtkMRMLNode> clonedNode = vtkSmartPointer<vtkMRMLNode>::Take(
    source->GetScene()->CreateNodeByClass("vtkMRMLPlotSeriesNode"));
  vtkMRMLPlotSeriesNode *clonedSeriesNode = vtkMRMLPlotSeriesNode::SafeDownCast(clonedNode);
  clonedSeriesNode->CopyWithScene(source);
  std::string nodeName(source->GetName() ? source->GetName() : "");
  nodeName += "_Copy";

  clonedSeriesNode->SetName(source->GetScene()->GetUniqueNameByString(nodeName.c_str()));
  source->GetScene()->AddNode(clonedSeriesNode);
  return clonedSeriesNode;
}

// --------------------------------------------------------------------------
void vtkSlicerPlotsLogic::ShowChartInLayout(vtkMRMLPlotChartNode* chartNode)
{
  // Switch to a layout that contains plot
  vtkMRMLLayoutNode* layoutNode = vtkMRMLLayoutNode::SafeDownCast(this->GetMRMLScene()->GetFirstNodeByClass("vtkMRMLLayoutNode"));
  if (layoutNode)
    {
    qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
    if (layoutManager)
      {
      int currentLayout = layoutManager->layout();
      int layoutWithPlot = vtkSlicerPlotsLogic::GetLayoutWithPlot(currentLayout);
      if (currentLayout != layoutWithPlot)
        {
        layoutNode->SetViewArrangement(layoutWithPlot);
        }
      }
    }

  // Show plot in viewers
  vtkSlicerApplicationLogic* appLogic = qSlicerApplication::application()->applicationLogic();
  if (appLogic)
    {
    vtkMRMLSelectionNode* selectionNode = appLogic->GetSelectionNode();
    if (selectionNode)
      {
      const char* chartNodeID = (chartNode ? chartNode->GetID() : NULL);
      selectionNode->SetActivePlotChartID(chartNodeID);
      }
    appLogic->PropagatePlotChartSelection();
    }
}
