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

// Slicer includes
#include "vtkSlicerConfigure.h"

// MRMLWidgets includes
#include <qMRMLNodeFactory.h>

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include "qMRMLWidget.h"

// STD includes
#include <cstdlib>

int qMRMLNodeFactoryTest1( int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  qMRMLNodeFactory nodeFactory;

  // Check default values
  if (nodeFactory.mrmlScene() != nullptr ||
      nodeFactory.createNode("vtkMRMLCameraNode") != nullptr)
    {
    std::cerr << "Line " << __LINE__ << " - qMRMLNodeFactory wrong default values" << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLScene> scene;

  {
    nodeFactory.setMRMLScene(scene.GetPointer());

    if (nodeFactory.mrmlScene() != scene.GetPointer())
      {
      std::cerr << "Line " << __LINE__ << " - qMRMLNodeFactory::setMRMLScene() failed" << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    vtkMRMLNode* createdEmptyNode = nodeFactory.createNode("");
    if (createdEmptyNode != nullptr)
      {
      std::cerr << "Line " << __LINE__ << " - qMRMLNodeFactory::createNode() created a bad node" << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    // Test a simple node
    vtkMRMLNode* createdNode = nodeFactory.createNode("vtkMRMLCameraNode");
    if (createdNode == nullptr ||
        createdNode->IsA("vtkMRMLCameraNode") != 1 ||
        createdNode->GetReferenceCount() != 2 ||
        scene->IsNodePresent(createdNode) == 0)
      {
      std::cerr << "Line " << __LINE__ << " - qMRMLNodeFactory::createNode() failed.\n"
                << " node: " << createdNode << "\n"
                << " class: " << createdNode->IsA("vtkMRMLCameraNode") << "\n"
                << " refCount: " << createdNode->GetReferenceCount() << "\n"
                << " present: " << scene->IsNodePresent(createdNode) << "\n"
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    // Test a singleton node
    vtkMRMLNode* createdSingletonNode =
      nodeFactory.createNode("vtkMRMLInteractionNode");

    if (createdSingletonNode == nullptr ||
        createdSingletonNode->IsA("vtkMRMLInteractionNode") != 1 ||
        createdSingletonNode->GetReferenceCount() != 2 ||
        scene->IsNodePresent(createdSingletonNode) == 0)
      {
      std::cerr << "Line " << __LINE__ << " - qMRMLNodeFactory::createNode() failed with singleton.\n"
                << " node: " << createdSingletonNode << "\n"
                << " class: " << createdSingletonNode->IsA("vtkMRMLInteractionNode") << "\n"
                << " refCount: " << createdSingletonNode->GetReferenceCount() << "\n"
                << " present: " << scene->IsNodePresent(createdSingletonNode) << "\n"
                << std::endl;
      return EXIT_FAILURE;
      }

    // Test another singleton
    vtkMRMLNode* createdSingletonNode2 =
      nodeFactory.createNode("vtkMRMLInteractionNode");

    // Adding the same singleton in the scene should copy the properties of the
    // node to add in the existing node. \sa vtkMRMLScene::AddNode
    if (createdSingletonNode2 == nullptr ||
        createdSingletonNode2 != createdSingletonNode ||
        createdSingletonNode2->IsA("vtkMRMLInteractionNode") != 1 ||
        createdSingletonNode2->GetReferenceCount() != 2 ||
        scene->IsNodePresent(createdSingletonNode2) == 0)
      {
      std::cerr << "Line " << __LINE__ << " - qMRMLNodeFactory::createNode() failed with singleton2.\n"
                << " node: " << createdSingletonNode << " / " << createdSingletonNode2 << "\n"
                << " class: " << createdSingletonNode2->IsA("vtkMRMLInteractionNode") << "\n"
                << " refCount: " << createdSingletonNode2->GetReferenceCount() << "\n"
                << " present: " << scene->IsNodePresent(createdSingletonNode2) << "\n"
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    // Test static utility method
    vtkMRMLNode* createdNodeStatic =
      qMRMLNodeFactory::createNode(scene.GetPointer(), "vtkMRMLCameraNode");
    if (createdNodeStatic == nullptr ||
        createdNodeStatic->IsA("vtkMRMLCameraNode") != 1 ||
        createdNodeStatic->GetReferenceCount() != 2 ||
        scene->IsNodePresent(createdNodeStatic) == 0)
      {
      std::cerr << "Line " << __LINE__ << " - qMRMLNodeFactory::createNode() failed.\n"
                << " node: " << createdNodeStatic << "\n"
                << " class: " << createdNodeStatic->IsA("vtkMRMLCameraNode") << "\n"
                << " refCount: " << createdNodeStatic->GetReferenceCount() << "\n"
                << " present: " << scene->IsNodePresent(createdNodeStatic) << "\n"
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    // Test attributes
    nodeFactory.addAttribute("attribute1", "value1");
    nodeFactory.addAttribute("attribute2", "value2");
    nodeFactory.removeAttribute("attribute2");
    nodeFactory.removeAttribute("attribute0");

    if (nodeFactory.attribute("attribute1") != "value1" ||
        (nodeFactory.attribute("attribute2").isNull() != true) ||
        (nodeFactory.attribute("attribute0").isNull() != true))
      {
      std::cerr << "Line " << __LINE__ << " - qMRMLNodeFactory::addAttribute failed:"
                << " attribute1: " << qPrintable(nodeFactory.attribute("attribute1"))
                << " attribute2: " << qPrintable(nodeFactory.attribute("attribute2"))
                << " attribute0: " << qPrintable(nodeFactory.attribute("attribute0"))
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    // Test createNode with attribute
    vtkMRMLNode* createdNodeWithAttribute1 =
      nodeFactory.createNode("vtkMRMLCameraNode");

    if (createdNodeWithAttribute1 == nullptr ||
        strcmp(createdNodeWithAttribute1->GetAttribute("attribute1"), "value1") != 0 ||
        createdNodeWithAttribute1->GetAttribute("attribute2") != nullptr)
      {
      std::cerr << "Line " << __LINE__ << " - qMRMLNodeFactory::createNode() with attribute failed." << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
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
      std::cerr << "Line " << __LINE__ << " - qMRMLFactory::setBaseName failed." << std::endl;
      return EXIT_FAILURE;
      }
  }
  return EXIT_SUCCESS;
}
