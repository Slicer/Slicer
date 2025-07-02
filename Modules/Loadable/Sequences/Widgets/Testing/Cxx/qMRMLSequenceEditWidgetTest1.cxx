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

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QTimer>
#include <QVBoxLayout>

// qMRML includes
#include "qSlicerCoreApplication.h"
#include "qMRMLNodeComboBox.h"
#include "qMRMLSequenceEditWidget.h"

// MRML includes
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkNew.h>

int qMRMLSequenceEditWidgetTest1(int argc, char* argv[])
{
  QApplication app(argc, argv);

  // Set up a test scene with some sequences and data nodes

  vtkNew<vtkMRMLScene> scene;

  // Add two sequence nodes with some transforms in them
  vtkNew<vtkMRMLSequenceNode> sequenceNodeNumeric;
  sequenceNodeNumeric->SetName("Transform sequence (numeric)");
  sequenceNodeNumeric->SetIndexType(vtkMRMLSequenceNode::NumericIndex);
  vtkNew<vtkMRMLSequenceNode> sequenceNodeText;
  sequenceNodeText->SetName("Transform sequence (text)");
  sequenceNodeText->SetIndexType(vtkMRMLSequenceNode::TextIndex);
  for (int i = 0; i < 80; i++)
  {
    vtkNew<vtkMRMLTransformNode> transformNode;
    double indexValue = i * 12.345;
    QString name = QString("Transform %1").arg(indexValue);
    transformNode->SetName(name.toStdString().c_str());
    sequenceNodeNumeric->SetDataNodeAtValue(transformNode, QString::number(indexValue).toStdString());
    sequenceNodeText->SetDataNodeAtValue(transformNode, name.toStdString());
  }
  scene->AddNode(sequenceNodeNumeric);
  scene->AddNode(sequenceNodeText);

  vtkNew<vtkMRMLSequenceNode> sequenceNodeModels;
  sequenceNodeModels->SetName("Model sequence");
  for (int i = 0; i < 10; i++)
  {
    vtkNew<vtkMRMLModelNode> modelNode;
    double indexValue = i * 1.5;
    QString name = QString("Model %1").arg(i);
    modelNode->SetName(name.toStdString().c_str());
    sequenceNodeModels->SetDataNodeAtValue(modelNode, QString::number(indexValue).toStdString());
  }
  scene->AddNode(sequenceNodeModels);

  vtkNew<vtkMRMLSequenceNode> sequenceNodeEmpty;
  sequenceNodeEmpty->SetName("An empty sequence");
  scene->AddNode(sequenceNodeEmpty);

  // Add some more data nodes to the scene (to be able to test node type filtering)
  for (int i = 0; i < 20; i++)
  {
    vtkNew<vtkMRMLTransformNode> transformNode;
    double indexValue = i * 12.345;
    QString name = QString("TestT %1").arg(indexValue);
    transformNode->SetName(name.toStdString().c_str());
    scene->AddNode(transformNode);
  }
  for (int i = 0; i < 30; i++)
  {
    vtkNew<vtkMRMLModelNode> modelNode;
    double indexValue = i * 1.23;
    QString name = QString("TestM %1").arg(indexValue);
    modelNode->SetName(name.toStdString().c_str());
    scene->AddNode(modelNode.GetPointer());
  }

  // Create GUI

  QWidget parentWidget;
  parentWidget.setWindowTitle("qMRMLSequenceBrowserWidgetsTest1");
  QVBoxLayout vbox;
  parentWidget.setLayout(&vbox);

  qMRMLNodeComboBox* nodeSelector = new qMRMLNodeComboBox();
  nodeSelector->setMRMLScene(scene);
  nodeSelector->setNodeTypes(QStringList() << "vtkMRMLSequenceNode");
  nodeSelector->setNoneEnabled(true);
  nodeSelector->setCurrentNode(nullptr);
  vbox.addWidget(nodeSelector);

  qMRMLSequenceEditWidget* editWidget = new qMRMLSequenceEditWidget();
  editWidget->setMRMLScene(scene);
  vbox.addWidget(editWidget);

  QObject::connect(
    nodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), editWidget, SLOT(setMRMLSequenceNode(vtkMRMLNode*)));

  parentWidget.show();
  parentWidget.raise();

  if (argc < 2 || QString(argv[1]) != "-I")
  {
    QTimer::singleShot(200, &app, SLOT(quit()));
  }

  return app.exec();
}
