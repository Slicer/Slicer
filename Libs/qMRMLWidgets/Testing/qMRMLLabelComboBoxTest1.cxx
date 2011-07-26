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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QTimer>

// CTK includes
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLLabelComboBox.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>
//#include <vtkMRMLFreeSurferProceduralColorNode.h>

// VTK includes

#include "TestingMacros.h"

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

// --------------------------------------------------------------------------
int qMRMLLabelComboBoxTest1( int argc, char * argv [] )
{
  ctkLogger::configure();

  QApplication app(argc, argv);

  VTK_CREATE(vtkMRMLColorTableNode, colorTableNode);
  colorTableNode->SetType(vtkMRMLColorTableNode::Labels);
  
  qMRMLLabelComboBox labelComboBox;

  // Test1 - Check if mrmlColorNode()/setMRMLColorNode() work
  ctkCompare(labelComboBox.mrmlColorNode(), 0);
  ctkExerciseMethod(&labelComboBox, setMRMLColorNode, mrmlColorNode,
                    colorTableNode, colorTableNode.GetPointer());
  labelComboBox.setMRMLColorNode(0);

  // Test2 - Check if noneEnabled()/setNoneEnabled() work
  ctkCompare(labelComboBox.noneEnabled(), false);
  ctkExerciseMethod(&labelComboBox, setNoneEnabled, noneEnabled, true, true);


  labelComboBox.setNoneEnabled(false);

  // Test3 - Check if currentColor()/setCurrentColor() work properly when
  // no ColorNode has been assigned  and NoneEnabled is false
  ctkCompare(labelComboBox.currentColor(), -1);
  ctkExerciseMethod(&labelComboBox, setCurrentColor, currentColor, 10, -1);

  labelComboBox.setMRMLColorNode(colorTableNode);

  // Test4 - Check if currentColor()/setCurrentColor() work properly when
  // a valid ColorNode has been assigned and NoneEnabled is false
  ctkCompare(labelComboBox.currentColor(), -1);
  ctkExerciseMethod(&labelComboBox, setCurrentColor, currentColor, 5, 5);
  ctkExerciseMethod(&labelComboBox, setCurrentColor, currentColor, -6, 5);
  ctkExerciseMethod(&labelComboBox, setCurrentColor, currentColor, -1, 5);
  ctkExerciseMethod(&labelComboBox, setCurrentColor, currentColor, 256, 256);
  ctkExerciseMethod(&labelComboBox, setCurrentColor, currentColor, 257, 256);

  qMRMLLabelComboBox labelComboBox2;

  labelComboBox2.setNoneEnabled(true);

  // Test5 - Check if currentColor()/setCurrentColor() work properly when
  // no ColorNode has been assigned  and NoneEnabled is true
  ctkCompare(labelComboBox2.currentColor(), -1);
  ctkExerciseMethod(&labelComboBox2, setCurrentColor, currentColor, 10, -1);

  labelComboBox2.setMRMLColorNode(colorTableNode);

  // Test6 - Check if currentColor()/setCurrentColor() work properly when
  // a valid ColorNode has been assigned and NoneEnabled is true
  ctkCompare(labelComboBox2.currentColor(), -1);
  ctkExerciseMethod(&labelComboBox2, setCurrentColor, currentColor, 5, 5);
  ctkExerciseMethod(&labelComboBox2, setCurrentColor, currentColor, -6, 5);
  ctkExerciseMethod(&labelComboBox2, setCurrentColor, currentColor, -1, -1);
  ctkExerciseMethod(&labelComboBox2, setCurrentColor, currentColor, 256, 256);
  ctkExerciseMethod(&labelComboBox2, setCurrentColor, currentColor, 257, 256);

  //Test7 - Check if the property colorNameVisible()/setColorNameVisible() 
  //work properly when assigned
  ctkCompare(labelComboBox2.colorNameVisible(), true);
  ctkExerciseMethod(&labelComboBox2, setColorNameVisible, colorNameVisible, false, false);

  labelComboBox2.setColorNameVisible(false);

  // Show widgets
  labelComboBox.show();
  labelComboBox.printAdditionalInfo();
  labelComboBox2.show();
  labelComboBox2.printAdditionalInfo();
  
  QTimer autoExit;
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QObject::connect(&autoExit, SIGNAL(timeout()), &app, SLOT(quit()));
    autoExit.start(1000);
    }

  return app.exec();
}
