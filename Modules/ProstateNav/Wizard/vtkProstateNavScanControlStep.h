#ifndef __vtkProstateNavScanControlStep_h
#define __vtkProstateNavScanControlStep_h

#include "vtkProstateNavStep.h"

#include "vtkMRMLFiducial.h"
#include "vtkMRMLFiducialListNode.h"

class vtkKWFrame;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWMatrixWidgetWithLabel;
class vtkKWPushButton;
class vtkMRMLSelectionNode;
class vtkMRMLFiducialListNode;

class VTK_PROSTATENAV_EXPORT vtkProstateNavScanControlStep : public vtkProstateNavStep
{
public:
  static vtkProstateNavScanControlStep *New();
  vtkTypeRevisionMacro(vtkProstateNavScanControlStep,vtkProstateNavStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ShowUserInterface();
  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);
  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

  void UpdateMRMLObserver(vtkMRMLSelectionNode* selnode);
  //void UpdateElement(int row, int col, char * str);
  void OnMultiColumnListUpdate(int row, int col, char * str);
  void OnMultiColumnListSelectionChanged();

  void SetGUIFromList(vtkMRMLFiducialListNode * activeFiducialListNode);

  //vtkGetStringMacro(FiducialListNodeID);
  //void SetFiducialListNodeID(char *id);
  //void SetFiducialListNode(vtkMRMLFiducialListNode *fiducialListNode);
  
protected:
  vtkProstateNavScanControlStep();
  ~vtkProstateNavScanControlStep();

  // GUI WIdgets
  vtkKWFrame* TargetListFrame;
  vtkKWMultiColumnListWithScrollbars* MultiColumnList;
  vtkKWMatrixWidgetWithLabel* NeedlePositionMatrix;
  vtkKWMatrixWidgetWithLabel* NeedleNormalMatrix;
  vtkKWFrame* TargetControlFrame;
  vtkKWPushButton *AddButton;
  vtkKWPushButton *RemoveButton;
  vtkKWPushButton *RemoveAllButton;


//  char *FiducialListNodeID;
//  vtkMRMLFiducialListNode *FiducialListNode;

private:
  vtkProstateNavScanControlStep(const vtkProstateNavScanControlStep&);
  void operator=(const vtkProstateNavScanControlStep&);
};

#endif
