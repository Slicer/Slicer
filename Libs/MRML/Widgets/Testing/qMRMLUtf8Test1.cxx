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
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>

// Slicer includes
#include "vtkSlicerConfigure.h"

// CTK includes
#include <ctkCallback.h>

// MRML includes
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"

// VTK includes
#include "vtkNew.h"
#include "qMRMLWidget.h"

// STD includes
#include <cstdlib>
#include <cstring>
#include <iostream>

vtkMRMLScene* myScene = nullptr;
vtkMRMLNode* myNode = nullptr;
QLineEdit* myLineEdit = nullptr;

//------------------------------------------------------------------------------
void saveScene(void* vtkNotUsed(data))
{
  myNode->SetName(myLineEdit->text().toUtf8());

  QLabel* label = new QLabel(QString::fromUtf8(myNode->GetName()), nullptr);
  label->show();
  label->setAttribute(Qt::WA_DeleteOnClose);
  QTimer::singleShot(500, label, SLOT(close()));
  std::cout << "output: " << myNode->GetName() << std::endl;
  const char* tmpScene = "scene-utf8.mrml";
  myScene->Commit(tmpScene);
}

//------------------------------------------------------------------------------
int qMRMLUtf8Test1(int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkNew<vtkMRMLScene> scene;
  myScene = scene.GetPointer();
  myScene->SetURL(argv[1]);
  myScene->Connect();

  // Check that active code page is UTF-8 on Windows
#ifdef _WIN32
  UINT activeCodePage = GetACP();
  std::cout << "Active code page: " << activeCodePage << std::endl;
  if (activeCodePage != CP_UTF8)
    {
    std::cout << "Active code page is not 65001 (UTF-8)."
      << " This is expected on Windows 10 versions before 1903 (May 2019 Update)."
      << " Further tests are skipped." << std::endl;
    return 0;
    }
#endif

  // Check if node name that contains special characters is loaded correctly
  vtkMRMLScalarVolumeDisplayNode* volumeDisplayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(scene->GetNodeByID("vtkMRMLScalarVolumeDisplayNode1"));
  CHECK_NOT_NULL(volumeDisplayNode);
  QString actualDisplayNodeName = QString::fromUtf8(volumeDisplayNode->GetName());
  // expectedDisplayNodeName contains a number of unicode characters that are not found in Latin1 character set
  // (the word is "a'rvi'ztu"ro" tu:ko:rfu'ro'ge'p" - https://en.wikipedia.org/wiki/Mojibake#Examples)
  QString expectedDisplayNodeName = QString::fromUtf8(
    u8"\u00e1\u0072\u0076\u00ed\u007a\u0074\u0171\u0072\u0151\u0020\u0074\u00fc\u006b\u00f6\u0072\u0066\u00fa\u0072\u00f3\u0067\u00e9\u0070");
  CHECK_BOOL(actualDisplayNodeName == expectedDisplayNodeName, true);

  // Check that volume can be saved into filename with special character and that file can be loaded
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(scene->GetNodeByID("vtkMRMLScalarVolumeNode1"));
  CHECK_NOT_NULL(volumeNode);
  vtkMRMLStorageNode* storageNode = vtkMRMLStorageNode::SafeDownCast(volumeNode->GetStorageNode());
  QString filenameWithSpecialChars = expectedDisplayNodeName + ".nrrd";
  storageNode->SetFileName(filenameWithSpecialChars.toUtf8().constData());
  QFile::remove(QString::fromUtf8(myScene->GetRootDirectory()) + QDir::separator() + filenameWithSpecialChars);
  CHECK_INT(storageNode->WriteData(volumeNode), 1);
  CHECK_INT(storageNode->ReadData(volumeNode), 1);

  // Check that unicode string can be get/set as node name and saved into file
  // (when the application exits, the string that is typed in the editbox is saved in the scene as camera node name)

  myNode = scene->GetNodeByID("vtkMRMLCameraNode1");
  CHECK_NOT_NULL(myNode);

  std::string cameraName = myNode->GetName();
  if (cameraName.find("camera") == std::string::npos)
    {
    std::cerr << "bad encoding." << std::endl;
    return EXIT_FAILURE;
    }

  std::string newName = cameraName.erase(0, std::strlen("camera"));
  myNode->SetName(newName.c_str());
  QString name = QString::fromUtf8(myNode->GetName());

  myLineEdit = new QLineEdit(nullptr);
  myLineEdit->setText(name);
  myLineEdit->show();

  ctkCallback callback;
  callback.setCallback(saveScene);

  QObject::connect(myLineEdit, SIGNAL(textChanged(QString)),
                   &callback, SLOT(invoke()));
  myLineEdit->setText(QString("cam") + myLineEdit->text());

  if (argc < 3 || QString(argv[2]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
