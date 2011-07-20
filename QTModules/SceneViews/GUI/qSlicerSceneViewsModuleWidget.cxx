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
#include <QMainWindow>
#include <QStatusBar>

#include "vtkObserverManager.h"
#include "vtkCollection.h"
#include "vtkSmartPointer.h"

// GUI includes
#include "GUI/qSlicerSceneViewsModuleDialog.h"

#include "qSlicerApplication.h"

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
// qSlicerSceneViewsModuleWidgetPrivate methods

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
void qSlicerSceneViewsModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  Q_Q(qSlicerSceneViewsModuleWidget);
  this->Ui_qSlicerSceneViewsModule::setupUi(widget);

  QObject::connect(this->hierarchyTreeView,
                   SIGNAL(restoreSceneViewRequested(const QString&)),
                   q, SLOT(restoreSceneView(const QString&)));
  QObject::connect(this->hierarchyTreeView,
                   SIGNAL(editSceneViewRequested(const QString&)),
                   q, SLOT(editSceneView(const QString&)));

  // setup the hierarchy treeWidget
  this->hierarchyTreeView->setLogic(this->logic());
  this->hierarchyTreeView->setMRMLScene(this->logic()->GetMRMLScene());
  this->logic()->SetAndObserveWidget(q);
  this->hierarchyTreeView->hideScene();

  q->connect(this->moveDownSelectedButton, SIGNAL(clicked()),
             q, SLOT(moveDownSelected()));
  q->connect(this->moveUpSelectedButton, SIGNAL(clicked()),
             q, SLOT(moveUpSelected()));

  QObject::connect(this->deleteSelectedButton, SIGNAL(clicked()),
                   this->hierarchyTreeView, SLOT(deleteSelected()));

  QObject::connect(this->sceneView, SIGNAL(clicked()),
                   q, SLOT(showSceneViewDialog()));

  q->connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), q, SLOT(refreshTree()));

  // update from the mrml scene
  q->refreshTree();
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
qSlicerSceneViewsModuleWidget::~qSlicerSceneViewsModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::setup()
{
  Q_D(qSlicerSceneViewsModuleWidget);
  this->Superclass::setup();
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::moveDownSelected()
{
  Q_D(qSlicerSceneViewsModuleWidget);

  const char* id = d->logic()->MoveSceneViewDown(
    d->hierarchyTreeView->firstSelectedNode().toLatin1());

  d->hierarchyTreeView->clearSelection();
  d->hierarchyTreeView->setSelectedNode(id);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::moveUpSelected()
{
  Q_D(qSlicerSceneViewsModuleWidget);

  const char* id = d->logic()->MoveSceneViewUp(
    d->hierarchyTreeView->firstSelectedNode().toLatin1());

  d->hierarchyTreeView->clearSelection();
  d->hierarchyTreeView->setSelectedNode(id);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::restoreSceneView(const QString& mrmlId)
{
  Q_D(qSlicerSceneViewsModuleWidget);

  d->logic()->RestoreSceneView(mrmlId.toLatin1());

  qSlicerApplication::application()->mainWindow()->statusBar()->showMessage("The SceneView was restored including the attached scene.", 2000);
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

  if (d->logic() && d->logic()->GetMRMLScene() &&
      d->logic()->GetMRMLScene()->GetIsUpdating())
    {
    // scene is updating, return
    return;
    }

  d->hierarchyTreeView->setMRMLScene(d->logic()->GetMRMLScene());
  d->hierarchyTreeView->hideScene();
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

