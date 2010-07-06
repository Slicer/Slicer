// Qt includes
#include <QDebug>
#include <QFileDialog>
#include <QList>
#include <QUrl>

// CTK includes
#include <ctkFileDialog.h>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerFileDialog.h"
#include "qSlicerIOManager.h"
#include "qSlicerIOOptionsWidget.h"

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
  ctkFileDialog fileDialog(qobject_cast<QWidget*>(this->parent()));
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
  // warning: we are responsible for the memory of options
  qSlicerIOOptions* options = ioManager->fileOptions(this->fileType());
  qSlicerIOOptionsWidget* optionsWidget =
    dynamic_cast<qSlicerIOOptionsWidget*>(options);
  // options is not necessary a qSLicerIOOptionsWidget (for the case of
  // readers/modules with no UI. If there is a UI then add it inside the  file
  // dialog.
  if (optionsWidget)
    {
    // fileDialog will reparent optionsWidget and take care of deleting
    // optionsWidget for us.
    fileDialog.setBottomWidget(optionsWidget, tr("Options:"));
    // TODO: support selection of more than 1 file
    connect(&fileDialog, SIGNAL(currentChanged(const QString&)),
            optionsWidget, SLOT(setFileName(const QString&)));
    connect(optionsWidget, SIGNAL(validChanged(bool)),
            &fileDialog, SLOT(setAcceptButtonEnable(bool)));
    fileDialog.setAcceptButtonEnable(optionsWidget->isValid());
    }
  // we do not delete options now as it is still useful later (even if there is
  // no UI.) they are the options of the reader, UI or not.
  bool res = fileDialog.exec();
  if (res)
    {
    qSlicerIO::IOProperties properties = ioProperties;
    if (options)
      {
      properties.unite(options->properties());
      }
    else
      {
      // TODO: support selection of more than 1 file
      properties["fileName"] = fileDialog.selectedFiles()[0];
      }
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
  // If options is not a qSlicerIOOptionsWidget, we are responsible for
  // deleting options. If it is, then fileDialog would have reparent
  // the options and take care of its destruction
  if (!optionsWidget)
    {
    delete options;
    options = 0;
    }
  return res;
}
