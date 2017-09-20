/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyPlotChartPlugin.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLPlotChartNode.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLPlotViewNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCollection.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyPlotChartPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyPlotChartPlugin);
protected:
  qSlicerSubjectHierarchyPlotChartPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyPlotChartPluginPrivate(qSlicerSubjectHierarchyPlotChartPlugin& object);
  ~qSlicerSubjectHierarchyPlotChartPluginPrivate();
  void init();
public:
  QIcon PlotIcon;

  QIcon VisibleIcon;
  QIcon HiddenIcon;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyPlotChartPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPlotChartPluginPrivate::qSlicerSubjectHierarchyPlotChartPluginPrivate(qSlicerSubjectHierarchyPlotChartPlugin& object)
: q_ptr(&object)
{
  this->PlotIcon = QIcon(":Icons/Chart.png");

  this->VisibleIcon = QIcon(":Icons/VisibleOn.png");
  this->HiddenIcon = QIcon(":Icons/VisibleOff.png");
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyPlotChartPluginPrivate::init()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPlotChartPluginPrivate::~qSlicerSubjectHierarchyPlotChartPluginPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyPlotChartPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPlotChartPlugin::qSlicerSubjectHierarchyPlotChartPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyPlotChartPluginPrivate(*this) )
{
  this->m_Name = QString("PlotChart");

  Q_D(qSlicerSubjectHierarchyPlotChartPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPlotChartPlugin::~qSlicerSubjectHierarchyPlotChartPlugin()
{
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyPlotChartPlugin::canAddNodeToSubjectHierarchy(
  vtkMRMLNode* node, vtkIdType parentItemID/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(parentItemID);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": Input node is NULL!";
    return 0.0;
    }
  else if (node->IsA("vtkMRMLPlotChartNode"))
    {
    // Node is a plotLayout
    return 0.5;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyPlotChartPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
{
  if (!itemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return 0.0;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return 0.0;
    }

  // PlotLayout
  vtkMRMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && associatedNode->IsA("vtkMRMLPlotChartNode"))
    {
    return 0.5; // There may be other plugins that can handle special PlotChart better
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyPlotChartPlugin::roleForPlugin()const
{
  return "PlotLayout";
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyPlotChartPlugin::icon(vtkIdType itemID)
{
  if (!itemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchyPlotChartPlugin);

  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    return d->PlotIcon;
    }

  // Item unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyPlotChartPlugin::visibilityIcon(int visible)
{
  Q_D(qSlicerSubjectHierarchyPlotChartPlugin);

  if (visible)
    {
    return d->VisibleIcon;
    }
  else
    {
    return d->HiddenIcon;
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyPlotChartPlugin::setDisplayVisibility(vtkIdType itemID, int visible)
{
  if (!itemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    return;
    }

  if (this->getDisplayVisibility(itemID) == visible)
    {
    return;
    }

  // Get layout node
  vtkMRMLLayoutNode* layoutNode = vtkMRMLLayoutNode::SafeDownCast(scene->GetFirstNodeByClass("vtkMRMLLayoutNode"));
  if (!layoutNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get layout node!";
    return;
    }

  vtkMRMLPlotViewNode* plotViewNode = this->getPlotViewNode();

  vtkMRMLPlotChartNode* associatedPlotChartNode = vtkMRMLPlotChartNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (associatedPlotChartNode && visible)
    {
    // Switch to four-up quantitative layout
    layoutNode->SetViewArrangement( vtkMRMLLayoutNode::SlicerLayoutConventionalPlotView );

    // Make sure we have a valid plot view node (if we want to show the plotLayout, but there was
    // no plot view, then one was just created when we switched to quantitative layout)
    if (!plotViewNode)
      {
      plotViewNode = this->getPlotViewNode();
      }

    // Hide currently shown plotLayout and trigger icon update
    if ( plotViewNode->GetPlotChartNodeID()
      && strcmp(plotViewNode->GetPlotChartNodeID(), associatedPlotChartNode->GetID()) )
      {
      vtkIdType plotLayoutItemID = shNode->GetItemByDataNode(scene->GetNodeByID(plotViewNode->GetPlotChartNodeID()));
      if (plotLayoutItemID)
        {
        plotViewNode->SetPlotChartNodeID(NULL);
        shNode->ItemModified(plotLayoutItemID);
        }
      }

    // Select plotLayout to show
    plotViewNode->SetPlotChartNodeID(associatedPlotChartNode->GetID());
    }
  else if (plotViewNode)
    {
    // Hide plotLayout
    plotViewNode->SetPlotChartNodeID(NULL);
    }

  // Trigger icon update
  shNode->ItemModified(itemID);
}

//-----------------------------------------------------------------------------
int qSlicerSubjectHierarchyPlotChartPlugin::getDisplayVisibility(vtkIdType itemID)const
{
  if (!itemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return -1;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return -1;
    }

  vtkMRMLPlotViewNode* plotViewNode = this->getPlotViewNode();
  if (!plotViewNode)
    {
    // No quantitative view has been set yet
    return 0;
    }

  // Return hidden if current layout is not one of the quantitative ones
  if ( qSlicerApplication::application()->layoutManager()->layout() != vtkMRMLLayoutNode::SlicerLayoutFourUpPlotView
    && qSlicerApplication::application()->layoutManager()->layout() != vtkMRMLLayoutNode::SlicerLayoutFourUpPlotTableView
    && qSlicerApplication::application()->layoutManager()->layout() != vtkMRMLLayoutNode::SlicerLayoutOneUpPlotView
    && qSlicerApplication::application()->layoutManager()->layout() != vtkMRMLLayoutNode::SlicerLayoutConventionalPlotView
    && qSlicerApplication::application()->layoutManager()->layout() != vtkMRMLLayoutNode::SlicerLayoutThreeOverThreePlotView)
    {
    return 0;
    }

  // Return shown if plotLayout in plot view is the examined item's associated data node
  vtkMRMLPlotChartNode* associatedPlotChartNode = vtkMRMLPlotChartNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if ( associatedPlotChartNode && plotViewNode->GetPlotChartNodeID()
    && !strcmp(plotViewNode->GetPlotChartNodeID(), associatedPlotChartNode->GetID()) )
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyPlotChartPlugin::editProperties(vtkIdType itemID)
{
  Q_UNUSED(itemID);
  // No module to edit PlotChart, just switch layout
}

//---------------------------------------------------------------------------
vtkMRMLPlotViewNode* qSlicerSubjectHierarchyPlotChartPlugin::getPlotViewNode()const
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    return NULL;
    }

  vtkMRMLPlotViewNode* plotViewNode = vtkMRMLPlotViewNode::SafeDownCast(scene->GetFirstNodeByClass("vtkMRMLPlotViewNode"));
  if (!plotViewNode)
    {
    return NULL;
    }

  return plotViewNode;
}
