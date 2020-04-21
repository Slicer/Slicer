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
#include <QAction>
#include <QFileDialog>
#include <QStringBuilder>

// C++ includes
#include <cmath>

// Slicer includes
#include "qSlicerTablesModuleWidget.h"
#include "ui_qSlicerTablesModuleWidget.h"

// vtkSlicerLogic includes
#include "vtkSlicerTablesLogic.h"

// MRMLWidgets includes
#include <qMRMLUtils.h>
#include <qMRMLTableModel.h>

// MRML includes
#include "vtkMRMLTableNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTable.h>

//-----------------------------------------------------------------------------
class qSlicerTablesModuleWidgetPrivate: public Ui_qSlicerTablesModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerTablesModuleWidget);
protected:
  qSlicerTablesModuleWidget* const q_ptr;
public:
  qSlicerTablesModuleWidgetPrivate(qSlicerTablesModuleWidget& object);
//  static QList<vtkSmartPointer<vtkMRMLTransformableNode> > getSelectedNodes(qMRMLTreeView* tree);

  vtkSlicerTablesLogic*      logic()const;
  vtkTable* table()const;

  vtkWeakPointer<vtkMRMLTableNode> MRMLTableNode;
  QAction*                      CopyAction;
  QAction*                      PasteAction;
  QAction*                      PlotAction;
};

//-----------------------------------------------------------------------------
qSlicerTablesModuleWidgetPrivate::qSlicerTablesModuleWidgetPrivate(qSlicerTablesModuleWidget& object)
  : q_ptr(&object)
{
  this->MRMLTableNode = nullptr;
  this->CopyAction = nullptr;
  this->PasteAction = nullptr;
  this->PlotAction = nullptr;
}
//-----------------------------------------------------------------------------
vtkSlicerTablesLogic* qSlicerTablesModuleWidgetPrivate::logic()const
{
  Q_Q(const qSlicerTablesModuleWidget);
  return vtkSlicerTablesLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
vtkTable* qSlicerTablesModuleWidgetPrivate::table()const
{
  if (this->MRMLTableNode.GetPointer()==nullptr)
    {
    return nullptr;
    }
  return this->MRMLTableNode->GetTable();
}

//-----------------------------------------------------------------------------
qSlicerTablesModuleWidget::qSlicerTablesModuleWidget(QWidget* _parentWidget)
  : Superclass(_parentWidget)
  , d_ptr(new qSlicerTablesModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerTablesModuleWidget::~qSlicerTablesModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerTablesModuleWidget::setup()
{
  Q_D(qSlicerTablesModuleWidget);
  d->setupUi(this);

  // Create shortcuts for copy/paste
  d->CopyAction = new QAction(this);
  d->CopyAction->setIcon(QIcon(":Icons/Medium/SlicerEditCopy.png"));
  d->CopyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  // set CTRL+C shortcut
  d->CopyAction->setShortcuts(QKeySequence::Copy);
  d->CopyAction->setToolTip(tr("Copy"));
  this->addAction(d->CopyAction);
  d->PasteAction = new QAction(this);
  d->PasteAction->setIcon(QIcon(":Icons/Medium/SlicerEditPaste.png"));
  d->PasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  // set CTRL+V shortcut
  d->PasteAction->setShortcuts(QKeySequence::Paste);
  d->PasteAction->setToolTip(tr("Paste"));
  this->addAction(d->PasteAction);
  d->PlotAction = new QAction(this);
  d->PlotAction->setIcon(QIcon(":Icons/Medium/SlicerInteractivePlotting.png"));
  d->PlotAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  // set CTRL+P shortcut
  d->PlotAction->setShortcuts(QKeySequence::Print);
  d->PlotAction->setToolTip(tr("Generate an Interactive Plot based on user-selection"
                               " of the columns of the table."));
  this->addAction(d->PlotAction);

  // Connect node selector with module itself
  this->connect(d->TableNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), SLOT(onNodeSelected(vtkMRMLNode*)));

  this->connect(d->LockTableButton, SIGNAL(clicked()), SLOT(onLockTableButtonClicked()));

  this->connect(d->ColumnInsertButton, SIGNAL(clicked()), d->TableView, SLOT(insertColumn()));
  this->connect(d->ColumnDeleteButton, SIGNAL(clicked()), d->TableView, SLOT(deleteColumn()));
  this->connect(d->RowInsertButton, SIGNAL(clicked()), d->TableView, SLOT(insertRow()));
  this->connect(d->RowDeleteButton, SIGNAL(clicked()), d->TableView, SLOT(deleteRow()));
  this->connect(d->LockFirstRowButton, SIGNAL(toggled(bool)), d->TableView, SLOT(setFirstRowLocked(bool)));
  this->connect(d->LockFirstColumnButton, SIGNAL(toggled(bool)), d->TableView, SLOT(setFirstColumnLocked(bool)));

  // Connect copy, paste and plot actions
  d->CopyButton->setDefaultAction(d->CopyAction);
  this->connect(d->CopyAction, SIGNAL(triggered()), d->TableView, SLOT(copySelection()));
  d->PasteButton->setDefaultAction(d->PasteAction);
  this->connect(d->PasteAction, SIGNAL(triggered()), d->TableView, SLOT(pasteSelection()));
  d->PlotButton->setDefaultAction(d->PlotAction);
  this->connect(d->PlotAction, SIGNAL(triggered()), d->TableView, SLOT(plotSelection()));

  d->SelectedColumnPropertiesWidget->setSelectionFromMRMLTableView(d->TableView);

  d->NewColumnPropertiesWidget->setMRMLTableColumnName(vtkMRMLTableNode::GetDefaultColumnName());
  this->connect(d->TableNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), d->NewColumnPropertiesWidget, SLOT(setMRMLTableNode(vtkMRMLNode*)));

  this->onNodeSelected(nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerTablesModuleWidget::onNodeSelected(vtkMRMLNode* node)
{
  Q_D(qSlicerTablesModuleWidget);
  vtkMRMLTableNode* tableNode = vtkMRMLTableNode::SafeDownCast(node);

  this->qvtkReconnect(d->MRMLTableNode, tableNode, vtkCommand::ModifiedEvent, this, SLOT(onMRMLTableNodeModified(vtkObject*)));
  d->MRMLTableNode = tableNode;

  // Update GUI from the newly selected node
  this->onMRMLTableNodeModified(d->MRMLTableNode);
}

//-----------------------------------------------------------------------------
void qSlicerTablesModuleWidget::onMRMLTableNodeModified(vtkObject* caller)
{
  Q_D(qSlicerTablesModuleWidget);

#ifndef QT_NO_DEBUG
  vtkMRMLTableNode* tableNode = vtkMRMLTableNode::SafeDownCast(caller);
  Q_ASSERT(d->MRMLTableNode == tableNode);
#else
  Q_UNUSED(caller);
#endif

  bool validNode = d->MRMLTableNode != nullptr;
  bool editableNode = d->MRMLTableNode != nullptr && !d->MRMLTableNode->GetLocked();

  d->DisplayEditCollapsibleWidget->setEnabled(validNode);
  d->LockTableButton->setEnabled(validNode);
  d->CopyButton->setEnabled(validNode);
  d->PasteButton->setEnabled(editableNode);
  d->EditControlsFrame->setEnabled(editableNode);

  if (!d->MRMLTableNode)
    {
    return;
    }

  if (d->MRMLTableNode->GetLocked())
    {
    d->LockTableButton->setIcon(QIcon(":Icons/Medium/SlicerLock.png"));
    d->LockTableButton->setToolTip(QString("Click to unlock this table so that values can be modified"));
    }
  else
    {
    d->LockTableButton->setIcon(QIcon(":Icons/Medium/SlicerUnlock.png"));
    d->LockTableButton->setToolTip(QString("Click to lock this table to prevent modification of the values in the user interface"));
    }

  if (d->MRMLTableNode->GetUseColumnNameAsColumnHeader() != d->LockFirstRowButton->isChecked())
    {
    bool wasBlocked = d->LockFirstRowButton->blockSignals(true);
    d->LockFirstRowButton->setChecked(d->MRMLTableNode->GetUseColumnNameAsColumnHeader());
    d->LockFirstRowButton->blockSignals(wasBlocked);
    }

  if (d->MRMLTableNode->GetUseFirstColumnAsRowHeader() != d->LockFirstColumnButton->isChecked())
    {
    bool wasBlocked = d->LockFirstColumnButton->blockSignals(true);
    d->LockFirstColumnButton->setChecked(d->MRMLTableNode->GetUseFirstColumnAsRowHeader());
    d->LockFirstColumnButton->blockSignals(wasBlocked);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTablesModuleWidget::onLockTableButtonClicked()
{
  Q_D(qSlicerTablesModuleWidget);

  if (!d->MRMLTableNode)
    {
    return;
    }

  // toggle the lock
  int locked = d->MRMLTableNode->GetLocked();
  d->MRMLTableNode->SetLocked(!locked);
}

//-----------------------------------------------------------------------------
void qSlicerTablesModuleWidget::setCurrentTableNode(vtkMRMLNode* tableNode)
{
  Q_D(qSlicerTablesModuleWidget);
  d->TableNodeSelector->setCurrentNode(tableNode);
}

//-----------------------------------------------------------
bool qSlicerTablesModuleWidget::setEditedNode(vtkMRMLNode* node,
                                              QString role /* = QString()*/,
                                              QString context /* = QString()*/)
{
  Q_D(qSlicerTablesModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);

  if (vtkMRMLTableNode::SafeDownCast(node))
    {
    d->TableNodeSelector->setCurrentNode(node);
    return true;
    }
  return false;
}
