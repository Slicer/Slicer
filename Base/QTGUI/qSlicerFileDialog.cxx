/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

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
class qSlicerFileDialogPrivate
{
public:
};
*/

//-----------------------------------------------------------------------------
qSlicerFileDialog::qSlicerFileDialog(QObject* _parent)
  :QObject(_parent)
{
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
class qSlicerStandardFileDialogPrivate
{
public:
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
  , d_ptr(new qSlicerStandardFileDialogPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerStandardFileDialog::~qSlicerStandardFileDialog()
{
}

//-----------------------------------------------------------------------------
void qSlicerStandardFileDialog::setFileType(qSlicerIO::IOFileType _fileType)
{
  Q_D(qSlicerStandardFileDialog);
  d->FileType = _fileType;
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerStandardFileDialog::fileType()const
{
  Q_D(const qSlicerStandardFileDialog);
  return d->FileType;
}


//-----------------------------------------------------------------------------
void qSlicerStandardFileDialog::setAction(qSlicerFileDialog::IOAction dialogAction)
{
  Q_D(qSlicerStandardFileDialog);
  d->Action = dialogAction;
}

//-----------------------------------------------------------------------------
qSlicerFileDialog::IOAction qSlicerStandardFileDialog::action()const
{
  Q_D(const qSlicerStandardFileDialog);
  return d->Action;
}

//-----------------------------------------------------------------------------
bool qSlicerStandardFileDialog::exec(const qSlicerIO::IOProperties& ioProperties)
{
  Q_D(qSlicerStandardFileDialog);
  Q_ASSERT(!ioProperties.contains("fileName"));
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();
  ctkFileDialog fileDialog(qobject_cast<QWidget*>(this->parent()));
#ifdef Slicer_USE_KWWIDGETS
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
