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
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLParser.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkLookupTable.h>

using namespace vtkMRMLCoreTestingUtilities;

//---------------------------------------------------------------------------
int vtkMRMLColorTableNodeTest1(int argc, char* argv[])
{
  vtkNew<vtkMRMLColorTableNode> node1;
  {
    vtkNew<vtkMRMLScene> scene;
    scene->AddNode(node1.GetPointer());
    EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  }

  if (argc != 3)
  {
    std::cerr << "Line " << __LINE__                                        //
              << " - Missing parameters !\n"                                //
              << "Usage: " << argv[0] << " /path/to/temp /path/to/TestData" //
              << std::endl;
    return EXIT_FAILURE;
  }

  const char* tempDir = argv[1];
  const char* testDataDir = argv[2];

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
        vtkSmartPointer<vtkMRMLStorageNode> colorStorageNode = vtkSmartPointer<vtkMRMLStorageNode>::Take(colorNode->CreateDefaultStorageNode());

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
        vtkMRMLColorTableNode* colorNode = vtkMRMLColorTableNode::SafeDownCast(scene->GetNodeByID(expectedColorNodeId.c_str()));
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

  // Read color table from file and test content
  {
    std::string colorTestFileName = std::string(testDataDir) + "/ColorTest.csv";
    std::string colorTestWithTerminologyFileName = std::string(testDataDir) + "/ColorTestWithTerminology.csv";
    std::string colorTestMissingAlphaFileName = std::string(testDataDir) + "/ColorTestMissingAlpha.csv";
    std::string colorTestMissingColorFileName = std::string(testDataDir) + "/ColorTestMissingColor.csv";
    std::string colorTestMissingLabelValueFileName = std::string(testDataDir) + "/ColorTestMissingLabelValue.csv";
    std::string colorTestMissingNameFileName = std::string(testDataDir) + "/ColorTestMissingName.csv";
    std::string colorTestInvalidColorFileName = std::string(testDataDir) + "/ColorTestInvalidColor.csv";

    {
      // Read minimal color table file -> success
      vtkNew<vtkMRMLColorTableNode> colorNode;
      vtkNew<vtkMRMLColorTableStorageNode> colorStorageNode;
      colorStorageNode->SetFileName(colorTestFileName.c_str());
      CHECK_INT(colorStorageNode->ReadData(colorNode), 1);
      CHECK_INT(colorStorageNode->GetUserMessages()->GetNumberOfMessages(), 0);
      CHECK_INT(colorNode->GetNumberOfColors(), 11);
      CHECK_BOOL(colorNode->GetContainsTerminology(), false);
    }

    {
      // Read color table file with terminology -> success
      vtkNew<vtkMRMLColorTableNode> colorNode;
      vtkNew<vtkMRMLColorTableStorageNode> colorStorageNode;
      colorStorageNode->SetFileName(colorTestWithTerminologyFileName.c_str());
      CHECK_INT(colorStorageNode->ReadData(colorNode), 1);
      CHECK_INT(colorStorageNode->GetUserMessages()->GetNumberOfMessages(), 0);
      CHECK_INT(colorNode->GetNumberOfColors(), 11);
      CHECK_BOOL(colorNode->GetContainsTerminology(), true);
      CHECK_STD_STRING(colorNode->GetTerminologyAsString(6), "~SCT^49755003^Morphologically Altered Structure~SCT^4147007^Mass~^^~~SCT^64033007^Kidney~SCT^24028007^Right~");
    }

    {
      // Read color table file with missing alpha column -> success (optional column)
      vtkNew<vtkMRMLColorTableNode> colorNode;
      vtkNew<vtkMRMLColorTableStorageNode> colorStorageNode;
      colorStorageNode->SetFileName(colorTestMissingAlphaFileName.c_str());
      CHECK_INT(colorStorageNode->ReadData(colorNode), 1);
      CHECK_INT(colorStorageNode->GetUserMessages()->GetNumberOfMessages(), 0);
      CHECK_INT(colorNode->GetNumberOfColors(), 11);
      double color[4] = { -1.0, -1.0, -1.0, -1.0 };
      colorNode->GetColor(5, color);
      CHECK_DOUBLE(color[3], 1.0);
    }

    {
      // Read color table file with missing name column -> success (optional column)
      vtkNew<vtkMRMLColorTableNode> colorNode;
      vtkNew<vtkMRMLColorTableStorageNode> colorStorageNode;
      colorStorageNode->SetFileName(colorTestMissingNameFileName.c_str());
      CHECK_INT(colorStorageNode->ReadData(colorNode), 1);
      CHECK_INT(colorStorageNode->GetUserMessages()->GetNumberOfMessages(), 0);
      CHECK_INT(colorNode->GetNumberOfColors(), 11);
      std::string name = colorNode->GetColorName(5);
      CHECK_STD_STRING(name, "");
    }

    {
      // Read color table file with missing color column -> warning
      vtkNew<vtkMRMLColorTableNode> colorNode;
      vtkNew<vtkMRMLColorTableStorageNode> colorStorageNode;
      colorStorageNode->SetFileName(colorTestMissingColorFileName.c_str());
      TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
      CHECK_INT(colorStorageNode->ReadData(colorNode), 1);
      TESTING_OUTPUT_ASSERT_WARNINGS_END();
      CHECK_INT(colorStorageNode->GetUserMessages()->GetNumberOfMessages(), 1);
      CHECK_INT(colorNode->GetNumberOfColors(), 11);
    }

    {
      // Read color table file with invalid color value -> error
      vtkNew<vtkMRMLColorTableNode> colorNode;
      vtkNew<vtkMRMLColorTableStorageNode> colorStorageNode;
      colorStorageNode->SetFileName(colorTestInvalidColorFileName.c_str());
      TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
      CHECK_INT(colorStorageNode->ReadData(colorNode), 0);
      TESTING_OUTPUT_ASSERT_ERRORS_END();
    }

    {
      // Read color table file with missing LabelValue column -> error (required column)
      vtkNew<vtkMRMLColorTableNode> colorNode;
      vtkNew<vtkMRMLColorTableStorageNode> colorStorageNode;
      colorStorageNode->SetFileName(colorTestMissingLabelValueFileName.c_str());
      TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
      CHECK_INT(colorStorageNode->ReadData(colorNode), 0);
      TESTING_OUTPUT_ASSERT_ERRORS_END();
      CHECK_INT(colorStorageNode->GetUserMessages()->GetNumberOfMessages(), 2);
      CHECK_INT(colorNode->GetNumberOfColors(), 0);
    }
  }

  return EXIT_SUCCESS;
}
