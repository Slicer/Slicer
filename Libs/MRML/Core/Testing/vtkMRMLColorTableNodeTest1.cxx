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
#include <vtkLookupTable.h>

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

  for (int fileFormat = 0; fileFormat < 2; fileFormat++)
  {
    // Test ctbl and csv file format
    std::string colorTableFileName = std::string(tempDir) + "/";
    if (fileFormat == 0)
    {
      colorTableFileName += "vtkMRMLColorTableNodeTest1.ctbl";
    }
    else
    {
      colorTableFileName += "vtkMRMLColorTableNodeTest1.csv";
    }
    std::cout << "Output file: " << colorTableFileName << std::endl;

    for (int colorSettingMethod = 0; colorSettingMethod < 2; colorSettingMethod++)
    {
      // Test one-by-one and bulk color setting
      if (colorSettingMethod == 0)
      {
        std::cout << "Setting colors one by one." << std::endl;
      }
      else
      {
        std::cout << "Setting colors in bulk using a lookup table." << std::endl;
      }

      std::string expectedColorNodeId;

      // Write color table to file
      {
        vtkNew<vtkMRMLColorTableNode> colorNode;
        colorNode->SetTypeToUser();

        if (colorSettingMethod == 0)
        {
          // Set colors and names one by one
          colorNode->SetNumberOfColors(3);
          colorNode->SetColor(0, "zero", 0.0, 0.0, 0.0, 1.0);
          colorNode->SetColor(1, "one", 1.0, 0.0, 0.0, 0.5);
          colorNode->SetColor(2, "two", 0.0, 1.0, 0.0, 1.0);
        }
        else
        {
          // Set colors in bulk using a lookup table
          vtkNew<vtkLookupTable> lut;
          lut->SetNumberOfTableValues(3);
          lut->SetTableValue(0, 0.0, 0.0, 0.0, 1.0);
          lut->SetTableValue(1, 1.0, 0.0, 0.0, 0.5);
          lut->SetTableValue(2, 0.0, 1.0, 0.0, 1.0);
          lut->Build();
          colorNode->SetAndObserveLookupTable(lut);
        }

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

      // Read color table from file and test content
      {
        vtkNew<vtkMRMLScene> scene;
        scene->SetRootDirectory(tempDir);
        vtkNew<vtkMRMLParser> parser;
        parser->SetMRMLScene(scene.GetPointer());
        parser->SetFileName(sceneFileName.c_str());
        CHECK_INT(parser->Parse(), 1);

        // test the color node
        vtkMRMLColorTableNode* colorNode =
          vtkMRMLColorTableNode::SafeDownCast(scene->GetNodeByID(expectedColorNodeId.c_str()));
        CHECK_NOT_NULL(colorNode);

        CHECK_INT(colorNode->GetStorageNode()->ReadData(colorNode), 1);

        if (colorSettingMethod == 0)
        {
          CHECK_STRING(colorNode->GetColorName(0), "zero");
          CHECK_STRING(colorNode->GetColorName(1), "one");
          CHECK_STRING(colorNode->GetColorName(2), "two");
        }
        else
        {
          CHECK_STRING(colorNode->GetColorName(0), "");
          CHECK_STRING(colorNode->GetColorName(1), "");
          CHECK_STRING(colorNode->GetColorName(2), "");
        }

        // Check colors
        double tolerance = 1.0 / 255.0; // color is saved as uint8_t, so 1/255 is the precision
        double color[4];
        colorNode->GetColor(0, color);
        CHECK_DOUBLE_TOLERANCE(color[0], 0.0, tolerance);
        CHECK_DOUBLE_TOLERANCE(color[1], 0.0, tolerance);
        CHECK_DOUBLE_TOLERANCE(color[2], 0.0, tolerance);
        CHECK_DOUBLE_TOLERANCE(color[3], 1.0, tolerance);

        colorNode->GetColor(1, color);
        CHECK_DOUBLE_TOLERANCE(color[0], 1.0, tolerance);
        CHECK_DOUBLE_TOLERANCE(color[1], 0.0, tolerance);
        CHECK_DOUBLE_TOLERANCE(color[2], 0.0, tolerance);
        CHECK_DOUBLE_TOLERANCE(color[3], 0.5, tolerance);

        colorNode->GetColor(2, color);
        CHECK_DOUBLE_TOLERANCE(color[0], 0.0, tolerance);
        CHECK_DOUBLE_TOLERANCE(color[1], 1.0, tolerance);
        CHECK_DOUBLE_TOLERANCE(color[2], 0.0, tolerance);
        CHECK_DOUBLE_TOLERANCE(color[3], 1.0, tolerance);
      }
    }
  }
  return EXIT_SUCCESS;
}
