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

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyPlotsPlugin.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLPlotChartNode.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLPlotViewNode.h>

#include <vtkSlicerPlotsLogic.h>
#include <qMRMLPlotWidget.h>

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
class qSlicerSubjectHierarchyPlotsPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyPlotsPlugin);
protected:
  qSlicerSubjectHierarchyPlotsPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyPlotsPluginPrivate(qSlicerSubjectHierarchyPlotsPlugin& object);
  ~qSlicerSubjectHierarchyPlotsPluginPrivate();
  void init();
public:
  QIcon PlotIcon;

  vtkWeakPointer<vtkSlicerPlotsLogic> PlotsLogic;

  QIcon VisibleIcon;
  QIcon HiddenIcon;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyPlotsPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPlotsPluginPrivate::qSlicerSubjectHierarchyPlotsPluginPrivate(qSlicerSubjectHierarchyPlotsPlugin& object)
: q_ptr(&object)
{
  this->PlotIcon = QIcon(":Icons/Chart.png");

  this->VisibleIcon = QIcon(":Icons/VisibleOn.png");
  this->HiddenIcon = QIcon(":Icons/VisibleOff.png");
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyPlotsPluginPrivate::init()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPlotsPluginPrivate::~qSlicerSubjectHierarchyPlotsPluginPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyPlotsPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPlotsPlugin::qSlicerSubjectHierarchyPlotsPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyPlotsPluginPrivate(*this) )
{
  Q_D(qSlicerSubjectHierarchyPlotsPlugin);
  this->m_Name = QString("PlotChart");
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPlotsPlugin::~qSlicerSubjectHierarchyPlotsPlugin()
{
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPlotsPlugin::setPlotsLogic(vtkSlicerPlotsLogic* plotsLogic)
{
  Q_D(qSlicerSubjectHierarchyPlotsPlugin);
  d->PlotsLogic = plotsLogic;
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyPlotsPlugin::canAddNodeToSubjectHierarchy(
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
double qSlicerSubjectHierarchyPlotsPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
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
const QString qSlicerSubjectHierarchyPlotsPlugin::roleForPlugin()const
{
  return "PlotLayout";
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyPlotsPlugin::icon(vtkIdType itemID)
{
  if (!itemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchyPlotsPlugin);

  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    return d->PlotIcon;
    }

  // Item unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyPlotsPlugin::visibilityIcon(int visible)
{
  Q_D(qSlicerSubjectHierarchyPlotsPlugin);

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
void qSlicerSubjectHierarchyPlotsPlugin::setDisplayVisibility(vtkIdType itemID, int visible)
{
  Q_D(qSlicerSubjectHierarchyPlotsPlugin);

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

  if (d->PlotsLogic)
    {
    vtkMRMLPlotChartNode* associatedPlotChartNode = vtkMRMLPlotChartNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    d->PlotsLogic->ShowChartInLayout(visible ? associatedPlotChartNode : NULL);
    }
  else
    {
    qWarning() << Q_FUNC_INFO << ": plotsLogic is not set, cannot show chart in layout";
    }

  // Update icons of all charts (if we show this chart then we may have hidden other charts)
  if (scene->IsBatchProcessing())
    {
    return;
    }
  std::vector< vtkMRMLNode* > chartNodes;
  scene->GetNodesByClass("vtkMRMLPlotChartNode", chartNodes);
  for (std::vector< vtkMRMLNode* >::iterator chartIt = chartNodes.begin(); chartIt != chartNodes.end(); ++chartIt)
    {
    vtkMRMLPlotChartNode* chartNode = vtkMRMLPlotChartNode::SafeDownCast(*chartIt);
    vtkIdType chartNodeId = shNode->GetItemByDataNode(chartNode);
    shNode->ItemModified(chartNodeId);
    }
}

//-----------------------------------------------------------------------------
int qSlicerSubjectHierarchyPlotsPlugin::getDisplayVisibility(vtkIdType itemID)const
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

  // Return shown if plotLayout in plot view is the examined item's associated data node
  vtkMRMLPlotChartNode* associatedPlotChartNode = vtkMRMLPlotChartNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!associatedPlotChartNode)
    {
    return 0;
    }

  return (associatedPlotChartNode == plotViewNode->GetPlotChartNode()) ? 1 : 0;
}

//---------------------------------------------------------------------------
vtkMRMLPlotViewNode* qSlicerSubjectHierarchyPlotsPlugin::getPlotViewNode()const
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    return NULL;
    }

  qMRMLLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
    {
    return NULL;
    }

  for (int i=0; i<layoutManager->plotViewCount(); i++)
    {
    qMRMLPlotWidget* plotWidget = layoutManager->plotWidget(0);
    if (!plotWidget)
      {
      // invalid plot widget
      continue;
      }
    vtkMRMLPlotViewNode* plotView = plotWidget->mrmlPlotViewNode();
    if (plotView)
      {
      return plotView;
      }
    }

  // no valid plot view in current layout
  return NULL;
}
