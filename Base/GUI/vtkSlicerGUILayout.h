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
    vtkGetMacro (StandardSliceGUIFrameHeight, int );
    vtkSetMacro (StandardSliceGUIFrameHeight, int );
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
    vtkGetMacro ( DefaultTopFrameHeight, int );
    vtkSetMacro ( DefaultTopFrameHeight, int );
    vtkGetMacro ( DefaultModuleControlPanelHeight, int );
    vtkSetMacro ( DefaultModuleControlPanelHeight, int );
    vtkGetMacro ( DefaultSlicesControlFrameHeight, int );
    vtkSetMacro ( DefaultSlicesControlFrameHeight, int );
    vtkGetMacro ( DefaultViewControlFrameHeight, int );
    vtkSetMacro ( DefaultViewControlFrameHeight, int );
    vtkGetMacro ( SliceViewerMinDim, int );
    vtkSetMacro ( SliceViewerMinDim, int );
    // Description:
    // If the screen resolution is less than this number
    // (default is 1200 pixels) when a SliceControllerWidget
    // is first created, then the SliceControllerWidget
    // will be packed differently to allow all of its widgets
    // to be displayed and usable.
    vtkGetMacro ( SliceControllerResolutionThreshold, int );
    // Description:
    // If the SliceViewer width is greater than this number
    // of pixels (default 260) when the overall layout
    // is *adjusted*, then the SliceControllerWidgets will
    // be packed differently to save space.
    vtkGetMacro ( SliceViewerWidthThreshold, int );

    virtual void InitializeLayoutDimensions ( int width, int height, int sliceHeight);
    virtual void InitializeMainSlicerWindowSize ( );
    virtual void ConfigureMainSlicerWindowPanels ( );

 protected:
    vtkSlicerGUILayout ( );
    virtual ~vtkSlicerGUILayout ( );
    
    //Description:
    // Pointer to Slicer's 3Dviewer and
    // Three main Slice viewers.
    vtkKWWindow *MainSlicerWindow;
    
    // Description:
    // Dimensions for the Default Window & components
    int DefaultSlicerWindowHeight;
    int DefaultSlicerWindowWidth;
    int Default3DViewerHeight;
    int DefaultMainViewerWidth;
    int StandardSliceGUIFrameHeight;
    int DefaultSliceGUIFrameHeight;
    int DefaultSliceGUIFrameWidth;
    int DefaultGUIPanelHeight;
    int DefaultGUIPanelWidth;
    int DefaultQuadrantHeight;
    int DefaultQuadrantWidth;
    
    // Description:
    // Dimensions for specific GUI panel components
    int DefaultTopFrameHeight;
    int DefaultModuleControlPanelHeight;
    int DefaultSlicesControlFrameHeight;    
    int DefaultViewControlFrameHeight;

    // Description:
    // Minimum size for the Slice Windows
    int SliceViewerMinDim;
    int SliceControllerResolutionThreshold;
    int SliceViewerWidthThreshold;

 private:
    vtkSlicerGUILayout ( const vtkSlicerGUILayout& ); // Not implemented
    void operator = ( const vtkSlicerGUILayout& ); // Not implemented
};

#endif

