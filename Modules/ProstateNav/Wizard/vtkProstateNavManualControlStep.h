#ifndef __vtkProstateNavManualControlStep_h
#define __vtkProstateNavManualControlStep_h

#include "vtkProstateNavStep.h"

class vtkKWFrame;
class vtkKWScaleWithEntry;

class VTK_PROSTATENAV_EXPORT vtkProstateNavManualControlStep : public vtkProstateNavStep
{
public:
  static vtkProstateNavManualControlStep *New();
  vtkTypeRevisionMacro(vtkProstateNavManualControlStep,vtkProstateNavStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ShowUserInterface();
  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);

protected:
  vtkProstateNavManualControlStep();
  ~vtkProstateNavManualControlStep();

  // GUI Widgets
  vtkKWFrame *MainFrame;
  vtkKWFrame *ControlFrame;

  vtkKWScaleWithEntry* PRScale;
  vtkKWScaleWithEntry* PAScale;
  vtkKWScaleWithEntry* PSScale;
  vtkKWScaleWithEntry* NRScale;
  vtkKWScaleWithEntry* NAScale;
  vtkKWScaleWithEntry* NSScale;

private:
  vtkProstateNavManualControlStep(const vtkProstateNavManualControlStep&);
  void operator=(const vtkProstateNavManualControlStep&);
};

#endif
