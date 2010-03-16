/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMimxTestErrorCallback.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkMimxTestErrorCallback - To display error messages encountered during
// program execution.
// .SECTION Description
// vtkMimxTestErrorCallback - to capture error during testing.
// The error is VTK generated.

#ifndef __vtkMimxTestErrorCallback_h
#define __vtkMimxTestErrorCallback_h

#include "vtkCommand.h"
#include "vtkKWApplication.h"
#include "vtkMimxCommonWin32Header.h"

class VTK_MIMXCOMMON_EXPORT vtkMimxTestErrorCallback : public vtkCommand
{
public:
  static vtkMimxTestErrorCallback *New() 
    {return new vtkMimxTestErrorCallback;};

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

  // Description:
  // Determines whether a vtk Error message has been invoked or not
  void SetState(vtkIdType currentState)
  {
    this->State = currentState;
  }

protected:
  vtkMimxTestErrorCallback();
  ~vtkMimxTestErrorCallback();
  vtkIdType State;
};


#endif /* __vtkMimxTestErrorCallback_h */
 

