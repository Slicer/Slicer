/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxInteractorStyleUser.h,v $
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

// .NAME vtkMimxInteractorStyleUser - provides customizable interaction routines
// 
// .SECTION Description
// The most common way to customize user interaction is to write a subclass
// of vtkInteractorStyle: vtkMimxInteractorStyleUser allows you to customize
// the interaction to without subclassing vtkInteractorStyle.  This is
// particularly useful for setting up custom interaction modes in
// scripting languages such as Tcl and Python.  This class allows you
// to hook into the MouseMove, ButtonPress/Release, KeyPress/Release,
// etc. events.  If you want to hook into just a single mouse button,
// but leave the interaction modes for the others unchanged, you
// must use e.g. SetMiddleButtonPressMethod() instead of the more
// general SetButtonPressMethod().

#ifndef __vtkMimxInteractorStyleUser_h
#define __vtkMimxInteractorStyleUser_h

#include "vtkInteractorStyleUser.h"
#include "vtkMimxCommonWin32Header.h"

// new motion flag
#define VTKIS_USERINTERACTION 8 

class VTK_MIMXCOMMON_EXPORT vtkMimxInteractorStyleUser : public vtkInteractorStyleUser 
{
public:
  static vtkMimxInteractorStyleUser *New();
  vtkTypeRevisionMacro(vtkMimxInteractorStyleUser,vtkInteractorStyleUser);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Get the most recent mouse position during mouse motion.  
  // In your user interaction method, you must use this to track
  // the mouse movement.  Do not use GetEventPosition(), which records
  // the last position where a mouse button was pressed.
  vtkGetVector2Macro(LastPos,int);

  // Description:
  // Get the previous mouse position during mouse motion, or after
  // a key press.  This can be used to calculate the relative 
  // displacement of the mouse.
  vtkGetVector2Macro(OldPos,int);

  // Description:
  // Test whether modifiers were held down when mouse button or key
  // was pressed
  vtkGetMacro(ShiftKey,int);
  vtkGetMacro(CtrlKey,int);

  // Description:
  // Get the character for a Char event.
  vtkGetMacro(Char,int);

  // Description:
  // Get the KeySym (in the same format as Tk KeySyms) for a 
  // KeyPress or KeyRelease method.
  vtkGetStringMacro(KeySym);

  // Description:
  // Get the mouse button that was last pressed inside the window
  // (returns zero when the button is released).
  vtkGetMacro(Button,int);

  // Description:
  // Generic event bindings
  virtual void OnMouseMove();
  virtual void OnLeftButtonDown();
  virtual void OnLeftButtonUp();
  virtual void OnMiddleButtonDown();
  virtual void OnMiddleButtonUp();
  virtual void OnRightButtonDown();
  virtual void OnRightButtonUp();

  // Description:
  // Keyboard functions
  virtual void OnChar();
  virtual void OnKeyPress();
  virtual void OnKeyRelease();

  // Description:
  // These are more esoteric events, but are useful in some cases.
  virtual void OnExpose();
  virtual void OnConfigure();
  virtual void OnEnter();
  virtual void OnLeave();

  virtual void OnTimer();

protected:

  vtkMimxInteractorStyleUser();
  ~vtkMimxInteractorStyleUser();

  int LastPos[2];
  int OldPos[2];

  int ShiftKey;
  int CtrlKey;
  int Char;
  char *KeySym;
  int Button;

private:
  vtkMimxInteractorStyleUser(const vtkMimxInteractorStyleUser&);  // Not implemented.
  void operator=(const vtkMimxInteractorStyleUser&);  // Not implemented.
};

#endif
