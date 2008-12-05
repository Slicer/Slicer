/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxQualityMenuGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.5.4.2 $

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
// .NAME vtkKWMimxQualityMenuGroup - GUI representing the Quality operations.
// .SECTION Description
// The class contains GUI to mesh Quality related operations. The options
// available are 1) Evaluate mesh quality and display (Quality metrics are
// volume, jacobian, skew and angle). 2) Improve mesh quality using laplacian
// smoothing and 3) save the mesh in either VTK format or export in Abaqus
// format.

#ifndef __vtkKWMimxQualityMenuGroup_h
#define __vtkKWMimxQualityMenuGroup_h

#include "vtkKWMimxMainMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"


class vtkKWMimxEditFEMeshLaplacianSmoothGroup;
class vtkKWMimxEvaluateMeshQualityGroup;
class vtkKWMimxExportAbaqusFEMeshGroup;
class vtkKWMimxSaveVTKFEMeshGroup;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxQualityMenuGroup : public vtkKWMimxMainMenuGroup
{
public:
  static vtkKWMimxQualityMenuGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxQualityMenuGroup,vtkKWMimxMainMenuGroup);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Update();
  virtual void UpdateEnableState();
  
  // Description:
  // Choose the quality metric for evaluation
  void EvaluateMeshVolumeCallback();
  void EvaluateMeshEdgeCollapseCallback();
  void EvaluateMeshJacobianCallback();
  void EvaluateMeshSkewCallback();
  void EvaluateMeshAngleCallback();

  // Description:
  // Initialize the mesh for quality calculation
  void EvaluateMeshCallback();

  // Description:
  // Launch the GUI for mesh quality metrics statistics
  void CreateMeshQualityGroup();

  // Description:
  // Laplacian smoothing options
  void SmoothLaplacianFEMeshCallback();

  // Description:
  // Save the mesh in VTK file format
  void SaveVTKFEMeshCallback();

  // Description:
  // Export the file in abaqus file format
  void ExportAbaqusFEMeshCallback();
  
  // Description:
  // Access to the lists of the objects used
  vtkSetObjectMacro(FEMeshList, vtkLinkedListWrapper);
  vtkSetObjectMacro(SurfaceList, vtkLinkedListWrapper);
  vtkSetObjectMacro(BBoxList, vtkLinkedListWrapper);
  
  void UpdateObjectLists();
  
protected:
        vtkKWMimxQualityMenuGroup();
        ~vtkKWMimxQualityMenuGroup();
        vtkKWMimxSaveVTKFEMeshGroup *SaveVTKFEMeshGroup;
  vtkKWMimxExportAbaqusFEMeshGroup *ExportAbaqusFEMeshGroup;
        virtual void CreateWidget();
        
private:
  vtkKWMimxQualityMenuGroup(const vtkKWMimxQualityMenuGroup&); // Not implemented
  void operator=(const vtkKWMimxQualityMenuGroup&); // Not implemented

  vtkKWMimxEvaluateMeshQualityGroup *EvaluateMeshInterface;
  vtkKWMimxEditFEMeshLaplacianSmoothGroup *FEMeshLaplacianSmoothGroup;

};

#endif
