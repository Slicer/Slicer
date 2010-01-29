///  vtkSlicerDataGUI
/// 
/// Main Data GUI and mediator methods for slicer3.

#ifndef __vtkSlicerDataGUI_h
#define __vtkSlicerDataGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerMRMLTreeWidget.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerFoundationIcons.h"
#include "vtkMRMLModelNode.h"

#include "vtkKWLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWTopLevel.h"

//#include "vtkSlicerDataLogic.h"

/// Description:
/// This class implements Slicer's Data GUI
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerDataGUI : public vtkSlicerModuleGUI
{
 public:
  /// 
  /// Usual vtk class functions
  static vtkSlicerDataGUI* New (  );
  vtkTypeRevisionMacro ( vtkSlicerDataGUI, vtkSlicerModuleGUI );
  void PrintSelf ( ostream& os, vtkIndent indent );
    
  /// 
  /// Get methods on class members (no Set methods required.)
  //vtkGetObjectMacro ( Logic, vtkSlicerDataLogic);
  //vtkGetObjectMacro ( MRMLNode, vtkMRMLNode );

  /// 
  /// API for setting MRMLNode, Logic and
  /// for both setting and observing them.
  /*
  /// classes not yet defined!
  void SetMRMLNode ( vtkMRMLNode *node )
  { this->SetMRML ( vtkObjectPointer( &this->MRMLNode), node ); }
  void SetAndObserveMRMLNode ( vtkMRMLNode *node )
  { this->SetAndObserveMRML ( vtkObjectPointer( &this->MRMLNode), node ); }

  void SetModuleLogic ( vtkSlicerDataLogic *logic )
  { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ) }
  void SetAndObserveModuleLogic ( vtkSlicerDataLogic *logic )
  { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ) }
  */
    
  /// 
  /// These Get methods provide the API for the load scene pushbuttons and frame
  vtkGetObjectMacro (SceneInformationButton, vtkKWPushButton );
  vtkGetObjectMacro (SceneInformationFrame, vtkKWFrame );
  vtkGetObjectMacro (LoadSceneButton, vtkKWPushButton );

  /// 
  /// These Get methods provide the API for the import scene pushbuttons and frame
  vtkGetObjectMacro (ImportSceneInformationButton, vtkKWPushButton );
  vtkGetObjectMacro (ImportSceneInformationFrame, vtkKWFrame );
  vtkGetObjectMacro (ImportSceneButton, vtkKWPushButton );

  /// 
  /// These Get methods provide the API for the load volume  pushbuttons and frame
  vtkGetObjectMacro (LoadVolumeButton, vtkKWPushButton );
  vtkGetObjectMacro (VolumeInformationButton, vtkKWPushButton );
  vtkGetObjectMacro (VolumeInformationFrame, vtkKWFrame );

  /// 
  /// These Get methods provide the API for the load dicom volume pushbuttons and frame
  vtkGetObjectMacro (LoadDicomVolumeButton, vtkKWPushButton );
  vtkGetObjectMacro ( DicomInformationButton, vtkKWPushButton );
  vtkGetObjectMacro ( DicomInformationFrame, vtkKWFrame );

  /// 
  /// These Get methods provide the API for the load model pushbuttons and frame
  vtkGetObjectMacro (ModelInformationButton, vtkKWPushButton );
  vtkGetObjectMacro (ModelInformationFrame, vtkKWFrame );
  vtkGetObjectMacro (LoadModelButton, vtkKWPushButton );

  /// 
  /// These Get methods provide the API for the load model pushbuttons and frame
  vtkGetObjectMacro (OverlayInformationButton, vtkKWPushButton );
  vtkGetObjectMacro (OverlayInformationFrame, vtkKWFrame );
  vtkGetObjectMacro (LoadOverlayButton, vtkKWPushButton );

  /// 
  /// These Get methods provide the API for the load transform pushbuttons and frame
  vtkGetObjectMacro (TransformInformationButton, vtkKWPushButton );
  vtkGetObjectMacro (TransformInformationFrame, vtkKWFrame );
  vtkGetObjectMacro (LoadTransformButton, vtkKWPushButton );

  /// 
  /// These Get methods provide the API for the load fiducial list pushbuttons and frame
  vtkGetObjectMacro (FiducialsInformationButton, vtkKWPushButton );
  vtkGetObjectMacro (FiducialsInformationFrame, vtkKWFrame );
  vtkGetObjectMacro (LoadFiducialsButton, vtkKWPushButton );

  /// 
  /// These Get methods provide the API for the load directory pushbuttons and frame
  vtkGetObjectMacro (DirectoryInformationButton, vtkKWPushButton );
  vtkGetObjectMacro (DirectoryInformationFrame, vtkKWFrame );
  vtkGetObjectMacro (LoadDirectoryButton, vtkKWPushButton );

  /// 
  /// These Get methods provide the API for the load any data pushbuttons and frame
  vtkGetObjectMacro (DataInformationButton, vtkKWPushButton );
  vtkGetObjectMacro (DataInformationFrame, vtkKWFrame );
  vtkGetObjectMacro (LoadDataButton, vtkKWPushButton );

  /// 
  /// These Get methods provide the API for the load color LUT pushbuttons and frame
  vtkGetObjectMacro (ColorLUTInformationButton, vtkKWPushButton );
  vtkGetObjectMacro (ColorLUTInformationFrame, vtkKWFrame );
  vtkGetObjectMacro (LoadColorLUTButton, vtkKWPushButton );  

  /// 
  /// These Get methods provide the API for the load fiber bundle pushbuttons and frame
  vtkGetObjectMacro (FiberBundleInformationButton, vtkKWPushButton );
  vtkGetObjectMacro (FiberBundleInformationFrame, vtkKWFrame );
  vtkGetObjectMacro (LoadFiberBundleButton, vtkKWPushButton );

  /// 
  /// Get/Set on the last directory the load filebrowsers open to.
  vtkGetStringMacro (LoadDirectory );
  vtkSetStringMacro (LoadDirectory );

  /// 
  /// Methods for adding module-specific key bindings and
  /// removing them.
  virtual void CreateModuleEventBindings ( );
  virtual void ReleaseModuleEventBindings ( );

  /// 
  /// This method builds the Data module's GUI
  virtual void BuildGUI ( void );
  virtual void BuildGUI ( vtkKWFrame * f ) { this->Superclass::BuildGUI(f); }
  virtual void BuildGUI ( vtkKWFrame * f, double * bgColor ) { this->Superclass::BuildGUI(f,bgColor); }

  /// 
  /// Theses methods builds the Load panels
  virtual void BuildScenePanel ( vtkKWFrame *parent );
  virtual void BuildGeneralPanel ( vtkKWFrame *parent );
  virtual void BuildVolumePanel ( vtkKWFrame *parent );
  virtual void BuildModelPanel ( vtkKWFrame *parent );
  virtual void BuildOverlayPanel (vtkKWFrame *parent );
  virtual void BuildTransformPanel ( vtkKWFrame *parent );
  virtual void BuildFiducialPanel ( vtkKWFrame *parent );
  virtual void BuildColorPanel ( vtkKWFrame *parent );
  virtual void BuildFiberBundlePanel ( vtkKWFrame *parent );

  /// 
  /// These methods pack and unpack the load information text.
  virtual void PackInformationFrame ( vtkKWFrame *f, vtkKWPushButton *b);
  virtual void UnpackInformationFrame ( vtkKWFrame *f, vtkKWPushButton *b);

  /// Descripgion:
  /// This method releases references and key-bindings,
  /// and optionally removes observers.
  virtual void TearDownGUI ( );

  /// 
  /// Methods to take apart pop-up windows and remove observers on widgets.
  virtual void WithdrawAddModelWindow();
  virtual void WithdrawAddScalarOverlayWindow();

  /// 
  /// Methods to raise pop-up windows and put observers on widgets.
  virtual void RaiseAddModelWindow();
  virtual void RaiseAddScalarOverlayWindow();
    
  /// 
  /// Methods to destroy pop-up windows and put observers on widgets.
  virtual void DestroyAddModelWindow();
  virtual void DestroyAddScalarOverlayWindow();

  /// 
  /// Add/Remove observers on widgets in the GUI
  virtual void AddGUIObservers ( );
  virtual void RemoveGUIObservers ( );

  /// 
  /// Class's mediator methods for processing events invoked by
  /// either the Logic, MRML or GUI.
  virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
    
  /// 
  /// Describe the behavior at module enter and exit.
  virtual void Enter ( );
  //BTX
  using vtkSlicerComponentGUI::Enter;
  //ETX
  virtual void Exit ( );
    
 protected:
  vtkSlicerDataGUI ( );
  virtual ~vtkSlicerDataGUI ( );

  vtkKWPushButton *LoadSceneButton;
  vtkKWPushButton *SceneInformationButton;
  vtkKWFrame *SceneInformationFrame;
  bool SceneInformationOpen;

  vtkKWPushButton *ImportSceneButton;
  vtkKWPushButton *ImportSceneInformationButton;
  vtkKWFrame *ImportSceneInformationFrame;
  bool ImportSceneInformationOpen;

  vtkKWPushButton *LoadVolumeButton;
  vtkKWPushButton *VolumeInformationButton;
  vtkKWFrame *VolumeInformationFrame;
  bool VolumeInformationOpen;
  
  vtkKWPushButton *LoadDicomVolumeButton;
  vtkKWPushButton *DicomInformationButton;
  vtkKWFrame *DicomInformationFrame;
  bool DicomInformationOpen;

  vtkKWPushButton *ModelInformationButton;
  vtkKWFrame *ModelInformationFrame;
  vtkKWPushButton *LoadModelButton;
  bool ModelInformationOpen;

  vtkKWPushButton *OverlayInformationButton;
  vtkKWFrame *OverlayInformationFrame;
  vtkKWPushButton *LoadOverlayButton;
  bool OverlayInformationOpen;

  vtkKWPushButton *TransformInformationButton;
  vtkKWFrame *TransformInformationFrame;
  vtkKWPushButton *LoadTransformButton;
  bool TransformInformationOpen;

  vtkKWPushButton *FiducialsInformationButton;
  vtkKWFrame *FiducialsInformationFrame;
  vtkKWPushButton *LoadFiducialsButton;
  bool FiducialsInformationOpen;

  vtkKWPushButton *ColorLUTInformationButton;
  vtkKWFrame *ColorLUTInformationFrame;
  vtkKWPushButton *LoadColorLUTButton;
  bool ColorLUTInformationOpen;

  vtkKWPushButton *FiberBundleInformationButton;
  vtkKWFrame *FiberBundleInformationFrame;
  vtkKWPushButton *LoadFiberBundleButton;
  bool FiberBundleInformationOpen;
  
  vtkKWPushButton *DirectoryInformationButton;
  vtkKWFrame *DirectoryInformationFrame;
  vtkKWPushButton *LoadDirectoryButton;
  bool DirectoryInformationOpen;

  vtkKWPushButton *DataInformationButton;
  vtkKWFrame *DataInformationFrame;
  vtkKWPushButton *LoadDataButton;
  bool DataInformationOpen;

  char *LoadDirectory;
    
  /// Widgets for AddModel functionality
  vtkKWLoadSaveButton *AddModelDialogButton;
  vtkKWLoadSaveButton *AddModelDirectoryDialogButton;
  vtkKWTopLevel *AddModelWindow;
  
  /// Widgets for AddScalarOverlay functionality
  vtkSlicerNodeSelectorWidget *ModelSelector;
  vtkKWLoadSaveButton *AddOverlayDialogButton;
  vtkKWTopLevel *AddOverlayWindow;
  vtkMRMLModelNode *SelectedModelNode;
  
  /// Widgets for AddFiducialList functionality
  vtkKWLoadSaveDialog *AddFiducialDialog;
  
  /// Widgets for AddColorLUT functionality
  vtkKWLoadSaveDialog *AddColorLUTDialog;
  
  /// Widgets for AddDTIFiberBundle functionality
  vtkKWLoadSaveDialog *AddFiberBundleDialog;

  /// Widgets for MRMLScene
  vtkSlicerMRMLTreeWidget *MRMLTreeWidget;
  /// Widgets for acknowledgement.
  vtkKWLabel *NACLabel;
  vtkKWLabel *NAMICLabel;
  vtkKWLabel *NCIGTLabel;
  vtkKWLabel *BIRNLabel;
  
 private:
  vtkSlicerDataGUI ( const vtkSlicerDataGUI& ); /// Not implemented.
  void operator = ( const vtkSlicerDataGUI& ); //Not implemented.
};


#endif
