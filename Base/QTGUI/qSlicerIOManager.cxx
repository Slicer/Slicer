/// Qt includes
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMap>
#include <QString>
#include <QUrl>

/// SlicerQt includes
#include "qSlicerIOManager.h"
#include "qSlicerFileDialog.h"
#include "qSlicerDataDialog.h"
#include "qSlicerSaveDataDialog.h"

/// MRML includes
#include <vtkMRMLScene.h>

//-----------------------------------------------------------------------------
class qSlicerIOManagerPrivate: public ctkPrivate<qSlicerIOManager>
{
public:
  CTK_DECLARE_PUBLIC(qSlicerIOManager);
  void init();
  QStringList                   History;
  QList<QUrl>                   Favorites;
  QMap<int, qSlicerFileDialog*> ReadDialogs;
  QMap<int, qSlicerFileDialog*> WriteDialogs;
};

//-----------------------------------------------------------------------------
void qSlicerIOManagerPrivate::init()
{
  CTK_P(qSlicerIOManager);
  this->Favorites << QUrl::fromLocalFile(QDir::homePath());
  p->registerDialog(new qSlicerStandardFileDialog(p));
  p->registerDialog(new qSlicerDataDialog(p));
  p->registerDialog(new qSlicerSaveDataDialog(p));
}

//-----------------------------------------------------------------------------
qSlicerIOManager::qSlicerIOManager(QObject* _parent):Superclass(_parent)
{
  CTK_INIT_PRIVATE(qSlicerIOManager);
  ctk_d()->init();
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
bool qSlicerIOManager::openImportSceneDialog()
{
  qSlicerIO::IOProperties properties;
  properties["clear"] = false;
  return this->openDialog(qSlicerIO::SceneFile, qSlicerFileDialog::Read, properties);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openLoadVolumeDialog()
{
  return this->openDialog(qSlicerIO::VolumeFile, qSlicerFileDialog::Read);
}

//-----------------------------------------------------------------------------
bool qSlicerIOManager::openDialog(qSlicerIO::IOFileType fileType, 
                                  qSlicerFileDialog::IOAction action, 
                                  const qSlicerIO::IOProperties& properties)
{
  CTK_D(qSlicerIOManager);
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
  CTK_D(qSlicerIOManager);
  d->History << path;
}

//-----------------------------------------------------------------------------
const QStringList& qSlicerIOManager::history()const
{
  CTK_D(const qSlicerIOManager);
  return d->History;
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::addFavorite(const QUrl& url)
{
  CTK_D(qSlicerIOManager);
  d->Favorites << url;
}

//-----------------------------------------------------------------------------
const QList<QUrl>& qSlicerIOManager::favorites()const
{
  CTK_D(const qSlicerIOManager);
  return d->Favorites;
}

//-----------------------------------------------------------------------------
void qSlicerIOManager::registerDialog(qSlicerFileDialog* dialog)
{
  CTK_D(qSlicerIOManager);
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
