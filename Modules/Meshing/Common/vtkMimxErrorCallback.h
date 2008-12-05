/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMimxErrorCallback.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkMimxErrorCallback - supports legacy function callbacks for VTK
// .SECTION Description
// vtkMimxErrorCallback is a callback that supports the legacy callback
// methods found in VTK. For example, the legacy method
// vtkProcessObject::SetStartMethod() is actually invoked using the
// command/observer design pattern of VTK, and the vtkMimxErrorCallback
// is used to provide the legacy functionality. The callback function should
// have the form void func(void *clientdata), where clientdata is special data
// that should is associated with this instance of vtkCallbackCommand.
//
// .SECTION Caveats
// This is legacy glue. Please do not use; it will be eventually eliminated.

// .SECTION See Also
// vtkCommand vtkCallbackCommand

#ifndef __vtkMimxErrorCallback_h
#define __vtkMimxErrorCallback_h

#include "vtkCommand.h"
#include "vtkKWApplication.h"
#include "vtkMimxCommonWin32Header.h"

class vtkKWMessageDialog;

// the old style void fund(void *) callbacks
class VTK_MIMXCOMMON_EXPORT vtkMimxErrorCallback : public vtkCommand
{
public:
  static vtkMimxErrorCallback *New() 
    {return new vtkMimxErrorCallback;};

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

  void SetKWApplication(vtkKWApplication *App)
  {
          this->KWApplication = App;
  }

  vtkKWApplication* GetKWApplication()
  {
          return this->KWApplication;
  } 

  vtkIdType GetState()
  {
          return this->State;
  }
  void SetState(vtkIdType currentState)
  {
          this->State = currentState;
  }
  void ErrorMessage(const char*);
protected:
  vtkMimxErrorCallback();
  ~vtkMimxErrorCallback();
  vtkKWApplication *KWApplication;
  vtkKWMessageDialog *KWMessageDialog;
  vtkIdType State;
};


#endif /* __vtkMimxErrorCallback_h */
 
