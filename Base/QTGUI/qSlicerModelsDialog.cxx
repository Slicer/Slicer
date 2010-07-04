/// Qt includes
#include <QDebug>
#include <QFileDialog>
#include <QList>

/// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerFileDialog.h"
#include "qSlicerIOManager.h"
#include "qSlicerIOOptionsWidget.h"
#include "qSlicerModelsDialog.h"
#include "qSlicerModelsDialog_p.h"

//-----------------------------------------------------------------------------
qSlicerModelsDialogPrivate::qSlicerModelsDialogPrivate(QWidget* parentWidget)
  :QDialog(parentWidget)
{
}

//-----------------------------------------------------------------------------
qSlicerModelsDialogPrivate::~qSlicerModelsDialogPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerModelsDialogPrivate::init()
{
  this->setupUi(this);
  this->AddModelToolButton->setIcon(this->style()->standardIcon(QStyle::SP_FileIcon));
  this->AddModelDirectoryToolButton->setIcon(this->style()->standardIcon(QStyle::SP_DirIcon));
  connect(this->AddModelToolButton, SIGNAL(clicked()),
          this, SLOT(openAddModelFileDialog()));
  connect(this->AddModelDirectoryToolButton, SIGNAL(clicked()),
          this, SLOT(openAddModelDirectoryDialog()));
}

//-----------------------------------------------------------------------------
QStringList qSlicerModelsDialogPrivate::selectedFiles()const
{
  return this->SelectedFiles;
}

//-----------------------------------------------------------------------------
void qSlicerModelsDialogPrivate::openAddModelFileDialog()
{
  CTK_P(qSlicerModelsDialog);
  QStringList filters = qSlicerFileDialog::nameFilters(p->fileType());
  // TODO add last open directory
  this->SelectedFiles = QFileDialog::getOpenFileNames(
    this, "Select Model file(s)", "", filters.join(", "));
  if (this->SelectedFiles.count() < 1)
    {
    return;
    }
  this->accept();
}

//-----------------------------------------------------------------------------
void qSlicerModelsDialogPrivate::openAddModelDirectoryDialog()
{
  CTK_P(qSlicerModelsDialog);
  // TODO add last open directory.
  QString modelDirectory = QFileDialog::getExistingDirectory(
    this, "Select a Model directory", "", QFileDialog::ReadOnly);
  if (modelDirectory.isEmpty())
    {
    return;
    }

  QStringList filters = qSlicerFileDialog::nameFilters(p->fileType());
  this->SelectedFiles = QDir(modelDirectory).entryList(filters);
  this->accept();
}

//-----------------------------------------------------------------------------
qSlicerModelsDialog::qSlicerModelsDialog(QObject* _parent)
  :qSlicerFileDialog(_parent)
{
  // FIXME give qSlicerModelsDialog as a parent of qSlicerModelsDialogPrivate;
  CTK_INIT_PRIVATE(qSlicerModelsDialog);
  CTK_D(qSlicerModelsDialog);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerModelsDialog::~qSlicerModelsDialog()
{
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerModelsDialog::fileType()const
{
  return qSlicerIO::ModelFile;
}

//-----------------------------------------------------------------------------
qSlicerFileDialog::IOAction qSlicerModelsDialog::action()const
{
  return qSlicerFileDialog::Read;
}

//-----------------------------------------------------------------------------
bool qSlicerModelsDialog::exec(const qSlicerIO::IOProperties& readerProperties)
{
  CTK_D(qSlicerModelsDialog);
  Q_ASSERT(!readerProperties.contains("fileName"));
#ifdef Slicer3_USE_KWWIDGETS
  d->setWindowFlags(d->windowFlags() | Qt::WindowStaysOnTopHint);
#endif
  bool res = false;
  if (d->exec() != QDialog::Accepted)
    {
    return res;
    }
  QStringList files = d->selectedFiles();
  foreach(QString file, files)
    {
    qSlicerIO::IOProperties properties = readerProperties;
    properties["fileName"] = file;
    res = qSlicerCoreApplication::application()->coreIOManager()
      ->loadNodes(this->fileType(),
                  properties) || res;
    }
  return res;
}

