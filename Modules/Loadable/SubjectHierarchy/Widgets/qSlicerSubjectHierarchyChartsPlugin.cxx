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

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLSubjectHierarchyConstants.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyChartsPlugin.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLChartNode.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLChartViewNode.h>

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
class qSlicerSubjectHierarchyChartsPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyChartsPlugin);
protected:
  qSlicerSubjectHierarchyChartsPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyChartsPluginPrivate(qSlicerSubjectHierarchyChartsPlugin& object);
  ~qSlicerSubjectHierarchyChartsPluginPrivate();
  void init();
public:
  QIcon ChartIcon;

  QIcon VisibleIcon;
  QIcon HiddenIcon;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyChartsPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyChartsPluginPrivate::qSlicerSubjectHierarchyChartsPluginPrivate(qSlicerSubjectHierarchyChartsPlugin& object)
: q_ptr(&object)
{
  this->ChartIcon = QIcon(":Icons/Chart.png");

  this->VisibleIcon = QIcon(":Icons/VisibleOn.png");
  this->HiddenIcon = QIcon(":Icons/VisibleOff.png");
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyChartsPluginPrivate::~qSlicerSubjectHierarchyChartsPluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyChartsPlugin::qSlicerSubjectHierarchyChartsPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyChartsPluginPrivate(*this) )
{
  this->m_Name = QString("Charts");

  Q_D(qSlicerSubjectHierarchyChartsPlugin);
  d->init();
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyChartsPluginPrivate::init()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyChartsPlugin::~qSlicerSubjectHierarchyChartsPlugin()
{
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyChartsPlugin::canAddNodeToSubjectHierarchy(vtkMRMLNode* node, vtkMRMLSubjectHierarchyNode* parent/*=NULL*/)const
{
  Q_UNUSED(parent);
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyChartsPlugin::canAddNodeToSubjectHierarchy: Input node is NULL!";
    return 0.0;
    }
  else if (node->IsA("vtkMRMLChartNode"))
    {
    // Node is a chart
    return 0.5;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyChartsPlugin::canOwnSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)const
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyChartsPlugin::canOwnSubjectHierarchyNode: Input node is NULL!";
    return 0.0;
    }

  // Chart
  vtkMRMLNode* associatedNode = node->GetAssociatedNode();
  if (associatedNode && associatedNode->IsA("vtkMRMLChartNode"))
    {
    return 0.5; // There may be other plugins that can handle special charts better
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyChartsPlugin::roleForPlugin()const
{
  return "Chart";
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyChartsPlugin::icon(vtkMRMLSubjectHierarchyNode* node)
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyChartsPlugin::icon: NULL node given!";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchyChartsPlugin);

  if (this->canOwnSubjectHierarchyNode(node))
    {
    return d->ChartIcon;
    }

  // Node unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyChartsPlugin::visibilityIcon(int visible)
{
  Q_D(qSlicerSubjectHierarchyChartsPlugin);

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
void qSlicerSubjectHierarchyChartsPlugin::setDisplayVisibility(vtkMRMLSubjectHierarchyNode* node, int visible)
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyChartsPlugin::setDisplayVisibility: NULL node!";
    return;
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();
  if (!scene)
    {
    qCritical() << "qSlicerSubjectHierarchyChartsPlugin::setDisplayVisibility: Invalid MRML scene!";
    return;
    }
  if (this->getDisplayVisibility(node) == visible)
    {
    return;
    }

  // Get layout node
  vtkSmartPointer<vtkCollection> layoutNodes =
    vtkSmartPointer<vtkCollection>::Take( scene->GetNodesByClass("vtkMRMLLayoutNode") );
  layoutNodes->InitTraversal();
  vtkObject* layoutNodeVtkObject = layoutNodes->GetNextItemAsObject();
  vtkMRMLLayoutNode* layoutNode = vtkMRMLLayoutNode::SafeDownCast(layoutNodeVtkObject);
  if (!layoutNode)
    {
    qCritical() << "qSlicerSubjectHierarchyChartsPlugin::getChartViewNode: Unable to get layout node!";
    return;
    }

  vtkMRMLChartViewNode* chartViewNode = this->getChartViewNode();

  vtkMRMLChartNode* associatedChartNode = vtkMRMLChartNode::SafeDownCast(node->GetAssociatedNode());
  if (associatedChartNode && visible)
    {
    // Switch to four-up quantitative layout
    layoutNode->SetViewArrangement( vtkMRMLLayoutNode::SlicerLayoutConventionalQuantitativeView );

    // Make sure we have a valid chart view node (if we want to show the chart, but there was
    // no chart view, then one was just created when we switched to quantitative layout)
    if (!chartViewNode)
      {
      chartViewNode = this->getChartViewNode();
      }

    // Hide currently shown chart and trigger icon update
    if ( chartViewNode->GetChartNodeID()
      && strcmp(chartViewNode->GetChartNodeID(), associatedChartNode->GetID()) )
      {
      vtkMRMLSubjectHierarchyNode* currentChartShNode =
        vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(
        scene->GetNodeByID(chartViewNode->GetChartNodeID()) );
      if (currentChartShNode)
        {
        chartViewNode->SetChartNodeID(NULL);
        currentChartShNode->Modified();
        }
      }

    // Select chart to show
    chartViewNode->SetChartNodeID(associatedChartNode->GetID());
    }
  else if (chartViewNode)
    {
    // Hide chart
    chartViewNode->SetChartNodeID(NULL);
    }

  // Trigger icon update
  node->Modified();
}

//-----------------------------------------------------------------------------
int qSlicerSubjectHierarchyChartsPlugin::getDisplayVisibility(vtkMRMLSubjectHierarchyNode* node)const
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyChartsPlugin::getDisplayVisibility: NULL node!";
    return -1;
    }

  vtkMRMLChartViewNode* chartViewNode = this->getChartViewNode();
  if (!chartViewNode)
    {
    // No quantitative view has been set yet
    return 0;
    }

  // Return hidden if current layout is not one of the quantitative ones
  if ( qSlicerApplication::application()->layoutManager()->layout() != vtkMRMLLayoutNode::SlicerLayoutFourUpQuantitativeView
    && qSlicerApplication::application()->layoutManager()->layout() != vtkMRMLLayoutNode::SlicerLayoutOneUpQuantitativeView
    && qSlicerApplication::application()->layoutManager()->layout() != vtkMRMLLayoutNode::SlicerLayoutConventionalQuantitativeView
    && qSlicerApplication::application()->layoutManager()->layout() != vtkMRMLLayoutNode::SlicerLayoutThreeOverThreeQuantitativeView )
    {
    return 0;
    }

  // Return shown if chart in chart view is the examined node's associated data node
  vtkMRMLChartNode* associatedChartNode = vtkMRMLChartNode::SafeDownCast(node->GetAssociatedNode());
  if ( associatedChartNode && chartViewNode->GetChartNodeID()
    && !strcmp(chartViewNode->GetChartNodeID(), associatedChartNode->GetID()) )
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyChartsPlugin::editProperties(vtkMRMLSubjectHierarchyNode* node)
{
  Q_UNUSED(node);
  // No module to edit Charts, just switch layout
}

//---------------------------------------------------------------------------
vtkMRMLChartViewNode* qSlicerSubjectHierarchyChartsPlugin::getChartViewNode()const
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();
  if (!scene)
    {
    qCritical() << "qSlicerSubjectHierarchyChartsPlugin::getChartViewNode: Invalid MRML scene!";
    return NULL;
    }

  vtkSmartPointer<vtkCollection> chartViewNodes =
    vtkSmartPointer<vtkCollection>::Take( scene->GetNodesByClass("vtkMRMLChartViewNode") );
  chartViewNodes->InitTraversal();
  vtkMRMLChartViewNode* chartViewNode = vtkMRMLChartViewNode::SafeDownCast( chartViewNodes->GetNextItemAsObject() );
  if (!chartViewNode)
    {
    qCritical() << "qSlicerSubjectHierarchyChartsPlugin::getChartViewNode: Unable to get chart view node!";
    return NULL;
    }

  return chartViewNode;
}
