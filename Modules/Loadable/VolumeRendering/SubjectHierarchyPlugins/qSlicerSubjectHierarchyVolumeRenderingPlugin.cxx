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
#include "qSlicerSubjectHierarchyVolumeRenderingPlugin.h"
#include "qSlicerSubjectHierarchyVolumesPlugin.h"

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"

// Volume Rendering includes
#include "vtkSlicerVolumeRenderingLogic.h"

// MRML includes
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLViewLogic.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLVolumeRenderingDisplayNode.h>
#include <vtkMRMLVolumePropertyNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// Qt includes
#include <QDebug>
#include <QAction>
#include <QSettings>

// MRML widgets includes
#include "qMRMLNodeComboBox.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyVolumeRenderingPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyVolumeRenderingPlugin);
protected:
  qSlicerSubjectHierarchyVolumeRenderingPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyVolumeRenderingPluginPrivate(qSlicerSubjectHierarchyVolumeRenderingPlugin& object);
  ~qSlicerSubjectHierarchyVolumeRenderingPluginPrivate() override;
  void init();
public:
  vtkWeakPointer<vtkSlicerVolumeRenderingLogic> VolumeRenderingLogic;

  QAction* ToggleVolumeRenderingAction;
  QAction* VolumeRenderingOptionsAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyVolumeRenderingPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumeRenderingPluginPrivate::qSlicerSubjectHierarchyVolumeRenderingPluginPrivate(qSlicerSubjectHierarchyVolumeRenderingPlugin& object)
: q_ptr(&object)
, ToggleVolumeRenderingAction(nullptr)
, VolumeRenderingOptionsAction(nullptr)
{
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumeRenderingPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyVolumeRenderingPlugin);

  this->ToggleVolumeRenderingAction = new QAction("Show in 3D views as volume rendering",q);
  QObject::connect(this->ToggleVolumeRenderingAction, SIGNAL(toggled(bool)), q, SLOT(toggleVolumeRenderingForCurrentItem(bool)));
  this->ToggleVolumeRenderingAction->setCheckable(true);
  this->ToggleVolumeRenderingAction->setChecked(false);

  this->VolumeRenderingOptionsAction = new QAction("Volume rendering options...",q);
  this->VolumeRenderingOptionsAction->setToolTip(qSlicerSubjectHierarchyVolumeRenderingPlugin::tr("Switch to Volume Rendering "
                                                  "module to manage display options"));
  QObject::connect(this->VolumeRenderingOptionsAction, SIGNAL(triggered()), q, SLOT(showVolumeRenderingOptionsForCurrentItem()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumeRenderingPluginPrivate::~qSlicerSubjectHierarchyVolumeRenderingPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyVolumeRenderingPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumeRenderingPlugin::qSlicerSubjectHierarchyVolumeRenderingPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyVolumeRenderingPluginPrivate(*this) )
{
  this->m_Name = QString("VolumeRendering");

  Q_D(qSlicerSubjectHierarchyVolumeRenderingPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumeRenderingPlugin::~qSlicerSubjectHierarchyVolumeRenderingPlugin() = default;

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumeRenderingPlugin::setVolumeRenderingLogic(vtkSlicerVolumeRenderingLogic* volumeRenderingLogic)
{
  Q_D(qSlicerSubjectHierarchyVolumeRenderingPlugin);
  d->VolumeRenderingLogic = volumeRenderingLogic;
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyVolumeRenderingPlugin::visibilityContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyVolumeRenderingPlugin);

  QList<QAction*> actions;
  actions << d->ToggleVolumeRenderingAction << d->VolumeRenderingOptionsAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumeRenderingPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyVolumeRenderingPlugin);

  if (!itemID)
    {
    // There are no scene actions in this plugin
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Volume
  if (qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Volumes")->canOwnSubjectHierarchyItem(itemID))
    {
    vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    vtkMRMLVolumeRenderingDisplayNode* displayNode = nullptr;
    if (!volumeNode)
      {
      qCritical() << Q_FUNC_INFO << ": Failed to find scalar volume node associated to subject hierarchy item " << itemID;
      return;
      }
    if (d->VolumeRenderingLogic)
      {
      displayNode = d->VolumeRenderingLogic->GetFirstVolumeRenderingDisplayNode(volumeNode);
      }
    else
      {
      qWarning() << Q_FUNC_INFO << ": volume rendering logic is not set, cannot set up toggle volume rendering action";
      }

    d->ToggleVolumeRenderingAction->blockSignals(true);
    d->ToggleVolumeRenderingAction->setChecked(displayNode ? displayNode->GetVisibility() : false);
    d->ToggleVolumeRenderingAction->blockSignals(false);
    d->ToggleVolumeRenderingAction->setVisible(true);

    d->VolumeRenderingOptionsAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumeRenderingPlugin::toggleVolumeRenderingForCurrentItem(bool on)
{
  Q_D(qSlicerSubjectHierarchyVolumeRenderingPlugin);
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }
  // Whenever the volume is shown/hidden, volume rendering should be shown as requested by this method.
  this->showVolumeRendering(on, currentItemID, nullptr);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumeRenderingPlugin::resetFieldOfView(vtkMRMLDisplayNode* displayNode, vtkMRMLViewNode* viewNode/*=nullptr*/)
{
  Q_D(qSlicerSubjectHierarchyVolumeRenderingPlugin);
  vtkMRMLDisplayableNode* volumeNode = displayNode->GetDisplayableNode();
  double rasBounds[6] = { 0.0 };
  volumeNode->GetRASBounds(rasBounds);
  double cameraFocalPoint[3] =
    {
    (rasBounds[0] + rasBounds[1]) / 2.0,
    (rasBounds[2] + rasBounds[3]) / 2.0,
    (rasBounds[4] + rasBounds[5]) / 2.0,
    };

  // Get list of view nodes that will have their FOV reset
  QList<vtkMRMLViewNode*> viewNodes;
  if (viewNode)
    {
    // Specific view is provided - reset FOV in that single view
    viewNodes << viewNode;
    }
  else
    {
    // FOV reset in all views is requested - do it in all views where the volume is visible in
    qMRMLLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
    if (!layoutManager)
      {
      qCritical() << Q_FUNC_INFO << " failed: invalid layout manager";
      return;
      }
    for (int i = 0; i < layoutManager->threeDViewCount(); i++)
      {
      qMRMLThreeDWidget* threeDWidget = layoutManager->threeDWidget(i);
      if (!threeDWidget)
        {
        continue;
        }
      vtkMRMLViewNode* currentViewNode = threeDWidget->mrmlViewNode();
      if (!currentViewNode)
        {
        continue;
        }
      if (!displayNode->IsDisplayableInView(currentViewNode->GetID()))
        {
        continue;
        }
      viewNodes << currentViewNode;
      }
    }

  vtkSlicerApplicationLogic* appLogic = qSlicerApplication::application()->applicationLogic();
  if (!appLogic)
    {
    qCritical() << Q_FUNC_INFO << " failed: cannot get application logic";
    return;
    }
  foreach(vtkMRMLViewNode* currentViewNode, viewNodes)
    {
    // Show the volume in slice view
    vtkMRMLViewLogic* viewLogic = appLogic->GetViewLogic(currentViewNode);
    if (!viewLogic)
      {
      qCritical() << Q_FUNC_INFO << " failed: cannot get slice logic";
      continue;
      }
    vtkMRMLCameraNode* cameraNode = viewLogic->GetCameraNode();
    if (!cameraNode)
      {
      continue;
      }
    cameraNode->SetFocalPoint(cameraFocalPoint);
    cameraNode->ResetClippingRange();
    }
}


//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchyVolumeRenderingPlugin::showVolumeRendering(bool show, vtkIdType itemID, vtkMRMLViewNode* viewNode/*=nullptr*/)
{
  Q_D(qSlicerSubjectHierarchyVolumeRenderingPlugin);

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
    }
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!volumeNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find scalar volume node associated to subject hierarchy item " << itemID;
    return false;
    }
  if (!d->VolumeRenderingLogic)
    {
    qWarning() << Q_FUNC_INFO << ": volume rendering logic is not set, cannot set up toggle volume rendering action";
    return false;
    }
  bool wasVisible = false;
  vtkMRMLVolumeRenderingDisplayNode* displayNode = d->VolumeRenderingLogic->GetFirstVolumeRenderingDisplayNode(volumeNode);
  if (displayNode)
    {
    wasVisible = displayNode->GetVisibility();
    }
  else
    {
    // there is no volume rendering display node
    if (!show)
      {
      // not visible and should not be visible, so we are done
      return true;
      }
    displayNode = d->VolumeRenderingLogic->CreateDefaultVolumeRenderingNodes(volumeNode);
    }
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to create volume rendering display node for scalar volume node " << volumeNode->GetName();
    return false;
    }

  // Prevent volume rendering from show up again in 3D view when eye icon is clicked,
  // after the volume rendering checkbox was unchecked.
  displayNode->SetShowMode(show ? vtkMRMLDisplayNode::ShowDefault : vtkMRMLDisplayNode::ShowIgnore);

  if (viewNode)
    {
    // Show/hide in specific view
    MRMLNodeModifyBlocker blocker(displayNode);
    if (show)
      {
      // show
      if (!wasVisible)
        {
        displayNode->SetVisibility(true);
        // This was hidden in all views, show it only in the currently selected view
        displayNode->RemoveAllViewNodeIDs();
        }
      displayNode->AddViewNodeID(viewNode->GetID());
      }
    else
      {
      // This hides the volume rendering in all views, which is a bit more than asked for,
      // but since drag-and-drop to view only requires selective showing (and not selective hiding),
      // this should be good enough. The behavior can be refined later if needed.
      displayNode->SetVisibility(false);
      }
    }
  else
    {
    // Show in all views
    MRMLNodeModifyBlocker blocker(displayNode);
    displayNode->RemoveAllViewNodeIDs();
    displayNode->SetVisibility(show);
    }

  if (show)
    {
    QSettings settings;
    bool resetFieldOfView = settings.value("SubjectHierarchy/ResetFieldOfViewOnShowVolume", true).toBool();
    if (resetFieldOfView)
      {
      this->resetFieldOfView(displayNode, viewNode);
      }
    }

  return true;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumeRenderingPlugin::showVolumeRenderingOptionsForCurrentItem()
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (!currentItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }

  qSlicerAbstractModuleWidget* moduleWidget = qSlicerSubjectHierarchyAbstractPlugin::switchToModule("VolumeRendering");
  if (moduleWidget)
    {
    // Get node selector combobox
    qMRMLNodeComboBox* nodeSelector = moduleWidget->findChild<qMRMLNodeComboBox*>("VolumeNodeComboBox");

    // Choose current data node
    if (nodeSelector)
      {
      nodeSelector->setCurrentNode(shNode->GetItemDataNode(currentItemID));
      }
    }
}

//-----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyVolumeRenderingPlugin::showItemInView(vtkIdType itemID, vtkMRMLAbstractViewNode* viewNode, vtkIdList* allItemsToShow)
{
  vtkMRMLViewNode* threeDViewNode = vtkMRMLViewNode::SafeDownCast(viewNode);
  if (threeDViewNode)
    {
    return this->showVolumeRendering(true, itemID, threeDViewNode);
    }
  else
    {
    // Use volume's module implementation for displaying volume in slice views
    qSlicerSubjectHierarchyVolumesPlugin* volumesPlugin = qobject_cast<qSlicerSubjectHierarchyVolumesPlugin*>(
      qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Volumes"));
    if (!volumesPlugin)
      {
      qCritical() << Q_FUNC_INFO << ": Failed to access Volumes subject hierarchy plugin";
      return false;
      }
    return volumesPlugin->showItemInView(itemID, viewNode, allItemsToShow);
    }
}
