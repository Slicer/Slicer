/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkIA_FEMeshGUI.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkIA_FEMeshGUI_h
#define __vtkIA_FEMeshGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"

#include "vtkMRMLScene.h"
#include "vtkIA_FEMeshLogic.h"

#include "vtkTcl.h"

// had to add this to force loading of these support libraries now that the module is a loadable
// module.  Only libraries with entries that will be dynamically called from TCL need to be instantiated
extern "C" int Mimxcommon_Init(Tcl_Interp *interp);
extern "C" int Buildingblock_Init(Tcl_Interp *interp);

class vtkSlicerSliceWidget;
class vtkKWFrame;
class vtkKWScaleWithEntry;
class vtkKWPushButton;
class vtkSlicerNodeSelectorWidget;

class vtkKWMimxViewProperties;
class vtkLinkedListWrapperTree;
class vtkKWLoadSaveDialog;
class vtkKWMimxMainUserInterfacePanel;
class vtkKWMimxMainWindow;
class vtkMRMLScene;


// added for UIowa Mimx integration.  Using MRML-based notebook instead of
// local list-based notebook.

//class vtkKWMimxMainNotebook;
class vtkIA_FEMeshMRMLNotebook;

class VTK_IA_FEMesh_EXPORT vtkIA_FEMeshGUI : public vtkSlicerModuleGUI
{
  public:
  static vtkIA_FEMeshGUI *New();
  vtkTypeMacro(vtkIA_FEMeshGUI,vtkSlicerModuleGUI);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // set a specific category for this module.  All Meshing modules will be together in their own category.
  const char *GetCategory() const {return "Wizards";}

  virtual void Register(vtkObject *o) { Superclass::Register(o); };
  //BTX
  using vtkObjectBase::Register; 
  //ETX
  virtual void UnRegister(vtkObject *o) { Superclass::UnRegister(o); };
  //BTX
  using vtkObjectBase::UnRegister; 
  //ETX


   // Description: Get/Set MRML node
  vtkGetObjectMacro (Logic, vtkIA_FEMeshLogic);
  vtkSetObjectMacro (Logic, vtkIA_FEMeshLogic);

  virtual void BuildGUI ( );
  //BTX
  using vtkSlicerComponentGUI::BuildGUI; 
  //ETX
  virtual void TearDownGUI ( );

  virtual void AddGUIObservers ( );
  virtual void RemoveGUIObservers ( );
  virtual void AddMRMLObservers ( );
  virtual void RemoveMRMLObservers ( );


  virtual void ProcessLogicEvents(vtkObject *vtkNotUsed(caller),
                                  unsigned long vtkNotUsed(event),
                                  void *vtkNotUsed(callData)){};
  virtual void ProcessGUIEvents( vtkObject *caller, unsigned long event,
                                  void *callData );
  virtual void ProcessMRMLEvents( vtkObject *caller, unsigned long event,
                                   void *callData );
  // Description:
  // Describe behavior at module startup and exit.
  virtual void Enter ( );
  //BTX
  using vtkSlicerComponentGUI::Enter; 
  //ETX
  virtual void Exit ( );

  virtual void SetActiveViewer(vtkSlicerViewerWidget *activeViewer);

//  vtkGetObjectMacro(ViewProperties, vtkKWMimxViewProperties);
//  vtkGetObjectMacro(MainUserInterfacePanel, vtkKWMimxMainUserInterfacePanel);

  // Added for loadable modules
  // Description:
  // Set the logic pointer from parent class pointer.
  // Overloads implementation in vtkSlicerModulesGUI
  // to allow loadable modules.
  virtual void SetModuleLogic ( vtkSlicerLogic *logic )
  {
    this->SetLogic(reinterpret_cast<vtkIA_FEMeshLogic*> (logic));
  }

protected:
  vtkIA_FEMeshGUI();
  virtual ~vtkIA_FEMeshGUI();

  vtkIA_FEMeshGUI(const vtkIA_FEMeshGUI&);
  void operator=(const vtkIA_FEMeshGUI&);

  void InitializeSupportingLibraries();

//  vtkIA_FEMeshMRMLNotebook *MimxMainNotebook;
//  //vtkKWMimxMainNotebook *MimxMainNotebook;
//  vtkKWMimxViewProperties *ViewProperties;
//  vtkLinkedListWrapperTree *DoUndoTree;
//  vtkKWLoadSaveDialog *LoadSaveDialog;
//  vtkKWMimxMainUserInterfacePanel *MainUserInterfacePanel;
//  vtkKWMimxDisplayPropertiesGroup *DisplayPropertyDialog;
//  vtkKWPushButton* ApplyButton;
  // callback to create the separate UI
  //void BuildSeparateFEMeshGUI();
  
  vtkKWMimxMainWindow *MeshingUI;
  vtkMRMLScene *StoredMRMLState;
  vtkIA_FEMeshLogic *Logic;
  
  // save variables read from the MRML scene when entering the module 
  int SavedBoxState;
  int SavedAxisLabelState;
  int SavedLayoutEnumeration;
  double SavedBackgroundColor[3];
  bool FirstEntryToModule;

};

#endif

