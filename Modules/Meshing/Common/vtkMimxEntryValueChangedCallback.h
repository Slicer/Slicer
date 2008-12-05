/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxEntryValueChangedCallback.h,v $
Language:  C++
Date:      $Date: 2008/07/06 20:30:13 $
Version:   $Revision: 1.2 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// .NAME vtkMimxEntryValueChangedCallback - supports legacy function callbacks for VTK
// .SECTION Description
// vtkMimxEntryValueChangedCallback is a callback that supports the legacy callback
// methods found in VTK. For example, the legacy method
// vtkProcessObject::SetStartMethod() is actually invoked using the
// command/observer design pattern of VTK, and the vtkMimxEntryValueChangedCallback
// is used to provide the legacy functionality. The callback function should
// have the form void func(void *clientdata), where clientdata is special data
// that should is associated with this instance of vtkCallbackCommand.
//
// .SECTION Caveats
// This is legacy glue. Please do not use; it will be eventually eliminated.

// .SECTION See Also
// vtkCommand vtkCallbackCommand

#ifndef __vtkMimxEntryValueChangedCallback_h
#define __vtkMimxEntryValueChangedCallback_h

#include "vtkCommand.h"
#include "vtkMimxCommonWin32Header.h"

// the old style void fund(void *) callbacks
class VTK_MIMXCOMMON_EXPORT vtkMimxEntryValueChangedCallback : public vtkCommand
{
public:
  static vtkMimxEntryValueChangedCallback *New() 
    {return new vtkMimxEntryValueChangedCallback;};

  // Description:
  // Satisfy the superclass API for callbacks.
  void Execute(vtkObject *invoker, unsigned long eid, void *calldata);

  // Description:
  // Methods to set and get client and callback information.
  void SetClientData(void *cd) 
    {this->ClientData = cd;};
  void SetCallback(void (*f)(void *clientdata)) 
    {this->Callback = f;};
  void SetClientDataDeleteCallback(void (*f)(void *))
    {this->ClientDataDeleteCallback = f;};
  
  void *ClientData;
  void (*Callback)(void *);
  void (*ClientDataDeleteCallback)(void *);

  vtkIdType GetState()
  {
          return this->State;
  }
  void SetState(vtkIdType currentState)
  {
          this->State = currentState;
  }
protected:
  vtkMimxEntryValueChangedCallback();
  ~vtkMimxEntryValueChangedCallback();
   vtkIdType State;
};


#endif /* __vtkMimxEntryValueChangedCallback_h */
 
