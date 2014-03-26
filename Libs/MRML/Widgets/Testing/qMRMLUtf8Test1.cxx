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

// QT includes
#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>

// CTK includes
#include <ctkCallback.h>

// MRML includes
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include "vtkNew.h"

// STD includes
#include <cstdlib>
#include <cstring>
#include <iostream>

vtkMRMLScene* myScene = 0;
vtkMRMLNode* myNode = 0;
QLineEdit* myLineEdit = 0;

//------------------------------------------------------------------------------
void saveScene(void* vtkNotUsed(data))
{
  myNode->SetName(myLineEdit->text().toUtf8());

  QLabel* label = new QLabel(QString::fromUtf8(myNode->GetName()), 0);
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
  QApplication app(argc, argv);

  vtkNew<vtkMRMLScene> scene;
  myScene = scene.GetPointer();
  myScene->SetURL(argv[1]);
  myScene->Connect();

  myNode = scene->GetNthNode(0);

  std::string cameraName = myNode->GetName();
  if (cameraName.find("camera") == std::string::npos)
    {
    std::cerr << "bad encoding." << std::endl;
    return EXIT_FAILURE;
    }

  std::string newName = cameraName.erase(0, std::strlen("camera"));
  myNode->SetName(newName.c_str());

  myLineEdit = new QLineEdit(0);
  myLineEdit->setText(QString::fromUtf8(myNode->GetName()));
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

