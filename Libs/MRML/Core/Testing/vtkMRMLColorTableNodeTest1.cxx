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

using namespace vtkMRMLCoreTestingUtilities;

//---------------------------------------------------------------------------
int vtkMRMLColorTableNodeTest1(int argc, char * argv[])
{
  vtkNew<vtkMRMLColorTableNode> node1;
  {
    vtkNew<vtkMRMLScene> scene;
    scene->AddNode(node1.GetPointer());
    EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  }

  if (argc != 2)
    {
    std::cerr << "Line " << __LINE__
              << " - Missing parameters !\n"
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
    colorNode->NamesInitialisedOn();

    // add node to the scene
    vtkNew<vtkMRMLScene> scene;
    scene->SetRootDirectory(tempDir);
    CHECK_NOT_NULL(scene->AddNode(colorNode.GetPointer()));

    // add storage node to the scene
    vtkSmartPointer<vtkMRMLStorageNode> colorStorageNode =
        vtkSmartPointer<vtkMRMLStorageNode>::Take(colorNode->CreateDefaultStorageNode());

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
    CHECK_INT(scene->Commit(), 1);
  }

  {
    vtkNew<vtkMRMLScene> scene;
    scene->SetRootDirectory(tempDir);
    vtkNew<vtkMRMLParser> parser;
    parser->SetMRMLScene(scene.GetPointer());
    parser->SetFileName(sceneFileName.c_str());
    CHECK_INT(parser->Parse(), 1);

    // test the color node
    vtkMRMLColorTableNode *colorNode =
        vtkMRMLColorTableNode::SafeDownCast(scene->GetNodeByID(expectedColorNodeId.c_str()));
    CHECK_NOT_NULL(colorNode);

    CHECK_INT(colorNode->GetStorageNode()->ReadData(colorNode),1);

    CHECK_STRING(colorNode->GetColorName(0), "zero")
    CHECK_STRING(colorNode->GetColorName(1), "one")
    CHECK_STRING(colorNode->GetColorName(2), "two")
  }

  return EXIT_SUCCESS;
}
