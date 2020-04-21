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
#include "vtkMRMLSceneViewNode.h"

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
/// \ingroup Slicer_QtModules_SceneViews
class qSlicerSceneViewsModuleWidgetPrivate: public Ui_qSlicerSceneViewsModuleWidget
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
  void updateTableRowFromSceneView(int row, vtkMRMLSceneViewNode *sceneView);

  QPointer<qSlicerSceneViewsModuleDialog> SceneViewDialog;
};

//-----------------------------------------------------------------------------
// qSlicerSceneViewsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
vtkSlicerSceneViewsModuleLogic*
qSlicerSceneViewsModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerSceneViewsModuleWidget);
  return vtkSlicerSceneViewsModuleLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleDialog* qSlicerSceneViewsModuleWidgetPrivate::sceneViewDialog()
{
  if (!this->SceneViewDialog)
    {
    this->SceneViewDialog = new qSlicerSceneViewsModuleDialog();

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
  this->SceneViewTableWidget->setHorizontalHeaderLabels(QStringList() << "Thumbnail" << "Description" << "Actions");
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
void qSlicerSceneViewsModuleWidgetPrivate::updateTableRowFromSceneView(int row, vtkMRMLSceneViewNode *sceneView)
{
  Q_Q(qSlicerSceneViewsModuleWidget);
  if (row >= this->SceneViewTableWidget->rowCount())
    {
    return;
    }

  // Thumbnail
  vtkImageData* thumbnailImage = sceneView->GetScreenShot();
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
  QString name = sceneView->GetName();
  QString description = sceneView->GetSceneViewDescription().c_str();
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
qSlicerSceneViewsModuleWidget::qSlicerSceneViewsModuleWidget(QWidget* parent) :
  qSlicerAbstractModuleWidget(parent)
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
void qSlicerSceneViewsModuleWidget::moveDownSelected(QString mrmlId)
{
  Q_D(qSlicerSceneViewsModuleWidget);

  const char* id = d->logic()->MoveSceneViewDown(mrmlId.toUtf8());

  if (id != nullptr &&
      strcmp(id, "") != 0)
    {
    this->updateFromMRMLScene();
    }
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::moveUpSelected(QString mrmlId)
{
  Q_D(qSlicerSceneViewsModuleWidget);

  const char* id = d->logic()->MoveSceneViewUp(mrmlId.toUtf8());

  if (id != nullptr &&
      strcmp(id, "") != 0)
    {
    this->updateFromMRMLScene();
    }
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::restoreSceneView(const QString& mrmlId)
{
  Q_D(qSlicerSceneViewsModuleWidget);

  // by default, make sure no nodes from the current scene are lost, adding them to
  // the scene view about to be restored
  d->logic()->RestoreSceneView(mrmlId.toUtf8(), false);

  // check if there was a problem restoring
  if (this->mrmlScene()->GetErrorCode() != 0)
    {
    // reset the error state
    this->mrmlScene()->SetErrorCode(0);
    this->mrmlScene()->SetErrorMessage("");

    // ask if the user wishes to save the current scene nodes, restore and delete them or cancel
    ctkMessageBox missingNodesMsgBox;
    missingNodesMsgBox.setWindowTitle("Data missing from Scene View");
    vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->mrmlScene()->GetNodeByID(mrmlId.toUtf8()));
    QString sceneViewName;
    if (viewNode)
      {
      sceneViewName = QString(viewNode->GetName());
      }
    QString labelText = QString("Add data to scene view \"")
      + sceneViewName
      + QString("\" before restoring?\n"
                "\n");
    QString infoText = QString(
      "Data is present in the current scene but not in the scene view.\n"
      "\n"
      "If you don't add and restore, data not already saved to disk"
      ", or saved in another scene view,"
      " will be permanently lost!\n");
    missingNodesMsgBox.setText(labelText + infoText);
    // until CTK bug is fixed, informative text will overlap the don't show
    // again message so put it all in the label text
    // missingNodesMsgBox.setInformativeText(infoText);
    QPushButton *continueButton = missingNodesMsgBox.addButton(QMessageBox::Discard);
    continueButton->setText("Restore without saving");
    QPushButton *addButton = missingNodesMsgBox.addButton(QMessageBox::Save);
    addButton->setText("Add and Restore");
    missingNodesMsgBox.addButton(QMessageBox::Cancel);

    missingNodesMsgBox.setIcon(QMessageBox::Warning);
    missingNodesMsgBox.setDontShowAgainVisible(true);
    missingNodesMsgBox.setDontShowAgainSettingsKey("SceneViewsModule/AlwaysRemoveNodes");
    int ret = missingNodesMsgBox.exec();
    switch (ret)
      {
      case QMessageBox::Discard:
        d->logic()->RestoreSceneView(mrmlId.toUtf8(), true);
        break;
      case QMessageBox::Save:
        if (viewNode)
          {
          viewNode->AddMissingNodes();

          // and restore again
          d->logic()->RestoreSceneView(mrmlId.toUtf8(), false);
          }
        break;
      case QMessageBox::Cancel:
      default:
        break;
      }
    }

  qSlicerApplication::application()->mainWindow()->statusBar()->showMessage("The SceneView was restored including the attached scene.", 2000);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::editSceneView(const QString& mrmlId)
{
  Q_D(qSlicerSceneViewsModuleWidget);
  d->sceneViewDialog()->loadNode(mrmlId);
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
  int numSceneViews = this->mrmlScene()->GetNumberOfNodesByClass("vtkMRMLSceneViewNode");

  // don't recreate the table if the number of items is not changed to preserve selection state
  d->SceneViewTableWidget->setRowCount(numSceneViews);

  std::vector<vtkMRMLNode*> sceneViewNodes;
  this->mrmlScene()->GetNodesByClass("vtkMRMLSceneViewNode", sceneViewNodes);
  int rowIndex = 0;
  for (std::vector<vtkMRMLNode*>::iterator it = sceneViewNodes.begin(); it != sceneViewNodes.end(); ++it)
    {
    vtkMRMLSceneViewNode* sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(*it);
    if (!sceneViewNode)
      {
      continue;
      }
    d->updateTableRowFromSceneView(rowIndex, sceneViewNode);
    ++rowIndex;
    }

  d->SceneViewTableWidget->resizeRowsToContents();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::enter()
{
  this->Superclass::enter();

  // set up mrml scene observations so that the GUI gets updated
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::NodeAddedEvent,
                    this, SLOT(onMRMLSceneEvent(vtkObject*, vtkObject*)));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::NodeRemovedEvent,
                    this, SLOT(onMRMLSceneEvent(vtkObject*, vtkObject*)));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndCloseEvent,
                    this, SLOT(onMRMLSceneReset()));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndImportEvent,
                    this, SLOT(onMRMLSceneReset()));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndRestoreEvent,
                    this, SLOT(onMRMLSceneReset()));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndBatchProcessEvent,
                    this, SLOT(onMRMLSceneReset()));

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
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }
  vtkMRMLSceneViewNode* sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(node);
  if (sceneViewNode)
    {
    this->updateFromMRMLScene();
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
// SceneView functionality
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::showSceneViewDialog()
{
  Q_D(qSlicerSceneViewsModuleWidget);
  // show the dialog
  d->sceneViewDialog()->reset();
  d->sceneViewDialog()->exec();
}

//-----------------------------------------------------------
bool qSlicerSceneViewsModuleWidget::setEditedNode(vtkMRMLNode* node,
                                                  QString role /* = QString()*/,
                                                  QString context /* = QString()*/)
{
  Q_UNUSED(role);
  Q_UNUSED(context);
  Q_D(qSlicerSceneViewsModuleWidget);
  if (!vtkMRMLSceneViewNode::SafeDownCast(node))
    {
    return false;
    }
  std::vector<vtkMRMLNode*> sceneViewNodes;
  this->mrmlScene()->GetNodesByClass("vtkMRMLSceneViewNode", sceneViewNodes);
  int rowIndex = 0;
  for (std::vector<vtkMRMLNode*>::iterator it = sceneViewNodes.begin(); it != sceneViewNodes.end(); ++it)
    {
    vtkMRMLSceneViewNode* sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(*it);
    if (!sceneViewNode)
      {
      continue;
      }
    if (node == sceneViewNode)
      {
      // scene view node found
      this->updateFromMRMLScene();
      QModelIndex newIndex = d->SceneViewTableWidget->model()->index(rowIndex, SCENE_VIEW_ACTIONS_COLUMN);
      d->SceneViewTableWidget->setCurrentIndex(newIndex);
      return true;
      }
    ++rowIndex;
    }
  // scene view node not found
  return false;
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
  vtkMRMLNode* sceneViewNode = this->mrmlScene()->GetNthNodeByClass(rowIndex, "vtkMRMLSceneViewNode");
  if (!sceneViewNode || !sceneViewNode->GetID())
    {
    return;
    }
  this->restoreSceneView(QString(sceneViewNode->GetID()));
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
  vtkMRMLNode* sceneViewNode = this->mrmlScene()->GetNthNodeByClass(rowIndex, "vtkMRMLSceneViewNode");
  if (!sceneViewNode || !sceneViewNode->GetID())
    {
    return;
    }
  this->editSceneView(QString(sceneViewNode->GetID()));
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
  vtkMRMLNode* sceneViewNode = this->mrmlScene()->GetNthNodeByClass(rowIndex, "vtkMRMLSceneViewNode");
  if (!sceneViewNode)
    {
    return;
    }
  this->mrmlScene()->RemoveNode(sceneViewNode);
}
