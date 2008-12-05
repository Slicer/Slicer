/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxEvaluateMeshQualityGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.15.2.1 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __vtkKWMimxEvaluateMeshQualityGroup_h
#define __vtkKWMimxEvaluateMeshQualityGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkDoubleArray;
class vtkIdList;

class vtkKWComboBoxWithLabel;
class vtkKWCheckButton;
class vtkKWCheckButtonWithLabel;
class vtkKWEntryWithLabel;
class vtkKWFrame;
class vtkKWLabel;
class vtkKWListBoxWithScrollbars;
class vtkKWLoadSaveDialog;
class vtkKWMenuButtonWithLabel;
class vtkKWPushButton;

class vtkKWMimxViewPropertiesOptionGroup;

#define MESH_QUALITY_VOLUME    1
#define MESH_QUALITY_EDGE      2
#define MESH_QUALITY_JACOBIAN  3
#define MESH_QUALITY_SKEW      4
#define MESH_QUALITY_ANGLE     5
#define MESH_QUALITY_MIN_ANGLE 6
#define MESH_QUALITY_MAX_ANGLE 7

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxEvaluateMeshQualityGroup : public vtkKWMimxGroupBase
{
public:
  static vtkKWMimxEvaluateMeshQualityGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxEvaluateMeshQualityGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();
  void UpdateObjectLists();

  
  // Description: 
  // Hide the GUI being displayed
  int EvaluateMeshQualityCancelCallback();

  // Description:
  // Evaluate the mesh quality based on the type chosen. Quality
  // types are defined at the beginning of the header file 1 to 7
  int EvaluateMeshQualityApplyCallback(int qualityType);

  // Description:
  // Show only the distorted elements
  int ViewDistortedElemenetsCallback();

  // Description:
  // Hide the GUI associated with the distorted elements
  int DistortedElementDialogCancelCallback();

  // Description:
  // Store the chosen mesh quality in .csv style format
  int DistortedElementDialogSaveCallback();
  
  // Description:
  // Invert the cutting plane used to visualize the internal elements
  int InvertPlaneCallback(int mode);

  // Description:
  // Show/hide the clipping plane
  int ClippingPlaneCallback(int mode);

  // Description:
  // To display the chosen quality metric values for the given mesh
  int ViewQualityLegendCallback(int mode);
  
  // Description:
  // Setting the quality type metric for given volume
  int SetQualityTypeToVolume() {QualityType = MESH_QUALITY_VOLUME;
  return 1;};
  int SetQualityTypeToEdgeCollapse() {QualityType = MESH_QUALITY_EDGE;
  return 1;};
  int SetQualityTypeToJacobian() {QualityType = MESH_QUALITY_JACOBIAN;
  return 1;};
  int SetQualityTypeToSkew() {QualityType = MESH_QUALITY_SKEW;
  return 1;};
  int SetQualityTypeToAngle() {QualityType = MESH_QUALITY_ANGLE;
  return 1;};

  // Description:
  // Reset the values of all the entries in the GUI
  void ClearStatsEntry();  

  // Description:
  // Call back for selected mesh change
  void SelectionChangedCallback(const char* selection);

  // Description:
  // To launch a window containing options for setting mesh quality
  // visualization properties
  void DisplayOptionsCallback(); 
protected:
  vtkKWMimxEvaluateMeshQualityGroup();
  ~vtkKWMimxEvaluateMeshQualityGroup();
  
  vtkKWComboBoxWithLabel *MeshListComboBox;
  
  // Description:
  // Create the widget.
  virtual void CreateWidget();
 
  vtkKWFrameWithLabel *DisplayFrame;
  vtkKWFrame *ViewFrame;
  vtkKWFrameWithLabel *ComponentFrame;

  vtkKWEntryWithLabel *NumberOfDistortedEntry;
  vtkKWEntryWithLabel *NumberOfElementsEntry;
  vtkKWEntryWithLabel *QualityMinimumEntry;
  vtkKWEntryWithLabel *QualityMaximumEntry;
  vtkKWEntryWithLabel *QualityAverageEntry;
  vtkKWEntryWithLabel *QualityVarianceEntry;
  vtkKWPushButton *SaveButton;
  vtkKWCheckButton *ViewQualityButton;
  vtkKWCheckButton *ClippingPlaneButton;
  vtkKWCheckButton *ViewLegendButton;
  vtkKWCheckButtonWithLabel *InvertPlaneButton;
  vtkKWTopLevel *DistortedElementDialog;
  vtkKWFrame *DistortedButtonFrame;
  vtkKWPushButton *SaveDistortedButton;
  vtkKWPushButton *CancelDistortedButton;
  vtkKWListBoxWithScrollbars *DistortedElementsReport;
  vtkKWLoadSaveDialog *FileBrowserDialog;
  vtkKWFrame *ButtonFrame;
  vtkKWFrame *SummaryFrame;
  vtkKWFrame *SummaryReviewFrame;
  vtkKWMenuButtonWithLabel *QualityTypeButton;
  vtkKWLabel *QualityTypeLabel;
  vtkKWLabel *DistoredListLabel;
  vtkKWPushButton *DisplayOptionsButton;
  vtkKWMimxViewPropertiesOptionGroup* ViewOptionsGroup;
private:
  vtkKWMimxEvaluateMeshQualityGroup(const vtkKWMimxEvaluateMeshQualityGroup&);   // Not implemented.
  void operator=(const vtkKWMimxEvaluateMeshQualityGroup&);  // Not implemented.
  
  vtkIdList *DistortedElementList;
  vtkDoubleArray *DistortedMeshQuality;
  int NumberOfCells;
  int QualityType;
  char meshName[64];
  char qualityName[64];
  char PreviousSelection[64];
  double minimumQuality;
  double maximumQuality;
  double averageQuality;
  double varianceQuality;
  //void AddOrientationWidget(void);
  
};

#endif
