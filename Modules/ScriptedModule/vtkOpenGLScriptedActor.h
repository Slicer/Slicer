/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkOpenGLScriptedActor.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOpenGLScriptedActor - OpenGL actor
// .SECTION Description
// vtkOpenGLScriptedActor is a concrete implementation of the abstract class vtkScriptedActor.
// vtkOpenGLScriptedActor interfaces to the OpenGL rendering library.

#ifndef __vtkOpenGLScriptedActor_h
#define __vtkOpenGLScriptedActor_h

#include "vtkActor.h"

class vtkOpenGLRenderer;

#include "vtkScriptedModule.h"


class VTK_SCRIPTED_EXPORT vtkOpenGLScriptedActor : public vtkActor
{
protected:
  
public:
  static vtkOpenGLScriptedActor *New();
  vtkTypeRevisionMacro(vtkOpenGLScriptedActor,vtkActor);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Actual actor render method.
  void Render(vtkRenderer *ren, vtkMapper *mapper);

  // Description:
  // The string to execute to implement rendering of the actor
  // - user should embed references to appropriate Renderer, Actor and Mapper
  //   in the script so it can reference the data contained in them in order
  //   to implement the rendering
  vtkGetStringMacro(Script);
  vtkSetStringMacro(Script);

  // Description:
  // Should the Script be evaluated Tcl or Python?  Right now, just python
  // in order to test PyOpenGL
  vtkGetMacro(Language,int);
  vtkSetMacro(Language,int);
  //void SetLanguageToTcl() { this->SetLanguage(Tcl); } // Not yet supported
  void SetLanguageToPython() { this->SetLanguage(Python); }
//BTX
  enum {
    Tcl = 0,
    Python
  };
//ETX

protected:
  vtkOpenGLScriptedActor();
  ~vtkOpenGLScriptedActor();

private:
  vtkOpenGLScriptedActor(const vtkOpenGLScriptedActor&);  // Not implemented.
  void operator=(const vtkOpenGLScriptedActor&);  // Not implemented.

  int Language;
  char *Script;
};

#endif

