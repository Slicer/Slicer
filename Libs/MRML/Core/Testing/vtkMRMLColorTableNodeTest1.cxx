/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLParser.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkObjectFactory.h>

//---------------------------------------------------------------------------
class vtkMRMLColorTableNodeTestHelper1 : public vtkMRMLColorTableNode
{
public:
  // Provide a concrete New.
  static vtkMRMLColorTableNodeTestHelper1 *New();

  vtkTypeMacro(vtkMRMLColorTableNodeTestHelper1,vtkMRMLColorTableNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return vtkMRMLColorTableNodeTestHelper1::New();
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLColorTableNodeTestHelper1";
    }

  virtual int ReadData(vtkMRMLNode *vtkNotUsed(refNode)) { return 0; }
  virtual int WriteData(vtkMRMLNode *vtkNotUsed(refNode)) { return 0; }
};
vtkStandardNewMacro(vtkMRMLColorTableNodeTestHelper1);

//---------------------------------------------------------------------------
int vtkMRMLColorTableNodeTest1(int argc, char * argv[] )
{
  vtkNew<vtkMRMLColorTableNodeTestHelper1> node1;

  EXERCISE_BASIC_OBJECT_METHODS(node1.GetPointer());

  EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS(vtkMRMLColorTableNodeTestHelper1, node1.GetPointer());

  // set up the scene
  const char *sceneFileName = "/tmp/colorTickTest.mrml";
  if (argc > 1)
    {
    sceneFileName = argv[1];
    }

  // check that extra single quotes don't appear in color names via round trip
  // to xml

  vtkNew<vtkMRMLColorTableNode> colorNode;
  colorNode->SetTypeToUser();
  colorNode->SetNumberOfColors(3);
  colorNode->SetColor(0, "zero", 0.0, 0.0, 0.0, 1.0);
  colorNode->SetColor(1, "one", 1.0, 0.0, 0.0, 1.0);
  colorNode->SetColor(2, "two", 0.0, 1.0, 0.0, 1.0);

  std::string colorNodeId;

  {
    // add node to the scene
    vtkNew<vtkMRMLScene> scene;
    if (!scene->AddNode(colorNode.GetPointer()))
      {
      std::cerr << "Problem adding colorNode to the scene !" << std::endl;
      return EXIT_FAILURE;
      }

    // keep track of the id
    colorNodeId = std::string(colorNode->GetID());

    // write MRML file
    scene->SetURL(sceneFileName);
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
    vtkNew<vtkMRMLParser> parser;
    parser->SetMRMLScene(scene.GetPointer());
    parser->SetFileName(sceneFileName);
    int result = parser->Parse();
    if (result != 1)
      {
      std::cerr << "Failed to parse scene file " << sceneFileName << std::endl;
      return EXIT_FAILURE;
      }

    // test the color node
    vtkMRMLColorTableNode *colorNode =
        vtkMRMLColorTableNode::SafeDownCast(scene->GetNodeByID(colorNodeId.c_str()));
    if (!colorNode)
      {
      std::cerr << "Failed to get colorNode [id: " << colorNodeId << "]"
                << " from scene file " << sceneFileName
                << std::endl;
      return EXIT_FAILURE;
      }

    const char *zero = colorNode->GetColorName(0);
    const char *one = colorNode->GetColorName(1);
    const char *two = colorNode->GetColorName(2);
    std::cout << "\nColor names:\n"
                 "\tzero = " << zero << "\n"
                 "\tone = " << one << "\n"
                 "\ttwo = " << two
              << std::endl;
    const char *tickPtr0 = strstr(zero, "'");
    const char *tickPtr1 = strstr(one, "'");
    const char *tickPtr2 = strstr(two, "'");
    if (tickPtr0 != NULL ||
        tickPtr1 != NULL ||
        tickPtr2 != NULL)
      {
      std::cerr << "ERROR: Tick marks are left in color names read from MRML." << std::endl;
      return EXIT_FAILURE;
      }
    }
  return EXIT_SUCCESS;
}
