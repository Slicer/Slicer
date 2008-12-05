/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxRecalculateInteriorNodesTPS.cxx,v $
Language:  C++
Date:      $Date: 2007/11/14 21:25:05 $
Version:   $Revision: 1.4 $

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

#include "vtkMimxRecalculateInteriorNodesTPS.h"

#include "vtkCellData.h"
#include "vtkExecutive.h"
#include "vtkGenericCell.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkUnstructuredGrid.h"
#include "vtkIdList.h"
#include "vtkCleanPolyData.h"
#include "vtkMaskPoints.h"
#include "vtkThinPlateSplineTransform.h"
#include "vtkTransformFilter.h"
#include "vtkGeometryFilter.h"
#include "vtkPointLocator.h"

//-----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkMimxRecalculateInteriorNodesTPS, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkMimxRecalculateInteriorNodesTPS);
//------------------------------------------------------------------------------
// Construct object to Set all of the input data.
vtkMimxRecalculateInteriorNodesTPS::vtkMimxRecalculateInteriorNodesTPS()
{
        this->SetNumberOfInputPorts(2);
        this->SamplingNumber = 1;
}
//------------------------------------------------------------------------------
vtkMimxRecalculateInteriorNodesTPS::~vtkMimxRecalculateInteriorNodesTPS()
{
        
}
//------------------------------------------------------------------------------
int vtkMimxRecalculateInteriorNodesTPS::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
        // get the info objects
        vtkInformation *sourceInfo = inputVector[0]->GetInformationObject(0);
        vtkInformation *targetInfo = inputVector[1]->GetInformationObject(0);

        vtkInformation *outInfo = outputVector->GetInformationObject(0);
        
        vtkUnstructuredGrid *source = vtkUnstructuredGrid::SafeDownCast(
                sourceInfo->Get(vtkDataObject::DATA_OBJECT()));

        vtkUnstructuredGrid *target = vtkUnstructuredGrid::SafeDownCast(
                targetInfo->Get(vtkDataObject::DATA_OBJECT()));

        vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
                outInfo->Get(vtkDataObject::DATA_OBJECT()));
        
        int numNodesSource = source->GetNumberOfPoints();
        int numCellsSource = source->GetNumberOfCells();

        int numNodesTarget = target->GetNumberOfPoints();
        int numCellsTarget = target->GetNumberOfCells();

        if(numNodesSource <= 0 || numCellsSource <= 0 ||
                numNodesTarget <=0 || numCellsTarget <=0){
                vtkErrorMacro("Invalid input");
        return 0;}

        if(numNodesSource != numNodesTarget || numCellsSource != numCellsTarget){
                        vtkErrorMacro("Both Source and Target should contain same number of nodes and cells");
                        return 0;}

        vtkPolyData *UndefPoly;
        vtkPolyData *DefPoly;
        int i;

        /* Geometry Filter to convert Unstructured Grid to Surface Mesh */
        vtkGeometryFilter *DefGeometry = vtkGeometryFilter::New();
        DefGeometry->SetInput( target );
        DefGeometry->Update( );
        DefPoly = DefGeometry->GetOutput( );

        vtkGeometryFilter *UndefGeometry = vtkGeometryFilter::New();
        UndefGeometry->SetInput( source );
        UndefGeometry->Update( );
        UndefPoly = UndefGeometry->GetOutput( );

        //
        DefGeometry->GetOutput()->BuildLinks();
        UndefGeometry->GetOutput()->BuildLinks();
        vtkIdType *ptsdef, *ptsundef;
        unsigned short nptsdef, nptsundef;

        vtkPoints *UndefPoints = vtkPoints::New();
        vtkPoints *DefPoints = vtkPoints::New();

        for (i=0; i <numNodesSource; i++)
        {
                DefPoly->GetPointCells(i, nptsdef, ptsdef);
                UndefPoly->GetPointCells(i, nptsundef, ptsundef);
                if(nptsdef != nptsundef)
                {
                        vtkErrorMacro("Connectivity information varying between source and target");
                        UndefPoints->Delete();
                        DefPoints->Delete();
                        DefGeometry->Delete();
                        UndefGeometry->Delete();
                        return 0;
                }
                if(nptsdef !=0)
                {
                DefPoints->InsertNextPoint(DefPoly->GetPoint(i));
                UndefPoints->InsertNextPoint(UndefPoly->GetPoint(i));
                }
        }
        //

        // clean the polydata so as to remove all the points not being used

        //vtkCleanPolyData *DefPolyClean = vtkCleanPolyData::New();
        //vtkCleanPolyData *UndefPolyClean = vtkCleanPolyData::New();
        //DefPolyClean->SetInput(DefPoly);
        //DefPolyClean->Update();

        //UndefPolyClean->SetInput(UndefPoly);
        //UndefPolyClean->Update();

        //vtkMaskPoints *maskDefPoints = vtkMaskPoints::New();
        //maskDefPoints->SetInput( DefPolyClean->GetOutput() );
        //maskDefPoints->SetOnRatio(this->SamplingNumber);
        //maskDefPoints->Update(  );

        //vtkPoints *DefPoints = maskDefPoints->GetOutput( )->GetPoints( );

        //vtkPointLocator *DefLocator = vtkPointLocator::New( );
        //DefLocator->InitPointInsertion( 
        //      maskDefPoints->GetOutput( )->GetPoints( ), maskDefPoints->GetOutput( )->GetBounds( ) );
        //DefLocator->SetDataSet( maskDefPoints->GetOutput( ) );
        //DefLocator->Update( );

        //vtkIdList *IdList = vtkIdList::New( );
        //IdList->Allocate( DefPoints->GetNumberOfPoints( ) );

        //for ( int i = 0; i < DefPolyClean->GetOutput()->GetNumberOfPoints( ); i++ )
        //{
        //      if( DefLocator->IsInsertedPoint( DefPolyClean->GetOutput()->GetPoint( i ) ) != -1 )
        //      {
        //              IdList->InsertNextId( i );
        //      }
        //}

        //vtkPoints *UndefPoints = vtkPoints::New( );
        //UndefPoints->SetNumberOfPoints( DefPoints->GetNumberOfPoints( ) );
        //vtkIdType pointId;
        //for ( i = 0; i < IdList->GetNumberOfIds( ); i++ )
        //{
        //      pointId = IdList->GetId( i );
        //      UndefPoints->SetPoint( i, UndefPolyClean->GetOutput()->GetPoint( pointId ) );
        //}
        vtkThinPlateSplineTransform *thinTransform = vtkThinPlateSplineTransform::New( );
        thinTransform->SetBasisToR( );
        thinTransform->SetSourceLandmarks( UndefPoints);
        thinTransform->SetTargetLandmarks( DefPoints);
        thinTransform->Update( );

        vtkTransformFilter *finalTransform = vtkTransformFilter::New( );
        finalTransform->SetInput(source);
        finalTransform->SetTransform( thinTransform );
        finalTransform->Update( );
        vtkPoints *points = vtkPoints::New();
        points->DeepCopy(finalTransform->GetOutput()->GetPoints());
        output->Allocate(source->GetNumberOfCells(), source->GetNumberOfCells());

        for(i=0; i <source->GetNumberOfCells(); i++)
        {
                vtkIdList *locallist = vtkIdList::New();
                locallist->DeepCopy(source->GetCell(i)->GetPointIds());
                output->InsertNextCell(source->GetCellType(i), locallist);
                locallist->Delete();
        }
        output->SetPoints(points);
        points->Delete();

        finalTransform->Delete();
        thinTransform->Delete();
        UndefPoints->Delete();
        DefPoints->Delete();
        //IdList->Delete();
        //DefLocator->Delete();
        //maskDefPoints->Delete();
        //DefPolyClean->Delete();
        //UndefPolyClean->Delete();
        UndefGeometry->Delete();
        DefGeometry->Delete();

        return 1;
}
//------------------------------------------------------------------------------
void vtkMimxRecalculateInteriorNodesTPS::SetSource(vtkUnstructuredGrid *UGrid)
{
        this->SetInput(0, UGrid);
}
//------------------------------------------------------------------------------
void vtkMimxRecalculateInteriorNodesTPS::SetTarget(vtkUnstructuredGrid *UGrid)
{
        this->SetInput(1, UGrid);
}
//------------------------------------------------------------------------------
void vtkMimxRecalculateInteriorNodesTPS::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
