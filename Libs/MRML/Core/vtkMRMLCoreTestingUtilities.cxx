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

namespace vtkMRMLCoreTestingUtilities
{

//----------------------------------------------------------------------------
template<typename TYPE>
bool Check(int line, const std::string& description,
           TYPE current, TYPE expected,
           const std::string& _testName)
{
  std::string testName = _testName.empty() ? "Check" : _testName;
  if(current != expected)
    {
    std::cerr << "\nLine " << line << " - " << description
              << " : " << testName << " failed"
              << "\n\tcurrent :" << current
              << "\n\texpected:" << expected
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool CheckInt(int line, const std::string& description,
              int current, int expected)
{
  return Check<int>(line, description, current, expected, "CheckInt");
}

//----------------------------------------------------------------------------
bool CheckNotNull(int line, const std::string& description,
                  const void* pointer)
{
  if(!pointer)
    {
    std::cerr << "\nLine " << line << " - " << description
              << " : CheckNotNull failed"
              << "\n\tpointer:" << pointer
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool CheckNull(int line, const std::string& description, const void* pointer)
{
  if(pointer)
    {
    std::cerr << "\nLine " << line << " - " << description
              << " : CheckNull failed"
              << "\n\tpointer:" << pointer
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool CheckPointer(int line, const std::string& description,
                  void* current, void* expected)
{
  return Check<void*>(line, description, current, expected, "CheckPointer");
}

//----------------------------------------------------------------------------
bool CheckString(int line, const std::string& description,
                 const char* current, const char* expected, bool errorIfDifferent /* = true */)
{
  std::string testName = "CheckString";

  bool different = true;
  if (current == 0 || expected == 0)
    {
    different = !(current == 0 && expected == 0);
    }
  else if(strcmp(current, expected) == 0)
    {
    different = false;
    }
  if(different == errorIfDifferent)
    {
    std::cerr << "\nLine " << line << " - " << description
              << " : " << testName << "  failed"
              << "\n\tcurrent :" << (current ? current : "<null>")
              << "\n\texpected:" << (expected ? expected : "<null>")
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool CheckNodeInSceneByID(int line, vtkMRMLScene* scene,
                          const char* nodeID, vtkMRMLNode* expected)
{
  std::string testName = "CheckNodeInSceneByID";

  if (!scene)
    {
    std::cerr << "\nLine " << line << " - scene is NULL"
              << " : " << testName << " failed" << std::endl;
    return false;
    }
  if (!nodeID)
    {
    std::cerr << "\nLine " << line << " - nodeID is NULL"
              << " : " << testName << " failed" << std::endl;
    return false;
    }
  if (nodeID[0] == '\0')
    {
    std::cerr << "\nLine " << line << " - nodeID is an empty string"
              << " : " << testName << " failed" << std::endl;
    return false;
    }
  if (!expected)
    {
    std::cerr << "\nLine " << line << " - expected node is NULL"
              << " : " << testName << " failed" << std::endl;
    return false;
    }
  vtkMRMLNode* current = scene->GetNodeByID(nodeID);
  if (current != expected)
    {
    const char* currentID = (current ? current->GetID() : 0);
    const char* expectedID = (expected ? expected->GetID() : 0);
    std::cerr << "\nLine " << line << " - GetNodeByID(\"" << nodeID << "\")"
              << " : " << testName << " failed"

              << "\n\tcurrent :" << current
              << ", ID: " << (currentID ? currentID : "(null)")

              << "\n\texpected:" << expected
              << ", ID: " << (expectedID ? expectedID : "(null)")
              << std::endl;
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool CheckNodeIdAndName(int line, vtkMRMLNode* node,
                        const char* expectedID, const char* expectedName)
{
  std::string testName = "CheckNodeIdAndName";
  if (!node)
    {
    std::cerr << "\nLine " << line << " - node is NULL"
              << " : " << testName << " failed" << std::endl;
    return false;
    }
  if (!CheckString(
        line, std::string(testName) + ": Unexpected nodeID",
        node->GetID(), expectedID)

      ||!CheckString(
        line, std::string(testName) + ": Unexpected nodeName",
        node->GetName(), expectedName))
    {
    return false;
    }
  return true;
}

}
