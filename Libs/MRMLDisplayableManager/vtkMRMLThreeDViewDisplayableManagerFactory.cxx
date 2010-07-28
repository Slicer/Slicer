/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLThreeDViewDisplayableManagerFactory.cxx,v $
  Date:      $Date: 2010-06-19 12:48:04 -0400 (Sat, 19 Jun 2010) $
  Version:   $Revision: 13859 $

=========================================================================auto=*/

// MRMLDisplayableManager includes
#include "vtkMRMLThreeDViewDisplayableManagerFactory.h"

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkMRMLThreeDViewDisplayableManagerFactory, "$Revision: 13859 $");

//----------------------------------------------------------------------------
// Needed when we don't use the vtkStandardNewMacro.
vtkInstantiatorNewMacro(vtkMRMLThreeDViewDisplayableManagerFactory);

//----------------------------------------------------------------------------
// vtkMRMLThreeDViewDisplayableManagerFactory methods

//----------------------------------------------------------------------------
// Up the reference count so it behaves like New
vtkMRMLThreeDViewDisplayableManagerFactory* vtkMRMLThreeDViewDisplayableManagerFactory::New()
{
  vtkMRMLThreeDViewDisplayableManagerFactory* instance = Self::GetInstance();
  instance->Register(0);
  return instance;
}

//----------------------------------------------------------------------------
vtkMRMLThreeDViewDisplayableManagerFactory* vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()
{
  if(!Self::Instance)
    {
    // Try the factory first
    Self::Instance = (vtkMRMLThreeDViewDisplayableManagerFactory*)
                     vtkObjectFactory::CreateInstance("vtkMRMLThreeDViewDisplayableManagerFactory");

    // if the factory did not provide one, then create it here
    if(!Self::Instance)
      {
      // if the factory failed to create the object,
      // then destroy it now, as vtkDebugLeaks::ConstructClass was called
      // with "vtkMRMLThreeDViewDisplayableManagerFactory", and not the real name of the class
#ifdef VTK_DEBUG_LEAKS
      vtkDebugLeaks::DestructClass("vtkMRMLThreeDViewDisplayableManagerFactory");
#endif
      Self::Instance = new vtkMRMLThreeDViewDisplayableManagerFactory;
      }
    }
  // return the instance
  return Self::Instance;
}

//----------------------------------------------------------------------------
vtkMRMLThreeDViewDisplayableManagerFactory::
    vtkMRMLThreeDViewDisplayableManagerFactory():Superclass()
{
}

//----------------------------------------------------------------------------
vtkMRMLThreeDViewDisplayableManagerFactory::~vtkMRMLThreeDViewDisplayableManagerFactory()
{
}

//----------------------------------------------------------------------------
void vtkMRMLThreeDViewDisplayableManagerFactory::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

VTK_SINGLETON_CXX(vtkMRMLThreeDViewDisplayableManagerFactory);

