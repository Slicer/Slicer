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

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"

class vtkObject;
class vtkSlicerApplicationLogic;
class vtkKWLoadSaveButton;
class vtkKWRenderWidget;
class vtkImageViewer2;
class vtkKWScale;
class vtkKWWindowLevelPresetSelector;


// Description:
// This class implements Slicer's main Application GUI.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerApplicationGUI : public vtkSlicerComponentGUI
{
 public:
    static vtkSlicerApplicationGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerApplicationGUI, vtkSlicerComponentGUI );

    // Description:
    // This method builds Slicer's main GUI
    virtual void BuildGUI ( );
    virtual void AddGUIObservers ( );
    virtual void AddLogicObservers ( );
    virtual void UpdateGUIWithLogicEvents ( vtkObject *caller, unsigned long event,
                                            void *callData );
    virtual void UpdateLogicWithGUIEvents ( vtkObject *caller, unsigned long event,
                                           void *callData );
    
 protected:
    vtkSlicerApplicationGUI ( );
    ~vtkSlicerApplicationGUI ( );
    // And widgets.
    vtkKWLoadSaveButton *FileBrowseButton;
    vtkImageViewer2 *ImageViewer;
    vtkKWWindowLevelPresetSelector *WindowLevelPresetSelector;
    vtkKWRenderWidget *RenderWidget;
    vtkKWScale *Scale;

 private:
    vtkSlicerApplicationGUI ( const vtkSlicerApplicationGUI& ); // Not implemented.
    void operator = ( const vtkSlicerApplicationGUI& ); //Not implemented.
}; 

#endif
