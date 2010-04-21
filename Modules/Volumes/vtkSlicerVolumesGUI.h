// .NAME vtkSlicerVolumesGUI 
// .SECTION Description
// Main Volumes GUI and mediator methods for slicer3. 


#ifndef __vtkSlicerVolumesGUI_h
#define __vtkSlicerVolumesGUI_h

#include "vtkVolumes.h"
#include "vtkSlicerModuleGUI.h"

#include "vtkSlicerVolumesLogic.h"
#include "vtkMRMLVolumeNode.h"

#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerVolumeDisplayWidget.h"
#include "vtkSlicerDiffusionWeightedVolumeDisplayWidget.h"
#include "vtkSlicerDiffusionTensorVolumeDisplayWidget.h"
#include "vtkSlicerScalarVolumeDisplayWidget.h"
#include "vtkSlicerVectorVolumeDisplayWidget.h"
#include "vtkSlicerLabelMapVolumeDisplayWidget.h"
#include "vtkSlicerDiffusionEditorWidget.h"

#include "vtkSlicerVolumeHeaderWidget.h"
#include "vtkITKArchetypeImageSeriesReader.h"


// Had to add this to force loading of these support libraries
// Only libraries with entries that will be dynamically called from TCL need to be instantiated
#include "vtkTcl.h"
extern "C" int Vtkslicervolumesmodulelogic_Init(Tcl_Interp *interp);

class vtkKWPushButton;
class vtkKWLoadSaveButtonWithLabel;
class vtkKWLoadSaveButton;
class vtkKWEntryWithLabel;
class vtkSlicerModuleCollapsibleFrame;
class vtkSlicerVolumeFileHeaderWidget;
class vtkSlicerVolumeFileHeaderWidget;

// Description:
// This class implements Slicer's Volumes GUI
//
class VTK_VOLUMES_EXPORT vtkSlicerVolumesGUI : public vtkSlicerModuleGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerVolumesGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerVolumesGUI, vtkSlicerModuleGUI );
    void PrintSelf (ostream& os, vtkIndent indent );
    
    // Description:
    // Get methods on class members (no Set methods required)
    vtkGetObjectMacro ( LoadVolumeButton, vtkKWLoadSaveButtonWithLabel );

    vtkGetObjectMacro ( Logic, vtkSlicerVolumesLogic );
    vtkGetObjectMacro ( VolumeNode, vtkMRMLVolumeNode );

    vtkGetStringMacro ( SelectedVolumeID);
    vtkSetStringMacro ( SelectedVolumeID );

    vtkGetObjectMacro (HelpFrame, vtkSlicerModuleCollapsibleFrame );
    vtkGetObjectMacro (LoadFrame, vtkSlicerModuleCollapsibleFrame );
    vtkGetObjectMacro (DisplayFrame, vtkSlicerModuleCollapsibleFrame );
    vtkGetObjectMacro (OptionFrame, vtkSlicerModuleCollapsibleFrame );

    vtkGetObjectMacro ( VolumeDisplayWidget, vtkSlicerVolumeDisplayWidget );
    vtkGetObjectMacro ( VolumeSelectorWidget, vtkSlicerNodeSelectorWidget );

    // Description:
    // API for setting VolumeNode, VolumeLogic and
    // for both setting and observing them.
    virtual void SetModuleLogic ( vtkSlicerVolumesLogic *logic )
        { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
    virtual void SetAndObserveModuleLogic ( vtkSlicerVolumesLogic *logic )
        { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }

    vtkGetObjectMacro(DiffusionEditorWidget, vtkSlicerDiffusionEditorWidget);

    // Description
    // Overloaded method for this modules' logic type
    virtual void SetModuleLogic( vtkSlicerLogic *logic )
        { this->SetModuleLogic (reinterpret_cast<vtkSlicerVolumesLogic*> (logic)); }
  
    // Description:
    // This method builds the Volumes module GUI
    virtual void BuildGUI ( );
    //BTX
    using vtkSlicerComponentGUI::BuildGUI; 
    //ETX

       // Description:
    // This method releases references and key-bindings,
    // and optionally removes observers.
    virtual void TearDownGUI ( );

        // Description:
    // Methods for adding module-specific key bindings and
    // removing them.
    virtual void CreateModuleEventBindings ( );
    virtual void ReleaseModuleEventBindings ( );
    
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
    // Describe behavior at module startup and exit.
    virtual void Enter ( vtkMRMLNode *node );
    virtual void Enter ( ) { this->Enter(NULL); };
    virtual void Exit ( );

    void CreateScalarDisplayWidget ( );
    void CreateVectorDisplayWidget ( );
    void CreateLabelMapDisplayWidget ( );
    void CreateDWIDisplayWidget ( );
    void CreateDTIDisplayWidget ( );

    //BTX
    // Description:
    // Return the MetaDataDictionary from the ITK layer
    std::vector<std::string> Tags;
    std::vector<std::string> TagValues;
    void CopyTagAndValues( vtkITKArchetypeImageSeriesReader* reader );
    void CheckLoadingOptions( int &LoadingOptions, bool &KeepAll );
    //ETX

    unsigned int GetNumberOfItemsInDictionary(); 
    bool HasKey( char* tag );
    const char* GetNthKey( unsigned int n );
    const char* GetNthValue( unsigned int n );
    const char* GetTagValue( char* tag );

    virtual void UpdateFramesFromMRML();
protected:
    vtkSlicerVolumesGUI ( );
    virtual ~vtkSlicerVolumesGUI ( );

    char *SelectedVolumeID;

    // Module logic and mrml pointers
    vtkSlicerVolumesLogic *Logic;
    vtkMRMLVolumeNode *VolumeNode;



    // Widgets for the Volumes module
    vtkKWLoadSaveButtonWithLabel *LoadVolumeButton;
    vtkKWEntryWithLabel  *NameEntry;
    vtkKWMenuButtonWithLabel *CenterImageMenu;
    vtkKWMenuButtonWithLabel *OrientImageMenu;
    vtkKWCheckButton *LabelMapCheckButton;
    vtkKWCheckButton *SingleFileCheckButton;
    vtkKWPushButton *ApplyButton;

    // load other images in the same directory
    vtkKWPushButton *LoadPreviousButton;
    vtkKWPushButton *LoadNextButton;
    vtkKWPushButton *CineButton;
    vtkKWCheckButton *KeepAllCheckButton;
    

    vtkSlicerModuleCollapsibleFrame *HelpFrame;
    vtkSlicerModuleCollapsibleFrame *LoadFrame;
    vtkSlicerModuleCollapsibleFrame *DisplayFrame;
    vtkSlicerModuleCollapsibleFrame *InfoFrame;
    vtkSlicerModuleCollapsibleFrame *OptionFrame;
    vtkSlicerModuleCollapsibleFrame *GradientFrame;

    vtkKWFrame *LabelMapDisplayFrame;
    vtkKWFrame *ScalarDisplayFrame;
    vtkKWFrame *VectorDisplayFrame;
    vtkKWFrame *DWIDisplayFrame;
    vtkKWFrame *DTIDisplayFrame;
    vtkKWFrame *VolumeDisplayFrame;

    vtkSlicerNodeSelectorWidget* VolumeSelectorWidget;
    vtkSlicerVolumeDisplayWidget *VolumeDisplayWidget;
    vtkSlicerLabelMapVolumeDisplayWidget *labelVDW;
    vtkSlicerScalarVolumeDisplayWidget *scalarVDW;
    vtkSlicerVectorVolumeDisplayWidget *vectorVDW;
    vtkSlicerDiffusionWeightedVolumeDisplayWidget *dwiVDW;
    vtkSlicerDiffusionTensorVolumeDisplayWidget *dtiVDW;
    vtkSlicerVolumeHeaderWidget *VolumeHeaderWidget;
    vtkSlicerVolumeFileHeaderWidget *VolumeFileHeaderWidget;    
    vtkSlicerDiffusionEditorWidget *DiffusionEditorWidget;

    vtkKWLabel *NACLabel;
    vtkKWLabel *NAMICLabel;
    vtkKWLabel *NCIGTLabel;
    vtkKWLabel *BIRNLabel;

    //BTX
    std::vector<std::string> AllFileNames;
    int IndexCurrentFile;
    //ETX

 private:
    vtkSlicerVolumesGUI ( const vtkSlicerVolumesGUI& ); // Not implemented.
    void operator = ( const vtkSlicerVolumesGUI& ); //Not implemented.
};

#endif
