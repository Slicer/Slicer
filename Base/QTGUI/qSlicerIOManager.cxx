/// Qt includes
#include <QDebug>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMainWindow>
#include <QMetaProperty>
#include <QProgressDialog>
#include <QSettings>
#include <QTimer>

// CTK includes
#include <ctkMessageBox.h>
#include <ctkScreenshotDialog.h>

/// Slicer includes
#include "qSlicerIOManager.h"
#include "qSlicerDataDialog.h"
#include "qSlicerGenericIOOptionsWidget.h"
#include "qSlicerModelsDialog.h"
#include "qSlicerSaveDataDialog.h"
#include "qSlicerApplication.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractCoreModule.h"

/// MRML includes
#include <vtkMRMLIOOptionsDescription.h>
#include <vtkMRMLOperationProgress.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLMessageCollection.h>

/// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>

//-----------------------------------------------------------------------------
class qSlicerIOManagerPrivate
{
  Q_DECLARE_PUBLIC(qSlicerIOManager);

protected:
  qSlicerIOManager* const q_ptr;

public:
  qSlicerIOManagerPrivate(qSlicerIOManager& object);

  vtkMRMLScene* currentScene() const;

  void readSettings();
  void writeSettings();
  QString createUniqueDialogName(qSlicerIO::IOFileType, qSlicerFileDialog::IOAction, const qSlicerIO::IOProperties&);

  qSlicerFileDialog* findDialog(qSlicerIO::IOFileType fileType, qSlicerFileDialog::IOAction) const;

  QStringList History;
  QList<QUrl> Favorites;
  QList<qSlicerFileDialog*> ReadDialogs;
  QList<qSlicerFileDialog*> WriteDialogs;

  QSharedPointer<ctkScreenshotDialog> ScreenshotDialog;
  QProgressDialog* ProgressDialog{ nullptr };

  /// File dialog that next call of execDelayedFileDialog signal will execute
  qSlicerFileDialog* DelayedFileDialog{ nullptr };
};

//-----------------------------------------------------------------------------
// qSlicerIOManagerPrivate methods

//-----------------------------------------------------------------------------
qSlicerIOManagerPrivate::qSlicerIOManagerPrivate(qSlicerIOManager& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qSlicerIOManagerPrivate::currentScene() const
{
  return qSlicerCoreApplication::application()->mrmlScene();
}

//-----------------------------------------------------------------------------
void qSlicerIOManagerPrivate::readSettings()
{
  QSettings settings;
  settings.beginGroup("ioManager");

  if (!settings.value("favoritesPaths").toList().isEmpty())
  {
    QStringList paths = qSlicerCoreApplication::application()->toSlicerHomeAbsolutePaths(settings.value("favoritesPaths").toStringList());
    for (const QString& varUrl : paths)
    {
      this->Favorites << QUrl(varUrl);
    }
  }
  else
  {
    this->Favorites << QUrl::fromLocalFile(QDir::homePath());
  }

  settings.endGroup();
}

//-----------------------------------------------------------------------------
void qSlicerIOManagerPrivate::writeSettings()
{
  Q_Q(qSlicerIOManager);
  QSettings settings;
  settings.beginGroup("ioManager");
  QStringList list;
  for (const QUrl& url : q->favorites())
  {
    list << url.toString();
  }
  QStringList paths = qSlicerCoreApplication::application()->toSlicerHomeRelativePaths(list);
  settings.setValue("favoritesPaths", QVariant(paths));
  settings.endGroup();
}

//-----------------------------------------------------------------------------
QString qSlicerIOManagerPrivate::createUniqueDialogName(qSlicerIO::IOFileType fileType, qSlicerFileDialog::IOAction action, const qSlicerIO::IOProperties& ioProperties)
{
  QString objectName;

  objectName += action == qSlicerFileDialog::Read ? /*no tr*/ "Add" : /*no tr*/ "Save";
  objectName += fileType;
  objectName += ioProperties["multipleFiles"].toBool() ? "s" : "";
  objectName += /*no tr*/ "Dialog";

  return objectName;
}

//-----------------------------------------------------------------------------
qSlicerFileDialog* qSlicerIOManagerPrivate::findDialog(qSlicerIO::IOFileType fileType, qSlicerFileDialog::IOAction action) const
{
  const QList<qSlicerFileDialog*>& dialogs = (action == qSlicerFileDialog::Read) ? this->ReadDialogs : this->WriteDialogs;
  for (qSlicerFileDialog* const dialog : dialogs)
  {
    if (dialog->fileType() == fileType)
    {
      return dialog;
    }
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
// qSlicerIOManager methods

//-----------------------------------------------------------------------------
qSlicerIOManager::qSlicerIOManager(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerIOManagerPrivate(*this))
{
  Q_D(qSlicerIOManager);
  d->readSettings();
}

//-----------------------------------------------------------------------------
qSlicerIOManager::~qSlicerIOManager()
{
  Q_D(qSlicerIOManager);
  d->writeSettings();
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openLoadSceneDialog()
{
  qSlicerIO::IOProperties properties;
  properties["clear"] = true;
  return this->openDialog(QString("SceneFile"), qSlicerFileDialog::Read, properties);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openAddSceneDialog()
{
  qSlicerIO::IOProperties properties;
  properties["clear"] = false;
  return this->openDialog(QString("SceneFile"), qSlicerFileDialog::Read, properties);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openDialog(qSlicerIO::IOFileType fileType, qSlicerFileDialog::IOAction action, qSlicerIO::IOProperties properties, vtkCollection* loadedNodes)
{
  Q_D(qSlicerIOManager);
  bool deleteDialog = false;
  if (properties["objectName"].toString().isEmpty())
  {
    QString name = d->createUniqueDialogName(fileType, action, properties);
    properties["objectName"] = name;
  }
  qSlicerFileDialog* dialog = d->findDialog(fileType, action);
  if (dialog == nullptr)
  {
    deleteDialog = true;
    qSlicerStandardFileDialog* standardDialog = new qSlicerStandardFileDialog(this);
    standardDialog->setFileType(fileType);
    standardDialog->setAction(action);
    dialog = standardDialog;
  }
  bool res = dialog->exec(properties);
  if (loadedNodes)
  {
    for (const QString& nodeID : dialog->loadedNodes())
    {
      vtkMRMLNode* node = d->currentScene()->GetNodeByID(nodeID.toUtf8());
      if (node)
      {
        loadedNodes->AddItem(node);
      }
    }
  }
  if (deleteDialog)
  {
    delete dialog;
  }
  return res;
}

//---------------------------------------------------------------------------
void qSlicerIOManager::dragEnterEvent(QDragEnterEvent* event)
{
  Q_D(qSlicerIOManager);
  for (qSlicerFileDialog* const dialog : d->ReadDialogs)
  {
    if (dialog->isMimeDataAccepted(event->mimeData()))
    {
      event->accept();
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::dropEvent(QDropEvent* event)
{
  Q_D(qSlicerIOManager);
  QStringList supportedReaders;
  QStringList genericReaders; // those must be last in the choice menu
  for (qSlicerFileDialog* const dialog : d->ReadDialogs)
  {
    if (dialog->isMimeDataAccepted(event->mimeData()))
    {
      QString supportedReader = dialog->description();
      if (dialog->fileType() == "NoFile")
      {
        genericReaders << supportedReader;
      }
      else
      {
        supportedReaders << supportedReader;
      }
    }
  }
  supportedReaders << genericReaders;
  QString selectedReader;
  if (supportedReaders.size() > 1)
  {
    QString title = tr("Select a reader");
    QString label = tr("Select a reader to use for your data:");
    int current = 0;
    bool editable = false;
    bool ok = false;
    qSlicerApplication* app = qSlicerApplication::application();
    QWidget* mainWindow = app ? app->mainWindow() : nullptr;
    selectedReader = QInputDialog::getItem(mainWindow, title, label, supportedReaders, current, editable, &ok);
    if (!ok)
    {
      selectedReader = QString();
    }
  }
  else if (supportedReaders.size() == 1)
  {
    selectedReader = supportedReaders[0];
  }
  if (selectedReader.isEmpty())
  {
    return;
  }
  for (qSlicerFileDialog* const dialog : d->ReadDialogs)
  {
    if (dialog->description() == selectedReader)
    {
      dialog->dropEvent(event);
      if (event->isAccepted())
      {
        // If we immediately executed the dialog here then we would block the application
        // that initiated the drag-and-drop operation. Therefore we return and open the dialog
        // with a timer.
        d->DelayedFileDialog = dialog;
        QTimer::singleShot(0, this, SLOT(execDelayedFileDialog()));
        break;
      }
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::execDelayedFileDialog()
{
  Q_D(qSlicerIOManager);
  if (d->DelayedFileDialog)
  {
    d->DelayedFileDialog->exec();
  }
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::addHistory(const QString& path)
{
  Q_D(qSlicerIOManager);
  d->History << path;
}

//-----------------------------------------------------------------------------
QStringList qSlicerIOManager::history() const
{
  Q_D(const qSlicerIOManager);
  return d->History;
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::setFavorites(const QList<QUrl>& urls)
{
  Q_D(qSlicerIOManager);
  QList<QUrl> newFavorites;
  for (const QUrl& url : urls)
  {
    newFavorites << url;
  }
  d->Favorites = newFavorites;
}

//-----------------------------------------------------------------------------
QList<QUrl> qSlicerIOManager::favorites() const
{
  Q_D(const qSlicerIOManager);
  return d->Favorites;
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::registerDialog(qSlicerFileDialog* dialog)
{
  Q_D(qSlicerIOManager);
  Q_ASSERT(dialog);
  qSlicerFileDialog* existingDialog = d->findDialog(dialog->fileType(), dialog->action());
  if (existingDialog)
  {
    d->ReadDialogs.removeAll(existingDialog);
    d->WriteDialogs.removeAll(existingDialog);
    existingDialog->deleteLater();
  }
  if (dialog->action() == qSlicerFileDialog::Read)
  {
    d->ReadDialogs.prepend(dialog);
  }
  else if (dialog->action() == qSlicerFileDialog::Write)
  {
    d->WriteDialogs.prepend(dialog);
  }
  else
  {
    Q_ASSERT(dialog->action() == qSlicerFileDialog::Read || //
             dialog->action() == qSlicerFileDialog::Write);
  }
  dialog->setParent(this);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::isDialogRegistered(qSlicerIO::IOFileType fileType, qSlicerFileDialog::IOAction action) const
{
  Q_D(const qSlicerIOManager);
  qSlicerFileDialog* existingDialog = d->findDialog(fileType, action);
  return existingDialog != nullptr;
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::loadNodes(const qSlicerIO::IOFileType& fileType,
                                 const qSlicerIO::IOProperties& parameters,
                                 vtkCollection* loadedNodes,
                                 vtkMRMLMessageCollection* userMessages /*=nullptr*/)
{
  // speed up data loading by disabling re-rendering
  // (it can make a big difference if hundreds of nodes are loaded)
  SlicerRenderBlocker renderBlocker;
  return this->qSlicerCoreIOManager::loadNodes(fileType, parameters, loadedNodes, userMessages);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::loadNodes(const QList<qSlicerIO::IOProperties>& files, vtkCollection* loadedNodes, vtkMRMLMessageCollection* userMessages /*=nullptr*/)
{
  // Speed up data loading by disabling re-rendering
  // (it can make a big difference if hundreds of nodes are loaded)
  SlicerRenderBlocker renderBlocker;
  return this->qSlicerCoreIOManager::loadNodes(files, loadedNodes, userMessages);
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::startProgress(vtkMRMLOperationProgress* progress)
{
  Q_D(qSlicerIOManager);
  delete d->ProgressDialog;
  QString title = progress ? QString::fromStdString(progress->GetTitle()) : QString();
  const bool abortable = progress && progress->GetAbortable();
  Qt::WindowFlags windowFlags = Qt::Dialog;
  if (!abortable)
  {
    // The operation cannot be aborted, therefore the window close button is not offered (only a title bar is shown)
    windowFlags |= Qt::CustomizeWindowHint | Qt::WindowTitleHint;
  }
  d->ProgressDialog = new QProgressDialog(title, qSlicerIOManager::tr("Cancel"), 0, 100, nullptr, windowFlags);
  d->ProgressDialog->setWindowTitle(title);
  if (!abortable)
  {
    d->ProgressDialog->setCancelButton(nullptr);
  }
  d->ProgressDialog->setWindowModality(Qt::WindowModal);
  d->ProgressDialog->setMinimumDuration(1000);
  d->ProgressDialog->setValue(0);
  this->updateProgress(progress);
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::updateProgress(vtkMRMLOperationProgress* progress)
{
  Q_D(qSlicerIOManager);
  if (!d->ProgressDialog || !progress)
  {
    return;
  }
  if (d->ProgressDialog->wasCanceled())
  {
    // Do not update the dialog after it is canceled (it would be shown again)
    progress->AbortExecuteOn();
    return;
  }
  QString text = QString::fromStdString(progress->GetProgressText());
  if (!text.isEmpty() && progress->GetNumberOfFiles() > 1)
  {
    //: %1 is the index of the current file, %2 is the total number of files
    text += " " + qSlicerIOManager::tr("(%1 of %2)").arg(progress->GetCurrentFileIndex() + 1).arg(progress->GetNumberOfFiles());
  }
  if (!text.isEmpty() && d->ProgressDialog->labelText() != text)
  {
    d->ProgressDialog->setLabelText(text);
  }
  if (progress->GetNumberOfFiles() == 0)
  {
    // Unknown amount of work: show busy indicator
    d->ProgressDialog->setRange(0, 0);
  }
  else
  {
    // QProgressDialog only shows itself and processes events (displays changes and handles clicking the Cancel button)
    // when its value changes. Use a range that is large enough so that the value changes at each progress update:
    // progress of processing each file is reported in percent.
    const int maximumValue = 100 * progress->GetNumberOfFiles();
    d->ProgressDialog->setRange(0, maximumValue);
    // The maximum is only set when processing is completed (setting it would close the dialog)
    d->ProgressDialog->setValue(qBound(0, static_cast<int>(progress->GetProgress() * maximumValue), maximumValue - 1));
  }
  if (d->ProgressDialog->wasCanceled())
  {
    progress->AbortExecuteOn();
  }
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::endProgress(vtkMRMLOperationProgress* vtkNotUsed(progress))
{
  Q_D(qSlicerIOManager);
  delete d->ProgressDialog;
  d->ProgressDialog = nullptr;
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::openScreenshotDialog()
{
  Q_D(qSlicerIOManager);
  // try opening the Annotation module's screen shot
  qSlicerModuleManager* moduleManager = qSlicerApplication::application()->moduleManager();

  qSlicerAbstractCoreModule* modulePointer = nullptr;
  if (moduleManager)
  {
    modulePointer = moduleManager->module("Annotations");
  }
  if (modulePointer)
  {
    QMetaObject::invokeMethod(modulePointer, "showScreenshotDialog");
  }
  else
  {
    qWarning() << "qSlicerIOManager::openScreenshotDialog: Unable to get Annotations module (annotations), using the CTK screen shot dialog.";
    // use the ctk one
    if (!d->ScreenshotDialog)
    {
      d->ScreenshotDialog = QSharedPointer<ctkScreenshotDialog>(new ctkScreenshotDialog());
      d->ScreenshotDialog->setWidgetToGrab(qSlicerApplication::application()->layoutManager()->viewport());
    }
    d->ScreenshotDialog->show();
  }
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::openSceneViewsDialog()
{
  //  Q_D(qSlicerIOManager);
  qSlicerModuleManager* moduleManager = qSlicerApplication::application()->moduleManager();
  if (!moduleManager)
  {
    qWarning() << "qSlicerIOManager::openSceneViewsDialog: unable to get module manager, can't get at the Scene Views module";
    return;
  }

  qSlicerAbstractCoreModule* modulePointer = moduleManager->module("SceneViews");
  if (modulePointer == nullptr)
  {
    qWarning() << "qSlicerIOManager::openSceneViewsDialog: Unable to get at the SceneViews module (sceneviews).";
    return;
  }
  QMetaObject::invokeMethod(modulePointer, "showSceneViewDialog");
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::showLoadNodesResultDialog(bool overallSuccess, vtkMRMLMessageCollection* userMessages)
{
  bool isTestingEnabled = qSlicerApplication::testAttribute(qSlicerCoreApplication::AA_EnableTesting);
  if (isTestingEnabled)
  {
    // Do not block the execution with popup windows if testing mode is enabled.
    return;
  }
  if (overallSuccess                                                            //
      && userMessages->GetNumberOfMessagesOfType(vtkCommand::WarningEvent) == 0 //
      && userMessages->GetNumberOfMessagesOfType(vtkCommand::ErrorEvent) == 0)
  {
    // Everything is OK, no need to show error popup.
    return;
  }
  qSlicerApplication* app = qSlicerApplication::application();
  QWidget* mainWindow = app ? app->mainWindow() : nullptr;
  ctkMessageBox* messageBox = new ctkMessageBox(mainWindow);
  QString text;
  if (overallSuccess)
  {
    messageBox->setWindowTitle(tr("Adding data succeeded"));
    messageBox->setIcon(QMessageBox::Information);
    text = tr("The selected files were loaded successfully but errors or warnings were reported.");
  }
  else
  {
    messageBox->setWindowTitle(tr("Adding data failed"));
    messageBox->setIcon(QMessageBox::Critical);
    text = tr("Error occurred while loading the selected files.");
  }
  text += "\n";
  if (userMessages)
  {
    text += tr("Click 'Show details' button and check the application log for more information.");
    QString messagesStr = QString::fromStdString(userMessages->GetAllMessagesAsString());
    messageBox->setDetailedText(messagesStr);
    qWarning() << Q_FUNC_INFO << "Errors occurred while loading nodes:" << messagesStr;
  }
  else
  {
    text += tr("Check the application log for more information.");
  }
  messageBox->setText(text);
  messageBox->exec();
  messageBox->deleteLater();
}

//-----------------------------------------------------------------------------
qSlicerIOOptionsWidget* qSlicerIOManager::fileOptionsWidget(const QString& fileDescription, QWidget* parent) const
{
  qSlicerIOOptions* options = this->fileOptions(fileDescription);
  qSlicerIOOptionsWidget* optionsWidget = dynamic_cast<qSlicerIOOptionsWidget*>(options);
  if (!optionsWidget)
  {
    delete options;
    return nullptr;
  }
  if (parent)
  {
    optionsWidget->setParent(parent);
  }
  return optionsWidget;
}

//-----------------------------------------------------------------------------
qSlicerIOOptionsWidget* qSlicerIOManager::fileWriterOptionsWidget(vtkObject* object, const QString& extension, QWidget* parent) const
{
  qSlicerIOOptions* options = this->fileWriterOptions(object, extension);
  qSlicerIOOptionsWidget* optionsWidget = dynamic_cast<qSlicerIOOptionsWidget*>(options);
  if (!optionsWidget)
  {
    delete options;
    return nullptr;
  }
  if (parent)
  {
    optionsWidget->setParent(parent);
  }
  return optionsWidget;
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerIOManager::createGenericOptions(vtkMRMLFileIOHandler* ioHandler) const
{
  if (!ioHandler)
  {
    return nullptr;
  }
  vtkNew<vtkMRMLIOOptionsDescription> description;
  ioHandler->FillOptionsDescription(nullptr, description);
  if (description->GetNumberOfOptions() == 0)
  {
    return nullptr;
  }
  qSlicerGenericIOOptionsWidget* options = new qSlicerGenericIOOptionsWidget();
  options->setMRMLScene(ioHandler->GetScene() ? ioHandler->GetScene() : qSlicerCoreApplication::application()->mrmlScene());
  options->setIOHandler(ioHandler);
  return options;
}
