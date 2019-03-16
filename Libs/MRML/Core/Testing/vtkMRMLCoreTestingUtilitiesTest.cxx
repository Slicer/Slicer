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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 1U24CA194354-01

==============================================================================*/

// MRML includes
#include "vtkMRMLCoreTestingUtilities.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

using namespace vtkMRMLCoreTestingUtilities;

//----------------------------------------------------------------------------
bool TestCheckNodeInSceneByID();
bool TestCheckNodeIdAndName();

//----------------------------------------------------------------------------
int vtkMRMLCoreTestingUtilitiesTest(int , char * [] )
{
  bool res = true;
  res = res && TestCheckNodeInSceneByID();
  res = res && TestCheckNodeIdAndName();
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

namespace
{

//---------------------------------------------------------------------------
class vtkMRMLCoreTestingUtilitiesNode : public vtkMRMLNode
{
public:
  static vtkMRMLCoreTestingUtilitiesNode *New();
  vtkTypeMacro(vtkMRMLCoreTestingUtilitiesNode, vtkMRMLNode);
  vtkMRMLNode* CreateNodeInstance() override
    {
    return vtkMRMLCoreTestingUtilitiesNode::New();
    }
  const char* GetNodeTagName() override
    {
    return "Testing";
    }
private:
  vtkMRMLCoreTestingUtilitiesNode() = default;
};
vtkStandardNewMacro(vtkMRMLCoreTestingUtilitiesNode);

}

//----------------------------------------------------------------------------
bool TestCheckNodeInSceneByID()
{
  if (CheckNodeInSceneByID(
        __LINE__, nullptr, nullptr, nullptr))
    {
    return false;
    }

  vtkNew<vtkMRMLScene> scene;
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLCoreTestingUtilitiesNode>::New());

  if (CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(), nullptr, nullptr))
    {
    return false;
    }

  if (CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLCoreTestingUtilitiesNode", nullptr))
    {
    return false;
    }

  vtkNew<vtkMRMLCoreTestingUtilitiesNode> node1;

  if (CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLCoreTestingUtilitiesNode", node1.GetPointer()))
    {
    return false;
    }

  scene->AddNode(node1.GetPointer());

  if (!CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLCoreTestingUtilitiesNode1", node1.GetPointer()))
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool TestCheckNodeIdAndName()
{
  if (CheckNodeIdAndName(
        __LINE__, nullptr, nullptr, nullptr))
    {
    return false;
    }

  vtkNew<vtkMRMLCoreTestingUtilitiesNode> node1;

  if (!CheckNodeIdAndName(
        __LINE__, node1.GetPointer(), nullptr, nullptr))
    {
    return false;
    }

  if (CheckNodeIdAndName(
        __LINE__, node1.GetPointer(), "vtkMRMLCoreTestingUtilitiesNode1", "Testing"))
    {
    return false;
    }

  vtkNew<vtkMRMLScene> scene;
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLCoreTestingUtilitiesNode>::New());

  scene->AddNode(node1.GetPointer());

  if (!CheckNodeIdAndName(
        __LINE__, node1.GetPointer(), "vtkMRMLCoreTestingUtilitiesNode1", "Testing"))
    {
    return false;
    }

  return true;
}
