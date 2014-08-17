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
#include <vtkMatrixToLinearTransform.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>
#include <stack>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTransformNode);

//----------------------------------------------------------------------------
vtkMRMLTransformNode::vtkMRMLTransformNode()
{
  this->TransformToParent=NULL;
  this->TransformFromParent=NULL;
  this->ReadWriteAsTransformToParent = 0;
  this->DisableTransformModifiedEvent=0;
  this->TransformModifiedEventPending=0;
}

//----------------------------------------------------------------------------
vtkMRMLTransformNode::~vtkMRMLTransformNode()
{
  vtkSetAndObserveMRMLObjectMacro(this->TransformToParent, NULL);
  vtkSetAndObserveMRMLObjectMacro(this->TransformFromParent, NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::WriteXML(ostream& of, int nIndent)
{
  vtkIndent indent(nIndent);

  Superclass::WriteXML(of, nIndent);
  of << indent << " readWriteAsTransformToParent=\"" << (this->ReadWriteAsTransformToParent ? "true" : "false") << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::ReadXMLAttributes(const char** atts)
{
  int disabledTransformModify = this->StartTransformModify();
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "readWriteAsTransformToParent"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->ReadWriteAsTransformToParent = 1;
        }
      else
        {
        this->ReadWriteAsTransformToParent = 0;
        }
      }
    }

  this->EndModify(disabledModify);
  this->EndTransformModify(disabledTransformModify);
}

//---------------------------------------------------------------------------
void vtkMRMLTransformNode::FlattenGeneralTransform(vtkCollection* outputTransformList, vtkAbstractTransform* inputTransform)
{
  outputTransformList->RemoveAllItems();
  if (inputTransform==NULL)
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

//----------------------------------------------------------------------------
int vtkMRMLTransformNode::DeepCopyTransform(vtkAbstractTransform* dst, vtkAbstractTransform* src)
{
  if (src==NULL || dst==NULL)
    {
    // we should log an error, but unfortunately we cannot log a VTK macro in a static method
    return 0;
    }

  if (src->IsA("vtkGeneralTransform"))
    {
    // DeepCopy of GeneralTransforms copies concatenated transforms by reference
    // (so it is actually a shallow copy), we make a true deepcopy

    // Flatten the transform list to make the copying simpler
    vtkGeneralTransform* dstGeneral=vtkGeneralTransform::SafeDownCast(dst);
    vtkNew<vtkCollection> sourceTransformList;
    FlattenGeneralTransform(sourceTransformList.GetPointer(), src);

    // Copy the concatentated transforms
    vtkCollectionSimpleIterator it;
    vtkAbstractTransform* concatenatedTransform = NULL;
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
#if (VTK_MAJOR_VERSION <= 5)
    vtkImageData* srcCoefficients=vtkBSplineTransform::SafeDownCast(src)->GetCoefficients();
#else
    vtkImageData* srcCoefficients=vtkBSplineTransform::SafeDownCast(src)->GetCoefficientData();
#endif

    if (srcCoefficients)
      {
      vtkNew<vtkImageData> dstCoefficients;
      dstCoefficients->DeepCopy(srcCoefficients);
#if (VTK_MAJOR_VERSION <= 5)
      vtkBSplineTransform::SafeDownCast(dst)->SetCoefficients(dstCoefficients.GetPointer());
#else
      vtkBSplineTransform::SafeDownCast(dst)->SetCoefficientData(dstCoefficients.GetPointer());
#endif
      }
    }
  else if (src->IsA("vtkGridTransform")) // this handles vtkOrientedGridTransform as well
    {
    // Fix up the DeepCopy for vtkGridTransform (it performs only a ShallowCopy on the displacement grid)
    dst->DeepCopy(src);
    vtkImageData* srcDisplacementGrid=vtkGridTransform::SafeDownCast(src)->GetDisplacementGrid();
    if (srcDisplacementGrid)
      {
      vtkNew<vtkImageData> dstDisplacementGrid;
      dstDisplacementGrid->DeepCopy(srcDisplacementGrid);
#if (VTK_MAJOR_VERSION <= 5)
      vtkGridTransform::SafeDownCast(dst)->SetDisplacementGrid(dstDisplacementGrid.GetPointer());
#else
      vtkGridTransform::SafeDownCast(dst)->SetDisplacementGridData(dstDisplacementGrid.GetPointer());
#endif
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
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLTransformNode::Copy(vtkMRMLNode *anode)
{
  int disabledTransformModify = this->StartTransformModify();
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLTransformNode *node = (vtkMRMLTransformNode *) anode;
  if (node==NULL)
    {
    vtkErrorMacro("vtkMRMLTransformNode::Copy: input node type is incompatible");
    return;
    }

  this->SetReadWriteAsTransformToParent(node->GetReadWriteAsTransformToParent());

  // Unfortunately VTK transform DeepCopy actually performs a shallow copy (only data object
  // pointers are copied, but not the contents itself), so we have to apply our custom DeepCopy
  // operation.
  if (node->TransformToParent)
    {
    vtkAbstractTransform* transformCopy=node->TransformToParent->MakeTransform();
    DeepCopyTransform(transformCopy,node->TransformToParent);
    vtkSetAndObserveMRMLObjectMacro(this->TransformToParent, transformCopy);
    transformCopy->Delete();
    }
  if (node->TransformFromParent)
    {
    vtkAbstractTransform* transformCopy=node->TransformFromParent->MakeTransform();
    DeepCopyTransform(transformCopy,node->TransformFromParent);
    vtkSetAndObserveMRMLObjectMacro(this->TransformFromParent, transformCopy);
    transformCopy->Delete();
    }

  this->Modified();
  this->TransformModified();

  this->EndModify(disabledModify);
  this->EndTransformModify(disabledTransformModify);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "ReadWriteAsTransformToParent: " << this->ReadWriteAsTransformToParent << "\n";

  // Flatten the transform list to make the copying simpler
  if (this->TransformToParent)
    {
    os << indent << "TransformToParent: \n";
    vtkNew<vtkCollection> sourceTransformList;
    FlattenGeneralTransform(sourceTransformList.GetPointer(), this->TransformToParent);
    vtkCollectionSimpleIterator it;
    vtkAbstractTransform* concatenatedTransform = NULL;
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
    vtkAbstractTransform* concatenatedTransform = NULL;
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
    return NULL;
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
    return NULL;
    }
}

//----------------------------------------------------------------------------
int  vtkMRMLTransformNode::IsTransformToWorldLinear()
{
  if (this->IsLinear() == 0)
    {
    return 0;
    }
  else
    {
    vtkMRMLTransformNode *parent = this->GetParentTransformNode();
    if (parent != NULL)
      {
      return parent->IsTransformToWorldLinear();
      }
    else
      {
      return 1;
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::GetTransformToWorld(vtkGeneralTransform* transformToWorld)
{
  if (transformToWorld==NULL)
    {
    vtkErrorMacro("vtkMRMLTransformNode::GetTransformToWorld failed: transformToWorld is invalid");
    return;
    }

  if (transformToWorld->GetNumberOfConcatenatedTransforms() == 0)
    {
    transformToWorld->Identity();
    }

  transformToWorld->PostMultiply();

  vtkAbstractTransform* transformToParent=this->GetTransformToParent();
  if (transformToParent!=NULL)
    {
    transformToWorld->Concatenate(transformToParent);
    }

  vtkMRMLTransformNode *parent = this->GetParentTransformNode();
  if (parent != NULL)
    {
    parent->GetTransformToWorld(transformToWorld);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::GetTransformFromWorld(vtkGeneralTransform* transformFromWorld)
{
  if (transformFromWorld==NULL)
    {
    vtkErrorMacro("vtkMRMLTransformNode::GetTransformFromWorld failed: transformToWorld is invalid");
    return;
    }

  if (transformFromWorld->GetNumberOfConcatenatedTransforms() == 0)
    {
    transformFromWorld->Identity();
    }

  transformFromWorld->PreMultiply();

  vtkAbstractTransform* transformFromParent=this->GetTransformFromParent();
  if (transformFromParent!=NULL)
    {
    transformFromWorld->Concatenate(transformFromParent);
    }

  vtkMRMLTransformNode *parent = this->GetParentTransformNode();
  if (parent != NULL)
    {
    parent->GetTransformFromWorld(transformFromWorld);
    }
}

//----------------------------------------------------------------------------
int  vtkMRMLTransformNode::IsTransformToNodeLinear(vtkMRMLTransformNode* node)
{
  if (this->IsTransformNodeMyParent(node))
    {
    vtkMRMLTransformNode *parent = this->GetParentTransformNode();
    if (parent != NULL)
      {
      if (!strcmp(parent->GetID(), node->GetID()) )
        {
        return this->IsLinear();
        }
      else
        {
        return this->IsLinear() * parent->IsTransformToNodeLinear(node);
        }
      }
    else return this->IsLinear();
    }
  else if (this->IsTransformNodeMyChild(node))
    {
    vtkMRMLTransformNode *parent = node->GetParentTransformNode();
    if (parent != NULL)
      {
      if (!strcmp(parent->GetID(), this->GetID()) )
        {
        return node->IsLinear();
        }
      else
        {
        return node->IsLinear() * parent->IsTransformToNodeLinear(this);
        }
      }
    else return node->IsLinear();
    }
  else if (this->IsTransformToWorldLinear() == 1 &&
           node->IsTransformToWorldLinear() == 1)
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
void  vtkMRMLTransformNode::GetTransformToNode(vtkMRMLTransformNode* node,
                                               vtkGeneralTransform* transformToNode)
{
  if (this->IsTransformNodeMyParent(node))
    {
    vtkMRMLTransformNode *parent = this->GetParentTransformNode();
    if (parent != NULL)
      {
      vtkAbstractTransform* transformToParent=this->GetTransformToParent();
      if (transformToParent)
        {
        transformToNode->Concatenate(transformToParent);
        }
      if (strcmp(parent->GetID(), node->GetID()) )
        {
        this->GetTransformToNode(node, transformToNode);
        }
      }
    else if (this->GetTransformToParent())
      {
      vtkAbstractTransform* transformToParent=this->GetTransformToParent();
      if (transformToParent)
        {
        transformToNode->Concatenate(transformToParent);
        }
      }
    }
  else if (this->IsTransformNodeMyChild(node))
    {
    vtkMRMLTransformNode *parent = node->GetParentTransformNode();
    if (parent != NULL)
      {
      vtkAbstractTransform* transformToParent=node->GetTransformToParent();
      if (transformToParent)
        {
        transformToNode->Concatenate(transformToParent);
        }
      if (strcmp(parent->GetID(), this->GetID()) )
        {
        node->GetTransformToNode(this, transformToNode);
        }
      }
    else
      {
      vtkAbstractTransform* transformToParent=node->GetTransformToParent();
      if (transformToParent)
        {
        transformToNode->Concatenate(transformToParent);
        }
      }
    }
  else
    {
    this->GetTransformToWorld(transformToNode);

    vtkNew<vtkGeneralTransform> transformToWorld2;
    node->GetTransformToWorld(transformToWorld2.GetPointer());
    transformToWorld2->Inverse();

    transformToNode->Concatenate(transformToWorld2.GetPointer());
    }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformNode::IsTransformNodeMyParent(vtkMRMLTransformNode* node)
{
  vtkMRMLTransformNode *parent = this->GetParentTransformNode();
  if (parent != NULL)
    {
    if (!strcmp(parent->GetID(), node->GetID()) )
      {
      return 1;
      }
    else
      {
      return parent->IsTransformNodeMyParent(node);
      }
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformNode::IsTransformNodeMyChild(vtkMRMLTransformNode* node)
{
  return node->IsTransformNodeMyParent(this);
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformNode::CanApplyNonLinearTransforms()const
{
  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::ApplyTransform(vtkAbstractTransform* transform)
{
  if (transform==NULL)
    {
    vtkErrorMacro("vtkMRMLTransformNode::ApplyTransform failed: input transform is invalid");
    return;
    }

  vtkAbstractTransform* transformCopy=transform->MakeTransform();
  DeepCopyTransform(transformCopy, transform);

  if (this->TransformToParent)
    {
    vtkGeneralTransform* transformToParentGeneral=vtkGeneralTransform::SafeDownCast(this->TransformToParent);
    if (transformToParentGeneral==NULL)
      {
      // we need to convert this to a general transform
      vtkNew<vtkGeneralTransform> generalTransform;
      generalTransform->Concatenate(this->TransformToParent);
      vtkSetAndObserveMRMLObjectMacro(this->TransformToParent, generalTransform.GetPointer());
      transformToParentGeneral=generalTransform.GetPointer();
      }
    transformToParentGeneral->Concatenate(transformCopy);
    }
  else if (this->TransformFromParent)
    {
    vtkGeneralTransform* transformFromParentGeneral=vtkGeneralTransform::SafeDownCast(this->TransformFromParent);
    if (transformFromParentGeneral==NULL)
      {
      // we need to convert this to a general transform
      vtkNew<vtkGeneralTransform> generalTransform;
      generalTransform->Concatenate(this->TransformFromParent);
      vtkSetAndObserveMRMLObjectMacro(this->TransformFromParent, generalTransform.GetPointer());
      transformFromParentGeneral=generalTransform.GetPointer();
      }
    transformFromParentGeneral->Inverse();
    transformFromParentGeneral->PostMultiply();
    transformFromParentGeneral->Concatenate(transformCopy);
    transformFromParentGeneral->PreMultiply();
    transformFromParentGeneral->Inverse();
    }
  else
    {
    vtkSetAndObserveMRMLObjectMacro(this->TransformToParent, transformCopy);
    }
  transformCopy->Delete();
}

//----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLTransformNode::CreateDefaultStorageNode()
{
  return vtkMRMLTransformStorageNode::New();
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::CreateDefaultDisplayNodes()
{
  if (vtkMRMLTransformDisplayNode::SafeDownCast(this->GetDisplayNode())!=NULL)
    {
    // display node already exists
    return;
    }
  if (this->GetScene()==NULL)
    {
    vtkErrorMacro("vtkMRMLTransformNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkNew<vtkMRMLTransformDisplayNode> dispNode;
  this->GetScene()->AddNode(dispNode.GetPointer());
  this->SetAndObserveDisplayNodeID(dispNode->GetID());
}

//---------------------------------------------------------------------------
bool vtkMRMLTransformNode::GetModifiedSinceRead()
{
  if (this->Superclass::GetModifiedSinceRead())
    {
    return true;
    }
  if (ReadWriteAsTransformToParent)
    {
    if (this->TransformToParent && this->TransformToParent->GetMTime() > this->GetStoredTime())
      {
      return true;
      }
    }
  else
    {
    if (this->TransformFromParent && this->TransformFromParent->GetMTime() > this->GetStoredTime())
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
int  vtkMRMLTransformNode::GetMatrixTransformToWorld(vtkMatrix4x4* transformToWorld)
{
  // The fact that this method is called means that this is a non-linear transform,
  // so we cannot return the transform as a matrix
  transformToWorld->Identity();
  return 0;
}

//----------------------------------------------------------------------------
int  vtkMRMLTransformNode::GetMatrixTransformToNode(vtkMRMLTransformNode* vtkNotUsed(node),
                                                          vtkMatrix4x4* transformToNode)
{
  // The fact that this method is called means that this is a non-linear transform,
  // so we cannot return the transform as a matrix
  vtkWarningMacro("vtkMRMLTransformNode::GetMatrixTransformToNode failed: this is not a linear transform");
  transformToNode->Identity();
  return 0;
}

//----------------------------------------------------------------------------
vtkAbstractTransform* vtkMRMLTransformNode::GetAbstractTransformAs(vtkAbstractTransform* inputTransform, const char* transformClassName, bool logErrorIfFails)
{
  if (transformClassName==NULL)
    {
    vtkErrorMacro("vtkMRMLTransformNode::GetAbstractTransformAs failed: transformClassName is invalid");
    return NULL;
    }
  if (inputTransform==NULL)
    {
    if (logErrorIfFails)
      {
      vtkErrorMacro("vtkMRMLTransformNode::GetAbstractTransformAs failed: inputTransform is invalid");
      }
    return NULL;
    }
  if (inputTransform->IsA(transformClassName))
    {
    return inputTransform;
    }

  // Flatten the transform list to make the copying simpler
  vtkGeneralTransform* generalTransform=vtkGeneralTransform::SafeDownCast(inputTransform);
  if (generalTransform==NULL)
    {
    if (logErrorIfFails)
      {
      vtkErrorMacro("vtkMRMLTransformNode::GetAbstractTransformAs failed: expected a "<<transformClassName<<" type class and found "<<inputTransform->GetClassName()<<" instead");
      }
    return NULL;
    }

  vtkNew<vtkCollection> transformList;
  FlattenGeneralTransform(transformList.GetPointer(), generalTransform);

  if (transformList->GetNumberOfItems()==1)
    {
    vtkAbstractTransform* transform=vtkAbstractTransform::SafeDownCast(transformList->GetItemAsObject(0));
    if (transform==NULL)
      {
      vtkErrorMacro("vtkMRMLTransformNode::GetAbstractTransformAs failed: the stored transform is invalid");
      return NULL;
      }
    if (!transform->IsA(transformClassName))
      {
      if (logErrorIfFails)
        {
        vtkErrorMacro("vtkMRMLTransformNode::GetAbstractTransformAs failed: expected a "<<transformClassName<<" type class and found "<<transform->GetClassName()<<" instead");
        }
      return NULL;
      }
    return transform;
    }
  else if (transformList->GetNumberOfItems()==0)
    {
    if (logErrorIfFails)
      {
      vtkErrorMacro("vtkMRMLTransformNode::GetAbstractTransformAs failed: the input transform does not contain any transforms");
      }
    return NULL;
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
    return NULL;
    }
}

//----------------------------------------------------------------------------
vtkAbstractTransform* vtkMRMLTransformNode::GetTransformToParentAs(const char* transformClassName, bool logErrorIfFails/* =true */)
{
  if (this->TransformToParent)
    {
    return GetAbstractTransformAs(this->TransformToParent, transformClassName, logErrorIfFails);
    }
  else if (this->TransformFromParent)
    {
    vtkAbstractTransform *transform = GetAbstractTransformAs(this->TransformFromParent, transformClassName, logErrorIfFails);
    if (transform!=NULL)
      {
      return transform->GetInverse();
      }
    }
  return NULL;
}

//----------------------------------------------------------------------------
vtkAbstractTransform* vtkMRMLTransformNode::GetTransformFromParentAs(const char* transformClassName, bool logErrorIfFails/* =true */)
{
  if (this->TransformFromParent)
    {
    return GetAbstractTransformAs(this->TransformFromParent, transformClassName, logErrorIfFails);
    }
  else if (this->TransformToParent)
    {
    vtkAbstractTransform *transform = GetAbstractTransformAs(this->TransformToParent, transformClassName, logErrorIfFails);
    if (transform!=NULL)
      {
      return transform->GetInverse();
      }
    }
  return NULL;
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
  int disabledTransformModify = this->StartTransformModify();
  int disabledModify = this->StartModify();

  vtkSetAndObserveMRMLObjectMacro((*originalTransformPtr), transform);

  // We set the inverse to NULL, which means that it's unknown and will be computed atuomatically from the original transform
  vtkSetAndObserveMRMLObjectMacro((*inverseTransformPtr), NULL);

  SetReadWriteAsTransformToParentAuto();

  this->StorableModifiedTime.Modified();
  this->TransformModified();

  this->EndModify(disabledModify);
  this->EndTransformModify(disabledTransformModify);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformNode::SetReadWriteAsTransformToParentAuto()
{
  bool isToParentForward=false;
  bool isFromParentForward=false;

  // We don't care about linear transforms (as they can be written in any direction) and we cannot
  // do anything about unknown transform types, so just handle warp transforms (such as bspline and grid)

  if (this->TransformToParent)
    {
    vtkWarpTransform* warpTransformToParent=vtkWarpTransform::SafeDownCast(this->GetTransformToParentAs("vtkWarpTransform", false));
    if (warpTransformToParent)
      {
      // Update is needed bacause it refreshes the inverse flag (the flag may be out-of-date if the transform depends on its inverse)
      warpTransformToParent->Update();
      isToParentForward = !warpTransformToParent->GetInverseFlag();
      }
    }
  if (this->TransformFromParent)
    {
    vtkWarpTransform* warpTransformFromParent=vtkWarpTransform::SafeDownCast(this->GetTransformFromParentAs("vtkWarpTransform", false));
    if (warpTransformFromParent)
      {
      // Update is needed bacause it refreshes the inverse flag (the flag may be out-of-date if the transform depends on its inverse)
      warpTransformFromParent->Update();
      isFromParentForward = !warpTransformFromParent->GetInverseFlag();
      }
    }

  if (this->TransformToParent==NULL && this->TransformToParent!=NULL)
    {
    // toParent is computed automatically as inv(fromParent)
    isToParentForward=!isFromParentForward;
    }
  else if (this->TransformToParent!=NULL && this->TransformToParent==NULL)
    {
    // fromParent is computed automatically as inv(toParent)
    isFromParentForward=!isToParentForward;
    }

  if (isToParentForward==isFromParentForward)
    {
    // if both transform are forward (or both inverse or unavailable) then don't make any changes
    return;
    }

  SetReadWriteAsTransformToParent(isToParentForward);
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

  if (event ==  vtkCommand::ModifiedEvent && caller!=NULL)
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
    // this should only happen if they are both NULL
    return;
    }
  vtkAbstractTransform* oldTransformToParent=this->TransformToParent;
  vtkAbstractTransform* oldTransformFromParent=this->TransformFromParent;
  this->TransformToParent=oldTransformFromParent;
  this->TransformFromParent=oldTransformToParent;

  this->ReadWriteAsTransformToParent = !this->ReadWriteAsTransformToParent;

  this->StorableModifiedTime.Modified();
  this->Modified();
  this->TransformModified();
}

//----------------------------------------------------------------------------
unsigned long vtkMRMLTransformNode::GetTransformToWorldMTime()
{
  unsigned long latestMTime=0;
  vtkAbstractTransform* transformToParent=this->GetTransformToParent();
  if (transformToParent!=NULL)
    {
    latestMTime=transformToParent->GetMTime();
    }

  vtkMRMLTransformNode *parent = this->GetParentTransformNode();
  if (parent != NULL)
    {
    unsigned long parentMTime=parent->GetTransformToWorldMTime();
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
  if (this->TransformToParent==NULL)
    {
    if (this->TransformFromParent==NULL)
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
  if (this->TransformFromParent==NULL)
    {
    if (this->TransformToParent==NULL)
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
  if (inputTransform==NULL)
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
    vtkObject* transform=transformList->GetItemAsObject(i);

    vtkMatrixToLinearTransform* linearTransform=vtkMatrixToLinearTransform::SafeDownCast(transform);
    vtkBSplineTransform* bsplineTransform=vtkBSplineTransform::SafeDownCast(transform);
    vtkGridTransform* gridTransform=vtkGridTransform::SafeDownCast(transform);
    if (linearTransform!=NULL)
      {
      ss << " Linear";
      vtkMatrix4x4* m=linearTransform->GetMatrix();
      ss << std::endl <<"    "<<m->GetElement(0,0)<<"  "<<m->GetElement(0,1)<<"  "<<m->GetElement(0,2)<<"  "<<m->GetElement(0,3);
      ss << std::endl <<"    "<<m->GetElement(1,0)<<"  "<<m->GetElement(1,1)<<"  "<<m->GetElement(1,2)<<"  "<<m->GetElement(1,3);
      ss << std::endl <<"    "<<m->GetElement(2,0)<<"  "<<m->GetElement(2,1)<<"  "<<m->GetElement(2,2)<<"  "<<m->GetElement(2,3);
      ss << std::endl <<"    "<<m->GetElement(3,0)<<"  "<<m->GetElement(3,1)<<"  "<<m->GetElement(3,2)<<"  "<<m->GetElement(3,3);
      }
    else if (bsplineTransform!=NULL)
      {
      ss << " B-spline:";
#if (VTK_MAJOR_VERSION <= 5)
      vtkImageData* coefficients=bsplineTransform->GetCoefficients();
#else
      vtkImageData* coefficients=bsplineTransform->GetCoefficientData();
#endif
      if (coefficients!=NULL)
        {
        int* extent = coefficients->GetExtent();
        int gridSize[3]={extent[1]-extent[0]+1, extent[3]-extent[2]+1, extent[5]-extent[4]+1};
        ss << std::endl << "  Grid size: " << gridSize[0] << " " << gridSize[1] << " " <<gridSize[2];
        double* gridOrigin = coefficients->GetOrigin();
        ss << std::endl << "  Grid origin: " << gridOrigin[0] << " " << gridOrigin[1] << " " <<gridOrigin[2];
        double* gridSpacing = coefficients->GetSpacing();
        ss << std::endl << "  Grid spacing: " << gridSpacing[0] << " " << gridSpacing[1] << " " <<gridSpacing[2];
        vtkOrientedBSplineTransform* orientedBsplineTransform=vtkOrientedBSplineTransform::SafeDownCast(transform);
        if (orientedBsplineTransform!=NULL)
          {
          vtkMatrix4x4* gridOrientation = orientedBsplineTransform->GetGridDirectionMatrix();
          if (gridOrientation!=NULL)
            {
            ss << std::endl << "  Grid orientation:";
            for (int i=0; i<3; i++)
              {
              ss << std::endl <<"    "<<gridOrientation->GetElement(i,0)<<"  "<<gridOrientation->GetElement(i,1)<<"  "<<gridOrientation->GetElement(i,2);
              }
            }
          vtkMatrix4x4* bulkTransform = orientedBsplineTransform->GetBulkTransformMatrix();
          if (bulkTransform!=NULL)
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
    else if (gridTransform!=NULL)
      {
      ss << " Displacement field:";
      vtkImageData* displacementField=gridTransform->GetDisplacementGrid();
      if (displacementField!=NULL)
        {
        int* extent=displacementField->GetExtent();
        ss << std::endl << "  Grid size: " << (extent[1]-extent[0]+1) << " " << (extent[3]-extent[2]+1) << " " << (extent[5]-extent[4]+1);
        double* origin=displacementField->GetOrigin();
        ss << std::endl << "  Grid origin: " << origin[0] << " " << origin[1] << " " << origin[2];
        double* spacing=displacementField->GetSpacing();
        ss << std::endl << "  Grid spacing: " << spacing[0] << " " << spacing[1] << " " << spacing[2];
        vtkOrientedGridTransform* orientedGridTransform=vtkOrientedGridTransform::SafeDownCast(transform);
        if (orientedGridTransform!=NULL)
          {
          vtkMatrix4x4* gridOrientation = orientedGridTransform->GetGridDirectionMatrix();
          if (gridOrientation!=NULL)
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
    else
      {
      const char* className=transform->GetClassName();
      ss << " " << (className?className:"invalid");
      }
    }

  ss << std::ends;
  this->TransformInfo=ss.str();
  return this->TransformInfo.c_str();
}
