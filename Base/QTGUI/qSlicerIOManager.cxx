/// Qt includes
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMap>
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

  QStringList                   History;
  QList<QUrl>                   Favorites;
  QMap<int, qSlicerFileDialog*> ReadDialogs;
  QMap<int, qSlicerFileDialog*> WriteDialogs;

  QSharedPointer<ctkScreenshotDialog> ScreenshotDialog;
};

//-----------------------------------------------------------------------------
qSlicerIOManagerPrivate::qSlicerIOManagerPrivate(qSlicerIOManager& object)
  :q_ptr(&object)
{
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
void qSlicerIOManager::openScreenshotDialog()
{
  Q_D(qSlicerIOManager);
  if (!d->ScreenshotDialog)
    {
    d->ScreenshotDialog = QSharedPointer<ctkScreenshotDialog>(
        new ctkScreenshotDialog());
    d->ScreenshotDialog->setWidgetToGrab(
        qSlicerApplication::application()->layoutManager()->viewport());
    }
  d->ScreenshotDialog->show();
}
