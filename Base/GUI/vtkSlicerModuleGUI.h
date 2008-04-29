#ifndef __vtkSlicerModuleGUI_h
#define __vtkSlicerModuleGUI_h

#include "vtkObject.h"
#include "vtkKWObject.h"
#include "vtkKWUserInterfacePanel.h"
#include "vtkKWWidget.h"
#include "vtkKWText.h"
#include "vtkKWTextWithHyperlinksWithScrollbars.h"
#include "vtkKWIcon.h"
#include "vtkKWNotebook.h"
#include "vtkKWFrame.h"
#include "vtkSmartPointer.h"

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerBaseAcknowledgementLogoIcons.h"

// Description:
// This is a base class from which all SlicerModuleGUIs that include
// their GUI in Slicer's shared GUI panel are derived. SlicerModuleGUIs
// that don't populate that panel with their widgets can derive
// directly from vtkSlicerComponentGUI.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerModuleGUI : public vtkSlicerComponentGUI
{

 public:
    static vtkSlicerModuleGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerModuleGUI, vtkKWObject );
    void PrintSelf (ostream& os, vtkIndent indent );

    // Description:
    // Get/Set pointers to the ApplicationLogic
    vtkGetObjectMacro ( UIPanel, vtkKWUserInterfacePanel );
    vtkSetObjectMacro ( UIPanel, vtkKWUserInterfacePanel );
    // Description:
    // Get the Collapsing frame that contains the notebook.
    vtkGetObjectMacro (HelpAndAboutFrame, vtkSlicerModuleCollapsibleFrame );
    // Description:
    // Get the Notebook that contains help and acknowledgements for module.
    vtkGetObjectMacro (HelpAndAboutNotebook, vtkKWNotebook);
    // Description:
    // Get the help text widget.
    vtkGetObjectMacro (HelpText, vtkKWTextWithHyperlinksWithScrollbars );
    // Description:
    // Get the about text widget.
    vtkGetObjectMacro (AboutText, vtkKWTextWithHyperlinksWithScrollbars );
    // Description:
    // Get the Frame into which contributor and sponsor logos are packed.
    vtkGetObjectMacro (LogoFrame, vtkKWFrame );

    // Description:
    // Get the Icons for Acknowledging Sponsorship of Slicer's base.
  vtkSlicerBaseAcknowledgementLogoIcons* GetAcknowledgementIcons ();

  // Description:
  // Get/Set Macro for ApplicationGUI: allow Modules to access
  // the overall application context
  vtkGetObjectMacro ( ApplicationGUI, vtkSlicerApplicationGUI );
  vtkSetObjectMacro ( ApplicationGUI, vtkSlicerApplicationGUI );

  // Description:
  // Get the categorization of the module.  THe category is used for
  // grouping modules together into menus.
  virtual const char *GetCategory() const {return "None";}
  
  // Description:
  // Get a logo for the module
  virtual vtkKWIcon* GetLogo() const;

  // Description:
  // Set the module logic
  virtual void SetModuleLogic( vtkSlicerLogic *logic ) { }; 
  
  //Description:
  // Implemented module initialization if needed
  virtual void Init() { };

  // Description:
  // Configures a module's help frame, with acknowledgment
  // in a consistent manner
  virtual void BuildHelpAndAboutFrame ( vtkKWWidget *parent,
                                        const char *help,
                                        const char *about);

  virtual void CreateModuleEventBindings ( ) { };
  virtual void ReleaseModuleEventBindings ( ) { };
  
  // Description:
  // propagate events generated in logic layer to GUI
    virtual void ProcessLogicEvents ( vtkObject * /*caller*/,
      unsigned long /*event*/, void * /*callData*/ ) { };
    // Description:
    // alternative method to propagate events generated in GUI to logic / mrml
    virtual void ProcessGUIEvents ( vtkObject * /*caller*/, 
      unsigned long /*event*/, void * /*callData*/ ) { };
    
    // Description:
    // alternative method to propagate events generated in GUI to logic / mrml
    virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
      unsigned long /*event*/, void * /*callData*/ ) { };
 //BTX
  enum
    {
      ModuleSelectedEvent = 73300,
   };
//ETX
 protected:

    // Description:
    // This user interface panel is populated with the GUI's widgets,
    // and is raised in Slicer's shared GUI panel when the module
    // is selected for use.
    vtkKWUserInterfacePanel *UIPanel;
    
    // Description:
    // A module's UIPanel's HelpAndAboutFrame contains a notebook
    // with a tab for module help, and a tab for acknowledgments
    // and logos.
    vtkSlicerModuleCollapsibleFrame *HelpAndAboutFrame;
    vtkKWNotebook *HelpAndAboutNotebook;

    // Description:
    // HelpText contains the text describing the
    // module's functionality and how to use it.
    vtkKWTextWithHyperlinksWithScrollbars *HelpText;

    // Description:
    // AboutText contains text describing
    // the module's authors and sponsors.
    vtkKWTextWithHyperlinksWithScrollbars *AboutText;
    // The LogoFrame is where a contributor's logos may
    // be packed.
    vtkKWFrame *LogoFrame;

    // Description:
    // Describes whether the GUI has been built or not.
    bool Built;
    
    // constructor, destructor.
    vtkSlicerModuleGUI ( );
    virtual ~vtkSlicerModuleGUI ( );

    static vtkSlicerBaseAcknowledgementLogoIcons *AcknowledgementIcons;
    vtkSlicerApplicationGUI *ApplicationGUI;
//BTX
    vtkSmartPointer<vtkKWIcon> Logo;
//ETX
 private:
    vtkSlicerModuleGUI ( const vtkSlicerModuleGUI& ); // Not implemented.
    void operator = ( const vtkSlicerModuleGUI& ); // Not implemented.
};


#endif


