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

  this->m_SnapShotDialog = 0;


}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleWidget::~qSlicerSceneViewsModuleWidget()
{

  if (this->m_SnapShotDialog)
    {
    this->m_SnapShotDialog->close();
    delete this->m_SnapShotDialog;
    this->m_SnapShotDialog = 0;
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

  this->connect(d->screenShot, SIGNAL(clicked()), this,
      SLOT(onSnapShotButtonClicked()));

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

  // the selected entry is a snapshot node,
  // we check if we have to create a new dialog..

  if (!this->m_SnapShotDialog)
    {

    // no snapshot dialog exists yet..
    this->m_SnapShotDialog = new qSlicerSceneViewsModuleDialog();

    // pass a pointer to the logic class
    this->m_SnapShotDialog->setLogic(d->logic());

    // create slots which listen to events fired by the OK and CANCEL button on the dialog
    this->connect(this->m_SnapShotDialog, SIGNAL(dialogRejected()), this,
        SLOT(snapshotRejected()));
    this->connect(this->m_SnapShotDialog, SIGNAL(dialogAccepted()), this,
        SLOT(snapshotAccepted()));

    }

  // in any case, show the dialog
  this->m_SnapShotDialog->setVisible(true);

  // reset all fields of the dialog
  this->m_SnapShotDialog->reset();

  // now we initialize it with existing values
  this->m_SnapShotDialog->initialize(mrmlIdArray.data());

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
// Signal callback when the OK button of the snapshot dialog was clicked
void qSlicerSceneViewsModuleWidget::snapshotAccepted()
{

  this->m_SnapShotDialog->setVisible(false);
  //std::cout << "Snapshot accepted" << std::endl;
}

//-----------------------------------------------------------------------------
// Signal callback when the CANCEL button of the snapshot dialog was clicked
void qSlicerSceneViewsModuleWidget::snapshotRejected()
{
  this->m_SnapShotDialog->setVisible(false);
  //std::cout << "Snapshot rejected" << std::endl;
}


//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::onSnapShotButtonClicked()
{

  Q_D(qSlicerSceneViewsModuleWidget);

  if (!this->m_SnapShotDialog)
    {

    this->m_SnapShotDialog = new qSlicerSceneViewsModuleDialog();

    // pass a pointer to the logic class
    this->m_SnapShotDialog->setLogic(d->logic());

    // create slots which listen to events fired by the OK and CANCEL button on the dialog
    this->connect(this->m_SnapShotDialog, SIGNAL(dialogRejected()), this,
        SLOT(snapshotRejected()));
    this->connect(this->m_SnapShotDialog, SIGNAL(dialogAccepted()), this,
        SLOT(snapshotAccepted()));

    }

  // show the dialog
  this->m_SnapShotDialog->setVisible(true);
  this->m_SnapShotDialog->raise();
  this->m_SnapShotDialog->activateWindow();
  this->m_SnapShotDialog->reset();



}

