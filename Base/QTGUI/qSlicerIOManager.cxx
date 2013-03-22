/// Qt includes
#include <QDebug>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QMetaProperty>
#include <QProgressDialog>
#include <QSettings>

// CTK includes
#include "ctkScreenshotDialog.h"

/// SlicerQt includes
#include "qSlicerIOManager.h"
#include "qSlicerDataDialog.h"
#include "qSlicerModelsDialog.h"
#include "qSlicerSaveDataDialog.h"
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractCoreModule.h"

/// MRML includes
#include <vtkMRMLScene.h>

//-----------------------------------------------------------------------------
class qSlicerIOManagerPrivate
{
  Q_DECLARE_PUBLIC(qSlicerIOManager);

protected:
  qSlicerIOManager* const q_ptr;

public:
  qSlicerIOManagerPrivate(qSlicerIOManager& object);

  /// Return true if a dialog is created, false if a dialog already existed
  bool startProgressDialog(int steps = 1);
  void stopProgressDialog();
  void readSettings();
  void writeSettings();
  QString createUniqueDialogName(qSlicerIO::IOFileType,
                                 qSlicerFileDialog::IOAction,
                                 const qSlicerIO::IOProperties&);

  QStringList                       History;
  QList<QUrl>                       Favorites;
  QMap<QString, qSlicerFileDialog*> ReadDialogs;
  QMap<QString, qSlicerFileDialog*> WriteDialogs;

  QSharedPointer<ctkScreenshotDialog> ScreenshotDialog;
  QProgressDialog*                    ProgressDialog;
};

//-----------------------------------------------------------------------------
// qSlicerIOManagerPrivate methods

//-----------------------------------------------------------------------------
qSlicerIOManagerPrivate::qSlicerIOManagerPrivate(qSlicerIOManager& object)
  :q_ptr(&object)
{
  this->ProgressDialog = 0;
}

//-----------------------------------------------------------------------------
bool qSlicerIOManagerPrivate::startProgressDialog(int steps)
{
  Q_Q(qSlicerIOManager);
  if (this->ProgressDialog)
    {
    return false;
    }
  int max = (steps != 1 ? steps : 100);
  this->ProgressDialog = new QProgressDialog("Loading file... ", "Cancel", 0, max);
  this->ProgressDialog->setWindowTitle(QString("Loading ..."));
  if (steps == 1)
    {
    // We only support cancelling a load action if we can have control over it
    this->ProgressDialog->setCancelButton(0);
    }
  this->ProgressDialog->setWindowModality(Qt::WindowModal);
  this->ProgressDialog->setMinimumDuration(1000);
  this->ProgressDialog->setValue(0);

  if (steps == 1)
    {
    q->qvtkConnect(qSlicerCoreApplication::application()->mrmlScene(),
                    vtkMRMLScene::NodeAddedEvent,
                    q, SLOT(updateProgressDialog()));
    }
  return true;
}

//-----------------------------------------------------------------------------
void qSlicerIOManagerPrivate::stopProgressDialog()
{
  Q_Q(qSlicerIOManager);
  if (!this->ProgressDialog)
    {
    return;
    }
  this->ProgressDialog->setValue(this->ProgressDialog->maximum());

  q->qvtkDisconnect(qSlicerCoreApplication::application()->mrmlScene(),
                    vtkMRMLScene::NodeAddedEvent,
                    q, SLOT(updateProgressDialog()));
  delete this->ProgressDialog;
  this->ProgressDialog = 0;
}

//-----------------------------------------------------------------------------
void qSlicerIOManagerPrivate::readSettings()
{
  QSettings settings;
  settings.beginGroup("ioManager");

  if (!settings.value("favoritesPaths").toList().isEmpty())
    {
    foreach (const QString& varUrl, settings.value("favoritesPaths").toStringList())
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
  foreach (const QUrl& url, q->favorites())
    {
    list << url.toString();
    }
  settings.setValue("favoritesPaths", QVariant(list));
  settings.endGroup();
}

//-----------------------------------------------------------------------------
QString qSlicerIOManagerPrivate::
createUniqueDialogName(qSlicerIO::IOFileType fileType,
                       qSlicerFileDialog::IOAction action,
                       const qSlicerIO::IOProperties& ioProperties)
{
  QString objectName;

  objectName += action == qSlicerFileDialog::Read ? "Add" : "Save";
  objectName += fileType;
  objectName += ioProperties["multipleFiles"].toBool() ? "s" : "";
  objectName += "Dialog";

  return objectName;
}

//-----------------------------------------------------------------------------
// qSlicerIOManager methods

//-----------------------------------------------------------------------------
qSlicerIOManager::qSlicerIOManager(QObject* _parent):Superclass(_parent)
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
bool qSlicerIOManager::openDialog(qSlicerIO::IOFileType fileType, 
                                  qSlicerFileDialog::IOAction action,
                                  qSlicerIO::IOProperties properties)
{
  Q_D(qSlicerIOManager);
  bool deleteDialog = false;
  if (properties["objectName"].toString().isEmpty())
    {
    QString name = d->createUniqueDialogName(fileType, action, properties);
    properties["objectName"] = name;
    }
  qSlicerFileDialog* dialog = action == qSlicerFileDialog::Read ? 
    d->ReadDialogs[fileType] : d->WriteDialogs[fileType];
  if (dialog == 0)
    {
    deleteDialog = true;
    qSlicerStandardFileDialog* standardDialog = 
      new qSlicerStandardFileDialog(this);
    standardDialog->setFileType(fileType);
    standardDialog->setAction(action);
    dialog = standardDialog;
    }
  bool res = dialog->exec(properties);
  if (deleteDialog)
   {
    delete dialog;
    }
  return res;
}

//---------------------------------------------------------------------------
void qSlicerIOManager::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("text/uri-list"))
    {
    event->acceptProposedAction();
    }
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::dropEvent(QDropEvent *event)
{
  qSlicerDataDialog* dataDialog = new qSlicerDataDialog(this);
  dataDialog->dropEvent(event);
  dataDialog->exec();
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::addHistory(const QString& path)
{
  Q_D(qSlicerIOManager);
  d->History << path;
}

//-----------------------------------------------------------------------------
const QStringList& qSlicerIOManager::history()const
{
  Q_D(const qSlicerIOManager);
  return d->History;
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::setFavorites(const QList<QUrl>& urls)
{
  Q_D(qSlicerIOManager);
  QList<QUrl> newFavorites;
  foreach(const QUrl& url, urls)
    {
    newFavorites << url;
    }
  d->Favorites = newFavorites;
}

//-----------------------------------------------------------------------------
const QList<QUrl>& qSlicerIOManager::favorites()const
{
  Q_D(const qSlicerIOManager);
  return d->Favorites;
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::registerDialog(qSlicerFileDialog* dialog)
{
  Q_D(qSlicerIOManager);
  Q_ASSERT(dialog);
  if (dialog->action() == qSlicerFileDialog::Read)
    {
    if (d->ReadDialogs[dialog->fileType()])
      {
      delete d->ReadDialogs[dialog->fileType()];
      }
    d->ReadDialogs[dialog->fileType()] = dialog;
    }
  else if (dialog->action() == qSlicerFileDialog::Write)
    {
    if (d->WriteDialogs[dialog->fileType()])
      {
      delete d->WriteDialogs[dialog->fileType()];
      }
    d->WriteDialogs[dialog->fileType()] = dialog;
    }
  else
    {      
    Q_ASSERT(dialog->action() == qSlicerFileDialog::Read ||
             dialog->action() == qSlicerFileDialog::Write);
    }
  dialog->setParent(this);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::loadNodes(const qSlicerIO::IOFileType& fileType,
                                 const qSlicerIO::IOProperties& parameters,
                                 vtkCollection* loadedNodes)
{
  Q_D(qSlicerIOManager);

  bool needStop = d->startProgressDialog(1);
  d->ProgressDialog->setLabelText(
    "Loading file " + parameters.value("fileName").toString() + " ...");
  if (needStop)
    {
    d->ProgressDialog->setValue(25);
    }

  bool res = this->qSlicerCoreIOManager::loadNodes(fileType, parameters, loadedNodes);
  if (needStop)
    {
    d->stopProgressDialog();
    }
  return res;
}


//-----------------------------------------------------------------------------
bool qSlicerIOManager::loadNodes(const QList<qSlicerIO::IOProperties>& files,
                                 vtkCollection* loadedNodes)
{
  Q_D(qSlicerIOManager);

  bool needStop = d->startProgressDialog(files.count());
  bool res = true;
  foreach(qSlicerIO::IOProperties fileProperties, files)
    {
    res = this->loadNodes(
      static_cast<qSlicerIO::IOFileType>(fileProperties["fileType"].toString()),
      fileProperties,
      loadedNodes) && res;

    this->updateProgressDialog();
    if (d->ProgressDialog->wasCanceled())
      {
      res = false;
      break;
      }
    }

  if (needStop)
    {
    d->stopProgressDialog();
    }

  return res;
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::updateProgressDialog()
{
  Q_D(qSlicerIOManager);
  if (!d->ProgressDialog)
    {
    return;
    }
  int progress = d->ProgressDialog->value();
  d->ProgressDialog->setValue(qMin(progress + 1, d->ProgressDialog->maximum() - 1) );
  // Give time to process graphic events including the progress dialog if needed
  // TBD: Not needed ?
  //qApp->processEvents();
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::openScreenshotDialog()
{
  Q_D(qSlicerIOManager);
  // try opening the Annotation module's screen shot
  qSlicerModuleManager *moduleManager = qSlicerApplication::application()->moduleManager();

  qSlicerAbstractCoreModule *modulePointer = NULL;
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
      d->ScreenshotDialog = QSharedPointer<ctkScreenshotDialog>(
        new ctkScreenshotDialog());
      d->ScreenshotDialog->setWidgetToGrab(
        qSlicerApplication::application()->layoutManager()->viewport());
      }
    d->ScreenshotDialog->show();
    }
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::openSceneViewsDialog()
{
//  Q_D(qSlicerIOManager);
  qSlicerModuleManager *moduleManager = qSlicerApplication::application()->moduleManager();
  if (!moduleManager)
    {
    qWarning() << "qSlicerIOManager::openSceneViewsDialog: unable to get module manager, can't get at the Scene Views module";
    return;
    }

  qSlicerAbstractCoreModule *modulePointer = moduleManager->module("SceneViews");
  if (modulePointer == NULL)
    {
    qWarning() << "qSlicerIOManager::openSceneViewsDialog: Unable to get at the SceneViews module (sceneviews).";
    return;
    }
  QMetaObject::invokeMethod(modulePointer, "showSceneViewDialog");
}

