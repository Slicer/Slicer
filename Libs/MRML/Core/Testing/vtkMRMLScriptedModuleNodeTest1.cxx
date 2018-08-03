/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLScriptedModuleNode.h"

int TestScriptedModuleParameterSaveLoadSpecialCharacters();

int vtkMRMLScriptedModuleNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLScriptedModuleNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  CHECK_EXIT_SUCCESS(TestScriptedModuleParameterSaveLoadSpecialCharacters());
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int TestScriptedModuleParameterSaveLoadSpecialCharacters()
{
  vtkNew<vtkMRMLScene> scene;

  const std::string special1 = "Test that special characters can be stored -- ampersand : (&) quote : (\") apostrophe: (') less: (<) greater: (>)";
  const std::string special2 = "Test that content is not double-decoded -- &amp; &quot; &apos; &lt; &gt; &nbsp;";
  const std::string special3 = "Test attribute percentage encoding -- % %3B %25 %% something";

  vtkNew<vtkMRMLScriptedModuleNode> node;
  scene->AddNode(node.GetPointer());
  node->SetName(special1.c_str());
  node->SetDescription(special2.c_str());
  node->SetParameter("special1Att", special1.c_str());
  node->SetParameter("special2%Att%20", special2.c_str());
  node->SetParameter("speci\"al3 Att", special3.c_str());

  std::stringstream ss;

  // Write scene to XML string
  scene->SetSaveToXMLString(1);
  CHECK_BOOL(scene->Commit() != 0, true);
  std::string sceneXMLString = scene->GetSceneXMLString();

  std::cout << sceneXMLString << std::endl;

  vtkNew<vtkMRMLScene> sceneReloaded;
  sceneReloaded->SetLoadFromXMLString(1);
  sceneReloaded->SetSceneXMLString(sceneXMLString);
  CHECK_BOOL(sceneReloaded->Import() != 0, true);

  vtkMRMLNode* nodeReloaded = sceneReloaded->GetFirstNodeByClass("vtkMRMLScriptedModuleNode");
  CHECK_NOT_NULL(nodeReloaded);

  CHECK_STRING(node->GetName(), nodeReloaded->GetName());
  CHECK_STRING(node->GetDescription(), nodeReloaded->GetDescription());
  CHECK_STRING(node->GetAttribute("special1Att"), nodeReloaded->GetAttribute("special1Att"));
  CHECK_STRING(node->GetAttribute("special2%Att%20"), nodeReloaded->GetAttribute("special2Att"));
  CHECK_STRING(node->GetAttribute("speci\"al3 Att"), nodeReloaded->GetAttribute("special3Att"));

  std::cout << "Test completed successfully." << std::endl;
  return EXIT_SUCCESS;
}
