/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerFiberBundleDisplayLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <itksys/SystemTools.hxx> 
#include <itksys/Directory.hxx> 

#include "vtkSlicerFiberBundleDisplayLogic.h"

#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleStorageNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"

vtkCxxRevisionMacro(vtkSlicerFiberBundleDisplayLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerFiberBundleDisplayLogic);

//----------------------------------------------------------------------------
vtkSlicerFiberBundleDisplayLogic::vtkSlicerFiberBundleDisplayLogic()
{
  vtkDebugWithObjectMacro(this,"Making it real");
  this->LineModel = NULL;
  this->TubeModel = NULL;
  this->GlyphModel = NULL;

  this->FiberBundleNode = NULL;

}

//----------------------------------------------------------------------------
vtkSlicerFiberBundleDisplayLogic::~vtkSlicerFiberBundleDisplayLogic()
{

  if ( this->FiberBundleNode ) 
    {
    vtkSetAndObserveMRMLNodeMacro( this->FiberBundleNode, NULL );
    }

}


//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::SetAndObserveFiberBundleNode( vtkMRMLFiberBundleNode *fiberBundleNode )
{
  vtkSetAndObserveMRMLNodeMacro( this->FiberBundleNode , fiberBundleNode );

}

//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::ProcessMRMLEvents( vtkObject * caller, 
                                            unsigned long event, 
                                            void * callData )
{


  if (vtkMRMLFiberBundleNode::SafeDownCast(caller) != NULL
           && event == vtkMRMLFiberBundleNode::DisplayModifiedEvent)
    {
    //this->UpdateModelPolyData(vtkMRMLModelNode::SafeDownCast(caller));
    
    vtkDebugWithObjectMacro(this,"Hooray caught a display modified event");
    vtkErrorWithObjectMacro(this,"Hooray caught a display modified event from fiber bundle node");
    }

  vtkDebugWithObjectMacro(this,"Process MRML Events " << event );
  vtkErrorWithObjectMacro(this,"Process MRML Events " << event );
}


//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerFiberBundleDisplayLogic:             " << this->GetClassName() << "\n";

}


