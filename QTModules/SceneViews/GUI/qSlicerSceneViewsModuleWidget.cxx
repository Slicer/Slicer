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
class qSlicerSceneViewsModuleWidgetPrivate: public Ui_qSlicerSceneViewsModule
{
  Q_DECLARE_PUBLIC(qSlicerSceneViewsModuleWidget);
protected:
  qSlicerSceneViewsModuleWidget* const q_ptr;
public:

  qSlicerSceneViewsModuleWidgetPrivate(qSlicerSceneViewsModuleWidget& object);
  ~qSlicerSceneViewsModuleWidgetPrivate();
  void
  setupUi(qSlicerWidget* widget);


  vtkSlicerSceneViewLogic*
  logic() const;


protected slots:

protected:

private:


};

//-----------------------------------------------------------------------------
vtkSlicerSceneViewLogic*
qSlicerSceneViewsModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerSceneViewsModuleWidget);
  return vtkSlicerSceneViewLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleWidgetPrivate::qSlicerSceneViewsModuleWidgetPrivate(qSlicerSceneViewsModuleWidget& object)
  : q_ptr(&object)
{

}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleWidgetPrivate::~qSlicerSceneViewsModuleWidgetPrivate()
{

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

  this->m_SceneViewDialog = 0;


}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleWidget::~qSlicerSceneViewsModuleWidget()
{

  if (this->m_SceneViewDialog)
    {
    this->m_SceneViewDialog->close();
    delete this->m_SceneViewDialog;
    this->m_SceneViewDialog = 0;
    }


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
      SLOT(deleteSelectedButtonClicked()));

  this->connect(d->sceneView, SIGNAL(clicked()), this,
      SLOT(onSceneViewButtonClicked()));

}



//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::moveDownSelected()
{
  Q_D(qSlicerSceneViewsModuleWidget);

  const char* id = d->logic()->MoveSceneViewDown(d->hierarchyTreeWidget->firstSelectedNode());

  d->hierarchyTreeWidget->clearSelection();
  d->hierarchyTreeWidget->setSelectedNode(id);

}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::moveUpSelected()
{
  Q_D(qSlicerSceneViewsModuleWidget);

  const char* id = d->logic()->MoveSceneViewUp(d->hierarchyTreeWidget->firstSelectedNode());

  d->hierarchyTreeWidget->clearSelection();
  d->hierarchyTreeWidget->setSelectedNode(id);

}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::restoreButtonClicked(QString mrmlId)
{

  Q_D(qSlicerSceneViewsModuleWidget);

  QByteArray mrmlIdArray = mrmlId.toLatin1();

  d->logic()->RestoreSceneView(mrmlIdArray.data());

  QMessageBox::information(this, "3D Slicer SceneView updated",
                                 "The SceneView was restored including the attached scene.");


}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::propertyEditButtonClicked(QString mrmlId)
{
  Q_D(qSlicerSceneViewsModuleWidget);

  QByteArray mrmlIdArray = mrmlId.toLatin1();

  // the selected entry is a sceneView node,
  // we check if we have to create a new dialog..

  if (!this->m_SceneViewDialog)
    {

    // no sceneView dialog exists yet..
    this->m_SceneViewDialog = new qSlicerSceneViewsModuleDialog();

    // pass a pointer to the logic class
    this->m_SceneViewDialog->setLogic(d->logic());

    // create slots which listen to events fired by the OK and CANCEL button on the dialog
    this->connect(this->m_SceneViewDialog, SIGNAL(dialogRejected()), this,
        SLOT(sceneViewRejected()));
    this->connect(this->m_SceneViewDialog, SIGNAL(dialogAccepted()), this,
        SLOT(sceneViewAccepted()));

    }

  // reset all fields of the dialog
  this->m_SceneViewDialog->reset();

  // now we initialize it with existing values
  this->m_SceneViewDialog->initialize(mrmlIdArray.data());

  // in any case, show the dialog
  this->m_SceneViewDialog->open();

}


//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::deleteSelectedButtonClicked()
{

  Q_D(qSlicerSceneViewsModuleWidget);

  d->hierarchyTreeWidget->deleteSelected();

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
// Signal callback when the OK button of the sceneView dialog was clicked
void qSlicerSceneViewsModuleWidget::sceneViewAccepted()
{

  this->m_SceneViewDialog->setVisible(false);
  //std::cout << "SceneView accepted" << std::endl;
}

//-----------------------------------------------------------------------------
// Signal callback when the CANCEL button of the sceneView dialog was clicked
void qSlicerSceneViewsModuleWidget::sceneViewRejected()
{
  this->m_SceneViewDialog->setVisible(false);
  //std::cout << "SceneView rejected" << std::endl;
}


//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::onSceneViewButtonClicked()
{

  Q_D(qSlicerSceneViewsModuleWidget);

  if (!this->m_SceneViewDialog)
    {

    this->m_SceneViewDialog = new qSlicerSceneViewsModuleDialog();

    // pass a pointer to the logic class
    this->m_SceneViewDialog->setLogic(d->logic());

    // create slots which listen to events fired by the OK and CANCEL button on the dialog
    this->connect(this->m_SceneViewDialog, SIGNAL(dialogRejected()), this,
        SLOT(sceneViewRejected()));
    this->connect(this->m_SceneViewDialog, SIGNAL(dialogAccepted()), this,
        SLOT(sceneViewAccepted()));

    }

  // show the dialog
  this->m_SceneViewDialog->reset();
  this->m_SceneViewDialog->open();



}


//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::showSceneViewDialog()
{
  this->onSceneViewButtonClicked();
}
