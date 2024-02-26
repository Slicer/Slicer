/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
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
#include <QMainWindow>

// CTK includes
#include <ctkFileDialog.h>
#include <ctkUtils.h>

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerFileReader.h"
#include "qSlicerIOManager.h"
#include "qSlicerIOOptionsWidget.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>

/*
//-----------------------------------------------------------------------------
class qSlicerFileDialogPrivate
{
public:
};
*/

//-----------------------------------------------------------------------------
qSlicerFileDialog::qSlicerFileDialog(QObject* _parent)
  : QObject(_parent)
{
  qRegisterMetaType<qSlicerFileDialog::IOAction>("qSlicerFileDialog::IOAction");
}

//-----------------------------------------------------------------------------
qSlicerFileDialog::~qSlicerFileDialog() = default;

//-----------------------------------------------------------------------------
QStringList qSlicerFileDialog::nameFilters(qSlicerIO::IOFileType fileType)
{
  QStringList filters;
  QStringList extensions;
  QList<qSlicerFileReader*> readers = qSlicerApplication::application()->ioManager()->readers(fileType);
  foreach (const qSlicerFileReader* reader, readers)
  {
    foreach (const QString& filter, reader->extensions())
    {
      QString nameFilter = filter.contains('(') ? filter : reader->description() + " (" + filter + ")";
      filters << nameFilter;
      extensions << ctk::nameFilterToExtensions(nameFilter);
    }
  }
  filters.insert(0, tr("All") + " (" + extensions.join(" ") + ")");
  return filters;
}

//-----------------------------------------------------------------------------
bool qSlicerFileDialog::isMimeDataAccepted(const QMimeData* mimeData) const
{
  Q_UNUSED(mimeData);
  return false;
}

//-----------------------------------------------------------------------------
void qSlicerFileDialog::dropEvent(QDropEvent* event)
{
  Q_UNUSED(event);
}

//-----------------------------------------------------------------------------
QStringList qSlicerFileDialog::loadedNodes() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
class qSlicerStandardFileDialogPrivate
{
public:
  qSlicerStandardFileDialogPrivate();
  qSlicerIO::IOFileType FileType;
  QString Description;
  qSlicerFileDialog::IOAction Action;
  QStringList LoadedNodes;
};

//-----------------------------------------------------------------------------
qSlicerStandardFileDialogPrivate::qSlicerStandardFileDialogPrivate()
{
  this->FileType = QString("NoFile");
  this->Action = qSlicerFileDialog::Read;
}

//-----------------------------------------------------------------------------
qSlicerStandardFileDialog::qSlicerStandardFileDialog(QObject* _parent)
  : qSlicerFileDialog(_parent)
  , d_ptr(new qSlicerStandardFileDialogPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerStandardFileDialog::~qSlicerStandardFileDialog() = default;

//-----------------------------------------------------------------------------
void qSlicerStandardFileDialog::setFileType(qSlicerIO::IOFileType _fileType)
{
  Q_D(qSlicerStandardFileDialog);
  d->FileType = _fileType;
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerStandardFileDialog::fileType() const
{
  Q_D(const qSlicerStandardFileDialog);
  return d->FileType;
}

//-----------------------------------------------------------------------------
void qSlicerStandardFileDialog::setDescription(const QString& description)
{
  Q_D(qSlicerStandardFileDialog);
  d->Description = description;
}

//-----------------------------------------------------------------------------
QString qSlicerStandardFileDialog::description() const
{
  Q_D(const qSlicerStandardFileDialog);
  return d->Description;
}

//-----------------------------------------------------------------------------
void qSlicerStandardFileDialog::setAction(qSlicerFileDialog::IOAction dialogAction)
{
  Q_D(qSlicerStandardFileDialog);
  d->Action = dialogAction;
}

//-----------------------------------------------------------------------------
qSlicerFileDialog::IOAction qSlicerStandardFileDialog::action() const
{
  Q_D(const qSlicerStandardFileDialog);
  return d->Action;
}

//-----------------------------------------------------------------------------
QStringList qSlicerStandardFileDialog::loadedNodes() const
{
  Q_D(const qSlicerStandardFileDialog);
  return d->LoadedNodes;
}

//-----------------------------------------------------------------------------
ctkFileDialog* qSlicerStandardFileDialog::createFileDialog(const qSlicerIO::IOProperties& ioProperties,
                                                           QWidget* parent /*=nullptr*/)
{
  if (ioProperties["objectName"].toString().isEmpty())
  {
    qWarning() << "Impossible to create the ctkFileDialog - No object name has been set";
    return nullptr;
  }

  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();
  ctkFileDialog* fileDialog = new ctkFileDialog(parent);

  if (ioProperties["fileType"].toBool())
  {
    fileDialog->setNameFilters(
      qSlicerFileDialog::nameFilters((qSlicerIO::IOFileType)ioProperties["fileType"].toString()));
  }
  fileDialog->setHistory(ioManager->history());
  if (ioManager->favorites().count())
  {
    fileDialog->setSidebarUrls(ioManager->favorites());
  }
#ifdef Q_OS_MAC
  // Workaround for Mac to show mounted volumes.
  // See issue #2240
  QList<QUrl> sidebarUrls = fileDialog->sidebarUrls();
  sidebarUrls.append(QUrl::fromLocalFile("/Volumes"));
  fileDialog->setSidebarUrls(sidebarUrls);
#endif
  if (ioProperties["multipleFiles"].toBool())
  {
    fileDialog->setFileMode(QFileDialog::ExistingFiles);
  }
  if (ioProperties["fileMode"].toBool())
  {
    fileDialog->setOption(QFileDialog::ShowDirsOnly);
    fileDialog->setFileMode(QFileDialog::Directory);
  }

  fileDialog->setObjectName(ioProperties["objectName"].toString());

  return fileDialog;
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerStandardFileDialog ::options(const qSlicerIO::IOProperties& ioProperties) const
{
  Q_D(const qSlicerStandardFileDialog);
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();
  // warning: we are responsible for the memory of options
  qSlicerIOOptions* options = nullptr;
  if (d->Action == qSlicerFileDialog::Read)
  {
    QStringList fileDescriptions = ioManager->fileDescriptionsByType(this->fileType());
    options = fileDescriptions.count() ? ioManager->fileOptions(fileDescriptions[0]) : nullptr;
  }
  else if (d->Action == qSlicerFileDialog::Write)
  {
    vtkMRMLScene* scene = qSlicerCoreApplication::application()->mrmlScene();
    vtkMRMLNode* nodeToSave = nullptr;
    if (!ioProperties["nodeID"].toString().isEmpty())
    {
      nodeToSave = scene->GetNodeByID(ioProperties["nodeID"].toString().toUtf8());
    }
    QStringList fileDescriptions = ioManager->fileWriterDescriptions(this->fileType());
    // TODO: this seems wrong, a description is provided while the method expects an extension
    options = fileDescriptions.count() ? ioManager->fileWriterOptions(nodeToSave, fileDescriptions[0]) : nullptr;
  }
  // Update options widget based on properties.
  // This allows customizing default settings in the widget. For example,
  // in scene open dialog, Clear scene option can be set to enabled or disabled by default.
  qSlicerIOOptionsWidget* optionsWidget = dynamic_cast<qSlicerIOOptionsWidget*>(options);
  if (optionsWidget)
  {
    optionsWidget->updateGUI(ioProperties);
  }
  return options;
}

//-----------------------------------------------------------------------------
bool qSlicerStandardFileDialog::exec(const qSlicerIO::IOProperties& ioProperties)
{
  Q_D(qSlicerStandardFileDialog);
  Q_ASSERT(!ioProperties.contains("fileName"));

  d->LoadedNodes.clear();

  qSlicerIO::IOProperties properties = ioProperties;
  properties["fileType"] = d->FileType;
  qSlicerApplication* app = qSlicerApplication::application();
  QWidget* mainWindow = app ? app->mainWindow() : nullptr;
  ctkFileDialog* fileDialog = qSlicerStandardFileDialog::createFileDialog(properties, mainWindow);
  QFileDialog::AcceptMode acceptMode =
    (d->Action == qSlicerFileDialog::Read) ? QFileDialog::AcceptOpen : QFileDialog::AcceptSave;
  fileDialog->setAcceptMode(acceptMode);

  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();

  qSlicerIOOptions* options = this->options(properties);
  // warning: we are responsible for the memory of options
  qSlicerIOOptionsWidget* optionsWidget = dynamic_cast<qSlicerIOOptionsWidget*>(options);
  // options is not necessary a qSlicerIOOptionsWidget (for the case of
  // readers/modules with no UI. If there is a UI then add it inside the file
  // dialog.
  if (optionsWidget)
  {
    // fileDialog will reparent optionsWidget and take care of deleting
    // optionsWidget for us.
    fileDialog->setBottomWidget(optionsWidget, tr("Options:"));
    connect(fileDialog, SIGNAL(fileSelectionChanged(QStringList)), optionsWidget, SLOT(setFileNames(QStringList)));
    connect(optionsWidget, SIGNAL(validChanged(bool)), fileDialog, SLOT(setAcceptButtonEnable(bool)));
    fileDialog->setAcceptButtonEnable(optionsWidget->isValid());
  }

  if (ioProperties.contains("defaultFileName"))
  {
    fileDialog->selectFile(ioProperties["defaultFileName"].toString());
  }

  // we do not delete options now as it is still useful later (even if there is
  // no UI.) they are the options of the reader, UI or not.
  bool res = fileDialog->exec();
  if (res)
  {
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    properties = ioProperties;
    if (options)
    {
      properties.unite(options->properties());
    }
    properties["fileName"] = fileDialog->selectedFiles();
    if (d->Action == qSlicerFileDialog::Read)
    {
      vtkNew<vtkCollection> loadedNodes;
      ioManager->loadNodes(this->fileType(), properties, loadedNodes.GetPointer());
      for (int i = 0; i < loadedNodes->GetNumberOfItems(); ++i)
      {
        vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(loadedNodes->GetItemAsObject(i));
        if (node)
        {
          d->LoadedNodes << node->GetID();
        }
      }
      res = !d->LoadedNodes.isEmpty();
    }
    else if (d->Action == qSlicerFileDialog::Write)
    {
      res = ioManager->saveNodes(this->fileType(), properties);
    }
    else
    {
      res = false;
      Q_ASSERT(d->Action == qSlicerFileDialog::Read || d->Action == qSlicerFileDialog::Write);
    }
    QApplication::restoreOverrideCursor();
  }

  ioManager->setFavorites(fileDialog->sidebarUrls());

  // If options is not a qSlicerIOOptionsWidget, we are responsible for
  // deleting options. If it is, then fileDialog would have reparent
  // the options and take care of its destruction
  if (!optionsWidget)
  {
    delete options;
    options = nullptr;
  }

  fileDialog->deleteLater();
  return res;
}

//-----------------------------------------------------------------------------
QStringList qSlicerStandardFileDialog::getOpenFileName(qSlicerIO::IOProperties ioProperties)
{
  QStringList files;
  ioProperties["multipleFiles"] = QFileDialog::ExistingFiles;
  ioProperties["objectName"] = "getOpenFileName";
  qSlicerApplication* app = qSlicerApplication::application();
  QWidget* mainWindow = app ? app->mainWindow() : nullptr;
  ctkFileDialog* fileDialog = qSlicerStandardFileDialog::createFileDialog(ioProperties, mainWindow);
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();

  if (fileDialog->exec() == QDialog::Accepted)
  {
    files = fileDialog->selectedFiles();
  }
  ioManager->setFavorites(fileDialog->sidebarUrls());
  fileDialog->deleteLater();
  return files;
}

//-----------------------------------------------------------------------------
QString qSlicerStandardFileDialog::getExistingDirectory(qSlicerIO::IOProperties ioProperties)
{
  QString directory;
  ioProperties["fileMode"] = QFileDialog::Directory;
  ioProperties["objectName"] = "getExistingDirectory";
  qSlicerApplication* app = qSlicerApplication::application();
  QWidget* mainWindow = app ? app->mainWindow() : nullptr;
  ctkFileDialog* fileDialog = qSlicerStandardFileDialog::createFileDialog(ioProperties, mainWindow);
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();

  if (fileDialog->exec() == QDialog::Accepted)
  {
    directory = fileDialog->selectedFiles().value(0);
  }
  ioManager->setFavorites(fileDialog->sidebarUrls());
  fileDialog->deleteLater();
  return directory;
}
