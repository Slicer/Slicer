#ifndef __vtkBrainlabModuleGUI_h
#define __vtkBrainlabModuleGUI_h

#include "vtkSlicerModuleGUI.h"
#include "vtkBrainlabModuleLogic.h"
#include "vtkBrainlabModule.h"

class vtkBrainlabModuleMRMLManager;
class vtkBrainlabModuleLoadingDataStep;
class vtkBrainlabModuleNavigationStep;
class vtkBrainlabModuleConnectionStep;
class vtkMRMLNode;
class vtkKWWizardWidget;
 
class VTK_BRAINLABMODULE_EXPORT vtkBrainlabModuleGUI : public vtkSlicerModuleGUI
{
public:
  static vtkBrainlabModuleGUI *New();
  vtkTypeMacro(vtkBrainlabModuleGUI,vtkSlicerModuleGUI);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: 
  // Get the categorization of the module.
  const char *GetCategory() const { return "IGT"; }

  // Description: 
  // Get/Set logic node
  vtkGetObjectMacro(Logic, vtkBrainlabModuleLogic);
  vtkSetObjectMacro(Logic, vtkBrainlabModuleLogic);

  /// Implement setter for vtkLogic* pointer
  // to allow access to logic's MRML Manager from GUI
  void SetModuleLogic ( vtkSlicerLogic *logic );
 
  // Get/Set mrml manager node
  vtkGetObjectMacro(MRMLManager, vtkBrainlabModuleMRMLManager);
  virtual void SetMRMLManager(vtkBrainlabModuleMRMLManager*);

  // Description: 
  // Get/Set MRML node
  vtkGetObjectMacro(Node, vtkMRMLNode);
  virtual void SetNode(vtkMRMLNode*);

  // Description:
  // Create widgets
  virtual void BuildGUI();

  // Description:
  // Initialize module
  virtual void Init();

  // Description:
  // Delete Widgets
  virtual void TearDownGUI();

  // Description:
  // Add observers to GUI widgets
  virtual void AddGUIObservers();
  
  // Description:
  // Remove observers to GUI widgets
  //virtual void RemoveGUIObservers();

  // Description:
  // Remove observers to MRML node
  //virtual void RemoveMRMLNodeObservers();

  // Description:
  // Remove observers to Logic
  virtual void RemoveLogicObservers();

  // Description:
  // Describe behavior at module startup and exit.
  virtual void Enter();
  virtual void Exit();

  // Description: The name of the Module - this is used to 
  // construct the proc invocations
  vtkGetStringMacro(ModuleName);
  vtkSetStringMacro(ModuleName);

 // Description: 
  // Get wizard widget
  vtkGetObjectMacro(WizardWidget, vtkKWWizardWidget);

  // Description: set an observer by number (work around
  // limitation in kwwidgets tcl wrapping)
  unsigned long AddObserverByNumber(vtkObject *observee, unsigned long event);

protected:
  vtkBrainlabModuleGUI();
  ~vtkBrainlabModuleGUI();
  vtkBrainlabModuleGUI(const vtkBrainlabModuleGUI&);
  void operator=(const vtkBrainlabModuleGUI&);
  void BuildGUIForHelpFrame();

  // Description:
  // Updates GUI widgets based on parameters values in MRML node
  void UpdateGUI();

  // Description:
  // Updates parameters values in MRML node based on GUI widgets 
  void UpdateMRML();
  
  vtkBrainlabModuleLogic                *Logic;
  vtkBrainlabModuleMRMLManager          *MRMLManager;
  vtkMRMLNode                           *Node;
  
  char *ModuleName;

  // Description:
  // The wizard widget and steps
  vtkBrainlabModuleConnectionStep       *ConnectionStep;
  vtkBrainlabModuleLoadingDataStep      *LoadingDataStep;
  vtkBrainlabModuleNavigationStep       *NavigationStep; 
  vtkKWWizardWidget                     *WizardWidget;

};

#endif
