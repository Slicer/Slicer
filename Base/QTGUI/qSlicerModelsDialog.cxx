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

/// Qt includes
#include <QFileDialog>
#include <QStyle>

/// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerIOManager.h"
#include "qSlicerModelsDialog_p.h"

// VTK includes
#include "vtkCollection.h"
#include "vtkMRMLNode.h"
#include "vtkNew.h"

//-----------------------------------------------------------------------------
qSlicerModelsDialogPrivate::qSlicerModelsDialogPrivate(qSlicerModelsDialog& object, QWidget* _parentWidget)
  : QDialog(_parentWidget)
  , q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerModelsDialogPrivate::~qSlicerModelsDialogPrivate() = default;

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
void qSlicerModelsDialogPrivate::openAddModelFileDialog()
{
  Q_Q(qSlicerModelsDialog);
  QStringList filters = qSlicerFileDialog::nameFilters(q->fileType());
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
  Q_Q(qSlicerModelsDialog);
  // TODO add last open directory.
  QString modelDirectory = QFileDialog::getExistingDirectory(
    this, "Select a Model directory", "", QFileDialog::ReadOnly);
  if (modelDirectory.isEmpty())
    {
    return;
    }

  QStringList filters = qSlicerFileDialog::nameFilters(q->fileType());
  this->SelectedFiles = QDir(modelDirectory).entryList(filters);
  this->accept();
}

//-----------------------------------------------------------------------------
qSlicerModelsDialog::qSlicerModelsDialog(QObject* _parent)
  : qSlicerFileDialog(_parent)
  , d_ptr(new qSlicerModelsDialogPrivate(*this, nullptr))
{
  // FIXME give qSlicerModelsDialog as a parent of qSlicerModelsDialogPrivate;
  Q_D(qSlicerModelsDialog);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerModelsDialog::~qSlicerModelsDialog() = default;

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerModelsDialog::fileType()const
{
  return QString("ModelFile");
}

//-----------------------------------------------------------------------------
QString qSlicerModelsDialog::description()const
{
  return tr("Models");
}

//-----------------------------------------------------------------------------
qSlicerFileDialog::IOAction qSlicerModelsDialog::action()const
{
  return qSlicerFileDialog::Read;
}

//-----------------------------------------------------------------------------
bool qSlicerModelsDialog::exec(const qSlicerIO::IOProperties& readerProperties)
{
  Q_D(qSlicerModelsDialog);
  Q_ASSERT(!readerProperties.contains("fileName"));

  d->LoadedNodeIDs.clear();
  bool res = false;
  if (d->exec() != QDialog::Accepted)
    {
    return res;
    }
  QStringList files = d->SelectedFiles;
  foreach(QString file, files)
    {
    qSlicerIO::IOProperties properties = readerProperties;
    properties["fileName"] = file;
    vtkNew<vtkCollection> loadedNodes;
    res = qSlicerCoreApplication::application()->coreIOManager()
      ->loadNodes(this->fileType(),
                  properties, loadedNodes.GetPointer()) || res;
    for (int i = 0; i < loadedNodes->GetNumberOfItems(); ++i)
      {
      d->LoadedNodeIDs << vtkMRMLNode::SafeDownCast(loadedNodes->GetItemAsObject(i))
        ->GetID();
      }
    }
  return res;
}

//-----------------------------------------------------------------------------
QStringList qSlicerModelsDialog::loadedNodes()const
{
  Q_D(const qSlicerModelsDialog);
  return d->LoadedNodeIDs;
}
