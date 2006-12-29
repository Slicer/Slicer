// .NAME vtkSlicerColorGUI 
// .SECTION Description
// Main Color GUI and mediator methods for slicer3. 


#ifndef __vtkSlicerColorGUI_h
#define __vtkSlicerColorGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerColorLogic.h"
#include "vtkMRMLColorNode.h"

#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"
#include "vtkKWChangeColorButton.h"

#include "vtkKWScaleWithEntry.h"
#include "vtkKWScale.h"

// Description:
// This class implements Slicer's Color GUI
//
class vtkSlicerColorDisplayWidget;
class vtkKWPushButton;
class vtkKWMessage;
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerColorGUI : public vtkSlicerModuleGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerColorGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerColorGUI, vtkSlicerModuleGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );
    
    // Description:
    // Get methods on class members ( no Set methods required. )
    vtkGetObjectMacro ( Logic, vtkSlicerColorLogic);
    vtkGetObjectMacro ( AddColorButton, vtkKWPushButton);

    // Description:
    // add a colour to a user defined table
    vtkKWPushButton *AddColorButton;


    // Description:
    // API for setting ColorNode, Logic and
    // for both setting and observing them.
    
    /*
    void SetMRMLNode ( vtkMRMLColorNode *node )
        { this->SetMRML ( vtkObjectPointer( &this->ColorNode), node ); }
    void SetAndObserveMRMLNode ( vtkMRMLColorNode *node )
        { this->SetAndObserveMRML ( vtkObjectPointer( &this->ColorNode), node ); }
    */
    
    void SetModuleLogic ( vtkSlicerColorLogic *logic )
    { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
    void SetAndObserveModuleLogic ( vtkSlicerColorLogic *logic )
    { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }

    // Description:
    // This method builds the Color module GUI
    virtual void BuildGUI ( ) ;

    // Description:
    // Add/Remove observers on widgets in the GUI
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );

    // Description:
    // Class's mediator methods for processing events invoked by
    // either the Logic, MRML or GUI.
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

    // Description:
    // Once know that the GUI has to be cleared and updated to show elements
    // from a new list, use this call
    virtual void SetGUIFromNode(vtkMRMLColorNode * activeColorNode);
    
    // Description:
    // Methods describe behavior at module enter and exit.
    virtual void Enter ( );
    virtual void Exit ( );

    // Description:
    // Getting and setting the mrml color node id
    vtkGetStringMacro(ColorNodeID);
    //vtkSetStringMacro(ColorlListNodeID);
    void SetColorNodeID(char *id);
   
    
    // Description:
    // Set the selected node, the color id, and update the widgets
    void SetColorNode(vtkMRMLColorNode *colorNode);

    //BTX
    // Description:
    // ColorIDModifiedEvent is generated when the ColorNodeID is
    // changed
    enum
    {
        ColorIDModifiedEvent = 30000,
    };
    //ETX


 protected:
    vtkSlicerColorGUI ( );
    virtual ~vtkSlicerColorGUI ( );

    // Module logic and mrml pointers
    vtkSlicerColorLogic *Logic;


    // Description:
    // The ID of the color node that is currently displayed in the GUI
    // TODO: probably redundant
    char *ColorNodeID;

    // Description:
    // The the color node that is currently displayed in the GUI
    vtkMRMLColorNode *ColorNode;
    
    // Description:
    // the widget that displays the colour node
    vtkSlicerColorDisplayWidget *ColorDisplayWidget;

private:
    vtkSlicerColorGUI ( const vtkSlicerColorGUI& ); // Not implemented.
    void operator = ( const vtkSlicerColorGUI& ); //Not implemented.
};


#endif
