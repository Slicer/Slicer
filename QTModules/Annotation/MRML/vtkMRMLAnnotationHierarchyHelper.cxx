#include "vtkMRMLAnnotationHierarchyHelper.h"

// Annotation MRML includes
#include "vtkMRMLAnnotationHierarchyNode.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>


// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationHierarchyHelper);
vtkCxxRevisionMacro (vtkMRMLAnnotationHierarchyHelper, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
void vtkMRMLAnnotationHierarchyHelper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyHelper::vtkMRMLAnnotationHierarchyHelper()
{
  this->m_scene = 0;
  this->m_activeHierarchyNode = 0;
}

//---------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyHelper::~vtkMRMLAnnotationHierarchyHelper()
{
  this->m_scene = 0;
  this->m_activeHierarchyNode = 0;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationHierarchyHelper::SetMRMLScene(vtkMRMLScene* scene)
{
  if (!scene)
    {
    vtkErrorMacro("SetMRMLScene: Scene is NULL!")
    return;
    }
  this->m_scene = scene;
}


//---------------------------------------------------------------------------
//
/// Set the pointer to the active (eq. currently selected) hierarchy node.
/// This gets used to set the parent for new hierarchy nodes.
void vtkMRMLAnnotationHierarchyHelper::SetActiveHierarchyNode(vtkMRMLAnnotationHierarchyNode* hierarchyNode)
{
  if (!hierarchyNode)
    {
    vtkErrorMacro("SetActiveHierarchyNode: Could not get hierarchyNode.")
    return;
    }

  this->m_activeHierarchyNode = hierarchyNode;
}

//---------------------------------------------------------------------------
//
/// Add a new annotation hierarchy node under the active hierarchy node. If there is no
/// active hierarchy node, use the top-level annotation hierarchy node as the parent.
/// If there is no top-level annotation hierarchy node, create additionally a top-level hierarchy node which serves as
/// a parent to the new hierarchy node. Return the new hierarchy node.
vtkMRMLAnnotationHierarchyNode* vtkMRMLAnnotationHierarchyHelper::AddNewHierarchyNode()
{

  if (!this->m_activeHierarchyNode)
    {
    // no active hierarchy node, this means we create the new node directly under the top-level hierarchy node
    vtkMRMLAnnotationHierarchyNode* toplevelHierarchyNode = this->GetTopLevelHierarchyNode();

    this->m_activeHierarchyNode = toplevelHierarchyNode;

    }

  // Create a hierarchy node
  vtkMRMLAnnotationHierarchyNode* hierarchyNode = vtkMRMLAnnotationHierarchyNode::New();
  hierarchyNode->SetScene(this->m_scene);
  std::cout << this->m_activeHierarchyNode->GetID() << std::endl;
  hierarchyNode->SetParentNodeID(this->m_activeHierarchyNode->GetID());

  this->m_scene->AddNode(hierarchyNode);

  return hierarchyNode;

}

//---------------------------------------------------------------------------
//
/// Return the top level annotation hierarchy node in the scene. If there is none, create a new one.
vtkMRMLAnnotationHierarchyNode* vtkMRMLAnnotationHierarchyHelper::GetTopLevelHierarchyNode()
{
  vtkMRMLAnnotationHierarchyNode* toplevelNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(this->m_scene->GetNthNodeByClass(0,"vtkMRMLAnnotationHierarchyNode"));

  if (!toplevelNode)
    {
    // no annotation hierarchy node is currently in the scene, create a new one
    vtkMRMLAnnotationHierarchyNode* toplevelNode = vtkMRMLAnnotationHierarchyNode::New();
    toplevelNode->SetScene(this->m_scene);

    this->m_scene->AddNodeNoNotify(toplevelNode);
    }

  return toplevelNode;
}
