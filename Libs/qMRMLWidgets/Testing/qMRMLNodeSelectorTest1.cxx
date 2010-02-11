// QT includes
#include <QApplication>

// qMRML includes
#include "qMRMLNodeSelector.h"
#include "qMRMLSceneFactoryWidget.h"

// MRML includes
#include "vtkMRMLScene.h"

// STD includes
#include <stdlib.h>
#include <iostream>

int qMRMLNodeSelectorTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  qMRMLNodeSelector   nodeSelector;
  qMRMLSceneFactoryWidget sceneFactory;
  //qMRMLNodeFactory nodeFactory;

  sceneFactory.generateScene();
  
  // Test: setMRMLScene()/mrmlScene()
  nodeSelector.setMRMLScene(sceneFactory.mrmlScene());
  if (nodeSelector.mrmlScene() != sceneFactory.mrmlScene())
    {
    std::cerr<< "qMRMLNodeSelector::setMRMLScene() failed." << std::endl;
    return EXIT_FAILURE;
    }
  // test a second time, just to make sure methods are well reinit.
  sceneFactory.generateScene();
  nodeSelector.setMRMLScene(sceneFactory.mrmlScene());
  if (nodeSelector.mrmlScene() != sceneFactory.mrmlScene())
    {
    std::cerr<< "qMRMLNodeSelector::setMRMLScene() failed." << std::endl;
    return EXIT_FAILURE;
    }
  
  // no type has been given yet -> no item shoud be listed
  if (nodeSelector.count())
    {
    std::cerr << "qMRMLNodeSelector::count() failed." << std::endl;
    return EXIT_FAILURE;
    }

  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();

  // no type has been given yet -> no item shoud be listed
  if (nodeSelector.count())
    {
    std::cerr << "qMRMLNodeSelector::count() failed." << std::endl;
    return EXIT_FAILURE;
    }

  nodeSelector.setMRMLScene(0);
  if (nodeSelector.mrmlScene() != 0)
    {
    std::cerr << "qMRMLNodeSelector::setMRMLScene() failed." << std::endl;
    return EXIT_FAILURE;
    }
  
  // test nodeType
  sceneFactory.generateScene();
  sceneFactory.generateNode("vtkMRMLViewNode");

  nodeSelector.setNodeTypes(QStringList("vtkMRMLViewNode"));
  nodeSelector.setMRMLScene(sceneFactory.mrmlScene());
  if (nodeSelector.count() != 1)
    {
    std::cerr << "qMRMLNodeSelector: NodeType failed." << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.generateNode("vtkMRMLViewNode");
  if (nodeSelector.count() != 2)
    {
    std::cerr << "qMRMLNodeSelector: NodeType failed." << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.mrmlScene()->RemoveNode(sceneFactory.mrmlScene()->GetNthNode(0));
  if (nodeSelector.count() != 1)
    {
    std::cerr << "qMRMLNodeSelector: NodeType failed." << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.mrmlScene()->RemoveNode(sceneFactory.mrmlScene()->GetNthNode(0));
  if (nodeSelector.count() != 0)
    {
    std::cerr << "qMRMLNodeSelector: NodeType failed." << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.generateNode("vtkMRMLViewNode");
  if (nodeSelector.count() != 1)
    {
    std::cerr << "qMRMLNodeSelector: NodeType failed." << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.deleteScene();
  if (nodeSelector.count() != 0)
    {
    std::cerr << "qMRMLNodeSelector: mrml scene events failed." << std::endl;
    return EXIT_FAILURE;
    }
  if (nodeSelector.currentNode() != 0)
    {
    std::cerr << "qMRMLNodeSelector: currentNode failed." << std::endl;
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
  if (nodeSelector.count() != 4)
    {
    std::cerr << "qMRMLNodeSelector: attribute filtering failed." 
              << nodeSelector.count() << std::endl;
    return EXIT_FAILURE;
    }

  // Check hide child node type
  sceneFactory.generateNode("vtkMRMLNonlinearTransformNode");
  sceneFactory.generateNode("vtkMRMLNonlinearTransformNode");
  sceneFactory.generateNode("vtkMRMLGridTransformNode");
  nodeSelector.setNodeTypes(QStringList("vtkMRMLNonlinearTransformNode"));
  if (nodeSelector.count() != 3)
    {
    std::cerr << "qMRMLNodeSelector: node type filtering failed." << nodeSelector.count() << std::endl;
    return EXIT_FAILURE;
    }
  nodeSelector.setShowChildNodeTypes(false);
  if (nodeSelector.count() != 2)
    {
    std::cerr << "qMRMLNodeSelector: show child node types failed." << std::endl;
    return EXIT_FAILURE;
    }
  nodeSelector.setShowChildNodeTypes(true);
  if (nodeSelector.count() != 3)
    {
    std::cerr << "qMRMLNodeSelector: show child node types failed." << std::endl;
    return EXIT_FAILURE;
    }
  nodeSelector.setHideChildNodeTypes(QStringList("vtkMRMLGridTransformNode"));
  if (nodeSelector.count() != 2)
    {
    std::cerr << "qMRMLNodeSelector: show child node types failed." << std::endl;
    return EXIT_FAILURE;
    }

  // Checks with more than 1 type
  QStringList types;
  types << "vtkMRMLViewNode" << "vtkMRMLCameraNode";
  //test setNodeTypes()/nodeTypes()
  nodeSelector.setNodeTypes(types);
  
  if (nodeSelector.nodeTypes() != types)
    {
    std::cerr << "qMRMLNodeSelector::setNodeTypes() failed." << std::endl;
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
  if (nodeSelector.count() != 5)
    {
    std::cerr << "qMRMLNodeSelector:NodeTypeS: setMRMLScene fails." << std::endl;
    return EXIT_FAILURE;
    }
  sceneFactory.generateNode(types[1]);
  if (nodeSelector.count() != 6)
    {
    std::cerr << "qMRMLNodeSelector:NodeTypeS: node added to the scene fails." << std::endl;
    return EXIT_FAILURE;
    }
  
  return EXIT_SUCCESS;
}
