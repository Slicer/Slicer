#ifndef __vtkProstateNavTargetingStep_h
#define __vtkProstateNavTargetingStep_h

#include "vtkProstateNavStep.h"

class vtkKWFrame;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWMatrixWidgetWithLabel;
class vtkKWPushButton;
class vtkMRMLSelectionNode;
class vtkMRMLFiducialListNode;

class VTK_PROSTATENAV_EXPORT vtkProstateNavTargetingStep : public vtkProstateNavStep
{
public:
  static vtkProstateNavTargetingStep *New();
  vtkTypeRevisionMacro(vtkProstateNavTargetingStep,vtkProstateNavStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ShowUserInterface();
  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);  
  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

  void UpdateMRMLObserver(vtkMRMLSelectionNode* selnode);
  void OnMultiColumnListUpdate(int row, int col, char * str);
  void OnMultiColumnListSelectionChanged();
  void SetGUIFromList(vtkMRMLFiducialListNode * activeFiducialListNode);

protected:
  vtkProstateNavTargetingStep();
  ~vtkProstateNavTargetingStep();

  // GUI Widgets
  vtkKWFrame *MainFrame;
  vtkKWFrame *TargetListFrame;
  vtkKWFrame *TargetControlFrame;

  vtkKWMultiColumnListWithScrollbars* MultiColumnList;
  vtkKWMatrixWidgetWithLabel* NeedlePositionMatrix;
  vtkKWMatrixWidgetWithLabel* NeedleOrientationMatrix;
  vtkKWPushButton *MoveButton;
  vtkKWPushButton *StopButton;

private:
  vtkProstateNavTargetingStep(const vtkProstateNavTargetingStep&);
  void operator=(const vtkProstateNavTargetingStep&);
};

#endif
