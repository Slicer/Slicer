/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkProstateNavStepVerification_h
#define __vtkProstateNavStepVerification_h

#include "vtkProstateNavStep.h"

#include "vtkMRMLFiducial.h"
#include "vtkMRMLFiducialListNode.h"

class vtkKWFrame;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWMatrixWidgetWithLabel;
class vtkKWPushButton;
class vtkMRMLSelectionNode;
class vtkMRMLFiducialListNode;

class VTK_PROSTATENAV_EXPORT vtkProstateNavStepVerification : public vtkProstateNavStep
{
public:
  static vtkProstateNavStepVerification *New();
  vtkTypeRevisionMacro(vtkProstateNavStepVerification,vtkProstateNavStep);
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
  vtkProstateNavStepVerification();
  ~vtkProstateNavStepVerification();

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
  vtkProstateNavStepVerification(const vtkProstateNavStepVerification&);
  void operator=(const vtkProstateNavStepVerification&);
};

#endif
