// .NAME vtkSlicerGUILayout
// .SECTION Description
// Main application layout for slicer3

#ifndef __vtkSlicerGUILayout_h
#define __vtkSlicerGUILayout_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkObject.h"

#include "vtkKWObject.h"
#include "vtkKWWindow.h"
#include "vtkKWFrame.h"


// Description:
// This class contains the default dimensions for the various window layouts
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerGUILayout : public vtkKWObject
{

 public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerGUILayout* New ( );
    vtkTypeRevisionMacro ( vtkSlicerGUILayout, vtkKWObject );
    void PrintSelf ( ostream& os, vtkIndent indent );
    
    vtkGetObjectMacro ( MainSlicerWindow, vtkKWWindow );
    vtkSetObjectMacro ( MainSlicerWindow, vtkKWWindow );
    vtkGetMacro (CurrentViewArrangement, int );
    vtkSetMacro (CurrentViewArrangement, int );

    // Description:
    // Main Slicer Window layout types

    //BTX
    enum
        {
            SlicerLayoutInitialView = 0,
            SlicerLayoutDefaultView,
            SlicerLayoutFourUpView,
            SlicerLayoutOneUp3DView,
            SlicerLayoutOneUpSliceView,
            SlicerLayoutTabbed3DView,
            SlicerLayoutTabbedSliceView,
            SlicerLayoutLightboxView
        };
    //ETX
    
   // Description:
    // Get/Set Macros for protected vtkSlicerApplicationGUI dimensions
    vtkGetMacro ( DefaultSlicerWindowWidth, int );
    vtkSetMacro ( DefaultSlicerWindowWidth, int );
    vtkGetMacro ( DefaultSlicerWindowHeight, int );
    vtkSetMacro ( DefaultSlicerWindowHeight, int );
    vtkGetMacro ( DefaultMainViewerWidth, int);
    vtkSetMacro ( DefaultMainViewerWidth, int);
    vtkGetMacro ( Default3DViewerHeight, int);
    vtkSetMacro ( Default3DViewerHeight, int);
    vtkGetMacro ( DefaultSliceGUIFrameHeight, int);
    vtkSetMacro ( DefaultSliceGUIFrameHeight, int);
    vtkGetMacro ( DefaultSliceGUIFrameWidth, int);
    vtkSetMacro ( DefaultSliceGUIFrameWidth, int);
    vtkGetMacro ( DefaultQuadrantHeight, int );
    vtkSetMacro ( DefaultQuadrantHeight, int );
    vtkGetMacro ( DefaultQuadrantWidth, int );
    vtkSetMacro ( DefaultQuadrantWidth, int );
    vtkGetMacro ( DefaultGUIPanelWidth, int);
    vtkSetMacro ( DefaultGUIPanelWidth, int);
    vtkGetMacro ( DefaultGUIPanelHeight, int);
    vtkSetMacro ( DefaultGUIPanelHeight, int);
    vtkGetMacro ( DefaultLogoFrameHeight, int );
    vtkSetMacro ( DefaultLogoFrameHeight, int );
    vtkGetMacro ( DefaultModuleChooseFrameHeight, int );
    vtkSetMacro ( DefaultModuleChooseFrameHeight, int );
    vtkGetMacro ( DefaultModuleControlPanelHeight, int );
    vtkSetMacro ( DefaultModuleControlPanelHeight, int );
    vtkGetMacro ( DefaultSlicesControlFrameHeight, int );
    vtkSetMacro ( DefaultSlicesControlFrameHeight, int );
    vtkGetMacro ( DefaultViewControlFrameHeight, int );
    vtkSetMacro ( DefaultViewControlFrameHeight, int );
    vtkGetMacro ( SliceViewerMinDim, int );
    vtkSetMacro ( SliceViewerMinDim, int );

    virtual void InitializeLayoutDimensions ( );
    virtual void InitializeMainSlicerWindowSize ( );
    virtual void ConfigureMainSlicerWindowPanels ( );

 protected:
    vtkSlicerGUILayout ( );
    virtual ~vtkSlicerGUILayout ( );
    
    //Description:
    // Pointer to Slicer's 3Dviewer and
    // Three main Slice viewers.
    vtkKWWindow *MainSlicerWindow;
    int CurrentViewArrangement;
    
    // Description:
    // Dimensions for the Default Window & components
    int DefaultSlicerWindowHeight;
    int DefaultSlicerWindowWidth;
    int Default3DViewerHeight;
    int DefaultMainViewerWidth;
    int DefaultSliceGUIFrameHeight;
    int DefaultSliceGUIFrameWidth;
    int DefaultGUIPanelHeight;
    int DefaultGUIPanelWidth;
    int DefaultQuadrantHeight;
    int DefaultQuadrantWidth;
    
    // Description:
    // Dimensions for specific GUI panel components
    int DefaultLogoFrameHeight;
    int DefaultModuleChooseFrameHeight;
    int DefaultModuleControlPanelHeight;
    int DefaultSlicesControlFrameHeight;    
    int DefaultViewControlFrameHeight;

    // Description:
    // Minimum size for the Slice Windows
    int SliceViewerMinDim;
    
 private:
    vtkSlicerGUILayout ( const vtkSlicerGUILayout& ); // Not implemented
    void operator = ( const vtkSlicerGUILayout& ); // Not implemented
};

#endif

