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
#include "vtkIntArray.h"

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerBaseAcknowledgementLogoIcons.h"

/// Description:
/// This is a base class from which all SlicerModuleGUIs that include
/// their GUI in Slicer's shared GUI panel are derived. SlicerModuleGUIs
/// that don't populate that panel with their widgets can derive
/// directly from vtkSlicerComponentGUI.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerModuleGUI : public vtkSlicerComponentGUI
{

 public:
    static vtkSlicerModuleGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerModuleGUI, vtkKWObject );
    void PrintSelf (ostream& os, vtkIndent indent );

    /// 
    /// Get/Set pointers to the ApplicationLogic
    vtkGetObjectMacro ( UIPanel, vtkKWUserInterfacePanel );
    vtkSetObjectMacro ( UIPanel, vtkKWUserInterfacePanel );
    /// 
    /// Get the Collapsing frame that contains the notebook.
    vtkGetObjectMacro (HelpAndAboutFrame, vtkSlicerModuleCollapsibleFrame );
    /// 
    /// Get the Notebook that contains help and acknowledgements for module.
    vtkGetObjectMacro (HelpAndAboutNotebook, vtkKWNotebook);
    /// 
    /// Get the help text widget.
    vtkGetObjectMacro (HelpText, vtkKWTextWithHyperlinksWithScrollbars );
    /// 
    /// Get the about text widget.
    vtkGetObjectMacro (AboutText, vtkKWTextWithHyperlinksWithScrollbars );
    /// 
    /// Get the Frame into which contributor and sponsor logos are packed.
    vtkGetObjectMacro (LogoFrame, vtkKWFrame );

    /// 
    /// Get the Icons for Acknowledging Sponsorship of Slicer's base.
  vtkSlicerBaseAcknowledgementLogoIcons* GetAcknowledgementIcons ();

  /// 
  /// Get/Set Macro for ApplicationGUI: allow Modules to access
  /// the overall application context
  vtkGetObjectMacro ( ApplicationGUI, vtkSlicerApplicationGUI );
  vtkSetObjectMacro ( ApplicationGUI, vtkSlicerApplicationGUI );

  /// 
  /// Set the category for menu grouping
  vtkSetStringMacro(Category);
  virtual const char *GetCategory() const {return this->Category;}

  /// 
  /// Get/Set the index of the module.  The index is used for sorting the 
  /// modules within a menu. Modules are sorted first by index, then alphabetical
  virtual unsigned short GetIndex() const {return this->Index;}
  vtkSetMacro(Index, unsigned short);
  
  /// 
  /// Get a logo for the module
  virtual vtkKWIcon* GetLogo() const;

  /// 
  /// Set the module logic
  virtual void SetModuleLogic( vtkSlicerLogic * ); 
  
  ///  
  /// The name of the Module
  vtkGetStringMacro (ModuleName);
  vtkSetStringMacro (ModuleName);

  //Description:
  /// Implemented module initialization if needed
  virtual void Init() { };

  /// 
  /// Configures a module's help frame, with acknowledgment
  /// in a consistent manner
  virtual void BuildHelpAndAboutFrame ( vtkKWWidget *parent,
                                        const char *help,
                                        const char *about);

  virtual void CreateModuleEventBindings ( ) { };
  virtual void ReleaseModuleEventBindings ( ) { };

  virtual void SetActiveViewer(vtkSlicerViewerWidget * vtkNotUsed( activeViewer ) ){};
  
  /// 
  /// propagate events generated in logic layer to GUI
    virtual void ProcessLogicEvents ( vtkObject * /*caller*/,
      unsigned long /*event*/, void * /*callData*/ ) { };
    /// 
    /// alternative method to propagate events generated in GUI to logic / mrml
    virtual void ProcessGUIEvents ( vtkObject * /*caller*/, 
      unsigned long /*event*/, void * /*callData*/ ) { };
    
    /// 
    /// alternative method to propagate events generated in GUI to logic / mrml
    virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
      unsigned long /*event*/, void * /*callData*/ ) { };

  /// Overload in modules that observe events, used during Loadable Module
  /// discovery. CLIENT MUST DELETE!
  virtual vtkIntArray* NewObservableEvents() { return vtkIntArray::New(); };

 //BTX
  enum
    {
      ModuleSelectedEvent = 73300,
   };
//ETX
 protected:

  char *ModuleName;

    /// 
    /// This user interface panel is populated with the GUI's widgets,
    /// and is raised in Slicer's shared GUI panel when the module
    /// is selected for use.
    vtkKWUserInterfacePanel *UIPanel;
    
    /// 
    /// A module's UIPanel's HelpAndAboutFrame contains a notebook
    /// with a tab for module help, and a tab for acknowledgments
    /// and logos.
    vtkSlicerModuleCollapsibleFrame *HelpAndAboutFrame;
    vtkKWNotebook *HelpAndAboutNotebook;

    /// 
    /// HelpText contains the text describing the
    /// module's functionality and how to use it.
    vtkKWTextWithHyperlinksWithScrollbars *HelpText;

    /// 
    /// AboutText contains text describing
    /// the module's authors and sponsors.
    vtkKWTextWithHyperlinksWithScrollbars *AboutText;
    /// The LogoFrame is where a contributor's logos may
    /// be packed.
    vtkKWFrame *LogoFrame;

    /// 
    /// Describes whether the GUI has been built or not.
    bool Built;

    /// 
    /// If true, a user has visited this module
    /// so its "Enter()" method has been invoked.
    bool Visited;

    /// 
    /// Priority order for menus 
    unsigned short Index;

    /// 
    /// the category for menu grouping
    char *Category;
    


    /// constructor, destructor.
    vtkSlicerModuleGUI ( );
    virtual ~vtkSlicerModuleGUI ( );

    static vtkSlicerBaseAcknowledgementLogoIcons *AcknowledgementIcons;
    vtkSlicerApplicationGUI *ApplicationGUI;
//BTX
    vtkSmartPointer<vtkKWIcon> Logo;
//ETX
 private:
    vtkSlicerModuleGUI ( const vtkSlicerModuleGUI& ); /// Not implemented.
    void operator = ( const vtkSlicerModuleGUI& ); /// Not implemented.
};


#endif


