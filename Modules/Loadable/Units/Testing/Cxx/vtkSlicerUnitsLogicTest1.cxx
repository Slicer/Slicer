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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Volumes logic
#include "vtkSlicerUnitsLogic.h"

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLUnitNode.h>

// VTK includes
#include <vtkNew.h>

// STD includes
#include <algorithm>

namespace
{
template <class T>void printNodes(const std::vector<T>& nodes);

// This function will check that a given list of nodes is valid or not.
// Note: nodes obtained using vtkMRMLSelectionNode::GetUnitNodes() are expected
// to be alphabetically sorted
template <class T>bool areValidNodes(const std::vector<T>& nodes, bool sorted, const char* testName = nullptr);

bool testScene();
bool testSelectionNode();
bool testCloseScene();
bool testSaveAndReloadScene();
bool testImportScene(const char* sceneFilePath);
bool testGetSIPrefixCoefficient();
}

//-----------------------------------------------------------------------------
int vtkSlicerUnitsLogicTest1( int argc , char * argv[] )
{
  bool res = true;
  res = res && testScene();
  res = res && testSelectionNode();
  res = res && testCloseScene();
  res = res && testSaveAndReloadScene();
  res = res && testGetSIPrefixCoefficient();
  if (argc > 1)
    {
    res = res && testImportScene(argv[1]);
    }
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

namespace
{

//-----------------------------------------------------------------------------
template <class T> void printNodes(const std::vector<T>& nodes)
{
  for (size_t i = 0; i < nodes.size(); ++i)
    {
    vtkMRMLUnitNode* node = vtkMRMLUnitNode::SafeDownCast(nodes[i]);
    if (!node)
      {
      std::cout << i << ": NULL" << std::endl;
      }
    else
      {
      std::cout << i << ": " << node->GetID() << " "  << node << std::endl;
      }
    }
}

//-----------------------------------------------------------------------------
template <class T> bool areValidNodes(const std::vector<T>& nodes,
                                      bool sorted,
                                      const char* testName)
{
  std::vector<std::string> unitNodeIDs;
  unitNodeIDs.emplace_back("vtkMRMLUnitNodeApplicationLength");
  unitNodeIDs.emplace_back("vtkMRMLUnitNodeApplicationTime");
  unitNodeIDs.emplace_back("vtkMRMLUnitNodeApplicationFrequency");
  unitNodeIDs.emplace_back("vtkMRMLUnitNodeApplicationVelocity");
  unitNodeIDs.emplace_back("vtkMRMLUnitNodeApplicationIntensity");
  const size_t numberOfUnits = unitNodeIDs.size();

  if (nodes.size() != numberOfUnits)
    {
    std::cerr << (testName ? testName : "") << ":" << std::endl
              << "Not the right number of unit node. "
              << "Expected " << numberOfUnits << " nodes, "
              << " got instead " << nodes.size() << " nodes."
              << std::endl;
    printNodes(nodes);
    return false;
    }

  if (sorted)
    {
    std::sort(unitNodeIDs.begin(), unitNodeIDs.end());
    }

  for (size_t i = 0; i < numberOfUnits; ++i)
    {
    vtkMRMLUnitNode* node = vtkMRMLUnitNode::SafeDownCast(nodes[i]);
    if (!node || strcmp(node->GetID(), unitNodeIDs[i].c_str()) != 0)
      {
      std::cerr << (testName ? testName : "") << ":" << std::endl
                << "Expecting node " << unitNodeIDs[i]<<" Got: "
                << (node ? node->GetID() : "NONE") << std::endl;
      return false;
      }

    if (node->GetSaveWithScene())
      {
      std::cerr << (testName ? testName : "") << ":" << std::endl
                << "Node " << node->GetID()
                <<" should not be saved with the scene !" << std::endl;
      return false;
      }
    }
  return true;
}

//-----------------------------------------------------------------------------
bool testScene()
{
  std::cout << "test scene..." << std::endl;
  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkSlicerUnitsLogic> logic;
  logic->SetMRMLScene(scene.GetPointer());

  // Test scene
  std::vector<vtkMRMLNode*> nodes;
  scene->GetNodesByClass("vtkMRMLUnitNode", nodes);

  return areValidNodes(nodes, /*sorted =*/ false, "testScene");
}

//-----------------------------------------------------------------------------
bool testSelectionNode()
{
  std::cout << "test selection node..." << std::endl;

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLSelectionNode> selectionNode;
  scene->AddNode(selectionNode.GetPointer());

  vtkNew<vtkSlicerUnitsLogic> logic;
  logic->SetMRMLScene(scene.GetPointer());

  // Test scene
  std::vector<vtkMRMLUnitNode*> nodes;
  selectionNode->GetUnitNodes(nodes);

  return areValidNodes(nodes, /*sorted =*/ true, "testSelectionNode");
}

//-----------------------------------------------------------------------------
bool testCloseScene()
{
  std::cout << "test close scene..." << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLSelectionNode> selectionNode;
  scene->AddNode(selectionNode.GetPointer());

  vtkNew<vtkSlicerUnitsLogic> logic;
  logic->SetMRMLScene(scene.GetPointer());

  // logic should ensure units stay in the selection node even after a Clear().
  scene->Clear(0);

  std::vector<vtkMRMLUnitNode*> nodes;
  selectionNode->GetUnitNodes(nodes);

  return areValidNodes(nodes, /*sorted =*/ true, "testCloseScene");
}

//-----------------------------------------------------------------------------
bool testSaveAndReloadScene()
{
  std::cout << "test save&reload scene..." << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLSelectionNode> selectionNode;
  scene->AddNode(selectionNode.GetPointer());

  vtkNew<vtkSlicerUnitsLogic> logic;
  logic->SetMRMLScene(scene.GetPointer());

  std::cout << "  ...commit" << std::endl;
  scene->SetSaveToXMLString(1);
  scene->Commit();
  std::string xmlScene = scene->GetSceneXMLString();

  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(xmlScene);
  scene->Import();

  // Test scene
  std::vector<vtkMRMLNode*> nodes;
  scene->GetNodesByClass("vtkMRMLUnitNode", nodes);
  if (!areValidNodes(nodes, /*sorted =*/ false, "testSaveAndReloadScene-sceneAfterImport"))
    {
    return false;
    }
  nodes.clear();

  std::vector<vtkMRMLUnitNode*> unitNodes;
  selectionNode->GetUnitNodes(unitNodes);

  if (!areValidNodes(unitNodes, /*sorted =*/ true, "testSaveAndReloadScene-selectionNodeAfterImport"))
    {
    return false;
    }
  unitNodes.clear();

  // logic should ensure units stay in the selection node even after a Clear().
  std::cout << "  ...clear" << std::endl;
  scene->Clear(0);

  scene->GetNodesByClass("vtkMRMLUnitNode", nodes);
  if (!areValidNodes(nodes, /*sorted =*/ false, "testSaveAndReloadScene-sceneAfterClear"))
    {
    return false;
    }
  nodes.clear();

  selectionNode->GetUnitNodes(unitNodes);
  return areValidNodes(unitNodes, /*sorted =*/ true, "testSaveAndReloadScene->selectionNodeAfterClear");
}

//-----------------------------------------------------------------------------
bool testImportScene(const char* sceneFilePath)
{
  std::cout << "test import scene..." << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLSelectionNode> selectionNode;
  scene->AddNode(selectionNode.GetPointer());

  vtkNew<vtkSlicerUnitsLogic> logic;
  logic->SetMRMLScene(scene.GetPointer());

  std::cout << "  ...import" << std::endl;
  scene->SetURL(sceneFilePath);
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  scene->Import();
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  // Test scene
  std::vector<vtkMRMLNode*> nodes;
  scene->GetNodesByClass("vtkMRMLUnitNode", nodes);
  if (!areValidNodes(nodes, /*sorted =*/ false, "testImportScene-sceneAfterImport"))
    {
    return false;
    }
  nodes.clear();

  std::vector<vtkMRMLUnitNode*> unitNodes;
  selectionNode->GetUnitNodes(unitNodes);

  if (!areValidNodes(unitNodes, /*sorted =*/ true, "testImportScene-selectionNodeAfterImport"))
    {
    return false;
    }
  unitNodes.clear();

  // logic should ensure units stay in the selection node even after a Clear().
  std::cout << "  ...clear" << std::endl;
  scene->Clear(0);

  scene->GetNodesByClass("vtkMRMLUnitNode", nodes);
  if (!areValidNodes(nodes, /*sorted =*/ false, "testImportScene-sceneAfterClear"))
    {
    return false;
    }
  nodes.clear();

  selectionNode->GetUnitNodes(unitNodes);
  return areValidNodes(unitNodes, /*sorted =*/ true, "testImportScene->selectionNodeAfterClear");
}

//-----------------------------------------------------------------------------
bool testGetSIPrefixCoefficient()
{
  std::map<std::string, double> coefficients;
  coefficients["yotta"] = 1000000000000000000000000.;
  coefficients["zetta"] = 1000000000000000000000.;
  coefficients["exa"] = 1000000000000000000.;
  coefficients["peta"] = 1000000000000000.;
  coefficients["tera"] = 1000000000000.;
  coefficients["giga"] = 1000000000.;
  coefficients["mega"] = 1000000.;
  coefficients["kilo"] = 1000.;
  coefficients["hecto"] = 100.;
  coefficients["deca"] = 10.;
  coefficients[""] = 1.;
  coefficients["deci"] = 0.1;
  coefficients["centi"] = 0.01;
  coefficients["milli"] = 0.001;
  coefficients["micro"] = 0.000001;
  coefficients["nano"] = 0.000000001;
  coefficients["pico"] = 0.000000000001;
  coefficients["femto"] = 0.000000000000001;
  coefficients["atto"] = 0.000000000000000001;
  coefficients["zepto"] = 0.000000000000000000001;
  coefficients["yocto"] = 0.000000000000000000000001;

  for (std::map<std::string, double>::iterator it = coefficients.begin();
       it != coefficients.end(); ++it)
    {
    std::string prefix = it->first;
    double expectedCoefficient = it->second;
    double coefficient = vtkSlicerUnitsLogic::GetSIPrefixCoefficient(prefix.c_str());
    if (coefficient != expectedCoefficient)
      {
      std::cerr << "Line " << __LINE__
                << " - Problem with GetSIPrefixCoefficient(\"" << prefix << "\")\n"
                << "  coefficient: " << coefficient << "\n"
                << "  expectedCoefficient: " << expectedCoefficient
                << std::endl;
      return false;
      }
    }
  return true;
}

} // end namespace
