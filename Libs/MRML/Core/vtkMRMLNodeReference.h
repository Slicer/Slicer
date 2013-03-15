/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.18 $

=========================================================================auto=*/

#ifndef __vtkMRMLNodeReference_h
#define __vtkMRMLNodeReference_h

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


///
/// class to hold information about a referenced node used by refering node
class VTK_MRML_EXPORT vtkMRMLNodeReference : public vtkObject
{
public: 
  vtkTypeMacro(vtkMRMLNodeReference,vtkObject);
  static vtkMRMLNodeReference *New();
  void PrintSelf(ostream& os, vtkIndent indent){};

  vtkMRMLNodeReference()
  {
    ReferencedNode = 0;
    ReferencingNode = 0;
    Events = 0;
  }

  virtual ~vtkMRMLNodeReference()
  {
  }

public:
  std::string     ReferenceRole;
  std::string     ReferencedNodeID;
  vtkMRMLNode*    ReferencingNode;
  vtkMRMLNode*    ReferencedNode;
  vtkIntArray*    Events;
};

#endif
