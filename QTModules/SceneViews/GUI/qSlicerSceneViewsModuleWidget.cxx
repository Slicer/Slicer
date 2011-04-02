#include "GUI/qSlicerSceneViewsModuleWidget.h"
#include "ui_qSlicerSceneViewsModule.h"

// CTK includes
#include "ctkCollapsibleButton.h"

// QT includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>
#include <QMessageBox>
#include <QTextBrowser>
#include <QFile>
#include <QLineEdit>
#include <QFileDialog>
#include <QPrintDialog>
#include <QPrinter>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QBuffer>
#include <QImageWriter>

#include "vtkObserverManager.h"
#include "vtkCollection.h"
#include "vtkSmartPointer.h"

// GUI includes
#include "GUI/qSlicerSceneViewsModuleDialog.h"

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SceneViews
class qSlicerSceneViewsModuleWidgetPrivate: public Ui_qSlicerSceneViewsModule
{
  Q_DECLARE_PUBLIC(qSlicerSceneViewsModuleWidget);
protected:
  qSlicerSceneViewsModuleWidget* const q_ptr;
public:

  qSlicerSceneViewsModuleWidgetPrivate(qSlicerSceneViewsModuleWidget& object);
  ~qSlicerSceneViewsModuleWidgetPrivate();
  void setupUi(qSlicerWidget* widget);

  vtkSlicerSceneViewsModuleLogic* logic() const;

  QPointer<qSlicerSceneViewsModuleDialog> SceneViewDialog;
};

//-----------------------------------------------------------------------------
vtkSlicerSceneViewsModuleLogic*
qSlicerSceneViewsModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerSceneViewsModuleWidget);
  return vtkSlicerSceneViewsModuleLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleWidgetPrivate::qSlicerSceneViewsModuleWidgetPrivate(qSlicerSceneViewsModuleWidget& object)
  : q_ptr(&object)
{
  this->SceneViewDialog = 0;
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
// qSlicerSceneViewsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  this->Ui_qSlicerSceneViewsModule::setupUi(widget);
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleWidget::qSlicerSceneViewsModuleWidget(QWidget* parent) :
  qSlicerAbstractModuleWidget(parent)
  , d_ptr(new qSlicerSceneViewsModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleWidget::~qSlicerSceneViewsModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::setup()
{
  Q_D(qSlicerSceneViewsModuleWidget);
  d->setupUi(this);

  // setup the hierarchy treeWidget
  d->hierarchyTreeWidget->setAndObserveWidget(this);
  d->hierarchyTreeWidget->setAndObserveLogic(d->logic());
  d->hierarchyTreeWidget->setMRMLScene(this->logic()->GetMRMLScene());
  d->hierarchyTreeWidget->hideScene();

  this->connect(d->moveDownSelectedButton, SIGNAL(clicked()),
      SLOT(moveDownSelected()));
  this->connect(d->moveUpSelectedButton, SIGNAL(clicked()),
      SLOT(moveUpSelected()));

  this->connect(d->deleteSelectedButton, SIGNAL(clicked()),
                d->hierarchyTreeWidget, SLOT(deleteSelected()));

  this->connect(d->sceneView, SIGNAL(clicked()), this,
      SLOT(showSceneViewDialog()));
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::moveDownSelected()
{
  Q_D(qSlicerSceneViewsModuleWidget);

  const char* id = d->logic()->MoveSceneViewDown(
    d->hierarchyTreeWidget->firstSelectedNode().toLatin1());

  d->hierarchyTreeWidget->clearSelection();
  d->hierarchyTreeWidget->setSelectedNode(id);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::moveUpSelected()
{
  Q_D(qSlicerSceneViewsModuleWidget);

  const char* id = d->logic()->MoveSceneViewUp(
    d->hierarchyTreeWidget->firstSelectedNode().toLatin1());

  d->hierarchyTreeWidget->clearSelection();
  d->hierarchyTreeWidget->setSelectedNode(id);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::restoreSceneView(const QString& mrmlId)
{
  Q_D(qSlicerSceneViewsModuleWidget);

  d->logic()->RestoreSceneView(mrmlId.toLatin1());

  QMessageBox::information(this, "3D Slicer SceneView updated",
                                 "The SceneView was restored including the attached scene.");
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::editSceneView(const QString& mrmlId)
{
  Q_D(qSlicerSceneViewsModuleWidget);

  this->showSceneViewDialog();
  if (d->SceneViewDialog)
    {
    // now we initialize it with existing values
    d->SceneViewDialog->initialize(mrmlId);
    }
}

//-----------------------------------------------------------------------------
// Refresh the hierarchy tree after an sceneView was added or modified.
// Just do some layout changes - nothing special!
//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::refreshTree()
{
  Q_D(qSlicerSceneViewsModuleWidget);
  d->hierarchyTreeWidget->setMRMLScene(d->logic()->GetMRMLScene());
  d->hierarchyTreeWidget->hideScene();
}

//-----------------------------------------------------------------------------
// SceneView functionality
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::showSceneViewDialog()
{
  Q_D(qSlicerSceneViewsModuleWidget);

  if (!d->SceneViewDialog)
    {

    d->SceneViewDialog = new qSlicerSceneViewsModuleDialog();

    // pass a pointer to the logic class
    d->SceneViewDialog->setLogic(d->logic());

    // create slots which listen to events fired by the OK and CANCEL button on the dialog
    this->connect(d->SceneViewDialog, SIGNAL(rejected()),
                  d->SceneViewDialog, SLOT(hide()));
    this->connect(d->SceneViewDialog, SIGNAL(accepted()),
                  d->SceneViewDialog, SLOT(hide()));
    }

  // show the dialog
  d->SceneViewDialog->reset();
  d->SceneViewDialog->open();
}

