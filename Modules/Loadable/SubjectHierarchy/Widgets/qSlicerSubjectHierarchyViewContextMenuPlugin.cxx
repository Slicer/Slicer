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
#include <vtkMRMLScene.h>

// Slicer includes
#include <qSlicerApplication.h>
#include <qSlicerLayoutManager.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// CTK includes
#include "ctkSignalMapper.h"
#include "ctkVTKWidgetsUtils.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyViewContextMenuPluginPrivate: public QObject
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

  QAction* CopyImageAction = nullptr;

  vtkWeakPointer<vtkMRMLInteractionNode> InteractionNode;
  vtkWeakPointer<vtkMRMLAbstractViewNode> ViewNode;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyViewContextMenuPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyViewContextMenuPluginPrivate::qSlicerSubjectHierarchyViewContextMenuPluginPrivate(qSlicerSubjectHierarchyViewContextMenuPlugin& object)
: q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyViewContextMenuPlugin);

  // Interaction mode

  this->InteractionModeViewTransformAction = new QAction("View transform",q);
  this->InteractionModeViewTransformAction->setObjectName("MouseModeViewTransformAction");
  this->InteractionModeViewTransformAction->setCheckable(true);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->InteractionModeViewTransformAction,
    qSlicerSubjectHierarchyAbstractPlugin::SectionInteraction, 0);

  this->InteractionModeAdjustWindowLevelAction = new QAction("Adjust window/level",q);
  this->InteractionModeAdjustWindowLevelAction->setObjectName("MouseModeAdjustWindowLevelAction");
  this->InteractionModeAdjustWindowLevelAction->setCheckable(true);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->InteractionModeAdjustWindowLevelAction,
    qSlicerSubjectHierarchyAbstractPlugin::SectionInteraction, 1);

  this->InteractionModePlaceAction = new QAction("Place", q);
  this->InteractionModePlaceAction->setObjectName("MouseModePlaceAction");
  this->InteractionModePlaceAction->setCheckable(true);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->InteractionModePlaceAction,
    qSlicerSubjectHierarchyAbstractPlugin::SectionInteraction, 2);

  QActionGroup* interactionModeActions = new QActionGroup(q);
  interactionModeActions->setExclusive(true);

  interactionModeActions->addAction(this->InteractionModeViewTransformAction);
  interactionModeActions->addAction(this->InteractionModeAdjustWindowLevelAction);
  interactionModeActions->addAction(this->InteractionModePlaceAction);

  this->InteractionModeMapper = new ctkSignalMapper(q);
  this->InteractionModeMapper->setMapping(this->InteractionModeViewTransformAction, vtkMRMLInteractionNode::ViewTransform);
  this->InteractionModeMapper->setMapping(this->InteractionModeAdjustWindowLevelAction, vtkMRMLInteractionNode::AdjustWindowLevel);
  this->InteractionModeMapper->setMapping(this->InteractionModePlaceAction, vtkMRMLInteractionNode::Place);
  QObject::connect(interactionModeActions, SIGNAL(triggered(QAction*)), this->InteractionModeMapper, SLOT(map(QAction*)));
  QObject::connect(this->InteractionModeMapper, SIGNAL(mapped(int)), q, SLOT(setInteractionMode(int)));

  // Other

  this->CopyImageAction = new QAction(tr("Copy image"), q);
  this->CopyImageAction->setObjectName("CopyImageAction");
  this->CopyImageAction->setToolTip(tr("Copy a screenshot of this view to the clipboard"));
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->CopyImageAction,
    qSlicerSubjectHierarchyAbstractPlugin::SectionDefault, 0);

  QObject::connect(this->CopyImageAction, SIGNAL(triggered()), q, SLOT(saveScreenshot()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyViewContextMenuPluginPrivate::~qSlicerSubjectHierarchyViewContextMenuPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyViewContextMenuPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyViewContextMenuPlugin::qSlicerSubjectHierarchyViewContextMenuPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyViewContextMenuPluginPrivate(*this) )
{
  this->m_Name = QString("ViewContextMenu");

  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyViewContextMenuPlugin::~qSlicerSubjectHierarchyViewContextMenuPlugin() = default;

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyViewContextMenuPlugin::viewContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyViewContextMenuPlugin);
  QList<QAction*> actions;
  actions << d->InteractionModeViewTransformAction
    << d->InteractionModeAdjustWindowLevelAction
    << d->InteractionModePlaceAction
    << d->CopyImageAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::showViewContextMenuActionsForItem(vtkIdType itemID, QVariantMap eventData)
{
  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);

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

  d->CopyImageAction->setVisible(true);

  // Cache nodes to have them available for the menu action execution.
  d->InteractionNode = interactionNode;
  d->ViewNode = viewNode;
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
  if (!qSlicerApplication::application()
    || !qSlicerApplication::application()->layoutManager())
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
