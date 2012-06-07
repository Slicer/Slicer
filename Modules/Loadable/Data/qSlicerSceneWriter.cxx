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
#include <QFileInfo>
#include <QPixmap>

// QtCore includes
#include "qMRMLUtils.h"
#include "qSlicerSceneWriter.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSceneViewNode.h>
#include <vtkMRMLStorageNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

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
  return qSlicerIO::SceneFile;
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
  return QStringList("MRML Scene(.mrml)");
}

//----------------------------------------------------------------------------
bool qSlicerSceneWriter::write(const qSlicerIO::IOProperties& properties)
{
  this->setWrittenNodes(QStringList());

  // save an explicit default scene view recording the state of the scene when
  // saved to file
  const char *defaultSceneName = "Master Scene View";
  vtkSmartPointer<vtkMRMLSceneViewNode> sceneViewNode = NULL;
  vtkSmartPointer<vtkCollection> oldSceneViewNodes;
  oldSceneViewNodes.TakeReference(
    this->mrmlScene()->GetNodesByClassByName("vtkMRMLSceneViewNode", defaultSceneName));
  if (oldSceneViewNodes->GetNumberOfItems() == 0)
    {
    // make a new one
    vtkNew<vtkMRMLSceneViewNode> newSceneViewNode;
    //newSceneViewNode->SetScene(this->mrmlScene());
    newSceneViewNode->SetName(defaultSceneName);
    newSceneViewNode->SetSceneViewDescription("Scene at MRML file save point");
    this->mrmlScene()->AddNode(newSceneViewNode.GetPointer());

    // create a storage node
    vtkMRMLStorageNode *storageNode = newSceneViewNode->CreateDefaultStorageNode();
    // set the file name from the node name
    std::string fname = std::string(newSceneViewNode->GetName()) + std::string(".png");
    storageNode->SetFileName(fname.c_str());
    this->mrmlScene()->AddNode(storageNode);
    newSceneViewNode->SetAndObserveStorageNodeID(storageNode->GetID());
    storageNode->Delete();

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
    QPixmap screenShot = properties["screenShot"].value<QPixmap>();
    // convert to vtkImageData
    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    qMRMLUtils::qImageToVtkImageData(screenShot.toImage(), imageData);
    sceneViewNode->SetScreenShot(imageData);
    // mark it modified since read so that the screen shot will get saved to disk
    sceneViewNode->ModifiedSinceReadOn();
    }
  sceneViewNode->StoreScene();

  // force a write
  sceneViewNode->GetStorageNode()->WriteData(sceneViewNode);

  Q_ASSERT(!properties["fileName"].toString().isEmpty());
  QString fileName = properties["fileName"].toString();

  this->mrmlScene()->SetURL(fileName.toLatin1());
  this->mrmlScene()->SetVersion("Slicer4");
  bool res = this->mrmlScene()->Commit();
  return res;
}
