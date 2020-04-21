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

// Qt includes
#include <QActionGroup>
#include <QDebug>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QHBoxLayout>

// VTK includes
#include <vtkStringArray.h>

// CTK includes
#include <ctkLogger.h>
#include <ctkPopupWidget.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLNodeFactory.h"
#include "qMRMLSceneViewMenu.h"
#include "qMRMLTableView.h"
#include "qMRMLTableViewControllerWidget_p.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLDoubleArrayNode.h>
#include <vtkMRMLTableViewNode.h>
#include <vtkMRMLTableNode.h>
#include <vtkMRMLSceneViewNode.h>

// STD include
#include <string>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLTableViewControllerWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLTableViewControllerWidgetPrivate methods

//---------------------------------------------------------------------------
qMRMLTableViewControllerWidgetPrivate::qMRMLTableViewControllerWidgetPrivate(
  qMRMLTableViewControllerWidget& object)
  : Superclass(object)
{
  this->TableNode = nullptr;
  this->TableViewNode = nullptr;
  this->TableView = nullptr;
  this->CopyAction = nullptr;
  this->PasteAction = nullptr;
  this->PlotAction = nullptr;
}

//---------------------------------------------------------------------------
qMRMLTableViewControllerWidgetPrivate::~qMRMLTableViewControllerWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qMRMLTableViewControllerWidgetPrivate::setupPopupUi()
{
  Q_Q(qMRMLTableViewControllerWidget);

  this->Superclass::setupPopupUi();
  this->PopupWidget->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
  this->Ui_qMRMLTableViewControllerWidget::setupUi(this->PopupWidget);

  // Create shortcuts for copy/paste
  this->CopyAction = new QAction(this);
  this->CopyAction->setIcon(QIcon(":Icons/Medium/SlicerEditCopy.png"));
  this->CopyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  // set CTRL+C shortcut
  this->CopyAction->setShortcuts(QKeySequence::Copy);
  this->CopyAction->setToolTip(tr("Copy"));
  q->addAction(this->CopyAction);
  this->PasteAction = new QAction(this);
  this->PasteAction->setIcon(QIcon(":Icons/Medium/SlicerEditPaste.png"));
  this->PasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  // set CTRL+V shortcut
  this->PasteAction->setShortcuts(QKeySequence::Paste);
  this->PasteAction->setToolTip(tr("Paste"));
  q->addAction(this->PasteAction);
  this->PlotAction = new QAction(this);
  this->PlotAction->setIcon(QIcon(":Icons/Medium/SlicerInteractivePlotting.png"));
  this->PlotAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  // set CTRL+P shortcut
  this->PlotAction->setShortcuts(QKeySequence::Print);
  this->PlotAction->setToolTip(tr("Generate an Interactive Plot based on user-selection"
                               " of the columns of the table."));
  q->addAction(this->PlotAction);

  // Connect Table selector
  this->connect(this->tableComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                SLOT(onTableNodeSelected(vtkMRMLNode*)));

  this->connect(this->LockTableButton, SIGNAL(clicked()), SLOT(onLockTableButtonClicked()));

  this->connect(this->ColumnInsertButton, SIGNAL(clicked()), SLOT(insertColumn()));
  this->connect(this->ColumnDeleteButton, SIGNAL(clicked()), SLOT(deleteColumn()));
  this->connect(this->RowInsertButton, SIGNAL(clicked()), SLOT(insertRow()));
  this->connect(this->RowDeleteButton, SIGNAL(clicked()), SLOT(deleteRow()));
  this->connect(this->LockFirstRowButton, SIGNAL(toggled(bool)), SLOT(setFirstRowLocked(bool)));
  this->connect(this->LockFirstColumnButton, SIGNAL(toggled(bool)), SLOT(setFirstColumnLocked(bool)));

  // Connect copy and paste actions
  this->CopyButton->setDefaultAction(this->CopyAction);
  this->connect(this->CopyAction, SIGNAL(triggered()), SLOT(copySelection()));
  this->PasteButton->setDefaultAction(this->PasteAction);
  this->connect(this->PasteAction, SIGNAL(triggered()), SLOT(pasteSelection()));
  this->PlotButton->setDefaultAction(this->PlotAction);
  this->connect(this->PlotAction, SIGNAL(triggered()), SLOT(plotSelection()));

  // Connect the scene
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->tableComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));

  onTableNodeSelected(nullptr);
}

//---------------------------------------------------------------------------
void qMRMLTableViewControllerWidgetPrivate::init()
{
  this->Superclass::init();
  this->ViewLabel->setText(qMRMLTableViewControllerWidget::tr("1"));
  this->BarLayout->addStretch(1);
  this->setColor(QColor("#e1ba3c"));
}

// --------------------------------------------------------------------------
void qMRMLTableViewControllerWidgetPrivate::onTableNodeSelected(vtkMRMLNode * node)
{
  Q_Q(qMRMLTableViewControllerWidget);

  if (!this->TableViewNode)
    {
    return;
    }

  if (this->TableNode.GetPointer() == node)
    {
    return;
    }

  this->qvtkReconnect(this->TableNode, node, vtkCommand::ModifiedEvent,
                      q, SLOT(updateWidgetFromMRML()));
  this->TableNode = node;

  this->TableViewNode->SetTableNodeID(this->TableNode ? this->TableNode->GetID() : nullptr);

  q->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLTableViewControllerWidgetPrivate::onLockTableButtonClicked()
{
  if (!this->TableNode)
    {
    qWarning("qMRMLTableViewControllerWidgetPrivate::onLockTableButtonClicked failed: tableNode is invalid");
    return;
    }

  // toggle the lock
  int locked = this->TableNode->GetLocked();
  this->TableNode->SetLocked(!locked);
}

//-----------------------------------------------------------------------------
void qMRMLTableViewControllerWidgetPrivate::insertColumn()
{
  if (!this->TableView)
    {
    qWarning("qMRMLTableViewControllerWidgetPrivate::insertColumn failed: TableView is invalid");
    return;
    }
  this->TableView->insertColumn();
}

//-----------------------------------------------------------------------------
void qMRMLTableViewControllerWidgetPrivate::deleteColumn()
{
  if (!this->TableView)
    {
    qWarning("qMRMLTableViewControllerWidgetPrivate::deleteColumn failed: TableView is invalid");
    return;
    }
  this->TableView->deleteColumn();
}

//-----------------------------------------------------------------------------
void qMRMLTableViewControllerWidgetPrivate::insertRow()
{
  if (!this->TableView)
    {
    qWarning("qMRMLTableViewControllerWidgetPrivate::insertRow failed: TableView is invalid");
    return;
    }
  this->TableView->insertRow();
}

//-----------------------------------------------------------------------------
void qMRMLTableViewControllerWidgetPrivate::deleteRow()
{
  if (!this->TableView)
    {
    qWarning("qMRMLTableViewControllerWidgetPrivate::deleteRow failed: TableView is invalid");
    return;
    }
  this->TableView->deleteRow();
}

//-----------------------------------------------------------------------------
void qMRMLTableViewControllerWidgetPrivate::setFirstRowLocked(bool locked)
{
  if (!this->TableView)
    {
    qWarning("qMRMLTableViewControllerWidgetPrivate::setFirstRowLocked failed: TableView is invalid");
    return;
    }
  this->TableView->setFirstRowLocked(locked);
}

//-----------------------------------------------------------------------------
void qMRMLTableViewControllerWidgetPrivate::setFirstColumnLocked(bool locked)
{
  if (!this->TableView)
    {
    qWarning("qMRMLTableViewControllerWidgetPrivate::setFirstColumnLocked failed: TableView is invalid");
    return;
    }
  this->TableView->setFirstColumnLocked(locked);
}

//-----------------------------------------------------------------------------
void qMRMLTableViewControllerWidgetPrivate::copySelection()
{
  if (!this->TableView)
    {
    qWarning("qMRMLTableViewControllerWidgetPrivate::copySelection failed: TableView is invalid");
    return;
    }
  this->TableView->copySelection();
}

//-----------------------------------------------------------------------------
void qMRMLTableViewControllerWidgetPrivate::pasteSelection()
{
  if (!this->TableView)
    {
    qWarning("qMRMLTableViewControllerWidgetPrivate::pasteSelection failed: TableView is invalid");
    return;
    }
  this->TableView->pasteSelection();
}

//-----------------------------------------------------------------------------
void qMRMLTableViewControllerWidgetPrivate::plotSelection()
{
  if (!this->TableView)
    {
    qWarning("qMRMLTableViewControllerWidgetPrivate::plotSelection failed: TableView is invalid");
    return;
    }
  this->TableView->plotSelection();
}

// --------------------------------------------------------------------------
// qMRMLTableViewControllerWidget methods

// --------------------------------------------------------------------------
qMRMLTableViewControllerWidget::qMRMLTableViewControllerWidget(QWidget* parentWidget)
  : Superclass(new qMRMLTableViewControllerWidgetPrivate(*this), parentWidget)
{
  Q_D(qMRMLTableViewControllerWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLTableViewControllerWidget::~qMRMLTableViewControllerWidget()
{
  this->setMRMLScene(nullptr);
}

// --------------------------------------------------------------------------
void qMRMLTableViewControllerWidget::setTableView(qMRMLTableView* view)
{
  Q_D(qMRMLTableViewControllerWidget);

  d->TableView = view;
}

//---------------------------------------------------------------------------
void qMRMLTableViewControllerWidget::setViewLabel(const QString& newViewLabel)
{
  Q_D(qMRMLTableViewControllerWidget);

  if (d->TableViewNode)
    {
    logger.error("setViewLabel should be called before setViewNode !");
    return;
    }

  d->TableViewLabel = newViewLabel;
  d->ViewLabel->setText(d->TableViewLabel);
}

//---------------------------------------------------------------------------
CTK_GET_CPP(qMRMLTableViewControllerWidget, QString, viewLabel, TableViewLabel);


// --------------------------------------------------------------------------
void qMRMLTableViewControllerWidget::setMRMLTableViewNode(
    vtkMRMLTableViewNode * viewNode)
{
  Q_D(qMRMLTableViewControllerWidget);
  this->qvtkReconnect(d->TableViewNode, viewNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromMRML()));
  d->TableViewNode = viewNode;
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLTableViewControllerWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLTableViewControllerWidget);

  //qDebug() << "qMRMLTableViewControllerWidget::updateWidgetFromMRML()";

  if (!d->TableViewNode || !this->mrmlScene())
    {
    return;
    }

  vtkMRMLTableNode *tableNode
    = vtkMRMLTableNode::SafeDownCast(this->mrmlScene()->GetNodeByID(d->TableViewNode->GetTableNodeID()));

  // TableNode selector
  d->tableComboBox->setCurrentNodeID(tableNode ? tableNode->GetID() : nullptr);

  bool validNode = tableNode != nullptr;
  bool editableNode = tableNode != nullptr && !tableNode->GetLocked();

  d->LockTableButton->setEnabled(validNode);
  d->CopyButton->setEnabled(validNode);
  d->PasteButton->setEnabled(editableNode);
  d->EditControlsFrame->setEnabled(editableNode);

  if (!tableNode)
    {
    return;
    }

  if (tableNode->GetLocked())
    {
    d->LockTableButton->setIcon(QIcon(":Icons/Medium/SlicerLock.png"));
    d->LockTableButton->setToolTip(QString("Click to unlock this table so that values can be modified"));
    }
  else
    {
    d->LockTableButton->setIcon(QIcon(":Icons/Medium/SlicerUnlock.png"));
    d->LockTableButton->setToolTip(QString("Click to lock this table to prevent modification of the values in the user interface"));
    }

  if (tableNode->GetUseColumnNameAsColumnHeader() != d->LockFirstRowButton->isChecked())
    {
    bool wasBlocked = d->LockFirstRowButton->blockSignals(true);
    d->LockFirstRowButton->setChecked(tableNode->GetUseColumnNameAsColumnHeader());
    d->LockFirstRowButton->blockSignals(wasBlocked);
    }

  if (tableNode->GetUseFirstColumnAsRowHeader() != d->LockFirstColumnButton->isChecked())
    {
    bool wasBlocked = d->LockFirstColumnButton->blockSignals(true);
    d->LockFirstColumnButton->setChecked(tableNode->GetUseFirstColumnAsRowHeader());
    d->LockFirstColumnButton->blockSignals(wasBlocked);
    }
}

// --------------------------------------------------------------------------
void qMRMLTableViewControllerWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLTableViewControllerWidget);

  if (this->mrmlScene() == newScene)
    {
    return;
    }

   d->qvtkReconnect(this->mrmlScene(), newScene, vtkMRMLScene::EndBatchProcessEvent,
                    this, SLOT(updateWidgetFromMRML()));

  // Disable the node selectors as they would fire signal currentIndexChanged(0)
  // meaning that there is no current node anymore. It's not true, it just means
  // that the current node was not in the combo box list menu before
  bool tableBlockSignals = d->tableComboBox->blockSignals(true);
  //bool arrayBlockSignals = d->arrayComboBox->blockSignals(true);

  this->Superclass::setMRMLScene(newScene);

  d->tableComboBox->blockSignals(tableBlockSignals);
  //d->arrayComboBox->blockSignals(arrayBlockSignals);

  if (this->mrmlScene())
    {
    this->updateWidgetFromMRML();
    }
}
