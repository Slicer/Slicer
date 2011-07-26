/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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
#include <QTimer>

// qMRML includes
#include "qMRMLNodeComboBox.h"

// MRML includes
#include "vtkMRMLScene.h"

// VTK includes
#include "vtkSmartPointer.h"

// STD includes

int qMRMLNodeComboBoxTest3( int argc, char * argv [] )
{
  if (argc < 2)
    {
    std::cerr<< "Wrong number of arguments." << std::endl;
    return EXIT_FAILURE;
    }
  QApplication app(argc, argv);

  qMRMLNodeComboBox nodeSelector;
  nodeSelector.show();
  nodeSelector.setNodeTypes(QStringList("vtkMRMLViewNode"));
  vtkSmartPointer<vtkMRMLScene> scene =  vtkSmartPointer<vtkMRMLScene>::New();
  nodeSelector.setMRMLScene(scene);
  scene->SetURL(argv[1]);
  scene->Connect();

  if (argc < 3 || QString(argv[2]) != "-I")
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
