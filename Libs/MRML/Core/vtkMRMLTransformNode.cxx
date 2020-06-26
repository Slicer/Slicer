/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTransformNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include "vtkMRMLTransformNode.h"

// MRML includes
#include "vtkMRMLBSplineTransformNode.h"
#include "vtkMRMLGridTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "vtkMRMLLinearTransformSequenceStorageNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformStorageNode.h"
#include "vtkMRMLTransformDisplayNode.h"
#include "vtkOrientedBSplineTransform.h"
#include "vtkOrientedGridTransform.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkCollection.h>
#include <vtkCollectionIterator.h>
#include <vtkGeneralTransform.h>
#include <vtkImageData.h>
#include <vtkLinearTransform.h>
#include <vtkHomogeneousTransform.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkThinPlateSplineTransform.h>
#include <vtkTransform.h>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <sstream>
#include <stack>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTransformNode);

//----------------------------------------------------------------------------
vtkMRMLTransformNode::vtkMRMLTransformNode()
{
  this->TransformToParent=nullptr;
  this->TransformFromParent=nullptr;
  this->ReadAsTransformToParent=0;

  this->CachedMatrixTransformToParent=vtkMatrix4x4::New();
  this->CachedMatrixTransformFromParent=vtkMatrix4x4::New();

  this->ContentModifiedEvents->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
}

//----------------------------------------------------------------------------
vtkMRMLTransformNode::~vtkMRMLTransformNode()
{
  vtkSetAndObserveMRMLObjectMacro(this->TransformToParent, nullptr);
  vtkSetAndObserveMRMLObjectMacro(this->TransformFromParent, nullptr);

  this->CachedMatrixTransformToParent->Delete();
  this->CachedMatrixTransformToParent=nullptr;
  this->CachedMatrixTransformFromParent->Delete();
  this->CachedMatrixTransformFromParent=nullptr;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);

    // For backward compatibility only
    if (!strcmp(attName, "readWriteAsTransformToParent"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->ReadAsTransformToParent = 1;
        }
      else
        {
        this->ReadAsTransformToParent = 0;
        }
      }

    }

  this->EndModify(disabledModify);
}

//---------------------------------------------------------------------------
void vtkMRMLTransformNode::FlattenGeneralTransform(vtkCollection* outputTransformList, vtkAbstractTransform* inputTransform)
{
  outputTransformList->RemoveAllItems();
  if (inputTransform==nullptr)
    {
    return;
    }

  vtkGeneralTransform* inputGeneralTransform=vtkGeneralTransform::SafeDownCast(inputTransform);
  if (inputGeneralTransform)
    {
    inputGeneralTransform->Update();
    }

  // Push the transforms onto the stack in reverse order
  std::stack< vtkAbstractTransform* > tstack;
  tstack.push(inputTransform);

  // Write out all the transforms on the stack
  while (!tstack.empty())
    {
    vtkAbstractTransform *transform = tstack.top();
    tstack.pop();
    if (transform->IsA("vtkGeneralTransform"))
      {
      // Decompose general transforms
      vtkGeneralTransform *gtrans = (vtkGeneralTransform *)transform;
      gtrans->Update();
      int n = gtrans->GetNumberOfConcatenatedTransforms();
      while (n > 0)
        {
        tstack.push(gtrans->GetConcatenatedTransform(--n));
        }
      }
    else
      {
      // Simple transform, just add to the list
      outputTransformList->AddItem(transform);
      }
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLTransformNode::AreTransformsEqual(vtkAbstractTransform* transform1, vtkAbstractTransform* transform2)
{
  if (transform1 == transform2)
    {
    return true;
    }
  vtkNew<vtkCollection> transformList1;
  vtkMRMLTransformNode::FlattenGeneralTransform(transformList1.GetPointer(), transform1);
  vtkNew<vtkCollection> transformList2;
  vtkMRMLTransformNode::FlattenGeneralTransform(transformList2.GetPointer(), transform2);
  if (transformList1->GetNumberOfItems() != transformList2->GetNumberOfItems())
    {
    return false;
    }
  vtkCollectionSimpleIterator it1;
  transformList1->InitTraversal(it1);
  vtkCollectionSimpleIterator it2;
  transformList2->InitTraversal(it2);
  vtkObject* transformComponent1 = nullptr;
  vtkObject* transformComponent2 = nullptr;
  while ((transformComponent1 = transformList1->GetNextItemAsObject(it1)) != nullptr
    && (transformComponent2 = transformList2->GetNextItemAsObject(it2)) != nullptr)
    {
    if (transformComponent1 != transformComponent2)
      {
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformNode::DeepCopyTransform(vtkAbstractTransform* dst, vtkAbstractTransform* src)
{
  if (src==nullptr || dst==nullptr)
    {
    vtkGenericWarningMacro("vtkMRMLTransformNode::DeepCopyTransform failed: source or destination transform is invalid");
    return 0;
    }

  if (src->IsA("vtkGeneralTransform"))
    {
    // DeepCopy of GeneralTransforms copies concatenated transforms by reference
    // (so it is actually a shallow copy), we make a true deepcopy

    // Flatten the transform list to make the copying simpler
    vtkGeneralTransform* dstGeneral=vtkGeneralTransform::SafeDownCast(dst);
    if (dstGeneral==nullptr)
      {
      vtkGenericWarningMacro("vtkMRMLTransformNode::DeepCopyTransform failed: destination transform has to be vtkGeneralTransform");
      return 0;
      }
    vtkNew<vtkCollection> sourceTransformList;
    FlattenGeneralTransform(sourceTransformList.GetPointer(), src);

    // Copy the concatenated transforms
    vtkCollectionSimpleIterator it;
    vtkAbstractTransform* concatenatedTransform = nullptr;
    dstGeneral->PreMultiply();
    for (sourceTransformList->InitTraversal(it); (concatenatedTransform = vtkAbstractTransform::SafeDownCast(sourceTransformList->GetNextItemAsObject(it))) ;)
      {
      vtkAbstractTransform* concatenatedTransformCopy=concatenatedTransform->MakeTransform();
      DeepCopyTransform(concatenatedTransformCopy,concatenatedTransform);
      dstGeneral->Concatenate(concatenatedTransformCopy);
      concatenatedTransformCopy->Delete();
      }
    }
  else if (src->IsA("vtkBSplineTransform")) // this handles vtkOrientedBSplineTransform as well
    {
    // Fix up the DeepCopy for vtkBSplineTransform (it performs only a ShallowCopy on the coefficient grid)
    dst->DeepCopy(src);
    if (vtkBSplineTransform::SafeDownCast(dst)==nullptr)
      {
      vtkGenericWarningMacro("vtkMRMLTransformNode::DeepCopyTransform failed: destination transform has to be vtkBSplineTransform");
      return 0;
      }
    vtkImageData* srcCoefficients=vtkBSplineTransform::SafeDownCast(src)->GetCoefficientData();
    if (srcCoefficients)
      {
      vtkNew<vtkImageData> dstCoefficients;
      dstCoefficients->DeepCopy(srcCoefficients);
      vtkBSplineTransform::SafeDownCast(dst)->SetCoefficientData(dstCoefficients.GetPointer());
      }
    }
  else if (src->IsA("vtkGridTransform")) // this handles vtkOrientedGridTransform as well
    {
    // Fix up the DeepCopy for vtkGridTransform (it performs only a ShallowCopy on the displacement grid)
    dst->DeepCopy(src);
    if (vtkGridTransform::SafeDownCast(dst)==nullptr)
      {
      vtkGenericWarningMacro("vtkMRMLTransformNode::DeepCopyTransform failed: destination transform has to be vtkGridTransform");
      return 0;
      }
    vtkImageData* srcDisplacementGrid=vtkGridTransform::SafeDownCast(src)->GetDisplacementGrid();
    if (srcDisplacementGrid)
      {
      vtkNew<vtkImageData> dstDisplacementGrid;
      dstDisplacementGrid->DeepCopy(srcDisplacementGrid);
      vtkGridTransform::SafeDownCast(dst)->SetDisplacementGridData(dstDisplacementGrid.GetPointer());
      }
    }
  else if (src->IsA("vtkThinPlateSplineTransform"))
    {
    // Fix up the DeepCopy for vtkThinPlateSplineTransform (it performs only a ShallowCopy on the landmark points)
    dst->DeepCopy(src);
    if (vtkThinPlateSplineTransform::SafeDownCast(dst)==nullptr)
      {
      vtkGenericWarningMacro("vtkMRMLTransformNode::DeepCopyTransform failed: destination transform has to be vtkThinPlateSplineTransform");
      return 0;
      }
    vtkPoints* srcSourceLandmarks=vtkThinPlateSplineTransform::SafeDownCast(src)->GetSourceLandmarks();
    if (srcSourceLandmarks)
      {
      vtkNew<vtkPoints> dstSourceLandmarks;
      dstSourceLandmarks->DeepCopy(srcSourceLandmarks);
      vtkThinPlateSplineTransform::SafeDownCast(dst)->SetSourceLandmarks(dstSourceLandmarks.GetPointer());
      }
    vtkPoints* srcTargetLandmarks=vtkThinPlateSplineTransform::SafeDownCast(src)->GetTargetLandmarks();
    if (srcTargetLandmarks)
      {
      vtkNew<vtkPoints> dstTargetLandmarks;
      dstTargetLandmarks->DeepCopy(srcTargetLandmarks);
      vtkThinPlateSplineTransform::SafeDownCast(dst)->SetTargetLandmarks(dstTargetLandmarks.GetPointer());
      }
    }
  else
    {
    // In other cases the actual deepcopy works well
    dst->DeepCopy(src);
    }

  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLTransformNode* node = vtkMRMLTransformNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }
  if (deepCopy)
  {
  this->SetReadAsTransformToParent(node->GetReadAsTransformToParent());

  // Unfortunately VTK transform DeepCopy actually performs a shallow copy (only data object
  // pointers are copied, but not the contents itself), so we have to apply our custom DeepCopy
  // operation.
  if (node->TransformToParent)
    {
    vtkSmartPointer<vtkAbstractTransform> transformCopy = vtkSmartPointer<vtkAbstractTransform>::Take(
      node->TransformToParent->MakeTransform());
    DeepCopyTransform(transformCopy, node->TransformToParent);
    vtkSetAndObserveMRMLObjectMacro(this->TransformToParent, transformCopy);
    }
  else
    {
    vtkSetAndObserveMRMLObjectMacro(this->TransformToParent, nullptr);
    }
  if (node->TransformFromParent)
    {
    vtkSmartPointer<vtkAbstractTransform> transformCopy = vtkSmartPointer<vtkAbstractTransform>::Take(
      node->TransformFromParent->MakeTransform());
    DeepCopyTransform(transformCopy, node->TransformFromParent);
    vtkSetAndObserveMRMLObjectMacro(this->TransformFromParent, transformCopy);
    }
  else
    {
    vtkSetAndObserveMRMLObjectMacro(this->TransformFromParent, nullptr);
    }
  }
else
  {
  // shallow-copy
  vtkSetAndObserveMRMLObjectMacro(this->TransformToParent, node->TransformToParent);
  vtkSetAndObserveMRMLObjectMacro(this->TransformFromParent, node->TransformFromParent);
  }
  this->Modified();
  this->TransformModified();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "ReadAsTransformToParent: " << this->ReadAsTransformToParent << "\n";

  // Flatten the transform list to make the copying simpler
  if (this->TransformToParent)
    {
    os << indent << "TransformToParent: \n";
    vtkNew<vtkCollection> sourceTransformList;
    FlattenGeneralTransform(sourceTransformList.GetPointer(), this->TransformToParent);
    vtkCollectionSimpleIterator it;
    vtkAbstractTransform* concatenatedTransform = nullptr;
    for (sourceTransformList->InitTraversal(it); (concatenatedTransform = vtkAbstractTransform::SafeDownCast(sourceTransformList->GetNextItemAsObject(it))) ;)
      {
      os << indent.GetNextIndent() << "Transform: "<<concatenatedTransform->GetClassName()<<"\n";
      concatenatedTransform->PrintSelf(os, indent.GetNextIndent().GetNextIndent());
      }
    }

  // Flatten the transform list to make the copying simpler
  if (this->TransformFromParent)
    {
    os << indent << "TransformFromParent: \n";
    vtkNew<vtkCollection> sourceTransformList;
    FlattenGeneralTransform(sourceTransformList.GetPointer(), this->TransformFromParent);
    vtkCollectionSimpleIterator it;
    vtkAbstractTransform* concatenatedTransform = nullptr;
    for (sourceTransformList->InitTraversal(it); (concatenatedTransform = vtkAbstractTransform::SafeDownCast(sourceTransformList->GetNextItemAsObject(it))) ;)
      {
      os << indent.GetNextIndent() << "Transform: "<<concatenatedTransform->GetClassName()<<"\n";
      concatenatedTransform->PrintSelf(os, indent.GetNextIndent().GetNextIndent());
      }
    }
}

//----------------------------------------------------------------------------
vtkAbstractTransform* vtkMRMLTransformNode::GetTransformToParent()
{
  if (this->TransformToParent)
    {
    return this->TransformToParent;
    }
  else if (this->TransformFromParent)
    {
    return this->TransformFromParent->GetInverse();
    }
  else
    {
    return nullptr;
    }
}

//----------------------------------------------------------------------------
vtkAbstractTransform* vtkMRMLTransformNode::GetTransformFromParent()
{
  if (this->TransformFromParent)
    {
    return this->TransformFromParent;
    }
  else if (this->TransformToParent)
    {
    return this->TransformToParent->GetInverse();
    }
  else
    {
    return nullptr;
    }
}

//----------------------------------------------------------------------------
int  vtkMRMLTransformNode::IsTransformToWorldLinear()
{
  for (vtkMRMLTransformNode* current = this; current != nullptr; current = current->GetParentTransformNode())
    {
    if (!current->IsLinear())
      {
      return 0;
      }
    }
  // not found non-linear transform component, therefore it's a linear transform
  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::GetTransformToWorld(vtkGeneralTransform* transformToWorld)
{
  if (transformToWorld==nullptr)
    {
    vtkErrorMacro("vtkMRMLTransformNode::GetTransformToWorld failed: transformToWorld is invalid");
    return;
    }
  vtkMRMLTransformNode::GetTransformBetweenNodes(this, nullptr, transformToWorld);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::GetTransformFromWorld(vtkGeneralTransform* transformFromWorld)
{
  if (transformFromWorld==nullptr)
    {
    vtkErrorMacro("vtkMRMLTransformNode::GetTransformFromWorld failed: transformToWorld is invalid");
    return;
    }
  vtkMRMLTransformNode::GetTransformBetweenNodes(nullptr, this, transformFromWorld);
}

//----------------------------------------------------------------------------
int  vtkMRMLTransformNode::IsTransformToNodeLinear(vtkMRMLTransformNode* targetNode)
{
  if (this == targetNode)
    {
    return 1;
    }
  if (this->IsTransformNodeMyParent(targetNode))
    {
    // traverse the transform tree from bottom to top, from this to target
    for (vtkMRMLTransformNode* current = this; current != targetNode; current = current->GetParentTransformNode())
      {
      if (!current->IsLinear())
        {
        return 0;
        }
      }
    return 1;
    }
  else if (this->IsTransformNodeMyChild(targetNode))
    {
    // traverse the transform tree from bottom to top, from target to this
    for (vtkMRMLTransformNode* current = targetNode; current != this; current = current->GetParentTransformNode())
      {
      if (!current->IsLinear())
        {
        return 0;
        }
      }
    return 1;
    }
  else
    {
    vtkMRMLTransformNode* firstCommonParentNode = this->GetFirstCommonParent(targetNode);
    if (this->IsTransformToNodeLinear(firstCommonParentNode) &&
        targetNode->IsTransformToNodeLinear(firstCommonParentNode) )
      {
      return 1;
      }
    else
      {
      return 0;
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::GetTransformToNode(vtkMRMLTransformNode* node,
                                               vtkGeneralTransform* transformToNode)
{
  vtkMRMLTransformNode::GetTransformBetweenNodes(this, node, transformToNode);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::GetTransformFromNode(vtkMRMLTransformNode* node,
                                               vtkGeneralTransform* transformFromNode)
{
  vtkMRMLTransformNode::GetTransformBetweenNodes(node, this, transformFromNode);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::GetTransformBetweenNodes(vtkMRMLTransformNode* sourceNode,
  vtkMRMLTransformNode* targetNode, vtkGeneralTransform* transformSourceToTarget)
{
  if (transformSourceToTarget == nullptr)
    {
    vtkGenericWarningMacro("vtkMRMLTransformNode::GetTransformToNode failed: transformSourceToTarget is invalid");
    return;
    }

  transformSourceToTarget->Identity();
  transformSourceToTarget->PostMultiply();

  if (targetNode == sourceNode)
    {
    return;
    }

  if (sourceNode != nullptr && sourceNode->IsTransformNodeMyParent(targetNode))
    {
    // traverse the transform tree from bottom to top, from sourceNode to targetNode
    for (vtkMRMLTransformNode* current = sourceNode; current != targetNode; current = current->GetParentTransformNode())
      {
      vtkAbstractTransform* transformToParent=current->GetTransformToParent();
      if (transformToParent)
        {
        transformSourceToTarget->Concatenate(transformToParent);
        }
      }
    }
  else if (sourceNode == nullptr || sourceNode->IsTransformNodeMyChild(targetNode))
    {
    // traverse the transform tree from bottom to top, from targetNode to sourceNode
    for (vtkMRMLTransformNode* current = targetNode; current != sourceNode; current = current->GetParentTransformNode())
      {
      vtkAbstractTransform* transformToParent=current->GetTransformToParent();
      if (transformToParent)
        {
        transformSourceToTarget->Concatenate(transformToParent);
        }
      }
    // in transformSourceToTarget we have transform targetNode->sourceNode,
    // need to invert to get sourceNode->targetNode
    transformSourceToTarget->Inverse();
    }
  else
    {
    vtkMRMLTransformNode* firstCommonParentNode = sourceNode->GetFirstCommonParent(targetNode);

    sourceNode->GetTransformToNode(firstCommonParentNode, transformSourceToTarget);

    vtkNew<vtkGeneralTransform> transformFromCommonParentNode;
    targetNode->GetTransformToNode(firstCommonParentNode, transformFromCommonParentNode.GetPointer());
    transformFromCommonParentNode->Inverse();

    transformSourceToTarget->Concatenate(transformFromCommonParentNode.GetPointer());
    }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformNode::IsTransformNodeMyParent(vtkMRMLTransformNode* node)
{
  if (node == nullptr)
    {
    // the nullptr (world) node is parent of all nodes, we don't have to iterate through the parents to know that it's
    // the parent of this transform node
    return 1;
    }
  for (vtkMRMLTransformNode* current=this->GetParentTransformNode(); current != nullptr; current = current->GetParentTransformNode())
    {
    if (node == current)
      {
      // node is a parent of this
      return 1;
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformNode::IsTransformNodeMyChild(vtkMRMLTransformNode* node)
{
  if (node == nullptr)
    {
    vtkErrorMacro("vtkMRMLTransformNode::IsTransformNodeMyChild failed: input node is invalid");
    return 0;
    }
  return node->IsTransformNodeMyParent(this);
}

//----------------------------------------------------------------------------
vtkMRMLTransformNode* vtkMRMLTransformNode::GetFirstCommonParent(vtkMRMLTransformNode* targetNode)
{
  if (targetNode==nullptr)
    {
    // target is the world node, so the common parent is the world
    return nullptr;
    }
  // traverse the transform tree from bottom to top, from this to target
  for (vtkMRMLTransformNode* current = this; current != nullptr; current = current->GetParentTransformNode())
    {
    if (targetNode->IsTransformNodeMyParent(current))
      {
      // parent of this and targetNode as well
      return current;
      }
    }
  return nullptr;
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformNode::CanApplyNonLinearTransforms()const
{
  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::ApplyTransform(vtkAbstractTransform* transform)
{
  if (transform==nullptr)
    {
    vtkErrorMacro("vtkMRMLTransformNode::ApplyTransform failed: input transform is invalid");
    return;
    }

  // Get the applied transform components
  vtkSmartPointer<vtkAbstractTransform> transformCopy=vtkSmartPointer<vtkAbstractTransform>::Take(transform->MakeTransform());
  DeepCopyTransform(transformCopy, transform);
  // Flatten the transform list that will be applied to make the resulting general transform simpler
  // (have a simple list instead of a complex hierarchy)
  vtkNew<vtkCollection> transformCopyList;
  FlattenGeneralTransform(transformCopyList.GetPointer(), transformCopy);

  vtkAbstractTransform* oldTransformToParent = GetTransformToParent();
  if (oldTransformToParent==nullptr && transformCopyList->GetNumberOfItems()==1)
    {
    // The transform was empty before and a non-composite transform is applied,
    // therefore there is no need to create a composite transform, just set the applied transform.
    vtkAbstractTransform* appliedTransform = vtkAbstractTransform::SafeDownCast(transformCopyList->GetItemAsObject(0));
    SetAndObserveTransformToParent(appliedTransform);
    return;
    }

  // We need the current transform to be a vtkGeneralTransform, which can store all the transform components.
  // (if the current transform is already a general transform tben we can just use that, otherwise we convert)
  // We arbitrarily pick the ToParent transform to store the new composited transform.
  vtkSmartPointer<vtkGeneralTransform> transformToParentGeneral = vtkGeneralTransform::SafeDownCast(oldTransformToParent);

  if (transformToParentGeneral.GetPointer()==nullptr)
    {
    transformToParentGeneral = vtkSmartPointer<vtkGeneralTransform>::New();
    if (oldTransformToParent!=nullptr)
      {
      transformToParentGeneral->Concatenate(oldTransformToParent);
      }
    }

  // Add components
  transformToParentGeneral->PostMultiply();
  for (int transformComponentIndex = transformCopyList->GetNumberOfItems()-1; transformComponentIndex>=0; transformComponentIndex--)
    {
    vtkAbstractTransform* transformComponent = vtkAbstractTransform::SafeDownCast(transformCopyList->GetItemAsObject(transformComponentIndex));
    transformToParentGeneral->Concatenate(transformComponent);
    }

  // Save the new transform
  SetAndObserveTransformToParent(transformToParentGeneral);
}

//-----------------------------------------------------------------------------
int vtkMRMLTransformNode::Split()
{
  if (!IsComposite())
    {
    // not composite, cannot split
    return 0;
    }
  vtkNew<vtkCollection> transformComponentList;
  vtkAbstractTransform* transformToParent = this->GetTransformToParent();
  if (transformToParent==nullptr)
    {
    // no transform available, cannot split
    return 0;
    }
  FlattenGeneralTransform(transformComponentList.GetPointer(), transformToParent);
  int numberOfTransformComponents = transformComponentList->GetNumberOfItems();
  if (numberOfTransformComponents<1)
    {
    // no items, nothing to split
    return 0;
    }
  // If number of items is 1 we still continue, in this case we simplify the transform
  // (as one transform can be in a general transform hierarchy)
  vtkMRMLTransformNode* parentTransformNode = this->GetParentTransformNode();
  for (int transformComponentIndex = numberOfTransformComponents-1; transformComponentIndex>=0; transformComponentIndex--)
    {
    vtkAbstractTransform* transformComponent = vtkAbstractTransform::SafeDownCast(transformComponentList->GetItemAsObject(transformComponentIndex));
    vtkSmartPointer<vtkMRMLTransformNode> transformComponentNode;
    // Create a new transform node if for all transforms (parent transforms) but the last (the transform that is being split)
    if (transformComponentIndex>0)
      {
      // Create a new transform node with the most suitable type.
      // The generic vtkMRMLTransformNode could handle everything but at a couple of places
      // specific transform node classes are still used. When vtkMRMLLinearTransformNode, vtkMRMLBSplineTransformNode, and
      // vtkMRMLGridTransformNode classes will be removed then we can simply create a vtkMRMLTransformNode regardless the VTK transform type.
      if (transformComponent->IsA("vtkLinearTransform"))
        {
        transformComponentNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
        }
      else if (transformComponent->IsA("vtkBSplineTransform"))
        {
        transformComponentNode = vtkSmartPointer<vtkMRMLBSplineTransformNode>::New();
        }
      else if (transformComponent->IsA("vtkGridTransform"))
        {
        transformComponentNode = vtkSmartPointer<vtkMRMLGridTransformNode>::New();
        }
      else
        {
        // vtkThinPlateSplineTransform and general transform
        transformComponentNode = vtkSmartPointer<vtkMRMLTransformNode>::New();
        }
      std::string baseName = std::string(this->GetName())+"_Component";
      std::string uniqueName = this->GetScene()->GenerateUniqueName(baseName.c_str());
      transformComponentNode->SetName(uniqueName.c_str());
      this->GetScene()->AddNode(transformComponentNode.GetPointer());
      }
    else
      {
      transformComponentNode = this;
      }

    // Set as to/from parent so that the transform will not be inverted
    // It is important to set the non-inverted transform because when the
    // transform is edited then we have to update the forward transform
    // (inverse transform is computed, therefore changing an inverse transform would not affect the forward transform
    // which would lead to inconsistency between the forward and inverse transform;
    // also, any update of the forward transform overwrites the computed inverse transform)
    bool transformComputedFromInverse = vtkMRMLTransformNode::IsAbstractTransformComputedFromInverse(transformComponent);
    if (transformComputedFromInverse)
      {
      transformComponentNode->SetAndObserveTransformFromParent(transformComponent->GetInverse());
      }
    else
      {
      transformComponentNode->SetAndObserveTransformToParent(transformComponent);
      }
    transformComponentNode->SetAndObserveTransformNodeID(parentTransformNode ? parentTransformNode->GetID() : nullptr);
    parentTransformNode=transformComponentNode.GetPointer();
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformNode::IsAbstractTransformComputedFromInverse(vtkAbstractTransform* abstractTransform)
{
  if (abstractTransform==nullptr)
    {
    return false;
    }

  vtkTransform* linearTransformComponent = vtkTransform::SafeDownCast(abstractTransform);
  if (linearTransformComponent)
    {
    linearTransformComponent->Update(); // Update is needed because it refreshes the inverse flag (the flag may be out-of-date if the transform depends on its inverse)
    return linearTransformComponent->GetInverseFlag();
    }

  vtkWarpTransform* warpTransformComponent = vtkWarpTransform::SafeDownCast(abstractTransform);
  if (warpTransformComponent)
    {
    warpTransformComponent->Update(); // Update is needed because it refreshes the inverse flag (the flag may be out-of-date if the transform depends on its inverse)
    return warpTransformComponent->GetInverseFlag();
    }

  vtkGeneralTransform* generalTransformComponent = vtkGeneralTransform::SafeDownCast(abstractTransform);
  if (generalTransformComponent)
    {
    generalTransformComponent->Update(); // Update is needed because it refreshes the inverse flag (the flag may be out-of-date if the transform depends on its inverse)
    return generalTransformComponent->GetInverseFlag();
    }

  vtkGenericWarningMacro("vtkMRMLTransformNode::IsTransformComputedFromInverse cannot determine if input transform " << abstractTransform->GetClassName() << " is inverted or not. Assuming not inverted.");
  return false;
}

//----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLTransformNode::CreateDefaultStorageNode()
{
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLTransformStorageNode"));
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::CreateDefaultDisplayNodes()
{
  if (vtkMRMLTransformDisplayNode::SafeDownCast(this->GetDisplayNode())!=nullptr)
    {
    // display node already exists
    return;
    }
  if (this->GetScene()==nullptr)
    {
    vtkErrorMacro("vtkMRMLTransformNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkMRMLTransformDisplayNode* dispNode = vtkMRMLTransformDisplayNode::SafeDownCast(
    this->GetScene()->AddNewNodeByClass("vtkMRMLTransformDisplayNode") );
  this->SetAndObserveDisplayNodeID(dispNode->GetID());
}

//---------------------------------------------------------------------------
bool vtkMRMLTransformNode::GetModifiedSinceRead()
{
  if (this->Superclass::GetModifiedSinceRead())
    {
    return true;
    }
  if (this->TransformToParent && this->TransformToParent->GetMTime() > this->GetStoredTime())
    {
    return true;
    }
  if (this->TransformFromParent && this->TransformFromParent->GetMTime() > this->GetStoredTime())
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformNode::GetMatrixTransformToParent(vtkMatrix4x4* matrix)
{
  if (matrix==nullptr)
    {
    vtkErrorMacro("vtkMRMLTransformNode::GetMatrixTransformToParent failed: matrix is invalid");
    return 0;
    }
  // No transform means identity transform, which is a linear transform
  if (this->TransformToParent==nullptr && this->TransformFromParent==nullptr)
    {
    matrix->Identity();
    return 1;
    }
  vtkLinearTransform* transform=vtkLinearTransform::SafeDownCast(GetTransformToParentAs("vtkLinearTransform", false));
  if (transform==nullptr)
    {
    vtkErrorMacro("Failed to get transformation matrix because transform is not linear");
    matrix->Identity();
    return 0;
    }
  transform->GetMatrix(matrix);
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformNode::GetMatrixTransformFromParent(vtkMatrix4x4* matrix)
{
  vtkNew<vtkMatrix4x4> transformToParentMatrix;
  int result = GetMatrixTransformToParent(transformToParentMatrix.GetPointer());
  vtkMatrix4x4::Invert(transformToParentMatrix.GetPointer(), matrix);
  return result;
}

//----------------------------------------------------------------------------
int  vtkMRMLTransformNode::GetMatrixTransformToWorld(vtkMatrix4x4* transformToWorld)
{
  return vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(this, nullptr, transformToWorld);
}

//----------------------------------------------------------------------------
int  vtkMRMLTransformNode::GetMatrixTransformFromWorld(vtkMatrix4x4* transformFromWorld)
{
  return vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(nullptr, this, transformFromWorld);
}

//----------------------------------------------------------------------------
int  vtkMRMLTransformNode::GetMatrixTransformToNode(vtkMRMLTransformNode* node, vtkMatrix4x4* transformToNode)
{
  return vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(this, node, transformToNode);
}

//----------------------------------------------------------------------------
int  vtkMRMLTransformNode::GetMatrixTransformFromNode(vtkMRMLTransformNode* node, vtkMatrix4x4* transformFromNode)
{
  return vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(node, this, transformFromNode);
}

//----------------------------------------------------------------------------
int  vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(vtkMRMLTransformNode* sourceNode, vtkMRMLTransformNode* targetNode, vtkMatrix4x4* transformSourceToTarget)
{
  if (transformSourceToTarget == nullptr)
    {
    vtkGenericWarningMacro("vtkMRMLTransformNode::GetMatrixTransformBetweenNodes failed: transformSourceToTarget matrix is invalid");
    return 0;
    }

  if (targetNode == sourceNode)
    {
    transformSourceToTarget->Identity();
    return 1;
    }

  if (sourceNode && sourceNode->IsTransformNodeMyParent(targetNode))
    {
    transformSourceToTarget->Identity();
    // traverse the transform tree from bottom to top, from sourceNode to target
    for (vtkMRMLTransformNode* current = sourceNode; current != targetNode; current = current->GetParentTransformNode())
      {
      vtkNew<vtkMatrix4x4> toParentMatrix;
      if (!current->GetMatrixTransformToParent(toParentMatrix.GetPointer()))
        {
        vtkGenericWarningMacro("vtkMRMLTransformNode::GetMatrixTransformBetweenNodes failed: expected linear transforms between nodes");
        transformSourceToTarget->Identity();
        return 0;
        }
      vtkMatrix4x4::Multiply4x4(toParentMatrix.GetPointer(), transformSourceToTarget, transformSourceToTarget);
      }
    }
  else if (sourceNode == nullptr || sourceNode->IsTransformNodeMyChild(targetNode))
    {
    transformSourceToTarget->Identity();
    vtkNew<vtkMatrix4x4> transformFromTargetNode;
    // traverse the transform tree from bottom to top, from target to sourceNode
    for (vtkMRMLTransformNode* current = targetNode; current != sourceNode; current = current->GetParentTransformNode())
      {
      vtkNew<vtkMatrix4x4> toParentMatrix;
      current->GetMatrixTransformToParent(toParentMatrix.GetPointer());
      if (!current->GetMatrixTransformToParent(toParentMatrix.GetPointer()))
        {
        vtkGenericWarningMacro("vtkMRMLTransformNode::GetMatrixTransformBetweenNodes failed: expected linear transforms between nodes");
        transformSourceToTarget->Identity();
        return 0;
        }
      vtkMatrix4x4::Multiply4x4(toParentMatrix.GetPointer(), transformFromTargetNode.GetPointer(), transformFromTargetNode.GetPointer());
      }
    vtkMatrix4x4::Invert(transformFromTargetNode.GetPointer(), transformSourceToTarget);
    }
  else
    {
    vtkMRMLTransformNode* firstCommonParentNode = sourceNode->GetFirstCommonParent(targetNode);

    if (!sourceNode->GetMatrixTransformToNode(firstCommonParentNode, transformSourceToTarget))
      {
      vtkGenericWarningMacro("vtkMRMLTransformNode::GetMatrixTransformBetweenNodes failed: expected linear transforms between nodes");
      transformSourceToTarget->Identity();
      return 0;
      }

    vtkNew<vtkMatrix4x4> transformFromCommonParentNode;
    if (!targetNode->GetMatrixTransformToNode(firstCommonParentNode, transformFromCommonParentNode.GetPointer()))
      {
      vtkGenericWarningMacro("vtkMRMLTransformNode::GetMatrixTransformBetweenNodes failed: expected linear transforms between nodes");
      transformSourceToTarget->Identity();
      return 0;
      }
    transformFromCommonParentNode->Invert();

    vtkMatrix4x4::Multiply4x4(transformFromCommonParentNode.GetPointer(), transformSourceToTarget, transformSourceToTarget);
    }
  return 1;
}

//----------------------------------------------------------------------------
vtkAbstractTransform* vtkMRMLTransformNode::GetAbstractTransformAs(vtkAbstractTransform* inputTransform, const char* transformClassName, bool logErrorIfFails)
{
  if (transformClassName==nullptr)
    {
    vtkErrorMacro("vtkMRMLTransformNode::GetAbstractTransformAs failed: transformClassName is invalid");
    return nullptr;
    }
  if (inputTransform==nullptr)
    {
    if (logErrorIfFails)
      {
      vtkErrorMacro("vtkMRMLTransformNode::GetAbstractTransformAs failed: inputTransform is invalid");
      }
    return nullptr;
    }
  if (inputTransform->IsA(transformClassName))
    {
    return inputTransform;
    }

  // Flatten the transform list to make the copying simpler
  vtkGeneralTransform* generalTransform=vtkGeneralTransform::SafeDownCast(inputTransform);
  if (generalTransform==nullptr)
    {
    if (logErrorIfFails)
      {
      vtkErrorMacro("vtkMRMLTransformNode::GetAbstractTransformAs failed: expected a "<<transformClassName<<" type class and found "<<inputTransform->GetClassName()<<" instead");
      }
    return nullptr;
    }

  vtkNew<vtkCollection> transformList;
  FlattenGeneralTransform(transformList.GetPointer(), generalTransform);

  if (transformList->GetNumberOfItems()==1)
    {
    vtkAbstractTransform* transform=vtkAbstractTransform::SafeDownCast(transformList->GetItemAsObject(0));
    if (transform==nullptr)
      {
      vtkErrorMacro("vtkMRMLTransformNode::GetAbstractTransformAs failed: the stored transform is invalid");
      return nullptr;
      }
    if (!transform->IsA(transformClassName))
      {
      if (logErrorIfFails)
        {
        vtkErrorMacro("vtkMRMLTransformNode::GetAbstractTransformAs failed: expected a "<<transformClassName<<" type class and found "<<transform->GetClassName()<<" instead");
        }
      return nullptr;
      }
    return transform;
    }
  else if (transformList->GetNumberOfItems()==0)
    {
    if (logErrorIfFails)
      {
      vtkErrorMacro("vtkMRMLTransformNode::GetAbstractTransformAs failed: the input transform does not contain any transforms");
      }
    return nullptr;
    }
  else
    {
    if (logErrorIfFails)
      {
      std::stringstream ss;
      for (int i=0; i<transformList->GetNumberOfItems(); i++)
        {
        const char* className=transformList->GetItemAsObject(i)->GetClassName();
        ss << " " << (className?className:"undefined");
        }
      ss << std::ends;
      vtkErrorMacro("vtkMRMLTransformNode::GetAbstractTransformAs failed: "<<generalTransform->GetNumberOfConcatenatedTransforms()
        <<" transforms are saved in a single node:"<<ss.str());
      }
    return nullptr;
    }
}

//----------------------------------------------------------------------------
vtkAbstractTransform* vtkMRMLTransformNode::GetTransformToParentAs(const char* transformClassName,
  bool logErrorIfFails/* =true */, bool modifiableOnly/* =false */)
{
  vtkAbstractTransform *transform = nullptr;
  if (this->TransformToParent)
    {
    transform = GetAbstractTransformAs(this->TransformToParent, transformClassName, logErrorIfFails);
    }
  else if (this->TransformFromParent)
    {
    vtkAbstractTransform *inverseTransform = GetAbstractTransformAs(this->TransformFromParent, transformClassName, logErrorIfFails);
    if (inverseTransform != nullptr)
      {
      transform = inverseTransform->GetInverse();
      }
    }
  if (modifiableOnly && transform != nullptr)
    {
    // if a transform is computed from its inverse then it is not editable
    if (vtkMRMLTransformNode::IsAbstractTransformComputedFromInverse(transform))
      {
      if (logErrorIfFails)
        {
        vtkErrorMacro("vtkMRMLTransformNode::GetTransformToParentAs failed: transform is available but not modifiable");
        }
      return nullptr;
      }
    }
  return transform;
}

//----------------------------------------------------------------------------
vtkAbstractTransform* vtkMRMLTransformNode::GetTransformFromParentAs(const char* transformClassName,
  bool logErrorIfFails/* =true */, bool modifiableOnly/* =false */)
{
  vtkAbstractTransform *transform = nullptr;
  if (this->TransformFromParent)
    {
    transform = GetAbstractTransformAs(this->TransformFromParent, transformClassName, logErrorIfFails);
    }
  else if (this->TransformToParent)
    {
    vtkAbstractTransform *inverseTransform = GetAbstractTransformAs(this->TransformToParent, transformClassName, logErrorIfFails);
    if (inverseTransform != nullptr)
      {
      transform = inverseTransform->GetInverse();
      }
    }
  if (modifiableOnly && transform != nullptr)
    {
    // if a transform is computed from its inverse then it is not editable
    if (vtkMRMLTransformNode::IsAbstractTransformComputedFromInverse(transform))
      {
      if (logErrorIfFails)
        {
        vtkErrorMacro("vtkMRMLTransformNode::GetTransformFromParentAs failed: transform is available but not modifiable");
        }
      return nullptr;
      }
    }
  return transform;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::SetAndObserveTransform(vtkAbstractTransform** originalTransformPtr, vtkAbstractTransform** inverseTransformPtr, vtkAbstractTransform *transform)
{
  if ((*originalTransformPtr)==transform)
    {
    // no change
    return;
    }

  // Temporarily disable all Modified and TransformModified events to make sure that
  // the operations are performed without interruption.
  int disabledModify = this->StartModify();

  vtkSetAndObserveMRMLObjectMacro((*originalTransformPtr), transform);

  // We set the inverse to nullptr, which means that it's unknown and will be computed atuomatically from the original transform
  vtkSetAndObserveMRMLObjectMacro((*inverseTransformPtr), nullptr);

  this->StorableModifiedTime.Modified();
  this->TransformModified();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::SetAndObserveTransformToParent(vtkAbstractTransform *transform)
{
  SetAndObserveTransform(&(this->TransformToParent), &(this->TransformFromParent), transform);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::SetAndObserveTransformFromParent(vtkAbstractTransform *transform)
{
  SetAndObserveTransform(&(this->TransformFromParent), &(this->TransformToParent), transform);
}

//---------------------------------------------------------------------------
void vtkMRMLTransformNode::ProcessMRMLEvents ( vtkObject *caller,
                                                    unsigned long event,
                                                    void *callData )
{
  Superclass::ProcessMRMLEvents ( caller, event, callData );

  if (event ==  vtkCommand::ModifiedEvent && caller!=nullptr)
    {
    if (caller == this->TransformToParent)
      {
      this->TransformModified();
      this->StorableModifiedTime.Modified();
      }
    else if (caller == this->TransformFromParent)
      {
      this->TransformModified();
      this->StorableModifiedTime.Modified();
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::Inverse()
{
  if (this->TransformToParent==this->TransformFromParent)
    {
    // this should only happen if they are both nullptr
    return;
    }
  vtkAbstractTransform* oldTransformToParent=this->TransformToParent;
  vtkAbstractTransform* oldTransformFromParent=this->TransformFromParent;
  this->TransformToParent=oldTransformFromParent;
  this->TransformFromParent=oldTransformToParent;

  this->StorableModifiedTime.Modified();
  this->Modified();
  this->TransformModified();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::InverseName()
{
  std::string nodeName = (this->GetName() ? this->GetName() : "");
  const std::string inverseSuffix(" (-)"); // this could be moved to a member variable
  if (vtksys::SystemTools::StringEndsWith(nodeName, inverseSuffix.c_str()))
    {
    nodeName.erase(nodeName.size() - inverseSuffix.size(), inverseSuffix.size());
    }
  else
    {
    nodeName += inverseSuffix;
    }
  this->SetName(nodeName.c_str());
}

//----------------------------------------------------------------------------
vtkMTimeType vtkMRMLTransformNode::GetTransformToWorldMTime()
{
  vtkMTimeType latestMTime=0;
  vtkAbstractTransform* transformToParent=this->GetTransformToParent();
  if (transformToParent!=nullptr)
    {
    latestMTime=transformToParent->GetMTime();
    }

  vtkMRMLTransformNode *parent = this->GetParentTransformNode();
  if (parent != nullptr)
    {
    vtkMTimeType parentMTime=parent->GetTransformToWorldMTime();
    if (parentMTime>latestMTime)
      {
      latestMTime=parentMTime;
      }
    }
  return latestMTime;
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransformNode::GetTransformToParentInfo()
{
  if (this->TransformToParent==nullptr)
    {
    if (this->TransformFromParent==nullptr)
      {
      this->TransformInfo="Not specified";
      }
    else
      {
      this->TransformInfo="Computed by inverting transform from parent.";
      }
    return this->TransformInfo.c_str();
    }
  return GetTransformInfo(this->TransformToParent);
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransformNode::GetTransformFromParentInfo()
{
  if (this->TransformFromParent==nullptr)
    {
    if (this->TransformToParent==nullptr)
      {
      this->TransformInfo="Not specified";
      }
    else
      {
      this->TransformInfo="Computed by inverting transform to parent.";
      }
    return this->TransformInfo.c_str();
    }
  return this->GetTransformInfo(this->TransformFromParent);
}


//----------------------------------------------------------------------------
const char* vtkMRMLTransformNode::GetTransformInfo(vtkAbstractTransform* inputTransform)
{
  this->TransformInfo="Not specified";
  if (inputTransform==nullptr)
    {
    // invalid
    return this->TransformInfo.c_str();
    }
  vtkNew<vtkCollection> transformList;
  this->FlattenGeneralTransform(transformList.GetPointer(), inputTransform);

  if (transformList->GetNumberOfItems()==0)
    {
    // empty generic transform
    return this->TransformInfo.c_str();
    }

  std::stringstream ss;
  for (int i=0; i<transformList->GetNumberOfItems(); i++)
    {
    if (transformList->GetNumberOfItems()>1)
      {
      if (i>0)
        {
        ss << std::endl;
        }
      ss << "Transform "<<i+1<<":";
      }
    vtkAbstractTransform* transform=vtkAbstractTransform::SafeDownCast(transformList->GetItemAsObject(i));
    if (transform)
      {
      transform->Update();
      }

    vtkHomogeneousTransform* linearTransform=vtkHomogeneousTransform::SafeDownCast(transform);
    vtkBSplineTransform* bsplineTransform=vtkBSplineTransform::SafeDownCast(transform);
    vtkGridTransform* gridTransform=vtkGridTransform::SafeDownCast(transform);
    vtkThinPlateSplineTransform* tpsTransform=vtkThinPlateSplineTransform::SafeDownCast(transform);
    if (linearTransform!=nullptr)
      {
      ss << " Linear";
      vtkMatrix4x4* m=linearTransform->GetMatrix();
      ss << std::endl <<"    "<<m->GetElement(0,0)<<"  "<<m->GetElement(0,1)<<"  "<<m->GetElement(0,2)<<"  "<<m->GetElement(0,3);
      ss << std::endl <<"    "<<m->GetElement(1,0)<<"  "<<m->GetElement(1,1)<<"  "<<m->GetElement(1,2)<<"  "<<m->GetElement(1,3);
      ss << std::endl <<"    "<<m->GetElement(2,0)<<"  "<<m->GetElement(2,1)<<"  "<<m->GetElement(2,2)<<"  "<<m->GetElement(2,3);
      ss << std::endl <<"    "<<m->GetElement(3,0)<<"  "<<m->GetElement(3,1)<<"  "<<m->GetElement(3,2)<<"  "<<m->GetElement(3,3);
      }
    else if (bsplineTransform!=nullptr)
      {
      ss << " B-spline:";
      bsplineTransform->Update(); // compute if inverse
      vtkImageData* coefficients=bsplineTransform->GetCoefficientData();
      if (coefficients!=nullptr)
        {
        int* extent = coefficients->GetExtent();
        int gridSize[3]={extent[1]-extent[0]+1, extent[3]-extent[2]+1, extent[5]-extent[4]+1};
        ss << std::endl << "  Grid size: " << gridSize[0] << " " << gridSize[1] << " " <<gridSize[2];
        double* gridOrigin = coefficients->GetOrigin();
        ss << std::endl << "  Grid origin: " << gridOrigin[0] << " " << gridOrigin[1] << " " <<gridOrigin[2];
        double* gridSpacing = coefficients->GetSpacing();
        ss << std::endl << "  Grid spacing: " << gridSpacing[0] << " " << gridSpacing[1] << " " <<gridSpacing[2];
        vtkOrientedBSplineTransform* orientedBsplineTransform=vtkOrientedBSplineTransform::SafeDownCast(transform);
        if (orientedBsplineTransform!=nullptr)
          {
          vtkMatrix4x4* gridOrientation = orientedBsplineTransform->GetGridDirectionMatrix();
          if (gridOrientation!=nullptr)
            {
            ss << std::endl << "  Grid orientation:";
            for (int i=0; i<3; i++)
              {
              ss << std::endl <<"    "<<gridOrientation->GetElement(i,0)<<"  "<<gridOrientation->GetElement(i,1)<<"  "<<gridOrientation->GetElement(i,2);
              }
            }
          vtkMatrix4x4* bulkTransform = orientedBsplineTransform->GetBulkTransformMatrix();
          if (bulkTransform!=nullptr)
            {
            ss << std::endl << "  Additive bulk transform:";
            for (int i=0; i<4; i++)
              {
              ss << std::endl <<"    "<<bulkTransform->GetElement(i,0)
                <<"  "<<bulkTransform->GetElement(i,1)<<"  "<<bulkTransform->GetElement(i,2)<<"  "<<bulkTransform->GetElement(i,3);
              }
            }
          }
        }
      if (bsplineTransform->GetInverseFlag())
        {
        ss << std::endl << "  Computed from its inverse.";
        }
      }
    else if (gridTransform!=nullptr)
      {
      ss << " Displacement field:";
      gridTransform->Update(); // compute if inverse
      vtkImageData* displacementField=gridTransform->GetDisplacementGrid();
      if (displacementField!=nullptr)
        {
        int* extent=displacementField->GetExtent();
        ss << std::endl << "  Grid size: " << (extent[1]-extent[0]+1) << " " << (extent[3]-extent[2]+1) << " " << (extent[5]-extent[4]+1);
        double* origin=displacementField->GetOrigin();
        ss << std::endl << "  Grid origin: " << origin[0] << " " << origin[1] << " " << origin[2];
        double* spacing=displacementField->GetSpacing();
        ss << std::endl << "  Grid spacing: " << spacing[0] << " " << spacing[1] << " " << spacing[2];
        vtkOrientedGridTransform* orientedGridTransform=vtkOrientedGridTransform::SafeDownCast(transform);
        if (orientedGridTransform!=nullptr)
          {
          vtkMatrix4x4* gridOrientation = orientedGridTransform->GetGridDirectionMatrix();
          if (gridOrientation!=nullptr)
            {
            ss << std::endl << "  Grid orientation:";
            for (int i=0; i<3; i++)
              {
              ss << std::endl <<"    "<<gridOrientation->GetElement(i,0)<<"  "<<gridOrientation->GetElement(i,1)<<"  "<<gridOrientation->GetElement(i,2);
              }
            }
          }
        }
      else
        {
        ss << std::endl << "  Displacement field is invalid.";
        }
      if (gridTransform->GetInverseFlag())
        {
        ss << std::endl << "  Computed from its inverse.";
        }
      }
    else if (tpsTransform!=nullptr)
      {
      ss << " Thin plate spline:";
      tpsTransform->Update(); // compute if inverse
      int numberOfSourceLandmarks = 0;
      if (tpsTransform->GetSourceLandmarks()!=nullptr)
        {
        numberOfSourceLandmarks = tpsTransform->GetSourceLandmarks()->GetNumberOfPoints();
        }
      int numberOfTargetLandmarks = 0;
      if (tpsTransform->GetTargetLandmarks()!=nullptr)
        {
        numberOfTargetLandmarks = tpsTransform->GetTargetLandmarks()->GetNumberOfPoints();
        }
      ss << std::endl << "  Number of source landmarks: " << numberOfSourceLandmarks;
      ss << std::endl << "  Number of target landmarks: " << numberOfTargetLandmarks;
      if (tpsTransform->GetInverseFlag())
        {
        ss << std::endl << "  Computed from its inverse.";
        }
      }
    else
      {
      const char* className = (transform ? transform->GetClassName() : nullptr);
      ss << " " << (className?className:"invalid");
      }
    }

  ss << std::ends;
  this->TransformInfo=ss.str();
  return this->TransformInfo.c_str();
}


//----------------------------------------------------------------------------
int vtkMRMLTransformNode::IsLinear()
{
  // Most often linear transform is a single vtkTransform stored in this->TransformToParent.
  // Do a simple check for this specific case first to make this method as fast as possible.
  if (this->TransformToParent!=nullptr && this->TransformToParent->IsA("vtkLinearTransform"))
    {
    return 1;
    }
  if (this->TransformFromParent!=nullptr && this->TransformFromParent->IsA("vtkLinearTransform"))
    {
    return 1;
    }
  // No transform means identity transform, which is a linear transform
  if (this->TransformToParent==nullptr && this->TransformFromParent==nullptr)
    {
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformNode::IsComposite()
{
  if (this->TransformToParent!=nullptr && this->TransformToParent->IsA("vtkGeneralTransform"))
    {
    return 1;
    }
  if (this->TransformFromParent!=nullptr && this->TransformFromParent->IsA("vtkGeneralTransform"))
    {
    return 1;
    }
  return 0;
}

// Deprecated method - kept temporarily for compatibility with extensions that are not yet updated
//----------------------------------------------------------------------------
vtkMatrix4x4* vtkMRMLTransformNode::GetMatrixTransformToParent()
{
  vtkWarningMacro("vtkMRMLTransformNode::GetMatrixTransformToParent() method is deprecated. Use vtkMRMLTransformNode::GetMatrixTransformToParent(vtkMatrix4x4*) instead");
  GetMatrixTransformToParent(this->CachedMatrixTransformToParent);
  return this->CachedMatrixTransformToParent;
}

// Deprecated method - kept temporarily for compatibility with extensions that are not yet updated
//----------------------------------------------------------------------------
vtkMatrix4x4* vtkMRMLTransformNode::GetMatrixTransformFromParent()
{
  vtkWarningMacro("vtkMRMLTransformNode::GetMatrixTransformFromParent() method is deprecated. Use vtkMRMLTransformNode::GetMatrixTransformFromParent(vtkMatrix4x4*) instead");
  GetMatrixTransformFromParent(this->CachedMatrixTransformFromParent);
  return this->CachedMatrixTransformFromParent;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformNode::SetMatrixTransformToParent(vtkMatrix4x4 *matrix)
{
  if (!this->IsLinear())
    {
    vtkErrorMacro("Cannot set matrix because vtkMRMLTransformNode contains a composite or non-linear transform. To overwrite the transform, first reset it by calling SetAndObserveTransformToParent(nullptr).");
    return 0;
    }

  // Temporarily disable all Modified and TransformModified events to make sure that
  // the operations are performed without interruption.
  int oldModify=this->StartModify();

  vtkTransform* currentTransform = nullptr;
  if (this->TransformToParent!=nullptr)
    {
    currentTransform = vtkTransform::SafeDownCast(GetTransformToParentAs("vtkTransform"));
    }

  // If current transform is an inverse transform then don't reuse it
  // (inverse transform is computed, therefore changing an inverse transform would not affect the forward transform
  // which would lead to inconsistency between the forward and inverse transform;
  // also, any update of the forward transform overwrites the computed inverse transform)
  if (currentTransform)
    {
    currentTransform->Update();
    if (currentTransform->GetInverseFlag())
      {
      currentTransform = nullptr;
      }
    }

  if (currentTransform)
    {
    // Set matrix
    if (matrix)
      {
      currentTransform->SetMatrix(matrix);
      }
    else
      {
      currentTransform->Identity();
      }
    }
  else
    {
    // Transform does not exist or not the right type, replace it with a new one
    vtkNew<vtkTransform> transform;
    if (matrix)
      {
      transform->SetMatrix(matrix);
      }
    this->SetAndObserveTransformToParent(transform.GetPointer());
    }

  this->TransformToParent->Modified();
  this->EndModify(oldModify);
  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformNode::SetMatrixTransformFromParent(vtkMatrix4x4 *matrix)
{
  vtkNew<vtkMatrix4x4> inverseMatrix;
  vtkMatrix4x4::Invert(matrix, inverseMatrix.GetPointer());
  return SetMatrixTransformToParent(inverseMatrix.GetPointer());
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::ApplyTransformMatrix(vtkMatrix4x4* transformMatrix)
{
  if (transformMatrix==nullptr)
    {
    vtkErrorMacro("vtkMRMLTransformNode::ApplyTransformMatrix failed: input transform is invalid");
    return;
    }
  if (!this->IsLinear())
    {
    // This object stores a non-linear transform, so we cannot merge the matrix with the existing transform, so
    // concatenate the linear transform (defined by transformMatrix) instead.
    vtkNew<vtkTransform> transform;
    transform->SetMatrix(transformMatrix);
    ApplyTransform(transform.GetPointer());
    return;
    }
  vtkNew<vtkMatrix4x4> matrixToParent;
  this->GetMatrixTransformToParent(matrixToParent.GetPointer());
  // vtkMatrix4x4::Multiply4x4 computes the output in an internal buffer and then
  // copies the result to the output matrix, therefore it is safe to use
  // one of the input matrices as output
  vtkMatrix4x4::Multiply4x4(transformMatrix, matrixToParent.GetPointer(), matrixToParent.GetPointer());
  SetMatrixTransformToParent(matrixToParent.GetPointer());
}

// Deprecated method - kept temporarily for compatibility with extensions that are not yet updated
//----------------------------------------------------------------------------
int vtkMRMLTransformNode::SetAndObserveMatrixTransformToParent(vtkMatrix4x4 *matrix)
{
  vtkWarningMacro("vtkMRMLTransformNode::SetAndObserveMatrixTransformToParent method is deprecated. Use vtkMRMLTransformNode::SetMatrixTransformToParent instead");
  return SetMatrixTransformToParent(matrix);
}

// Deprecated method - kept temporarily for compatibility with extensions that are not yet updated
//----------------------------------------------------------------------------
int vtkMRMLTransformNode::SetAndObserveMatrixTransformFromParent(vtkMatrix4x4 *matrix)
{
  vtkWarningMacro("vtkMRMLTransformNode::SetAndObserveMatrixTransformFromParent method is deprecated. Use vtkMRMLTransformNode::SetMatrixTransformFromParent instead");
  return SetMatrixTransformFromParent(matrix);
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformNode::IsGeneralTransformLinear(vtkAbstractTransform* inputTransform, vtkTransform* concatenatedLinearTransform/*=nullptr*/)
{
  if (inputTransform==nullptr)
    {
    return true;
    }

  if (concatenatedLinearTransform)
    {
    concatenatedLinearTransform->Identity();
    concatenatedLinearTransform->PostMultiply();
    }

  vtkHomogeneousTransform* inputHomogeneousTransform=vtkHomogeneousTransform::SafeDownCast(inputTransform);
  if (inputHomogeneousTransform)
    {
    if (concatenatedLinearTransform)
      {
      concatenatedLinearTransform->Concatenate(inputHomogeneousTransform->GetMatrix());
      }
    return true;
    }

  // Push the transforms onto the stack in reverse order (use a stack to avoid recursive method call)
  std::stack< vtkAbstractTransform* > tstack;
  tstack.push(inputTransform);

  // Put all the transforms on the stack
  while (!tstack.empty())
    {
    vtkAbstractTransform *transform = tstack.top();
    tstack.pop();
    if (transform->IsA("vtkGeneralTransform"))
      {
      // Decompose general transforms
      vtkGeneralTransform *gtrans = (vtkGeneralTransform *)transform;
      gtrans->Update();
      int n = gtrans->GetNumberOfConcatenatedTransforms();
      while (n > 0)
        {
        tstack.push(gtrans->GetConcatenatedTransform(--n));
        }
      }
    else
      {
      // Simple transform, just concatenate (if non-linear then return with false)
      vtkHomogeneousTransform* homogeneousTransform=vtkHomogeneousTransform::SafeDownCast(transform);
      if (homogeneousTransform)
        {
        if (concatenatedLinearTransform)
          {
          concatenatedLinearTransform->Concatenate(homogeneousTransform->GetMatrix());
          }
        }
      else
        {
        return false;
        }
      }
    }
  return true;
}

//----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLTransformNode::CreateDefaultSequenceStorageNode()
{
  return vtkMRMLLinearTransformSequenceStorageNode::New();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::CreateDefaultSequenceDisplayNodes()
{
  // don't create display nodes for transforms by default
}
