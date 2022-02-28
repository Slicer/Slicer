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

// Segmentations includes
#include "qMRMLSegmentationRepresentationsListView.h"
#include "ui_qMRMLSegmentationRepresentationsListView.h"

#include "qMRMLSegmentationConversionParametersWidget.h"
#include "qSlicerApplication.h"

#include "vtkMRMLSegmentationNode.h"
#include "vtkSegmentation.h"

#include "ctkMenuButton.h"

// Qt includes
#include <QAction>
#include <QDebug>
#include <QDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QToolButton>
#include <QVariant>

#define REPRESENTATION_NAME_PROPERTY "RepresentationName"

// --------------------------------------------------------------------------
class qMRMLSegmentationRepresentationsListViewPrivate: public Ui_qMRMLSegmentationRepresentationsListView
{
  Q_DECLARE_PUBLIC(qMRMLSegmentationRepresentationsListView);
protected:
  qMRMLSegmentationRepresentationsListView* const q_ptr;
public:
  qMRMLSegmentationRepresentationsListViewPrivate(qMRMLSegmentationRepresentationsListView& object);
  void init();

  /// Sets table message and takes care of the visibility of the label
  void setMessage(const QString& message);

  /// Return the column index for a given string, -1 if not a valid header
  int columnIndex(QString label);

public:
  /// Segmentation MRML node containing shown segments
  vtkWeakPointer<vtkMRMLSegmentationNode> SegmentationNode;

private:
  QStringList ColumnLabels;
};

// --------------------------------------------------------------------------
qMRMLSegmentationRepresentationsListViewPrivate::qMRMLSegmentationRepresentationsListViewPrivate(qMRMLSegmentationRepresentationsListView& object)
  : q_ptr(&object)
{
  this->SegmentationNode = nullptr;
}

// --------------------------------------------------------------------------
void qMRMLSegmentationRepresentationsListViewPrivate::init()
{
  Q_Q(qMRMLSegmentationRepresentationsListView);
  this->setupUi(q);

  this->setMessage(QString());

  // Set up initial look of node representations list
  this->RepresentationsList->setSelectionMode(QAbstractItemView::NoSelection);
  this->RepresentationsList->setStyleSheet( "QListWidget::item { border-bottom: 1px solid lightGray; }" );
}

//-----------------------------------------------------------------------------
int qMRMLSegmentationRepresentationsListViewPrivate::columnIndex(QString label)
{
  return this->ColumnLabels.indexOf(label);
}

// --------------------------------------------------------------------------
void qMRMLSegmentationRepresentationsListViewPrivate::setMessage(const QString& message)
{
  this->RepresentationsListMessageLabel->setVisible(!message.isEmpty());
  this->RepresentationsListMessageLabel->setText(message);
}


// --------------------------------------------------------------------------
// qMRMLSegmentationRepresentationsListView methods

// --------------------------------------------------------------------------
qMRMLSegmentationRepresentationsListView::qMRMLSegmentationRepresentationsListView(QWidget* _parent)
  : QWidget(_parent)
  , d_ptr(new qMRMLSegmentationRepresentationsListViewPrivate(*this))
{
  Q_D(qMRMLSegmentationRepresentationsListView);
  d->init();
  this->populateRepresentationsList();
}

// --------------------------------------------------------------------------
qMRMLSegmentationRepresentationsListView::~qMRMLSegmentationRepresentationsListView() = default;

//-----------------------------------------------------------------------------
void qMRMLSegmentationRepresentationsListView::setSegmentationNode(vtkMRMLNode* node)
{
  Q_D(qMRMLSegmentationRepresentationsListView);
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(node);
  if (d->SegmentationNode == segmentationNode)
    {
    // no change
    return;
    }

  qvtkReconnect( d->SegmentationNode, segmentationNode, vtkSegmentation::MasterRepresentationModified,
                 this, SLOT( populateRepresentationsList() ) );
  qvtkReconnect( d->SegmentationNode, segmentationNode, vtkSegmentation::ContainedRepresentationNamesModified,
                 this, SLOT( populateRepresentationsList() ) );
  qvtkReconnect( d->SegmentationNode, segmentationNode, vtkSegmentation::SegmentModified,
                 this, SLOT( populateRepresentationsList() ) );
  qvtkReconnect( d->SegmentationNode, segmentationNode, vtkSegmentation::SegmentAdded,
                 this, SLOT( populateRepresentationsList() ) );
  qvtkReconnect( d->SegmentationNode, segmentationNode, vtkSegmentation::SegmentRemoved,
                 this, SLOT( populateRepresentationsList() ) );

  d->SegmentationNode = segmentationNode;
  this->populateRepresentationsList();
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qMRMLSegmentationRepresentationsListView::segmentationNode()
{
  Q_D(qMRMLSegmentationRepresentationsListView);

  return d->SegmentationNode;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationRepresentationsListView::populateRepresentationsList()
{
  Q_D(qMRMLSegmentationRepresentationsListView);

  d->setMessage(QString());

  // Block signals so that onMasterRepresentationChanged function is not called when populating
  QSignalBlocker blocker(d->RepresentationsList);

  d->RepresentationsList->clear();

  if (!d->SegmentationNode)
    {
    d->setMessage(tr("No node is selected"));
    return;
    }

  // Get available representation names
  std::set<std::string> representationNames;
  vtkSegmentation* segmentation = d->SegmentationNode->GetSegmentation();
  segmentation->GetAvailableRepresentationNames(representationNames);

  int row = 0;
  for (std::set<std::string>::iterator reprIt=representationNames.begin(); reprIt!=representationNames.end(); ++reprIt, ++row)
    {
    QString name(reprIt->c_str());

    QWidget* representationWidget = new QWidget(d->RepresentationsList);
    QHBoxLayout* representationLayout = new QHBoxLayout(representationWidget);
    representationLayout->setContentsMargins(4, 2, 0, 2);
    representationLayout->setSpacing(6);

    QListWidgetItem* representationItem = new QListWidgetItem();
    representationItem->setFlags(representationItem->flags() & ~Qt::ItemIsEditable);
    //representationItem->setSizeHint(QSize(-1,26)); //TODO:

    // Representation name
    QLabel* nameLabel = new QLabel(name, representationWidget);
    QFont nameFont;
    nameFont.setWeight(QFont::Bold);
    nameLabel->setFont(nameFont);
    nameLabel->setMinimumWidth(128);
    representationLayout->addWidget(nameLabel);

    // Determine whether current representation is master or is present
    bool master = !name.compare(segmentation->GetMasterRepresentationName().c_str());
    bool present = segmentation->ContainsRepresentation(reprIt->c_str());

    // Status
    if (master)
      {
      representationItem->setIcon(QIcon(":/Icons/Master.png"));
      representationItem->setToolTip(tr("This is the master representation.\n  1. This representation is saved on disk\n  2. If this representation is modified, the others are cleared"));
      }
    else if (present)
      {
      representationItem->setIcon(QIcon(":/Icons/Present.png"));
      representationItem->setToolTip(tr("This representation is present"));
      }
    else
      {
      QPixmap emptyPixmap(16, 16);
      emptyPixmap.fill(Qt::transparent);
      QIcon emptyIcon(emptyPixmap);
      representationItem->setIcon(emptyIcon);
      representationItem->setToolTip(tr("This representation is not present"));
      }

    // Action
    if (!master)
      {
      if (present || segmentation->GetNumberOfSegments() == 0) // if there are no segments we allow making any representation the master
        {
        if (present)
          {
          // Existing representations get an update button
          ctkMenuButton* updateButton = new ctkMenuButton(representationWidget);
          updateButton->setText("Update");
          QString updateButtonTooltip = QString("Update %1 representation using custom conversion parameters.\n\n"
            "Click the down-arrow button for additional operations.").arg(name);
          updateButton->setToolTip(updateButtonTooltip);
          updateButton->setProperty(REPRESENTATION_NAME_PROPERTY, QVariant(name));
          updateButton->setMinimumWidth(updateButton->sizeHint().width() + 10);
          QObject::connect(updateButton, SIGNAL(clicked()), this, SLOT(createRepresentationAdvanced()));

          QMenu* updateMenu = new QMenu(updateButton);
          updateButton->setMenu(updateMenu);

          // Set up actions for the update button
          QAction* removeAction = new QAction("Remove", updateButton);
          QString removeActionTooltip = QString("Remove %1 representation from segmentation").arg(name);
          removeAction->setToolTip(removeActionTooltip);
          removeAction->setProperty(REPRESENTATION_NAME_PROPERTY, QVariant(name));
          QObject::connect(removeAction, SIGNAL(triggered()), this, SLOT(removeRepresentation()));
          updateMenu->addAction(removeAction);

          representationLayout->addWidget(updateButton);
          }

        QPushButton* makeMasterButton = new QPushButton(representationWidget);
        makeMasterButton->setText("Make master");
        makeMasterButton->setProperty(REPRESENTATION_NAME_PROPERTY, QVariant(name));
        QObject::connect(makeMasterButton, SIGNAL(clicked()), this, SLOT(makeMaster()));

        representationLayout->addWidget(makeMasterButton);
        }
      else
        {
        // Missing representations get a create button
        ctkMenuButton* createButton = new ctkMenuButton(representationWidget);
        createButton->setText("Create");
        QString convertButtonTooltip = QString("Create %1 representation using default conversion parameters.\n\nPress and hold button to access advanced conversion and removal options.").arg(name);
        createButton->setToolTip(convertButtonTooltip);
        createButton->setProperty(REPRESENTATION_NAME_PROPERTY, QVariant(name));
        createButton->setMinimumWidth(createButton->sizeHint().width() + 10);
        QObject::connect(createButton, SIGNAL(clicked()), this, SLOT(createRepresentationDefault()));

        QMenu* createMenu = new QMenu(createButton);
        createButton->setMenu(createMenu);

        // Set up actions for the create button
        QAction* advancedAction = new QAction("Advanced create...", createMenu);
        QString advancedActionTooltip = QString("Create %1 representation using custom conversion parameters").arg(name);
        advancedAction->setToolTip(advancedActionTooltip);
        advancedAction->setProperty(REPRESENTATION_NAME_PROPERTY, QVariant(name));
        QObject::connect(advancedAction, SIGNAL(triggered()), this, SLOT(createRepresentationAdvanced()));
        createMenu->addAction(advancedAction);

        representationLayout->addWidget(createButton);
        }
      }

    representationLayout->addStretch();
    d->RepresentationsList->addItem(representationItem);
    d->RepresentationsList->setItemWidget(representationItem, representationWidget);
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationRepresentationsListView::createRepresentationDefault()
{
  Q_D(qMRMLSegmentationRepresentationsListView);

  if (!d->SegmentationNode)
    {
    return;
    }

  MRMLNodeModifyBlocker blocker(d->SegmentationNode);

  // Get representation name
  QString representationName = this->sender()->property(REPRESENTATION_NAME_PROPERTY).toString();

  // Perform conversion using cheapest path and default conversion parameters
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  if (!d->SegmentationNode->GetSegmentation()->CreateRepresentation(representationName.toUtf8().constData()))
    {
    QString message = QString("Failed to convert %1 to %2!\n\nProbably there is no valid conversion path between the master representation and %2").arg(d->SegmentationNode->GetName()).arg(representationName);
    QMessageBox::warning(nullptr, tr("Conversion failed"), message);
    }
  QApplication::restoreOverrideCursor();

  this->populateRepresentationsList();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationRepresentationsListView::createRepresentationAdvanced()
{
  Q_D(qMRMLSegmentationRepresentationsListView);

  // Get representation name
  QString representationName = this->sender()->property(REPRESENTATION_NAME_PROPERTY).toString();

  // Create dialog to show the parameters widget in a popup window
  qSlicerApplication* app = qSlicerApplication::application();
  QWidget* mainWindow = app ? app->mainWindow() : nullptr;
  QDialog* parametersDialog = new QDialog(mainWindow, Qt::Dialog);
  parametersDialog->setObjectName("SegmentationConversionParametersWindow");
  parametersDialog->setWindowTitle("Advanced segmentation conversion");
  QVBoxLayout* layout = new QVBoxLayout(parametersDialog);

  // Load last saved dialog position (the dialog may contain several options, so
  // it is useful if last size&position is preserved.
  QSettings settings;
  settings.beginGroup("Segmentations");
  parametersDialog->restoreGeometry(settings.value("SegmentationConversionParametersWindowGeometry").toByteArray());
  settings.endGroup();

  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(4);

  // Create conversion parameters widget to allow user editing conversion details
  qMRMLSegmentationConversionParametersWidget* parametersWidget = new qMRMLSegmentationConversionParametersWidget(parametersDialog);
  parametersWidget->setSegmentationNode(d->SegmentationNode);
  parametersWidget->setTargetRepresentationName(representationName);
  layout->addWidget(parametersWidget);

  // Connect conversion done event to dialog close
  QObject::connect(parametersWidget, SIGNAL(conversionDone()),
                   parametersDialog, SLOT(accept()));

  // Show dialog
  parametersDialog->exec();

  // Save dialog size&position
  settings.beginGroup("Segmentations");
  settings.setValue("SegmentationConversionParametersWindowGeometry", parametersDialog->saveGeometry());
  settings.endGroup();

  // Delete dialog when done
  parametersDialog->deleteLater();

  this->populateRepresentationsList();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationRepresentationsListView::removeRepresentation()
{
  Q_D(qMRMLSegmentationRepresentationsListView);

  if (!d->SegmentationNode)
    {
    return;
    }

  MRMLNodeModifyBlocker blocker(d->SegmentationNode);

  // Get representation name
  QString representationName = this->sender()->property(REPRESENTATION_NAME_PROPERTY).toString();

  // Remove representation from segmentation
  d->SegmentationNode->GetSegmentation()->RemoveRepresentation(representationName.toUtf8().constData());

  this->populateRepresentationsList();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationRepresentationsListView::makeMaster()
{
  Q_D(qMRMLSegmentationRepresentationsListView);

  if (!d->SegmentationNode)
    {
    return;
    }

  MRMLNodeModifyBlocker blocker(d->SegmentationNode);

  // Get representation name
  QString representationName = this->sender()->property(REPRESENTATION_NAME_PROPERTY).toString();

  if (d->SegmentationNode->GetSegmentation()->GetNumberOfSegments() > 0)
    {
    // Warn user about the consequences of changing master representation
    QMessageBox::StandardButton answer =
      QMessageBox::question(nullptr, tr("Confirm master representation change"),
      tr("Changing master representation will make the 'gold standard' representation the selected one, "
      "and will result in deletion of all the other representations.\n"
      "This may mean losing important data that cannot be created again from the new master representation.\n\n"
      "(Reminder: Master representation is the data type which is saved to disk, and which is used as input when creating other representations)\n\n"
      "Do you wish to proceed with changing master representation?"),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes)
      {
      return;
      }
    }

  d->SegmentationNode->GetSegmentation()->SetMasterRepresentationName(representationName.toUtf8().constData());
  this->populateRepresentationsList();
}
