#include "GUI/qSlicerSceneViewsModuleWidget.h"
#include "ui_qSlicerSceneViewsModuleWidget.h"

// CTK includes
#include "ctkCollapsibleButton.h"
#include "ctkMessageBox.h"

// QT includes
#include <QBuffer>
#include <QButtonGroup>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFontMetrics>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QImageWriter>
#include <QLineEdit>
#include <QList>
#include <QMainWindow>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QSettings>
#include <QStatusBar>
#include <QTextBrowser>
#include <QWebFrame>
#include <QWebSettings>
#include <QUrl>

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSceneViewNode.h"

// VTK includes
#include "vtkCollection.h"
#include "vtkImageData.h"
#include "vtkImageResize.h"
#include "vtkNew.h"
#include "vtkPNGWriter.h"
#include "vtkSmartPointer.h"

// GUI includes
#include "GUI/qSlicerSceneViewsModuleDialog.h"

#include "qSlicerApplication.h"

#include "qMRMLSceneModel.h"

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

  QPointer<qSlicerSceneViewsModuleDialog> SceneViewDialog;

  QString htmlFromSceneView(vtkMRMLSceneViewNode *sceneView);
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
  this->Ui_qSlicerSceneViewsModuleWidget::setupUi(widget);

  this->sceneViewsWebView->setMRMLScene(q->mrmlScene());
  this->sceneViewsWebView->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);

  // propagate fonts from the application
  QSettings *settings =
    qSlicerApplication::application()->settingsDialog()->settings();
  QFont currentFont = settings->value("Font").value<QFont>();
  this->sceneViewsWebView->settings()->setFontFamily(QWebSettings::StandardFont,
                                                     currentFont.family());
  this->sceneViewsWebView->settings()->setFontSize(QWebSettings::DefaultFontSize,
                                                   currentFont.pointSize());

  // capture link clicked
  this->sceneViewsWebView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
  QObject::connect(this->sceneViewsWebView, SIGNAL(linkClicked(const QUrl &)),
                    q, SLOT(captureLinkClicked(QUrl)));
  // restore scroll bar position when the contents have been changed
  QObject::connect(this->sceneViewsWebView->page()->mainFrame(),
                   SIGNAL(contentsSizeChanged(const QSize &)),
                   q, SLOT(restoreScrollPosition(QSize)));
}

//-----------------------------------------------------------------------------
QString qSlicerSceneViewsModuleWidgetPrivate::htmlFromSceneView(vtkMRMLSceneViewNode *sceneView)
{
  QString html;

  if (!sceneView)
    {
    return html;
    }
  QString name = sceneView->GetName();
  QString id = sceneView->GetID();
  QString description = sceneView->GetSceneViewDescription().c_str();
  // replace any carriage returns with html line breaks
  description.replace(QString("\n"),
                      QString("<br>\n"));
  QString tempDir = qSlicerApplication::application()->defaultTemporaryPath();
  QString thumbnailPath = tempDir + "/" + id + ".png";
  // the scene view node might have been added to the scene without a screen shot
  if (sceneView->GetScreenShot())
    {
    /// tbd: always write out the image?
    if (!QFile::exists(thumbnailPath))
      {
      vtkNew<vtkPNGWriter> writer;
      writer->SetFileName(thumbnailPath.toLatin1());
      vtkNew<vtkImageResize> resizeFilter;
      resizeFilter->SetResizeMethodToOutputDimensions();
      resizeFilter->SetInputData(sceneView->GetScreenShot());
      // try to keep the aspect ratio while setting a height
      int dims[3];
      sceneView->GetScreenShot()->GetDimensions(dims);
      float newHeight = 200;
      float newWidth = (newHeight/(float)(dims[0])) * (float)(dims[1]);
      resizeFilter->SetOutputDimensions(newHeight, newWidth, 1);
      writer->SetInputConnection(resizeFilter->GetOutputPort());
      try
        {
        writer->Write();
        }
      catch (...)
        {
        qWarning() << "Unable to write file " << thumbnailPath;
        }
      }
    }
  QString restoreImagePath = QString("qrc:///Icons/Restore.png");
  QString deleteImagePath = QString("qrc:///Icons/Delete.png");

  html = "<li>";
  html += " <div style=\"width:100%;overflow-x:hidden;overflow-y:hidden;background-image:none;\">\n";
  html += "  <div style=\"float:left; width:200px; margin:5px;\">\n";
  html += "   <a href=\"Edit " + id + "\">\n";
  if (sceneView->GetScreenShot())
    {
    html += "    <img src=\"file://" + thumbnailPath + "\" ";
    html += "style=\"visibility:visible; max-width:200; max-height:none; ";
    html += "display:block; image-rendering:auto; width:auto; height:auto; ";
    html += "margin-left:10px; margin-top:0px; opacity:1;\">\n";
    }
  html += "   </a>\n";
  html += "  </div>\n";
  html += "  <div style=\"margin-left: 240px;\">";
  html += "   <h3><a href=\"Restore " + id  + "\"><img src=\"" + restoreImagePath + "\"></a> ";
  html += "   " + name;
  html += "    <a href=\"Delete " + id  + "\"><img src=\"" + deleteImagePath + "\"></a> ";
  html += "   </h3>\n";
  // don't underline the link
  html += "   <a href=\"Edit " + id + "\" style=\"text-decoration:none;\">\n";
  html += "   " + description + "\n";
  html += "   </a>\n";
  html += "  </div>\n";
  html += " </div>\n";
  html += "</li>\n";

  return html;
}

//-----------------------------------------------------------------------------
// qSlicerSceneViewsModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleWidget::qSlicerSceneViewsModuleWidget(QWidget* parent) :
  qSlicerAbstractModuleWidget(parent)
  , d_ptr(new qSlicerSceneViewsModuleWidgetPrivate(*this))
{
  this->savedScrollPosition = 0;
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
void qSlicerSceneViewsModuleWidget::moveDownSelected(QString mrmlId)
{
  Q_D(qSlicerSceneViewsModuleWidget);

  const char* id = d->logic()->MoveSceneViewDown(mrmlId.toLatin1());

  if (id != NULL &&
      strcmp(id, "") != 0)
    {
    this->updateFromMRMLScene();
    }
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::moveUpSelected(QString mrmlId)
{
  Q_D(qSlicerSceneViewsModuleWidget);

  const char* id = d->logic()->MoveSceneViewUp(mrmlId.toLatin1());

  if (id != NULL &&
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
  d->logic()->RestoreSceneView(mrmlId.toLatin1(), false);

  // check if there was a problem restoring
  if (this->mrmlScene()->GetErrorCode() != 0)
    {
    // reset the error state
    this->mrmlScene()->SetErrorCode(0);
    this->mrmlScene()->SetErrorMessage("");

    // ask if the user wishes to save the current scene nodes, retore and delete them or cancel
    ctkMessageBox missingNodesMsgBox;
    missingNodesMsgBox.setWindowTitle("Data missing from Scene View");
    vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->mrmlScene()->GetNodeByID(mrmlId.toLatin1()));
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
        d->logic()->RestoreSceneView(mrmlId.toLatin1(), true);
        break;
      case QMessageBox::Save:
        if (viewNode)
          {
          viewNode->AddMissingNodes();

          // and restore again
          d->logic()->RestoreSceneView(mrmlId.toLatin1(), false);
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

  // clear the cache so new thumbnails will be used
  d->sceneViewsWebView->settings()->clearMemoryCaches();

  int numSceneViews = this->mrmlScene()->GetNumberOfNodesByClass("vtkMRMLSceneViewNode");
  QString createImagePath = QString("qrc:///Icons/Camera.png");

  QString headerHtml;
  headerHtml = "<html>";
  headerHtml += "<head></head>";
  headerHtml += "<body link=\"000000\">";
  headerHtml += " <div>";
  headerHtml += "  <a href=\"Create\"><img src=\"" + createImagePath + "\"></a> ";
  headerHtml += "   Scene Views ("
    + QString::number(numSceneViews) + "):</div>";
  headerHtml += "   <div style=\"width:100%;\">";
  headerHtml += "    <ul>";

  QString htmlPage = headerHtml;

  for (int i = 0; i < numSceneViews; ++i)
    {
    vtkMRMLNode *mrmlNode = this->mrmlScene()->GetNthNodeByClass(i, "vtkMRMLSceneViewNode");
    if (!mrmlNode)
      {
      continue;
      }
    vtkMRMLSceneViewNode *sceneView = vtkMRMLSceneViewNode::SafeDownCast(mrmlNode);
    QString sceneViewHtml = d->htmlFromSceneView(sceneView);
    htmlPage += sceneViewHtml;
    // separarate scene views with a line
    if (i < numSceneViews - 1)
      {
      QString lineHtml;
      lineHtml = "<hr>\n";
      htmlPage += lineHtml;
      }
    }
  QString footerHtml;
  footerHtml = "    </ul>\n";
  footerHtml += "  </div>\n";
  footerHtml += " </body>\n";
  footerHtml += "</html>\n";
  htmlPage += footerHtml;

  QString baseURL;
  // save the scroll bar position so can restore it once the html
  // has been rendered
  this->savedScrollPosition = d->sceneViewsWebView->page()->mainFrame()->scrollBarValue(Qt::Vertical);

  d->sceneViewsWebView->setHtml(htmlPage, baseURL);
  d->sceneViewsWebView->show();

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

  // this call needed for the case of a scene with scene views having been
  // loaded while not in the scene views module, clear out the old thumbnails.
  this->removeTemporaryFiles();

  // and update the web view
  this->updateFromMRMLScene();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::exit()
{
  this->Superclass::exit();

  // qDebug() << "exit widget";

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

  // clear temp files to avoid thumbnail clashes with reused node ids
  this->removeTemporaryFiles();

  // update the web view
  this->updateFromMRMLScene();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::removeTemporaryFiles()
{
  QString tempDirectoryPath = qSlicerApplication::application()->defaultTemporaryPath();
  // look for files with vtkMRMLSceneViewNodeX.png file names
  QDir tempDir = QDir(tempDirectoryPath);
  QStringList filters;
  filters << "vtkMRMLSceneViewNode*.png";
  tempDir.setNameFilters(filters);
  QStringList fileList = tempDir.entryList();
  for (int i = 0; i < fileList.size(); ++i)
    {
    QString imagePath = tempDir.absoluteFilePath(fileList.at(i));
    if (!QFile::remove(imagePath))
      {
      qWarning() << "Error removing scene view thumbnail file " << imagePath;
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::captureLinkClicked(const QUrl &url)
{
  QString toParse = url.toString();

  QStringList operationAndID = toParse.split(" ");
  QString operation = operationAndID[0];
  QString id;
  if (operationAndID.size() > 1)
    {
    // Create doesn't need an id
    id = operationAndID[1];
    }
  if (operation == QString("Edit"))
    {
    this->editSceneView(id);
    }
  else if (operation == QString("Restore"))
    {
    this->restoreSceneView(id);
    }
  else if (operation == QString("Delete"))
    {
    this->mrmlScene()->RemoveNode(this->mrmlScene()->GetNodeByID(id.toLatin1()));
    }
  else if (operation == QString("Create"))
    {
    this->showSceneViewDialog();
    }
  else
    {
    qWarning() << "captureLinkClicked: unsupported operation: " << operation;
    }
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::restoreScrollPosition(const QSize &size)
{
  Q_UNUSED(size);
  Q_D(qSlicerSceneViewsModuleWidget);
  d->sceneViewsWebView->page()->mainFrame()->setScrollBarValue(Qt::Vertical, this->savedScrollPosition);
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

