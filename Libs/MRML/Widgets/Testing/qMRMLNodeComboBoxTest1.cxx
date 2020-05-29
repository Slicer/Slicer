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

// Slicer includes
#include "vtkSlicerConfigure.h"

// qMRML includes
#include "qMRMLNodeComboBox.h"
#include "qMRMLSceneFactoryWidget.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include "qMRMLWidget.h"

// STD includes

int qMRMLNodeComboBoxTest1( int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  qMRMLNodeComboBox nodeSelector;

  // default state
  bool addEnabled = nodeSelector.addEnabled();
  bool removeEnabled = nodeSelector.removeEnabled();
  bool editEnabled = nodeSelector.editEnabled();
  // Add button should be enabled
  if (!(addEnabled && removeEnabled && !editEnabled))
    {
    std::cerr << __LINE__ << " - Incorrect state" << std::endl
                          << " Expected => Add[1], Remove[1], Edit[0]" << std::endl
                          << " Current => Add[" << addEnabled << "], "
                          << "Remove[" << removeEnabled << "], "
                          << "Edit[" << editEnabled << "]" << std::endl;
    return EXIT_FAILURE;
    }

  qMRMLSceneFactoryWidget sceneFactory;

  sceneFactory.generateScene();

  int currentCount = nodeSelector.nodeCount();
  if (currentCount != 0)
    {
    std::cerr << __LINE__ << " - Error in count() - Expected: 0, current:" << currentCount << std::endl;
    return EXIT_FAILURE;
    }

  // Test: setMRMLScene()/mrmlScene()
  nodeSelector.setMRMLScene(sceneFactory.mrmlScene());
  if (nodeSelector.mrmlScene() != sceneFactory.mrmlScene())
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector::setMRMLScene() failed." << std::endl;
    return EXIT_FAILURE;
    }
  // test a second time, just to make sure methods are well reinit.
  sceneFactory.generateScene();
  nodeSelector.setMRMLScene(sceneFactory.mrmlScene());
  if (nodeSelector.mrmlScene() != sceneFactory.mrmlScene())
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector::setMRMLScene() failed." << std::endl;
    return EXIT_FAILURE;
    }

  // no type has been given yet -> no item should be listed
  if (nodeSelector.nodeCount() != 0)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector::count() failed: "
              << nodeSelector.nodeCount() << " nodes." <<std::endl;
    return EXIT_FAILURE;
    }

  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();

  // All the types are accepted when no type has been given.
  // the nodeselector may or may not contain nodes (some are hidden)
  /*if (nodeSelector.nodeCount())
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector::count() failed:"
              << nodeSelector.nodeCount() << " nodes instead of "
              << sceneFactory.mrmlScene()->GetNumberOfNodes() << std::endl;
    return EXIT_FAILURE;
    }
  */

  nodeSelector.setMRMLScene(nullptr);
  if (nodeSelector.mrmlScene() != nullptr)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector::setMRMLScene() failed." << std::endl;
    return EXIT_FAILURE;
    }

  // test nodeType
  sceneFactory.generateScene();
  sceneFactory.generateNode("vtkMRMLViewNode");

  nodeSelector.setNodeTypes(QStringList("vtkMRMLViewNode"));
  nodeSelector.setNodeTypeLabel("3D view", "vtkMRMLViewNode");
  nodeSelector.setMRMLScene(sceneFactory.mrmlScene());
  if (nodeSelector.nodeCount() != 1)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector: NodeType failed."
              << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.generateNode("vtkMRMLViewNode");
  if (nodeSelector.nodeCount() != 2)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector: NodeType failed."
              << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.mrmlScene()->RemoveNode(sceneFactory.mrmlScene()->GetNthNode(0));
  if (nodeSelector.nodeCount() != 1)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector: NodeType failed."
              << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.mrmlScene()->RemoveNode(sceneFactory.mrmlScene()->GetNthNode(0));
  if (nodeSelector.nodeCount() != 0)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector: NodeType failed."
              << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.generateNode("vtkMRMLViewNode");
  if (nodeSelector.nodeCount() != 1)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector: NodeType failed."
              << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.deleteScene();
  if (nodeSelector.nodeCount() != 0)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector: mrml scene events failed."
              << std::endl;
    return EXIT_FAILURE;
    }
  if (nodeSelector.currentNode() != nullptr)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector: currentNode failed."
              << std::endl;
    return EXIT_FAILURE;
    }
  // FIXME: add more basic tests here

  // Check Attributes
  sceneFactory.generateScene();
  sceneFactory.generateNode("vtkMRMLViewNode");
  sceneFactory.generateNode("vtkMRMLViewNode");
  sceneFactory.generateNode("vtkMRMLViewNode");
  sceneFactory.generateNode("vtkMRMLViewNode");
  sceneFactory.generateNode("vtkMRMLViewNode");

  vtkMRMLNode* node = sceneFactory.mrmlScene()->GetNthNode(0);
  node->SetAttribute("foo", "bar");
  node = sceneFactory.mrmlScene()->GetNthNode(1);
  node->SetAttribute("foo", "bar2");

  nodeSelector.addAttribute("vtkMRMLViewNode", "foo", QString("bar2"));
  nodeSelector.setMRMLScene(sceneFactory.mrmlScene());
  if (nodeSelector.nodeCount() != 1)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector: attribute filtering failed, expected 1 node with attribute foo set to bar2. nodeCount = "
              << nodeSelector.nodeCount() << std::endl;
    return EXIT_FAILURE;
    }

  // Check hide child node type
  sceneFactory.generateNode("vtkMRMLTransformNode");
  sceneFactory.generateNode("vtkMRMLTransformNode");
  sceneFactory.generateNode("vtkMRMLGridTransformNode");
  nodeSelector.setNodeTypes(QStringList("vtkMRMLTransformNode"));
  if (nodeSelector.nodeCount() != 3)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector: node type filtering failed." << nodeSelector.nodeCount() << std::endl;
    return EXIT_FAILURE;
    }
  nodeSelector.setShowChildNodeTypes(false);
  if (nodeSelector.nodeCount() != 2)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector: show child node types failed." << std::endl;
    return EXIT_FAILURE;
    }
  nodeSelector.setShowChildNodeTypes(true);
  if (nodeSelector.nodeCount() != 3)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector: show child node types failed." << std::endl;
    return EXIT_FAILURE;
    }
  nodeSelector.setHideChildNodeTypes(QStringList("vtkMRMLGridTransformNode"));
  if (nodeSelector.nodeCount() != 2)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector: show child node types failed." << std::endl;
    return EXIT_FAILURE;
    }

  // Checks with more than 1 type
  QStringList types;
  //types << "vtkMRMLViewNode" << "vtkMRMLCameraNode";
  // don't use the view node as that has an attribute filter on it
  types << "vtkMRMLModelNode" << "vtkMRMLCameraNode";
  //test setNodeTypes()/nodeTypes()
  nodeSelector.setNodeTypes(types);

  if (nodeSelector.nodeTypes() != types)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector::setNodeTypes() failed." << std::endl;
    return EXIT_FAILURE;
    }

  sceneFactory.generateScene();
  sceneFactory.generateNode(types[0]);
  sceneFactory.generateNode(types[0]);
  sceneFactory.generateNode(types[0]);
  sceneFactory.generateNode("vtkMRMLLinearTransformNode");
  sceneFactory.generateNode(types[1]);
  sceneFactory.generateNode(types[1]);

  nodeSelector.setMRMLScene(sceneFactory.mrmlScene());
  // had added 5 nodes of the right type, and one extra
  if (nodeSelector.nodeCount() != 5)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector:NodeTypeS: setMRMLScene fails, expected node count of 5, got " << nodeSelector.nodeCount() << std::endl;
    return EXIT_FAILURE;
    }
  // add another node of the right type, expect 6 now
  sceneFactory.generateNode(types[1]);
  if (nodeSelector.nodeCount() != 6)
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector:NodeTypeS: node added to the scene fails, expected nodeCount of 6, got " << nodeSelector.nodeCount() << std::endl;
    return EXIT_FAILURE;
    }

  nodeSelector.setMRMLScene(nullptr);

  currentCount = nodeSelector.nodeCount();
  if (currentCount != 0)
    {
    std::cerr << __LINE__ << " - Error in count() - Expected: 0, current:" << currentCount << std::endl;
    return EXIT_FAILURE;
    }

  //
  // Let's connect the sceneFactory with the widget
  //

  QObject::connect(&sceneFactory, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   &nodeSelector, SLOT(setMRMLScene(vtkMRMLScene*)));


  // Let's check the state of the buttons

  sceneFactory.generateScene();

  // Test: setMRMLScene()/mrmlScene()
  nodeSelector.setMRMLScene(sceneFactory.mrmlScene());
  if (nodeSelector.mrmlScene() != sceneFactory.mrmlScene())
    {
    std::cerr << __LINE__ << " - qMRMLNodeSelector::setMRMLScene() failed." << std::endl;
    return EXIT_FAILURE;
    }

  sceneFactory.deleteScene();

  return EXIT_SUCCESS;
}
