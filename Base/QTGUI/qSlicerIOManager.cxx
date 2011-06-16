/// Qt includes
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMap>
#include <QProgressDialog>
#include <QString>
#include <QUrl>

// CTK includes
#include "ctkScreenshotDialog.h"

/// SlicerQt includes
#include "qSlicerIOManager.h"
#include "qSlicerFileDialog.h"
#include "qSlicerDataDialog.h"
#include "qSlicerModelsDialog.h"
#include "qSlicerSaveDataDialog.h"
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerAbstractModuleRepresentation.h"

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

  void init();
  /// Return true if a dialog is created, false if a dialog already existed
  bool startProgressDialog(int steps = 1);
  void stopProgressDialog();

  QStringList                   History;
  QList<QUrl>                   Favorites;
  QMap<int, qSlicerFileDialog*> ReadDialogs;
  QMap<int, qSlicerFileDialog*> WriteDialogs;

  QSharedPointer<ctkScreenshotDialog> ScreenshotDialog;
  QProgressDialog*              ProgressDialog;
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
void qSlicerIOManagerPrivate::init()
{
  Q_Q(qSlicerIOManager);
  this->Favorites << QUrl::fromLocalFile(QDir::homePath());
  //p->registerDialog(new qSlicerStandardFileDialog(p));
  q->registerDialog(new qSlicerDataDialog(q));
  q->registerDialog(new qSlicerModelsDialog(q));
  q->registerDialog(new qSlicerSaveDataDialog(q));
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
// qSlicerIOManager methods

//-----------------------------------------------------------------------------
qSlicerIOManager::qSlicerIOManager(QObject* _parent):Superclass(_parent)
  , d_ptr(new qSlicerIOManagerPrivate(*this))
{
  Q_D(qSlicerIOManager);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerIOManager::~qSlicerIOManager()
{
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openLoadSceneDialog()
{
  qSlicerIO::IOProperties properties;
  properties["clear"] = true;
  return this->openDialog(qSlicerIO::SceneFile, qSlicerFileDialog::Read, properties);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openAddSceneDialog()
{
  qSlicerIO::IOProperties properties;
  properties["clear"] = false;
  return this->openDialog(qSlicerIO::SceneFile, qSlicerFileDialog::Read, properties);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openDialog(qSlicerIO::IOFileType fileType, 
                                  qSlicerFileDialog::IOAction action, 
                                  const qSlicerIO::IOProperties& properties)
{
  Q_D(qSlicerIOManager);
  bool deleteDialog = false;
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
void qSlicerIOManager::addFavorite(const QUrl& url)
{
  Q_D(qSlicerIOManager);
  d->Favorites << url;
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
    res = this->loadNodes(static_cast<qSlicerIO::IOFileType>(
                            fileProperties["fileType"].toInt()),
                          fileProperties, loadedNodes)
      && res;
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
  if (moduleManager && moduleManager->isLoaded("annotation") == false)
    {
    // load it?
    if (moduleManager->loadModule("annotation") == false)
      {
      qWarning() << "qSlicerIOManager::openScreenshotDialog: Unable to load Annotation module (annotation).";
      }
    }

  qSlicerAbstractCoreModule *modulePointer = NULL;
  if (moduleManager)
    {
    modulePointer = moduleManager->module("annotation");
    }
  if (modulePointer)
    {
    QMetaObject::invokeMethod(modulePointer, "showScreenshotDialog");
    }
  else
    {
    qWarning() << "qSlicerIOManager::openScreenshotDialog: Unable to get Annotation module (annotation), using the CTK screen shot dialog.";
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
  if (moduleManager->isLoaded("sceneviews") == false)
    {
    // load it?
    if (moduleManager->loadModule("sceneviews") == false)
      {
      qWarning() << "qSlicerIOManager::openSceneViewsDialog: Unable to load Scene Views module (sceneviews).";
      return;
      }
    }
  
  qSlicerAbstractCoreModule *modulePointer = moduleManager->module("sceneviews");
  if (modulePointer == NULL)
    {
    qWarning() << "qSlicerIOManager::openSceneViewsDialog: Unable to get at the SceneViews module (sceneviews).";
    return;
    }
  QMetaObject::invokeMethod(modulePointer, "showSceneViewDialog");
}

