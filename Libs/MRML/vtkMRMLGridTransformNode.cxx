/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLGridTransformNode.cxx,v $
Date:      $Date: 2006/03/17 17:01:53 $
Version:   $Revision: 1.14 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkGridTransform.h"

#include "vtkMRMLGridTransformNode.h"
#include "vtkMRMLScene.h"


//------------------------------------------------------------------------------
vtkMRMLGridTransformNode* vtkMRMLGridTransformNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLGridTransformNode");
  if(ret)
    {
    return (vtkMRMLGridTransformNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLGridTransformNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLGridTransformNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLGridTransformNode");
  if(ret)
    {
    return (vtkMRMLGridTransformNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLGridTransformNode;
}

//----------------------------------------------------------------------------
vtkMRMLGridTransformNode::vtkMRMLGridTransformNode()
{
  vtkGridTransform *grid = vtkGridTransform::New();
  this->SetAndObserveWarpTransformToParent(grid);
  grid->Delete();
}

//----------------------------------------------------------------------------
vtkMRMLGridTransformNode::~vtkMRMLGridTransformNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLGridTransformNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  if (this->WarpTransformToParent != NULL)
    {
    // this transform should be a grid transform
    vtkGridTransform *grid = dynamic_cast<vtkGridTransform*>(this->WarpTransformToParent);
    if( grid == NULL )
      {
      vtkErrorMacro("Transform is not a GridTransform");
      return;
      }

    of << " interpolationMode=\"" << grid->GetInterpolationMode() << "\" ";
    of << " displacementScale=\"" << grid->GetDisplacementScale() << "\" ";
    of << " displacementShift=\"" << grid->GetDisplacementShift() << "\" ";
    of << grid->GetDisplacementGrid();
    of << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLGridTransformNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLGridTransformNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLGridTransformNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}


// End
