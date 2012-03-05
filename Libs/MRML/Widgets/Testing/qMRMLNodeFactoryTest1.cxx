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

  This file was originally developed by Luis Ibanez, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>

// MRMLWidgets includes
#include <qMRMLNodeFactory.h>

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>

// STD includes
#include <cstdlib>

int qMRMLNodeFactoryTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  qMRMLNodeFactory nodeFactory;

  // Check default values
  if (nodeFactory.mrmlScene() != 0 ||
      nodeFactory.createNode("vtkMRMLCameraNode") != 0)
    {
    std::cerr << "qMRMLNodeFactory wrong default values" << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLScene> scene;
  nodeFactory.setMRMLScene(scene.GetPointer());

  if (nodeFactory.mrmlScene() != scene.GetPointer())
    {
    std::cerr << "qMRMLNodeFactory::setMRMLScene() failed" << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLNode* createdEmptyNode = nodeFactory.createNode("");
  if (createdEmptyNode != 0)
    {
    std::cerr << "qMRMLNodeFactory::createNode() created a bad node" << std::endl;
    return EXIT_FAILURE;
    }

  // Test a simple node
  vtkMRMLNode* createdNode = nodeFactory.createNode("vtkMRMLCameraNode");
  if (createdNode == 0 ||
      createdNode->IsA("vtkMRMLCameraNode") != 1 ||
      createdNode->GetReferenceCount() != 1 ||
      scene->IsNodePresent(createdNode) == 0)
    {
    std::cerr << "qMRMLNodeFactory::createNode() failed." << std::endl;
    return EXIT_FAILURE;
    }

  // Test a singleton node
  vtkMRMLNode* createdSingletonNode =
    nodeFactory.createNode("vtkMRMLInteractionNode");

  if (createdSingletonNode == 0 ||
      createdSingletonNode->IsA("vtkMRMLInteractionNode") != 1 ||
      createdSingletonNode->GetReferenceCount() != 1 ||
      scene->IsNodePresent(createdSingletonNode) == 0)
    {
    std::cerr << "qMRMLNodeFactory::createNode() failed with singleton:"
              << " node: " << createdSingletonNode
              << " class: " << createdSingletonNode->IsA("vtkMRMLInteractionNode")
              << " refCount: " << createdSingletonNode->GetReferenceCount()
              << " present: " << scene->IsNodePresent(createdSingletonNode)
              << std::endl;
    return EXIT_FAILURE;
    }

  // Test another singleton
  vtkMRMLNode* createdSingletonNode2 =
    nodeFactory.createNode("vtkMRMLInteractionNode");

  // Adding the same singleton in the scene should copy the properties of the
  // node to add in the existing node. \sa vtkMRMLScene::AddNode
  if (createdSingletonNode2 == 0 ||
      createdSingletonNode2 != createdSingletonNode || 
      createdSingletonNode2->IsA("vtkMRMLInteractionNode") != 1 ||
      createdSingletonNode2->GetReferenceCount() != 1 ||
      scene->IsNodePresent(createdSingletonNode2) == 0)
    {
    std::cerr << "qMRMLNodeFactory::createNode() failed with singleton2." << std::endl;
    return EXIT_FAILURE;
    }

  // Test static utility method
  vtkMRMLNode* createdNodeStatic =
    qMRMLNodeFactory::createNode(scene.GetPointer(), "vtkMRMLCameraNode");
  if (createdNodeStatic == 0 ||
      createdNodeStatic->IsA("vtkMRMLCameraNode") != 1 ||
      createdNodeStatic->GetReferenceCount() != 1 ||
      scene->IsNodePresent(createdNodeStatic) == 0)
    {
    std::cerr << "qMRMLNodeFactory::createNode() failed." << std::endl;
    return EXIT_FAILURE;
    }

  // Test attributes
  nodeFactory.addAttribute("attribute1", "value1");
  nodeFactory.addAttribute("attribute2", "value2");
  nodeFactory.removeAttribute("attribute2");
  nodeFactory.removeAttribute("attribute0");

  if (nodeFactory.attribute("attribute1") != "value1" ||
      (nodeFactory.attribute("attribute2").isNull() != true) ||
      (nodeFactory.attribute("attribute0").isNull() != true))
    {
    std::cerr << "qMRMLNodeFactory::addAttribute failed:"
              << " attribute1: " << qPrintable(nodeFactory.attribute("attribute1"))
              << " attribute2: " << qPrintable(nodeFactory.attribute("attribute2"))
              << " attribute0: " << qPrintable(nodeFactory.attribute("attribute0"))
              << std::endl;
    return EXIT_FAILURE;
    }

  // Test createNode with attribute
  vtkMRMLNode* createdNodeWithAttribute1 =
    nodeFactory.createNode("vtkMRMLCameraNode");

  if (createdNodeWithAttribute1 == 0 ||
      strcmp(createdNodeWithAttribute1->GetAttribute("attribute1"), "value1") != 0 ||
      createdNodeWithAttribute1->GetAttribute("attribute2") != 0)
    {
    std::cerr << "qMRMLNodeFactory::createNode() with attribute failed." << std::endl;
    return EXIT_FAILURE;
    }

  // Test basename
  nodeFactory.setBaseName("vtkMRMLCameraNode", "MyBaseName");
  vtkMRMLNode* createdNodeWithBaseName =
    nodeFactory.createNode("vtkMRMLCameraNode");
  vtkMRMLNode* createdNodeWithoutBaseName =
    nodeFactory.createNode("vtkMRMLColorTableNode");
  if (nodeFactory.baseName("vtkMRMLCameraNode") != "MyBaseName" ||
      nodeFactory.baseName("vtkMRMLColorTableNode").isNull() != true ||
      strcmp(createdNodeWithBaseName->GetName(), "MyBaseName") != 0 ||
      strcmp(createdNodeWithoutBaseName->GetName(), "MyBaseName") == 0)
    {
    std::cerr << "qMRMLFactory::setBaseName failed." << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
