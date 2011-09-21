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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QComboBox>
#include <QTimer>

// CTK includes
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLColorModel.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>
//#include <vtkMRMLFreeSurferProceduralColorNode.h>

// VTK includes

#include "TestingMacros.h"

// --------------------------------------------------------------------------
int qMRMLColorModelTest2( int argc, char * argv [] )
{
  ctkLogger::configure();

  QApplication app(argc, argv);

  vtkSmartPointer<vtkMRMLColorTableNode> colorTableNode =
    vtkSmartPointer<vtkMRMLColorTableNode>::New();
  colorTableNode->SetType(vtkMRMLColorTableNode::Labels);
  
  qMRMLColorModel colorModel;
  QComboBox labelComboBox;
  labelComboBox.setModel(&colorModel);
  labelComboBox.setRootModelIndex(colorModel.index(0,0));
  labelComboBox.setIconSize(QSize(20,20));

  // Test1 - Check if mrmlColorNode()/setMRMLColorNode() work
  ctkCompare(colorModel.mrmlColorNode(), 0);
  ctkExerciseMethod(&colorModel, setMRMLColorNode, mrmlColorNode,
                    colorTableNode, colorTableNode.GetPointer());
  colorModel.setMRMLColorNode(0);

  // Test2 - Check if noneEnabled()/setNoneEnabled() work
  ctkCompare(colorModel.noneEnabled(), false);
  ctkExerciseMethod(&colorModel, setNoneEnabled, noneEnabled, true, true);

  //colorModel.setNoneEnabled(false);

  colorModel.setMRMLColorNode(colorTableNode);
  // Show widgets
  labelComboBox.show();
  
  QTimer autoExit;
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QObject::connect(&autoExit, SIGNAL(timeout()), &app, SLOT(quit()));
    autoExit.start(1000);
    }

  return app.exec();
}
