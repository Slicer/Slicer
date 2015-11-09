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
bool TestCheckInt();
bool TestCheckNotNull();
bool TestCheckNull();
bool TestCheckPointer();
bool TestCheckString();
bool TestCheckNodeInSceneByID();
bool TestCheckNodeIdAndName();

//----------------------------------------------------------------------------
int vtkMRMLCoreTestingUtilitiesTest(int , char * [] )
{
  bool res = true;
  res = res && TestCheckInt();
  res = res && TestCheckNotNull();
  res = res && TestCheckNull();
  res = res && TestCheckPointer();
  res = res && TestCheckString();
  res = res && TestCheckNodeInSceneByID();
  res = res && TestCheckNodeIdAndName();
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//----------------------------------------------------------------------------
bool TestCheckInt()
{
  if (!CheckInt(__LINE__, "TestCheckInt", 1, 1)
      || CheckInt(__LINE__, "TestCheckInt Expected Failure", 1, -1))
    {
    std::cerr << "Line " << __LINE__ << " - TestCheckInt failed" << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool TestCheckNotNull()
{
  int foo = 1;
  if (!CheckNotNull(__LINE__, "TestCheckNotNull", &foo)
      || CheckNotNull(__LINE__, "TestCheckNotNull Expected Failure", 0))
    {
    std::cerr << "Line " << __LINE__ << " - TestCheckNotNull failed" << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool TestCheckNull()
{
  int foo = 1;
  if (!CheckNull(__LINE__, "TestCheckNull", 0)
      || CheckNull(__LINE__, "TestCheckNull Expected Failure", &foo))
    {
    std::cerr << "Line " << __LINE__ << " - TestCheckNull failed" << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool TestCheckPointer()
{
  int foo = 1;
  int bar = 1;
  if (!CheckPointer(__LINE__, "TestCheckPointer", &foo, &foo)
      || CheckPointer(__LINE__, "TestCheckPointer Expected Failure", &foo, &bar))
    {
    std::cerr << "Line " << __LINE__ << " - TestCheckPointer failed" << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool TestCheckString()
{
  const char* foo = "foo";
  const char* bar = "bar";
  if (!CheckString(__LINE__, "TestCheckString", 0, 0)
      ||!CheckString(__LINE__, "TestCheckString", foo, foo)
      || CheckString(__LINE__, "TestCheckString Expected Failure", foo, bar)
      || CheckString(__LINE__, "TestCheckString Expected Failure", foo, 0))
    {
    std::cerr << "Line " << __LINE__ << " - TestCheckString failed" << std::endl;
    return false;
    }
  return true;
}

namespace
{

//---------------------------------------------------------------------------
class vtkMRMLCoreTestingUtilitiesNode : public vtkMRMLNode
{
public:
  static vtkMRMLCoreTestingUtilitiesNode *New();
  vtkTypeMacro(vtkMRMLCoreTestingUtilitiesNode, vtkMRMLNode);
  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return vtkMRMLCoreTestingUtilitiesNode::New();
    }
  virtual const char* GetNodeTagName()
    {
    return "Testing";
    }
private:
  vtkMRMLCoreTestingUtilitiesNode(){}
};
vtkStandardNewMacro(vtkMRMLCoreTestingUtilitiesNode);

}

//----------------------------------------------------------------------------
bool TestCheckNodeInSceneByID()
{
  if (CheckNodeInSceneByID(
        __LINE__, 0, 0, 0))
    {
    return false;
    }

  vtkNew<vtkMRMLScene> scene;
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLCoreTestingUtilitiesNode>::New());

  if (CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(), 0, 0))
    {
    return false;
    }

  if (CheckNodeInSceneByID(
        __LINE__, scene.GetPointer(),
        "vtkMRMLCoreTestingUtilitiesNode", 0))
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
        __LINE__, 0, 0, 0))
    {
    return false;
    }

  vtkNew<vtkMRMLCoreTestingUtilitiesNode> node1;

  if (!CheckNodeIdAndName(
        __LINE__, node1.GetPointer(), 0, 0))
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
