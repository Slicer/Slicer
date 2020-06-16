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
#include "qSlicerSubjectHierarchyLabelMapsPlugin.h"
#include "qSlicerSubjectHierarchyVolumesPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "vtkSlicerApplicationLogic.h"

// MRML includes
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCollection.h>
#include <vtkImageData.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Plugins
class qSlicerSubjectHierarchyLabelMapsPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyLabelMapsPlugin);
protected:
  qSlicerSubjectHierarchyLabelMapsPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyLabelMapsPluginPrivate(qSlicerSubjectHierarchyLabelMapsPlugin& object);
  ~qSlicerSubjectHierarchyLabelMapsPluginPrivate() override;
  void init();
public:
  QIcon LabelmapIcon;

  QAction* ToggleOutlineVisibilityAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyLabelMapsPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyLabelMapsPluginPrivate::qSlicerSubjectHierarchyLabelMapsPluginPrivate(qSlicerSubjectHierarchyLabelMapsPlugin& object)
: q_ptr(&object)
{
  this->LabelmapIcon = QIcon(":Icons/Labelmap.png");

  this->ToggleOutlineVisibilityAction = nullptr;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyLabelMapsPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyLabelMapsPlugin);

  this->ToggleOutlineVisibilityAction = new QAction("2D outline visibility",q);
  QObject::connect(this->ToggleOutlineVisibilityAction, SIGNAL(toggled(bool)), q, SLOT(toggle2DOutlineVisibility(bool)));
  this->ToggleOutlineVisibilityAction->setCheckable(true);
  this->ToggleOutlineVisibilityAction->setChecked(false);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyLabelMapsPluginPrivate::~qSlicerSubjectHierarchyLabelMapsPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyLabelMapsPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyLabelMapsPlugin::qSlicerSubjectHierarchyLabelMapsPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyLabelMapsPluginPrivate(*this) )
{
  this->m_Name = QString("LabelMaps");

  Q_D(qSlicerSubjectHierarchyLabelMapsPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyLabelMapsPlugin::~qSlicerSubjectHierarchyLabelMapsPlugin() = default;

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyLabelMapsPlugin::canAddNodeToSubjectHierarchy(
  vtkMRMLNode* node, vtkIdType parentItemID/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(parentItemID);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": Input node is nullptr!";
    return 0.0;
    }
  else if (node->IsA("vtkMRMLLabelMapVolumeNode"))
    {
    // Node is a labelmap
    return 0.7;
    }
  return 0.0;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyLabelMapsPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
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

  // Labelmap volume
  vtkMRMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && associatedNode->IsA("vtkMRMLLabelMapVolumeNode"))
    {
    return 0.7;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyLabelMapsPlugin::roleForPlugin()const
{
  return "Label map volume";
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyLabelMapsPlugin::tooltip(vtkIdType itemID)const
{
  return qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Volumes")->tooltip(itemID);
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyLabelMapsPlugin::icon(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyLabelMapsPlugin);

  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }

  // Volume
  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    return d->LabelmapIcon;
    }

  // Item unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyLabelMapsPlugin::visibilityIcon(int visible)
{
  return qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Volumes")->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyLabelMapsPlugin::setDisplayVisibility(vtkIdType itemID, int visible)
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
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

  vtkMRMLLabelMapVolumeNode* associatedLabelMapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  // Labelmap volume
  if (associatedLabelMapNode)
    {
    if (visible)
      {
      // If visibility is on, then show the labelmap in the label layer of all slice views
      this->showLabelMapInAllViews(associatedLabelMapNode);
      }
    else
      {
      // If visibility is off, then hide the labelmap from all slice views
      qSlicerSubjectHierarchyVolumesPlugin* volumesPlugin = qobject_cast<qSlicerSubjectHierarchyVolumesPlugin*>(
        qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Volumes") );
      if (!volumesPlugin)
        {
        qCritical() << Q_FUNC_INFO << ": Failed to access Volumes subject hierarchy plugin";
        return;
        }
      volumesPlugin->hideVolumeFromAllViews(associatedLabelMapNode);
      }
    }
  // Default
  else
    {
    qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->setDisplayVisibility(itemID, visible);
    }
}

//-----------------------------------------------------------------------------
int qSlicerSubjectHierarchyLabelMapsPlugin::getDisplayVisibility(vtkIdType itemID)const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
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
  qSlicerSubjectHierarchyVolumesPlugin* volumesPlugin = qobject_cast<qSlicerSubjectHierarchyVolumesPlugin*>(
    qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Volumes") );
  if (!volumesPlugin)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access Volumes subject hierarchy plugin";
    return -1;
    }

  // Sanity checks for labelmap
  vtkMRMLLabelMapVolumeNode* labelMapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!labelMapNode)
    {
    return -1;
    }

  // Collect all volumes that are shown in any slice views in any layers
  QSet<vtkIdType> shownVolumeItemIDs;

  volumesPlugin->collectShownVolumes(shownVolumeItemIDs);
  if (shownVolumeItemIDs.contains(itemID))
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyLabelMapsPlugin::showLabelMapInAllViews(vtkMRMLLabelMapVolumeNode* node)
{
  qSlicerSubjectHierarchyVolumesPlugin* volumesPlugin = qobject_cast<qSlicerSubjectHierarchyVolumesPlugin*>(
    qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Volumes") );
  if (!volumesPlugin)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access Volumes subject hierarchy plugin";
    return;
    }

  volumesPlugin->showVolumeInAllViews(node, vtkMRMLApplicationLogic::LabelLayer);
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyLabelMapsPlugin::visibilityContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyLabelMapsPlugin);

  QList<QAction*> actions;
  actions << d->ToggleOutlineVisibilityAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyLabelMapsPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyLabelMapsPlugin);

  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    return;
    }

  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();

  // LabelMap
  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    // Determine current state of the toggle labelmap outline checkbox (from the first slice view)
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast ( scene->GetNthNodeByClass( 0, "vtkMRMLSliceNode" ) );
    int useLabelOutline = sliceNode->GetUseLabelOutline();
    d->ToggleOutlineVisibilityAction->blockSignals(true);
    d->ToggleOutlineVisibilityAction->setChecked(useLabelOutline);
    d->ToggleOutlineVisibilityAction->blockSignals(false);

    d->ToggleOutlineVisibilityAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyLabelMapsPlugin::toggle2DOutlineVisibility(bool checked)
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  vtkMRMLSliceNode* sliceNode = nullptr;
  const int numberOfSliceNodes = scene->GetNumberOfNodesByClass("vtkMRMLSliceNode");

  for (int i=0; i<numberOfSliceNodes; i++)
    {
    sliceNode = vtkMRMLSliceNode::SafeDownCast ( scene->GetNthNodeByClass( i, "vtkMRMLSliceNode" ) );
    sliceNode->SetUseLabelOutline(checked);
    }
}
