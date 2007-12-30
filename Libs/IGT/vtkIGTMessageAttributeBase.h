/*=auto=========================================================================

Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: $
Date:      $Date: $
Version:   $Revision: $

=========================================================================auto=*/

#ifndef IGTMESSAGEATTRIBUTEBASE_H
#define IGTMESSAGEATTRIBUTEBASE_H

#include <string>
#include <vector>

#include "vtkIGTWin32Header.h" 
#include "vtkObject.h"
//#include "vtkIGTMessageGenericAttribute.h"

class vtkIGTMessageAttributeSet;

//BTX
template <class T> class vtkIGTMessageGenericAttribute;
//ETX

class VTK_IGT_EXPORT vtkIGTMessageAttributeBase : public vtkObject {

public:

  static vtkIGTMessageAttributeBase *New();
  vtkTypeRevisionMacro(vtkIGTMessageAttributeBase,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkIGTMessageAttributeBase();
  ~vtkIGTMessageAttributeBase();
  
  const char* GetType();

  //BTX

  // Get/set functions -- getting/setting data with type defined in a child class
  //
  //virtual int SetAttribute(T* ptr) {};
  /*
  template <typename T>
  int SetAttribute(T* ptr)
  {
    vtkIGTMessageGenericAttribute<T>* pTc 
      = static_cast<vtkIGTMessageGenericAttribute<T>*>(this);
    return pTc->SetAttribute(ptr);
  }

  //int GetAttribute(T* ptr) {};
  template <typename T>
  int GetAttribute(T* ptr)
  {
    vtkIGTMessageGenericAttribute<T>* pTc 
      = static_cast<vtkIGTMessageGenericAttribute<T>*>(this);
    return pTc->GetAttribute(ptr);
  }
  */

  //ETX

  // Junichi Tokuda 12/03/07 Notes:
  // Since it is impossible to make templated functions virtual,
  // the pointers are passed as (void*).
  virtual int SetAttribute(void* ptr) {};
  virtual int GetAttribute(void* ptr) {};

  virtual void ClearAttribute() {};
  /*
  {
    Tc* pTc = static_cast<Tc*>(this);
    pTc->ClearAttribute();
  }
  */



  int GetTypeID()
  {
    return this->type_id;
  }

protected:
  void SetTypeID(int id)
  {
    this->type_id = id;
  }

  const char*   type_name;

private:

  virtual int Alloc() {};// = 0;
  virtual int Free() {}; // = 0;
  // should be pure virutal function but I don't know how to do it in vtk ...

  int     type_id;     // this may be redundant; 
                       // used in vtkIGTMessageAttributeSet class to improve searching performance

  //BTX
  friend class vtkIGTMessageAttributeSet;
  //ETX
};


#endif // IGTMESSAGEATTRIBUTEBASE_H
