#include "GUI/qSlicerAnnotationModuleWidget.h"
#include "ui_qSlicerAnnotationModule.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"



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
#include "vtkCommand.h"

#include "qMRMLSceneDisplayableModel.h"

// GUI includes
#include "GUI/qSlicerAnnotationModuleReportDialog.h"
#include "GUI/qSlicerAnnotationModulePropertyDialog.h"
#include "GUI/qSlicerAnnotationModuleSnapShotDialog.h"

// MRML includes
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLSelectionNode.h"

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()


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
  // logic - make sure the logic knows about this widget
  this->logic()->SetAndObserveWidget(q);
  this->hierarchyTreeView->hideScene();

  // connect the tree's edit property button to the widget's slot
  QObject::connect(this->hierarchyTreeView, SIGNAL(onPropertyEditButtonClicked(QString)),
                   q, SLOT(propertyEditButtonClicked(QString)));



  // annotation tools
  q->connect(this->fiducialTypeButton, SIGNAL(clicked()), q,
      SLOT(onFiducialNodeButtonClicked()));
  q->connect(this->textTypeButton, SIGNAL(clicked()), q,
      SLOT(onTextNodeButtonClicked()));
  q->connect(this->bidimensionalTypeButton, SIGNAL(clicked()), q,
      SLOT(onBidimensionalNodeButtonClicked()));
  q->connect(this->rulerTypeButton, SIGNAL(clicked()), q,
      SLOT(onRulerNodeButtonClicked()));
  q->connect(this->roiTypeButton, SIGNAL(clicked()), q,
        SLOT(onROINodeButtonClicked()));

  // mouse modes
  q->connect(this->pauseButton, SIGNAL(clicked()), q,
      SLOT(onPauseButtonClicked()));
  q->connect(this->resumeButton, SIGNAL(clicked()), q,
      SLOT(onResumeButtonClicked()));
  q->connect(this->cancelButton, SIGNAL(clicked()), q,
      SLOT(onCancelButtonClicked()));
  q->connect(this->doneButton, SIGNAL(clicked()), q,
      SLOT(onDoneButtonClicked()));

  // edit panel
  q->connect(this->selectAllButton, SIGNAL(clicked()),
        SLOT(selectAllButtonClicked()));
  q->connect(this->unselectAllButton, SIGNAL(clicked()),
        SLOT(unselectAllButtonClicked()));

  q->connect(this->visibleSelectedButton, SIGNAL(clicked()),
      SLOT(visibleSelectedButtonClicked()));
  q->connect(this->lockSelectedButton, SIGNAL(clicked()), q,
      SLOT(lockSelectedButtonClicked()));

  q->connect(this->restoreViewButton, SIGNAL(clicked()), q,
      SLOT(onRestoreViewButtonClicked()));

  q->connect(this->moveDownSelectedButton, SIGNAL(clicked()),
      SLOT(moveDownSelected()));
  q->connect(this->moveUpSelectedButton, SIGNAL(clicked()),
      SLOT(moveUpSelected()));

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
  q->connect(this->screenShot, SIGNAL(clicked()), q,
      SLOT(onSnapShotButtonClicked()));
  q->connect(this->generateReport, SIGNAL(clicked()), q,
      SLOT(onReportButtonClicked()));


  q->connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), q, SLOT(refreshTree()));

  // listen for updates to the logic to update the active hierarchy label
  q->qvtkConnect(this->logic(), vtkCommand::ModifiedEvent,
                 q, SLOT(updateActiveHierarchyLabel()));

  // update from the mrml scene
  q->refreshTree();

  // update the state according to the interaction node
  q->updateWidgetFromInteractionMode(NULL);
}

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleWidget::qSlicerAnnotationModuleWidget(QWidget* parent) :
  qSlicerAbstractModuleWidget(parent)
  , d_ptr(new qSlicerAnnotationModuleWidgetPrivate(*this))
{
  this->m_ReportDialog = 0;
  this->m_SnapShotDialog = 0;
  this->m_PropertyDialog = 0;
  this->m_CurrentAnnotationType = 0;
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

  if (this->m_SnapShotDialog)
    {
    this->m_SnapShotDialog->close();
    delete this->m_SnapShotDialog;
    this->m_SnapShotDialog = 0;
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
void qSlicerAnnotationModuleWidget::onRestoreViewButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  // TODO

  d->logic()->RestoreAnnotationView(d->hierarchyTreeView->firstSelectedNode());
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
    // we check if we have to create a new dialog..

    if (!this->m_SnapShotDialog)
      {

      // no snapshot dialog exists yet..

      this->m_SnapShotDialog = new qSlicerAnnotationModuleSnapShotDialog();

      // pass a pointer to the logic class
      this->m_SnapShotDialog->setLogic(d->logic());

      // create slots which listen to events fired by the OK and CANCEL button on the dialog
      this->connect(this->m_SnapShotDialog, SIGNAL(dialogRejected()), this,
          SLOT(snapshotRejected()));
      this->connect(this->m_SnapShotDialog, SIGNAL(dialogAccepted()), this,
          SLOT(snapshotAccepted()));

      }

    // reset all fields of the dialog
    this->m_SnapShotDialog->reset();

    // now we initialize it with existing values
    this->m_SnapShotDialog->initialize(mrmlIdArray.data());

    // in any case, show the dialog
    this->m_SnapShotDialog->open();

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

    this->m_PropertyDialog->setVisible(true);

    this->connect(this->m_PropertyDialog, SIGNAL(dialogRejected()), this,
        SLOT(propertyRestored()));
    this->connect(this->m_PropertyDialog, SIGNAL(dialogAccepted()), this,
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


// Resume, Pause, Cancel and Done buttons
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::enableMouseModeButtons()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->pauseButton->setChecked(false);
  d->resumeButton->setChecked(false);
  d->cancelButton->setChecked(false);
  d->doneButton->setChecked(false);
  d->pauseButton->setEnabled(true);
  d->resumeButton->setEnabled(true);
  d->cancelButton->setEnabled(true);
  d->doneButton->setEnabled(true);

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::disableMouseModeButtons()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->pauseButton->setChecked(false);
  d->resumeButton->setChecked(false);
  d->cancelButton->setChecked(false);
  d->doneButton->setChecked(false);
  d->pauseButton->setEnabled(false);
  d->resumeButton->setEnabled(false);
  d->cancelButton->setEnabled(false);
  d->doneButton->setEnabled(false);

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onResumeButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->pauseButton->setChecked(false);
  d->resumeButton->setChecked(true);

  bool persistent = false;
  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast(this->logic()->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
  if (interactionNode)
    {
    persistent = interactionNode->GetPlaceModePersistence() ? true : false;
    }
 
  switch (this->m_CurrentAnnotationType)
    {
    case qSlicerAnnotationModuleWidget::TextNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationTextNode", persistent);
      break;
    case qSlicerAnnotationModuleWidget::AngleNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationAngleNode", persistent);
      break;
    case qSlicerAnnotationModuleWidget::FiducialNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationFiducialNode", persistent);
      break;
    case qSlicerAnnotationModuleWidget::StickyNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationStickyNode", persistent);
      break;
    case qSlicerAnnotationModuleWidget::SplineNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationSplineNode", persistent);
      break;
    case qSlicerAnnotationModuleWidget::RulerNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationRulerNode", persistent);
      break;
    case qSlicerAnnotationModuleWidget::BidimensionalNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationBidimensionalNode", persistent);
      break;
    case qSlicerAnnotationModuleWidget::ROINode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationROINode", persistent);
      break;
    }

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onPauseButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->resumeButton->setChecked(false);
  d->pauseButton->setChecked(true);
  bool persistent = false;
  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast(this->logic()->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
  if (interactionNode)
    {
    persistent = interactionNode->GetPlaceModePersistence() ? true : false;
    }
  d->logic()->StopPlaceMode(persistent);

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onCancelButtonClicked()
{

  this->cancelOrRemoveLastAddedAnnotationNode();

  this->enableAllAnnotationTools();
  this->resetAllAnnotationTools();
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::cancelOrRemoveLastAddedAnnotationNode()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->logic()->CancelCurrentOrRemoveLastAddedAnnotationNode();

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onDoneButtonClicked()
{

  this->enableAllAnnotationTools();
  this->resetAllAnnotationTools();

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::resetAllAnnotationButtons()
{
  Q_D(qSlicerAnnotationModuleWidget);
  
  d->textTypeButton->setChecked(false);
  //d->angleTypeButton->setChecked(false);
  d->roiTypeButton->setChecked(false);
  d->fiducialTypeButton->setChecked(false);
  //d->splineTypeButton->setChecked(false);
  //d->stickyTypeButton->setChecked(false);
  d->rulerTypeButton->setChecked(false);
  d->bidimensionalTypeButton->setChecked(false);
  d->screenShot->setChecked(false);
}
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::resetAllAnnotationTools()
{
  Q_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = 0;

  this->resetAllAnnotationButtons();

  bool persistent = false;
  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast(this->logic()->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
  if (interactionNode)
    {
    persistent = interactionNode->GetPlaceModePersistence() ? true : false;
    }
  d->logic()->StopPlaceMode(persistent);

  this->disableMouseModeButtons();

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::disableAllAnnotationTools()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->textTypeButton->setEnabled(false);
  //d->angleTypeButton->setEnabled(false);
  d->roiTypeButton->setEnabled(false);
  d->fiducialTypeButton->setEnabled(false);
  //d->splineTypeButton->setEnabled(false);
  //d->stickyTypeButton->setEnabled(false);
  d->rulerTypeButton->setEnabled(false);
  d->bidimensionalTypeButton->setEnabled(false);
  d->screenShot->setEnabled(false);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::enableAllAnnotationTools()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->textTypeButton->setEnabled(true);
  //d->angleTypeButton->setEnabled(true);
  d->roiTypeButton->setEnabled(true);
  d->fiducialTypeButton->setEnabled(true);
  //d->splineTypeButton->setEnabled(true);
  //d->stickyTypeButton->setEnabled(true);
  d->rulerTypeButton->setEnabled(true);
  d->bidimensionalTypeButton->setEnabled(true);
  d->screenShot->setEnabled(true);
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
//
//
// Add methods for the annotation tools
//
//
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Sticky Node
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onStickyNodeButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::StickyNode;

  // d->logic()->SetAndObserveWidget(this);

  this->enableMouseModeButtons();
  this->onResumeButtonClicked();

  this->disableAllAnnotationTools();

  // this is a hack to export the sticky note icon
  // *sigh*
  QIcon icon = QIcon(":/Icons/AnnotationNote.png");
  QPixmap pixmap = icon.pixmap(32, 32);
  //QString tempdir = QString(std::getenv("TMPDIR"));

  QString tempdir = QString("/tmp/");
  tempdir.append("sticky.png");
  pixmap.save(tempdir);
  // end of hack

  //d->stickyTypeButton->setChecked(true);
  d->resumeButton->setChecked(true);

}

//-----------------------------------------------------------------------------
// Angle Node
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onAngleNodeButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::AngleNode;

  // d->logic()->SetAndObserveWidget(this);

  this->enableMouseModeButtons();
  this->onResumeButtonClicked();

  this->disableAllAnnotationTools();

  //d->angleTypeButton->setChecked(true);
  d->resumeButton->setChecked(true);
}

//-----------------------------------------------------------------------------
// Text Node
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onTextNodeButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::TextNode;

  // d->logic()->SetAndObserveWidget(this);

  this->enableMouseModeButtons();
  this->onResumeButtonClicked();

  this->disableAllAnnotationTools();

  d->textTypeButton->setChecked(true);
  d->resumeButton->setChecked(true);
}

//-----------------------------------------------------------------------------
// Spline Node
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onSplineNodeButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::SplineNode;

  // d->logic()->SetAndObserveWidget(this);

  this->enableMouseModeButtons();
  this->onResumeButtonClicked();

  this->disableAllAnnotationTools();

  //d->splineTypeButton->setChecked(true);
  d->resumeButton->setChecked(true);
}

//-----------------------------------------------------------------------------
// Ruler Node
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onRulerNodeButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::RulerNode;

  // d->logic()->SetAndObserveWidget(this);

  this->enableMouseModeButtons();
  this->onResumeButtonClicked();

  this->disableAllAnnotationTools();

  d->rulerTypeButton->setChecked(true);
  d->resumeButton->setChecked(true);
}

//-----------------------------------------------------------------------------
// Fiducial Node
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onFiducialNodeButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::FiducialNode;

  // d->logic()->SetAndObserveWidget(this);

  this->enableMouseModeButtons();
  this->onResumeButtonClicked();

  this->disableAllAnnotationTools();

  d->fiducialTypeButton->setChecked(true);
  d->resumeButton->setChecked(true);
}

//-----------------------------------------------------------------------------
// Bidimensional Node
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onBidimensionalNodeButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType
      = qSlicerAnnotationModuleWidget::BidimensionalNode;

  // d->logic()->SetAndObserveWidget(this);

  this->enableMouseModeButtons();
  this->onResumeButtonClicked();

  this->disableAllAnnotationTools();

  d->bidimensionalTypeButton->setChecked(true);
  d->resumeButton->setChecked(true);
}

//-----------------------------------------------------------------------------
// ROI Node
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onROINodeButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::ROINode;

  // d->logic()->SetAndObserveWidget(this);

  this->enableMouseModeButtons();
  this->onResumeButtonClicked();

  this->disableAllAnnotationTools();

  d->roiTypeButton->setChecked(true);
  d->resumeButton->setChecked(true);
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
      d->logic()->GetMRMLScene()->GetIsUpdating())
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

//-----------------------------------------------------------------------------
// Signal callback when the OK button of the snapshot dialog was clicked
void qSlicerAnnotationModuleWidget::snapshotAccepted()
{

  this->m_SnapShotDialog->setVisible(false);
  //std::cout << "Snapshot accepted" << std::endl;
}

//-----------------------------------------------------------------------------
// Signal callback when the CANCEL button of the snapshot dialog was clicked
void qSlicerAnnotationModuleWidget::snapshotRejected()
{
  this->m_SnapShotDialog->setVisible(false);
  //std::cout << "Snapshot rejected" << std::endl;
}


//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onSnapShotButtonClicked()
{

  Q_D(qSlicerAnnotationModuleWidget);

  if (!this->m_SnapShotDialog)
    {

    this->m_SnapShotDialog = new qSlicerAnnotationModuleSnapShotDialog();

    // pass a pointer to the logic class
    this->m_SnapShotDialog->setLogic(d->logic());

    // create slots which listen to events fired by the OK and CANCEL button on the dialog
    this->connect(this->m_SnapShotDialog, SIGNAL(dialogRejected()), this,
        SLOT(snapshotRejected()));
    this->connect(this->m_SnapShotDialog, SIGNAL(dialogAccepted()), this,
        SLOT(snapshotAccepted()));

    }

  this->enableAllAnnotationTools();
  this->resetAllAnnotationTools();

  // show the dialog
  this->m_SnapShotDialog->reset();
  this->m_SnapShotDialog->open();

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


  VTK_CREATE(vtkCollection,collection);

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

//-----------------------------------------------------------------------------
// Respond to changes in the interaction node
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Report dialog closed without saving
void qSlicerAnnotationModuleWidget::updateWidgetFromInteractionMode(vtkMRMLInteractionNode *interactionNode)
{
  Q_D(qSlicerAnnotationModuleWidget);
  
  vtkMRMLInteractionNode *iNode = interactionNode;
  if (iNode == NULL)
    {
    if (this->logic()->GetMRMLScene() != NULL &&
        this->logic()->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"))
      {
      iNode =
        vtkMRMLInteractionNode::SafeDownCast(
             this->logic()->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
      }
    }
  if (!iNode)
    {
    std::cout << "updateWidgetFromInteractionMode: no interaction node from which to update!" << std::endl;
    return;
    }

   // get the annotation type from the selection node
  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(
                                                                           this->logic()->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
  if (!selectionNode)
    {
    std::cerr << "updateWidgetFromInteractionNode: No selection node in the scene, no idea what we're placing." << std::endl;
    return;
    }

  QString activeAnnotationType(selectionNode->GetActiveAnnotationID());
  
  //std::cout << "updatewidgetFromInteractionMode" << std::endl;
  if (iNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
    {
    this->enableMouseModeButtons();
    this->disableAllAnnotationTools();
    d->resumeButton->setChecked(true);
    if (activeAnnotationType == QString("vtkMRMLAnnotationFiducialNode"))
      {
      this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::FiducialNode;
      d->fiducialTypeButton->setChecked(true);
      }
    else if (activeAnnotationType == QString("vtkMRMLAnnotationRulerNode"))
      {
      this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::RulerNode;
      d->rulerTypeButton->setChecked(true);
      }
    else if (activeAnnotationType == QString("vtkMRMLAnnotationBidimensionalNode"))
      {
      this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::BidimensionalNode;
      d->bidimensionalTypeButton->setChecked(true);
      }
    else if (activeAnnotationType == QString("vtkMRMLAnnotationTextNode"))
      {
      this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::TextNode;
      d->textTypeButton->setChecked(true);
      }
    else if (activeAnnotationType == QString("vtkMRMLAnnotationROINode"))
      {
      this->m_CurrentAnnotationType = qSlicerAnnotationModuleWidget::ROINode;
      d->roiTypeButton->setChecked(true);
      }
    else
      {
      std::cout << "Unhandled annotation type " << activeAnnotationType.toLatin1().data() << std::endl;
      }

    }
  else
    {
    // anything else, stop and go back to where could pick any tool
    this->resetAllAnnotationButtons();
    this->enableAllAnnotationTools();
    // unchecks them all
    this->enableMouseModeButtons();
    // then disables them
    this->disableMouseModeButtons();
    }
}

