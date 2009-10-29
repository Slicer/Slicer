/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxMeshActor.cxx,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.48.2.3 $


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

#include "vtkMimxMeshActor.h"

#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkCellData.h"
#include "vtkCellTypes.h"
#include "vtkCommand.h"
#include "vtkDataSetMapper.h"
#include "vtkDoubleArray.h"
#include "vtkExtractCells.h"
#include "vtkExtractGeometry.h"
#include "vtkFeatureEdges.h"
#include "vtkFloatArray.h"
#include "vtkGenericCell.h"
#include "vtkGeometryFilter.h"
#include "vtkHexahedron.h"
#include "vtkIdList.h"
#include "vtkIntArray.h"
#include "vtkLookupTable.h"
#include "vtkMath.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPlaneWidget.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkScalarBarActor.h"
#include "vtkShrinkFilter.h"
#include "vtkStringArray.h"
#include "vtkTextProperty.h"
#include "vtkTubeFilter.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkMimxMeshActor, "$Revision: 1.48.2.3 $");
vtkStandardNewMacro(vtkMimxMeshActor);

// Create a callback function so the selection plane can invoke this when it is
// moved by the operator.  The corresponding implicit plane function is passed 
// back into a pre-allocated vtkPlane instance for use elsewhere in the application

//--------------------------------------------------------------------------------------
class vtkPlaneWidgetEventCallback : public vtkCommand
{
public:
  static vtkPlaneWidgetEventCallback *New() 
    { return new vtkPlaneWidgetEventCallback; }
  virtual void Execute(vtkObject *caller, unsigned long, void*)
    {
      vtkPlaneWidget *planeWidget = reinterpret_cast<vtkPlaneWidget*>(caller);
      planeWidget->GetPlane(this->PlaneInstance);
    }
  
  vtkPlane *PlaneInstance;
};

//----------------------------------------------------------------------------------------
vtkMimxMeshActor::vtkMimxMeshActor()
{
  this->ElementShrinkFactor = 1.0;
  this->DataType = ACTOR_FE_MESH;
  this->ElementSetName = NULL;
  this->IsVisible = true;
  this->SavedVisibility = this->IsVisible;
  this->ElementSetDisplayList.clear();
  this->DisplayMode = vtkMimxMeshActor::DisplayMesh;
  this->DisplayType = vtkMimxMeshActor::DisplaySurfaceAndOutline;
  this->NumberOfElementSets = 0;
  this->Interactor = NULL;
  this->Renderer = NULL;
  this->CuttingPlaneEnabled = false;
  this->LegendPrecision = 3;
  this->TextColor[0] = this->TextColor[1] = this->TextColor[2] = 1.0;
        
  /* Setup the Pipeline for the Mesh */
  vtkPoints *points = vtkPoints::New();
  this->UnstructuredGrid = vtkUnstructuredGrid::New();
  this->UnstructuredGrid->SetPoints(points);
  points->Delete();
  
  this->ShrinkFilter = vtkShrinkFilter::New();
  this->ShrinkFilter->SetInput(this->UnstructuredGrid);
  this->ShrinkFilter->SetShrinkFactor(this->ElementShrinkFactor);

  vtkDataSetMapper*  meshMapper = vtkDataSetMapper::New();
  meshMapper->SetInputConnection(this->ShrinkFilter->GetOutputPort());
  this->UnstructuredGridMapper = meshMapper;
  this->UnstructuredGridMapper->SetScalarVisibility(0);
  this->Actor = vtkActor::New();
  this->Actor->SetMapper(this->UnstructuredGridMapper);
        
  /* Setup the Pipeline for the Wireframe */
  this->OutlineGeometryFilter = vtkGeometryFilter::New();
  this->OutlineGeometryFilter->SetInput( this->UnstructuredGrid );

  this->FeatureEdges = vtkFeatureEdges::New();
  this->FeatureEdges->SetInput( this->OutlineGeometryFilter->GetOutput() );
  this->FeatureEdges->BoundaryEdgesOn();
  this->FeatureEdges->ManifoldEdgesOn();
  this->FeatureEdges->FeatureEdgesOff();
  this->FeatureEdges->ColoringOff();
  
  this->TubeFilter = vtkTubeFilter::New();
  this->TubeFilter->SetInputConnection(this->FeatureEdges->GetOutputPort());
  this->TubeFilter->SetRadius(0.03);
   
  this->OutlineMapper = vtkPolyDataMapper::New();
  this->OutlineMapper->SetInputConnection(  this->TubeFilter->GetOutputPort() );
  this->OutlineMapper->SetScalarVisibility( 0 );
  
  this->OutlineActor = vtkActor::New();
  this->OutlineActor->SetMapper( this->OutlineMapper );
  this->OutlineActor->GetProperty()->SetColor(0.0,0.0,0.0);
  this->OutlineActor->GetProperty()->SetRepresentationToSurface();
  this->OutlineActor->GetProperty()->SetAmbient(1);
  this->OutlineActor->GetProperty()->SetSpecular(0);
  this->OutlineActor->GetProperty()->SetDiffuse(0);
  this->OutlineActor->GetProperty()->SetSpecularPower(0);
  //this->OutlineActor->GetProperty()->SetLineWidth(1.0);
  
  /* Setup the Pipeline for Interior Display */
  /*** NOTE: This is not currently being used ***/
  this->InteriorShrinkFilter = vtkShrinkFilter::New();
  this->InteriorShrinkFilter->SetInput( this->UnstructuredGrid );
  this->InteriorShrinkFilter->SetShrinkFactor(0.995);
  
  this->InteriorMapper = vtkDataSetMapper::New();
  this->InteriorMapper->SetInputConnection(this->InteriorShrinkFilter->GetOutputPort());
  this->InteriorMapper->ScalarVisibilityOff();
  
  this->InteriorActor = vtkActor::New();
  this->InteriorActor->SetMapper( this->InteriorMapper );
  this->InteriorActor->GetProperty()->SetColor(0.5,0.5,0.5);
  this->InteriorActor->GetProperty()->SetOpacity(0.25);
  this->InteriorActor->GetProperty()->SetRepresentationToWireframe();
  this->InteriorActor->GetProperty()->SetAmbient(1.0);
  this->InteriorActor->SetVisibility(0);
  /* This is used to display a Portion of the Mesh when the Cutting Plane is enabled */
  
  this->CuttingPlaneWidget = vtkPlaneWidget::New();
  this->CuttingPlaneWidget->SetInput( this->UnstructuredGrid );
  this->CuttingPlaneWidget->SetRepresentationToSurface();
  this->CuttingPlaneWidget->GetPlaneProperty()->SetColor(0.2,0.2,0);
  this->CuttingPlaneWidget->GetPlaneProperty()->SetOpacity(0.2);
  this->CuttingPlaneWidget->GetSelectedPlaneProperty()->SetOpacity(0.2);  
  this->CuttingPlaneWidget->SetHandleSize(0.02);
  this->CuttingPlane = vtkPlane::New();
  
  this->ClipPlaneGeometryFilter = vtkExtractGeometry::New();
  this->ClipPlaneGeometryFilter->SetInput( this->UnstructuredGrid );
        
  /* This is used for Scale Display */
  //
  this->lutFilter = NULL;
  this->LegendActor = NULL;

  this->IsAverageEdgeLengthCalculated = 0;
  this->AverageEdgeLength = 0.0;
  this->PointSetOfNodeSet = NULL;
  this->InvertCuttingPlane = 0;
  this->MeshType = 0;
  this->ColorRangeType = vtkMimxMeshActor::BlueToRed;
}

//----------------------------------------------------------------------------------------
vtkMimxMeshActor::~vtkMimxMeshActor()
{
  this->UnstructuredGrid->SetPoints(NULL);
  this->UnstructuredGrid->Delete();
  this->UnstructuredGridMapper->Delete();
  this->Actor->Delete();
  this->OutlineMapper->Delete();
  this->OutlineActor->Delete();
  this->InteriorMapper->Delete();
  this->InteriorActor->Delete();
  this->ShrinkFilter->Delete();
  this->InteriorShrinkFilter->Delete();
  if(this->PointSetOfNodeSet)
    this->PointSetOfNodeSet->Delete();
  this->ClipPlaneGeometryFilter->Delete();
  this->FeatureEdges->Delete();
  this->CuttingPlaneWidget->Delete();
  this->CuttingPlane->Delete();
  this->TubeFilter->Delete();
  this->OutlineGeometryFilter->Delete();
  if (this->LegendActor)
    {
    vtkWarningMacro("Legend Actor Delete");
    this->LegendActor->Delete();
    this->LegendActor = NULL;
    }
  if (this->lutFilter)
    {
    this->lutFilter->Delete();
    this->lutFilter = NULL;
    }
}

//----------------------------------------------------------------------------------------
void vtkMimxMeshActor::BuildScalarBar()
{
  if (!this->lutFilter)
    {
    this->lutFilter = vtkLookupTable::New();
    }
  this->BlueToRedLookUpTable();
  if (!this->LegendActor)
    {
    vtkWarningMacro("LegendActor: vtkScalarBarActor::New");
    this->LegendActor = vtkScalarBarActor::New();
    }
  vtkTextProperty *textProperty = this->LegendActor->GetTitleTextProperty();
  textProperty->SetFontFamilyToArial();
  textProperty->SetColor( TextColor );
  textProperty->ShadowOff();
  textProperty->ItalicOff();
  textProperty->SetFontSize(40);
  
  this->LegendActor->SetTitleTextProperty( textProperty );
  this->LegendActor->SetLabelTextProperty( textProperty );
  textProperty->Delete();
  this->LegendActor->SetLookupTable((this->lutFilter));
  //
  this->LegendActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
  this->LegendActor->GetPositionCoordinate()->SetValue(0.1,0.05);
  this->LegendActor->SetOrientationToVertical();
  this->LegendActor->SetWidth(0.1);
  this->LegendActor->SetHeight(0.9);
  this->LegendActor->SetPosition(0.01,0.1);
  this->LegendActor->SetLabelFormat("%6.3f");
  this->LegendActor->SetVisibility( 0 );
}

//----------------------------------------------------------------------------------------
vtkUnstructuredGrid* vtkMimxMeshActor::GetDataSet()
{
  return this->UnstructuredGrid;
}

//----------------------------------------------------------------------------------------
void vtkMimxMeshActor::SetDataSet(vtkUnstructuredGrid *mesh)
{
  this->UnstructuredGrid->DeepCopy( mesh );
  // add node and element set names list
  vtkFieldData *fielddata = this->UnstructuredGrid->GetFieldData();
  if ( fielddata )
    {
    vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
      fielddata->GetAbstractArray("Element_Set_Names"));
    if (! stringarray )
      {
      stringarray = vtkStringArray::New();
      stringarray->SetName("Element_Set_Names");
      stringarray->SetNumberOfTuples(0);
      this->UnstructuredGrid->GetFieldData()->AddArray(stringarray);
      stringarray->Delete();
      }
    stringarray = vtkStringArray::SafeDownCast(
      fielddata->GetAbstractArray("Element_Set_Names"));
    // create an element set out of the initial input if no element set is present
    if(!stringarray->GetNumberOfTuples())
      {
      vtkIntArray *scalararray = vtkIntArray::New();
      scalararray->SetName("Initial_Elements");
      stringarray->InsertNextValue("Initial_Elements");
      int numCells = this->UnstructuredGrid->GetNumberOfCells();
      scalararray->SetNumberOfValues(numCells);
      int i;
      for (i=0; i<numCells; i++)
        {
        scalararray->SetValue(i,1);
        }
      this->UnstructuredGrid->GetCellData()->AddArray(scalararray);
      scalararray->Delete();
      }
    vtkStringArray *nodesetnamearray = vtkStringArray::SafeDownCast(
      fielddata->GetAbstractArray("Node_Set_Names"));
    if (! nodesetnamearray )
      {
      nodesetnamearray = vtkStringArray::New();
      nodesetnamearray->SetName("Node_Set_Names");
      stringarray->SetNumberOfTuples(0);
      this->UnstructuredGrid->GetFieldData()->AddArray(nodesetnamearray);
      nodesetnamearray->Delete();
      }
    nodesetnamearray = vtkStringArray::SafeDownCast(
      fielddata->GetAbstractArray("Node_Set_Names"));
    if(!nodesetnamearray->GetNumberOfTuples())
      {
      vtkIntArray *scalararray = vtkIntArray::New();
      scalararray->SetName("Initial_Nodes");
      nodesetnamearray->InsertNextValue("Initial_Nodes");
      int numNodes = this->UnstructuredGrid->GetNumberOfPoints();
      scalararray->SetNumberOfValues(numNodes);
      int i;
      for (i=0; i<numNodes; i++)
        {
        scalararray->SetValue(i,1);
        }
      this->UnstructuredGrid->GetPointData()->AddArray(scalararray);
      scalararray->Delete();
      }
    }
  //

  //
  vtkIntArray *scalararray = vtkIntArray::New();
  scalararray->SetName("mimxCellScalars");
  int numCells = this->UnstructuredGrid->GetNumberOfCells();
  scalararray->SetNumberOfValues(numCells);
  int i;
  for (i=0; i<numCells; i++)
    {
    scalararray->SetValue(i,i);
    }
  this->UnstructuredGrid->GetCellData()->AddArray(scalararray);
  scalararray->Delete();
  this->UnstructuredGrid->GetCellData()->SetActiveAttribute(
    "mimxCellScalars", vtkDataSetAttributes::SCALARS);
  this->UnstructuredGridMapper->SetScalarVisibility(0);
  this->UnstructuredGrid->Modified();
  this->CreateElementSetList();

  //
  vtkDataArray *dataarray = this->UnstructuredGrid->GetCellData()->GetArray("Element_Numbers");
  if(!dataarray)
    {
    vtkIntArray *elementarray = vtkIntArray::New();
    elementarray->SetName("Element_Numbers");
    elementarray->SetNumberOfValues(numCells);
    for (i=0; i<numCells; i++)
      {
      elementarray->SetValue(i, i+1);
      }
    this->UnstructuredGrid->GetCellData()->AddArray(elementarray);
    elementarray->Delete();
    }
  //
  int numNodes = this->UnstructuredGrid->GetNumberOfPoints();
  dataarray = this->UnstructuredGrid->GetPointData()->GetArray("Node_Numbers");
  if(!dataarray)
    {
    vtkIntArray *nodearray = vtkIntArray::New();
    nodearray->SetNumberOfValues(numNodes);
    nodearray->SetName("Node_Numbers");
    for (i=0; i<numNodes; i++)
      {
      nodearray->SetValue(i, i+1);
      }
    this->UnstructuredGrid->GetPointData()->AddArray(nodearray);
    nodearray->Delete();
    }
  //check for the element types
  vtkCellTypes *cellTypes = vtkCellTypes::New();
  this->UnstructuredGrid->GetCellTypes(cellTypes);
  if(cellTypes->GetNumberOfTypes() == 1)
    {
    if(cellTypes->GetCellType(0) == VTK_QUAD || cellTypes->GetCellType(0) == VTK_TRIANGLE)
      {
      this->MeshType = vtkMimxMeshActor::SurfaceMesh;
      }
    else
      {
      this->MeshType = vtkMimxMeshActor::VolumeMesh;
      }
    }
  else
    {
    this->MeshType = vtkMimxMeshActor::MixedMesh;
    }
}


//----------------------------------------------------------------------------------
void vtkMimxMeshActor::DeleteNodeSet(const char *Name)
{
  vtkFieldData *fielddata = this->UnstructuredGrid->GetFieldData();
  vtkPointData *pointdata = this->UnstructuredGrid->GetPointData();

  vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
    fielddata->GetAbstractArray("Node_Set_Names"));

  if(!stringarray)        return;

  vtkDataArray *datasetarray;

  datasetarray = pointdata->GetArray(Name);
  if(datasetarray)        pointdata->RemoveArray(Name);
        
  // for displacement related field data
  char DispX[256], DispY[256], DispZ[256];

  strcpy(DispX, Name);
  strcpy(DispY, Name);
  strcpy(DispZ, Name);

  strcat(DispX, "_Displacement_X");
  strcat(DispY, "_Displacement_Y");
  strcat(DispZ, "_Displacement_Z");

  datasetarray = fielddata->GetArray(DispX);
  if(datasetarray)        fielddata->RemoveArray(DispX);

  datasetarray = fielddata->GetArray(DispY);
  if(datasetarray)        fielddata->RemoveArray(DispY);

  datasetarray = fielddata->GetArray(DispZ);
  if(datasetarray)        fielddata->RemoveArray(DispZ);

  // for Force related field data
  char ForceX[256], ForceY[256], ForceZ[256];

  strcpy(ForceX, Name);
  strcpy(ForceY, Name);
  strcpy(ForceZ, Name);

  strcat(ForceX, "_Force_X");
  strcat(ForceY, "_Force_Y");
  strcat(ForceZ, "_Force_Z");

  datasetarray = fielddata->GetArray(ForceX);
  if(datasetarray)        fielddata->RemoveArray(ForceX);

  datasetarray = fielddata->GetArray(ForceY);
  if(datasetarray)        fielddata->RemoveArray(ForceY);

  datasetarray = fielddata->GetArray(ForceZ);
  if(datasetarray)        fielddata->RemoveArray(ForceZ);

  // for Rotation related field data
  char RotationX[256], RotationY[256], RotationZ[256];

  strcpy(RotationX, Name);
  strcpy(RotationY, Name);
  strcpy(RotationZ, Name);

  strcat(RotationX, "_Rotation_X");
  strcat(RotationY, "_Rotation_Y");
  strcat(RotationZ, "_Rotation_Z");

  datasetarray = fielddata->GetArray(RotationX);
  if(datasetarray)        fielddata->RemoveArray(RotationX);

  datasetarray = fielddata->GetArray(RotationY);
  if(datasetarray)        fielddata->RemoveArray(RotationY);

  datasetarray = fielddata->GetArray(RotationZ);
  if(datasetarray)        fielddata->RemoveArray(RotationZ);

  // for Moment related field data
  char MomentX[256], MomentY[256], MomentZ[256];

  strcpy(MomentX, Name);
  strcpy(MomentY, Name);
  strcpy(MomentZ, Name);

  strcat(MomentX, "_Moment_X");
  strcat(MomentY, "_Moment_Y");
  strcat(MomentZ, "_Moment_Z");

  datasetarray = fielddata->GetArray(MomentX);
  if(datasetarray)        fielddata->RemoveArray(MomentX);

  datasetarray = fielddata->GetArray(MomentY);
  if(datasetarray)        fielddata->RemoveArray(MomentY);

  datasetarray = fielddata->GetArray(MomentZ);
  if(datasetarray)        fielddata->RemoveArray(MomentZ);

  vtkStringArray *temparray = vtkStringArray::New();
  temparray->DeepCopy(stringarray);
  stringarray->Initialize();
  int i;

  for (i=0; i<temparray->GetNumberOfTuples(); i++)
    {
    if(strcmp(temparray->GetValue(i), Name))
      {
      stringarray->InsertNextValue(temparray->GetValue(i));
      }
    }
  temparray->Delete();
}
//----------------------------------------------------------------------------------
void vtkMimxMeshActor::DeleteElementSet(const char *Name)
{
  vtkFieldData *fielddata = this->UnstructuredGrid->GetFieldData();
  vtkCellData *celldata = this->UnstructuredGrid->GetCellData();

  vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
    fielddata->GetAbstractArray("Element_Set_Names"));

  if(!stringarray)        return;

  vtkDataArray *datasetarray;
        
  datasetarray = celldata->GetArray(Name);
  if(datasetarray)        celldata->RemoveArray(Name);

  char Young[256];
  strcpy(Young, Name);
  strcat(Young, "_Constant_Youngs_Modulus");
  datasetarray = fielddata->GetArray(Young);
  if(datasetarray)        fielddata->RemoveArray(Young);

  char Poisson[256];
  strcpy(Poisson, Name);
  strcat(Poisson, "_Constant_Poissons_Ratio");
  datasetarray = fielddata->GetArray(Poisson);
  if(datasetarray)        fielddata->RemoveArray(Poisson);
        
  char ImageBased[256];
  strcpy(ImageBased, Name);
  strcat(ImageBased, "_Image_Based_Material_Property");
  datasetarray = celldata->GetArray(ImageBased);
  if(datasetarray)        celldata->RemoveArray(ImageBased);
        
  strcat(ImageBased, "_ReBin");
  datasetarray = celldata->GetArray(ImageBased);
  if(datasetarray)        celldata->RemoveArray(ImageBased);

  vtkStringArray *temparray = vtkStringArray::New();
  temparray->DeepCopy(stringarray);
  stringarray->Initialize();
  int i;

  for (i=0; i<temparray->GetNumberOfTuples(); i++)
    {
    if(strcmp(temparray->GetValue(i), Name))
      {
      stringarray->InsertNextValue(temparray->GetValue(i));
      }
    }
  temparray->Delete();
  this->DeleteElementSetListItem(Name);
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetDisplayMode( int mode )
{

  switch ( mode )
    {
    case vtkMimxMeshActor::DisplayMesh:
      this->DisplayMode = mode;
      UpdateElementSetDisplay();
      UpdateMeshDisplay();
      break;
    case vtkMimxMeshActor::DisplayElementSets:
      this->DisplayMode = mode;
      UpdateMeshDisplay();
      UpdateElementSetDisplay();
      break;
    }
  
}

//----------------------------------------------------------------------------------
int vtkMimxMeshActor::GetDisplayMode( )
{
  return this->DisplayMode;
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::UpdateMeshDisplay()
{
  if ((this->DisplayMode == vtkMimxMeshActor::DisplayMesh) && (this->IsVisible == true))
    {
    switch ( this->DisplayType )
      {
      case vtkMimxMeshActor::DisplaySurface:
        this->Actor->SetVisibility( 1 );
        //this->InteriorActor->SetVisibility( 1 );
        this->OutlineActor->SetVisibility( 0 );
        break;
      case vtkMimxMeshActor::DisplayOutline:
        this->Actor->SetVisibility( 0 );
        //this->InteriorActor->SetVisibility( 0 );
        this->OutlineActor->SetVisibility( 1 );
        break;
      case vtkMimxMeshActor::DisplaySurfaceAndOutline:
        this->Actor->SetVisibility( 1 );
        //this->InteriorActor->SetVisibility( 1 );
        this->OutlineActor->SetVisibility( 1 );
        break;
      }
    }
  else
    {
    this->Actor->SetVisibility( 0 );
    this->InteriorActor->SetVisibility( 0 );
    this->OutlineActor->SetVisibility( 0 );
    }
    
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::UpdateElementSetDisplay()
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin(); it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    if (( currentSet->IsVisible == true ) && 
        (this->DisplayMode == vtkMimxMeshActor::DisplayElementSets)  &&
        (this->IsVisible == true) )
      {
      switch ( currentSet->DisplayType )
        {
        case vtkMimxMeshActor::DisplaySurface:
          currentSet->SurfaceActor->SetVisibility( 1 );
          //currentSet->InteriorActor->SetVisibility( 1 );
          currentSet->OutlineActor->SetVisibility( 0 );
          break;
        case vtkMimxMeshActor::DisplayOutline:
          currentSet->SurfaceActor->SetVisibility( 0 );
          //currentSet->InteriorActor->SetVisibility( 0 );
          currentSet->OutlineActor->SetVisibility( 1 );
          break;
        case vtkMimxMeshActor::DisplaySurfaceAndOutline:
          currentSet->SurfaceActor->SetVisibility( 1 );
          //currentSet->InteriorActor->SetVisibility( 1 );
          currentSet->OutlineActor->SetVisibility( 1 );
          break;
        }
      }
    else
      {
      currentSet->SurfaceActor->SetVisibility( 0 );
      currentSet->InteriorActor->SetVisibility( 0 );
      currentSet->OutlineActor->SetVisibility( 0 );
      }
    }
  
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetMeshDisplayType( int type)
{
  switch ( type )
    {
    case vtkMimxMeshActor::DisplaySurface:
    case vtkMimxMeshActor::DisplayOutline:
    case vtkMimxMeshActor::DisplaySurfaceAndOutline:
      this->DisplayType = type;
      break;
    }
  UpdateElementSetDisplay();
  UpdateMeshDisplay();
}


//----------------------------------------------------------------------------------
int vtkMimxMeshActor::GetMeshDisplayType( )
{
  return this->DisplayType;
}


//----------------------------------------------------------------------------------
void vtkMimxMeshActor::GetMeshOutlineColor(double &red, double &green, double &blue)
{
  this->OutlineActor->GetProperty()->GetColor(red, green, blue);
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::GetMeshOutlineColor(double rgb[3])
{
  this->OutlineActor->GetProperty()->GetColor(rgb);
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetMeshOutlineColor(double red, double green, double blue)
{
  this->OutlineActor->GetProperty()->SetColor(red, green, blue);
  this->OutlineActor->GetProperty()->SetEdgeColor(red, green, blue);
  this->OutlineActor->Modified();
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetMeshOutlineColor(double rgb[3])
{
  this->SetMeshOutlineColor(rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------------
double vtkMimxMeshActor::GetMeshOutlineRadius( )
{
  return this->TubeFilter->GetRadius( );
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetMeshOutlineRadius(double radius)
{
  this->TubeFilter->SetRadius(radius/100.0);
  this->OutlineActor->Modified();
}

//----------------------------------------------------------------------------------
double vtkMimxMeshActor::GetMeshShrinkFactor( )
{
  return this->ShrinkFilter->GetShrinkFactor();
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetMeshShrinkFactor(double shrinkFactor)
{
  if ((this->Actor->GetProperty()->GetOpacity() != 1.0) && (shrinkFactor == 1.0) && (!this->CuttingPlaneEnabled))
    this->UnstructuredGridMapper->SetInput(this->UnstructuredGrid);
  else
    this->UnstructuredGridMapper->SetInput(this->ShrinkFilter->GetOutput());
  this->UnstructuredGridMapper->Modified();
                
  this->ElementShrinkFactor = shrinkFactor;
  this->ShrinkFilter->SetShrinkFactor( shrinkFactor );
  this->Actor->Modified();
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetMeshColor(double red, double green, double blue)
{
  this->Actor->GetProperty()->SetColor(red, green, blue);
  this->Actor->Modified();
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetMeshColor(double rgb[3])
{
  this->SetMeshColor(rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::GetMeshColor(double &red, double &green, double &blue)
{
  this->Actor->GetProperty()->GetColor(red, green, blue);
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::GetMeshColor(double rgb[3])
{
  this->Actor->GetProperty()->GetColor(rgb);
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetMeshOpacity(double opacity)
{
  if ((opacity != 1.0) && (this->ElementShrinkFactor == 1.0) && (!this->CuttingPlaneEnabled))
    this->UnstructuredGridMapper->SetInput(this->UnstructuredGrid);
  else
    this->UnstructuredGridMapper->SetInput(this->ShrinkFilter->GetOutput());
  this->UnstructuredGridMapper->Modified();

  this->Actor->GetProperty()->SetOpacity( opacity );
  this->Actor->Modified();
}

//----------------------------------------------------------------------------------
double vtkMimxMeshActor::GetMeshOpacity( )
{
  return this->Actor->GetProperty()->GetOpacity( );
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetMeshScalarVisibility(bool visibility)
{
  this->UnstructuredGridMapper->SetScalarVisibility(static_cast<int>(visibility));
  this->Actor->Modified();
}

//----------------------------------------------------------------------------------
bool vtkMimxMeshActor::GetMeshScalarVisibility( )
{
  bool visibility = false;
  if(this->UnstructuredGridMapper->GetScalarVisibility( ))        visibility = true;
  return visibility;
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetMeshLegendVisibility(bool visible)
{
  if (!this->LegendActor)
    {
    this->BuildScalarBar();
    }
  this->LegendActor->SetVisibility(static_cast<int>(visible));
  this->LegendActor->Modified();
}
//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetElementSetLegendVisibility(std::string setName, bool visible)
{
  this->HideAllElementSetLegends();
  std::list<MeshDisplayProperty*>::iterator it;

  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;

    if (setName == currentSet->name)
      {
      currentSet->LegendActor->SetVisibility(static_cast<int>(visible));
      this->Renderer->AddViewProp(currentSet->LegendActor);
      currentSet->LegendActor->Modified();
      break;
      }
    }
}
//----------------------------------------------------------------------------------
bool vtkMimxMeshActor::GetMeshLegendVisibility( )
{
  bool visibility = false;
    if (!this->LegendActor)
    {
    this->BuildScalarBar();
    }
  if (this->LegendActor->GetVisibility( ) )
    {
    visibility = true;
    }
  return visibility;
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetMeshScalarName(std::string attributeName)
{
  double* range;
  range = this->ComputeMeshScalarRange(attributeName.c_str());
  
  if ( range ) this->GenerateMeshMapperLookUpTable(attributeName.c_str(), range);
  this->Actor->Modified();
  if (!this->LegendActor)
    {
    this->BuildScalarBar();
    }
  this->LegendActor->SetTitle( attributeName.c_str() );
}

//----------------------------------------------------------------------------------
std::string vtkMimxMeshActor::GetMeshScalarName( )
{
  return this->activeAttribute;
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::EnableMeshCuttingPlane( )
{
  this->CuttingPlaneWidget->UpdatePlacement( );
  this->CuttingPlaneWidget->PlaceWidget();
  if(this->Renderer)
    {
    vtkCamera *camera = this->Renderer->IsActiveCameraCreated() ? this->Renderer->GetActiveCamera() : NULL;

    double normal[3];
    camera->GetViewPlaneNormal(normal);
    if(this->InvertCuttingPlane)
      this->CuttingPlaneWidget->SetNormal(-normal[0], -normal[1], -normal[2]);
    else
      this->CuttingPlaneWidget->SetNormal(normal[0], normal[1], normal[2]);
    }
  this->CuttingPlaneWidget->SetEnabled( 1 );
  this->CuttingPlaneWidget->GetPlane( this->CuttingPlane );
  this->CuttingPlaneEnabled = true;

  this->ShrinkFilter->SetInput( this->ClipPlaneGeometryFilter->GetOutput() );
  this->OutlineGeometryFilter->SetInput( this->ClipPlaneGeometryFilter->GetOutput() );
  this->InteriorShrinkFilter->SetInput( this->ClipPlaneGeometryFilter->GetOutput() );
  this->UnstructuredGridMapper->SetInput(this->ShrinkFilter->GetOutput());
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::DisableMeshCuttingPlane( )
{
  this->CuttingPlaneWidget->SetEnabled( 0 );
  this->CuttingPlaneEnabled = false;
  
  this->ShrinkFilter->SetInput(this->UnstructuredGrid);
  this->OutlineGeometryFilter->SetInput( this->UnstructuredGrid );
  this->InteriorShrinkFilter->SetInput( this->UnstructuredGrid );
  
  if ((this->Actor->GetProperty()->GetOpacity( ) != 1.0) && (this->ElementShrinkFactor == 1.0) && (!this->CuttingPlaneEnabled))
    this->UnstructuredGridMapper->SetInput(this->UnstructuredGrid);
  else
    this->UnstructuredGridMapper->SetInput(this->ShrinkFilter->GetOutput());
  this->UnstructuredGridMapper->Modified();
        
}
//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetInvertCuttingPlane( bool invert )
{
  if(this->InvertCuttingPlane != static_cast<int> ( invert ))
    {
    this->InvertCuttingPlane = static_cast<int> ( invert );
    double normal[3];
    this->CuttingPlaneWidget->GetNormal(normal);
    this->CuttingPlaneWidget->SetNormal(-normal[0], -normal[1], -normal[2]);
    this->CuttingPlaneWidget->GetPlane(this->CuttingPlane);
    this->ClipPlaneGeometryFilter->Modified();
    this->ClipPlaneGeometryFilter->Update();
    }
}
//----------------------------------------------------------------------------------
bool vtkMimxMeshActor::GetInvertCuttingPlane( )
{
  bool visibility = false;
  if(this->InvertCuttingPlane)    visibility = true;
  return visibility; 
}

//----------------------------------------------------------------------------------
bool vtkMimxMeshActor::GetMeshVisibility()
{
  return this->IsVisible;
}


//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SaveMeshVisibility()
{
  this->SavedVisibility =  this->IsVisible;
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::RestoreMeshVisibility()
{
  if (this->SavedVisibility)
   this->ShowMesh(); 
  else
    this->HideMesh();
}


//----------------------------------------------------------------------------------
void vtkMimxMeshActor::ShowMesh()
{
  this->IsVisible = true;
  UpdateElementSetDisplay();
  UpdateMeshDisplay();
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::HideMesh()
{
  this->IsVisible = false;
  UpdateMeshDisplay();
  UpdateElementSetDisplay();
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetElementSetDisplayType( std::string setName, int type )
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      switch ( type )
        {
        case vtkMimxMeshActor::DisplaySurface:
        case vtkMimxMeshActor::DisplayOutline:
        case vtkMimxMeshActor::DisplaySurfaceAndOutline:
          currentSet->DisplayType = type;
          break;
        }
      break;
      }
    }
  UpdateMeshDisplay();
  UpdateElementSetDisplay();
}


//----------------------------------------------------------------------------------
int vtkMimxMeshActor::GetElementSetDisplayType( std::string setName )
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      return currentSet->DisplayType;
      }
    }
  return vtkMimxMeshActor::DisplaySurfaceAndOutline;
}


//----------------------------------------------------------------------------------
void vtkMimxMeshActor::GetElementSetOutlineColor(std::string setName, double &red, double &green, double &blue)
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    if (setName == currentSet->name)
      {
      currentSet->OutlineActor->GetProperty()->GetColor(red, green, blue);
      return;
      }
    }
  red = green = blue = 0.0;
  return;
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::GetElementSetOutlineColor(std::string setName, double rgb[3])
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    if (setName == currentSet->name)
      {
      currentSet->OutlineActor->GetProperty()->GetColor(rgb);
      return;
      }
    }
  rgb[0] = rgb[1] = rgb[2] = 0.0;
  return;
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetElementSetOutlineColor(std::string setName, double red, double green, double blue)
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      currentSet->OutlineActor->GetProperty()->SetColor(red, green, blue);
      currentSet->OutlineActor->GetProperty()->SetEdgeColor(red, green, blue);
      currentSet->OutlineActor->Modified();
      return;
      }
    }
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetElementSetOutlineColor(std::string setName, double rgb[3])
{
  this->SetElementSetOutlineColor(setName, rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------------
double vtkMimxMeshActor::GetElementSetShrinkFactor( std::string setName )
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      return currentSet->ShrinkFilter->GetShrinkFactor();
      }
    }
  
  return 0.0;
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetElementSetShrinkFactor(std::string setName, double shrinkFactor)
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      currentSet->ShrinkFilter->SetShrinkFactor( shrinkFactor );
      currentSet->SurfaceActor->Modified();
      return;
      }
    }
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetElementSetColor(std::string setName, double red, double green, double blue)
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      currentSet->SurfaceActor->GetProperty()->SetColor(red, green, blue);
      currentSet->SurfaceActor->Modified();
      return;
      }
    }
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetElementSetColor(std::string setName, double rgb[3])
{
  this->SetElementSetColor(setName, rgb[0], rgb[1], rgb[2]);
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::GetElementSetColor(std::string setName, double &red, double &green, double &blue)
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      currentSet->SurfaceActor->GetProperty()->GetColor(red, green, blue);
      return;
      }
    }
  red = green = blue = 0.0;
  return;  
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::GetElementSetColor(std::string setName, double rgb[3])
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      currentSet->SurfaceActor->GetProperty()->GetColor(rgb);
      return;
      }
    }
  rgb[0] = rgb[1] = rgb[2] = 0.0;
  return;
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetElementSetOpacity(std::string setName, double opacity)
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      currentSet->SurfaceActor->GetProperty()->SetOpacity( opacity );
      currentSet->SurfaceActor->Modified();
      return;
      }
    }
}

//----------------------------------------------------------------------------------
double vtkMimxMeshActor::GetElementSetOpacity( std::string setName )
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      return currentSet->SurfaceActor->GetProperty()->GetOpacity( );
      }
    }
  
  return 0.0;
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::ShowElementSet( std::string setName )
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      currentSet->IsVisible = true;
      this->SetDisplayMode(vtkMimxMeshActor::DisplayElementSets);
      break;
      }
    } 
  UpdateMeshDisplay();
  UpdateElementSetDisplay();
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::HideElementSet( std::string setName )
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      currentSet->IsVisible = false;
      break;
      }
    }
  
  UpdateMeshDisplay();
  UpdateElementSetDisplay();
}
//----------------------------------------------------------------------------------
void vtkMimxMeshActor::HideAllElementSets( )
{
  std::list<MeshDisplayProperty*>::iterator it;

  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    currentSet->IsVisible = false;
    }
  UpdateMeshDisplay();
  UpdateElementSetDisplay();
}
//----------------------------------------------------------------------------------
bool vtkMimxMeshActor::GetElementSetVisibility( std::string setName )
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      return currentSet->IsVisible;
      break;
      }
    }
  return false;
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetElementSetOutlineRadius( std::string setName, double radius )
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      currentSet->TubeFilter->SetRadius(radius/100.0);
      currentSet->OutlineActor->Modified();
      break;
      }
    }  
}

//----------------------------------------------------------------------------------
double vtkMimxMeshActor::GetElementSetOutlineRadius( std::string setName )
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      return currentSet->TubeFilter->GetRadius();
      break;
      }
    }
  
  return 0.0;
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::CreateElementSetList( )
{
  while (!ElementSetDisplayList.empty())
    {
    MeshDisplayProperty *currentSet = ElementSetDisplayList.front();
    if (currentSet->SurfaceActor) currentSet->SurfaceActor->Delete();
    if (currentSet->OutlineActor) currentSet->OutlineActor->Delete();
    if (currentSet->InteriorActor) currentSet->InteriorActor->Delete();
    if (currentSet->SurfaceMapper) currentSet->SurfaceMapper->Delete();
    if (currentSet->OutlineMapper) currentSet->OutlineMapper->Delete();
    if (currentSet->InteriorMapper) currentSet->InteriorMapper->Delete();
    if (currentSet->ShrinkFilter) currentSet->ShrinkFilter->Delete();
    if (currentSet->LegendActor)
      {
      vtkWarningMacro("current set LegendActor Delete");
      currentSet->LegendActor->Delete();
      }
    if (currentSet->lutFilter) currentSet->lutFilter->Delete();
    ElementSetDisplayList.pop_front();
    }
  
  this->NumberOfElementSets = 0;
  vtkFieldData *fielddata = this->UnstructuredGrid->GetFieldData();
  if ( fielddata )
    {
    vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
      fielddata->GetAbstractArray("Element_Set_Names"));
    if ( stringarray )
      {
      this->NumberOfElementSets = stringarray->GetNumberOfValues();
      for (int i=0;i<stringarray->GetNumberOfValues();i++)
        {
        vtkStdString name = stringarray->GetValue(i);  
        AddElementSetListItem( name );
        }
      }
    }
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::DeleteElementSetListItem( std::string setName )
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      if (currentSet->SurfaceActor) currentSet->SurfaceActor->Delete();
      if (currentSet->OutlineActor) currentSet->OutlineActor->Delete();
      if (currentSet->InteriorActor) currentSet->InteriorActor->Delete();
      if (currentSet->SurfaceMapper) currentSet->SurfaceMapper->Delete();
      if (currentSet->OutlineMapper) currentSet->OutlineMapper->Delete();
      if (currentSet->InteriorMapper) currentSet->InteriorMapper->Delete();
      if (currentSet->ShrinkFilter) currentSet->ShrinkFilter->Delete();
      ElementSetDisplayList.erase(it);
      return;
      }
    }
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::AddElementSetListItem( std::string setName )
{
  MeshDisplayProperty *elementSetProperty =  new MeshDisplayProperty;   
  elementSetProperty->name = setName;
  elementSetProperty->IsVisible = false;
  elementSetProperty->DisplayType = vtkMimxMeshActor::DisplaySurfaceAndOutline;
  
  /* Extract Cells */
  vtkCellData *celldata = this->UnstructuredGrid->GetCellData();
  vtkIntArray *datasetarray = vtkIntArray::SafeDownCast(celldata->GetArray(setName.c_str()));
  vtkIdList *cellIds = vtkIdList::New();
  for (int i=0;i<datasetarray->GetNumberOfTuples();i++)
    {
    if (datasetarray->GetValue(i) > 0 ) cellIds->InsertNextId(i);
    }
  elementSetProperty->ExtractCellsFilter = vtkExtractCells::New();
  elementSetProperty->ExtractCellsFilter->SetInput( this->UnstructuredGrid );
  elementSetProperty->ExtractCellsFilter->SetCellList( cellIds );
 
  elementSetProperty->ShrinkFilter = vtkShrinkFilter::New();
  elementSetProperty->ShrinkFilter->SetInput(elementSetProperty->ExtractCellsFilter->GetOutput());
  elementSetProperty->ShrinkFilter->SetShrinkFactor(1.0);

  elementSetProperty->ShrinkFilter->Update();
  elementSetProperty->SurfaceMapper = vtkDataSetMapper::New();
  elementSetProperty->SurfaceMapper->SetInputConnection(elementSetProperty->ShrinkFilter->GetOutputPort());

  elementSetProperty->SurfaceMapper->SetScalarVisibility(0);

  elementSetProperty->SurfaceActor = vtkActor::New();
  elementSetProperty->SurfaceActor->SetMapper(elementSetProperty->SurfaceMapper);
  double redRand, greenRand, blueRand;
  redRand = static_cast<double> ( rand() ) / static_cast<double> ( RAND_MAX );
  greenRand = static_cast<double> ( rand() ) / static_cast<double> ( RAND_MAX );
  blueRand = static_cast<double> ( rand() ) / static_cast<double> ( RAND_MAX );
  elementSetProperty->SurfaceActor->GetProperty()->SetColor(redRand,greenRand,blueRand);
  
  /* Create the Outlines */
  elementSetProperty->GeometryFilter = vtkGeometryFilter::New();
  elementSetProperty->GeometryFilter->SetInput( elementSetProperty->ExtractCellsFilter->GetOutput() );

  // TODO: these will leak:
  vtkFeatureEdges* featureEdges = vtkFeatureEdges::New();
  featureEdges->SetInput( elementSetProperty->GeometryFilter->GetOutput() );
  featureEdges->BoundaryEdgesOn();
  featureEdges->ManifoldEdgesOn();
  featureEdges->FeatureEdgesOff();
  featureEdges->ColoringOff();
  
  elementSetProperty->TubeFilter = vtkTubeFilter::New();
  elementSetProperty->TubeFilter->SetInputConnection(featureEdges->GetOutputPort());
  elementSetProperty->TubeFilter->SetRadius(0.03);
  
  elementSetProperty->OutlineMapper = vtkPolyDataMapper::New();
  elementSetProperty->OutlineMapper->SetInputConnection( elementSetProperty->TubeFilter->GetOutputPort() );
  elementSetProperty->OutlineMapper->SetScalarVisibility( 0 );
  
  elementSetProperty->OutlineActor = vtkActor::New();
  elementSetProperty->OutlineActor->SetMapper( elementSetProperty->OutlineMapper );
  elementSetProperty->OutlineActor->GetProperty()->SetColor(0.0,0.0,0.0);
  elementSetProperty->OutlineActor->GetProperty()->SetRepresentationToSurface();
  elementSetProperty->OutlineActor->GetProperty()->SetAmbient(0);

  /* Create the Interior View */
  // TODO: this will leak
  elementSetProperty->InteriorShrinkFilter = vtkShrinkFilter::New();
  elementSetProperty->InteriorShrinkFilter->SetInput( elementSetProperty->ExtractCellsFilter->GetOutput() );
  elementSetProperty->InteriorShrinkFilter->SetShrinkFactor(0.995);
  
  elementSetProperty->InteriorMapper = vtkDataSetMapper::New();
  elementSetProperty->InteriorMapper->SetInputConnection(elementSetProperty->InteriorShrinkFilter->GetOutputPort());
  elementSetProperty->InteriorMapper->ScalarVisibilityOff();
  
  elementSetProperty->InteriorActor = vtkActor::New();
  elementSetProperty->InteriorActor->SetMapper( elementSetProperty->InteriorMapper );
  elementSetProperty->InteriorActor->GetProperty()->SetColor(0.5,0.5,0.5);
  elementSetProperty->InteriorActor->GetProperty()->SetOpacity(0.25);
  elementSetProperty->InteriorActor->GetProperty()->SetRepresentationToWireframe();
  elementSetProperty->InteriorActor->GetProperty()->SetAmbient(1.0);
  elementSetProperty->InteriorActor->SetVisibility(0);
  /* for display of scalar values of the element set*/
  elementSetProperty->lutFilter = NULL;
  vtkWarningMacro("elemetn set property LegendActor  vtkScalarBarActor::New()");
  elementSetProperty->LegendActor = vtkScalarBarActor::New();
  vtkTextProperty *textProperty = elementSetProperty->LegendActor->GetTitleTextProperty();
  textProperty->SetFontFamilyToArial();
  textProperty->SetColor( TextColor );
  textProperty->ShadowOff();
  textProperty->ItalicOff();
  //textProperty->SetFontSize(40);

  elementSetProperty->LegendActor->SetTitleTextProperty( textProperty );
  elementSetProperty->LegendActor->SetLabelTextProperty( textProperty );
  elementSetProperty->LegendActor->SetTitle("Quality");
  elementSetProperty->LegendActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
  elementSetProperty->LegendActor->GetPositionCoordinate()->SetValue(0.1,0.05);
  elementSetProperty->LegendActor->SetOrientationToVertical();
  elementSetProperty->LegendActor->SetWidth(0.1);
  elementSetProperty->LegendActor->SetHeight(0.9);
  elementSetProperty->LegendActor->SetPosition(0.01,0.1);
  elementSetProperty->LegendActor->SetLabelFormat("%6.3f");
  elementSetProperty->LegendActor->SetVisibility( 0 );

  if(this->Renderer)
    {
    this->Renderer->AddViewProp(elementSetProperty->SurfaceActor);
    this->Renderer->AddViewProp(elementSetProperty->OutlineActor);
    this->Renderer->AddViewProp(elementSetProperty->InteriorActor);
    elementSetProperty->SurfaceActor->SetVisibility(0);
    elementSetProperty->OutlineActor->SetVisibility(0);
    elementSetProperty->InteriorActor->SetVisibility(0);
    }
  /* Add the Display to the List */
  if (ElementSetDisplayList.size() == 0)
    elementSetProperty->IsVisible = true;
  ElementSetDisplayList.push_back( elementSetProperty );
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetRenderer(vtkRenderer *renderer)
{
  this->Renderer = renderer;
  this->Renderer->AddViewProp(this->Actor);
  this->Renderer->AddViewProp(this->OutlineActor);
  this->Renderer->AddViewProp(this->InteriorActor);
  if (!this->LegendActor)
    {
    this->BuildScalarBar();
    }
  this->Renderer->AddViewProp(this->LegendActor);
    
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    { 
    MeshDisplayProperty *currentSet = *it;
    this->Renderer->AddViewProp(currentSet->SurfaceActor);
    this->Renderer->AddViewProp(currentSet->OutlineActor);
    this->Renderer->AddViewProp(currentSet->InteriorActor);
    }
  
  UpdateElementSetDisplay();
  UpdateMeshDisplay(); 
}

//----------------------------------------------------------------------------------
vtkRenderer* vtkMimxMeshActor::GetRenderer( )
{
  return (this->Renderer);
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetInteractor(vtkRenderWindowInteractor *interactor)
{
  this->Interactor = interactor;
  this->CuttingPlaneWidget->SetInteractor( interactor );
  this->CuttingPlaneWidget->SetEnabled( 0 );
  
  /***VAM ***/
  vtkPlaneWidgetEventCallback* PlaneMoveCallback = vtkPlaneWidgetEventCallback::New();
  PlaneMoveCallback->PlaneInstance = this->CuttingPlane;
  this->CuttingPlaneWidget->AddObserver(vtkCommand::InteractionEvent,PlaneMoveCallback);  
  this->ClipPlaneGeometryFilter->SetImplicitFunction(this->CuttingPlane);
}

//----------------------------------------------------------------------------------
vtkRenderWindowInteractor* vtkMimxMeshActor::GetInteractor( )
{
  return (this->Interactor);
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetLegendRange(double min, double max)
{
  if (!this->LegendActor)
    {
    this->BuildScalarBar();
    }
  this->lutFilter->SetTableRange(min, max);
  this->lutFilter->ForceBuild();
  this->UnstructuredGridMapper->SetScalarRange(min, max);
  this->LegendActor->SetLookupTable(this->lutFilter);
  this->LegendActor->Modified();
}
//---------------------------------------------------------------------------------
void vtkMimxMeshActor::SetElementSetLegendRange(double min, double max, 
                                                MeshDisplayProperty *currentSet)
{
  if(!currentSet) return;
  currentSet->lutFilter->SetTableRange(min, max);
  currentSet->lutFilter->ForceBuild();
  currentSet->SurfaceMapper->SetScalarRange(min, max);
  currentSet->SurfaceMapper->Modified();
  currentSet->LegendActor->SetLookupTable(currentSet->lutFilter);
  currentSet->LegendActor->Modified();
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetAllElementSetScalarName( std::string scalarName )
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    double *range = ComputeElementSetScalarRange(currentSet->name.c_str(), scalarName.c_str());
    if ( range ) this->GenerateElementSetMapperLookUpTable(currentSet->name.c_str(), scalarName.c_str(), range);
        
    currentSet->SurfaceActor->Modified();
    currentSet->LegendActor->SetTitle( scalarName.c_str() );
    }
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetElementSetScalarName(std::string setName, std::string scalarName)
{
  std::list<MeshDisplayProperty*>::iterator it;
        
  MeshDisplayProperty *currentSet;
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    currentSet = *it;
    if (setName == currentSet->name)
      {
      double *range = ComputeElementSetScalarRange(setName.c_str(), scalarName.c_str());
      if ( range ) this->GenerateElementSetMapperLookUpTable(setName.c_str(), scalarName.c_str(), range);
        
      currentSet->SurfaceActor->Modified();
      currentSet->LegendActor->SetTitle( scalarName.c_str() );
      break;
      }
    }
}

//----------------------------------------------------------------------------------
std::string vtkMimxMeshActor::GetElementSetScalarName( std::string setName )
{
  return this->GetMeshScalarName( );
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::EnableElementSetCuttingPlane( std::string setName )
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      this->ClipPlaneGeometryFilter->SetInput( currentSet->ExtractCellsFilter->GetOutput() );
      this->CuttingPlaneWidget->UpdatePlacement( );
      this->CuttingPlaneWidget->PlaceWidget();
      if(this->Renderer)
        {
        vtkCamera *camera = this->Renderer->IsActiveCameraCreated() ? this->Renderer->GetActiveCamera() : NULL;
        double normal[3];
        camera->GetViewPlaneNormal(normal);
        if(this->InvertCuttingPlane)
          this->CuttingPlaneWidget->SetNormal(-normal[0], -normal[1], -normal[2]);
        else
          this->CuttingPlaneWidget->SetNormal(normal[0], normal[1], normal[2]);
        }
      this->CuttingPlaneWidget->SetEnabled( 1 );
      this->CuttingPlaneWidget->GetPlane( this->CuttingPlane );
      this->CuttingPlaneEnabled = true;
      
      currentSet->ShrinkFilter->SetInput( this->ClipPlaneGeometryFilter->GetOutput() );
      currentSet->GeometryFilter->SetInput( this->ClipPlaneGeometryFilter->GetOutput() );
      currentSet->InteriorShrinkFilter->SetInput( this->ClipPlaneGeometryFilter->GetOutput() );
      break;
      }
    }  
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::DisableElementSetCuttingPlane( std::string setName )
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      this->CuttingPlaneWidget->SetEnabled( 0 );
      this->CuttingPlaneEnabled = false;
      this->ClipPlaneGeometryFilter->SetInput( this->UnstructuredGrid );
      
      currentSet->ShrinkFilter->SetInput( currentSet->ExtractCellsFilter->GetOutput() );
      currentSet->GeometryFilter->SetInput( currentSet->ExtractCellsFilter->GetOutput() );
      currentSet->InteriorShrinkFilter->SetInput( currentSet->ExtractCellsFilter->GetOutput() );
      this->Actor->Modified();
      break;
      }
    }  
}
//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetAllElementSetScalarVisibility( bool visibility )
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    currentSet->SurfaceMapper->SetScalarVisibility(static_cast<int>(visibility));
    currentSet->SurfaceActor->Modified();
    }
}
//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetElementSetScalarVisibility(std::string setName, bool visibility)
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      currentSet->SurfaceMapper->SetScalarVisibility(static_cast<int>(visibility));
      currentSet->SurfaceActor->Modified();
      break;
      }
    }
}
//----------------------------------------------------------------------------------
bool vtkMimxMeshActor::GetElementSetScalarVisibility( std::string setName )
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      bool visibility = false;
      if(currentSet->SurfaceMapper->GetScalarVisibility( ))   visibility = true;
      return visibility; 
      break;
      }
    }
  return false;

}
      
//----------------------------------------------------------------------------------
void vtkMimxMeshActor::DeleteBoundaryConditionStep(int StepNum)
{
  int i,j;
  if(StepNum < 1) return;
  vtkUnstructuredGrid *ugrid = this->UnstructuredGrid;
  vtkIntArray *boundCond = vtkIntArray::SafeDownCast(
    ugrid->GetFieldData()->GetArray("Boundary_Condition_Number_Of_Steps"));
  if(!boundCond)  return;
  int numSteps = boundCond->GetValue(0);
  if(StepNum > numSteps)  return;
        
  char charStepNum0[10];
  sprintf(charStepNum0, "%d", StepNum);

  vtkStringArray *nodesetnamestring = vtkStringArray::SafeDownCast(
    ugrid->GetFieldData()->GetAbstractArray("Node_Set_Names"));
  if(!nodesetnamestring)  return;

  vtkStringArray *elementsetnamestring = vtkStringArray::SafeDownCast(
    ugrid->GetFieldData()->GetAbstractArray("Element_Set_Names"));

  vtkFloatArray *floatarray;
  vtkFieldData *fieldData = ugrid->GetFieldData();

  char stepParameters0[128];
  sprintf(stepParameters0, "Step_%d_%s", StepNum, "Input_Parameters");
  if(fieldData->GetAbstractArray(stepParameters0)) fieldData->RemoveArray(stepParameters0);

  char subHeading0[128];
  sprintf(subHeading0, "Step_%d_%s", StepNum, "SubHeading");
  if(fieldData->GetAbstractArray(subHeading0))             fieldData->RemoveArray(subHeading0);

  for(i=0; i<nodesetnamestring->GetNumberOfValues(); i++)
    {
    const char* nodesetname =  nodesetnamestring->GetValue(i);

    char Concatenate1[256];
    this->ConcatenateStrings("Step", charStepNum0, nodesetname, "Force", "X", Concatenate1);
    floatarray = vtkFloatArray::SafeDownCast(
      ugrid->GetFieldData()->GetArray(Concatenate1));
    if(floatarray)
      {
      fieldData->RemoveArray(Concatenate1);
      }
    //
    char Concatenate2[256];
    this->ConcatenateStrings("Step", charStepNum0, nodesetname, "Force", "Y", Concatenate2);
    floatarray = vtkFloatArray::SafeDownCast(
      ugrid->GetFieldData()->GetArray(Concatenate2));
    if(floatarray)
      {
      fieldData->RemoveArray(Concatenate2);
      }
    //
    char Concatenate3[256];
    this->ConcatenateStrings("Step", charStepNum0, nodesetname, "Force", "Z", Concatenate3);
    floatarray = vtkFloatArray::SafeDownCast(
      ugrid->GetFieldData()->GetArray(Concatenate3));
    if(floatarray)
      {
      fieldData->RemoveArray(Concatenate3);
      }
    //
    char Concatenate4[256];
    this->ConcatenateStrings("Step", charStepNum0, nodesetname, "Displacement", "X", Concatenate4);
    floatarray = vtkFloatArray::SafeDownCast(
      ugrid->GetFieldData()->GetArray(Concatenate4));
    if(floatarray)
      {
      fieldData->RemoveArray(Concatenate4);
      }
    //
    char Concatenate5[256];
    this->ConcatenateStrings("Step", charStepNum0, nodesetname, "Displacement", "Y", Concatenate5);
    floatarray = vtkFloatArray::SafeDownCast(
      ugrid->GetFieldData()->GetArray(Concatenate5));
    if(floatarray)
      {
      fieldData->RemoveArray(Concatenate5);
      }
    //
    char Concatenate6[256];
    this->ConcatenateStrings("Step", charStepNum0, nodesetname, "Displacement", "Z", Concatenate6);
    floatarray = vtkFloatArray::SafeDownCast(
      ugrid->GetFieldData()->GetArray(Concatenate6));
    if(floatarray)
      {
      fieldData->RemoveArray(Concatenate6);
      }
    //
    char Concatenate7[256];
    this->ConcatenateStrings("Step", charStepNum0, nodesetname, "Rotation", "X", Concatenate7);
    floatarray = vtkFloatArray::SafeDownCast(
      ugrid->GetFieldData()->GetArray(Concatenate7));
    if(floatarray)
      {
      fieldData->RemoveArray(Concatenate7);
      }
    //
    char Concatenate8[256];
    this->ConcatenateStrings("Step", charStepNum0, nodesetname, "Rotation", "Y", Concatenate8);
    floatarray = vtkFloatArray::SafeDownCast(
      ugrid->GetFieldData()->GetArray(Concatenate8));
    if(floatarray)
      {
      fieldData->RemoveArray(Concatenate8);
      }
    //
    char Concatenate9[256];
    this->ConcatenateStrings("Step", charStepNum0, nodesetname, "Rotation", "Z", Concatenate9);
    floatarray = vtkFloatArray::SafeDownCast(
      ugrid->GetFieldData()->GetArray(Concatenate9));
    if(floatarray)
      {
      fieldData->RemoveArray(Concatenate9);
      }
    //
    char Concatenate10[256];
    this->ConcatenateStrings("Step", charStepNum0, nodesetname, "Moment", "X", Concatenate10);
    floatarray = vtkFloatArray::SafeDownCast(
      ugrid->GetFieldData()->GetArray(Concatenate10));
    if(floatarray)
      {
      fieldData->RemoveArray(Concatenate10);
      }
    //
    char Concatenate11[256];
    this->ConcatenateStrings("Step", charStepNum0, nodesetname, "Moment", "Y", Concatenate11);
    floatarray = vtkFloatArray::SafeDownCast(
      ugrid->GetFieldData()->GetArray(Concatenate11));
    if(floatarray)
      {
      fieldData->RemoveArray(Concatenate11);
      }
    //
    char Concatenate12[256];
    this->ConcatenateStrings("Step", charStepNum0, nodesetname, "Moment", "Z", Concatenate12);
    floatarray = vtkFloatArray::SafeDownCast(
      ugrid->GetFieldData()->GetArray(Concatenate12));
    if(floatarray)
      {
      fieldData->RemoveArray(Concatenate12);
      }
    // for all the node print and output statements
    char nodePrint[128];
    sprintf(nodePrint, "Step_%d_%s_%s_%s", StepNum, "Node_Set", nodesetname, "Print");
    if(fieldData->GetAbstractArray(nodePrint))      fieldData->RemoveArray(nodePrint);

    char nodeOutput[128];
    sprintf(nodeOutput, "Step_%d_%s_%s_%s", StepNum, "Node_Set", nodesetname, "Output");
    if(fieldData->GetAbstractArray(nodeOutput))     fieldData->RemoveArray(nodeOutput);
    }
  //
  for (i=0; i<elementsetnamestring->GetNumberOfValues(); i++)
    {
    const char *elementsetname = elementsetnamestring->GetValue(i);
    char elementPrint[128];
    sprintf(elementPrint, "Step_%d_%s_%s_%s", StepNum, "Element_Set", elementsetname, "Print");
    if(fieldData->GetAbstractArray(elementPrint))   fieldData->RemoveArray(elementPrint);

    char elementOutput[128];
    sprintf(elementOutput, "Step_%d_%s_%s_%s", StepNum, "Element_Set", elementsetname, "Output");
    if(fieldData->GetAbstractArray(elementOutput))  fieldData->RemoveArray(elementOutput);
    }
  // loop through all the higher numbered steps to change the step numbers
  for (j=StepNum+1; j<=numSteps;j++)
    {
    char stepParameters[128];
    sprintf(stepParameters, "Step_%d_%s", j, "Input_Parameters");
    char stepParametersNew[128];
    sprintf(stepParametersNew, "Step_%d_%s", j-1, "Input_Parameters");
    if(fieldData->GetAbstractArray(stepParameters)) fieldData->GetAbstractArray(stepParameters)->SetName(stepParametersNew);

    char subHeading[128];
    sprintf(subHeading, "Step_%d_%s", j, "SubHeading");
    char subHeadingNew[128];
    sprintf(subHeadingNew, "Step_%d_%s", j-1, "SubHeading");
    if(fieldData->GetAbstractArray(subHeading))             fieldData->GetAbstractArray(subHeading)->SetName(subHeadingNew);

    char charStepNum[10];
    char charStepNumNew[10];
    sprintf(charStepNum, "%d", j);
    sprintf(charStepNumNew, "%d", j-1);
    //itoa(j, charStepNum, 10);
    //itoa(j-1, charStepNumNew, 10);
    for(i=0; i<nodesetnamestring->GetNumberOfValues(); i++)
      {
      const char* nodesetname =  nodesetnamestring->GetValue(i);
                        
      char Concatenate13[256];
      char ConcatenateNew13[256];
      this->ConcatenateStrings("Step", charStepNum, nodesetname, "Force", "X", Concatenate13);
      this->ConcatenateStrings("Step", charStepNumNew, nodesetname, "Force", "X", ConcatenateNew13);
      floatarray = vtkFloatArray::SafeDownCast(
        ugrid->GetFieldData()->GetArray(Concatenate13));
      if(floatarray)
        {
        floatarray->SetName(ConcatenateNew13);
        }
      //
      char Concatenate14[256];
      char ConcatenateNew14[256];
      this->ConcatenateStrings("Step", charStepNum, nodesetname, "Force", "Y", Concatenate14);
      this->ConcatenateStrings("Step", charStepNumNew, nodesetname, "Force", "Y", ConcatenateNew14);
      floatarray = vtkFloatArray::SafeDownCast(
        ugrid->GetFieldData()->GetArray(Concatenate14));
      if(floatarray)
        {
        floatarray->SetName(ConcatenateNew14);
        }
      //
      char Concatenate15[256];
      char ConcatenateNew15[256];
      this->ConcatenateStrings("Step", charStepNum, nodesetname, "Force", "Z", Concatenate15);
      this->ConcatenateStrings("Step", charStepNumNew, nodesetname, "Force", "Z", ConcatenateNew15);
      floatarray = vtkFloatArray::SafeDownCast(
        ugrid->GetFieldData()->GetArray(Concatenate15));
      if(floatarray)
        {
        floatarray->SetName(ConcatenateNew15);
        }
      //
      char Concatenate16[256];
      char ConcatenateNew16[256];
      this->ConcatenateStrings("Step", charStepNum, nodesetname, "Displacement", "X", Concatenate16);
      this->ConcatenateStrings("Step", charStepNumNew, nodesetname, "Displacement", "X", ConcatenateNew16);
      floatarray = vtkFloatArray::SafeDownCast(
        ugrid->GetFieldData()->GetArray(Concatenate16));
      if(floatarray)
        {
        floatarray->SetName(ConcatenateNew16);
        }
      //
      char Concatenate17[256];
      char ConcatenateNew17[256];
      this->ConcatenateStrings("Step", charStepNum, nodesetname, "Displacement", "Y", Concatenate17);
      this->ConcatenateStrings("Step", charStepNumNew, nodesetname, "Displacement", "Y", ConcatenateNew17);
      floatarray = vtkFloatArray::SafeDownCast(
        ugrid->GetFieldData()->GetArray(Concatenate17));
      if(floatarray)
        {
        floatarray->SetName(ConcatenateNew17);
        }
      //
      char Concatenate18[256];
      char ConcatenateNew18[256];
      this->ConcatenateStrings("Step", charStepNum, nodesetname, "Displacement", "Z", Concatenate18);
      this->ConcatenateStrings("Step", charStepNumNew, nodesetname, "Displacement", "Z", ConcatenateNew18);
      floatarray = vtkFloatArray::SafeDownCast(
        ugrid->GetFieldData()->GetArray(Concatenate18));
      if(floatarray)
        {
        floatarray->SetName(ConcatenateNew18);
        }
      //
      char Concatenate19[256];
      char ConcatenateNew19[256];
      this->ConcatenateStrings("Step", charStepNum, nodesetname, "Rotation", "X", Concatenate19);
      this->ConcatenateStrings("Step", charStepNumNew, nodesetname, "Rotation", "X", ConcatenateNew19);
      floatarray = vtkFloatArray::SafeDownCast(
        ugrid->GetFieldData()->GetArray(Concatenate19));
      if(floatarray)
        {
        floatarray->SetName(ConcatenateNew19);
        }
      //
      char Concatenate20[256];
      char ConcatenateNew20[256];
      this->ConcatenateStrings("Step", charStepNum, nodesetname, "Rotation", "Y", Concatenate20);
      this->ConcatenateStrings("Step", charStepNumNew, nodesetname, "Rotation", "Y", ConcatenateNew20);
      floatarray = vtkFloatArray::SafeDownCast(
        ugrid->GetFieldData()->GetArray(Concatenate20));
      if(floatarray)
        {
        floatarray->SetName(ConcatenateNew20);
        }
      //
      char Concatenate21[256];
      char ConcatenateNew21[256];
      this->ConcatenateStrings("Step", charStepNum, nodesetname, "Rotation", "Z", Concatenate21);
      this->ConcatenateStrings("Step", charStepNumNew, nodesetname, "Rotation", "Z", ConcatenateNew21);
      floatarray = vtkFloatArray::SafeDownCast(
        ugrid->GetFieldData()->GetArray(Concatenate21));
      if(floatarray)
        {
        floatarray->SetName(ConcatenateNew21);
        }
      //
      char Concatenate22[256];
      char ConcatenateNew22[256];
      this->ConcatenateStrings("Step", charStepNum, nodesetname, "Moment", "X", Concatenate22);
      this->ConcatenateStrings("Step", charStepNumNew, nodesetname, "Moment", "X", ConcatenateNew22);
      floatarray = vtkFloatArray::SafeDownCast(
        ugrid->GetFieldData()->GetArray(Concatenate22));
      if(floatarray)
        {
        floatarray->SetName(ConcatenateNew22);
        }
      //
      char Concatenate23[256];
      char ConcatenateNew23[256];
      this->ConcatenateStrings("Step", charStepNum, nodesetname, "Moment", "Y", Concatenate23);
      this->ConcatenateStrings("Step", charStepNumNew, nodesetname, "Moment", "Y", ConcatenateNew23);
      floatarray = vtkFloatArray::SafeDownCast(
        ugrid->GetFieldData()->GetArray(Concatenate23));
      if(floatarray)
        {
        floatarray->SetName(ConcatenateNew23);
        }
      //
      char Concatenate24[256];
      char ConcatenateNew24[256];
      this->ConcatenateStrings("Step", charStepNum, nodesetname, "Moment", "Z", Concatenate24);
      this->ConcatenateStrings("Step", charStepNumNew, nodesetname, "Moment", "Z", ConcatenateNew24);
      floatarray = vtkFloatArray::SafeDownCast(
        ugrid->GetFieldData()->GetArray(Concatenate24));
      if(floatarray)
        {
        floatarray->SetName(ConcatenateNew24);
        }
      // for all the node print and output statements
      char nodePrint[128];
      sprintf(nodePrint, "Step_%d_%s_%s_%s", j, "Node_Set", nodesetname, "Print");
      char nodePrintNew[128];
      sprintf(nodePrintNew, "Step_%d_%s_%s_%s", j-1, "Node_Set", nodesetname, "Print");
      if(fieldData->GetAbstractArray(nodePrint))      fieldData->GetAbstractArray(nodePrint)->SetName(nodePrintNew);

      char nodeOutput[128];
      sprintf(nodeOutput, "Step_%d_%s_%s_%s", j, "Node_Set", nodesetname, "Output");
      char nodeOutputNew[128];
      sprintf(nodeOutputNew, "Step_%d_%s_%s_%s", j-1, "Node_Set", nodesetname, "Output");
      if(fieldData->GetAbstractArray(nodeOutput))     fieldData->GetAbstractArray(nodeOutput)->SetName(nodeOutputNew);
      }
    for (i=0; i<elementsetnamestring->GetNumberOfValues(); i++)
      {
      const char *elementsetname = elementsetnamestring->GetValue(i);
      char elementPrint[128];
      sprintf(elementPrint, "Step_%d_%s_%s_%s", j, "Element_Set", elementsetname, "Print");
      char elementPrintNew[128];
      sprintf(elementPrintNew, "Step_%d_%s_%s_%s", j-1, "Element_Set", elementsetname, "Print");
      if(fieldData->GetAbstractArray(elementPrint))   fieldData->GetAbstractArray(elementPrint)->SetName(elementPrintNew);

      char elementOutput[128];
      sprintf(elementOutput, "Step_%d_%s_%s_%s", j, "Element_Set", elementsetname, "Output");
      char elementOutputNew[128];
      sprintf(elementOutputNew, "Step_%d_%s_%s_%s", j-1, "Element_Set", elementsetname, "Output");
      if(fieldData->GetAbstractArray(elementOutput))  fieldData->GetAbstractArray(elementOutput)->SetName(elementOutputNew);
      }
    }
  boundCond->SetValue(0, numSteps-1);
}
//----------------------------------------------------------------------------------
void vtkMimxMeshActor::ConcatenateStrings(const char* Step, const char* Num, 
                                          const char* NodeSetName, const char* Type, const char* Direction, char *Name)
{
  strcpy(Name, Step);
  strcat(Name, "_");
  strcat(Name,Num);
  strcat(Name, "_");
  strcat(Name, NodeSetName);
  strcat(Name, "_");
  strcat(Name,Type);
  strcat(Name, "_");
  strcat(Name, Direction);
}
//----------------------------------------------------------------------------------
void vtkMimxMeshActor::ChangeElementSetNumbers(const char *ElSetName, int StartEleNum)
{
  vtkCellData *cellData = this->UnstructuredGrid->GetCellData();
  vtkDataArray *dataArray = cellData->GetArray(ElSetName);
  if(!dataArray)  return;
  vtkIntArray *elsetValues = vtkIntArray::SafeDownCast(dataArray);
  if(!elsetValues)        return;
  vtkDataArray *elementNumbers = cellData->GetArray("Element_Numbers");
  if(!elementNumbers)     return;
  vtkIntArray *elementNumbersInt = vtkIntArray::SafeDownCast(elementNumbers);
  if(!elementNumbersInt)  return;

  int i;
  int numCells = this->UnstructuredGrid->GetNumberOfCells();
  for (i=0; i<numCells; i++)
    {
    int belongs = elsetValues->GetValue(i);
    if(belongs)
      {
      elementNumbersInt->SetValue(i, StartEleNum++);
      }
    }
}
//----------------------------------------------------------------------------------
void vtkMimxMeshActor::ChangeNodeSetNumbers(const char *NodeSetName, int StartNodeNum)
{
  vtkPointData *pointData = this->UnstructuredGrid->GetPointData();
  vtkDataArray *dataArray = pointData->GetArray(NodeSetName);
  if(!dataArray)  return;
  vtkIntArray *nodesetValues = vtkIntArray::SafeDownCast(dataArray);
  if(!nodesetValues)      return;
  vtkDataArray *nodeNumbers = pointData->GetArray("Node_Numbers");
  if(!nodeNumbers)        return;
  vtkIntArray *nodeNumbersInt = vtkIntArray::SafeDownCast(nodeNumbers);
  if(!nodeNumbersInt)     return;

  int i;
  int numPoints = this->UnstructuredGrid->GetNumberOfPoints();
  for (i=0; i<numPoints; i++)
    {
    int belongs = nodesetValues->GetValue(i);
    if(belongs)
      {
      nodeNumbersInt->SetValue(i, StartNodeNum++);
      }
    }
}

//----------------------------------------------------------------------------------
void vtkMimxMeshActor::SetLegendTextColor(double color[3])
{
  TextColor[0] = color[0];
  TextColor[1] = color[1];
  TextColor[2] = color[2];
  if (!this->LegendActor)
    {
    this->BuildScalarBar();
    }
  vtkTextProperty *textProperty = this->LegendActor->GetTitleTextProperty();
  textProperty->SetColor( TextColor );
  
  this->LegendActor->SetTitleTextProperty( textProperty );
  this->LegendActor->SetLabelTextProperty( textProperty );
}

//----------------------------------------------------------------------------------
double *vtkMimxMeshActor::GetLegendTextColor( )
{
  return TextColor;
}
  
//----------------------------------------------------------------------------------
void vtkMimxMeshActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------------
void vtkMimxMeshActor::CalculateAverageEdgeLength()
{
  if(!this->UnstructuredGrid)     return;
  int numNodes, numCells;
  numNodes = this->UnstructuredGrid->GetNumberOfPoints();
  numCells = this->UnstructuredGrid->GetNumberOfCells();
  if(!numCells || !numNodes)      return;

  double cumdist = 0.0;
  vtkUnstructuredGrid *ugrid = this->UnstructuredGrid;
  int i,j;
  int count = 0;
  for (i=0; i<numCells; i++)
    {
    vtkCell *cell = ugrid->GetCell(i);
    for (j=0; j<cell->GetNumberOfEdges(); j++)
      {
      vtkCell *edge = cell->GetEdge(j);
      vtkIdList *ptids = edge->GetPointIds();
      int pt1 = ptids->GetId(0);
      int pt2 = ptids->GetId(1);
      double p1[3], p2[3];
      ugrid->GetPoint(pt1, p1);       ugrid->GetPoint(pt2, p2);
      cumdist = cumdist + sqrt(vtkMath::Distance2BetweenPoints(p1, p2));
      count ++;
      }
    }
  this->AverageEdgeLength = cumdist/count;
  this->IsAverageEdgeLengthCalculated = 1;
}
//----------------------------------------------------------------------------------
vtkPointSet* vtkMimxMeshActor::GetPointSetOfNodeSet(const char* NodeSetName)
{
  if(!this->UnstructuredGrid->GetPointData()->GetArray(NodeSetName))      return NULL;
  vtkIntArray *intarray = vtkIntArray::SafeDownCast(
    this->UnstructuredGrid->GetPointData()->GetArray(NodeSetName));

  vtkPoints *points = vtkPoints::New();
  if(!this->PointSetOfNodeSet)    this->PointSetOfNodeSet = vtkUnstructuredGrid::New();
  else    this->PointSetOfNodeSet->Initialize();
        
  int numPoints = this->UnstructuredGrid->GetNumberOfPoints();

  for (int i=0; i<numPoints; i++)
    {
    if(intarray->GetValue(i))
      {
      points->InsertNextPoint(this->UnstructuredGrid->GetPoint(i));
      }
    }
  this->PointSetOfNodeSet->SetPoints(points);
  points->Delete();
  return this->PointSetOfNodeSet;
}
//-----------------------------------------------------------------------------------
void vtkMimxMeshActor::StoreConstantMaterialProperty(
  const char* ElSetName, double YoungMod)
{
  int i;

  char imagebased[256];
  strcpy(imagebased, ElSetName);
  strcat(imagebased, "_Image_Based_Material_Property");

  vtkUnstructuredGrid *ugrid = this->UnstructuredGrid;

  vtkIntArray *intarray = vtkIntArray::SafeDownCast(
    ugrid->GetCellData()->GetArray(ElSetName));
  if(!intarray)   return;
  vtkDoubleArray *matarray = vtkDoubleArray::SafeDownCast(
    ugrid->GetCellData()->GetArray(imagebased));

  if(matarray)
    {
    ugrid->GetCellData()->RemoveArray(imagebased);
    strcat(imagebased, "_ReBin");

    matarray = vtkDoubleArray::SafeDownCast(ugrid->GetCellData()->GetArray(imagebased));

    if(matarray)    ugrid->GetCellData()->RemoveArray(imagebased);
    }

  matarray = vtkDoubleArray::SafeDownCast(ugrid->GetCellData()->GetArray("Youngs_Modulus"));
  int numCells = ugrid->GetNumberOfCells();
  if(!matarray)
    {
    matarray = vtkDoubleArray::New();
    matarray->SetNumberOfValues(numCells);
    matarray->SetName("Youngs_Modulus");
    for (i=0; i<numCells; i++)
      {
      matarray->SetValue(i, -9999);
      }
    ugrid->GetCellData()->AddArray(matarray);
    matarray->Delete();
    }
  matarray = vtkDoubleArray::SafeDownCast(ugrid->GetCellData()->GetArray("Youngs_Modulus"));
        
  for (i=0; i< numCells; i++)
    {
    if(intarray->GetValue(i))
      matarray->SetValue(i, YoungMod);
    }

  char young[256];
  strcpy(young, ElSetName);
  strcat(young, "_Constant_Youngs_Modulus");

  vtkDoubleArray *Earray = vtkDoubleArray::SafeDownCast(
    ugrid->GetFieldData()->GetArray(young));        

  if(Earray)
    {
    Earray->SetValue(0, YoungMod);
    }
  else
    {
    Earray = vtkDoubleArray::New();
    Earray->SetName(young);
    Earray->InsertNextValue(YoungMod);
    ugrid->GetFieldData()->AddArray(Earray);
    Earray->Delete();
    }
}
//------------------------------------------------------------------------------------
void vtkMimxMeshActor::StoreImageBasedMaterialProperty(const char *ElSetName)
{
  vtkUnstructuredGrid *ugrid = this->UnstructuredGrid;
  char str[256];
  int i;
  strcpy(str, ElSetName);
  strcat(str, "_Image_Based_Material_Property");
  vtkDoubleArray *matarray = vtkDoubleArray::SafeDownCast(
    ugrid->GetCellData()->GetArray(str));
  if(!matarray)   return;

  vtkIntArray *intarray = vtkIntArray::SafeDownCast(
    ugrid->GetCellData()->GetArray(ElSetName));
  if(!intarray)   return;

  vtkDoubleArray *youngsmodulus = vtkDoubleArray::SafeDownCast(
    ugrid->GetCellData()->GetArray("Youngs_Modulus"));
  int numCells = ugrid->GetNumberOfCells();
  if(!youngsmodulus)
    {
    youngsmodulus = vtkDoubleArray::New();
    youngsmodulus->SetNumberOfValues(numCells);
    youngsmodulus->SetName("Youngs_Modulus");
    for (i=0; i<numCells; i++)
      {
      youngsmodulus->SetValue(i, -9999);
      }
    ugrid->GetCellData()->AddArray(youngsmodulus);
    youngsmodulus->Delete();
    }
  youngsmodulus = vtkDoubleArray::SafeDownCast(ugrid->GetCellData()->GetArray("Youngs_Modulus")); 

  for (i=0; i< numCells; i++)
    {
    if(intarray->GetValue(i))
      youngsmodulus->SetValue(i, matarray->GetValue(i));
    }
}
//-------------------------------------------------------------------------------------
void vtkMimxMeshActor::StoreConstantPoissonsRatio(
  const char *ElSetName, double PoissonRatio)
{
  char poisson[256];
  strcpy(poisson, ElSetName);
  strcat(poisson, "_Constant_Poissons_Ratio");

  vtkUnstructuredGrid *ugrid = this->UnstructuredGrid;

  vtkFloatArray *Nuarray = vtkFloatArray::SafeDownCast(
    ugrid->GetFieldData()->GetArray(poisson));

  if(Nuarray)
    {
    Nuarray->SetValue(0, PoissonRatio);
    }
  else
    {
    Nuarray = vtkFloatArray::New();
    Nuarray->SetName(poisson);
    Nuarray->InsertNextValue(PoissonRatio);
    ugrid->GetFieldData()->AddArray(Nuarray);
    Nuarray->Delete();
    }
}
//-------------------------------------------------------------------------------------
void vtkMimxMeshActor::StoreImageBasedMaterialPropertyReBin(const char *ElSetName)
{
  vtkUnstructuredGrid *ugrid = this->UnstructuredGrid;
  char str[256];
  int i;
  strcpy(str, ElSetName);
  strcat(str, "_Image_Based_Material_Property_ReBin");
  vtkDoubleArray *matarray = vtkDoubleArray::SafeDownCast(
    ugrid->GetCellData()->GetArray(str));
  if(!matarray)   return;

  vtkIntArray *intarray = vtkIntArray::SafeDownCast(
    ugrid->GetCellData()->GetArray(ElSetName));
  if(!intarray)   return;

  vtkDoubleArray *youngsmodulus = vtkDoubleArray::SafeDownCast(
    ugrid->GetCellData()->GetArray("Youngs_Modulus"));
  int numCells = ugrid->GetNumberOfCells();
  if(!youngsmodulus)
    {
    youngsmodulus = vtkDoubleArray::New();
    youngsmodulus->SetNumberOfValues(numCells);
    youngsmodulus->SetName("Youngs_Modulus");
    for (i=0; i<numCells; i++)
      {
      matarray->SetValue(i, -9999);
      }
    ugrid->GetCellData()->AddArray(youngsmodulus);
    youngsmodulus->Delete();
    }
  youngsmodulus = vtkDoubleArray::SafeDownCast(ugrid->GetCellData()->GetArray("Youngs_Modulus")); 

  for (i=0; i< numCells; i++)
    {
    if(intarray->GetValue(i))
      youngsmodulus->SetValue(i, matarray->GetValue(i));
    }
}
//-------------------------------------------------------------------------------------
double* vtkMimxMeshActor::ComputeElementSetScalarRange(
  const char* ElSetName, const char* ArrayName)
{
  double* range = new double[2];
  double min = VTK_FLOAT_MAX;
  double max = VTK_FLOAT_MIN;

  vtkUnstructuredGrid *ugrid = this->UnstructuredGrid;

  vtkDoubleArray *doublearray = vtkDoubleArray::SafeDownCast(
    ugrid->GetCellData()->GetArray(ArrayName));
  if(!doublearray)        return NULL;

  vtkIntArray *intarray = vtkIntArray::SafeDownCast(
    ugrid->GetCellData()->GetArray(ElSetName));

  if(!intarray)   return NULL;

  int i;
  int numCells = ugrid->GetNumberOfCells();
  for (i=0; i<numCells; i++)
    {
    if(intarray->GetValue(i))
      {
      double val = doublearray->GetValue(i);
      if(val != -9999)
        {
        if(val > max)   max = val;
        if(val < min)   min = val;
        }
      }
    }
  range[0] = min;
  range[1] = max;
  return range;
}
//--------------------------------------------------------------------------------------
double* vtkMimxMeshActor::ComputeMeshScalarRange(const char* ArrayName)
{
  double* range = new double[2];
  double min = VTK_FLOAT_MAX;
  double max = VTK_FLOAT_MIN;
 
  vtkUnstructuredGrid *ugrid = this->UnstructuredGrid;

  vtkDoubleArray *doublearray = vtkDoubleArray::SafeDownCast(
    ugrid->GetCellData()->GetArray(ArrayName));
  if(!doublearray)        
    {
    range[0] = min;
    range[1] = max;
    return range;
    }
  int i;
  int numCells = ugrid->GetNumberOfCells();
  for (i=0; i<numCells; i++)
    {
    double val = doublearray->GetValue(i);
    if(val != -9999)
      {
      if(this->MeshType == vtkMimxMeshActor::MixedMesh)
        {
        if(this->UnstructuredGrid->GetCellType(i) != VTK_QUAD &&
           this->UnstructuredGrid->GetCellType(i) != VTK_TRIANGLE)
          {
          if(val > max)   max = val;
          if(val < min)   min = val;
          }
        }
      else
        {
        if(val > max)   max = val;
        if(val < min)   min = val;
        }
      }
    }
  range[0] = min;
  range[1] = max;
  return range;
}
//--------------------------------------------------------------------------------------
void vtkMimxMeshActor::ShowHideAllElementSets(bool Show)
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    currentSet->IsVisible = Show;
    }
  
  UpdateMeshDisplay();
  UpdateElementSetDisplay();
}
//-------------------------------------------------------------------------------------
void vtkMimxMeshActor::SetElementSetScalarRangeFromCompleteMesh(const char *setName, const char* scalarName)
{
  double* range;
  range = this->ComputeMeshScalarRange(scalarName);

  MeshDisplayProperty *currentSet = GetMeshDisplayProperty(setName);
  if(!currentSet) return;

  this->SetElementSetLegendRange(range[0], range[1], currentSet);

//      currentSet->ShrinkFilter->Modified();
  currentSet->LegendActor->SetTitle( scalarName);
  this->Renderer->AddViewProp(currentSet->SurfaceActor);
  this->Renderer->AddViewProp(currentSet->OutlineActor);
  this->Renderer->AddViewProp(currentSet->LegendActor);
  currentSet->LegendActor->SetVisibility(0);
}
//-------------------------------------------------------------------------------------
void vtkMimxMeshActor::SetElementSetScalarRangeFromElementSet(const char *setName, const char* scalarName)
{
  double* range;
  range = this->ComputeElementSetScalarRange(setName, scalarName);
  MeshDisplayProperty *currentSet = GetMeshDisplayProperty(setName);
  if(!currentSet) return;
  this->SetElementSetLegendRange(range[0], range[1], currentSet);

//      currentSet->ShrinkFilter->Modified();
  currentSet->LegendActor->SetTitle( scalarName);
  this->Renderer->AddViewProp(currentSet->SurfaceActor);
  this->Renderer->AddViewProp(currentSet->OutlineActor);
  this->Renderer->AddViewProp(currentSet->LegendActor);
  currentSet->LegendActor->SetVisibility(0);
}
//-------------------------------------------------------------------------------------
MeshDisplayProperty* vtkMimxMeshActor::GetMeshDisplayProperty(const char *setName)
{
  std::list<MeshDisplayProperty*>::iterator it;
  
  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    
    if (setName == currentSet->name)
      {
      return currentSet;
      }
    }
  return NULL;
}
//-------------------------------------------------------------------------------------
void vtkMimxMeshActor::HideAllElementSetLegends()
{
  std::list<MeshDisplayProperty*>::iterator it;

  for ( it=this->ElementSetDisplayList.begin() ; it != this->ElementSetDisplayList.end(); it++ )
    {
    MeshDisplayProperty *currentSet = *it;
    this->Renderer->RemoveViewProp(currentSet->LegendActor);
    currentSet->LegendActor->SetVisibility(0);
    currentSet->LegendActor->Modified();
    }
}
//----------------------------------------------------------------------------------------
void vtkMimxMeshActor::GenerateMeshMapperLookUpTable(const char *ArrayName, double *range)
{
  // lookuptable for the data
  vtkUnstructuredGrid *ugrid = this->UnstructuredGrid;
  vtkDoubleArray *matarray = vtkDoubleArray::SafeDownCast(ugrid->GetCellData()->GetArray(ArrayName));
  int numCells = ugrid->GetNumberOfCells();
  int i;
  if(!matarray)
    {
    matarray = vtkDoubleArray::New();
    matarray->SetNumberOfValues(numCells);
    matarray->SetName(ArrayName);
    for (i=0; i<numCells; i++)
      {
      matarray->SetValue(i, -9999);
      }
    ugrid->GetCellData()->AddArray(matarray);
    matarray->Delete();
    range[0] = VTK_FLOAT_MAX;
    range[1] = VTK_FLOAT_MIN;
    }
  matarray = vtkDoubleArray::SafeDownCast(ugrid->GetCellData()->GetArray(ArrayName));
                
  if(range[0] == VTK_FLOAT_MAX && range[1] == VTK_FLOAT_MIN)
    {
    range[0] = 0.0; range[1] = 0.0;
    }
  vtkLookupTable* Lut = vtkLookupTable::New();
  if(this->ColorRangeType == vtkMimxMeshActor::BlueToRed)
    {
    this->BlueToRedLookUpTable();
    this->MapperBlueToRedLookUpTable(Lut, ArrayName, range);
    }
  else{
  this->RedToBlueLookUpTable();
  this->MapperRedToBlueLookUpTable(Lut, ArrayName, range);
  }
  Lut->SetTableRange(0,numCells-1);
  this->UnstructuredGridMapper->SetLookupTable(Lut);
  this->UnstructuredGridMapper->SetScalarRange(0,numCells-1);
  Lut->Delete();

  if (!this->LegendActor)
    {
    this->BuildScalarBar();
    }
  this->lutFilter->SetTableRange(range[0], range[1]);
  this->lutFilter->Modified();
  this->LegendActor->SetLookupTable(this->lutFilter);
  //this->LegendActor->SetTitle(ArrayName);
  this->IsVisible = true;
  this->SetDisplayMode(vtkMimxMeshActor::DisplayMesh);
  this->UpdateMeshDisplay();
}
//----------------------------------------------------------------------------------------------------
void vtkMimxMeshActor::GenerateElementSetMapperLookUpTable(
  const char *ElementSetname, const char *ArrayName, double *range)
{
  // lookuptable for the data
  vtkUnstructuredGrid *ugrid = this->UnstructuredGrid;
  vtkDoubleArray *matarray = vtkDoubleArray::SafeDownCast(ugrid->GetCellData()->GetArray(ArrayName));
  int numCells = ugrid->GetNumberOfCells();
  int i;
  if(!matarray)
    {
    matarray = vtkDoubleArray::New();
    matarray->SetNumberOfValues(numCells);
    matarray->SetName(ArrayName);
    for (i=0; i<numCells; i++)
      {
      matarray->SetValue(i, -9999);
      }
    ugrid->GetCellData()->AddArray(matarray);
    matarray->Delete();
    }
  matarray = vtkDoubleArray::SafeDownCast(ugrid->GetCellData()->GetArray(ArrayName));

  MeshDisplayProperty *currentSet = GetMeshDisplayProperty(ElementSetname);
  if(!currentSet) return;

  if(range[0] == VTK_FLOAT_MAX && range[1] == VTK_FLOAT_MIN)
    {
    range[0] = 0.0; range[1] = 0.0;
    }

  vtkLookupTable* Lut = vtkLookupTable::New();
  if(this->ColorRangeType == vtkMimxMeshActor::BlueToRed)
    {
    this->BlueToRedLookUpTable();
    this->MapperBlueToRedLookUpTable(Lut, ArrayName, range);
    }
  else{
  this->RedToBlueLookUpTable();
  this->MapperRedToBlueLookUpTable(Lut, ArrayName, range);
  }
  Lut->SetTableRange(0,numCells-1);
  currentSet->SurfaceMapper->SetLookupTable(Lut);
  currentSet->SurfaceMapper->SetScalarRange(0,numCells-1);
  currentSet->SurfaceMapper->SetScalarVisibility(1);
  currentSet->SurfaceMapper->Modified();
  currentSet->SurfaceMapper->Update();
  Lut->Delete();
  if (!this->LegendActor)
    {
    this->BuildScalarBar();
    }
  this->lutFilter->SetTableRange(range[0], range[1]);
  this->lutFilter->Modified();
  this->LegendActor->SetLookupTable(this->lutFilter);
  this->LegendActor->SetTitle(ArrayName);
  this->LegendActor->Modified();
}
//----------------------------------------------------------------------------------------------------------------
void vtkMimxMeshActor::RedToBlueLookUpTable()
{
  if (!this->lutFilter)
    {
    this->BuildScalarBar();
    }
  this->lutFilter->SetNumberOfColors(100);
  this->lutFilter->Build();
  double red = 1.0;
  double green = 0.0;
  double blue = 0.0;
  int i;
  for (i=0; i<50; i++)
    {
    double maxc;
    if(green > red) maxc = green;
    else    maxc = red;
    double factor = 1.0/maxc;
    this->lutFilter->SetTableValue(i, red*factor,green*factor,blue*factor,1.0);
    green = green + 0.02;
    red = red - 0.02;
    }
  for (i=50; i<100; i++)
    {
    double maxc;
    if(green > blue)        maxc = green;
    else    maxc = blue;
    double factor = 1.0/maxc;
    this->lutFilter->SetTableValue(i, red*factor,green*factor,blue*factor,1.0);
    green = green - 0.02;
    blue = blue + 0.02;
    }
  this->ColorRangeType = vtkMimxMeshActor::RedToBlue;
  this->lutFilter->Modified();
}
//-----------------------------------------------------------------------------------------------------------------
void vtkMimxMeshActor::BlueToRedLookUpTable()
{
  if (!this->lutFilter)
    {
    this->BuildScalarBar();
    }
  this->lutFilter->SetNumberOfColors(100);
  this->lutFilter->Build();
  double red = 0.0;
  double green = 0.0;
  double blue = 1.0;
  int i;
  for (i=0; i<50; i++)
    {
    double maxc;
    if(green > blue)        maxc = green;
    else    maxc = blue;
    double factor = 1.0/maxc;
    this->lutFilter->SetTableValue(i, red*factor,green*factor,blue*factor,1.0);
    green = green + 0.02;
    blue = blue - 0.02;
    }
  for (i=50; i<100; i++)
    {
    double maxc;
    if(green > red) maxc = green;
    else    maxc = red;
    double factor = 1.0/maxc;
    this->lutFilter->SetTableValue(i, red*factor,green*factor,blue*factor,1.0);
    green = green - 0.02;
    red = red + 0.02;
    }
  this->ColorRangeType = vtkMimxMeshActor::BlueToRed;
  this->lutFilter->Modified();
}
//-----------------------------------------------------------------------------------------------------------------
void vtkMimxMeshActor::MapperBlueToRedLookUpTable(vtkLookupTable *Lut, const char *ArrayName, double *range)
{
  int i;
  int numCells = this->UnstructuredGrid->GetNumberOfCells();
  Lut->SetNumberOfColors(numCells);
  Lut->Build();

  vtkDoubleArray *matarray = vtkDoubleArray::SafeDownCast(
    this->UnstructuredGrid->GetCellData()->GetArray(ArrayName));

  double max = range[1];
  double min = range[0];
  double mean = min + (max-min)/2.0;
  double mag = (range[1] - range[0])/2.0;

  if(range[0] == 0 && range[1] == 0)
    {
    for(i=0; i<numCells; i++)
      {
      if(this->MeshType == vtkMimxMeshActor::MixedMesh)
        {
        if(this->UnstructuredGrid->GetCellType(i) != VTK_QUAD &&
           this->UnstructuredGrid->GetCellType(i) != VTK_TRIANGLE)
          {
          double dist = matarray->GetValue(i);
          if(dist == -9999)
            {
            Lut->SetTableValue(i,0.5,0.5,0.5,1.0);
            }
          else
            Lut->SetTableValue(i,1.0,1.0,1.0,1.0);
          }
        else
          {
          Lut->SetTableValue(i,0.5,0.5,0.5,1.0);
          }
        }
      else
        {
        double dist = matarray->GetValue(i);
        if(dist == -9999)
          {
          Lut->SetTableValue(i,0.5,0.5,0.5,1.0);
          }
        else
          Lut->SetTableValue(i,1.0,1.0,1.0,1.0);
        }
      }
    }
  else
    {
    if(range[0] == range[1])
      {
      for(i=0; i<numCells; i++)
        {
        if(this->MeshType == vtkMimxMeshActor::MixedMesh)
          {
          if(this->UnstructuredGrid->GetCellType(i) != VTK_QUAD &&
             this->UnstructuredGrid->GetCellType(i) != VTK_TRIANGLE)
            {
            double dist = matarray->GetValue(i);
            if(dist == -9999)
              {
              Lut->SetTableValue(i,0.5,0.5,0.5,1.0);
              }
            else
              Lut->SetTableValue(i,0.0,0.0,1.0,1.0);
            }
          else
            {
            Lut->SetTableValue(i,0.5,0.5,0.5,1.0);
            }
          }
        else
          {
          double dist = matarray->GetValue(i);
          if(dist == -9999)
            {
            Lut->SetTableValue(i,0.5,0.5,0.5,1.0);
            }
          else
            Lut->SetTableValue(i,0.0,0.0,1.0,1.0);
          }
        }
      }
    else
      {
      for(i=0; i <numCells; i++)
        {
        if(this->MeshType == vtkMimxMeshActor::MixedMesh && (
             this->UnstructuredGrid->GetCellType(i) == VTK_QUAD ||
             this->UnstructuredGrid->GetCellType(i) == VTK_TRIANGLE))
          {
          Lut->SetTableValue(i,0.5,0.5,0.5,1.0);
          }
        else
          {
          double dist = matarray->GetValue(i);
          if(dist == -9999)
            {
            Lut->SetTableValue(i,0.5,0.5,0.5,1.0);
            }
          else
            {
            double red, green, blue, factor;
            if(dist < min)
              {
              red = 0.0;
              green = 0.0;
              blue = 0.0;
              factor = 1.0;
              }
            else if(dist <= mean)
              {
              red = 0.0;
              green = (1.0 - (mean-dist)/mag);
              blue = (mean-dist)/mag;
              double maxc;
              if(green > blue)        maxc = green;
              else    maxc = blue;
              factor = 1.0/maxc;
              }
            else if(dist > max)
              {
              red = 1.0;
              green = 1.0;
              blue = 1.0;
              factor = 1.0;
              }
            else
              {
              red = (dist-mean)/mag;
              green = (1.0 - (dist-mean)/mag);
              blue = 0.0;
              double maxc;
              if(green > red) maxc = green;
              else    maxc = red;
              factor = 1.0/maxc;
              }
            Lut->SetTableValue( i, red*factor, green*factor, blue*factor, 1.0);
            }
          }
        }
      }
    }
}
//-----------------------------------------------------------------------------------------------------------------
void vtkMimxMeshActor::MapperRedToBlueLookUpTable(vtkLookupTable *Lut, const char *ArrayName, double *range)
{
  int i;
  int numCells = this->UnstructuredGrid->GetNumberOfCells();
  Lut->SetNumberOfColors(numCells);
  Lut->Build();

  vtkDoubleArray *matarray = vtkDoubleArray::SafeDownCast(
    this->UnstructuredGrid->GetCellData()->GetArray(ArrayName));

  double max = range[1];
  double min = range[0];
  double mean = min + (max-min)/2.0;
  double mag = (range[1] - range[0])/2.0;

  if(range[0] == 0 && range[1] == 0)
    {
    for(i=0; i<numCells; i++)
      {
      if(this->MeshType == vtkMimxMeshActor::MixedMesh)
        {
        if(this->UnstructuredGrid->GetCellType(i) != VTK_QUAD &&
           this->UnstructuredGrid->GetCellType(i) != VTK_TRIANGLE)
          {
          double dist = matarray->GetValue(i);
          if(dist == -9999)
            {
            Lut->SetTableValue(i,0.5,0.5,0.5,1.0);
            }
          else
            Lut->SetTableValue(i,1.0,1.0,1.0,1.0);
          }
        else
          {
          Lut->SetTableValue(i,0.5,0.5,0.5,1.0);
          }
        }
      else
        {
        double dist = matarray->GetValue(i);
        if(dist == -9999)
          {
          Lut->SetTableValue(i,0.5,0.5,0.5,1.0);
          }
        else
          Lut->SetTableValue(i,1.0,1.0,1.0,1.0);
        }
      }
    }
  else
    {
    if(range[0] == range[1])
      {
      for(i=0; i<numCells; i++)
        {
        if(this->MeshType == vtkMimxMeshActor::MixedMesh)
          {
          if(this->UnstructuredGrid->GetCellType(i) != VTK_QUAD &&
             this->UnstructuredGrid->GetCellType(i) != VTK_TRIANGLE)
            {
            double dist = matarray->GetValue(i);
            if(dist == -9999)
              {
              Lut->SetTableValue(i,0.5,0.5,0.5,1.0);
              }
            else
              Lut->SetTableValue(i,1.0,0.0,0.0,1.0);
            }
          else
            {
            Lut->SetTableValue(i,0.5,0.5,0.5,1.0);
            }
          }
        else
          {
          double dist = matarray->GetValue(i);
          if(dist == -9999)
            {
            Lut->SetTableValue(i,0.5,0.5,0.5,1.0);
            }
          else
            Lut->SetTableValue(i,1.0,0.0,0.0,1.0);
          }
        }
      }
    else
      {
      for(i=0; i <numCells; i++)
        {
        if(this->MeshType == vtkMimxMeshActor::MixedMesh && (
             this->UnstructuredGrid->GetCellType(i) == VTK_QUAD ||
             this->UnstructuredGrid->GetCellType(i) == VTK_TRIANGLE))
          {
          Lut->SetTableValue(i,0.5,0.5,0.5,1.0);
          }
        else
          {

          double dist = matarray->GetValue(i);
          if(dist == -9999)
            {
            Lut->SetTableValue(i,1.0,1.0,1.0,1.0);
            }
          else
            {
            double red, green, blue, factor;
            if(dist < min)
              {
              red = 0.0;
              green = 0.0;
              blue = 0.0;
              factor = 1.0;
              }
            else if(dist <= mean)
              {
              red = (mean-dist)/mag;
              green = (1.0 - (mean-dist)/mag);
              blue = 0.0;
              double maxc;
              if(green > red) maxc = green;
              else    maxc = red;
              factor = 1.0/maxc;
              }
            else if(dist > max)
              {
              red = 1.0;
              green = 1.0;
              blue = 1.0;
              factor = 1.0;
              }
            else
              {
              blue = (dist-mean)/mag;
              green = (1.0 - (dist-mean)/mag);
              red = 0.0;
              double maxc;
              if(green > blue)        maxc = green;
              else    maxc = blue;
              factor = 1.0/maxc;
              }
            Lut->SetTableValue( i, red*factor, green*factor, blue*factor, 1.0);
            }
          }
        }
      }
    }
}
//-----------------------------------------------------------------------------------------------------------------
void vtkMimxMeshActor::SetColorRangeType(int RangeType, const char *ArrayName, 
                                         const char *aElementSetName, double *Range)
{
  this->ColorRangeType = RangeType;
  if(this->DisplayMode == vtkMimxMeshActor::DisplayMesh)
    {
    this->GenerateMeshMapperLookUpTable(ArrayName, Range);
    }
  else
    {
    this->GenerateElementSetMapperLookUpTable(aElementSetName, ArrayName, Range);
    }
}
//-----------------------------------------------------------------------------------------------------------------
void vtkMimxMeshActor::SetLegendTitle(const char *title)
{
  if (!this->LegendActor)
    {
    this->BuildScalarBar();
    }
  this->LegendActor->SetTitle( title );
}
//-----------------------------------------------------------------------------------------------------------------
const char *vtkMimxMeshActor::GetLegendTitle( )
{
  if (!this->LegendActor)
    {
    this->BuildScalarBar();
    }
  return this->LegendActor->GetTitle( );
}
//-----------------------------------------------------------------------------------------------------------------
void vtkMimxMeshActor::SetLegendPrecision(int precision)
{
  this->LegendPrecision = precision;
  char labelformat[20];
  sprintf(labelformat,"%%6.%df",precision);
  if (!this->LegendActor)
    {
    this->BuildScalarBar();
    }
  this->LegendActor->SetLabelFormat( labelformat );
}
//-----------------------------------------------------------------------------------------------------------------
int vtkMimxMeshActor::GetLegendPrecision( )
{
  return this->LegendPrecision;
}
//-----------------------------------------------------------------------------------------------------------------
void vtkMimxMeshActor::GetCurrentScalarRange( double *range )
{
  if (!this->lutFilter)
    {
    this->BuildScalarBar();
    }
  this->lutFilter->GetTableRange( range );
}
//------------------------------------------------------------------------------------------------------------------
void vtkMimxMeshActor::AppendNodeSet(const char *NodeSetName, vtkIdList *NodeList)
{
  vtkIntArray *intArray = vtkIntArray::SafeDownCast(
    this->UnstructuredGrid->GetPointData()->GetArray(NodeSetName));
  if(!intArray)   return;
  int i;
  for (i=0; i<intArray->GetNumberOfTuples(); i++)
    {
    if(NodeList->IsId(i) != -1)     intArray->SetValue(i, 1);
    }
}
//--------------------------------------------------------------------------------------------------------------------
void vtkMimxMeshActor::AppendElementSet(const char *ElSetName, vtkIdList *ElementList)
{
  vtkIntArray *intArray = vtkIntArray::SafeDownCast(
    this->UnstructuredGrid->GetCellData()->GetArray(ElSetName));
  if(!intArray)   return;
  int i;
  for (i=0; i<intArray->GetNumberOfTuples(); i++)
    {
    if(ElementList->IsId(i) != -1)  intArray->SetValue(i, 1);
    }
  this->DeleteElementSetListItem(ElSetName);
  this->AddElementSetListItem(ElSetName);
}
//--------------------------------------------------------------------------------------------------------------------
void vtkMimxMeshActor::AddReferenceNode(const char *aElementSetName)
{
  vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
    this->UnstructuredGrid->GetFieldData()->GetAbstractArray("Node_Set_Names"));
  int i;
  char refnode[64];
  strcpy(refnode, this->FoundationName), strcat(refnode, "_RN");
  if(this->UnstructuredGrid->GetFieldData()->GetAbstractArray("Reference_Node_Parameters"))       return;

  vtkStringArray *refnodearray = vtkStringArray::New();
  refnodearray->InsertNextValue(refnode);
  refnodearray->InsertNextValue(aElementSetName);
  refnodearray->SetName("Reference_Node_Parameters");
  this->UnstructuredGrid->GetFieldData()->AddArray(refnodearray);
  refnodearray->Delete();

  for (i=0; i<stringarray->GetNumberOfTuples(); i++)
    {
    vtkIntArray *intarray = vtkIntArray::SafeDownCast(
      this->UnstructuredGrid->GetPointData()->GetArray(stringarray->GetValue(i)));
    intarray->InsertNextValue(0);
    }
  vtkIntArray *nodenumbers = vtkIntArray::SafeDownCast(this->UnstructuredGrid->GetPointData()->GetArray("Node_Numbers"));
  int nodenummax;
  nodenummax = nodenumbers->GetValue(0);
  for (i=1; i<nodenumbers->GetNumberOfTuples(); i++)
    {
    if(nodenumbers->GetValue(i) > nodenummax)
      nodenummax = nodenumbers->GetValue(i);
    }
  nodenumbers->InsertNextValue(nodenummax+1);
  // create a new nodeset array;
  // position of the reference node
  double bounds[6];
  this->UnstructuredGrid->GetBounds(bounds);
  double pts[3];
  pts[0] = (bounds[0] +bounds[1])/2.0;
  pts[1] = (bounds[2] +bounds[3])/2.0;
  pts[2] = (bounds[4] +bounds[5])/2.0;
  this->UnstructuredGrid->GetPoints()->InsertNextPoint(pts);
  vtkIntArray *nodeset = vtkIntArray::New();
  nodeset->SetName(refnode);
  for (i=0; i<this->UnstructuredGrid->GetNumberOfPoints()-1; i++)
    {
    nodeset->InsertNextValue(0);
    }
  nodeset->InsertNextValue(1);
  this->UnstructuredGrid->GetPointData()->AddArray(nodeset);
  nodeset->Delete();
  stringarray->InsertNextValue(refnode);
}
//--------------------------------------------------------------------------------------------------------------------
