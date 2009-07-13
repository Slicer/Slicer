/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxMeshActor.h,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.25.2.2 $


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

// .NAME vtkMimxMeshActor - 
// .SECTION Description
// vtkMimxMeshActor is the class to handle display and manipulation
// of meshes. The actor has default representation for the input unstructuredgrid.
// In addition, the wire frame is represented through a tubefilter and element sets
// stored as seperate actors. For scalar display, cell based scalars are the only ones
// that are available.

#ifndef __vtkMimxMeshActor_h
#define __vtkMimxMeshActor_h

#include <string>
#include <list>

#include "vtkMimxActorBase.h"
#include "vtkMimxCommonWin32Header.h"

class vtkActor;
class vtkDataSetMapper;
class vtkExtractCells;
class vtkExtractGeometry;
class vtkFeatureEdges;
class vtkGeometryFilter;
class vtkIdList;
class vtkLookupTable;
class vtkPlane;
class vtkPlaneWidget;
class vtkPointSet;
class vtkPolyDataMapper;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkScalarBarActor;
class vtkShrinkFilter;
class vtkTubeFilter;
class vtkUnstructuredGrid;

class VTK_MIMXCOMMON_EXPORT MeshDisplayProperty
{
public:
  std::string name;
  bool IsVisible;

  int DisplayType;
  vtkLookupTable *lutFilter; 
  vtkExtractCells *ExtractCellsFilter;
  vtkActor *SurfaceActor;
  vtkActor *OutlineActor;
  vtkActor *InteriorActor;
  vtkScalarBarActor *LegendActor;
  vtkDataSetMapper *SurfaceMapper;
  vtkPolyDataMapper *OutlineMapper;
  vtkDataSetMapper *InteriorMapper;
  vtkShrinkFilter *ShrinkFilter;
  vtkGeometryFilter *GeometryFilter;
  vtkTubeFilter *TubeFilter;
  vtkShrinkFilter *InteriorShrinkFilter;
  std::string activeAttribute;
  MeshDisplayProperty()
    {
    this->LegendActor = NULL;
    }
};

class VTK_MIMXCOMMON_EXPORT vtkMimxMeshActor : public vtkMimxActorBase
{
public:
  static vtkMimxMeshActor *New();
  vtkTypeRevisionMacro(vtkMimxMeshActor,vtkMimxActorBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  vtkUnstructuredGrid* GetDataSet();
  void SetDataSet( vtkUnstructuredGrid *ugrid);

  // type of display
  enum { 
    DisplaySurface                = 1,
    DisplayOutline                = 2,
    DisplaySurfaceAndOutline      = 3
  };
  
  // what to display
  enum { 
    DisplayMesh                   = 1,
    DisplayElementSets            = 2
  };
  
  // scalar bar coloring
  enum{
          RedToBlue                                       = 1,
          BlueToRed                                       = 2
  };

  // type of mesh
  enum{
          SurfaceMesh                             = 1,
          VolumeMesh                              = 2,
          MixedMesh                                       = 3
  };
  vtkSetMacro(ElementSetName, char*);
  vtkGetMacro(ElementSetName, char*);
  
  vtkGetMacro(OutlineActor, vtkActor*);
  vtkGetMacro(InteriorActor, vtkActor*);
  
  vtkGetMacro(NumberOfElementSets, int);
 
  vtkGetMacro(IsAverageEdgeLengthCalculated, int);
  vtkGetMacro(AverageEdgeLength, double);

  // Description:
  // Recalculate scalar display based on color range. The range is from
  // Blue to Red and Red to Blue.
  void SetColorRangeType(int RangeType, const char *ArrayName, 
          const char *ElementSetName, double *Range);
  vtkGetMacro(ColorRangeType, int);

  // Description:
  // Delete the node set. (not fully implemented)
  void DeleteNodeSet(const char *Name);

  // Description:
  // Delete the element set. (not fully implemented)
  void DeleteElementSet(const char *Name);
  
  void SetRenderer(vtkRenderer *renderer);
  vtkRenderer* GetRenderer();
  
  void SetInteractor(vtkRenderWindowInteractor *interactor);
  vtkRenderWindowInteractor *GetInteractor( );
  
  // Description:
  // Get the type of display. Modes available are, solid, wireframe
  // and solid + wireframe.
  int  GetDisplayMode( );

  // Description:
  // Get the type of display. Modes available are, solid, wireframe
  // and solid + wireframe.
  void SetDisplayMode( int mode );
 
  // Description:
  // Display the mesh
  void ShowMesh();

  // Description:
  // Hide the mesh
  void HideMesh();
  void HideAllElementSets();
  bool GetMeshVisibility();
  void SaveMeshVisibility();
  void RestoreMeshVisibility();
  
  // Description:
  // Setting legend formatting for scalar display
  void SetLegendTextColor( double color[3] );
  double *GetLegendTextColor( );
  
  void SetLegendTitle( const char *title );
  const char *GetLegendTitle( );
  void SetLegendPrecision( int precision );
  int GetLegendPrecision( );
 
  // Description:
  // Setting mesh and legend display properties and visibility
  void SetMeshDisplayType( int mode );
  int  GetMeshDisplayType();
  void SetMeshOutlineColor(double red, double green, double blue);
  void SetMeshOutlineColor(double rgb[3]);
  void GetMeshOutlineColor(double &red, double &green, double &blue);
  void GetMeshOutlineColor(double rgb[3]);
  void SetMeshOutlineRadius(double radius);
  double GetMeshOutlineRadius(); 
  void SetMeshShrinkFactor(double shrinkFactor);
  double GetMeshShrinkFactor();
  void SetMeshColor(double red, double green, double blue);
  void SetMeshColor(double rgb[3]);
  void GetMeshColor(double &red, double &green, double &blue);
  void GetMeshColor(double rgb[3]);
  void SetMeshOpacity(double opacity);
  double GetMeshOpacity();
  void SetMeshScalarVisibility(bool visibility);
  bool GetMeshScalarVisibility( );
  void SetMeshLegendVisibility(bool visible);
  void SetElementSetLegendVisibility(std::string setName, bool visible);
  bool GetMeshLegendVisibility( );

  // Description:
  // Setting legend ranges. Both for the mesh and individual element sets.
  void SetLegendRange(double min, double max);
  void SetElementSetLegendRange(double min, double max, MeshDisplayProperty *currentSet);

  // Description:
  // Set/Get the active mesh scalar name
  void SetMeshScalarName(std::string scalarName);
  std::string GetMeshScalarName( );

  // Description:
  // Show and hide cutting plane.
  void EnableMeshCuttingPlane();
  void DisableMeshCuttingPlane();
 
  // Description:
  // Invert the cutting plane
  void SetInvertCuttingPlane( bool invert );
  bool GetInvertCuttingPlane( );

  // Description:
  // type of display for the element set.
  void SetElementSetDisplayType( std::string name, int mode );
  int  GetElementSetDisplayType( std::string name );

  // Description:
  // Changing the wireframe color
  void SetElementSetOutlineColor(std::string name, double red, double green, double blue);
  void SetElementSetOutlineColor(std::string name, double rgb[3]);
  void GetElementSetOutlineColor(std::string name, double &red, double &green, double &blue);
  void GetElementSetOutlineColor(std::string name, double rgb[3]);
  
  // Description:
  // Setting the size of element size shrink factor.
  void SetElementSetShrinkFactor(std::string name, double shrinkFactor);
  double GetElementSetShrinkFactor( std::string name );
  void SetElementSetColor(std::string name, double red, double green, double blue);
  void SetElementSetColor(std::string name, double rgb[3]);
  void GetElementSetColor(std::string name, double &red, double &green, double &blue);
  void GetElementSetColor(std::string name, double rgb[3]);
  void SetElementSetOpacity(std::string name, double opacity);
  double GetElementSetOpacity( std::string name );
  void ShowElementSet(std::string name);
  void HideElementSet(std::string name);
  bool GetElementSetVisibility(std::string name);

  // Description:
  // Changing the thickness of wireframe lines
  void SetElementSetOutlineRadius(std::string name, double radius);
  double GetElementSetOutlineRadius(std::string name); 
  
  // Description:
  // To choose the array from which scalar values are obtained.
  void SetElementSetScalarName(std::string setName, std::string scalarName);
  std::string GetElementSetScalarName( std::string setName );
  void EnableElementSetCuttingPlane( std::string setName );
  void DisableElementSetCuttingPlane( std::string setName );
  void SetAllElementSetScalarName(std::string scalarName);
  
  void SetElementSetScalarVisibility(std::string setName, bool visibility);
  bool GetElementSetScalarVisibility( std::string setName );
  void SetAllElementSetScalarVisibility( bool visibility );
  
  void DeleteElementSetListItem( std::string setName );
  void AddElementSetListItem( std::string setName );
  void CreateElementSetList( );

  // Description:
  // Delete a boundary condition step. All the associated field data are deleted.
  void DeleteBoundaryConditionStep(int StepNum);

  // Description:
  // To generate boundary condition array names. Should be modified in the future
  void ConcatenateStrings(const char* Step, const char* Num, 
          const char* NodeSetName, const char* Type, const char* Direction, char *Name);

  // Description:
  // Change the element numbers of a given element set
  void ChangeElementSetNumbers(const char* ElSetName, int StartEleNum);

  // Description:
  // Change the node numbers of a given node set.
  void ChangeNodeSetNumbers(const char* NodeSetName, int StartNodeNum);

  // Description:
  // Calculating the average edge length. This is required for glyph size estimate.
  void CalculateAverageEdgeLength();

  vtkPointSet* GetPointSetOfNodeSet(const char* NodeSetName);

  // Description:
  // Values from array ElementSetName_Image_Based_Material_Property are used.
  void StoreImageBasedMaterialProperty(const char* ElSetName);

  // Description:
  // Values from array ElementSetName_Image_Based_Material_Property_ReBin are used.
  void StoreImageBasedMaterialPropertyReBin(const char* ElSetName);

  // Description:
  // Values from array ElementSetName_Constant_Youngs_Modulus are used in storing.
  // Cell array named Youngs_Modulus is updated with latest changes
  void StoreConstantMaterialProperty(const char* ElSetName, double YoungMod);

  // Description:
  // Values from array ElementSetName_Constant_Poissons_Ratio are used in storing.
  void StoreConstantPoissonsRatio(const char* ElSetName, double PoissonRatio);

  // Description:
  // Range for color coding calculation.
  double* ComputeElementSetScalarRange(const char* ElSetName, const char* ArrayName);

  // Description:
   // Range for color coding calculation.
  double* ComputeMeshScalarRange(const char* ArrayName);

  void ShowHideAllElementSets(bool Show);
  void SetElementSetScalarRangeFromCompleteMesh(const char *ElSetName, const char* ArrayName);
  void SetElementSetScalarRangeFromElementSet(const char *ElSetName, const char* ArrayName);
  MeshDisplayProperty* GetMeshDisplayProperty(const char *ElSetName);
  void HideAllElementSetLegends();

  // Description:
  // To color code individual cells/elements based on the value from the scalar array under
  // consideration.
  void GenerateMeshMapperLookUpTable(const char *ArrayName, double *range);

  // Description:
  // To color code individual cells/elements based on the value from the scalar array under
  // consideration.
  void GenerateElementSetMapperLookUpTable(
          const char *ElementSetName, const char *ArrayName, double *range);

  // Description:
  // Color values for scalar bar actor. Right now the number of colors that make up the
  // RGB range is 100.
  void RedToBlueLookUpTable();

  // Description:
  // Color values for scalar bar actor. Right now the number of colors that make up the
  // RGB range is 100.
  void BlueToRedLookUpTable();

  // Description:
  // Get the active scalar range
  void GetCurrentScalarRange( double *range );

  // Description:
  // Append to the existing element set
  void AppendElementSet(const char *ElSetName, vtkIdList *ElementList);

  // Description:
  // Append to the existing node set
  void AppendNodeSet(const char *NodeSetName, vtkIdList *NodeList);

  // Description:
  // Add a reference node to the mesh. The node will be placed at the center of the 
  // bounds of the dataset. A field data named ElementSetName_RN would be added with
  // the co-ordinates.
  void AddReferenceNode(const char *ElementSetName);
protected:
  vtkMimxMeshActor();
  ~vtkMimxMeshActor();
  vtkUnstructuredGrid *UnstructuredGrid;
  vtkDataSetMapper *UnstructuredGridMapper;
  void UpdateMeshDisplay();
  void UpdateElementSetDisplay();
  void MapperRedToBlueLookUpTable(vtkLookupTable *Lut, const char *ArrayName, double *range);
  void MapperBlueToRedLookUpTable(vtkLookupTable *Lut, const char *ArrayName, double *range);

  void BuildScalarBar();

private:
  vtkActor *OutlineActor;
  vtkActor *InteriorActor;
  vtkPolyDataMapper *OutlineMapper;
  vtkDataSetMapper *InteriorMapper;
  vtkShrinkFilter *ShrinkFilter;
  vtkShrinkFilter *InteriorShrinkFilter;
  vtkGeometryFilter *OutlineGeometryFilter;
  vtkTubeFilter *TubeFilter;
  vtkRenderer *Renderer;
  vtkRenderWindowInteractor *Interactor;
  vtkPlaneWidget *CuttingPlaneWidget;
  vtkExtractGeometry *ClipPlaneGeometryFilter;
  vtkFeatureEdges *FeatureEdges;
  vtkPlane *CuttingPlane;
  vtkScalarBarActor *LegendActor;
  vtkLookupTable *lutFilter;
  double ElementShrinkFactor;
  bool IsVisible;
  bool SavedVisibility;    // use for enter/exit of module
        char *ElementSetName;
        int DisplayMode;
        int DisplayType;
        int NumberOfElementSets;
        int ColorRangeType;
        bool CuttingPlaneEnabled;
        double TextColor[3];
        std::string activeAttribute;
        std::list<MeshDisplayProperty*> ElementSetDisplayList;
        int IsAverageEdgeLengthCalculated;
        double AverageEdgeLength;
        vtkPointSet *PointSetOfNodeSet;
        int LegendPrecision;
        int InvertCuttingPlane;
        int MeshType;
  vtkMimxMeshActor(const vtkMimxMeshActor&);  // Not implemented.
  void operator=(const vtkMimxMeshActor&);  // Not implemented.
};

#endif

