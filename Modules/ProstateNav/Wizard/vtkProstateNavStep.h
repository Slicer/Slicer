#ifndef __vtkProstateNavStep_h
#define __vtkProstateNavStep_h

#include "vtkKWWizardStep.h"
#include "vtkProstateNav.h"
#include "vtkCommand.h"

#include "vtkObserverManager.h"

class vtkProstateNavGUI;
class vtkProstateNavLogic;
class vtkMRMLScene;


class VTK_PROSTATENAV_EXPORT vtkProstateNavStep : public vtkKWWizardStep
{
public:
  static vtkProstateNavStep *New();
  vtkTypeRevisionMacro(vtkProstateNavStep,vtkKWWizardStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description: 
  // Get/Set GUI
  vtkGetObjectMacro(GUI, vtkProstateNavGUI);
  vtkGetObjectMacro(Logic, vtkProstateNavLogic);
  virtual void SetGUI(vtkProstateNavGUI*);
  virtual void SetLogic(vtkProstateNavLogic*);

  void SetInMRMLCallbackFlag (int flag) {
    this->InMRMLCallbackFlag = flag;
  }
  vtkGetMacro(InMRMLCallbackFlag, int);
  void SetInGUICallbackFlag (int flag) {
    this->InGUICallbackFlag = flag;
    }
  vtkGetMacro(InGUICallbackFlag, int);

  void SetAndObserveMRMLScene ( vtkMRMLScene *mrml )
    {
    vtkMRMLScene *oldValue = this->MRMLScene;
    this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &this->MRMLScene), (vtkObject*)mrml );
    if ( oldValue != this->MRMLScene )
      {
      this->InvokeEvent (vtkCommand::ModifiedEvent);
      }
    }

  void SetTitleBackgroundColor (double r, double g, double b) {
    this->TitleBackgroundColor[0] = r;
    this->TitleBackgroundColor[1] = g;
    this->TitleBackgroundColor[2] = b;
  };

  // Description:
  // Reimplement the superclass's method (see vtkKWWizardStep).
  virtual void HideUserInterface();
  virtual void Validate();
  virtual int CanGoToSelf();
  virtual void ShowUserInterface();
  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData) {};
  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData) {};


protected:
  vtkProstateNavStep();
  ~vtkProstateNavStep();

  static void GUICallback(vtkObject *caller,
                          unsigned long eid, void *clientData, void *callData );

  static void MRMLCallback(vtkObject *caller,
                           unsigned long eid, void *clientData, void *callData );
  
  double TitleBackgroundColor[3];

  int InGUICallbackFlag;
  int InMRMLCallbackFlag;

  vtkProstateNavGUI   *GUI;
  vtkProstateNavLogic *Logic;
  vtkMRMLScene        *MRMLScene;

  vtkCallbackCommand *GUICallbackCommand;
  vtkCallbackCommand *MRMLCallbackCommand;

  vtkObserverManager *MRMLObserverManager;


private:
  vtkProstateNavStep(const vtkProstateNavStep&);
  void operator=(const vtkProstateNavStep&);

};

#endif
