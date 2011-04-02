/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QToolButton>
#include <QDebug>
#include <QItemEditorFactory>
#include <QStandardItemEditorCreator>
#include <QStyledItemDelegate>
#include <QRegExpValidator>
#include <QLineEdit>
#include <QEvent>

// CTK includes
#include <ctkLogger.h>

// qMRMLWidget includes
#include <qMRMLNodeComboBox.h>

// EMSegment includes
#include "qSlicerEMSegmentInputChannelListWidget.h"

// EMSegment/MRML includes
#include <vtkEMSegmentMRMLManager.h>
#include <vtkMRMLEMSTargetNode.h>


//--------------------------------------------------------------------------
static ctkLogger logger(
    "org.slicer.qtmodules.emsegment.widgets.qSlicerEMSegmentInputChannelListWidget");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_EMSegment
class qSlicerEMSegmentInputChannelListWidgetPrivate
{
  Q_DECLARE_PUBLIC(qSlicerEMSegmentInputChannelListWidget);
protected:
  qSlicerEMSegmentInputChannelListWidget* const q_ptr;
public:
  qSlicerEMSegmentInputChannelListWidgetPrivate(qSlicerEMSegmentInputChannelListWidget& object);

  void insertInputChannel(int rowId, const QString& inputChannelName = QString(),
                          vtkMRMLVolumeNode * volumeNodeToSelect = 0);

  void updateInputChannel(int rowId, const QString& inputChannelName,
                          vtkMRMLVolumeNode * volumeNodeToSelect);

  QTableWidget * TableWidget;
  QToolButton *  AddInputChannelButton;
  QToolButton *  RemoveInputChannelButton;

};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentInputChannelListWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentInputChannelListWidgetPrivate::qSlicerEMSegmentInputChannelListWidgetPrivate(qSlicerEMSegmentInputChannelListWidget& object)
  : q_ptr(&object)
{
  this->TableWidget = 0;
  this->AddInputChannelButton = 0;
  this->RemoveInputChannelButton = 0;
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentInputChannelListWidgetPrivate::insertInputChannel(
    int rowId, const QString& inputChannelName,
    vtkMRMLVolumeNode * volumeNodeToSelect)
{
  Q_Q(qSlicerEMSegmentInputChannelListWidget);

  //int rowId = this->TableWidget->rowCount();
  this->TableWidget->insertRow(rowId);

  // Set volume name
  QTableWidgetItem * volumeNameItem = new QTableWidgetItem(inputChannelName);
  this->TableWidget->setItem(rowId, 0, volumeNameItem);

  // Set volume node selector
  qMRMLNodeComboBox * volumeNodeComboBox = new qMRMLNodeComboBox(this->TableWidget);
  QStringList nodeTypes;
  nodeTypes << "vtkMRMLVolumeNode";
  volumeNodeComboBox->setNodeTypes(nodeTypes);
  volumeNodeComboBox->setNoneEnabled(false);
  volumeNodeComboBox->setAddEnabled(false);
  volumeNodeComboBox->setRemoveEnabled(false);
  volumeNodeComboBox->setMRMLScene(q->mrmlScene());
  volumeNodeComboBox->setCurrentNode(volumeNodeToSelect);
  this->TableWidget->setCellWidget(rowId, 1, volumeNodeComboBox);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentInputChannelListWidgetPrivate::updateInputChannel(
    int rowId, const QString& inputChannelName,
    vtkMRMLVolumeNode * volumeNodeToSelect)
{
  QTableWidgetItem * volumeNameItem = this->TableWidget->item(rowId, 0);
  volumeNameItem->setText(inputChannelName);

  qMRMLNodeComboBox * volumeNodeComboBox =
      qobject_cast<qMRMLNodeComboBox*>(this->TableWidget->cellWidget(rowId, 1));
  volumeNodeComboBox->setCurrentNode(volumeNodeToSelect);
}

//-----------------------------------------------------------------------------
// Custom editors

//-----------------------------------------------------------------------------
class InputChannelLineEdit : public QLineEdit
{
public:
  InputChannelLineEdit(QWidget * newParent):QLineEdit(newParent)
    {
    // Reject empty string
    regExp.setPattern(".+");
    this->setValidator(new QRegExpValidator(regExp, this));
    }
  QRegExp regExp;
};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentInputChannelListWidget methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentInputChannelListWidget::qSlicerEMSegmentInputChannelListWidget(QWidget *newParent)
  : Superclass(newParent)
  , d_ptr(new qSlicerEMSegmentInputChannelListWidgetPrivate(*this))
{
  Q_D(qSlicerEMSegmentInputChannelListWidget);

  // Layout TableWidget + (control buttons) horizontally
  QHBoxLayout * mainLayout = new QHBoxLayout(this);
  mainLayout->setSpacing(0);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  // Add TableWidget to the mainlayout
  d->TableWidget = new QTableWidget(this);
  d->TableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
  d->TableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  mainLayout->addWidget(d->TableWidget);

  // Layout control buttons vertically
  QVBoxLayout * controlButtonsLayout = new QVBoxLayout();

  d->AddInputChannelButton = new QToolButton();
  d->AddInputChannelButton->setIcon(QIcon(":/Icons/plus.png"));
  controlButtonsLayout->addWidget(d->AddInputChannelButton);

  d->RemoveInputChannelButton = new QToolButton();
  d->RemoveInputChannelButton->setIcon(QIcon(":/Icons/minus.png"));
  controlButtonsLayout->addWidget(d->RemoveInputChannelButton);

  controlButtonsLayout->addItem(
      new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

  // Add control button layout to mainlayout
  mainLayout->addLayout(controlButtonsLayout);

  // Add table columns
  d->TableWidget->setColumnCount(2);

  QTableWidgetItem * tablewidgetitem = new QTableWidgetItem();
  tablewidgetitem->setText(tr("Name"));
  d->TableWidget->setHorizontalHeaderItem(0, tablewidgetitem);

  QTableWidgetItem * tablewidgetitem1 = new QTableWidgetItem();
  tablewidgetitem1->setText(tr("Volume"));
  d->TableWidget->setHorizontalHeaderItem(1, tablewidgetitem1);

  d->TableWidget->horizontalHeader()->setVisible(true);
  d->TableWidget->horizontalHeader()->setStretchLastSection(true);

  // Register custom editors
  QItemEditorFactory *editorFactory = new QItemEditorFactory;
  editorFactory->registerEditor(
      QVariant::String, new QStandardItemEditorCreator<InputChannelLineEdit>());
  QStyledItemDelegate* defaultItemDelegate =
      qobject_cast<QStyledItemDelegate*>(d->TableWidget->itemDelegate());
  Q_ASSERT(defaultItemDelegate);
  defaultItemDelegate->setItemEditorFactory(editorFactory);

  // Connect tableWidget
  connect(d->TableWidget, SIGNAL(cellChanged(int,int)), SLOT(onCellChanged(int,int)));

  // Connect control buttons
  connect(d->AddInputChannelButton, SIGNAL(clicked()), SLOT(addInputChannel()));
  connect(d->RemoveInputChannelButton, SIGNAL(clicked()), SLOT(removeInputChannel()));

}

//-----------------------------------------------------------------------------
qSlicerEMSegmentInputChannelListWidget::~qSlicerEMSegmentInputChannelListWidget()
{
}

//-----------------------------------------------------------------------------
int qSlicerEMSegmentInputChannelListWidget::inputChannelCount()const
{
  Q_D(const qSlicerEMSegmentInputChannelListWidget);
  return d->TableWidget->rowCount();
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentInputChannelListWidget::inputChannelName(int rowId) const
{
  Q_D(const qSlicerEMSegmentInputChannelListWidget);

  QTableWidgetItem * item = d->TableWidget->item(rowId, 0);
  if (!item)
    {
    logger.error(QString("inputChannelName - Invalid rowId: %1").arg(rowId));
    return QString();
    }
  return item->text();
}

//-----------------------------------------------------------------------------
vtkMRMLVolumeNode* qSlicerEMSegmentInputChannelListWidget::inputChannelVolume(int rowId) const
{
  Q_D(const qSlicerEMSegmentInputChannelListWidget);

  qMRMLNodeComboBox * volumeNodeComboBox =
      qobject_cast<qMRMLNodeComboBox*>(d->TableWidget->cellWidget(rowId, 1));
  if (!volumeNodeComboBox)
    {
    logger.error(QString("inputChannelVolume - Invalid rowId: %1").arg(rowId));
    return 0;
    }

  Q_ASSERT(vtkMRMLVolumeNode::SafeDownCast(volumeNodeComboBox->currentNode()));

  return vtkMRMLVolumeNode::SafeDownCast(volumeNodeComboBox->currentNode());
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentInputChannelListWidget::setMRMLManager(
    vtkEMSegmentMRMLManager * newMRMLManager)
{
  this->qvtkReconnect(this->mrmlManager(), newMRMLManager, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromMRML()));

  this->Superclass::setMRMLManager(newMRMLManager);

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentInputChannelListWidget::updateMRMLFromWidget()
{
  Q_D(qSlicerEMSegmentInputChannelListWidget);

  if (!this->mrmlManager())
    {
    logger.warn("updateMRMLFromWidget - MRMLManager is NULL");
    return;
    }

  vtkMRMLEMSTargetNode *inputNodes = this->mrmlManager()->GetTargetInputNode();
  if (!inputNodes)
    {
    logger.warn("updateWidgetFromMRML - inputNodes is NULL");
    return;
    }

  inputNodes->SetNumberOfInputChannelName(this->inputChannelCount());

  for(int rowId = 0; rowId < this->inputChannelCount(); rowId++)
    {
    if (rowId == this->mrmlManager()->GetTargetNumberOfSelectedVolumes())
      {
      // Add input
      this->mrmlManager()->AddTargetSelectedVolumeByMRMLID(
          this->inputChannelVolume(rowId)->GetID());
      }
    else
      {
      // Update input
      this->mrmlManager()->SetTargetSelectedVolumeNthMRMLID(
          rowId, this->inputChannelVolume(rowId)->GetID());
      }

    // Update channel name
    inputNodes->SetNthInputChannelName(rowId, d->TableWidget->item(rowId, 0)->text().toLatin1());
    }
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentInputChannelListWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerEMSegmentInputChannelListWidget);

  if (!this->mrmlManager())
    {
    logger.warn("updateWidgetFromMRML - MRMLManager is NULL");
    return;
    }

  vtkMRMLEMSTargetNode *inputNodes = this->mrmlManager()->GetTargetInputNode();
  if (!inputNodes)
    {
    logger.warn("updateWidgetFromMRML - inputNodes is NULL");
    return;
    }

  // Loop through input nodes and update or insert row
  for (int rowId = 0; rowId < inputNodes->GetNumberOfVolumes(); rowId++)
    {
    QString inputChannelName = QLatin1String(inputNodes->GetNthInputChannelName(rowId));
    vtkMRMLVolumeNode * volumeNode = inputNodes->GetNthVolumeNode(rowId);
    Q_ASSERT(volumeNode);

    // Default volume name
    if (inputChannelName.isEmpty())
      {
      inputChannelName = QString("input %1").arg(rowId);
      }

    if (rowId < d->TableWidget->rowCount())
      {
      d->updateInputChannel(rowId, inputChannelName, volumeNode);
      }
    else
      {
      d->insertInputChannel(rowId, inputChannelName, volumeNode);
      }
    }

  // Remove extra rows
  if (d->TableWidget->rowCount() > inputNodes->GetNumberOfVolumes())
    {
    int rowId = d->TableWidget->rowCount() + 1 - inputNodes->GetNumberOfVolumes();
    for(int i = rowId; i < d->TableWidget->rowCount(); i++)
      {
      d->TableWidget->removeRow(i);
      }
    }

  int lastRowId = d->TableWidget->rowCount() - 1;

  // Unselect everything
  d->TableWidget->setRangeSelected(QTableWidgetSelectionRange(0, 0, lastRowId, 1), false);
  // Select the last row
  d->TableWidget->setRangeSelected(QTableWidgetSelectionRange(lastRowId, 0, lastRowId, 1), true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentInputChannelListWidget::addInputChannel()
{
  Q_D(qSlicerEMSegmentInputChannelListWidget);

  d->insertInputChannel(d->TableWidget->rowCount());

  int lastRowId = d->TableWidget->rowCount() - 1;

  // Unselect everything
  d->TableWidget->setRangeSelected(QTableWidgetSelectionRange(0, 0, lastRowId, 1), false);
  // Select the last row
  d->TableWidget->setRangeSelected(QTableWidgetSelectionRange(lastRowId, 0, lastRowId, 1), true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentInputChannelListWidget::removeInputChannel()
{
  Q_D(qSlicerEMSegmentInputChannelListWidget);

  QList<QTableWidgetSelectionRange> selectionRangeList = d->TableWidget->selectedRanges();
  while(selectionRangeList.count())
    {
    d->TableWidget->removeRow(selectionRangeList.at(0).topRow());
    selectionRangeList = d->TableWidget->selectedRanges();
    }

  // Select the last row
  int rowCount = d->TableWidget->rowCount();
  if (rowCount)
    {
    d->TableWidget->setRangeSelected(
        QTableWidgetSelectionRange(rowCount - 1, 0, rowCount - 1, 1), true);
    }
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentInputChannelListWidget::onCellChanged(int row, int column)
{
  Q_D(qSlicerEMSegmentInputChannelListWidget);
  QTableWidgetItem * item = d->TableWidget->item(row, column);
  if (item->text().isEmpty())
    {
    item->setText(QString("input %1").arg(row));
    }
}

//-----------------------------------------------------------------------------
bool qSlicerEMSegmentInputChannelListWidget::identicalInputVolumes() const
{
  Q_D(const qSlicerEMSegmentInputChannelListWidget);

  for(int i = 0; i < d->TableWidget->rowCount(); i++)
    {

    vtkMRMLVolumeNode * volumeNode = this->inputChannelVolume(i);

    for (int j = i + 1; j < d->TableWidget->rowCount(); j++)
      {
      if (volumeNode == this->inputChannelVolume(j))
        {
        return true;
        }
      }
    }
  return false;
}
