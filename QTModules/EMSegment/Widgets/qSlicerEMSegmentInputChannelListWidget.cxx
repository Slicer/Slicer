
// Qt includes
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QToolButton>
#include <QDebug>

// CTK includes
#include <ctkLogger.h>

// qMRMLWidget includes
#include <qMRMLNodeComboBox.h>

// EMSegment includes
#include "qSlicerEMSegmentInputChannelListWidget.h"


//--------------------------------------------------------------------------
static ctkLogger logger(
    "org.slicer.qtmodules.emsegment.widgets.qSlicerEMSegmentInputChannelListWidget");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class qSlicerEMSegmentInputChannelListWidgetPrivate :
    public ctkPrivate<qSlicerEMSegmentInputChannelListWidget>
{
public:
  qSlicerEMSegmentInputChannelListWidgetPrivate();

  QTableWidget * TableWidget;
  QToolButton *  AddInputChannelButton;
  QToolButton *  RemoveInputChannelButton;

};

//-----------------------------------------------------------------------------
// qSlicerEMSegmentInputChannelListWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentInputChannelListWidgetPrivate::qSlicerEMSegmentInputChannelListWidgetPrivate()
{
  this->TableWidget = 0;
  this->AddInputChannelButton = 0;
  this->RemoveInputChannelButton = 0;
}

//-----------------------------------------------------------------------------
// qSlicerEMSegmentInputChannelListWidget methods

//-----------------------------------------------------------------------------
qSlicerEMSegmentInputChannelListWidget::qSlicerEMSegmentInputChannelListWidget(QWidget *newParent):
Superclass(newParent)
{
  logger.setDebug();
  CTK_INIT_PRIVATE(qSlicerEMSegmentInputChannelListWidget);
  CTK_D(qSlicerEMSegmentInputChannelListWidget);

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

  // Connect control buttons
  connect(d->AddInputChannelButton, SIGNAL(clicked()), SLOT(addInputChannel()));
  connect(d->RemoveInputChannelButton, SIGNAL(clicked()), SLOT(removeInputChannel()));

}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentInputChannelListWidget::addInputChannel()
{
  CTK_D(qSlicerEMSegmentInputChannelListWidget);
  int newRowId = d->TableWidget->rowCount();
  d->TableWidget->insertRow(newRowId);

  // Set volume name
  QTableWidgetItem * volumeNameItem =
      new QTableWidgetItem(QString("input %1").arg(newRowId));
  d->TableWidget->setItem(newRowId, 0, volumeNameItem);

  // Set volume node selector
  qMRMLNodeComboBox * volumeNodeComboBox = new qMRMLNodeComboBox(d->TableWidget);
  QStringList nodeTypes;
  nodeTypes << "vtkMRMLVolumeNode";
  volumeNodeComboBox->setNodeTypes(nodeTypes);
  volumeNodeComboBox->setNoneEnabled(true);
  volumeNodeComboBox->setAddEnabled(false);
  volumeNodeComboBox->setRemoveEnabled(false);
  volumeNodeComboBox->setMRMLScene(this->mrmlScene());
  d->TableWidget->setCellWidget(newRowId, 1, volumeNodeComboBox);

  // Unselect everything
  d->TableWidget->setRangeSelected(QTableWidgetSelectionRange(0, 0, newRowId, 1), false);
  // Select the last row
  d->TableWidget->setRangeSelected(QTableWidgetSelectionRange(newRowId, 0, newRowId, 1), true);
}

//-----------------------------------------------------------------------------
void qSlicerEMSegmentInputChannelListWidget::removeInputChannel()
{
  CTK_D(qSlicerEMSegmentInputChannelListWidget);

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

