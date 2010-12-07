#include "qSlicerAnnotationModuleReportDialog.h"

// QT includes
#include <QButtonGroup>
#include <QList>
#include <QFontMetrics>
#include <QDebug>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QFileDialog>


//---------------------------------------------------------------------------
qSlicerAnnotationModuleReportDialog::qSlicerAnnotationModuleReportDialog()
{


  this->m_Logic = 0;

  this->m_Annotations = 0;

  this->ui.setupUi(this);

  // The restore button has to be configured since it is the reset button in the buttonbox
  // so we set Icons and Text here
  QPushButton* restoreButton = ui.buttonBox->button(QDialogButtonBox::Reset);
  restoreButton->setText("Print");
  restoreButton->setIcon(QIcon(":/Icons/AnnotationPrint.png"));

  ui.titleEdit->setText("Annotation Report");

  this->createConnection();

}

//---------------------------------------------------------------------------
qSlicerAnnotationModuleReportDialog::~qSlicerAnnotationModuleReportDialog()
{

  if (this->m_Logic)
    {
    this->m_Logic = 0;
    }

  if (this->m_Annotations)
    {
    this->m_Annotations->Delete();
    this->m_Annotations = 0;
    }

}


//---------------------------------------------------------------------------
void qSlicerAnnotationModuleReportDialog::createConnection()
{

  // connect the OK and CANCEL button to the individual Slots
  this->connect(this, SIGNAL(rejected()), this, SLOT(onDialogRejected()));
  this->connect(this, SIGNAL(accepted()), this, SLOT(onDialogAccepted()));

  QPushButton* cancelButton = ui.buttonBox->button(QDialogButtonBox::Cancel);
  this->connect(cancelButton, SIGNAL(clicked()), this, SLOT(onDialogRejected()));

  QPushButton* saveButton = ui.buttonBox->button(QDialogButtonBox::Save);
  this->connect(saveButton, SIGNAL(clicked()), this, SLOT(onDialogAccepted()));

  // connect the Print button
  QPushButton* printButton = ui.buttonBox->button(QDialogButtonBox::Reset);
  this->connect(printButton, SIGNAL(clicked()), this, SLOT(onPrintButtonClicked()));

  this->connect(ui.titleEdit, SIGNAL(textEdited(const QString &)), this, SLOT(onTextEdited()));



}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleReportDialog::setLogic(vtkSlicerAnnotationModuleLogic* logic)
{
  if (!logic)
    {
    qErrnoWarning("setLogic: We need the Annotation module logic here!");
    return;
    }

  this->m_Logic = logic;

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleReportDialog::setAnnotations(vtkCollection* collection)
{
  if (!collection)
    {
    qErrnoWarning("setAnnotations: we need a vtkCollection here!");
    return;
    }

  this->m_Annotations = vtkCollection::New();

  collection->InitTraversal();

  for(int i=0; i<collection->GetNumberOfItems(); ++i)
    {
    this->m_Annotations->AddItem(collection->GetItemAsObject(i));
    }

}

//---------------------------------------------------------------------------
/*void qSlicerAnnotationModuleReportDialog::onSaveReportButtonClicked()
{
    QString filename = QFileDialog::getSaveFileName(
        this,
        "Save Report",
        QDir::currentPath(),
        "Reports (*.html)");

    // save the documents...
    if ( !filename.isNull())
    {
        m_filename = filename;
        emit filenameSelected();
    }

}*/

//---------------------------------------------------------------------------
QString qSlicerAnnotationModuleReportDialog::generateReport()
{

  QString html = "<html>\n";
  html.append("<head>\n");
  html.append("<meta name=\"Author\" content=\"Daniel Haehn, Kilian Pohl, Yong Zhang\">\n");
  html.append("<title>");
  html.append("3D Slicer: ");
  html.append(ui.titleEdit->text());
  html.append("</title>\n");
  html.append("<style type=\"text/css\">\n");
  html.append("body {font-family: Helvetica, Arial;}\n");
  html.append(".heading {background-color:lightgrey;}\n");
  html.append("</style>\n");
  html.append("<body>\n");

  html.append("<h1>");
  html.append("3D Slicer: ");
  html.append(ui.titleEdit->text());
  html.append("</h1><br>\n");

  html.append("<table border=0 width='100%' cellPadding=3 cellSpacing=0>\n");

  html.append("<tr><td class='heading'><b>Type</b></td><td class='heading'><b>Value</b></td><td class='heading'><b>Text<b></td></tr>\n");

  // now run through the annotations
  this->m_Annotations->InitTraversal();

  for(int i=0; i<this->m_Annotations->GetNumberOfItems(); ++i)
    {

    vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(this->m_Annotations->GetItemAsObject(i));

    if (annotationNode)
      {

      html.append(QString(this->m_Logic->GetHTMLRepresentation(annotationNode)));

      }

    }

  html.append("</table>\n");

  html.append("</body>\n");
  html.append("</html>");

  return html;

}

//---------------------------------------------------------------------------
void qSlicerAnnotationModuleReportDialog::updateReport()
{

  this->ui.reportBrowser->setHtml(this->generateReport());

}

//---------------------------------------------------------------------------
void qSlicerAnnotationModuleReportDialog::onTextEdited()
{

  this->updateReport();

}

//---------------------------------------------------------------------------
void qSlicerAnnotationModuleReportDialog::onPrintButtonClicked()
{

  QPrinter printer;

  QPrintDialog *dialog = new QPrintDialog(&printer, this);
  dialog->setWindowTitle(tr("Print Annotation Report"));

  if( dialog->exec() == QDialog::Accepted )
  {

    this->ui.reportBrowser->print(&printer);

  }

}


//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleReportDialog::onDialogRejected()
{

  // emit an event which gets caught by main GUI window
  emit dialogRejected();

}

//-----------------------------------------------------------------------------
void qSlicerAnnotationModuleReportDialog::onDialogAccepted()
{

  if (this->saveReport())
    {
    // emit an event which gets caught by main GUI window
    emit dialogAccepted();
    }
}

//-----------------------------------------------------------------------------
bool qSlicerAnnotationModuleReportDialog::saveReport()
{

  QString filename = QFileDialog::getSaveFileName(this, "Save Annotation Report", QString(), "3D Slicer Annotation Report (*.html)");

  QString report = this->generateReport();

  if (filename.length() > 0)
    {

    // make sure the selected file ends with .html
    if ((!filename.endsWith(".html")) && (!filename.endsWith(".HTML")))
      {
      filename.append(".html");
      }

    // check, if we can write the file
    QFile file(filename);
    if (!file.open(QFile::WriteOnly | QFile::Text))
      {
      std::cerr << "Error: Cannot save file " << qPrintable(filename) << ": "
          << qPrintable(file.errorString()) << std::endl;
      return false;
      }
    QTextStream out(&file);

    // create a directory for the graphics
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

    // save a relative path to the imgdir
    QStringList list = imgdir.split("/");
    QString imgshortdir = list[list.size() - 1];

    // replace all QT icon pathes with real filepathes
    report.replace(QString(":/Icons/"), imgshortdir.append("/"));

    // save all icons for annotations
    QImage img(":/Icons/AnnotationPoint.png");
    img.save(imgdir+("/AnnotationPoint.png"));

    QImage img2(":/Icons/AnnotationBidimensional.png");
    img2.save(imgdir+("/AnnotationBidimensional.png"));

    QImage img3(":/Icons/AnnotationText.png");
    img3.save(imgdir+("/AnnotationText.png"));

    QImage img4(":/Icons/AnnotationDistance.png");
    img4.save(imgdir+("/AnnotationDistance.png"));

    out << report;
    file.close();

    return true;

    } // file dialog accepted

  return false; // file dialog cancelled

}
