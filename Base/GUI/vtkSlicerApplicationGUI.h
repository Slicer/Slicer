/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationGUI.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/
// .NAME vtkSlicerApplicationGUI 
// .SECTION Description
// Main application GUI for slicer3.  Points to the ApplicationLogic and
// reflects changes in that logic back onto the UI.  Also routes changes
// from the GUI into the Logic to effect the user's desires.


#ifndef __vtkSlicerApplicationGUI_h
#define __vtkSlicerApplicationGUI_h

#include "vtkSlicerGUI.h"

#include "vtkSlicerApplicationLogic.h"

#include "vtkKWApplication.h"

class vtkSlicerComponentGUI;
class vtkSlicerGUICollection;
class vtkKWWindowBase;
class vtkCollection;


// Description:
// Contains a vtkKWApplication object, 
// and a collection of vtkSlicerGUIUnits.

//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerApplicationGUI : public vtkSlicerGUI
{
 public:
    static vtkSlicerApplicationGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerApplicationGUI, vtkSlicerGUI );


    vtkGetObjectMacro(Logic, vtkSlicerApplicationLogic);
    vtkSetObjectMacro(Logic, vtkSlicerApplicationLogic);


    vtkGetObjectMacro(KWApplication, vtkKWApplication);
    vtkSetObjectMacro(KWApplication, vtkKWApplication);
    
    // Description:
    // This method collects GUIs added to Slicer.
    virtual void AddGUI ( vtkSlicerComponentGUI *gui );

    // Description:
    // Sets application behavior.
    virtual void ConfigureApplication ( );

    // Description:
    // Starts up the application with no window
    virtual int StartApplication ( );

    // Description:
    // Adds a new window to the application
    virtual void AddWindow ( vtkKWWindowBase *win );

    // Description:
    // Closes all application windows
    virtual void CloseWindows ( );

    vtkGetMacro ( NumberOfGUIs, int );
    
 protected:
    vtkSlicerApplicationGUI ( );
    ~vtkSlicerApplicationGUI ( );

    vtkSlicerApplicationLogic* Logic;

    // Tcl_Interp *interp; 

    // Description:
    // Application widget
    vtkKWApplication *KWApplication;

    // Description:
    // Collections of widgets
    vtkSlicerGUICollection *GUICollection;

    // Description:
    // numbers of widgets
    int NumberOfGUIs;
    
 private:
    vtkSlicerApplicationGUI ( const vtkSlicerApplicationGUI& ); // Not implemented.
    void operator = ( const vtkSlicerGUI& ); //Not implemented.
}; 

#endif
