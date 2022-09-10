/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QToolButton>
#include <QMenu>
#include <QCheckBox>
#include <QSignalMapper>
#include <QSplitter>
#include <QShortcut>
#include <QKeySequence>

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"

// MRML includes
#include "qMRMLMarkupsToolBar_p.h"
#include "qMRMLNodeComboBox.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"
#include "qMRMLSliceView.h"
#include "qMRMLSliceWidget.h"
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerModuleManager.h"

#include <vtkMRMLScene.h>
#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLWindowLevelWidget.h>
#include <qSlicerMarkupsPlaceWidget.h>
#include <vtkMRMLDisplayNode.h>

// SlicerLogic includes
#include <vtkSlicerApplicationLogic.h>
// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkSlicerMarkupsLogic.h>

// VTK includes
#include <vtkWeakPointer.h>
#include <vtkSmartPointer.h>

//---------------------------------------------------------------------------
qMRMLMarkupsToolBarPrivate::qMRMLMarkupsToolBarPrivate(qMRMLMarkupsToolBar& object)
  : q_ptr(&object)
{
  this->DefaultPlaceClassName = "vtkMRMLMarkupsFiducialNode";
}

//---------------------------------------------------------------------------
void qMRMLMarkupsToolBarPrivate::init()
{
  Q_Q(qMRMLMarkupsToolBar);

  // set up keyboard shortcuts
  q->addCreateNodeShortcut("Ctrl+Shift+A");
  q->addTogglePersistenceShortcut("Ctrl+Shift+T");
  q->addPlacePointShortcut("Ctrl+Shift+Space");

  // Markups node selector
  this->MarkupsNodeSelector = new qMRMLNodeComboBox(q);
  this->MarkupsNodeSelector->setObjectName(QString("MarkupsNodeSelector"));
  this->MarkupsNodeSelector->setNodeTypes(QStringList(QString("vtkMRMLMarkupsNode")));
  this->MarkupsNodeSelector->setNoneEnabled(false);
  this->MarkupsNodeSelector->setAddEnabled(false);
  this->MarkupsNodeSelector->setRenameEnabled(true);
  this->MarkupsNodeSelector->setEditEnabled(true);
  this->MarkupsNodeSelector->setMaximumWidth(165);
  this->MarkupsNodeSelector->setEnabled(true);
  this->MarkupsNodeSelector->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  this->MarkupsNodeSelector->setToolTip("Select active markup");
  this->MarkupsNodeSelector->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  this->NodeSelectorAction = q->addWidget(this->MarkupsNodeSelector);

  connect(this->MarkupsNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, SIGNAL(activeMarkupsNodeChanged(vtkMRMLNode*)));
  connect(this->MarkupsNodeSelector, SIGNAL(nodeActivated(vtkMRMLNode*)), q, SLOT(onMarkupsNodeChanged(vtkMRMLNode*)));

  // Get scene and application logic
  q->setApplicationLogic(qSlicerApplication::application()->applicationLogic());
  q->setMRMLScene(qSlicerApplication::application()->mrmlScene());
}

// --------------------------------------------------------------------------
void qMRMLMarkupsToolBarPrivate::addSetModuleButton(vtkSlicerMarkupsLogic* markupsLogic, const QString& moduleName)
{
  Q_UNUSED(markupsLogic);
  Q_Q(qMRMLMarkupsToolBar);

  QPushButton* moduleButton = new QPushButton();
  moduleButton->setObjectName(QString(moduleName + " module shortcut"));
  moduleButton->setToolTip("Open the " + moduleName + " module");
  QString iconName = ":/Icons/" + moduleName + ".png";
  moduleButton->setIcon(QIcon(iconName));
  QSignalMapper* mapper = new QSignalMapper(moduleButton);
  QObject::connect(moduleButton, SIGNAL(clicked()), mapper, SLOT(map()));
  mapper->setMapping(moduleButton, moduleName);
  QObject::connect(mapper, SIGNAL(mapped(const QString&)),
    this, SLOT(onSetModule(const QString&)));
  q->addWidget(moduleButton);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsToolBarPrivate::onSetModule(const QString& moduleName)
{
  qSlicerModuleManager* moduleManager = qSlicerCoreApplication::application()->moduleManager();
  if (!moduleManager)
    {
    return;
    }
  qSlicerAbstractCoreModule* module = moduleManager->module(moduleName);
  if (!module)
    {
    return;
    }
  qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
    {
    return;
    }
  layoutManager->setCurrentModule(moduleName);
}

// --------------------------------------------------------------------------
void qMRMLMarkupsToolBarPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_Q(qMRMLMarkupsToolBar);

  if (newScene == this->MRMLScene)
    {
    return;
    }

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::StartBatchProcessEvent,
    this, SLOT(onMRMLSceneStartBatchProcess()));

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::EndBatchProcessEvent,
    this, SLOT(onMRMLSceneEndBatchProcess()));

  this->MRMLScene = newScene;
  this->MarkupsNodeSelector->setMRMLScene(newScene);
  if (this->MarkupsPlaceWidget)
    {
    this->MarkupsPlaceWidget->setMRMLScene(newScene);
    }

  // watch for changes to the interaction, selection nodes so can update the widget
  q->setInteractionNode((this->MRMLAppLogic && this->MRMLScene) ? this->MRMLAppLogic->GetInteractionNode() : nullptr);
  q->setSelectionNode((this->MRMLAppLogic && this->MRMLScene) ? this->MRMLAppLogic->GetSelectionNode() : nullptr);

  vtkMRMLSelectionNode* selectionNode =
    (this->MRMLAppLogic && this->MRMLScene) ?
    this->MRMLAppLogic->GetSelectionNode() : nullptr;
  this->qvtkReconnect(selectionNode, vtkMRMLSelectionNode::ActivePlaceNodeClassNameChangedEvent,
    this, SLOT(updateWidgetFromMRML()));
  this->qvtkReconnect(selectionNode, vtkMRMLSelectionNode::PlaceNodeClassNameListModifiedEvent,
    this, SLOT(updateWidgetFromMRML()));

  // Update UI
  q->setEnabled(this->MRMLScene != nullptr);
  this->updateWidgetFromMRML();

}

//---------------------------------------------------------------------------
void qMRMLMarkupsToolBarPrivate::onMRMLSceneStartBatchProcess()
{
  Q_Q(qMRMLMarkupsToolBar);
  q->setEnabled(false);
}

//---------------------------------------------------------------------------
void qMRMLMarkupsToolBarPrivate::onMRMLSceneEndBatchProcess()
{
  Q_Q(qMRMLMarkupsToolBar);

  // re-enable in case it didn't get re-enabled for scene load
  q->setEnabled(true);

  q->setInteractionNode((this->MRMLAppLogic && this->MRMLScene) ? this->MRMLAppLogic->GetInteractionNode() : nullptr);
  q->setSelectionNode((this->MRMLAppLogic && this->MRMLScene) ? this->MRMLAppLogic->GetSelectionNode() : nullptr);

  // update the state from mrml
  this->updateWidgetFromMRML();
}


//---------------------------------------------------------------------------
QCursor qMRMLMarkupsToolBarPrivate::cursorFromIcon(QIcon& icon)
{
  QList<QSize> availableSizes = icon.availableSizes();
  if (availableSizes.size() > 0)
    {
    return QCursor(icon.pixmap(availableSizes[0]));
    }
  else
    {
    // use a default
    return QCursor(icon.pixmap(20));
    }
}

//---------------------------------------------------------------------------
void qMRMLMarkupsToolBarPrivate::updateWidgetFromMRML()
{
  Q_Q(qMRMLMarkupsToolBar);
  vtkMRMLInteractionNode* interactionNode = q->interactionNode();
  if (!interactionNode)
    {
    qDebug() << "Markups ToolBar: no interaction node";
    q->setEnabled(false);
    return;
    }
  vtkMRMLSelectionNode* selectionNode = q->selectionNode();
  if (!selectionNode)
    {
    q->setEnabled(false);
    return;
    }

  q->setEnabled(true);

  // Update active markups node
  vtkMRMLMarkupsNode* activeMarkupsNode = nullptr;
  if (selectionNode->GetScene())
    {
    activeMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(
      selectionNode->GetScene()->GetNodeByID(selectionNode->GetActivePlaceNodeID()));
    }
  // do not block signals so that signals are emitted
  this->MarkupsNodeSelector->setCurrentNode(activeMarkupsNode);

  if (this->MarkupsPlaceWidget)
    {
    // do not block signals so that activeMarkupsPlaceModeChanged signals are emitted
    this->MarkupsPlaceWidget->setEnabled(true);
    this->MarkupsPlaceWidget->setInteractionNode(interactionNode);
    this->MarkupsPlaceWidget->setSelectionNode(selectionNode);
    this->MarkupsPlaceWidget->setCurrentNode(activeMarkupsNode);
    }
}
//---------------------------------------------------------------------------
void qMRMLMarkupsToolBarPrivate::onActivePlaceNodeClassNameChangedEvent()
{
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qMRMLMarkupsToolBarPrivate::onPlaceNodeClassNameListModifiedEvent()
{
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
// qMRMLMarkupsToolBar methods

// --------------------------------------------------------------------------
qMRMLMarkupsToolBar::qMRMLMarkupsToolBar(const QString& title, QWidget* parentWidget)
  :Superclass(title, parentWidget)
   , d_ptr(new qMRMLMarkupsToolBarPrivate(*this))
{
  Q_D(qMRMLMarkupsToolBar);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLMarkupsToolBar::qMRMLMarkupsToolBar(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qMRMLMarkupsToolBarPrivate(*this))
{
  Q_D(qMRMLMarkupsToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qMRMLMarkupsToolBar::~qMRMLMarkupsToolBar() = default;

// --------------------------------------------------------------------------
void qMRMLMarkupsToolBar::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLMarkupsToolBar);
  d->setMRMLScene(scene);
}

// --------------------------------------------------------------------------
vtkMRMLMarkupsNode* qMRMLMarkupsToolBar::activeMarkupsNode()
{
  Q_D(qMRMLMarkupsToolBar);
  return vtkMRMLMarkupsNode::SafeDownCast(d->MarkupsNodeSelector->currentNode());
}

// --------------------------------------------------------------------------
void qMRMLMarkupsToolBar::setActiveMarkupsNode(vtkMRMLMarkupsNode* newActiveNode)
{
  Q_D(qMRMLMarkupsToolBar);
  vtkMRMLSelectionNode* selectionNode = (d->MRMLAppLogic && d->MRMLScene) ?
    d->MRMLAppLogic->GetSelectionNode() : nullptr;
  if (selectionNode == nullptr && newActiveNode != nullptr)
    {
    qWarning() << Q_FUNC_INFO << " failed: invalid selection node";
    return;
    }
  selectionNode->SetActivePlaceNodeID(newActiveNode ? newActiveNode->GetID() : nullptr);
  // the GUI will be updated via MRML node observations
}

//-----------------------------------------------------------------------------
vtkMRMLInteractionNode* qMRMLMarkupsToolBar::interactionNode()const
{
  Q_D(const qMRMLMarkupsToolBar);
  return d->InteractionNode;
}

//-----------------------------------------------------------------------------
vtkMRMLSelectionNode* qMRMLMarkupsToolBar::selectionNode()const
{
  Q_D(const qMRMLMarkupsToolBar);
  return d->SelectionNode;
}

//---------------------------------------------------------------------------
void qMRMLMarkupsToolBar::setPersistence(bool persistent)
{
  Q_D(qMRMLMarkupsToolBar);
  if (d->MarkupsPlaceWidget)
    {
    d->MarkupsPlaceWidget->setPlaceModePersistency(persistent ? true : false);
    }
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsToolBar::setInteractionNode(vtkMRMLInteractionNode* interactionNode)
{
  Q_D(qMRMLMarkupsToolBar);
  if (d->InteractionNode == interactionNode)
    {
    return;
    }
  d->qvtkReconnect(d->InteractionNode, interactionNode, vtkCommand::ModifiedEvent,
    d, SLOT(updateWidgetFromMRML()));
  d->InteractionNode = interactionNode;
  d->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsToolBar::setSelectionNode(vtkMRMLSelectionNode* selectionNode)
{
  Q_D(qMRMLMarkupsToolBar);

  if (d->SelectionNode == selectionNode)
    {
    return;
    }
  d->qvtkReconnect(d->SelectionNode, selectionNode, vtkCommand::ModifiedEvent,
    d, SLOT(updateWidgetFromMRML()));
  d->SelectionNode = selectionNode;
  d->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsToolBar::interactionModeActionTriggered(bool toggled)
{
  Q_D(qMRMLMarkupsToolBar);
  if (!toggled)
    {
    return;
    }
  QAction* sourceAction = qobject_cast<QAction*>(sender());
  if (!sourceAction)
    {
    return;
    }
  int selectedInteractionMode = sourceAction->data().toInt();
  if (!d->InteractionNode)
    {
    return;
    }
  d->InteractionNode->SetCurrentInteractionMode(selectedInteractionMode);

  // If no active place node class name is selected then use the default class
  if (d->InteractionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
    {
    vtkMRMLSelectionNode* selectionNode = (d->MRMLAppLogic && d->MRMLScene) ?
      d->MRMLAppLogic->GetSelectionNode() : nullptr;
    if (selectionNode)
      {
      const char* currentPlaceNodeClassName = selectionNode->GetActivePlaceNodeClassName();
      if (!currentPlaceNodeClassName || strlen(currentPlaceNodeClassName) == 0)
        {
        selectionNode->SetReferenceActivePlaceNodeClassName(d->DefaultPlaceClassName.toUtf8());
        }
      }
  }
}

//---------------------------------------------------------------------------
void qMRMLMarkupsToolBar::setApplicationLogic(vtkSlicerApplicationLogic* appLogic)
{
  Q_D(qMRMLMarkupsToolBar);
  d->MRMLAppLogic = appLogic;
}

//---------------------------------------------------------------------------
void qMRMLMarkupsToolBar::initializeToolBarLayout()
{
  Q_D(qMRMLMarkupsToolBar);

  vtkSlicerMarkupsLogic* markupsLogic =
    vtkSlicerMarkupsLogic::SafeDownCast(d->MRMLAppLogic->GetModuleLogic("Markups"));
  if (!markupsLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: invalid markups logic";
    return;
    }
  // Markups place widget
  d->MarkupsPlaceWidget = new qSlicerMarkupsPlaceWidget(this);
  d->MarkupsPlaceWidget->setObjectName(QString("MarkupsPlaceWidget"));
  d->MarkupsPlaceWidget->setDeleteAllControlPointsOptionVisible(true);
  d->MarkupsPlaceWidget->setPlaceMultipleMarkups(qSlicerMarkupsPlaceWidget::ShowPlaceMultipleMarkupsOption);
  d->MarkupsPlaceWidget->setMRMLScene(qSlicerApplication::application()->mrmlScene());
  this->addWidget(d->MarkupsPlaceWidget);
  connect(d->MarkupsPlaceWidget, SIGNAL(activeMarkupsPlaceModeChanged(bool)), this, SIGNAL(activeMarkupsPlaceModeChanged(bool)));

  // Module shortcuts
  this->addSeparator();
  d->addSetModuleButton(markupsLogic, "Markups");

  // Add event observers for registration/unregistration of markups
  this->qvtkConnect(markupsLogic, vtkSlicerMarkupsLogic::MarkupRegistered,
    this, SLOT(updateToolBarLayout()));
  this->qvtkConnect(markupsLogic, vtkSlicerMarkupsLogic::MarkupUnregistered,
    this, SLOT(updateToolBarLayout()));

  this->updateToolBarLayout();
}

//---------------------------------------------------------------------------
void qMRMLMarkupsToolBar::updateToolBarLayout()
{
  // Node creation buttons
  Q_D(qMRMLMarkupsToolBar);

  vtkSlicerMarkupsLogic* markupsLogic =
    vtkSlicerMarkupsLogic::SafeDownCast(d->MRMLAppLogic->GetModuleLogic("Markups"));
  if (!markupsLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: invalid markups logic";
    return;
    }

  for (const auto markupName : markupsLogic->GetRegisteredMarkupsTypes())
    {
    vtkMRMLMarkupsNode* markupsNode = markupsLogic->GetNodeByMarkupsType(markupName.c_str());
    if (markupsNode && markupsLogic->GetCreateMarkupsPushButton(markupName.c_str()))
      {
      bool buttonExists = false;
      for (int index=0; index< this->layout()->count(); index++)
        {
        std::string buttonName = this->layout()->itemAt(index)->widget()->objectName().toStdString();
        if (buttonName == "Create" + markupName + "PushButton")
          {
          buttonExists = true;
          break;
          }
        }
      if (!buttonExists)
        {
        QPushButton* markupCreateButton = new QPushButton();
        QSignalMapper* mapper = new QSignalMapper(markupCreateButton);
        std::string markupType = markupsNode->GetMarkupType() ? markupsNode->GetMarkupType() : "";
        std::string markupDisplayName = markupsNode->GetTypeDisplayName() ? markupsNode->GetTypeDisplayName() : "";
        markupCreateButton->setObjectName(QString::fromStdString("Create"+markupType+"PushButton"));
        markupCreateButton->setToolTip("Create new " + QString::fromStdString(markupDisplayName));
        markupCreateButton->setIcon(QIcon(markupsNode->GetPlaceAddIcon()));
        markupCreateButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        this->insertWidget(d->NodeSelectorAction, markupCreateButton);
        QObject::connect(markupCreateButton, SIGNAL(clicked()), mapper, SLOT(map()));
        mapper->setMapping(markupCreateButton, markupsNode->GetClassName());
        QObject::connect(mapper, SIGNAL(mapped(const QString&)), this, SLOT(onAddNewMarkupsNodeByClass(const QString&)));
        }
      }
    }

  for (int index = this->layout()->count()-1; index >=0 ; index--)
    {
    QString buttonName = this->layout()->itemAt(index)->widget()->objectName();
    if (!buttonName.startsWith("Create") || !buttonName.endsWith("PushButton"))
      {
      // Not a markup create button, leave it as is
      continue;
      }
    bool markupExists = false;
    for (const auto markupName : markupsLogic->GetRegisteredMarkupsTypes())
      {
      //QString markupButtonName = QString("Create%1PushButton").arg(QString::fromStdString(markupName));
      if (QString::fromStdString("Create"+markupName+"PushButton") == buttonName)
        {
        markupExists = true;
        break;
        }
      }
    if (markupExists)
      {
      // This button is still needed
      continue;
      }
    // Corresponding markup type is no longer available, delete this button
    QLayoutItem* item = this->layout()->takeAt(index);
    delete item->widget();
    delete item;
    }
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsToolBar::onAddNewMarkupsNodeByClass(const QString& className)
{
  Q_D(qMRMLMarkupsToolBar);
  // Add new markups node to the scene
  vtkMRMLMarkupsNode* markupsNode = nullptr;
  vtkSlicerMarkupsLogic* markupsLogic =
    vtkSlicerMarkupsLogic::SafeDownCast(d->MRMLAppLogic->GetModuleLogic("Markups"));
  if (markupsLogic)
    {
    markupsNode = markupsLogic->AddNewMarkupsNode(className.toStdString());
    }
  if (!markupsNode)
    {
    qCritical() << Q_FUNC_INFO << ": failed to create new markups node by class " << className;
    return;
    }
  // Update GUI
  d->updateWidgetFromMRML();
  if (d->MarkupsPlaceWidget)
    {
    d->MarkupsPlaceWidget->setPlaceModeEnabled(true);
    }
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsToolBar::onMarkupsNodeChanged(vtkMRMLNode* markupsNode)
{
  // called when the user selects a node on the toolbar
  Q_D(qMRMLMarkupsToolBar);
  this->setActiveMarkupsNode(vtkMRMLMarkupsNode::SafeDownCast(markupsNode));
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsToolBar::addCreateNodeShortcut(QString keySequence)
{
  QObject::connect(new QShortcut(QKeySequence(keySequence), this), SIGNAL(activated()), SLOT(onCreateNodeShortcut()));
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsToolBar::addTogglePersistenceShortcut(QString keySequence)
{
  QObject::connect(new QShortcut(QKeySequence(keySequence), this), SIGNAL(activated()), SLOT(onTogglePersistenceShortcut()));
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsToolBar::addPlacePointShortcut(QString keySequence)
{
  QObject::connect(new QShortcut(QKeySequence(keySequence), this), SIGNAL(activated()), SLOT(onPlacePointShortcut()));
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsToolBar::onCreateNodeShortcut()
{
  Q_D(qMRMLMarkupsToolBar);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->MarkupsNodeSelector->currentNode());
  QString className = d->DefaultPlaceClassName;
  if (currentMarkupsNode != nullptr && d->MarkupsPlaceWidget)
    {
    className = d->MarkupsPlaceWidget->currentNode()->GetClassName();
    }
  this->onAddNewMarkupsNodeByClass(className);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsToolBar::onTogglePersistenceShortcut()
{
  Q_D(qMRMLMarkupsToolBar);
  if (!d->MarkupsPlaceWidget)
    {
    return;
    }
  bool persistent = d->MarkupsPlaceWidget->placeModePersistency();
  this->setPersistence(persistent ? false : true);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsToolBar::onPlacePointShortcut()
{
  Q_D(qMRMLMarkupsToolBar);
  if (!d->MarkupsPlaceWidget)
    {
    return;
    }
  bool placeModeActive = d->MarkupsPlaceWidget->placeModeEnabled();
  d->MarkupsPlaceWidget->setPlaceModeEnabled(placeModeActive ? false : true);
}
