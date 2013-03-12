/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.18 $

=========================================================================auto=*/

#ifndef __MRMLReferencedNode_h
#define __MRMLReferencedNode_h

// MRML includes
#include "vtkMRML.h"
#include "vtkObserverManager.h"
#include "vtkIntArray.h"

class vtkMRMLScene;
class vtkMRMLNode;


// VTK includes
#include <vtkObject.h>
class vtkCallbackCommand;

// STD includes
#include <string>
#include <map>
#include <vector>



///
/// class to hold information about a referenced node used by refering node
class VTK_MRML_EXPORT MRMLReferencedNode 
{
public: 
  MRMLReferencedNode()
  {
    ReferencedNode = 0;
    Events = 0;
  }

  virtual ~MRMLReferencedNode()
  {
  }

public:
  std::string     ReferenceRole;
  std::string     ReferencedNodeID;
  vtkMRMLNode*    ReferencedNode;
  vtkIntArray* Events;
};

#endif
