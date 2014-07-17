/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLColorTableStorageNode.h"
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLParser.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

namespace
{

//----------------------------------------------------------------------------
bool CheckString(int line, const std::string& function, const char* current, const char* expected)
{
  bool different = true;
  if (current == 0 || expected == 0)
    {
    different = !(current == 0 && expected == 0);
    }
  else if(strcmp(current, expected) == 0)
    {
    different = false;
    }
  if(different)
    {
    std::cerr << "Line " << line << " - " << function << " : CheckString failed"
              << "\n\tcurrent:" << (current ? current : "<null>")
              << "\n\texpected:" << (expected ? expected : "<null>")
              << std::endl;
    return false;
    }
  return true;
}

}

//---------------------------------------------------------------------------
int vtkMRMLColorTableNodeTest1(int argc, char * argv[])
{
  vtkNew<vtkMRMLColorTableNode> node1;

  EXERCISE_BASIC_OBJECT_METHODS(node1.GetPointer());

  EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS(vtkMRMLColorTableNode, node1.GetPointer());

  if (argc != 2)
    {
    std::cerr << "Missing parameters !\n"
              << "Usage: " << argv[0] << " /path/to/temp"
              << std::endl;
    return EXIT_FAILURE;
    }

  const char* tempDir = argv[1];

  std::string sceneFileName = std::string(tempDir) + "/vtkMRMLColorTableNodeTest1.mrml";
  std::string colorTableFileName = std::string(tempDir) + "/vtkMRMLColorTableNodeTest1.ctbl";

  // check that extra single quotes don't appear in color names via round trip
  // to xml

  std::string expectedColorNodeId;

  {
    vtkNew<vtkMRMLColorTableNode> colorNode;
    colorNode->SetTypeToUser();
    colorNode->SetNumberOfColors(3);
    colorNode->SetColor(0, "zero", 0.0, 0.0, 0.0, 1.0);
    colorNode->SetColor(1, "one", 1.0, 0.0, 0.0, 1.0);
    colorNode->SetColor(2, "two", 0.0, 1.0, 0.0, 1.0);

    vtkSmartPointer<vtkMRMLStorageNode> colorStorageNode =
        vtkSmartPointer<vtkMRMLStorageNode>::Take(colorNode->CreateDefaultStorageNode());

    // add node to the scene
    vtkNew<vtkMRMLScene> scene;
    scene->SetRootDirectory(tempDir);
    if (!scene->AddNode(colorNode.GetPointer()))
      {
      std::cerr << "Problem adding colorNode to the scene !" << std::endl;
      return EXIT_FAILURE;
      }

    // add storage node to the scene
    scene->AddNode(colorStorageNode);
    colorNode->SetAndObserveStorageNodeID(colorStorageNode->GetID());

    // keep track of the id
    expectedColorNodeId = std::string(colorNode->GetID());

    // write color table file
    colorStorageNode->SetFileName(colorTableFileName.c_str());
    colorStorageNode->WriteData(colorNode.GetPointer());
    colorNode->SetName("CustomColorTable");

    // write MRML file
    scene->SetURL(sceneFileName.c_str());
    if (!scene->Commit())
      {
      std::cerr << "Failed to save color node [id:" << colorNode->GetID() << "]"
                << " into scene " << sceneFileName
                << std::endl;
      return EXIT_FAILURE;
      }
  }

  {
    vtkNew<vtkMRMLScene> scene;
    scene->SetRootDirectory(tempDir);
    vtkNew<vtkMRMLParser> parser;
    parser->SetMRMLScene(scene.GetPointer());
    parser->SetFileName(sceneFileName.c_str());
    int result = parser->Parse();
    if (result != 1)
      {
      std::cerr << "Failed to parse scene file " << sceneFileName << std::endl;
      return EXIT_FAILURE;
      }

    // test the color node
    vtkMRMLColorTableNode *colorNode =
        vtkMRMLColorTableNode::SafeDownCast(scene->GetNodeByID(expectedColorNodeId.c_str()));
    if (!colorNode)
      {
      std::cerr << "Failed to get colorNode [id: " << expectedColorNodeId << "]"
                << " from scene file " << sceneFileName
                << std::endl;
      return EXIT_FAILURE;
      }

    if (!colorNode->GetStorageNode()->ReadData(colorNode))
      {
      std::cerr << "Failed to read " << colorTableFileName << std::endl;
      return EXIT_FAILURE;
      }

    if (!CheckString(__LINE__, "GetColorName", colorNode->GetColorName(0), "zero"))
      {
      return EXIT_FAILURE;
      }
    if (!CheckString(__LINE__, "GetColorName", colorNode->GetColorName(1), "one"))
      {
      return EXIT_FAILURE;
      }
    if (!CheckString(__LINE__, "GetColorName", colorNode->GetColorName(2), "two"))
      {
      return EXIT_FAILURE;
      }
  }

  return EXIT_SUCCESS;
}
