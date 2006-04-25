// .NAME vtkSlicerGUILayout
// .SECTION Description
// Main application layout for slicer3

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkObject.h"

#ifndef __vtkSlicerGUILayout_h
#define __vtkSlicerGUILayout_h

// Description:
// This class contains the default dimensions for the various window layouts
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerGUILayout : public vtkObject
{

 public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerGUILayout* New ( );
    vtkTypeRevisionMacro ( vtkSlicerGUILayout, vtkObject );
    void PrintSelf ( ostream& os, vtkIndent indent );
    
   // Description:
    // Get/Set Macros for protected vtkSlicerApplicationGUI dimensions
    vtkGetMacro ( DefaultSlicerWindowWidth, int );
    vtkSetMacro ( DefaultSlicerWindowWidth, int );
    vtkGetMacro ( DefaultSlicerWindowHeight, int );
    vtkSetMacro ( DefaultSlicerWindowHeight, int );
    vtkGetMacro ( DefaultMainViewerWidth, int);
    vtkSetMacro ( DefaultMainViewerWidth, int);
    vtkGetMacro ( DefaultMainViewerHeight, int);
    vtkSetMacro ( DefaultMainViewerHeight, int);
    vtkGetMacro ( DefaultSliceGUIFrameHeight, int);
    vtkSetMacro ( DefaultSliceGUIFrameHeight, int);
    vtkGetMacro ( DefaultSliceGUIFrameWidth, int);
    vtkSetMacro ( DefaultSliceGUIFrameWidth, int);
    vtkGetMacro ( DefaultSliceWindowWidth, int);
    vtkSetMacro ( DefaultSliceWindowWidth, int);
    vtkGetMacro ( DefaultSliceWindowHeight, int);
    vtkSetMacro ( DefaultSliceWindowHeight, int);
    vtkGetMacro ( DefaultGUIPanelWidth, int);
    vtkSetMacro ( DefaultGUIPanelWidth, int);
    vtkGetMacro ( DefaultGUIPanelHeight, int);
    vtkSetMacro ( DefaultGUIPanelHeight, int);
    vtkGetMacro ( DefaultLogoFrameHeight, int );
    vtkSetMacro ( DefaultLogoFrameHeight, int );
    vtkGetMacro ( DefaultSlicerControlFrameHeight, int );
    vtkSetMacro ( DefaultSlicerControlFrameHeight, int );
    vtkGetMacro ( DefaultModuleControlPanelHeight, int );
    vtkSetMacro ( DefaultModuleControlPanelHeight, int );
    vtkGetMacro ( DefaultSliceControlFrameHeight, int );
    vtkSetMacro ( DefaultSliceControlFrameHeight, int );
    vtkGetMacro ( DefaultViewControlFrameHeight, int );
    vtkSetMacro ( DefaultViewControlFrameHeight, int );

    virtual void InitializeLayout ( );
    
 protected:
    vtkSlicerGUILayout ( );
    ~vtkSlicerGUILayout ( );
    
    // Description:
    // Dimensions for the Default Window & components
    int DefaultSlicerWindowHeight;
    int DefaultSlicerWindowWidth;
    int DefaultMainViewerHeight;
    int DefaultMainViewerWidth;
    int DefaultSliceGUIFrameHeight;
    int DefaultSliceGUIFrameWidth;
    int DefaultSliceWindowHeight;
    int DefaultSliceWindowWidth;
    int DefaultGUIPanelHeight;
    int DefaultGUIPanelWidth;
    
    // Description:
    // Dimensions for specific GUI panel components
    int DefaultLogoFrameHeight;
    int DefaultSlicerControlFrameHeight;
    int DefaultModuleControlPanelHeight;
    int DefaultSliceControlFrameHeight;    
    int DefaultViewControlFrameHeight;

 private:
    vtkSlicerGUILayout ( const vtkSlicerGUILayout& ); // Not implemented
    void operator = ( const vtkSlicerGUILayout& ); // Not implemented
};

#endif

