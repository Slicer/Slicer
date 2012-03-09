#include "GUI/qSlicerAnnotationModuleWidget.h"
#include "ui_qSlicerAnnotationModule.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"



// CTK includes
#include "ctkCollapsibleButton.h"
// QT includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
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

#include "qSlicerMouseModeToolBar.h"
#include "qMRMLSceneDisplayableModel.h"

// GUI includes
#include "GUI/qSlicerAnnotationModuleReportDialog.h"
#include "GUI/qSlicerAnnotationModulePropertyDialog.h"
#include "GUI/qSlicerAnnotationModuleSnapShotDialog.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

// MRML includes
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLSelectionNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Annotation
class qSlicerAnnotationModuleWidgetPrivate: public Ui_qSlicerAnnotationModule
{
  Q_DECLARE_PUBLIC(qSlicerAnnotationModuleWidget);
protected:
  qSlicerAnnotationModuleWidget* const q_ptr;
public:

  qSlicerAnnotationModuleWidgetPrivate(qSlicerAnnotationModuleWidget& object);
  ~qSlicerAnnotationModuleWidgetPrivate();

  void setupUi(qSlicerWidget* widget);

  vtkSlicerAnnotationModuleLogic* logic() const;

  qSlicerAnnotationModuleSnapShotDialog* m_SnapShotDialog;
};

//-----------------------------------------------------------------------------
vtkSlicerAnnotationModuleLogic*
qSlicerAnnotationModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerAnnotationModuleWidget);
  return vtkSlicerAnnotationModuleLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleWidgetPrivate::qSlicerAnnotationModuleWidgetPrivate(qSlicerAnnotationModuleWidget& object)
  : q_ptr(&object)
{
  this->m_SnapShotDialog = 0;
}

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleWidgetPrivate::~qSlicerAnnotationModuleWidgetPrivate()
{

}

//-----------------------------------------------------------------------------
// qSlicerAnnotationModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  Q_Q(qSlicerAnnotationModuleWidget);

  this->Ui_qSlicerAnnotationModule::setupUi(widget);


  // setup the hierarchy treeWidget
  this->hierarchyTreeView->setLogic(this->logic());
  this->hierarchyTreeView->setMRMLScene(this->logic()->GetMRMLScene());

  this->hierarchyTreeView->hideScene();
  // enable scrolling when drag past end of window
  this->hierarchyTreeView->setFitSizeToVisibleIndexes(false);
  
  // connect the tree's edit property button to the widget's slot
  QObject::connect(this->hierarchyTreeView, SIGNAL(onPropertyEditButtonClicked(QString)),
                   q, SLOT(propertyEditButtonClicked(QString)));

  // edit panel
  q->connect(this->selectAllButton, SIGNAL(clicked()),
        SLOT(selectAllButtonClicked()));
  q->connect(this->unselectAllButton, SIGNAL(clicked()),
        SLOT(unselectAllButtonClicked()));

  q->connect(this->visibleSelectedButton, SIGNAL(clicked()),
      SLOT(visibleSelectedButtonClicked()));
  q->connect(this->lockSelectedButton, SIGNAL(clicked()), q,
      SLOT(lockSelectedButtonClicked()));

  q->connect(this->jumpSlicesButton, SIGNAL(clicked()), q,
      SLOT(onJumpSlicesButtonClicked()));

  /*
  q->connect(this->moveDownSelectedButton, SIGNAL(clicked()),
      SLOT(moveDownSelected()));
  q->connect(this->moveUpSelectedButton, SIGNAL(clicked()),
      SLOT(moveUpSelected()));
  */
  q->connect(this->addHierarchyButton, SIGNAL(clicked()),
      SLOT(onAddHierarchyButtonClicked()));
  q->connect(this->deleteSelectedButton, SIGNAL(clicked()),
      SLOT(deleteSelectedButtonClicked()));

  // active list
  q->connect(this->visibleHierarchyButton, SIGNAL(clicked()), q,
      SLOT(visibleHierarchyButtonClicked()));
  q->connect(this->invisibleHierarchyButton, SIGNAL(clicked()), q,
      SLOT(invisibleHierarchyButtonClicked()));
  q->connect(this->lockHierarchyButton, SIGNAL(clicked()), q,
      SLOT(lockHierarchyButtonClicked()));
  q->connect(this->unlockHierarchyButton, SIGNAL(clicked()), q,
      SLOT(unlockHierarchyButtonClicked()));

  // Save Panel
  q->connect(this->generateReport, SIGNAL(clicked()), q,
      SLOT(onReportButtonClicked()));


  q->connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), q, SLOT(refreshTree()));

  // listen for updates to the logic to update the active hierarchy label
  q->qvtkConnect(this->logic(), vtkCommand::ModifiedEvent,
                 q, SLOT(updateActiveHierarchyLabel()));
  q->qvtkConnect(this->logic(), vtkSlicerAnnotationModuleLogic::RefreshRequestEvent,
                 q, SLOT(refreshTree()));

  // update from the mrml scene
  q->refreshTree();
}

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleWidget::qSlicerAnnotationModuleWidget(QWidget* parent) :
  qSlicerAbstractModuleWidget(parent)
  , d_ptr(new qSlicerAnnotationModuleWidgetPrivate(*this))
{
  Q_D(qSlicerAnnotationModuleWidget);
  this->m_ReportDialog = 0;
  this->m_PropertyDialog = 0;
  this->m_CurrentAnnotationType = 0;

  d->m_SnapShotDialog = new qSlicerAnnotationModuleSnapShotDialog(this);
}

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleWidget::~qSlicerAnnotationModuleWidget()
{
  if (this->m_ReportDialog)
    {
    this->m_ReportDialog->close();
    delete this->m_ReportDialog;
    this->m_ReportDialog = 0;
    }

  if (this->m_PropertyDialog)
    {
    this->m_PropertyDialog->close();
    delete this->m_PropertyDialog;
    this->m_PropertyDialog = 0;
    }
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::setup()
{
  Q_D(qSlicerAnnotationModuleWidget);
  this->Superclass::setup();
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::moveDownSelected()
{
  Q_D(qSlicerAnnotationModuleWidget);

  const char* mrmlId = d->logic()->MoveAnnotationDown(d->hierarchyTreeView->firstSelectedNode());

  d->hierarchyTreeView->clearSelection();
  d->hierarchyTreeView->setSelectedNode(mrmlId);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::moveUpSelected()
{
  Q_D(qSlicerAnnotationModuleWidget);

  const char* mrmlId = d->logic()->MoveAnnotationUp(d->hierarchyTreeView->firstSelectedNode());

  d->hierarchyTreeView->clearSelection();
  d->hierarchyTreeView->setSelectedNode(mrmlId);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::selectAllButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->hierarchyTreeView->selectAll();
  d->logic()->SetAllAnnotationsSelected(1);
  d->logic()->SetActiveHierarchyNodeID(NULL);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::unselectAllButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->hierarchyTreeView->clearSelection();
  d->logic()->SetAllAnnotationsSelected(0);
  d->logic()->SetActiveHierarchyNodeID(NULL);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onJumpSlicesButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->logic()->JumpSlicesToAnnotationCoordinate(d->hierarchyTreeView->firstSelectedNode());
}

//-----------------------------------------------------------------------------
//
// Property dialog
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::propertyEditButtonClicked(QString mrmlId)
{
  Q_D(qSlicerAnnotationModuleWidget);

  QByteArray mrmlIdArray = mrmlId.toLatin1();

  // special case for snapshots
  if (d->logic()->IsSnapshotNode(mrmlIdArray.data()))
    {

    // the selected entry is a snapshot node,

    // pass a pointer to the logic class
    d->m_SnapShotDialog->setLogic(d->logic());

    // reset all fields of the dialog
    d->m_SnapShotDialog->reset();

    // now we initialize it with existing values
    d->m_SnapShotDialog->loadNode(mrmlIdArray.data());

    // in any case, show the dialog
    d->m_SnapShotDialog->open();

    // bail out, everything below is not for snapshots
    return;
    }
  // end of special case for snapshots

  // check if there exists an annotationNode with the given ID
  // only then display the property dialog
  if (d->logic()->IsAnnotationNode(mrmlIdArray.data()) || d->logic()->IsAnnotationHierarchyNode(mrmlIdArray.data()))
    {

    if (this->m_PropertyDialog)
      {
      //QMessageBox::warning(d->hierarchyTreeView,
      //    QString("Modify Annotation Properties"), QString(
      //        "The property dialog is already open."));

      this->m_PropertyDialog->activateWindow();

      return;
      }

    // TODO
    //d->setItemEditable(d->tableWidget->selectedItems(), false);

    qSlicerAnnotationModulePropertyDialog* propertyDialog =
        new qSlicerAnnotationModulePropertyDialog(mrmlIdArray.data(), d->logic());

    this->m_PropertyDialog = propertyDialog;

    // TODO: update the property dialog elements when the mrml node changes,
    // for now just make the dialog modal so can't change the annotations
    // while have the dialog open
    this->m_PropertyDialog->setModal(true);
    
    this->m_PropertyDialog->setVisible(true);

    this->connect(this->m_PropertyDialog, SIGNAL(rejected()), this,
        SLOT(propertyRestored()));
    this->connect(this->m_PropertyDialog, SIGNAL(accepted()), this,
        SLOT(propertyAccepted()));

    }
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::propertyRestored()
{

  const char * mrmlID = this->m_PropertyDialog->GetID();
  Q_D(qSlicerAnnotationModuleWidget);

  // TODO?

  d->logic()->SetAnnotationSelected(mrmlID, false);

  //delete this->m_PropertyDialog;
  this->m_PropertyDialog = 0;

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::propertyAccepted()
{

  const char * mrmlID = this->m_PropertyDialog->GetID();
  Q_D(qSlicerAnnotationModuleWidget);

  d->logic()->SetAnnotationSelected(mrmlID,false);

  // TODO?


  //delete this->m_PropertyDialog;
  this->m_PropertyDialog = 0;

}


//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::visibleSelectedButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->hierarchyTreeView->toggleVisibilityForSelected();

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::lockSelectedButtonClicked()
{

  Q_D(qSlicerAnnotationModuleWidget);

  d->hierarchyTreeView->toggleLockForSelected();
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::deleteSelectedButtonClicked()
{

  Q_D(qSlicerAnnotationModuleWidget);

  d->hierarchyTreeView->deleteSelected();

}

// Active list buttons
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::invisibleHierarchyButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->logic()->SetHierarchyAnnotationsVisibleFlag(NULL, false);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::visibleHierarchyButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->logic()->SetHierarchyAnnotationsVisibleFlag(NULL, true);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::lockHierarchyButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->logic()->SetHierarchyAnnotationsLockFlag(NULL, true);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::unlockHierarchyButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->logic()->SetHierarchyAnnotationsLockFlag(NULL, false);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onAddHierarchyButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);
  d->logic()->SetActiveHierarchyNodeID(d->hierarchyTreeView->firstSelectedNode());
  this->refreshTree();
  if (d->logic()->AddHierarchy())
    {
    d->hierarchyTreeView->setSelectedNode(d->logic()->GetActiveHierarchyNodeID());
    }
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::updateActiveHierarchyLabel()
{
  Q_D(qSlicerAnnotationModuleWidget);

  const char *id = d->logic()->GetActiveHierarchyNodeID();
  //QString idString = QString(" (none)");
  QString name = QString("(none)");
  if (id)
    {
    //idString = QString(" (") + QString(id) + QString(")");
    if (d->logic() && d->logic()->GetMRMLScene() && d->logic()->GetMRMLScene()->GetNodeByID(id))
      {
      name = QString(d->logic()->GetMRMLScene()->GetNodeByID(id)->GetName());
      }
    }
  d->activeHierarchyLabel->setText(QString("Active list: ") + name);
}

//-----------------------------------------------------------------------------
// Refresh the hierarchy tree after an annotation was added or modified.
// Just do some layout changes - nothing special!
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::refreshTree()
{
  Q_D(qSlicerAnnotationModuleWidget);

  if (d->logic() && d->logic()->GetMRMLScene() &&
      d->logic()->GetMRMLScene()->IsBatchProcessing())
    {
    // scene is updating, return
    return;
    }
  // this gets called on scene closed, can we make sure that the widget is
  // visible?
  
  d->hierarchyTreeView->setMRMLScene(d->logic()->GetMRMLScene());
  d->hierarchyTreeView->hideScene();
}

//-----------------------------------------------------------------------------
// Annotation SnapShot functionality

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::grabSnapShot()
{
  Q_D(qSlicerAnnotationModuleWidget);

  // show the dialog
  d->m_SnapShotDialog->setLogic(d->logic());
  d->m_SnapShotDialog->reset();
  d->m_SnapShotDialog->open();
}


//-----------------------------------------------------------------------------
// Annotation Report functionality
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Show the report dialog
void qSlicerAnnotationModuleWidget::onReportButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  if (!this->m_ReportDialog)
    {

    this->m_ReportDialog = new qSlicerAnnotationModuleReportDialog();

    // pass a pointer to the logic class
    this->m_ReportDialog->setLogic(d->logic());

    // create slots which listen to events fired by the OK and CANCEL button on the dialog
    this->connect(this->m_ReportDialog, SIGNAL(dialogRejected()), this,
        SLOT(reportDialogRejected()));
    this->connect(this->m_ReportDialog, SIGNAL(dialogAccepted()), this,
        SLOT(reportDialogAccepted()));
    }

  vtkNew<vtkCollection> collection;

  d->hierarchyTreeView->selectedAsCollection(collection.GetPointer());

  // if nothing was selected, select all
  if(collection->GetNumberOfItems() == 0)
    {
    d->hierarchyTreeView->selectAll();
    d->hierarchyTreeView->selectedAsCollection(collection.GetPointer());
    }

  this->m_ReportDialog->setAnnotations(collection.GetPointer());

  this->m_ReportDialog->updateReport();

  this->m_ReportDialog->setVisible(true);

  this->m_ReportDialog->raise();
  this->m_ReportDialog->activateWindow();

}

//-----------------------------------------------------------------------------
// Report dialog closed after saving
void qSlicerAnnotationModuleWidget::reportDialogAccepted()
{

  this->m_ReportDialog->setVisible(false);

}

//-----------------------------------------------------------------------------
// Report dialog closed without saving
void qSlicerAnnotationModuleWidget::reportDialogRejected()
{

  this->m_ReportDialog->setVisible(false);

}

