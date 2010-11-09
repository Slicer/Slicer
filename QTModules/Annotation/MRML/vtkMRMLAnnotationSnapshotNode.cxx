#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "vtkObjectFactory.h"
#include "vtkMRMLAnnotationSnapshotNode.h"
#include "vtkBitArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkPointData.h"
#include "vtkStringArray.h"

//------------------------------------------------------------------------------
vtkMRMLAnnotationSnapshotNode::vtkMRMLAnnotationSnapshotNode()
{
  this->m_SnapShot = 0;
}

//------------------------------------------------------------------------------
vtkMRMLAnnotationSnapshotNode::~vtkMRMLAnnotationSnapshotNode()
{
  if (this->m_SnapShot)
    {
    this->m_SnapShot->Delete();
    this->m_SnapShot = 0;
    }
}

//------------------------------------------------------------------------------
vtkMRMLAnnotationSnapshotNode* vtkMRMLAnnotationSnapshotNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationSnapshotNode");
  if(ret)
    {
    return (vtkMRMLAnnotationSnapshotNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationSnapshotNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLAnnotationSnapshotNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationSnapshotNode");
  if(ret)
    {
    return (vtkMRMLAnnotationSnapshotNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationSnapshotNode;
}
