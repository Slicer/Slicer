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
#include <vtkMRMLSceneViewNode.h>
#include <vtkMRMLStorageNode.h>

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
qSlicerSceneWriter::~qSlicerSceneWriter()
{
}

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
  QString fileName = properties["fileName"].toString();

  this->mrmlScene()->SetURL(fileName.toLatin1());
  std::string parentDir = vtksys::SystemTools::GetParentDirectory(this->mrmlScene()->GetURL());
  this->mrmlScene()->SetRootDirectory(parentDir.c_str());

  // save an explicit default scene view recording the state of the scene when
  // saved to file
  const char *defaultSceneName = "Master Scene View";
  vtkSmartPointer<vtkMRMLSceneViewNode> sceneViewNode;
  vtkSmartPointer<vtkCollection> oldSceneViewNodes;
  oldSceneViewNodes.TakeReference(
    this->mrmlScene()->GetNodesByClassByName("vtkMRMLSceneViewNode", defaultSceneName));
  if (oldSceneViewNodes->GetNumberOfItems() == 0)
    {
    // make a new one
    vtkNew<vtkMRMLSceneViewNode> newSceneViewNode;
    newSceneViewNode->SetName(defaultSceneName);
    newSceneViewNode->SetSceneViewDescription("Scene at MRML file save point");
    this->mrmlScene()->AddNode(newSceneViewNode.GetPointer());

    // create a storage node
    // set the file name from the node name
    std::string fname = std::string(newSceneViewNode->GetName()) + std::string(".png");
    newSceneViewNode->AddDefaultStorageNode(fname.c_str());

    // use the new one
    sceneViewNode = newSceneViewNode.GetPointer();
    }
  else
    {
    // take the first one and over write it
    sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(
      oldSceneViewNodes->GetItemAsObject(0));
    }

  if (properties.contains("screenShot"))
    {
    // take a screen shot of the full layout
    sceneViewNode->SetScreenShotType(4);
    QImage screenShot = properties["screenShot"].value<QImage>();
    // convert to vtkImageData
    vtkNew<vtkImageData> imageData;
    qMRMLUtils::qImageToVtkImageData(screenShot, imageData.GetPointer());
    sceneViewNode->SetScreenShot(imageData.GetPointer());
    }
  sceneViewNode->StoreScene();

  // force a write
  sceneViewNode->GetStorageNode()->WriteData(sceneViewNode);

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
  QString basePath = fileInfo.absolutePath();
  if (!QFileInfo(basePath).isWritable())
    {
    qWarning() << "Failed to save" << fileInfo.absoluteFilePath() << ":"
               << "Path" << basePath << "is not writable";
    return false;
    }

  // TODO: switch to QTemporaryDir in Qt5.
  // For now, create a named directory and use Qt calls to remove it
  QString tempDir = qSlicerCoreApplication::application()->temporaryPath();
  QFileInfo pack(QDir(tempDir), //QDir::tempPath(),
                 QString("__BundleSaveTemp-") +
                  QDateTime::currentDateTime().toString("yyyy-MM-dd_hh+mm+ss.zzz"));
  qDebug() << "packing to " << pack.absoluteFilePath();

  // make a subdirectory with the name the user has chosen
  QFileInfo bundle = QFileInfo(QDir(pack.absoluteFilePath()),
                               fileInfo.completeBaseName());
  QString bundlePath = bundle.absoluteFilePath();
  if ( bundle.exists() )
    {
    if ( !ctk::removeDirRecursively(bundlePath) )
      {
      QMessageBox::critical(0, tr("Save Scene as MRB"), tr("Could not remove temp directory"));
      return false;
      }
    }

  if ( !QDir().mkpath(bundlePath) )
    {
    QMessageBox::critical(0, tr("Save scene as MRB"), tr("Could not make temp directory"));
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

  //
  // Now save the scene into the bundle directory and then make a zip (mrb) file
  // in the user's selected file location
  //
  vtkSlicerApplicationLogic* applicationLogic =
    qSlicerCoreApplication::application()->applicationLogic();
  Q_ASSERT(this->mrmlScene() == applicationLogic->GetMRMLScene());
  bool retval =
    applicationLogic->SaveSceneToSlicerDataBundleDirectory(bundlePath.toLatin1(),
                                                           imageData);
  if (!retval)
    {
    QMessageBox::critical(0, tr("Save scene as MRB"), tr("Failed to create bundle"));
    return false;
    }

  qDebug() << "zipping to " << fileInfo.absoluteFilePath();
  if ( !applicationLogic->Zip(fileInfo.absoluteFilePath().toLatin1(),
                              bundlePath.toLatin1()) )
    {
    QMessageBox::critical(0, tr("Save scene as MRB"), tr("Could not compress bundle"));
    return false;
    }

  //
  // Now clean up the temp directory
  //
  if ( !ctk::removeDirRecursively(bundlePath) )
    {
    QMessageBox::critical(0, tr("Save scene as MRB"), tr("Could not remove temp directory"));
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
  QString tempDir = qSlicerCoreApplication::application()->temporaryPath();
  QString saveDirName = properties["fileName"].toString();

  QDir saveDir(saveDirName);
  if (!saveDir.exists())
    {
    QDir().mkpath(saveDir.absolutePath());
    }
  int numFiles = saveDir.count() - 2;
  if (numFiles != 0)
    {
    ctkMessageBox *emptyMessageBox = new ctkMessageBox(0);
    QString error;
    switch(numFiles)
      {
      case -2:
      case -1:
        error = tr("fails to be created");
      case 1:
        error = tr("contains 1 file or directory");
      default:
        error = tr("contains %1 files or directories").arg(numFiles);
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
    saveDirName.toLatin1(), imageData);
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
