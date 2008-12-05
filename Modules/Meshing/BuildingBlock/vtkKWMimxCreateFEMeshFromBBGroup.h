/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateFEMeshFromBBGroup.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.24.2.1 $

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
// .NAME vtkKWMimxCreateFEMeshFromBBGroup - Class to create surface or solid FE mesh.
//
// .SECTION Description
// The class defines the GUI for FE mesh creation. Both surface and solid as well as
// quad, hex, tet and tri meshes can be created. Also meshes can be created by extruding
// the current mesh or a portion of the same. Element set creation option is once again
// used to select the element sets but for the surface elements. Option also exists for
// the specification of node and element numbers. If creation of FE mesh from building
// block is being used, the type of interpolation technique (elliptical or transfinite)
// to calculate the location of interior nodes and smoothing option (laplacian) iterations
// can also be specified.
//
// .SECTION See Also
// vtkKWMimxDefineElSetGroup, vtkMimxSolidEllipticalInterpolation, 
// vtkMimxSolidTransfiniteInterpolation

#ifndef __vtkKWMimxCreateFEMeshFromBBGroup_h
#define __vtkKWMimxCreateFEMeshFromBBGroup_h

#include "vtkKWMimxGroupBase.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxSurfaceMenuGroup.h"
#include "vtkBuildingBlockWin32Header.h"


#define HEX_BB_SOLID_MESH          1
#define HEX_EXTRUDE_SOLID_MESH     2
#define TET_BB_SOLID_MESH          3
#define TET_EXTRUDE_SOLID_MESH     4
#define QUAD_BB_SURFACE_MESH       5
#define QUAD_EXTRACT_SURFACE_MESH  6
#define TRI_BB_SURFACE_MESH        7
#define TRI_EXTRACT_SURFACE_MESH   8

class vtkIntArray;

class vtkKWCheckButtonWithLabel;
class vtkKWComboBoxWithLabel;
class vtkKWEntryWithLabel;
class vtkKWMenuButton;
class vtkKWMenuButtonWithLabel;
class vtkKWPushButton;
class vtkKWRadioButton;

class vtkKWMimxDefineElSetGroup;
class vtkKWMimxNodeElementNumbersGroup;
class vtkMimxMeshActor;

class VTK_BUILDINGBLOCK_EXPORT vtkKWMimxCreateFEMeshFromBBGroup : public vtkKWMimxGroupBase
{
public:

  static vtkKWMimxCreateFEMeshFromBBGroup* New();
  vtkTypeRevisionMacro(vtkKWMimxCreateFEMeshFromBBGroup,vtkKWMimxGroupBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void Update();
  virtual void UpdateEnableState();

  void CreateFEMeshFromBBDoneCallback();

  // Description:
  // Method to invoke FE mesh creation
  int CreateFEMeshFromBBApplyCallback();

  // Description:
  // Method to hide the GUI
  void CreateFEMeshFromBBCancelCallback();

  // Description:
  // Check box call back to whether the mesh created be smoothed or not.
  // If chosen, the number of iterations entry for smoothing is enabled.
  void SmoothFEMeshCallback(int state);

  // Description:
  // Update the FE mesh list. Used when the GUI is redrawn or to add newly
  // created FE mesh to the list.
  void UpdateObjectLists();

  // Description:
  // Show the GUI required for volumetric meshing
  void VolumetricMeshCallback();

  // Description:
  // Show the GUI required for surface meshing
  void SurfaceMeshCallback();

  // Description:
  // GUI for volumetric mesh from building block method.
  void VolumetricMeshBBCallback();

  // Description:
  // GUI for volumetric mesh from extrusion.
  void VolumetricMeshExtrudeCallback();

  // Description:
  // Choose the interpolation mode between Elliptical and Transfinite
  void InterpolationModeCallback();

  // Description:
  // Creation of surface mesh building block method
  void SurfaceMeshBBCallback();

  // Description:
  // Extract the surface mesh
  void SurfaceMeshExtractCallback();

  // Description:
  // Launch the window from which a subset selection of FE mesh be made
  void DefineRegionCallback();

  // Description:
  // Get the meshing mode. 8 types of meshing exist. Look for the description
  // at the beginning of the header file.
  int  GetMeshingMode( );

  // Description:
  // Any newly created mesh by default contains 5 mesh field data. 1) A cell data for element numbers
  // 2) Point data for node numbers, 3) Field data string array for element set names,
  // 4) Field data string array for node set names and 5) Filed data for number of boundary condition steps.
  void InitializeMeshFieldData(vtkUnstructuredGrid *mesh, const char *nodeSetName, 
                     const char *elementSetName, int nodeNumber, int elementNumber);

  // Description:
  // Create mesh from building block
  int CreateMeshFromBuildingBlock(vtkPolyData *polyDaya, vtkUnstructuredGrid *buildingBlock, 
                     bool smoothMesh, int numSmooth, const char *nodeSetName, int nodeNumber, 
                     const char *elementSetName, int elementNumber, int numRecalc, 
                     bool generateSurfaceMesh, bool generateTriMesh, const char *FoundationName);

  // Description:
  // Extrude hexahedral solid mesh from the chosen surface subset
  int ExtrudeHexSolidMesh(vtkMimxMeshActor *MeshActor, const char *NodeSetName,
          const char *ElSetName, int StartNodeNum, int StartElemNum,
          int NumberOfDiv, double ExtrusionLength, const char *FoundationName);

  // Description:
  // Create a surface mesh
        int ExtractSurfaceMesh(vtkMimxMeshActor *MeshActor, const char *NodeSetName,
          const char *ElSetName, int StartNodeNum, int StartElemNum, bool generateTriMesh, const char *FoundationName);
protected:
        vtkKWMimxCreateFEMeshFromBBGroup();
        ~vtkKWMimxCreateFEMeshFromBBGroup();
        virtual void CreateWidget();
  
  vtkKWFrame *ModeFrame;
  vtkKWRadioButton *VolumetricMeshButton;
  vtkKWRadioButton *SurfaceMeshButton;
  vtkKWRadioButton *HexMeshButton;
  vtkKWRadioButton *TetMeshButton;
  vtkKWRadioButton *QuadMeshButton;
  vtkKWRadioButton *TriMeshButton;
  vtkKWMenuButton *TypeMenuButton;
  vtkKWMenuButtonWithLabel *InterpolationMenuButton;
  
  vtkKWFrameWithLabel *ComponentFrame;
  vtkKWComboBoxWithLabel *SurfaceListComboBox;
  vtkKWComboBoxWithLabel *BBListComboBox;
  vtkKWComboBoxWithLabel *MeshListComboBox;
  vtkIntArray *OriginalPosition;
  vtkKWFrame *SmoothFrame;
  vtkKWFrame *InterpFrame;
  vtkKWEntryWithLabel *SmoothingIterationsEntry;
  vtkKWEntryWithLabel *ElementSetNameEntry;
  vtkKWEntryWithLabel *RecalculationIterationsEntry;
  vtkKWMimxNodeElementNumbersGroup *NodeElementNumbersGroup;
  vtkKWFrameWithLabel *MeshImprovementFrame;
  vtkKWCheckButtonWithLabel *SmoothButton;
  vtkKWFrame *SelectFrame;
  vtkKWRadioButton *SelectSurfaceButton;
  vtkKWRadioButton *SelectFaceButton;
  vtkKWRadioButton *SelectElementButton;
  vtkKWPushButton *SelectButton;
  vtkKWMimxDefineElSetGroup *DefineElementSetDialog;
  
private:
  vtkKWMimxCreateFEMeshFromBBGroup(const vtkKWMimxCreateFEMeshFromBBGroup&); // Not implemented
  void operator=(const vtkKWMimxCreateFEMeshFromBBGroup&); // Not implemented
  
  int ExtractCount;
  int BBCount;
  int ExtrudeCount;
 };

#endif

