/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLColorTableNode.h"

#include "vtkMRMLParser.h"
#include "vtkMRMLScene.h"

#include "vtkMRMLCoreTestingMacros.h"

class vtkMRMLColorTableNodeTestHelper1 : public vtkMRMLColorTableNode
{
public:
  // Provide a concrete New.
  static vtkMRMLColorTableNodeTestHelper1 *New(){return new vtkMRMLColorTableNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLColorTableNodeTestHelper1,vtkMRMLColorTableNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLColorTableNodeTestHelper1;
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLColorTableNodeTestHelper1";
    }

  virtual int ReadData(vtkMRMLNode *vtkNotUsed(refNode)) { return 0; }
  virtual int WriteData(vtkMRMLNode *vtkNotUsed(refNode)) { return 0; }
};
 
int vtkMRMLColorTableNodeTest1(int argc, char * argv[] )
{
  vtkSmartPointer< vtkMRMLColorTableNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLColorTableNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_TRANSFORMABLE_MRML_METHODS(vtkMRMLColorTableNodeTestHelper1, node1);

  // check that extra single quotes don't appear in color names via round trip
  // to xml
  std::cout << "\nTesting for tick marks around color names saved to mrml and then loaded (argc = " << argc << ")" << std::endl;
  vtkSmartPointer< vtkMRMLColorTableNode > colorNode = vtkSmartPointer< vtkMRMLColorTableNode >::New();
  colorNode->SetTypeToUser();
  colorNode->SetNumberOfColors(3);
  colorNode->SetColor(0, "zero", 0.0, 0.0, 0.0, 1.0);
  colorNode->SetColor(1, "one", 1.0, 0.0, 0.0, 1.0);
  colorNode->SetColor(2, "two", 0.0, 1.0, 0.0, 1.0);
  // set up the scene
  vtkMRMLScene *scene = vtkMRMLScene::New();
  scene->AddNode(colorNode);
  const char *id = colorNode->GetID();
  std::cout << "Added color node to scene, id = " << id << std::endl;
  // write it out
  const char *filename = "/tmp/colorTickTest.mrml";
  if (argc > 1)
    {
    filename = argv[1];
    }
  std::cout << "Using mrml file name of " << filename << std::endl;
  scene->SetURL(filename);
  scene->Commit();
  scene->Delete();
  // read and parse
  vtkMRMLParser* parser = vtkMRMLParser::New();
  vtkMRMLScene *loadedScene = vtkMRMLScene::New();
  parser->SetMRMLScene(loadedScene);
  parser->SetFileName(filename);
  int result = parser->Parse();
  parser->Delete();
  if (result != 1)
    {
    std::cerr << "Failed to parse file " << filename << std::endl;
    loadedScene->Delete();
    return EXIT_FAILURE;
    }
  // test the color node
  vtkMRMLNode *mrmlNode = loadedScene->GetNodeByID(id);
  if (mrmlNode)
    {
    vtkMRMLColorTableNode *readNode = vtkMRMLColorTableNode::SafeDownCast(mrmlNode);
    if (readNode)
      {
      const char *zero = readNode->GetColorName(0);
      const char *one = readNode->GetColorName(1);
      const char *two = readNode->GetColorName(2);
      std::cout << "\nColor names:\n\tzero = " << zero << "\n\tone = " << one << "\n\ttwo = " << two << std::endl;
      const char *tickPtr0 = strstr(zero, "'");
      const char *tickPtr1 = strstr(one, "'");
      const char *tickPtr2 = strstr(two, "'");
      if (tickPtr0 != NULL ||
          tickPtr1 != NULL ||
          tickPtr2 != NULL)
        {
        std::cerr << "ERROR: Tick marks are left in color names read from MRML." << std::endl;
        loadedScene->Delete();
        return EXIT_FAILURE;
        }
      }
    }
  // clean up
  loadedScene->Delete();
  
  return EXIT_SUCCESS;
}
