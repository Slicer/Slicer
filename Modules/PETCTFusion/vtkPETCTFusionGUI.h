// .NAME vtkPETCTFusionGUI
// .SECTION Description
// Slicer Welcome and Overview


#ifndef __vtkPETCTFusionGUI_h
#define __vtkPETCTFusionGUI_h

#include "vtkPETCTFusionWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkMRMLPETCTFusionNode.h"
#include "vtkMRMLColorNode.h"
#include "vtkPETCTFusionLogic.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include <string>

class vtkKWFrame;
class vtkKWCheckButton;
class vtkSlicerNodeSelectorWidget;
class vtkKWRadioButton;
class vtkKWRadioButtonSet;
class vtkKWEntry;
class vtkKWPushButton;
class vtkKWLabel;
class vtkKWRange;
class vtkKWMenuButton;
class vtkSlicerPopUpHelpWidget;
class vtkKWMultiColumnList;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWRadioButtonSetWithLabel;

class VTK_PETCTFUSION_EXPORT vtkPETCTFusionGUI : public vtkSlicerModuleGUI
{
  public:
  static vtkPETCTFusionGUI *New();
  vtkTypeRevisionMacro(vtkPETCTFusionGUI,vtkSlicerModuleGUI);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the logic pointer from parent class pointer.
  // Overloads implementation in vtkSlicerModulesGUI
  // to allow loadable modules.
  virtual void SetModuleLogic ( vtkSlicerLogic *logic )
  {
  this->SetLogic(reinterpret_cast<vtkPETCTFusionLogic*> (logic)); 
  };

  // Description: 
  // Get the categorization of the module.
  const char *GetCategory() const
  { return "Quantification"; }

  // Description:
  // Get/Set on Module Logic
  vtkGetObjectMacro (Logic, vtkPETCTFusionLogic);
  vtkSetObjectMacro (Logic, vtkPETCTFusionLogic);

   // Description:
  // Get/Set on PETCTFusion MRML node
  vtkGetObjectMacro (PETCTFusionNode, vtkMRMLPETCTFusionNode );
  vtkSetObjectMacro (PETCTFusionNode, vtkMRMLPETCTFusionNode );
  
  // Description:
  // Get/Set on all module widgets
  vtkGetObjectMacro(CTSelector, vtkSlicerNodeSelectorWidget);
  vtkGetObjectMacro(PETSelector, vtkSlicerNodeSelectorWidget);
  vtkGetObjectMacro(MaskSelector, vtkSlicerNodeSelectorWidget);
  vtkGetObjectMacro(VolumeRenderCheckbox, vtkKWCheckButton);
  vtkGetObjectMacro(TissueConcentrationEntry, vtkKWEntry);
  vtkGetObjectMacro(InjectedDoseEntry, vtkKWEntry);
  vtkGetObjectMacro(PatientWeightEntry, vtkKWEntry);
  vtkGetObjectMacro(ComputeButton, vtkKWPushButton);
  vtkGetObjectMacro(SaveButton, vtkKWPushButton);
  vtkGetObjectMacro (GetFromDICOMButton, vtkKWPushButton );
  vtkGetObjectMacro(HelpButton, vtkSlicerPopUpHelpWidget);
  vtkGetObjectMacro (PETRange, vtkKWRange );
  vtkGetObjectMacro (CTRange, vtkKWRange );  
  vtkGetObjectMacro (DoseUnitsMenuButton, vtkKWMenuButton );
  vtkGetObjectMacro (TissueUnitsMenuButton, vtkKWMenuButton );
  vtkGetObjectMacro (WeightUnitsMenuButton, vtkKWMenuButton );
  vtkGetObjectMacro ( ColorSet, vtkKWRadioButtonSetWithLabel );

  vtkGetObjectMacro ( ClearPlotArrayButton, vtkKWPushButton );
  vtkGetObjectMacro ( AddToPlotArrayButton, vtkKWPushButton );
  vtkGetObjectMacro ( ShowPlotButton, vtkKWPushButton );
  vtkGetObjectMacro ( SavePlotArrayButton, vtkKWPushButton );

  // Description:
  // Methods for building the module GUI (main BuildGUI method
  // and methods it calls for building each GUI sub-panel.
  virtual void BuildGUI ( );
  //BTX
  using vtkSlicerComponentGUI::BuildGUI;
  //ETX
  virtual void BuildDisplayFrame(vtkKWWidget *parent);
  virtual void BuildFusionFrame(vtkKWWidget *parent);
  virtual void BuildAnalysisFrame(vtkKWWidget *parent);
  virtual void BuildReportFrame( vtkKWWidget *parent);

  // Description:
  // This method is called when GUI is created, when a new PET volume
  // is selected. It clears the nuclear medicine DICOM tags, and the results.
  virtual void InitializeGUI ( );

  // Description:
  // This method is called when any other MRML event triggers a need
  // to update the GUI.  When updateDICOMevent = 1, the panel
  // that displays SUV attributes collected from the DICOM header
  // is also cleared and updated.
  virtual void UpdateGUIFromMRML ( int updateDICOMevent );

  // Description:
  // Method is called when a new PET volume is selected;
  // It updates the GUI panel to show what LUT
  // is used to colorize the volume.
  virtual void UpdateColorRadioButtonsFromMRML ();

  // Description:
  // Removes all GUI observers. Called when module exits.
  virtual void TearDownGUI ( );

  // Description:
  // Defines MRML Scene events that this module observes.
  vtkIntArray *NewObservableEvents();

  // Description:
  // Applies the selected LUT to the Pet Volume.
  virtual void ColorizePETVolume(int type);

  // Description:
  // These methods are used to interactively update
  // the window/level display of the PET volume as
  // user adjusts the PETRange widget.
  virtual void ProcessPETRangeCommand (double min, double max);
  virtual void ProcessPETRangeStartCommand(double min, double max);
  virtual void ProcessPETRangeStopCommand ( double min, double max);


  // Description:
  // These methods are used to interactively update
  // the window/level display of the CT volume as
  // user adjusts the CTRange widget.
  virtual void ProcessCTRangeCommand (double min, double max);
  virtual void ProcessCTRangeStartCommand(double min, double max);
  virtual void ProcessCTRangeStopCommand ( double min, double max);

  // Description:
  // Add observers to GUI widgets
  virtual void AddGUIObservers ( );

  // Description:
  // Remove observers to GUI widgets
  virtual void RemoveGUIObservers ( );

  // Description:
  // Methods to enable/disable volume rendering...
  // NOT currently implemented.
  virtual void EnablePETCTVolumeRendering();
  virtual void DisablePETCTVolumeRendering();

  // Description:
  // Process observed events generated by GUI widgets
  virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event,
                                  void *callData );
  // Description:
  // Process observed events generated by the MRML scene.
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event,
                                  void *callData );

  // Description:
  // Describe behavior at module startup and exit.
  virtual void Enter ( );
  //BTX
  using vtkSlicerComponentGUI::Enter;
  //ETX
  virtual void Exit ( );
  virtual void Init ( );

  // Description:
  // Clears the GUI panel containing SUV
  // attributes parsed from the PET volume's
  // DICOM header.
  virtual void ClearDICOMInformation ( );

  // Description:
  // Clears the GUI panel that permits
  // manual specification of SUV attributes.
  virtual void ResetManualEntryGUI ( );

  // Description:
  // updates the panel containing SUV attributes
  // parsed from the dicom header.
  virtual void UpdateDICOMPanel ( );

  // Description:
  // updates the display of colorized pet based on changes to
  // mrml color range. Changes window, level of PET
  // volume's display node.
  virtual void UpdatePETDisplayFromMRML ( );

  // Description:
  // updates the display of colorized pet based on changes to
  // mrml color range. Changes window, level of CT
  // volume's display node.
  virtual void UpdateCTDisplayFromMRML ( );

  // Description:
  // When a new CT volume is selected, this method
  // changes its color LUT to Greyscale, if that LUT
  // is not already selected, and references the LUT in
  // MRML.
  virtual void ApplyDefaultCTLUT();

  // Description:
  // When a new PET volume is selected, this method
  // changes its color LUT to PETheat, if a valid PETLUT
  // is not already selected, and references the LUT in
  // MRML.
  virtual void ApplyDefaultPETLUT();
  
  // Description:
  // Clears the Results GUI panel of
  // all SUV computation results.
  virtual void ClearResultsTable();

  // Description:
  // Repopulates GUI panel with SUV statistics
  // stored in PETCTFusionNode.
  virtual void UpdateResultsTableFromMRML();
  
  // Description:
  // Show plot of timeseries SUV measurements
  virtual void RaisePlot();

  // Description:
  // Convenience method that returns a 
  // vtkMRMLColorTableNodeID given its type.
  const char *GetCTColorTableNodeIDByType (int type);
  // Description:
  // Convenience method that returns a
  // vtkPETProceduralColorNode ID given its type.
  const char *GetPETColorNodeIDByType(int type);

  

  // Description:
  // Sets the max and min values in the PET dataset.
  // Method invoked when NO volume is selected.
  virtual void InitializePETMinAndMax();
  // Description:
  // Sets the max and min values in the CT dataset.
  // Method invoked when NO volume is selected.
  virtual void InitializeCTMinAndMax();




  // Description:
  // Gets the selected PET volume, and updates
  // the window/level range from the volume's display node
  virtual void UpdatePETRangeFromMRML();
  // Description:
  // Gets the selected CT volume, and updates
  // the window/level range from the volume's display node
  virtual void UpdateCTRangeFromMRML();



  // Description:
  // Method updates node's color range
  // when user adjusts the module's win/lev
  // range widget.
  virtual void UpdateNodePETColorRange ( );
  // Description:
  // Method updates node's color range
  // when user adjusts the module's win/lev
  // range widget.
  virtual void UpdateNodeCTColorRange ( );



  // Description:
  // Method is called from method Enter()
  // If PET, CT, or Mask volumes have had 
  // display adjusted in the volumes
  // module, this method tracks those changes.
  virtual void UpdateFusionDisplayFromMRML();

  

  // Descriptoin:
  // Scales the PET volume's color lut over a
  // range of selected values.
  virtual void ScalePETColormap(double min, double max);
  // Descriptoin:
  // Scales the CT volume's color lut over a
  // range of selected values.
  virtual void ScaleCTColormap(double min, double max);


protected:
  vtkPETCTFusionGUI();
  ~vtkPETCTFusionGUI();
  vtkPETCTFusionGUI(const vtkPETCTFusionGUI&);
  void operator=(const vtkPETCTFusionGUI&);

  // Description:
  // updates status text in the slicer window.
  void SetStatusText( const char *txt);

  vtkPETCTFusionLogic *Logic;
  vtkMRMLPETCTFusionNode *PETCTFusionNode;

  //---
  //--- widgets
  //---
  
  // Description:
  // Ruler GUI elements
  vtkSlicerNodeSelectorWidget* CTSelector;
  vtkSlicerNodeSelectorWidget* PETSelector;
  vtkSlicerNodeSelectorWidget* MaskSelector;
  vtkKWCheckButton *VolumeRenderCheckbox;
  vtkKWEntry *TissueConcentrationEntry;
  vtkKWEntry *InjectedDoseEntry;
  vtkKWEntry *PatientWeightEntry;
  vtkKWPushButton *ComputeButton;
  vtkKWPushButton *GetFromDICOMButton;
  vtkKWPushButton *SaveButton;
  vtkSlicerPopUpHelpWidget *HelpButton;
  vtkKWMenuButton *DoseUnitsMenuButton;
  vtkKWMenuButton *WeightUnitsMenuButton;
  vtkKWMenuButton *TissueUnitsMenuButton;  
  vtkKWRange *PETRange;
  vtkKWRange *CTRange;
  vtkKWRadioButtonSetWithLabel *ColorSet;
  vtkSlicerNodeSelectorWidget *PlotSelector;

  vtkKWPushButton *AddToPlotArrayButton;
  vtkKWPushButton *ShowPlotButton;
  vtkKWPushButton *SavePlotArrayButton;
  vtkKWPushButton *ClearPlotArrayButton;

  vtkMRMLDoubleArrayNode *SUVmaxArray;
  vtkMRMLDoubleArrayNode *SUVmeanArray;

  bool Raised;
  
  //--- for internal use only.
  vtkKWLabel *PatientWeightLabel;
  vtkKWLabel *InjectedDoseLabel;
  vtkKWLabel *PatientNameLabel;
  vtkKWLabel *StudyDateLabel;
  vtkKWMultiColumnList *ResultList;
  vtkKWMultiColumnListWithScrollbars *ResultListWithScrollbars;

  
  double PETMin;
  double PETMax;
  int UpdatingLUT;
  int UpdatingGUI;

};

#endif

