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

#include "qMRMLSceneDisplayableModel.h"

// GUI includes
#include "GUI/qSlicerAnnotationModulePushButton.h"
#include "GUI/qSlicerAnnotationModuleReportDialog.h"
#include "GUI/qSlicerAnnotationModulePropertyDialog.h"
#include "GUI/qSlicerAnnotationModuleImageUtil.h"
#include "GUI/qSlicerAnnotationModuleSnapShotDialog.h"

//-----------------------------------------------------------------------------
class qSlicerAnnotationModuleWidgetPrivate: public Ui_qSlicerAnnotationModule
{
  Q_DECLARE_PUBLIC(qSlicerAnnotationModuleWidget);
protected:
  qSlicerAnnotationModuleWidget* const q_ptr;
public:
  typedef enum
  {
    VisibleColumn = 0, LockColumn, TypeColumn, ValueColumn, TextColumn
  } TableColumnType;

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

  m_ReportDialog = NULL;
  m_SnapShotDialog = NULL;

  this->m_PropertyDialog = 0;
  this->m_CurrentAnnotationType = 0;


}

//-----------------------------------------------------------------------------
qSlicerAnnotationModuleWidget::~qSlicerAnnotationModuleWidget()
{
  if (m_ReportDialog)
    {
    delete m_ReportDialog;
    m_ReportDialog = NULL;
    }

  if (m_SnapShotDialog)
    {
    delete m_SnapShotDialog;
    m_SnapShotDialog = NULL;
    }

  if (m_PropertyDialog)
    {
    m_PropertyDialog->close();
    delete m_PropertyDialog;
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

  this->connect(d->moveDownSelectedButton, SIGNAL(clicked()),
      SLOT(moveDownSelected()));
  this->connect(d->moveUpSelectedButton, SIGNAL(clicked()),
      SLOT(moveUpSelected()));

  this->connect(d->addHierarchyButton, SIGNAL(clicked()),
      SLOT(onAddHierarchyButtonClicked()));

  // Save Panel
  this->connect(d->saveScene, SIGNAL(clicked()),
      SLOT(onSaveMRMLSceneButtonClicked()));
  this->connect(d->selectedAllButton, SIGNAL(clicked()),
      SLOT(selectedAllButtonClicked()));
  this->connect(d->visibleSelectedButton, SIGNAL(clicked()),
      SLOT(visibleSelectedButtonClicked()));

  this->connect(d->deleteSelectedButton, SIGNAL(clicked()),
      SLOT(deleteSelectedButtonClicked()));
  this->connect(d->generateReport, SIGNAL(clicked()), this,
      SLOT(onGenerateReportButtonClicked()));
  this->connect(d->saveAnnotation, SIGNAL(clicked()), this,
      SLOT(onSaveAnnotationButtonClicked()));
  this->connect(d->screenShot, SIGNAL(clicked()), this,
      SLOT(onSnapShotButtonClicked()));
  this->connect(d->lockSelected, SIGNAL(clicked()), this,
      SLOT(lockSelectedButtonClicked()));

  this->connect(d->restoreViewButton, SIGNAL(clicked()), this,
      SLOT(onRestoreViewButtonClicked()));


}



//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::moveDownSelected()
{
  //Q_D(qSlicerAnnotationModuleWidget);

  // TODO
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::moveUpSelected()
{
  //Q_D(qSlicerAnnotationModuleWidget);

  // TODO
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onSaveMRMLSceneButtonClicked()
{
  //Q_D(qSlicerAnnotationModuleWidget);

  // TODO
  //d->logic()->SaveMRMLScene();
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::selectedAllButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  d->hierarchyTreeWidget->selectAll();

}


//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::propertyEditButtonClicked(QString mrmlId)
{
  Q_D(qSlicerAnnotationModuleWidget);

  // special case for snapshots
  if (d->logic()->IsSnapshotNode(mrmlId.toLatin1()))
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
    this->m_SnapShotDialog->initialize(mrmlId.toLatin1());

    // bail out, everything below is not for snapshots
    return;
    }
  // end of special case for snapshots

  // TODO hierarchies

  // check if there exists an annotationNode with the given ID
  // only then display the property dialog
  if (d->logic()->IsAnnotationNode(mrmlId.toLatin1()))
    {

    if (this->m_PropertyDialog)
      {
      QMessageBox::warning(d->hierarchyTreeWidget,
          QString("Modify Annotation Properties"), QString(
              "The property dialog is already open."));

      return;
      }

    d->logic()->SetAnnotationSelected(mrmlId.toLatin1(), true);

    // TODO
    //d->setItemEditable(d->tableWidget->selectedItems(), false);

    qSlicerAnnotationModulePropertyDialog* propertyDialog =
        new qSlicerAnnotationModulePropertyDialog(mrmlId.toLatin1(), d->logic());

    this->m_PropertyDialog = propertyDialog;

    this->m_PropertyDialog->setVisible(true);

    this->connect(this->m_PropertyDialog, SIGNAL(dialogRejected()), this,
        SLOT(propertyRestored()));
    this->connect(this->m_PropertyDialog, SIGNAL(dialogAccepted()), this,
        SLOT(propertyAccepted()));

    }
}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleWidget::onRestoreViewButtonClicked()
{
  Q_D(qSlicerAnnotationModuleWidget);

  // TODO

  d->logic()->RestoreAnnotationView(d->hierarchyTreeWidget->firstSelectedNode());

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
void qSlicerAnnotationModuleWidget::onGenerateReportButtonClicked()
{

  // TODO

  /*
  Q_D(qSlicerAnnotationModuleWidget);

  if (m_ReportDialog == NULL)
    {
    m_ReportDialog = new qSlicerAnnotationModuleReportDialog();

    }

  Ui::qSlicerAnnotationModuleReportDialog ui =
      m_ReportDialog->getReportDialogUi();

  QString
      report =
          "<html>\n"
            "<head><meta name=\"Author\" content=\"Daniel Haehn, Kilian Pohl, Yong Zhang\"><title>3D Slicer Report</title>\n"
            "<style type=\"text/css\">\n"
            "<!--\n"
            "body {\n"
            "font-family: Helvetica, Arial;\n"
            "  padding-left: 0px;\n"
            "  padding-right: 0px;\n"
            "  padding-bottom: 0em;\n"
            "  text-align: justify;\n"
            "  margin-left: 0px;\n"
            "  line-height: 110%; \n"
            "}\n"
            "\n"
            "table.annotation  {\n"
            "  cellpadding:2;\n"
            "  cellspacing:0;\n"
            "  width:700;\n"
            "}\n"
            "\n"
            "table.title {\n"
            "  cellpadding:0;\n"
            "  cellspacing:0;\n"
            "  width:700;\n"
            "}\n"
            "\n"
            "table.title TH {\n"
            "  font-size: 20.0pt; \n"
            "  background: #ffffff\n"
            "}\n"
            "\n"
            "table.annotation TH { \n"
            "  font-size: 14.0pt;   \n"
            "}\n"
            "\n"
            "table.annotation TD {\n"
            "  font-size: 12.0pt; \n"
            "  background: #eeeeee;\n"
            "  vertical-align: top;\n"
            "}\n"
            "\n"
            ".snapshot {\n"
            "border: 10px #eeeeee solid;\n"
            "}\n"
            "\n"
            ".textField {\n"
            "text-align: left\n"
            "}\n"
            "\n"
            "-->\n"
            "</style>\n"
            "</head>\n"
            "\n"
            "<body>\n"
            "<table class=\"title\" WIDTH=700>\n"
            "<tbody>\n"
            "<TR><TH><font size=20 face=\"Helvetica\">3D Slicer Report</font></TH>\n"
            "</TR>\n"
            "</tbody>\n"
            "</table>\n"
            "<BR>\n";
  // We define style sheet and old style bc QT does not interpret stylte sheets
  QString TD = "<td  align=center  bgcolor=\"#eeeeee\">";
  QString TDtext = "<td  align=left  bgcolor=\"#eeeeee\">";

  QString TDend = "</TD>";
  QString TH = "<TH bgcolor=\"#cccccc\" ";
  QString THend = "</TH>";

  report.append("<table class=\"annotation\" cellspacing=2>\n<tbody>\n<tr>\n");
  report.append(TH).append("width=100 >&nbsp;Type").append(THend);
  report.append(TH).append("width=100 >Value").append(THend);
  report.append(TH).append("width=496 >Text").append(THend).append("\n</tr>\n");

  if (m_IDs.size() > 0)
    {
    for (unsigned int i = 0; i < m_IDs.size(); ++i)
      {
      QString labelString = QString("Seed %1").arg(QString::number(i + 1));

      const char * thevalue;
      QString valueString, textString;
      report.append("<tr>\n").append(TD);

      thevalue = d->logic()->GetAnnotationMeasurement(m_IDs[i], false);

      report.append("<img src='") .append(d->logic()->GetIconName(
          d->logic()->GetMRMLScene()->GetNodeByID(m_IDs[i]))) .append("'>");
      textString = d->logic()->GetAnnotationTextProperty(
          d->logic()->GetMRMLScene()->GetNodeByID(m_IDs[i]));
      report.append(TDend).append(TD).append(thevalue).append(TDend).append(TD).append(
          textString).append(TDend).append("\n</tr>\n");
      }
    }
  else
    {
    report.append("<tr>\n").append(
        "<td  ALIGN=center  bgcolor=\"#eeeeee\" colspan=3>There is no annotation information").append(
        TDend).append("\n</tr>\n");
    }

  report.append("</table>\n<BR><BR>\n");

  if (!m_screenshotList.isEmpty())
    {
    report.append("<table class=\"annotation\" cellspacing=2>\n<tbody>\n<tr>\n");
    report.append(TH).append(" width=700>Screen Shots").append(THend).append(
        "\n</TR>\n");

    foreach(QString filename, m_screenshotList)
        {
        QFile file(filename);
        QImage img(filename);
        if (img.isNull())
          {
          std::cerr << "Error: Cannot open screen shot file " << std::endl;
          return;
          }
        report.append("<TR>\n").append(TD).append(
            "<img width=680 class=\"snapshot\" src=\"").append(filename).append(
            "\">").append(TDend).append("\n</TR>\n");
        }
    report.append("</tbody>\n</TABLE>\n");
    }

  report.append("</body>");

  ui.reportBrowser->setHtml(report);
  m_ReportDialog->setVisible(true);

  this->connect(

  m_ReportDialog, SIGNAL(filenameSelected()), this,
      SLOT(saveAnnotationReport()));
  this->m_report = report;
*/
}

//-----------------------------------------------------------------------------
bool qSlicerAnnotationModuleWidget::saveAnnotationReport()
{

  /*
  QString filename = m_ReportDialog->getFileName();

  if ((!filename.endsWith(".html")) && (!filename.endsWith(".HTML")))
    {
    filename.append(".html");
    }

  QString imgdir(filename);
  imgdir.remove(imgdir.size() - 5, 5);
  imgdir.append("_files");
  QDir currentdir = QDir::current();

  if (currentdir.exists())
    {
    if (!currentdir.mkdir(imgdir))
      {
      std::cerr << "Error: cannot make directory" << std::endl;
      }
    }

  QStringList list = imgdir.split("/");
  QString imgshortdir = list[list.size() - 1];

  QFile file(filename);
  if (!file.open(QFile::WriteOnly | QFile::Text))
    {
    std::cerr << "Error: Cannot save file " << qPrintable(filename) << ": "
        << qPrintable(file.errorString()) << std::endl;
    return false;
    }
  QTextStream out(&file);

  if (m_report.contains("<img src=':/Icons/AnnotationPoint.png'>"))
    {
    QString oldstring("<img src=':/Icons/AnnotationPoint.png'>");
    QString newstring("");
    newstring.append("<img src=\"").append(imgshortdir).append(
        "/AnnotationPoint.png\"").append(">");
    m_report.replace(oldstring, newstring);

    // save the image
    QImage img(":/Icons/AnnotationPoint.png");
    QString imgname(imgdir);
    imgname.append("/AnnotationPoint.png");

    QFile imgfile(imgname);
    if (!imgfile.open(QFile::WriteOnly))
      {
      std::cerr << "Error: Cannot save file " << qPrintable(imgname) << ": "
          << qPrintable(file.errorString()) << std::endl;
      return false;
      }
    QImageWriter writer(&imgfile, "PNG");
    writer.write(img);
    imgfile.close();

    }

  if (m_report.contains("<img src=':/Icons/AnnotationAngle.png'>"))
    {
    QString oldstring("<img src=':/Icons/AnnotationAngle.png'>");
    QString newstring("");
    newstring.append("<img src=\"").append(imgshortdir).append(
        "/AnnotationAngle.png\"").append(">");
    m_report.replace(oldstring, newstring);

    // save the image
    QImage img(":/Icons/AnnotationAngle.png");
    QString imgname(imgdir);
    imgname.append("/AnnotationAngle.png");

    QFile imgfile(imgname);
    if (!imgfile.open(QFile::WriteOnly))
      {
      std::cerr << "Error: Cannot save file " << qPrintable(imgname) << ": "
          << qPrintable(file.errorString()) << std::endl;
      return false;
      }
    QImageWriter writer(&imgfile, "PNG");
    writer.write(img);
    imgfile.close();

    }

  if (m_report.contains("<img src=':/Icons/AnnotationDistance.png'>"))
    {
    QString oldstring("<img src=':/Icons/AnnotationDistance.png'>");
    QString newstring("");
    newstring.append("<img src=\"").append(imgshortdir).append(
        "/AnnotationDistance.png\"").append(">");
    m_report.replace(oldstring, newstring);

    // save the image
    QImage img(":/Icons/AnnotationDistance.png");
    QString imgname(imgdir);
    imgname.append("/AnnotationDistance.png");

    QFile imgfile(imgname);
    if (!imgfile.open(QFile::WriteOnly))
      {
      std::cerr << "Error: Cannot save file " << qPrintable(imgname) << ": "
          << qPrintable(file.errorString()) << std::endl;
      return false;
      }
    QImageWriter writer(&imgfile, "PNG");
    writer.write(img);
    imgfile.close();

    }

  if (!m_screenshotList.isEmpty())
    {
    foreach(QString filename, m_screenshotList )
        {
        QStringList names;
        names = filename.split("/");
        QString shortname = names[names.size() - 1];
        QString oldstring = filename;
        QString newstring("");
        newstring.append(imgshortdir).append("/").append(shortname);
        m_report.replace(oldstring, newstring);

        // save the image
        QImage img(filename);
        QString imgname(imgdir);
        imgname.append("/").append(shortname);

        QFile imgfile(imgname);
        if (!imgfile.open(QFile::WriteOnly))
          {
          std::cerr << "Error: Cannot save file " << qPrintable(imgname)
              << ": " << qPrintable(file.errorString()) << std::endl;
          return false;
          }
        QImageWriter writer(&imgfile, "PNG");
        writer.write(img);
        imgfile.close();

        }
    }

  out << m_report;
  file.close();

  m_ReportDialog->close();

  return true;

  */

  // TODO


  return false;
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

  // show the dialog
  this->m_SnapShotDialog->setVisible(true);
  this->m_SnapShotDialog->reset();

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
  d->logic()->AddHierarchy();
  this->refreshTree();
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

