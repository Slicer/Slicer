/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MRMLLogic includes
#include "vtkMRMLDisplayableHierarchyLogic.h"

// MRML includes
#include "vtkMRMLDisplayableHierarchyNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLDisplayableHierarchyLogicTest1(int , char * [] )
{
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLDisplayableHierarchyLogic* displayableHierarchyLogic = vtkMRMLDisplayableHierarchyLogic::New();
  displayableHierarchyLogic->SetDebug(true);

  displayableHierarchyLogic->SetMRMLScene(scene.GetPointer());

  // test null pointers
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  char *id = displayableHierarchyLogic->AddDisplayableHierarchyNodeForNode(nullptr);
  if (id != nullptr)
    {
    std::cerr << "AddDisplayableHierarchyNodeForNode did not return null for a null node" << std::endl;
    return EXIT_FAILURE;
    }
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  bool flag =  displayableHierarchyLogic->AddChildToParent(nullptr, nullptr);
  if (flag != false)
    {
    std::cerr << "AddChildToParent did not fail for null nodes" << std::endl;
    return EXIT_FAILURE;
    }
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  // make a couple of nodes
  vtkMRMLModelNode *m1 = vtkMRMLModelNode::New();
  scene->AddNode(m1);
  vtkMRMLModelNode *m2 = vtkMRMLModelNode::New();
  scene->AddNode(m2);

  id = displayableHierarchyLogic->AddDisplayableHierarchyNodeForNode(m1);
  if (id == nullptr)
    {
    std::cerr << "AddDisplayableHierarchyNodeForNode returned null for valid node " << m1->GetID() << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Added a displayable hierarchy with id " << id << " for node " << m1->GetID() << std::endl;
    }

  flag = displayableHierarchyLogic->AddChildToParent(m1, m2);
  if (flag == false)
    {
    std::cerr << "AddChildToParent failed for m1 " << m1->GetID() << ", and m2 " << m2->GetID() << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "AddChildToParent added hierarchies to make m1 " << m1->GetID() << " a child of m2 " << m2->GetID() << std::endl;
    }

  vtkMRMLHierarchyNode* h1 = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(scene.GetPointer(), m1->GetID());
  if (!h1)
    {
    std::cerr << "GetAssociatedHierarchyNode failed for m1 " << m1->GetID() << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Found hierarchy node for m1 with id " << h1->GetID() << std::endl;
    }

  vtkMRMLHierarchyNode* h2 = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(scene.GetPointer(), m2->GetID());
  if (!h2)
    {
    std::cerr << "GetAssociatedHierarchyNode failed for m2 " << m2->GetID() << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Found hierarchy node for m2 with id " << h2->GetID() << std::endl;
    }
  int numChildren = h2->GetNumberOfChildrenNodes();
  if (numChildren != 1)
    {
    std::cerr << "GetNumberOfChildrenNodes failed for h2, expected 1 but got " << numChildren << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "m2's hierarchy node has " << numChildren << " children nodes" << std::endl;
    }

  bool retval = displayableHierarchyLogic->DeleteHierarchyNodeAndChildren(vtkMRMLDisplayableHierarchyNode::SafeDownCast(h2));
  if (!retval)
    {
    std::cerr << "Failed to delete hierarchy node and children!" << std::endl;
    return EXIT_FAILURE;
    }

  m1->Delete();
  m2->Delete();

  displayableHierarchyLogic->Delete();

  return EXIT_SUCCESS;
}

