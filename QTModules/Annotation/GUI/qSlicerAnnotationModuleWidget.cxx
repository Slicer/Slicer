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

#include "qMRMLSceneDisplayableModel.h"

// GUI includes
#include "GUI/qSlicerAnnotationModuleReportDialog.h"
#include "GUI/qSlicerAnnotationModulePropertyDialog.h"
#include "GUI/qSlicerAnnotationModuleSnapShotDialog.h"

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()


//-----------------------------------------------------------------------------
class qSlicerAnnotationModuleWidgetPrivate: public Ui_qSlicerAnnotationModule
{
  Q_DECLARE_PUBLIC(qSlicerAnnotationModuleWidget);
protected:
  qSlicerAnnotationModuleWidget* const q_ptr;
public:

  qSlicerAnnotationModuleWidgetPrivate(qSlicerAnnotationModuleWidget& object);
  ~qSlicerAnnotationModuleWidgetPrivate();
  void
  setupUi(qSlicerWidget* widget);


  vtkSlicerAnnotationModuleLogic*
  logic() const;


protected slots:

protected:

private:


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
  this->Ui_qSlicerAnnotationModule::setupUi(widget);
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
  d->setupUi(this);

  // setup the hierarchy treeWidget
  d->hierarchyTreeWidget->setAndObserveWidget(this);
  d->hierarchyTreeWidget->setAndObserveLogic(d->logic());
  d->hierarchyTreeWidget->setMRMLScene(this->logic()->GetMRMLScene());
  d->hierarchyTreeWidget->hideScene();

  // annotation tools
  this->connect(d->fiducialTypeButton, SIGNAL(clicked()), this,
      SLOT(onFiducialNodeButtonClicked()));
  this->connect(d->textTypeButton, SIGNAL(clicked()), this,
      SLOT(onTextNodeButtonClicked()));
  this->connect(d->bidimensionalTypeButton, SIGNAL(clicked()), this,
      SLOT(onBidimensionalNodeButtonClicked()));
  this->connect(d->rulerTypeButton, SIGNAL(clicked()), this,
      SLOT(onRulerNodeButtonClicked()));

  // mouse modes
  this->connect(d->pauseButton, SIGNAL(clicked()), this,
      SLOT(onPauseButtonClicked()));
  this->connect(d->resumeButton, SIGNAL(clicked()), this,
      SLOT(onResumeButtonClicked()));
  this->connect(d->cancelButton, SIGNAL(clicked()), this,
      SLOT(onCancelButtonClicked()));
  this->connect(d->doneButton, SIGNAL(clicked()), this,
      SLOT(onDoneButtonClicked()));

  // edit panel
  this->connect(d->selectAllButton, SIGNAL(clicked()),
        SLOT(selectAllButtonClicked()));
  this->connect(d->visibleSelectedButton, SIGNAL(clicked()),
      SLOT(visibleSelectedButtonClicked()));
  this->connect(d->lockSelectedButton, SIGNAL(clicked()), this,
      SLOT(lockSelectedButtonClicked()));

  this->connect(d->restoreViewButton, SIGNAL(clicked()), this,
      SLOT(onRestoreViewButtonClicked()));

  this->connect(d->moveDownSelectedButton, SIGNAL(clicked()),
      SLOT(moveDownSelected()));
  this->connect(d->moveUpSelectedButton, SIGNAL(clicked()),
      SLOT(moveUpSelected()));

  this->connect(d->addHierarchyButton, SIGNAL(clicked()),
      SLOT(onAddHierarchyButtonClicked()));
  this->connect(d->deleteSelectedButton, SIGNAL(clicked()),
      SLOT(deleteSelectedButtonClicked()));

  // Save Panel
  this->connect(d->saveScene, SIGNAL(clicked()),
      SLOT(onSaveMRMLSceneButtonClicked()));
  this->connect(d->saveAnnotation, SIGNAL(clicked()), this,
      SLOT(onSaveAnnotationButtonClicked()));
  this->connect(d->screenShot, SIGNAL(clicked()), this,
      SLOT(onSnapShotButtonClicked()));
  this->connect(d->generateReport, SIGNAL(clicked()), this,
      SLOT(onReportButtonClicked()));

}



//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::moveDownSelected()
{
  Q_D(qSlicerAnnotationModuleWidget);

  const char* mrmlId = d->logic()->MoveAnnotationDown(d->hierarchyTreeWidget->firstSelectedNode());

  d->hierarchyTreeWidget->clearSelection();
  d->hierarchyTreeWidget->setSelectedNode(mrmlId);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::moveUpSelected()
{
  Q_D(qSlicerAnnotationModuleWidget);

  const char* mrmlId = d->logic()->MoveAnnotationUp(d->hierarchyTreeWidget->firstSelectedNode());

  d->hierarchyTreeWidget->clearSelection();
  d->hierarchyTreeWidget->setSelectedNode(mrmlId);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onSaveMRMLSceneButtonClicked()
{
  //Q_D(qSlicerAnnotationModuleWidget);


}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::selectAllButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->hierarchyTreeWidget->selectAll();
  d->logic()->SetActiveHierarchyNode(0);

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onRestoreViewButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  // TODO

  d->logic()->RestoreAnnotationView(d->hierarchyTreeWidget->firstSelectedNode());

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

      // just make sure the logic knows about this widget
      d->logic()->SetAndObserveWidget(this);

      // be sure to listen to the mrml events
      // this only has to be called if no real annotations were placed yet
      // double call does not hurt..
      d->logic()->InitializeEventListeners();

      this->m_SnapShotDialog = new qSlicerAnnotationModuleSnapShotDialog();

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

    // bail out, everything below is not for snapshots
    return;
    }
  // end of special case for snapshots

  // TODO hierarchies

  // check if there exists an annotationNode with the given ID
  // only then display the property dialog
  if (d->logic()->IsAnnotationNode(mrmlIdArray.data()))
    {

    if (this->m_PropertyDialog)
      {
      QMessageBox::warning(d->hierarchyTreeWidget,
          QString("Modify Annotation Properties"), QString(
              "The property dialog is already open."));

      return;
      }

    d->logic()->SetAnnotationSelected(mrmlIdArray.data(), true);

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
void qSlicerAnnotationModuleWidget::onSaveAnnotationButtonClicked()
{

  // TODO
  /*
  QString filename = QFileDialog::getSaveFileName(this, "Save Annotation",
      QDir::currentPath(), "Annotations (*.txt)");

  // save the documents...
  if (!filename.isNull())
    {
    QFile file(filename);
    if (!file.open(QFile::WriteOnly | QFile::Text))
      {
      std::cerr << "Error: Cannot save file " << qPrintable(filename) << ": "
          << qPrintable(file.errorString()) << std::endl;
      return;
      }

    QTextStream out(&file);
    // m_report has the contents for output
    out << m_report;

    }

    */

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::visibleSelectedButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->hierarchyTreeWidget->toggleVisibilityForSelected();

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::lockSelectedButtonClicked()
{

  Q_D(qSlicerAnnotationModuleWidget);

  d->hierarchyTreeWidget->toggleLockForSelected();
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::deleteSelectedButtonClicked()
{

  Q_D(qSlicerAnnotationModuleWidget);

  d->hierarchyTreeWidget->deleteSelected();

}

//-----------------------------------------------------------------------------
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

  switch (this->m_CurrentAnnotationType)
    {
    case qSlicerAnnotationModuleWidget::TextNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationTextNode");
      break;
    case qSlicerAnnotationModuleWidget::AngleNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationAngleNode");
      break;
    case qSlicerAnnotationModuleWidget::FiducialNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationFiducialNode");
      break;
    case qSlicerAnnotationModuleWidget::StickyNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationStickyNode");
      break;
    case qSlicerAnnotationModuleWidget::SplineNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationSplineNode");
      break;
    case qSlicerAnnotationModuleWidget::RulerNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationRulerNode");
      break;
    case qSlicerAnnotationModuleWidget::BidimensionalNode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationBidimensionalNode");
      break;
    case qSlicerAnnotationModuleWidget::ROINode:
      d->logic()->AddAnnotationNode("vtkMRMLAnnotationROINode");
      break;
    }

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onPauseButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->resumeButton->setChecked(false);
  d->pauseButton->setChecked(true);
  d->logic()->StopPlaceMode();

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
void qSlicerAnnotationModuleWidget::resetAllAnnotationTools()
{
  Q_D(qSlicerAnnotationModuleWidget);

  this->m_CurrentAnnotationType = 0;

  d->textTypeButton->setChecked(false);
  //d->angleTypeButton->setChecked(false);
  //d->roiTypeButton->setChecked(false);
  d->fiducialTypeButton->setChecked(false);
  //d->splineTypeButton->setChecked(false);
  //d->stickyTypeButton->setChecked(false);
  d->rulerTypeButton->setChecked(false);
  d->bidimensionalTypeButton->setChecked(false);

  d->logic()->StopPlaceMode();

  this->disableMouseModeButtons();

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::disableAllAnnotationTools()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->textTypeButton->setEnabled(false);
  //d->angleTypeButton->setEnabled(false);
  //d->roiTypeButton->setEnabled(false);
  d->fiducialTypeButton->setEnabled(false);
  //d->splineTypeButton->setEnabled(false);
  //d->stickyTypeButton->setEnabled(false);
  d->rulerTypeButton->setEnabled(false);
  d->bidimensionalTypeButton->setEnabled(false);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::enableAllAnnotationTools()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->textTypeButton->setEnabled(true);
  //d->angleTypeButton->setEnabled(true);
  //d->roiTypeButton->setEnabled(true);
  d->fiducialTypeButton->setEnabled(true);
  //d->splineTypeButton->setEnabled(true);
  //d->stickyTypeButton->setEnabled(true);
  d->rulerTypeButton->setEnabled(true);
  d->bidimensionalTypeButton->setEnabled(true);
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onAddHierarchyButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->logic()->SetActiveHierarchyNodeByID(d->hierarchyTreeWidget->firstSelectedNode());
  this->refreshTree();
  d->hierarchyTreeWidget->setSelectedNode(d->logic()->AddHierarchy()->GetID());
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

  d->logic()->SetAndObserveWidget(this);

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

  d->logic()->SetAndObserveWidget(this);

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

  d->logic()->SetAndObserveWidget(this);

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

  d->logic()->SetAndObserveWidget(this);

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

  d->logic()->SetAndObserveWidget(this);

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

  d->logic()->SetAndObserveWidget(this);

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

  d->logic()->SetAndObserveWidget(this);

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

  d->logic()->SetAndObserveWidget(this);

  this->enableMouseModeButtons();
  this->onResumeButtonClicked();

  this->disableAllAnnotationTools();

  //d->roiTypeButton->setChecked(true);
  d->resumeButton->setChecked(true);
}

//-----------------------------------------------------------------------------
// Refresh the hierarchy tree after an annotation was added or modified.
// Just do some layout changes - nothing special!
//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::refreshTree()
{
  Q_D(qSlicerAnnotationModuleWidget);
  d->hierarchyTreeWidget->setMRMLScene(d->logic()->GetMRMLScene());
  d->hierarchyTreeWidget->hideScene();
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

    d->logic()->SetAndObserveWidget(this);

    // be sure to listen to the mrml events
    // this only has to be called if no real annotations were placed yet
    // double call does not hurt..
    d->logic()->InitializeEventListeners();

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

    d->logic()->SetAndObserveWidget(this);

    // be sure to listen to the mrml events
    // this only has to be called if no real annotations were placed yet
    // double call does not hurt..
    d->logic()->InitializeEventListeners();

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

  d->hierarchyTreeWidget->selectedAsCollection(collection.GetPointer());

  // if nothing was selected, select all
  if(collection->GetNumberOfItems() == 0)
    {
    d->hierarchyTreeWidget->selectAll();
    d->hierarchyTreeWidget->selectedAsCollection(collection.GetPointer());
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

