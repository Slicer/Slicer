// Qt includes
#include <QDebug>
#include <QFileDialog>
#include <QList>
#include <QUrl>

// CTK includes

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerFileDialog.h"
#include "qSlicerIOManager.h"
/*
//-----------------------------------------------------------------------------
class qSlicerFileDialogPrivate: public ctkPrivate<qSlicerFileDialog>
{
public:
  CTK_DECLARE_PUBLIC(qSlicerFileDialog);
};
*/

//-----------------------------------------------------------------------------
qSlicerFileDialog::qSlicerFileDialog(QObject* _parent)
  :QObject(_parent)
{
  //CTK_INIT_PRIVATE(qSlicerFileDialog);
}

//-----------------------------------------------------------------------------
qSlicerFileDialog::~qSlicerFileDialog()
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerFileDialog::nameFilters(qSlicerIO::IOFileType fileType)
{
  QStringList filters;
  QStringList extensions;
  QList<qSlicerIO*> readers = 
    qSlicerApplication::application()->ioManager()->ios(fileType);
  foreach(const qSlicerIO* reader, readers)
    {
    QString nameFilter= reader->description() + " (" + reader->extensions() + ")";
    filters << nameFilter;
    extensions << reader->extensions();
    }
  filters.insert(0, QString("All (") + extensions.join(" ") + QString(")"));
  return filters;
}

//-----------------------------------------------------------------------------
class qSlicerStandardFileDialogPrivate: public ctkPrivate<qSlicerStandardFileDialog>
{
public:
  CTK_DECLARE_PUBLIC(qSlicerStandardFileDialog);
  qSlicerStandardFileDialogPrivate();
  qSlicerIO::IOFileType       FileType;
  qSlicerFileDialog::IOAction Action;
};

//-----------------------------------------------------------------------------
qSlicerStandardFileDialogPrivate::qSlicerStandardFileDialogPrivate()
{
  this->FileType = qSlicerIO::NoFile;
  this->Action = qSlicerFileDialog::Read;
}

//-----------------------------------------------------------------------------
qSlicerStandardFileDialog::qSlicerStandardFileDialog(QObject* _parent)
  :qSlicerFileDialog(_parent)
{
  CTK_INIT_PRIVATE(qSlicerStandardFileDialog);
}

//-----------------------------------------------------------------------------
void qSlicerStandardFileDialog::setFileType(qSlicerIO::IOFileType _fileType)
{
  CTK_D(qSlicerStandardFileDialog);
  d->FileType = _fileType;
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerStandardFileDialog::fileType()const
{
  CTK_D(const qSlicerStandardFileDialog);
  return d->FileType;
}


//-----------------------------------------------------------------------------
void qSlicerStandardFileDialog::setAction(qSlicerFileDialog::IOAction dialogAction)
{
  CTK_D(qSlicerStandardFileDialog);
  d->Action = dialogAction;
}

//-----------------------------------------------------------------------------
qSlicerFileDialog::IOAction qSlicerStandardFileDialog::action()const
{
  CTK_D(const qSlicerStandardFileDialog);
  return d->Action;
}

//-----------------------------------------------------------------------------
bool qSlicerStandardFileDialog::exec(const qSlicerIO::IOProperties& ioProperties)
{
  CTK_D(qSlicerStandardFileDialog);
  Q_ASSERT(!ioProperties.contains("fileName"));
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();
  QFileDialog fileDialog(qobject_cast<QWidget*>(this->parent()));
#ifdef Slicer3_USE_KWWIDGETS
  fileDialog.setWindowFlags(fileDialog.windowFlags() | Qt::WindowStaysOnTopHint);
#endif
  fileDialog.setNameFilters(
    qSlicerFileDialog::nameFilters(d->FileType));
  fileDialog.setHistory(ioManager->history());
  if (ioManager->favorites().count())
    {
    fileDialog.setSidebarUrls(ioManager->favorites());
    }
  bool res = fileDialog.exec();
  if (res)
    {
    qSlicerIO::IOProperties properties = ioProperties;
    properties["fileName"] = fileDialog.selectedFiles()[0];
    if (d->Action == qSlicerFileDialog::Read)
      {
      ioManager->loadNodes(this->fileType(), properties);
      }
    else if(d->Action == qSlicerFileDialog::Write)
      {
      ioManager->saveNodes(this->fileType(), properties);
      }
    else
      {
      Q_ASSERT(d->Action == qSlicerFileDialog::Read ||
               d->Action == qSlicerFileDialog::Write);
      }
    }
  return res;
}
