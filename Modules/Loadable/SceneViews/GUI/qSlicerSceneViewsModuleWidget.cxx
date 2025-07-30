#include "GUI/qSlicerSceneViewsModuleWidget.h"
#include "ui_qSlicerSceneViewsModuleWidget.h"

// CTK includes
#include "ctkMessageBox.h"
#include "ctkFittedTextBrowser.h"

// Qt includes
#include <QDebug>
#include <QLabel>
#include <QMainWindow>
#include <QPointer>
#include <QPushButton>
#include <QStatusBar>
#include <QToolButton>
#include <QVBoxLayout>

// MRML includes
#include "qMRMLUtils.h"
#include "vtkMRMLScene.h"

// Sequences MRML includes
#include "vtkMRMLSequenceBrowserNode.h"

// VTK includes
#include "vtkCollection.h"
#include "vtkImageData.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

// GUI includes
#include "GUI/qSlicerSceneViewsModuleDialog.h"
#include "qSlicerApplication.h"

enum
{
  SCENE_VIEW_THUMBNAIL_COLUMN = 0,
  SCENE_VIEW_DESCRIPTION_COLUMN,
  SCENE_VIEW_ACTIONS_COLUMN,
  // Add columns above this line
  SCENE_VIEW_NUMBER_OF_COLUMNS
};

static const char ROW_INDEX_PROPERTY[] = "RowIndex";

//-----------------------------------------------------------------------------
class qSlicerSceneViewsModuleWidgetPrivate : public Ui_qSlicerSceneViewsModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerSceneViewsModuleWidget);

protected:
  qSlicerSceneViewsModuleWidget* const q_ptr;

public:
  qSlicerSceneViewsModuleWidgetPrivate(qSlicerSceneViewsModuleWidget& object);
  ~qSlicerSceneViewsModuleWidgetPrivate();
  void setupUi(qSlicerWidget* widget);

  vtkSlicerSceneViewsModuleLogic* logic() const;
  qSlicerSceneViewsModuleDialog* sceneViewDialog();
  void updateTableRowFromSceneView(int row);

  QPointer<qSlicerSceneViewsModuleDialog> SceneViewDialog;
};

//-----------------------------------------------------------------------------
// qSlicerSceneViewsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
vtkSlicerSceneViewsModuleLogic* qSlicerSceneViewsModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerSceneViewsModuleWidget);
  return vtkSlicerSceneViewsModuleLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleDialog* qSlicerSceneViewsModuleWidgetPrivate::sceneViewDialog()
{
  if (!this->SceneViewDialog)
  {
    qSlicerApplication* app = qSlicerApplication::application();
    QWidget* mainWindow = app ? app->mainWindow() : nullptr;
    this->SceneViewDialog = new qSlicerSceneViewsModuleDialog(mainWindow);

    // pass a pointer to the logic class
    this->SceneViewDialog->setLogic(this->logic());
  }
  return this->SceneViewDialog;
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleWidgetPrivate::qSlicerSceneViewsModuleWidgetPrivate(qSlicerSceneViewsModuleWidget& object)
  : q_ptr(&object)
{
  this->SceneViewDialog = nullptr;
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleWidgetPrivate::~qSlicerSceneViewsModuleWidgetPrivate()
{
  if (this->SceneViewDialog)
  {
    this->SceneViewDialog->close();
    delete this->SceneViewDialog.data();
  }
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  Q_Q(qSlicerSceneViewsModuleWidget);
  this->Ui_qSlicerSceneViewsModuleWidget::setupUi(widget);

  this->SceneViewTableWidget->setColumnCount(SCENE_VIEW_NUMBER_OF_COLUMNS);
  this->SceneViewTableWidget->setHorizontalHeaderLabels(QStringList() //
                                                        << qSlicerSceneViewsModuleWidget::tr("Thumbnail") << qSlicerSceneViewsModuleWidget::tr("Description")
                                                        << qSlicerSceneViewsModuleWidget::tr("Actions"));
  this->SceneViewTableWidget->horizontalHeader()->hide();

  this->SceneViewTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  this->SceneViewTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  this->SceneViewTableWidget->horizontalHeader()->setSectionResizeMode(SCENE_VIEW_DESCRIPTION_COLUMN, QHeaderView::Stretch);

  // background of text browser widget is painted by the widget, and images has no background
  // either, so it is easier to just disable selection
  this->SceneViewTableWidget->setSelectionMode(QAbstractItemView::NoSelection);

  // listen for click on a markup
  QObject::connect(this->SceneViewTableWidget, SIGNAL(cellDoubleClicked(int, int)), q, SLOT(onSceneViewDoubleClicked(int, int)));
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidgetPrivate::updateTableRowFromSceneView(int row)
{
  Q_Q(qSlicerSceneViewsModuleWidget);
  if (row >= this->SceneViewTableWidget->rowCount())
  {
    return;
  }

  // Thumbnail
  vtkImageData* thumbnailImage = this->logic()->GetNthSceneViewScreenshot(row);
  QLabel* thumbnailWidget = dynamic_cast<QLabel*>(this->SceneViewTableWidget->cellWidget(row, SCENE_VIEW_THUMBNAIL_COLUMN));
  if (thumbnailWidget == nullptr)
  {
    thumbnailWidget = new QLabel;
    this->SceneViewTableWidget->setCellWidget(row, SCENE_VIEW_THUMBNAIL_COLUMN, thumbnailWidget);
  }
  if (thumbnailImage)
  {
    QImage qimage;
    qMRMLUtils::vtkImageDataToQImage(thumbnailImage, qimage);
    thumbnailWidget->setPixmap(QPixmap::fromImage(qimage).scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  }
  else
  {
    thumbnailWidget->setPixmap(QPixmap(":/Icons/Extension.png"));
  }

  // Description
  QString name = QString::fromStdString(this->logic()->GetNthSceneViewName(row));
  QString description = QString::fromStdString(this->logic()->GetNthSceneViewDescription(row));
  // replace any carriage returns with html line breaks
  description.replace(QString("\n"), QString("<br>"));
  ctkFittedTextBrowser* descriptionWidget = dynamic_cast<ctkFittedTextBrowser*>(this->SceneViewTableWidget->cellWidget(row, SCENE_VIEW_DESCRIPTION_COLUMN));
  if (descriptionWidget == nullptr)
  {
    descriptionWidget = new ctkFittedTextBrowser;
    descriptionWidget->setOpenExternalLinks(true);
    descriptionWidget->setAutoFillBackground(false);
    this->SceneViewTableWidget->setCellWidget(row, SCENE_VIEW_DESCRIPTION_COLUMN, descriptionWidget);
  }
  descriptionWidget->setHtml("<h3>" + name + "</h3>\n" + description);

  QFrame* actionsWidget = dynamic_cast<QFrame*>(this->SceneViewTableWidget->cellWidget(row, SCENE_VIEW_ACTIONS_COLUMN));
  if (actionsWidget == nullptr)
  {
    actionsWidget = new QFrame;
    QVBoxLayout* actionsLayout = new QVBoxLayout;
    actionsWidget->setLayout(actionsLayout);
    QToolButton* restoreButton = new QToolButton;
    restoreButton->setText(qSlicerSceneViewsModuleWidget::tr("Restore"));
    restoreButton->setToolTip(qSlicerSceneViewsModuleWidget::tr("Restore"));
    restoreButton->setIcon(QIcon(":/Icons/Restore.png"));
    restoreButton->setProperty(ROW_INDEX_PROPERTY, row);
    QObject::connect(restoreButton, SIGNAL(clicked()), q, SLOT(onRestoreButtonClicked()));
    QToolButton* editButton = new QToolButton;
    editButton->setText(qSlicerSceneViewsModuleWidget::tr("Edit"));
    editButton->setToolTip(qSlicerSceneViewsModuleWidget::tr("Edit"));
    editButton->setIcon(QIcon(":/Icons/Medium/SlicerConfigure.png"));
    editButton->setProperty(ROW_INDEX_PROPERTY, row);
    QObject::connect(editButton, SIGNAL(clicked()), q, SLOT(onEditButtonClicked()));
    QToolButton* deleteButton = new QToolButton;
    deleteButton->setText(qSlicerSceneViewsModuleWidget::tr("Delete"));
    deleteButton->setToolTip(qSlicerSceneViewsModuleWidget::tr("Delete"));
    deleteButton->setIcon(QIcon(":/Icons/Delete.png"));
    deleteButton->setProperty(ROW_INDEX_PROPERTY, row);
    QObject::connect(deleteButton, SIGNAL(clicked()), q, SLOT(onDeleteButtonClicked()));
    actionsLayout->addWidget(restoreButton);
    actionsLayout->addWidget(editButton);
    actionsLayout->addWidget(deleteButton);
    this->SceneViewTableWidget->setCellWidget(row, SCENE_VIEW_ACTIONS_COLUMN, actionsWidget);
  }
}

//-----------------------------------------------------------------------------
// qSlicerSceneViewsModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleWidget::qSlicerSceneViewsModuleWidget(QWidget* parent)
  : qSlicerAbstractModuleWidget(parent)
  , d_ptr(new qSlicerSceneViewsModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleWidget::~qSlicerSceneViewsModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::setup()
{
  Q_D(qSlicerSceneViewsModuleWidget);
  this->Superclass::setup();
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::moveDownSelected(QString sceneViewName)
{
  Q_D(qSlicerSceneViewsModuleWidget);

  std::string id = d->logic()->MoveSceneViewDown(sceneViewName.toStdString());
  if (id != "")
  {
    this->updateFromMRMLScene();
  }
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::moveUpSelected(QString sceneViewName)
{
  Q_D(qSlicerSceneViewsModuleWidget);

  std::string id = d->logic()->MoveSceneViewUp(sceneViewName.toStdString());
  if (id != "")
  {
    this->updateFromMRMLScene();
  }
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::restoreSceneView(const QString& sceneViewName)
{
  Q_D(qSlicerSceneViewsModuleWidget);

  if (!d->logic()->RestoreSceneView(sceneViewName.toStdString()))
  {
    qCritical() << "Failed to restore scene view " << sceneViewName;
  }

  qSlicerApplication::application()->mainWindow()->statusBar()->showMessage("The SceneView was restored including the attached scene.", 2000);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::editSceneView(int index)
{
  Q_D(qSlicerSceneViewsModuleWidget);
  d->sceneViewDialog()->loadSceneViewInfo(index);
  d->sceneViewDialog()->setSaveAsButtonVisibility(false);
  d->sceneViewDialog()->exec();
  this->updateFromMRMLScene();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::updateFromMRMLScene()
{
  Q_D(qSlicerSceneViewsModuleWidget);

  if (this->mrmlScene() == nullptr)
  {
    d->SceneViewTableWidget->setRowCount(0);
    return;
  }
  int numSceneViews = d->logic()->GetNumberOfSceneViews();

  // don't recreate the table if the number of items is not changed to preserve selection state
  d->SceneViewTableWidget->setRowCount(numSceneViews);

  for (int rowIndex = 0; rowIndex < numSceneViews; ++rowIndex)
  {
    d->updateTableRowFromSceneView(rowIndex);
  }
  d->SceneViewTableWidget->resizeRowsToContents();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::enter()
{
  this->Superclass::enter();

  // set up mrml scene observations so that the GUI gets updated
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::NodeAddedEvent, this, SLOT(onMRMLSceneEvent(vtkObject*, vtkObject*)));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::NodeRemovedEvent, this, SLOT(onMRMLSceneEvent(vtkObject*, vtkObject*)));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndCloseEvent, this, SLOT(onMRMLSceneReset()));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndImportEvent, this, SLOT(onMRMLSceneReset()));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndRestoreEvent, this, SLOT(onMRMLSceneReset()));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndBatchProcessEvent, this, SLOT(onMRMLSceneReset()));

  this->updateSceneViewObservers();
  this->updateFromMRMLScene();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::exit()
{
  this->Superclass::exit();

  // remove mrml scene observations, don't need to update the GUI while the
  // module is not showing
  this->qvtkDisconnectAll();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::onMRMLSceneEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerSceneViewsModuleWidget);
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
  {
    return;
  }

  vtkMRMLSequenceBrowserNode* sequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(node);
  if (d->logic()->IsSceneViewNode(sequenceBrowserNode))
  {
    this->updateSceneViewObservers();
  }
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::onMRMLSceneReset()
{
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
  {
    return;
  }
  this->updateFromMRMLScene();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::updateSceneViewObservers()
{
  Q_D(qSlicerSceneViewsModuleWidget);
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
  {
    return;
  }

  // get all the scene view nodes
  std::vector<vtkMRMLNode*> sequenceBrowserNodes;
  this->mrmlScene()->GetNodesByClass("vtkMRMLSequenceBrowserNode", sequenceBrowserNodes);
  for (std::vector<vtkMRMLNode*>::iterator it = sequenceBrowserNodes.begin(); it != sequenceBrowserNodes.end(); ++it)
  {
    vtkMRMLSequenceBrowserNode* sequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(*it);
    if (!sequenceBrowserNode)
    {
      continue;
    }

    if (!d->logic()->IsSceneViewNode(sequenceBrowserNode))
    {
      continue;
    }

    if (this->qvtkIsConnected(sequenceBrowserNode, vtkMRMLSequenceBrowserNode::SequenceNodeModifiedEvent, this, SLOT(updateFromMRMLScene())))
    {
      continue;
    }
    this->qvtkConnect(sequenceBrowserNode, vtkMRMLSequenceBrowserNode::SequenceNodeModifiedEvent, this, SLOT(updateFromMRMLScene()));
  }
}

//-----------------------------------------------------------------------------
// SceneView functionality
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::showSceneViewDialog()
{
  Q_D(qSlicerSceneViewsModuleWidget);
  // show the dialog
  d->sceneViewDialog()->setSaveAsButtonVisibility(false);
  d->sceneViewDialog()->reset();
  d->sceneViewDialog()->exec();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::onSceneViewDoubleClicked(int row, int column)
{
  Q_UNUSED(column);
  Q_D(qSlicerSceneViewsModuleWidget);
  vtkMRMLNode* sceneViewNode = this->mrmlScene()->GetNthNodeByClass(row, "vtkMRMLSceneViewNode");
  if (!sceneViewNode || !sceneViewNode->GetID())
  {
    return;
  }
  this->restoreSceneView(QString(sceneViewNode->GetID()));

  this->updateFromMRMLScene();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::onRestoreButtonClicked()
{
  Q_D(qSlicerSceneViewsModuleWidget);
  QToolButton* button = qobject_cast<QToolButton*>(this->sender());
  if (!button)
  {
    return;
  }
  int rowIndex = button->property(ROW_INDEX_PROPERTY).toInt();
  d->logic()->RestoreSceneView(rowIndex);

  this->updateFromMRMLScene();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::onEditButtonClicked()
{
  Q_D(qSlicerSceneViewsModuleWidget);
  QToolButton* button = qobject_cast<QToolButton*>(this->sender());
  if (!button)
  {
    return;
  }
  int rowIndex = button->property(ROW_INDEX_PROPERTY).toInt();
  this->editSceneView(rowIndex);

  this->updateFromMRMLScene();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::onDeleteButtonClicked()
{
  Q_D(qSlicerSceneViewsModuleWidget);
  QToolButton* button = qobject_cast<QToolButton*>(this->sender());
  if (!button)
  {
    return;
  }
  int rowIndex = button->property(ROW_INDEX_PROPERTY).toInt();
  d->logic()->RemoveSceneView(rowIndex);

  this->updateFromMRMLScene();
}
