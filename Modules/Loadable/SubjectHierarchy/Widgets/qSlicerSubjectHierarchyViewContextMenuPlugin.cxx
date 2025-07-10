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

==============================================================================*/

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "vtkMRMLSubjectHierarchyNode.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyViewContextMenuPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// Qt includes
#include <QAction>
#include <QClipboard>
#include <QDebug>
#include <QVariant>

// MRML includes
#include <qMRMLSliceView.h>
#include <qMRMLSliceWidget.h>
#include <qMRMLThreeDView.h>
#include <qMRMLThreeDWidget.h>
#include <vtkMRMLAbstractViewNode.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <qMRMLThreeDViewControllerWidget.h>

// Slicer includes
#include <qSlicerApplication.h>
#include <vtkSlicerApplicationLogic.h>
#include <qSlicerLayoutManager.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkMRMLCameraDisplayableManager.h>
#include <vtkMRMLCameraWidget.h>

// CTK includes
#include "ctkSignalMapper.h"
#include "ctkVTKWidgetsUtils.h"

//-----------------------------------------------------------------------------
class qSlicerSubjectHierarchyViewContextMenuPluginPrivate : public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyViewContextMenuPlugin);

protected:
  qSlicerSubjectHierarchyViewContextMenuPlugin* const q_ptr;

public:
  qSlicerSubjectHierarchyViewContextMenuPluginPrivate(qSlicerSubjectHierarchyViewContextMenuPlugin& object);
  ~qSlicerSubjectHierarchyViewContextMenuPluginPrivate() override;
  void init();

public:
  ctkSignalMapper* InteractionModeMapper;
  QAction* InteractionModeViewTransformAction = nullptr;
  QAction* InteractionModeAdjustWindowLevelAction = nullptr;
  QAction* InteractionModePlaceAction = nullptr;

  QAction* MaximizeViewAction = nullptr;
  QAction* FitSliceViewAction = nullptr;
  QAction* RefocusAllCamerasAction = nullptr;
  QAction* CenterThreeDViewAction = nullptr;
  QAction* RefocusCameraAction = nullptr;
  QAction* CopyImageAction = nullptr;
  QAction* ConfigureSliceViewAnnotationsAction = nullptr;
  QAction* ToggleTiltLockAction = nullptr;

  QAction* IntersectingSlicesVisibilityAction = nullptr;
  QAction* IntersectingSlicesInteractiveAction = nullptr;

  QAction* EnableSlabReconstructionAction = nullptr;
  QAction* SlabReconstructionInteractiveAction = nullptr;

  vtkWeakPointer<vtkMRMLInteractionNode> InteractionNode;
  vtkWeakPointer<vtkMRMLAbstractViewNode> ViewNode;
  vtkWeakPointer<vtkMRMLLayoutNode> LayoutNode;
  vtkWeakPointer<vtkMRMLCameraWidget> CameraWidget;

  QVariantMap ViewContextMenuEventData;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyViewContextMenuPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyViewContextMenuPluginPrivate::qSlicerSubjectHierarchyViewContextMenuPluginPrivate(
  qSlicerSubjectHierarchyViewContextMenuPlugin& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyViewContextMenuPlugin);

  // Interaction mode

  this->InteractionModeViewTransformAction =
    new QAction(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("View transform"), q);
  this->InteractionModeViewTransformAction->setObjectName("MouseModeViewTransformAction");
  this->InteractionModeViewTransformAction->setCheckable(true);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(
    this->InteractionModeViewTransformAction, qSlicerSubjectHierarchyAbstractPlugin::SectionInteraction, 0);

  this->InteractionModeAdjustWindowLevelAction =
    new QAction(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Adjust window/level"), q);
  this->InteractionModeAdjustWindowLevelAction->setObjectName("MouseModeAdjustWindowLevelAction");
  this->InteractionModeAdjustWindowLevelAction->setCheckable(true);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(
    this->InteractionModeAdjustWindowLevelAction, qSlicerSubjectHierarchyAbstractPlugin::SectionInteraction, 1);

  this->InteractionModePlaceAction = new QAction(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Place"), q);
  this->InteractionModePlaceAction->setObjectName("MouseModePlaceAction");
  this->InteractionModePlaceAction->setCheckable(true);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(
    this->InteractionModePlaceAction, qSlicerSubjectHierarchyAbstractPlugin::SectionInteraction, 2);

  QActionGroup* interactionModeActions = new QActionGroup(q);
  interactionModeActions->setExclusive(true);

  interactionModeActions->addAction(this->InteractionModeViewTransformAction);
  interactionModeActions->addAction(this->InteractionModeAdjustWindowLevelAction);
  interactionModeActions->addAction(this->InteractionModePlaceAction);

  this->InteractionModeMapper = new ctkSignalMapper(q);
  this->InteractionModeMapper->setMapping(this->InteractionModeViewTransformAction,
                                          vtkMRMLInteractionNode::ViewTransform);
  this->InteractionModeMapper->setMapping(this->InteractionModeAdjustWindowLevelAction,
                                          vtkMRMLInteractionNode::AdjustWindowLevel);
  this->InteractionModeMapper->setMapping(this->InteractionModePlaceAction, vtkMRMLInteractionNode::Place);
  QObject::connect(
    interactionModeActions, SIGNAL(triggered(QAction*)), this->InteractionModeMapper, SLOT(map(QAction*)));
  QObject::connect(this->InteractionModeMapper, SIGNAL(mapped(int)), q, SLOT(setInteractionMode(int)));

  // Other

  this->CenterThreeDViewAction = new QAction(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Center view"), q);
  this->CenterThreeDViewAction->setObjectName("CenterViewAction");
  this->CenterThreeDViewAction->setToolTip(
    qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Center the slice on "
                                                     "the currently visible 3D view content and all loaded volumes."));
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(
    this->CenterThreeDViewAction, qSlicerSubjectHierarchyAbstractPlugin::SectionDefault, 0);
  QObject::connect(this->CenterThreeDViewAction, SIGNAL(triggered()), q, SLOT(centerThreeDView()));

  this->RefocusCameraAction =
    new QAction(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Refocus camera on this point"), q);
  this->RefocusCameraAction->setObjectName("RefocusCameraAction");
  this->RefocusCameraAction->setToolTip(
    qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Set the camera focus to rotate around this point."));
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(
    this->RefocusCameraAction, qSlicerSubjectHierarchyAbstractPlugin::SectionDefault, 0);
  QObject::connect(this->RefocusCameraAction, SIGNAL(triggered()), q, SLOT(refocusCamera()));

  this->FitSliceViewAction = new QAction(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Reset field of view"), q);
  this->FitSliceViewAction->setObjectName("FitViewAction");
  this->FitSliceViewAction->setToolTip(
    qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Center the slice view on the currently displayed volume."));
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(
    this->FitSliceViewAction, qSlicerSubjectHierarchyAbstractPlugin::SectionDefault, 1);
  QObject::connect(this->FitSliceViewAction, SIGNAL(triggered()), q, SLOT(fitSliceView()));

  this->RefocusAllCamerasAction =
    new QAction(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Refocus cameras on this point"), q);
  this->RefocusAllCamerasAction->setObjectName("RefocusAllCamerasAction");
  this->RefocusAllCamerasAction->setToolTip(
    qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Set the focus of all cameras to rotate around this point."));
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(
    this->RefocusAllCamerasAction, qSlicerSubjectHierarchyAbstractPlugin::SectionDefault, 2);
  QObject::connect(this->RefocusAllCamerasAction, SIGNAL(triggered()), q, SLOT(refocusAllCameras()));

  this->MaximizeViewAction = new QAction(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Maximize view"), q);
  this->MaximizeViewAction->setObjectName("MaximizeViewAction");
  this->MaximizeViewAction->setToolTip(
    qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Show this view maximized in the view layout"));
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(
    this->MaximizeViewAction, qSlicerSubjectHierarchyAbstractPlugin::SectionDefault, 3);
  QObject::connect(this->MaximizeViewAction, SIGNAL(triggered()), q, SLOT(maximizeView()));

  this->ToggleTiltLockAction = new QAction(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Tilt lock"), q);
  this->ToggleTiltLockAction->setObjectName("TiltLockAction");
  this->ToggleTiltLockAction->setToolTip(
    qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Prevent rotation "
                                                     "around the horizontal axis when rotating this view."));
  this->ToggleTiltLockAction->setShortcut(QKeySequence(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Ctrl+b")));
  this->ToggleTiltLockAction->setCheckable(true);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(
    this->ToggleTiltLockAction, qSlicerSubjectHierarchyAbstractPlugin::SectionDefault, 4);
  QObject::connect(this->ToggleTiltLockAction, SIGNAL(triggered()), q, SLOT(toggleTiltLock()));

  this->ConfigureSliceViewAnnotationsAction =
    new QAction(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Configure slice view annotations..."), q);
  this->ConfigureSliceViewAnnotationsAction->setObjectName("ConfigureSliceViewAnnotationsAction");
  this->ConfigureSliceViewAnnotationsAction->setToolTip(
    qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Configures display of corner annotations"
                                                     " and color legend."));
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(
    this->ConfigureSliceViewAnnotationsAction, qSlicerSubjectHierarchyAbstractPlugin::SectionDefault, 5);
  QObject::connect(
    this->ConfigureSliceViewAnnotationsAction, SIGNAL(triggered()), q, SLOT(configureSliceViewAnnotationsAction()));

  this->CopyImageAction = new QAction(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Copy image"), q);
  this->CopyImageAction->setObjectName("CopyImageAction");
  this->CopyImageAction->setToolTip(
    qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Copy a screenshot of this view to the clipboard"));
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(
    this->CopyImageAction,
    qSlicerSubjectHierarchyAbstractPlugin::SectionDefault,
    20); // set to 20 to make it the last item in the action group
  QObject::connect(this->CopyImageAction, SIGNAL(triggered()), q, SLOT(saveScreenshot()));

  int sliceSection = qSlicerSubjectHierarchyAbstractPlugin::SectionDefault
                     + 5; // set section to +5 to allow placing other sections above

  // Slice intersections
  this->IntersectingSlicesVisibilityAction =
    new QAction(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Slice intersections"), q);
  this->IntersectingSlicesVisibilityAction->setObjectName("IntersectingSlicesAction");
  this->IntersectingSlicesVisibilityAction->setToolTip(
    qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Show how the "
                                                     "other slice planes intersect each slice plane."));
  this->IntersectingSlicesVisibilityAction->setCheckable(true);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->IntersectingSlicesVisibilityAction, sliceSection);
  QObject::connect(
    this->IntersectingSlicesVisibilityAction, SIGNAL(triggered(bool)), q, SLOT(setIntersectingSlicesVisible(bool)));

  // Interactive slice intersections
  this->IntersectingSlicesInteractiveAction =
    new QAction(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Interaction"), q);
  this->IntersectingSlicesInteractiveAction->setObjectName("IntersectingSlicesHandlesAction");
  this->IntersectingSlicesInteractiveAction->setToolTip(
    qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Show handles for slice interaction."));
  this->IntersectingSlicesInteractiveAction->setCheckable(true);
  this->IntersectingSlicesInteractiveAction->setEnabled(false);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->IntersectingSlicesInteractiveAction, sliceSection, 1);
  QObject::connect(this->IntersectingSlicesInteractiveAction,
                   SIGNAL(triggered(bool)),
                   q,
                   SLOT(setIntersectingSlicesHandlesVisible(bool)));

  int thickSlabSection = sliceSection + 5; // set section to +5 to allow placing other sections above

  // Thick slab reconstruction
  this->EnableSlabReconstructionAction =
    new QAction(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Thick slab reconstruction"), q);
  this->EnableSlabReconstructionAction->setObjectName("EnableSlabReconstructionAction");
  this->EnableSlabReconstructionAction->setToolTip(
    qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Enable Thick Slab Reconstruction (TSR). "
                                                     "TSR is used to merge contiguous slices within a certain range."));
  this->EnableSlabReconstructionAction->setCheckable(true);
  this->EnableSlabReconstructionAction->setEnabled(false);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->EnableSlabReconstructionAction, thickSlabSection);
  QObject::connect(
    this->EnableSlabReconstructionAction, SIGNAL(triggered(bool)), q, SLOT(setSlabReconstructionEnabled(bool)));

  this->SlabReconstructionInteractiveAction =
    new QAction(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Interactive"), q);
  this->SlabReconstructionInteractiveAction->setObjectName("SlabReconstructionInteractiveAction");
  this->SlabReconstructionInteractiveAction->setToolTip(qSlicerSubjectHierarchyViewContextMenuPlugin::tr(
    "Show handles for interactively adjusting slab reconstruction thickness."));
  this->SlabReconstructionInteractiveAction->setCheckable(true);
  this->SlabReconstructionInteractiveAction->setEnabled(false);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(
    this->SlabReconstructionInteractiveAction, thickSlabSection, 1);
  QObject::connect(this->SlabReconstructionInteractiveAction,
                   SIGNAL(triggered(bool)),
                   q,
                   SLOT(setSlabReconstructionInteractive(bool)));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyViewContextMenuPluginPrivate::~qSlicerSubjectHierarchyViewContextMenuPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyViewContextMenuPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyViewContextMenuPlugin::qSlicerSubjectHierarchyViewContextMenuPlugin(QObject* parent)
  : Superclass(parent)
  , d_ptr(new qSlicerSubjectHierarchyViewContextMenuPluginPrivate(*this))
{
  this->m_Name = QString("ViewContextMenu");

  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyViewContextMenuPlugin::~qSlicerSubjectHierarchyViewContextMenuPlugin() = default;

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyViewContextMenuPlugin::viewContextMenuActions() const
{
  Q_D(const qSlicerSubjectHierarchyViewContextMenuPlugin);
  QList<QAction*> actions;
  actions << d->InteractionModeViewTransformAction << d->InteractionModeAdjustWindowLevelAction
          << d->InteractionModePlaceAction << d->MaximizeViewAction << d->FitSliceViewAction
          << d->RefocusAllCamerasAction << d->CenterThreeDViewAction << d->RefocusCameraAction << d->CopyImageAction
          << d->ToggleTiltLockAction << d->ConfigureSliceViewAnnotationsAction << d->IntersectingSlicesVisibilityAction
          << d->IntersectingSlicesInteractiveAction << d->EnableSlabReconstructionAction
          << d->SlabReconstructionInteractiveAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::showViewContextMenuActionsForItem(vtkIdType itemID,
                                                                                     QVariantMap eventData)
{
  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);
  // make sure we don't use metadata from some previous view context menu calls
  d->ViewContextMenuEventData.clear();

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode || !shNode->GetScene())
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }
  if (itemID != shNode->GetSceneItemID())
  {
    return;
  }
  if (!eventData.contains("ViewNodeID"))
  {
    return;
  }
  vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(
    shNode->GetScene()->GetNodeByID(eventData["ViewNodeID"].toString().toStdString()));
  if (!viewNode)
  {
    return;
  }
  vtkMRMLInteractionNode* interactionNode = viewNode->GetInteractionNode();
  if (!interactionNode)
  {
    return;
  }

  d->ViewContextMenuEventData = eventData;

  d->InteractionModeViewTransformAction->setVisible(true);
  d->InteractionModeAdjustWindowLevelAction->setVisible(true);
  d->InteractionModePlaceAction->setVisible(true);

  int interactionMode = interactionNode->GetCurrentInteractionMode();

  bool wasBlocked = d->InteractionModeViewTransformAction->blockSignals(true);
  d->InteractionModeViewTransformAction->setChecked(interactionMode == vtkMRMLInteractionNode::ViewTransform);
  d->InteractionModeViewTransformAction->blockSignals(wasBlocked);

  wasBlocked = d->InteractionModeAdjustWindowLevelAction->blockSignals(true);
  d->InteractionModeAdjustWindowLevelAction->setChecked(interactionMode == vtkMRMLInteractionNode::AdjustWindowLevel);
  d->InteractionModeAdjustWindowLevelAction->blockSignals(wasBlocked);

  wasBlocked = d->InteractionModePlaceAction->blockSignals(true);
  d->InteractionModePlaceAction->setChecked(interactionMode == vtkMRMLInteractionNode::Place);
  d->InteractionModePlaceAction->blockSignals(wasBlocked);

  // Update view/restore view action
  bool isMaximized = false;
  bool canBeMaximized = false;
  d->LayoutNode = viewNode->GetMaximizedState(isMaximized, canBeMaximized);
  d->MaximizeViewAction->setVisible(canBeMaximized);
  if (canBeMaximized)
  {
    d->MaximizeViewAction->setProperty("maximize", QVariant(!isMaximized));
    if (isMaximized)
    {
      d->MaximizeViewAction->setText(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Restore view layout"));
    }
    else
    {
      d->MaximizeViewAction->setText(qSlicerSubjectHierarchyViewContextMenuPlugin::tr("Maximize view"));
    }
  }

  d->CopyImageAction->setVisible(true);

  // Cache nodes to have them available for the menu action execution.
  d->InteractionNode = interactionNode;
  d->ViewNode = viewNode;

  // Check tilt lock in camera widget and set menu item accordingly
  vtkMRMLSliceNode* sliceViewNode = vtkMRMLSliceNode::SafeDownCast(d->ViewNode);
  bool isSliceViewNode = (sliceViewNode != nullptr);
  d->ConfigureSliceViewAnnotationsAction->setVisible(isSliceViewNode);
  d->FitSliceViewAction->setVisible(isSliceViewNode);
  d->RefocusAllCamerasAction->setVisible(isSliceViewNode);
  d->CenterThreeDViewAction->setVisible(!isSliceViewNode);
  d->RefocusCameraAction->setVisible(!isSliceViewNode);

  vtkSlicerApplicationLogic* appLogic = qSlicerApplication::application()->applicationLogic();
  if (isSliceViewNode && appLogic)
  {
    d->IntersectingSlicesVisibilityAction->setVisible(true);
    d->IntersectingSlicesVisibilityAction->setEnabled(true);
    d->IntersectingSlicesVisibilityAction->setChecked(
      appLogic->GetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesVisibility));

    d->IntersectingSlicesInteractiveAction->setVisible(true);
    d->IntersectingSlicesInteractiveAction->setEnabled(d->IntersectingSlicesVisibilityAction->isChecked());
    d->IntersectingSlicesInteractiveAction->setChecked(
      appLogic->GetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesInteractive));
  }
  else
  {
    d->IntersectingSlicesVisibilityAction->setVisible(false);
    d->IntersectingSlicesInteractiveAction->setVisible(false);
  }

  if (sliceViewNode && appLogic)
  {
    d->EnableSlabReconstructionAction->setVisible(true);
    d->EnableSlabReconstructionAction->setEnabled(true);
    d->EnableSlabReconstructionAction->setChecked(sliceViewNode->GetSlabReconstructionEnabled());

    d->SlabReconstructionInteractiveAction->setVisible(true);
    d->SlabReconstructionInteractiveAction->setEnabled(d->EnableSlabReconstructionAction->isChecked());
    d->SlabReconstructionInteractiveAction->setChecked(
      appLogic->GetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesThickSlabInteractive));
  }
  else
  {
    d->EnableSlabReconstructionAction->setVisible(false);
    d->SlabReconstructionInteractiveAction->setVisible(false);
  }

  d->ToggleTiltLockAction->setVisible(!isSliceViewNode);
  if (!qSlicerApplication::application() || !qSlicerApplication::application()->layoutManager())
  {
    qWarning() << Q_FUNC_INFO << " failed: cannot get layout manager";
    return;
  }
  QWidget* widget = qSlicerApplication::application()->layoutManager()->viewWidget(d->ViewNode);

  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(widget);
  vtkMRMLCameraWidget* cameraWidget = nullptr;
  if (threeDWidget)
  {
    vtkMRMLCameraDisplayableManager* cameraDisplayableManager = vtkMRMLCameraDisplayableManager::SafeDownCast(
      threeDWidget->threeDView()->displayableManagerByClassName("vtkMRMLCameraDisplayableManager"));
    if (!cameraDisplayableManager)
    {
      qWarning() << Q_FUNC_INFO << " failed: cannot get cameraDisplayableManager";
      return;
    }
    else
    {
      cameraWidget = cameraDisplayableManager->GetCameraWidget();
      d->ToggleTiltLockAction->setChecked(cameraWidget->GetTiltLocked());
      // Cache camera widget pointer to have it available for the menu action execution.
      d->CameraWidget = cameraWidget;
    }
  }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::setInteractionMode(int mode)
{
  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);
  if (!d->InteractionNode)
  {
    qWarning() << Q_FUNC_INFO << " failed: invalid interaction node";
    return;
  }
  d->InteractionNode->SetCurrentInteractionMode(mode);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::saveScreenshot()
{
  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);
  if (!qSlicerApplication::application() || !qSlicerApplication::application()->layoutManager())
  {
    qWarning() << Q_FUNC_INFO << " failed: cannot get layout manager";
    return;
  }
  QWidget* widget = qSlicerApplication::application()->layoutManager()->viewWidget(d->ViewNode);

  // Get the inside of the widget (without the view controller bar)
  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(widget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(widget);
  if (sliceWidget)
  {
    widget = sliceWidget->sliceView();
  }
  else if (threeDWidget)
  {
    widget = threeDWidget->threeDView();
  }
  if (!widget)
  {
    qWarning() << Q_FUNC_INFO << " failed: cannot get view widget from layout manager";
    return;
  }

  // Grab image
  QImage screenshot = ctk::grabVTKWidget(widget);

  // Copy to clipboard
  QClipboard* clipboard = QApplication::clipboard();
  if (!clipboard)
  {
    qWarning() << Q_FUNC_INFO << " failed: cannot access the clipboard";
    return;
  }
  clipboard->setImage(screenshot);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::configureSliceViewAnnotationsAction()
{
  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);
  qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
  {
    return;
  }
  layoutManager->setCurrentModule("DataProbe");
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::maximizeView()
{
  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);
  if (!d->LayoutNode)
  {
    return;
  }
  bool maximizeView = d->MaximizeViewAction->property("maximize").toBool();
  if (maximizeView)
  {
    d->LayoutNode->AddMaximizedViewNode(d->ViewNode);
  }
  else
  {
    d->LayoutNode->RemoveMaximizedViewNode(d->ViewNode);
  }
}
//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::fitSliceView()
{
  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);

  if (!qSlicerApplication::application() || !qSlicerApplication::application()->layoutManager())
  {
    qWarning() << Q_FUNC_INFO << " failed: cannot get layout manager";
    return;
  }
  QWidget* widget = qSlicerApplication::application()->layoutManager()->viewWidget(d->ViewNode);

  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(widget);
  if (sliceWidget)
  {
    sliceWidget->fitSliceToBackground();
  }
  else
  {
    qWarning() << Q_FUNC_INFO << " failed: sliceWidget not found";
    return;
  }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::centerThreeDView()
{
  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);

  if (!qSlicerApplication::application() || !qSlicerApplication::application()->layoutManager())
  {
    qWarning() << Q_FUNC_INFO << " failed: cannot get layout manager";
    return;
  }
  QWidget* widget = qSlicerApplication::application()->layoutManager()->viewWidget(d->ViewNode);

  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(widget);
  if (threeDWidget)
  {
    qMRMLThreeDViewControllerWidget* threeDWidgetController = threeDWidget->threeDController();
    threeDWidgetController->resetFocalPoint();
  }
  else
  {
    qWarning() << Q_FUNC_INFO << " failed: threeDWidget not found";
    return;
  }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::refocusCamera()
{
  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);
  if (!d->CameraWidget)
  {
    qWarning() << Q_FUNC_INFO << " failed: camera widget not found.";
    return;
  }
  vtkMRMLCameraNode* cameraNode = d->CameraWidget->GetCameraNode();
  if (!cameraNode)
  {
    qWarning() << Q_FUNC_INFO << " failed: camera not found.";
    return;
  }
  if (d->ViewContextMenuEventData.find("WorldPosition") == d->ViewContextMenuEventData.end())
  {
    qCritical() << Q_FUNC_INFO << ": Failed to get world position";
    return;
  }
  QVariantList worldPosVector = d->ViewContextMenuEventData["WorldPosition"].toList();
  if (worldPosVector.size() != 3)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid world position";
    return;
  }
  double worldPos[3] = { worldPosVector[0].toDouble(), worldPosVector[1].toDouble(), worldPosVector[2].toDouble() };
  cameraNode->SetFocalPoint(worldPos);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::refocusAllCameras()
{
  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode || !shNode->GetScene())
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }
  if (d->ViewContextMenuEventData.find("WorldPosition") == d->ViewContextMenuEventData.end())
  {
    qCritical() << Q_FUNC_INFO << ": Failed to get world position";
    return;
  }
  QVariantList worldPosVector = d->ViewContextMenuEventData["WorldPosition"].toList();
  if (worldPosVector.size() != 3)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid world position";
    return;
  }
  double worldPos[3] = { worldPosVector[0].toDouble(), worldPosVector[1].toDouble(), worldPosVector[2].toDouble() };

  std::vector<vtkMRMLNode*> cameraNodes;
  shNode->GetScene()->GetNodesByClass("vtkMRMLCameraNode", cameraNodes);
  for (unsigned int i = 0; i < cameraNodes.size(); ++i)
  {
    vtkMRMLCameraNode* cameraNode = vtkMRMLCameraNode::SafeDownCast(cameraNodes[i]);
    if (!cameraNode)
    {
      continue;
    }
    cameraNode->SetFocalPoint(worldPos);
  }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::toggleTiltLock()
{
  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);
  if (!d->CameraWidget)
  {
    qWarning() << Q_FUNC_INFO << " failed: camera widget not found.";
    return;
  }
  d->CameraWidget->SetTiltLocked(!d->CameraWidget->GetTiltLocked());
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::setIntersectingSlicesVisible(bool visible)
{
  vtkSlicerApplicationLogic* appLogic = qSlicerApplication::application()->applicationLogic();
  if (!appLogic)
  {
    qCritical() << Q_FUNC_INFO << " failed: cannot get application logic";
    return;
  }
  appLogic->SetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesVisibility, visible);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::setIntersectingSlicesHandlesVisible(bool interaction)
{
  vtkSlicerApplicationLogic* appLogic = qSlicerApplication::application()->applicationLogic();
  if (!appLogic)
  {
    qCritical() << Q_FUNC_INFO << " failed: cannot get application logic";
    return;
  }
  appLogic->SetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesInteractive, interaction);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::setSlabReconstructionEnabled(bool enabled)
{
  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);
  vtkMRMLSliceNode* sliceViewNode = vtkMRMLSliceNode::SafeDownCast(d->ViewNode);
  if (!sliceViewNode)
  {
    qCritical() << Q_FUNC_INFO << " failed: cannot get slide node";
    return;
  }
  sliceViewNode->SetSlabReconstructionEnabled(enabled);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::setSlabReconstructionInteractive(bool interactive)
{
  vtkSlicerApplicationLogic* appLogic = qSlicerApplication::application()->applicationLogic();
  if (!appLogic)
  {
    qCritical() << Q_FUNC_INFO << " failed: cannot get application logic";
    return;
  }
  appLogic->SetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesThickSlabInteractive, interactive);
}
