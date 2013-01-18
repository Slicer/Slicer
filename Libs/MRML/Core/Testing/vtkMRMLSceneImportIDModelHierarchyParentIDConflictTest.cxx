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

==============================================================================*/

// MRML includes
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLScene.h"

// STD includes
#include <sstream>

// VTK includes
#include <vtkNew.h>

// ITKSYS includes
#include <itksys/SystemTools.hxx>

bool ImportIDModelHierarchyParentIDConflictTestXMLString();
bool ImportIDModelHierarchyParentIDConflictTestFile();

//---------------------------------------------------------------------------
int vtkMRMLSceneImportIDModelHierarchyParentIDConflictTest(int vtkNotUsed(argc), char * vtkNotUsed(argv) [])
{
  bool res = true;
  res = ImportIDModelHierarchyParentIDConflictTestXMLString() && res;
  res = ImportIDModelHierarchyParentIDConflictTestFile() && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//---------------------------------------------------------------------------
// add a 5 deep model hierarchy to a scene
void PopulateScene(vtkMRMLScene *scene)
{
  // add model hierarchy nodes
  for (int i = 0; i < 5; i++)
    {
    vtkSmartPointer<vtkMRMLModelHierarchyNode> mhn = vtkSmartPointer<vtkMRMLModelHierarchyNode>::New();
    scene->AddNode(mhn);
    std::string idNumberString;
    std::stringstream ss;
    ss << i;
    ss >> idNumberString;
    mhn->SetName(idNumberString.c_str());
    if (i > 0)
      {
      std::string parentNodeID = std::string("vtkMRMLModelHierarchyNode") + idNumberString;
      std::cout << "Setting parent node id on node " << mhn->GetID() << " to " << parentNodeID.c_str() << std::endl;
      mhn->SetParentNodeID(parentNodeID.c_str());
      }
    }
}

//---------------------------------------------------------------------------
// Print out the model hierarchy nodes in a scene
void PrintModelHierarchyNodes(vtkMRMLScene *scene)
{
  int numNodes = scene->GetNumberOfNodesByClass("vtkMRMLModelHierarchyNode");

  for (int i = 0; i < numNodes; i++)
    {
    vtkMRMLNode *mrmlNode = scene->GetNthNodeByClass(i, "vtkMRMLModelHierarchyNode");
    if (mrmlNode && mrmlNode->IsA("vtkMRMLModelHierarchyNode"))
      {
      vtkMRMLModelHierarchyNode *hnode = vtkMRMLModelHierarchyNode::SafeDownCast(mrmlNode);
      std::cout << i << ": Model Hierarchy node named " << hnode->GetName() << " with id " << hnode->GetID() << " has parent node id of " << (hnode->GetParentNodeID() ? hnode->GetParentNodeID() : "null") << std::endl;
      }
    }

}

//---------------------------------------------------------------------------
// The test makes sure the model hierarchy nodes correctly support node ID
// conflict in the parent node id. There are 2 steps in this test
// a) populates a scene with a 5 deep model hierarchy
// b) and imports a similar scene into the existing scene.
bool ImportIDModelHierarchyParentIDConflictTestXMLString()
{
  vtkNew<vtkMRMLScene> scene;

  PopulateScene(scene.GetPointer());

  std::cout << "XML: Starting scene has " << scene->GetNumberOfNodes() << " nodes" << std::endl;
  /// at this point the following node id should be in the scene
  /// vtkMRMLModelHierarchyNode1 with parent id null
  /// vtkMRMLModelHierarchyNode2 with parent id vtkMRMLModelHierarchyNode1
  /// vtkMRMLModelHierarchyNode3 with parent id vtkMRMLModelHierarchyNode2
  /// vtkMRMLModelHierarchyNode4 with parent id vtkMRMLModelHierarchyNode3
  /// vtkMRMLModelHierarchyNode5 with parent id vtkMRMLModelHierarchyNode4
 

  const char scene1XML[] =
    "<MRML >"
    "  <ModelHierarchy id=\"vtkMRMLModelHierarchyNode1\" name=\"vtkMRMLModelHierarchyNode1\"></ModelHierarchy>"
    "  <ModelHierarchy id=\"vtkMRMLModelHierarchyNode2\" name=\"vtkMRMLModelHierarchyNode2\" parentNodeRef=\"vtkMRMLModelHierarchyNode1\"></ModelHierarchy>"
    "  <ModelHierarchy id=\"vtkMRMLModelHierarchyNode3\" name=\"vtkMRMLModelHierarchyNode2\" parentNodeRef=\"vtkMRMLModelHierarchyNode2\"></ModelHierarchy>"
    "  <ModelHierarchy id=\"vtkMRMLModelHierarchyNode4\" name=\"vtkMRMLModelHierarchyNode2\" parentNodeRef=\"vtkMRMLModelHierarchyNode3\"></ModelHierarchy>"
    "  <ModelHierarchy id=\"vtkMRMLModelHierarchyNode5\" name=\"vtkMRMLModelHierarchyNode2\" parentNodeRef=\"vtkMRMLModelHierarchyNode4\"></ModelHierarchy>"
    "</MRML>"
    ;

  scene->SetSceneXMLString(scene1XML);
  scene->SetLoadFromXMLString(1);
  // When importing the scene, there is conflict between the existing nodes
  // and added nodes. New IDs are set by Import to the added nodes and the
  // parent node refs should be updated
  // The node ids in the scene after a proper import should be
  /// vtkMRMLModelHierarchyNode1 with parent node id NULL
  /// vtkMRMLModelHierarchyNode2 with parent id vtkMRMLModelHierarchyNode1
  /// vtkMRMLModelHierarchyNode3 with parent id vtkMRMLModelHierarchyNode2
  /// vtkMRMLModelHierarchyNode4 with parent id vtkMRMLModelHierarchyNode3
  /// vtkMRMLModelHierarchyNode5 with parent id vtkMRMLModelHierarchyNode4
  /// vtkMRMLModelHierarchyNode6 with parent node NULL
  /// vtkMRMLModelHierarchyNode7 with parent id vtkMRMLModelHierarchyNode6
  /// vtkMRMLModelHierarchyNode8 with parent id vtkMRMLModelHierarchyNode7
  /// vtkMRMLModelHierarchyNode9 with parent id vtkMRMLModelHierarchyNode8
  /// vtkMRMLModelHierarchyNode10 with parent id vtkMRMLModelHierarchyNode9
  
  scene->Import();
  int numNodes = scene->GetNumberOfNodesByClass("vtkMRMLModelHierarchyNode");
  if (numNodes != 10)
    {
    std::cerr << "Failed to import scene at line" << __LINE__
              << ", number of nodes: " << numNodes
              << ", expected 8"
              << std::endl;
    return false;
    }

  // print out
  PrintModelHierarchyNodes(scene.GetPointer());

  // check that the hierarchies point to the right parent nodes
  vtkMRMLModelHierarchyNode *hierarchyNode7 = vtkMRMLModelHierarchyNode::SafeDownCast(scene->GetNodeByID("vtkMRMLModelHierarchyNode7"));
  if (!hierarchyNode7)
    {
    std::cerr << "Import failure: imported hierarchy is not using id vtkMRMLModelHierarchyNode7" << std::endl;
    return false;
    }
  if (!hierarchyNode7->GetParentNodeID())
    {
    std::cerr << "Import failure: imported hierarchy node 7 does not have a parent node" << std::endl;
    return false;
    }
  if (strcmp(hierarchyNode7->GetParentNodeID(), "vtkMRMLModelHierarchyNode6") != 0)
    {
    std::cerr << "Import failure: new model hierarchy node 7 should point to new parent hierarchynode with id vtkMRMLModelHierarchyNode6, instead points to"
              << hierarchyNode7->GetParentNodeID() << std::endl;
    return false;
    }

  vtkMRMLModelHierarchyNode *hierarchyNode8 = vtkMRMLModelHierarchyNode::SafeDownCast(scene->GetNodeByID("vtkMRMLModelHierarchyNode8"));
  if (!hierarchyNode8)
    {
    std::cerr << "Import failure: imported hierarchy is not using id vtkMRMLModelHierarchyNode8" << std::endl;
    return false;
    }
  if (!hierarchyNode8->GetParentNodeID())
    {
    std::cerr << "Import failure: imported hierarchy node 8 does not have a parent node" << std::endl;
    return false;
    }
  if (strcmp(hierarchyNode8->GetParentNodeID(), "vtkMRMLModelHierarchyNode7") != 0)
    {
    std::cerr << "Import failure: new model hierarchy node 8 should point to new parent hierarchynode with id vtkMRMLModelHierarchyNode7, instead points to"
              << hierarchyNode8->GetParentNodeID() << std::endl;
    return false;
    }

  return true;
}

//---------------------------------------------------------------------------
// The test makes sure the model hierarchy nodes correctly support node ID
// conflict in the parent node id. 
// - populate a scene with one hierarchy node, save to disk
// - populates a scene with a 5 deep model hierarchy, save to disk
// - create new scene
// - import file with one hierarchy node
// - import file with 5 hierarchy nodes
bool ImportIDModelHierarchyParentIDConflictTestFile()
{
  vtkNew<vtkMRMLScene> scene1;

  // add a single hierarchy node
  vtkSmartPointer<vtkMRMLModelHierarchyNode> mhn = vtkSmartPointer<vtkMRMLModelHierarchyNode>::New();
  scene1->AddNode(mhn);

  std::cout << "\nFile: Starting scene 1 has " << scene1->GetNumberOfNodes() << " nodes" << std::endl;

  std::string filename1 = "ImportIDModelHierarchyParentIDConflictTestFile1.mrml";
  scene1->SetURL(filename1.c_str());
  scene1->Commit();

  // make a second scene file on disk with 5 nodes, the first one conflicting with the one node in the first scene
  vtkNew<vtkMRMLScene> scene2;

  PopulateScene(scene2.GetPointer());

  std::cout << "\nFile: Starting scene2 has " << scene2->GetNumberOfNodes() << " nodes" << std::endl;

  std::string filename2 = "ImportIDModelHierarchyParentIDConflictTestFile2.mrml";
  scene2->SetURL(filename2.c_str());
  scene2->Commit();

  // now read into a new scene
  vtkNew<vtkMRMLScene> scene3;

  scene3->SetURL(filename1.c_str());
  scene3->Import();
  std::cout << "After first import, new scene has " << scene3->GetNumberOfNodes() << " nodes" << std::endl;
  PrintModelHierarchyNodes(scene3.GetPointer());

  // now import file 2
  scene3->SetURL(filename2.c_str());
  scene3->Import();
  std::cout << "After second import, scene has " << scene3->GetNumberOfNodes() << " nodes" << std::endl;
  PrintModelHierarchyNodes(scene3.GetPointer());

  // now expect the scene to have
  // vtkMRMLModelHierarchyNode1 has parent node id of null
  // vtkMRMLModelHierarchyNode6 has parent node id of null
  // vtkMRMLModelHierarchyNode2 has parent node id of vtkMRMLModelHierarchyNode6
  // vtkMRMLModelHierarchyNode3 has parent node id of vtkMRMLModelHierarchyNode2
  // vtkMRMLModelHierarchyNode4 has parent node id of vtkMRMLModelHierarchyNode3
  // vtkMRMLModelHierarchyNode5 has parent node id of vtkMRMLModelHierarchyNode4
  vtkMRMLModelHierarchyNode *hierarchyNode2 = vtkMRMLModelHierarchyNode::SafeDownCast(scene3->GetNodeByID("vtkMRMLModelHierarchyNode2"));
  if (!hierarchyNode2)
    {
    std::cerr << "Import failure: imported hierarchy is not using id vtkMRMLModelHierarchyNode2" << std::endl;
    return false;
    }
  if (!hierarchyNode2->GetParentNodeID())
    {
    std::cerr << "Import failure: imported hierarchy node 2 does not have a parent node" << std::endl;
    return false;
    }
  if (strcmp(hierarchyNode2->GetParentNodeID(), "vtkMRMLModelHierarchyNode6") != 0)
    {
    std::cerr << "Import failure: new model hierarchy node 2 should point to new parent hierarchynode with id vtkMRMLModelHierarchyNode6, instead points to"
              << hierarchyNode2->GetParentNodeID() << std::endl;
    return false;
    }

  // clean up
  int removed1 = itksys::SystemTools::RemoveFile(filename1.c_str());
  if (!removed1)
    {
    std::cerr << "Unable to remove file " << filename1.c_str() << std::endl;
    return false;
    }
  int removed2 = itksys::SystemTools::RemoveFile(filename2.c_str());
  if (!removed2)
    {
    std::cerr << "Unable to remove file " << filename2.c_str() << std::endl;
    return false;
    }

  return true;
}
