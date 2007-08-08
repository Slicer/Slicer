/*
 *  mesh-quality-class.cpp
 *  tfm-cxx
 *
 *  Created by Curtis Lisle on 1/4/07.
 *  Copyright 2007 KnowledgeVis, LLC. 
 *
 */

#include "vtkMeshQualityClass.h"
#include "vtkObjectFactory.h"
#include "vtkRearrangeFields.h"
#include "vtkMeshQuality.h"
#include "vtkDataSet.h"
#include "vtkAssignAttribute.h"
#include "vtkShrinkFilter.h"
#include "vtkLookupTable.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkDataSetMapper.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkMath.h"
#include "vtkProperty.h"
#include "vtkLogLookupTable.h"
#include "vtkUnstructuredGrid.h"



//----------------------------------------------------------------------------


vtkCxxRevisionMacro(vtkMeshQualityClass, "$Revision: 1.1 $");
vtkStandardNewMacro( vtkMeshQualityClass );

//----------------------------------------------------------------
vtkMeshQualityClass::vtkMeshQualityClass(void) 
{
  this->threshIsAbsolute = 0;
  
  // default shrink size and opacity of each element 
  this->ElementShrinkFactor = 0.8;
  this->ElementOpacity = 1.0;
  this->OutlineIsEnabled = 1;
  this->outputActor = NULL;  
  this->IsInitialized = 0;
  this->SavedQualityMetric = 4;
  this->SavedMesh = NULL;
  this->SavedElementMapper = NULL;
  this->SavedQualityMetric = 1;
  
}

//----------------------------------------------------------------
vtkMeshQualityClass::~vtkMeshQualityClass(void) 
{
  cerr << "need to clean up memory yet" << endl;
}

void vtkMeshQualityClass::SetThresholdValue (double thresh) 
{
   this->thresholdValue = thresh;
   if (this->IsInitialized)
    this->UpdatePipeline();
}


//----------------------------------------------------------------
void vtkMeshQualityClass::SetElementShrinkFactor(double shrink)
{
  this->ElementShrinkFactor = shrink;

  // test  to determine if the pipeline is initialized first
  // and only call update afterwards
  if (this->IsInitialized)
    this->UpdatePipeline();
}

//----------------------------------------------------------------
void vtkMeshQualityClass::SetElementOpacity(double opacity)
{
  this->ElementOpacity = opacity;
  // test  to determine if the pipeline is initialized first
  // and only call update afterwards
  if (this->IsInitialized)
    this->UpdatePipeline();
}

//---------------------------------------------------------------- 
void vtkMeshQualityClass::SetOutlineEnabled(int outline)
{
  this->OutlineIsEnabled = outline;
}


//----------------------------------------------------------------  
void vtkMeshQualityClass::InitializeMeshFromFile(char* filestr)
{
  vtkUnstructuredGridReader* reader = vtkUnstructuredGridReader::New();
  reader->SetFileName(filestr);
  reader->Update();
  cout << "file read completed" << endl;
  this->InitialMesh = (reader->GetOutput());
  this->SetupVTKPipeline();
  //this->UpdatePipeline();
}

 //---------------------------------------------------------------- 
void vtkMeshQualityClass::InitializeFromExternalMesh(vtkUnstructuredGrid *grid)
{
 this->InitialMesh = grid;
 this->SetupVTKPipeline();
 //this->UpdatePipeline();
}
  
//----------------------------------------------------------------  
void vtkMeshQualityClass::SetQualityMeasure(int qualityTest)  
{
/****

  // store this because the user could set it before the pipeline is invoked
  this->SavedQualityMetric = qualityTest;
  if (this->IsInitialized)
   {
     this->InvokeQualityMeasure();
     this->UpdatePipeline();
    }
    *****/
}


  
//----------------------------------------------------------------
void vtkMeshQualityClass::InvokeQualityMeasure(void)
{
  // here include a switch to set the most likely tests to be selected
  // using the enumerated values 0..4
#if 0
  switch (this->SavedQualityMetric)
  {
    case 0: {this->VTKQualityFilterPtr->SetHexQualityMeasureToEdgeRatio(); break;}
    case 1: {this->VTKQualityFilterPtr->SetHexQualityMeasureToShape();break;}
    case 2: {this->VTKQualityFilterPtr->SetHexQualityMeasureToJacobian();break;}
    case 3: {this->VTKQualityFilterPtr->SetHexQualityMeasureToMedAspectFrobenius();break;}
    case 4: {this->VTKQualityFilterPtr->SetHexQualityMeasureToVolume();break;}
    default: {this->VTKQualityFilterPtr->SetHexQualityMeasureToEdgeRatio();}
  }
#endif
}

//----------------------------------------------------------------
vtkActor* vtkMeshQualityClass::CalculateMeshQuality(void) 
{
  return outputActor;
}


//----------------------------------------------------------------
void vtkMeshQualityClass::FindMinimumAndMaximumQualityForMesh(vtkUnstructuredGrid *mesh, 
                    double *minQualityFound, double *maxQualityFound)
{

   *minQualityFound = 9.9e10;
   *maxQualityFound = -9.9e10;
  
  // go through the cell data and record the smallest and largest values found
  // so we can use these values to autoscale the colors
  long numCells = ((mesh->GetCellData())->GetArray("Quality"))->GetNumberOfTuples();
  for (int i=0; i< numCells; i++) 
  {
    double thisQ = ((vtkDoubleArray*)(mesh->GetCellData())->GetArray("Quality"))->GetValue(i);
    if (thisQ > *maxQualityFound) *maxQualityFound = thisQ;
    if (thisQ < *minQualityFound) *minQualityFound = thisQ;
  }
  //cout << "Min quality found: " << *minQualityFound << " max quality found: " ;
  //cout << *maxQualityFound << endl;
  if (*minQualityFound < 0.0) 
  {
    *minQualityFound = 0.0;
    cout << "clamping min quality to 0" << endl;
  }
}



//----------------------------------------------------------------
void vtkMeshQualityClass::SetupVTKPipeline(void) 
{

   double minQualityFound;
   double maxQualityFound;

  // setup the pipeline here
  cout << "file read beginning" << endl;

  vtkMeshQuality* qual = vtkMeshQuality::New();
  qual->SetInput((vtkDataSet*)this->InitialMesh);
  qual->Update();  
  this->VTKQualityFilterPtr = qual;
  this->InvokeQualityMeasure();
  cout << "mesh quality completed" << endl;

  
  // now loop through the quality values and find the minimum and maximum
  // values for this particular dataset
  
  vtkUnstructuredGrid* mesh = (vtkUnstructuredGrid*)qual->GetOutput();
  
  // now reset this pointer to look at the mesh after it has had quality added to it
  this->SavedMesh = mesh;
  long numCells = ((mesh->GetCellData())->GetArray("Quality"))->GetNumberOfTuples();
  cout << "found " << numCells << " cells in the dataset" << endl;
  
  this->FindMinimumAndMaximumQualityForMesh(mesh,&minQualityFound,&maxQualityFound);
  cout << "Min quality found: " << minQualityFound << " max quality found: " ;
  cout << maxQualityFound << endl;
  
  vtkRearrangeFields* rf = (vtkRearrangeFields*) vtkRearrangeFields::New();
  rf->SetInputConnection(qual->GetOutputPort());
  rf->AddOperation("COPY","Quality","CELL_DATA", "POINT_DATA");
  cout << "completed rearranging fields" << endl; 
  
  vtkAssignAttribute* aa = (vtkAssignAttribute*) vtkAssignAttribute::New();
  aa->SetInputConnection(rf->GetOutputPort());
  aa->Update();
  cout << "completed assign attributes" << endl; 
  
  // srhink the elements so we can see each one.  We are using a shrink filter 
  // instantiated earlier because we needed a pointer to the filter to invoke its
  // modified method as part of a GUI callback to update the pipeline
  
  // store a handle to this filter instance so we can restrobe the pipeline 
  vtkShrinkFilter* shrinker = (vtkShrinkFilter*) vtkShrinkFilter::New();
  shrinker->SetInputConnection(aa->GetOutputPort());
  shrinker->SetShrinkFactor(this->ElementShrinkFactor);
  this->shrinkFilter = shrinker;
  //shrinker->Update();
    

  vtkLogLookupTable* lut = (vtkLogLookupTable*) vtkLogLookupTable::New();
  lut->SetHueRange (0.8, 0.0);
  lut->SetAlphaRange(this->ElementOpacity,this->ElementOpacity);
  lut->SetTableRange(minQualityFound, maxQualityFound);
  this->lutFilter = lut;
 
  // The scalar mode in the mapper needs to be set to cells here so the cells are uniformly
  // colored with the mesh quality metric.  Without this,the values at the points
  // cause color interpolation across the cell. 
  
  vtkDataSetMapper*  elemMap = vtkDataSetMapper::New();
  elemMap->SetInputConnection(shrinker->GetOutputPort());
  elemMap->SetScalarModeToUseCellData();
  elemMap->SetLookupTable(lut);
  elemMap->SetScalarRange (minQualityFound, maxQualityFound);
  this->SavedElementMapper = elemMap;

  vtkActor* elemAct = vtkActor::New();
  elemAct->SetMapper(elemMap);
  //elemAct->GetProperty()->BackfaceCullingOff();
  //elemAct->GetProperty()->FrontfaceCullingOff();
  //elemAct->GetProperty()->EdgeVisibilityOn();
  
  this->outputActor = elemAct;

  // the VTK pipeline exists, now it is OK to change values on the filters
  this->IsInitialized = 1;
  
}


//----------------------------------------------------------------
void vtkMeshQualityClass::UpdatePipeline(void)
{
  double alpha; 
  double minQualityFound;
  double maxQualityFound;
  
  //cout << "calling update pipeline" << endl;
  if (this->IsInitialized) 
  {
  
    // set the VTK supporting class to have the right test and check the range
    // of values to recalibrate the LUT when the test changes
    //this->InvokeQualityMeasure();
    this->FindMinimumAndMaximumQualityForMesh(this->SavedMesh,&minQualityFound,&maxQualityFound);
    this->lutFilter->SetTableRange(minQualityFound, maxQualityFound);
    this->SavedElementMapper->SetScalarRange (minQualityFound, maxQualityFound);
  
    this->shrinkFilter->SetShrinkFactor(this->ElementShrinkFactor);
    this->shrinkFilter->Update();
    cout << "shrink value is: " << this->shrinkFilter->GetShrinkFactor() << endl;    

    // also redo the look up table here because the value of the threshold could 
    // have changed
    
    for (int i=0; i<255; i++) 
    {
    double si = i/255.0;
    double decreasing_si = 1.0 - si;
    alpha = this->ElementOpacity;
    if( si < this->thresholdValue) { alpha = 0; }
    
    // assign a color according to the place in the spectrum.  If the value 
    // is below the threshold, then make it invisible.  Otherwise use the index and
    // input parameters to calculate the colors
    
    double r,g,b;
    vtkMath::HSVToRGB(decreasing_si,1.0,1.0,&r,&g,&b);
    ((vtkLookupTable*)this->lutFilter)->SetTableValue(i, r,g,b,alpha);
    }
  }
  
}

