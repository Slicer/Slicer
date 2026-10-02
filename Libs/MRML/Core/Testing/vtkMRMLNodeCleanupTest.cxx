/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==============================================================================*/

#include "vtkMRMLNodeCleanup.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

#include "vtkMRMLCoreTestingMacros.h"

#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

#include <type_traits>

namespace
{
bool ReturnEarly(vtkMRMLScene* scene)
{
  vtkNew<vtkMRMLModelNode> node;
  scene->AddNode(node);
  vtkMRMLNodeCleanup cleanup(scene, node);
  return false;
}

void ThrowFromScope(vtkMRMLScene* scene)
{
  vtkNew<vtkMRMLModelNode> node;
  scene->AddNode(node);
  vtkMRMLNodeCleanup cleanup(scene, node);
  throw 1;
}
} // namespace

//------------------------------------------------------------------------------
int vtkMRMLNodeCleanupTest(int, char*[])
{
  static_assert(!std::is_copy_constructible<vtkMRMLNodeCleanup>::value, "Cleanup guard must not be copied");
  static_assert(!std::is_copy_assignable<vtkMRMLNodeCleanup>::value, "Cleanup guard must not be assigned");

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLModelNode> node;
  scene->AddNode(node);
  {
    vtkMRMLNodeCleanup cleanup(scene, node);
    CHECK_BOOL(scene->IsNodePresent(node), true);
  }
  CHECK_BOOL(scene->IsNodePresent(node), false);
  CHECK_NULL(node->GetScene());

  CHECK_BOOL(ReturnEarly(scene), false);
  CHECK_INT(scene->GetNumberOfNodes(), 0);

  try
  {
    ThrowFromScope(scene);
  }
  catch (int)
  {
  }
  CHECK_INT(scene->GetNumberOfNodes(), 0);

  scene->AddNode(node);
  {
    vtkMRMLNodeCleanup cleanup(scene, node);
    scene->RemoveNode(node);
  }
  CHECK_INT(scene->GetNumberOfNodes(), 0);

  scene->AddNode(node);
  {
    vtkMRMLNodeCleanup noScene(nullptr, node);
    vtkMRMLNodeCleanup noNode(scene, nullptr);
  }
  CHECK_BOOL(scene->IsNodePresent(node), true);
  scene->RemoveNode(node);

  vtkNew<vtkMRMLScene> otherScene;
  scene->AddNode(node);
  {
    vtkMRMLNodeCleanup cleanup(scene, node);
    scene->RemoveNode(node);
    otherScene->AddNode(node);
  }
  CHECK_BOOL(otherScene->IsNodePresent(node), true);
  otherScene->RemoveNode(node);

  vtkWeakPointer<vtkMRMLScene> weakScene;
  vtkWeakPointer<vtkMRMLModelNode> weakNode;
  {
    vtkSmartPointer<vtkMRMLScene> ownedScene = vtkSmartPointer<vtkMRMLScene>::New();
    vtkSmartPointer<vtkMRMLModelNode> ownedNode = vtkSmartPointer<vtkMRMLModelNode>::New();
    ownedScene->AddNode(ownedNode);
    weakScene = ownedScene;
    weakNode = ownedNode;
    vtkMRMLNodeCleanup cleanup(ownedScene, ownedNode);
    ownedScene = nullptr;
    ownedNode = nullptr;
    CHECK_NOT_NULL(weakScene.GetPointer());
    CHECK_NOT_NULL(weakNode.GetPointer());
  }
  CHECK_NULL(weakScene.GetPointer());
  CHECK_NULL(weakNode.GetPointer());

  return EXIT_SUCCESS;
}
