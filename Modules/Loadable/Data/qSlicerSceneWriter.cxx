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
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>

// CTK includes
#include <ctkMessageBox.h>
#include <ctkUtils.h>

// QtCore includes
#include "qMRMLUtils.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerSceneWriter.h"
#include "vtkSlicerApplicationLogic.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
qSlicerSceneWriter::qSlicerSceneWriter(QObject* parentObject)
  : Superclass(parentObject)
{
}

//----------------------------------------------------------------------------
qSlicerSceneWriter::~qSlicerSceneWriter() = default;

//----------------------------------------------------------------------------
QString qSlicerSceneWriter::description()const
{
  return tr("MRML Scene");
}

//----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerSceneWriter::fileType()const
{
  return QString("SceneFile");
}

//----------------------------------------------------------------------------
bool qSlicerSceneWriter::canWriteObject(vtkObject* object)const
{
  return vtkMRMLScene::SafeDownCast(object);
}

//----------------------------------------------------------------------------
QStringList qSlicerSceneWriter::extensions(vtkObject* object)const
{
  Q_UNUSED(object);
  return QStringList()
    << tr("MRML Scene (.mrml)")
    << tr("Medical Reality Bundle (.mrb)")
    << tr("Slicer Data Bundle (*)");
}

//----------------------------------------------------------------------------
bool qSlicerSceneWriter::write(const qSlicerIO::IOProperties& properties)
{
  this->setWrittenNodes(QStringList());

  Q_ASSERT(!properties["fileName"].toString().isEmpty());
  QFileInfo fileInfo(properties["fileName"].toString());
  bool res = false;
  if (fileInfo.suffix() == "mrml")
    {
    res = this->writeToMRML(properties);
    }
  else if (fileInfo.suffix() == "mrb")
    {
    res = this->writeToMRB(properties);
    }
  else
    {
    res = this->writeToDirectory(properties);
    }
  return res;
}

//----------------------------------------------------------------------------
bool qSlicerSceneWriter::writeToMRML(const qSlicerIO::IOProperties& properties)
{
  // set the mrml scene url first
  Q_ASSERT(!properties["fileName"].toString().isEmpty());
  QFileInfo fileInfo(properties["fileName"].toString());
  QString baseDir = fileInfo.absolutePath();
  QString fullPath = fileInfo.absoluteFilePath();

  this->mrmlScene()->SetURL(fullPath.toUtf8());
  this->mrmlScene()->SetRootDirectory(baseDir.toUtf8());

  if (properties.contains("screenShot"))
    {
    // screenshot is provided, save along with the scene mrml file
    QImage screenShot = properties["screenShot"].value<QImage>();
    // convert to vtkImageData
    vtkNew<vtkImageData> imageData;
    qMRMLUtils::qImageToVtkImageData(screenShot, imageData.GetPointer());
    vtkSlicerApplicationLogic* applicationLogic = qSlicerCoreApplication::application()->applicationLogic();
    Q_ASSERT(this->mrmlScene() == applicationLogic->GetMRMLScene());
    applicationLogic->SaveSceneScreenshot(imageData.GetPointer());
    }

  // write out the mrml file
  bool res = this->mrmlScene()->Commit();

  return res;
}

//----------------------------------------------------------------------------
bool qSlicerSceneWriter::writeToMRB(const qSlicerIO::IOProperties& properties)
{
  //
  // make a temp directory to save the scene into - this will
  // be a uniquely named directory that contains a directory
  // named based on the user's selection.
  //

  QFileInfo fileInfo(properties["fileName"].toString());
  QString baseDir = fileInfo.absolutePath();
  QString fullPath = fileInfo.absoluteFilePath();

  // Save URL and root directory so next time when the scene is saved,
  // again, the same folder and filename is used by default.
  this->mrmlScene()->SetURL(fullPath.toUtf8());
  this->mrmlScene()->SetRootDirectory(baseDir.toUtf8());

  if (!QFileInfo(baseDir).isWritable())
    {
    qWarning() << "Failed to save" << fileInfo.absoluteFilePath() << ":"
               << "Path" << baseDir << "is not writable";
    QMessageBox::critical(nullptr, tr("Save scene as MRB"),
      tr("Failed to save scene as %1 (path %2 is not writeable)").arg(fileInfo.absoluteFilePath()).arg(baseDir));
    return false;
    }

  vtkSmartPointer<vtkImageData> thumbnail;
  if (properties.contains("screenShot"))
    {
    QPixmap screenShot = properties["screenShot"].value<QPixmap>();
    // convert to vtkImageData
    thumbnail = vtkSmartPointer<vtkImageData>::New();
    qMRMLUtils::qImageToVtkImageData(screenShot.toImage(), thumbnail);
    }

  bool success = this->mrmlScene()->WriteToMRB(fullPath.toUtf8(), thumbnail);
  if (!success)
    {
    QMessageBox::critical(nullptr, tr("Save scene as MRB"),
      tr("Failed to save scene as %1").arg(fileInfo.absoluteFilePath()));
    return false;
    }

  // Mark the storable nodes as modified since read, since that flag was reset
  // when the files were written out. If there was newly generated data in the
  // scene that only got saved to the MRB bundle directory, it would be marked
  // as unmodified since read when saving as a MRML file + data. This will not
  // disrupt multiple MRB saves.
  this->mrmlScene()->SetStorableNodesModifiedSinceRead();

  qDebug() << "saved " << fileInfo.absoluteFilePath();
  return true;
}

//---------------------------------------------------------------------------
bool qSlicerSceneWriter::writeToDirectory(const qSlicerIO::IOProperties& properties)
{
  // open a file dialog to let the user choose where to save
  QString saveDirName = properties["fileName"].toString();

  QDir saveDir(saveDirName);
  if (!saveDir.exists())
    {
    QDir().mkpath(saveDir.absolutePath());
    }
  int numFiles = saveDir.count() - 2;
  if (numFiles != 0)
    {
    ctkMessageBox *emptyMessageBox = new ctkMessageBox(nullptr);
    QString error;
    switch(numFiles)
      {
      case -2:
        VTK_FALLTHROUGH;
      case -1:
        error = tr("fails to be created");
        break;
      case 1:
        error = tr("contains 1 file or directory");
        break;
      default:
        error = tr("contains %1 files or directories").arg(numFiles);
        break;
      }
    QString message = tr("Selected directory\n\"%1\"\n%2.\n"
                         "Please choose an empty directory.")
                         .arg(saveDirName)
                         .arg(error);
    emptyMessageBox->setAttribute( Qt::WA_DeleteOnClose, true );
    emptyMessageBox->setIcon(QMessageBox::Warning);
    emptyMessageBox->setText(message);
    emptyMessageBox->exec();
    return false;
    }

  vtkSmartPointer<vtkImageData> imageData;
  if (properties.contains("screenShot"))
    {
    QPixmap screenShot = properties["screenShot"].value<QPixmap>();
    // convert to vtkImageData
    imageData = vtkSmartPointer<vtkImageData>::New();
    qMRMLUtils::qImageToVtkImageData(screenShot.toImage(), imageData);
    }

  vtkSlicerApplicationLogic* applicationLogic =
    qSlicerCoreApplication::application()->applicationLogic();
  Q_ASSERT(this->mrmlScene() == applicationLogic->GetMRMLScene());
  bool retval = applicationLogic->SaveSceneToSlicerDataBundleDirectory(
    saveDirName.toUtf8(), imageData);
  if (retval)
    {
    qDebug() << "Saved scene to dir" << saveDirName;
    }
  else
    {
    qDebug() << "Error saving scene to file!";
    }
  return retval ? true : false;
}
