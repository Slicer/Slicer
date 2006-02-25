/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerGUI.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/
// .NAME vtkSlicerGUI 
// .SECTION Description
// Main application GUI for slicer3.  Points to the ApplicationLogic and
// reflects changes in that logic back onto the UI.  Also routes changes
// from the GUI into the Logic to effect the user's desires.


#ifndef __vtkSlicerGUI_h
#define __vtkSlicerGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWApplication.h"
#include "vtkSlicerStyle.h"
#include "vtkKWWindow.h"
#include "vtkSlicerGUICollection.h"

class vtkSlicerComponentGUI;


// Description:
// Contains slicer's style, application and collection of associated guis.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerGUI : public vtkKWApplication
{
 public:
    static vtkSlicerGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerGUI, vtkKWApplication );

    vtkGetObjectMacro ( SlicerStyle, vtkSlicerStyle );
    vtkSetObjectMacro ( SlicerStyle, vtkSlicerStyle );
    
    vtkGetObjectMacro ( SlicerWin, vtkKWWindow );
    vtkSetObjectMacro ( SlicerWin, vtkKWWindow );
    
    vtkGetObjectMacro ( GUICollection, vtkSlicerGUICollection );
    vtkSetObjectMacro ( GUICollection, vtkSlicerGUICollection );
    
    vtkGetMacro ( NumberOfGUIs, int );
    vtkSetMacro ( NumberOfGUIs, int );

    // Description:
    // This method collects GUIs added to Slicer.
    virtual void AddGUI ( vtkSlicerComponentGUI *gui );
    // Description:
    // This method closes all windows associated with the application
    virtual void CloseAllWindows ( ) ;
    virtual void DisplaySlicerWindow ( );
    
    // Description:
    // Sets application behavior.
    virtual void ConfigureApplication ( );
    virtual int StartApplication ( );
    
 protected:
    vtkSlicerGUI ( );
    ~vtkSlicerGUI ( );

    vtkSlicerStyle *SlicerStyle;
    vtkKWWindow *SlicerWin;

    // Description:
    // Collections of widgets
    vtkSlicerGUICollection *GUICollection;
    // Description:
    // Contains the main window, the title, main menu,
    // and buttons that navigate modules and core.
    vtkSlicerComponentGUI *SlicerGUI;
    
    // Description:
    // numbers of widgets
    int NumberOfGUIs;
    
    // Tcl_Interp *interp; 

    
 private:
    vtkSlicerGUI ( const vtkSlicerGUI& ); // Not implemented.
    void operator = ( const vtkSlicerGUI& ); //Not implemented.
}; 

#endif
