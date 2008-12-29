/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMeshQualityExtended.cxx,v $
Language:  C++
Date:      $Date: 2008/07/01 00:09:32 $
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

ACKNOWLEDGEMENT: Originally Developed by
----------------------------------------

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMeshQualityExtended.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

  Copyright 2003-2006 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
  license for use of this work by or on behalf of the
  U.S. Government. Redistribution and use in source and binary forms, with
  or without modification, are permitted provided that this Notice and any
  statement of authorship are reproduced on all copies.

  Contact: dcthomp@sandia.gov,pppebay@sandia.gov

=========================================================================*/
#include "vtkMeshQualityExtended.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkDataSet.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkDoubleArray.h"
#include "vtkCell.h"
#include "vtkCellTypes.h"
#include "vtkPoints.h"
#include "vtkMath.h"
#include "vtkTetra.h"
#include "vtkTriangle.h"

#include "verdict.h"

#define EDGE_COLLAPSE_TOLERANCE  0.00001

vtkCxxRevisionMacro(vtkMeshQualityExtended,"$Revision: 1.4 $");
vtkStandardNewMacro(vtkMeshQualityExtended);

typedef double (*CellQualityType)( vtkCell*  );


const char* QualityMeasureExtendedNames[] =
{
  "EdgeRatio",
  "AspectRatio",
  "RadiusRatio",
  "AspectFrobenius",
  "MedAspectFrobenius",
  "MaxAspectFrobenius",
  "MaxEdgeRatios",
  "MinAngle",
  "CollapseRatio",
  "MaxAngle",
  "Condition",
  "ScaledJacobian",
  "Shear",
  "RelativeSizeSquared",
  "Shape",
  "ShapeAndSize",
  "Distortion",
  "EdgeCollapse",
  "AngleOutOfBounds"
};


void vtkMeshQualityExtended::PrintSelf(ostream& os, vtkIndent indent )
{
  const char onStr[] = "On";
  const char offStr[] = "Off";

  this->Superclass::PrintSelf( os, indent );

  os << indent << "SaveCellQuality:   "
     << (this->SaveCellQuality ? onStr : offStr) << endl;
  os << indent << "TetQualityMeasure: "
     << QualityMeasureExtendedNames[this->TetQualityMeasure] << endl;
  os << indent << "HexQualityMeasure: "
     << QualityMeasureExtendedNames[this->HexQualityMeasure] << endl;
  os << indent << "Volume: " 
     << (this->Volume ? onStr : offStr) << endl;
  os << indent << "CompatibilityMode: " 
     << (this->CompatibilityMode ? onStr : offStr) << endl;
}

vtkMeshQualityExtended::vtkMeshQualityExtended()
{
  this->SaveCellQuality = 1; // Default is On
  this->TetQualityMeasure = VTK_QUALITY_ASPECT_RATIO;
  this->HexQualityMeasure = VTK_QUALITY_MAX_ASPECT_FROBENIUS;
  this->Volume = 0;
  this->CompatibilityMode = 0;
  this->MetricTolerance = EDGE_COLLAPSE_TOLERANCE;
}

vtkMeshQualityExtended::~vtkMeshQualityExtended()
{
  // Nothing yet.
}


int vtkMeshQualityExtended::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkDataSet *in = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkDataSet *out = vtkDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  CellQualityType TriangleQuality,QuadQuality,TetQuality,HexQuality;
  vtkDoubleArray* quality = 0;
  vtkDoubleArray* volume = 0;
  vtkIdType N = in->GetNumberOfCells();
  double qtrim,qtriM,Eqtri,Eqtri2;
  double qquam,qquaM,Eqqua,Eqqua2;
  double qtetm,qtetM,Eqtet,Eqtet2;
  double qhexm,qhexM,Eqhex,Eqhex2;
  double q;
  double V = 0.;
  vtkIdType ntri = 0;
  vtkIdType nqua = 0;
  vtkIdType ntet = 0;
  vtkIdType nhex = 0;
  vtkCell* cell;
  int progressNumer = 0;
  double progressDenom = 20.;

  this->CellNormals = in->GetCellData()->GetNormals();

  if ( this->CellNormals  )
    v_set_tri_normal_func( (ComputeNormal) vtkMeshQuality::GetCurrentTriangleNormal );
  else
    v_set_tri_normal_func( 0 );

  // Initialize the min and max values, std deviations, etc.
  qtriM = qquaM = qtetM = qhexM = VTK_DOUBLE_MIN;
  qtrim = qquam = qtetm = qhexm = VTK_DOUBLE_MAX;
  Eqtri = Eqtri2 = Eqqua = Eqqua2 = Eqtet = Eqtet2 = Eqhex = Eqhex2 = 0.;

  switch ( this->GetTriangleQualityMeasure() )
    {
    case VTK_QUALITY_AREA:
      TriangleQuality = TriangleArea;
      break;
    case VTK_QUALITY_EDGE_RATIO:
      TriangleQuality = TriangleEdgeRatio;
      break;
    case VTK_QUALITY_ASPECT_RATIO:
      TriangleQuality = TriangleAspectRatio;
      break;
    case VTK_QUALITY_RADIUS_RATIO:
      TriangleQuality = TriangleRadiusRatio;
      break;
    case VTK_QUALITY_ASPECT_FROBENIUS:
      TriangleQuality = TriangleAspectFrobenius;
      break;
    case VTK_QUALITY_MIN_ANGLE:
      TriangleQuality = TriangleMinAngle;
      break;
    case VTK_QUALITY_MAX_ANGLE:
      TriangleQuality = TriangleMaxAngle;
      break;
    case VTK_QUALITY_CONDITION:
      TriangleQuality = TriangleCondition;
      break;
    case VTK_QUALITY_SCALED_JACOBIAN:
      TriangleQuality = TriangleScaledJacobian;
      break;
    case VTK_QUALITY_RELATIVE_SIZE_SQUARED:
      TriangleQuality = TriangleRelativeSizeSquared;
      break;
    case VTK_QUALITY_SHAPE:
      TriangleQuality = TriangleShape;
      break;
    case VTK_QUALITY_SHAPE_AND_SIZE:
      TriangleQuality = TriangleShapeAndSize;
      break;
    case VTK_QUALITY_DISTORTION:
      TriangleQuality = TriangleDistortion;
      break;
    case VTK_QUALITY_EDGE_COLLAPSE:
      TriangleQuality = TriangleEdgeCollapse;
      break;
    case VTK_QUALITY_ANGLE_OUT_OF_BOUNDS:
      TriangleQuality = TriangleAngleOutOfBounds;
      break;
    default:
      vtkWarningMacro( "Bad TriangleQualityMeasure ("
                       << this->GetTriangleQualityMeasure() << "), using RadiusRatio instead");
      TriangleQuality = TriangleRadiusRatio;
      break;
    }

  switch ( this->GetQuadQualityMeasure() )
    {
    case VTK_QUALITY_EDGE_RATIO:
      QuadQuality = QuadEdgeRatio;
      break;
    case VTK_QUALITY_ASPECT_RATIO:
      QuadQuality = QuadAspectRatio;
      break;
    case VTK_QUALITY_RADIUS_RATIO:
      QuadQuality = QuadRadiusRatio;
      break;
    case VTK_QUALITY_MED_ASPECT_FROBENIUS:
      QuadQuality = QuadMedAspectFrobenius;
      break;
    case VTK_QUALITY_MAX_ASPECT_FROBENIUS:
      QuadQuality = QuadMaxAspectFrobenius;
      break;
    case VTK_QUALITY_MIN_ANGLE:
      QuadQuality = QuadMinAngle;
      break;
    case VTK_QUALITY_MAX_EDGE_RATIO:
      QuadQuality = QuadMaxEdgeRatios;
      break;
    case VTK_QUALITY_SKEW:
      QuadQuality = QuadSkew;
      break;
    case VTK_QUALITY_TAPER:
      QuadQuality = QuadTaper;
      break;
    case VTK_QUALITY_WARPAGE:
      QuadQuality = QuadWarpage;
      break;
    case VTK_QUALITY_AREA:
      QuadQuality = QuadArea;
      break;
    case VTK_QUALITY_STRETCH:
      QuadQuality = QuadStretch;
      break;
      //case VTK_QUALITY_MIN_ANGLE:
    case VTK_QUALITY_MAX_ANGLE:
      QuadQuality = QuadMaxAngle;
      break;
    case VTK_QUALITY_ODDY:
      QuadQuality = QuadOddy;
      break;
    case VTK_QUALITY_CONDITION:
      QuadQuality = QuadCondition;
      break;
    case VTK_QUALITY_JACOBIAN:
      QuadQuality = QuadJacobian;
      break;
    case VTK_QUALITY_SCALED_JACOBIAN:
      QuadQuality = QuadScaledJacobian;
      break;
    case VTK_QUALITY_SHEAR:
      QuadQuality = QuadShear;
      break;
    case VTK_QUALITY_SHAPE:
      QuadQuality = QuadShape;
      break;
    case VTK_QUALITY_RELATIVE_SIZE_SQUARED:
      QuadQuality = QuadRelativeSizeSquared;
      break;
    case VTK_QUALITY_SHAPE_AND_SIZE:
      QuadQuality = QuadShapeAndSize;
      break;
    case VTK_QUALITY_SHEAR_AND_SIZE:
      QuadQuality = QuadShearAndSize;
      break;
    case VTK_QUALITY_DISTORTION:
      QuadQuality = QuadDistortion;
      break;
    case VTK_QUALITY_EDGE_COLLAPSE:
      QuadQuality = QuadEdgeCollapse;
      break;
    case VTK_QUALITY_ANGLE_OUT_OF_BOUNDS:
      QuadQuality = QuadAngleOutOfBounds;
      break;
    default:
      vtkWarningMacro( "Bad QuadQualityMeasure ("
                       << this->GetQuadQualityMeasure() << "), using EdgeRatio instead");
      QuadQuality = QuadEdgeRatio;
      break;
    }

  switch ( this->GetTetQualityMeasure() )
    {
    case VTK_QUALITY_EDGE_RATIO:
      TetQuality = TetEdgeRatio;
      break;
    case VTK_QUALITY_ASPECT_RATIO:
      TetQuality = TetAspectRatio;
      break;
    case VTK_QUALITY_RADIUS_RATIO:
      TetQuality = TetRadiusRatio;
      break;
    case VTK_QUALITY_ASPECT_FROBENIUS:
      TetQuality = TetAspectFrobenius;
      break;
    case VTK_QUALITY_MIN_ANGLE:
      TetQuality = TetMinAngle;
      break;
    case VTK_QUALITY_COLLAPSE_RATIO:
      TetQuality = TetCollapseRatio;
      break;
    case VTK_QUALITY_ASPECT_BETA:
      TetQuality = TetAspectBeta;
      break;
    case VTK_QUALITY_ASPECT_GAMMA:
      TetQuality = TetAspectGamma;
      break;
    case VTK_QUALITY_VOLUME:
      TetQuality = TetVolume;
      break;
    case VTK_QUALITY_CONDITION:
      TetQuality = TetCondition;
      break;
    case VTK_QUALITY_JACOBIAN:
      TetQuality = TetJacobian;
      break;
    case VTK_QUALITY_SCALED_JACOBIAN:
      TetQuality = TetScaledJacobian;
      break;
    case VTK_QUALITY_SHAPE:
      TetQuality = TetShape;
      break;
    case VTK_QUALITY_RELATIVE_SIZE_SQUARED:
      TetQuality = TetRelativeSizeSquared;
      break;
    case VTK_QUALITY_SHAPE_AND_SIZE:
      TetQuality = TetShapeandSize;
      break;
    case VTK_QUALITY_DISTORTION:
      TetQuality = TetDistortion;
      break;
    case VTK_QUALITY_EDGE_COLLAPSE:
      TetQuality = TetEdgeCollapse;
      break;
    case VTK_QUALITY_ANGLE_OUT_OF_BOUNDS:
      TetQuality = TetAngleOutOfBounds;
      break;
    default:
      vtkWarningMacro( "Bad TetQualityMeasure ("
                       << this->GetTetQualityMeasure() << "), using RadiusRatio instead");
      TetQuality = TetRadiusRatio;
      break;
    }

  switch ( this->GetHexQualityMeasure() )
    {
    case VTK_QUALITY_EDGE_RATIO:
      HexQuality = HexEdgeRatio;
      break;
    case VTK_QUALITY_MED_ASPECT_FROBENIUS:
      HexQuality = HexMedAspectFrobenius;
      break;
    case VTK_QUALITY_MAX_ASPECT_FROBENIUS:
      HexQuality = HexMaxAspectFrobenius;
      break;
    case VTK_QUALITY_MAX_EDGE_RATIO:
      HexQuality = HexMaxEdgeRatio;
      break;
    case VTK_QUALITY_SKEW:
      HexQuality = HexSkew;
      break;
    case VTK_QUALITY_TAPER:
      HexQuality = HexTaper;
      break;
    case VTK_QUALITY_VOLUME:
      HexQuality = HexVolume;
      break;
    case VTK_QUALITY_STRETCH:
      HexQuality = HexStretch;
      break;
    case VTK_QUALITY_DIAGONAL:
      HexQuality = HexDiagonal;
      break;
    case VTK_QUALITY_DIMENSION:
      HexQuality = HexDimension;
      break;
    case VTK_QUALITY_ODDY:
      HexQuality = HexOddy;
      break;
    case VTK_QUALITY_CONDITION:
      HexQuality = HexCondition;
      break;
    case VTK_QUALITY_JACOBIAN:
      HexQuality = HexJacobian;
      break;
    case VTK_QUALITY_SCALED_JACOBIAN:
      HexQuality = HexScaledJacobian;
      break;
    case VTK_QUALITY_SHEAR:
      HexQuality = HexShear;
      break;
    case VTK_QUALITY_SHAPE:
      HexQuality = HexShape;
      break;
    case VTK_QUALITY_RELATIVE_SIZE_SQUARED:
      HexQuality = HexRelativeSizeSquared;
      break;
    case VTK_QUALITY_SHAPE_AND_SIZE:
      HexQuality = HexShapeAndSize;
      break;
    case VTK_QUALITY_SHEAR_AND_SIZE:
      HexQuality = HexShearAndSize;
      break;
    case VTK_QUALITY_DISTORTION:
      HexQuality = HexDistortion;
      break;
    case VTK_QUALITY_EDGE_COLLAPSE:
      HexQuality = HexEdgeCollapse;
      break;
    case VTK_QUALITY_ANGLE_OUT_OF_BOUNDS:
      HexQuality = HexAngleOutOfBounds;
      break;
    default:
      vtkWarningMacro( "Bad HexQualityMeasure ("
                       << this->GetTetQualityMeasure() << "), using MaxAspectFrobenius instead");
      HexQuality = HexMaxAspectFrobenius;
      break;
    }

  out->ShallowCopy( in );

  if ( this->SaveCellQuality )
    {
    quality = vtkDoubleArray::New();
    if ( this->CompatibilityMode )
      {
      if ( this->Volume )
        {
        quality->SetNumberOfComponents(2);
        }
      else
        {
        quality->SetNumberOfComponents(1);
        }
      }
    else
      {
      quality->SetNumberOfComponents(1);
      }
    quality->SetNumberOfTuples( N );
    quality->SetName( "Quality" );
    out->GetCellData()->AddArray( quality );
    out->GetCellData()->SetActiveAttribute( "Quality", vtkDataSetAttributes::SCALARS );
    quality->Delete();

    if ( ! this->CompatibilityMode )
      {
      if ( this->Volume )
        {
        volume = vtkDoubleArray::New();
        volume->SetNumberOfComponents(1);
        volume->SetNumberOfTuples( N );
        volume->SetName( "Volume" );
        out->GetCellData()->AddArray( volume );
        volume->Delete();
        }
      }
    }

  // These measures require the average area/volume for all cells of the same type in the mesh.
  // Either use the hinted value (computed by a previous vtkMeshQuality filter) or compute it.
  if ( this->GetTriangleQualityMeasure() == VTK_QUALITY_RELATIVE_SIZE_SQUARED ||
       this->GetTriangleQualityMeasure() == VTK_QUALITY_SHAPE_AND_SIZE ||
       this->GetQuadQualityMeasure() == VTK_QUALITY_RELATIVE_SIZE_SQUARED ||
       this->GetQuadQualityMeasure() == VTK_QUALITY_SHAPE_AND_SIZE ||
       this->GetQuadQualityMeasure() == VTK_QUALITY_SHEAR_AND_SIZE ||
       this->GetTetQualityMeasure() == VTK_QUALITY_RELATIVE_SIZE_SQUARED ||
       this->GetTetQualityMeasure() == VTK_QUALITY_SHAPE_AND_SIZE ||
       this->GetHexQualityMeasure() == VTK_QUALITY_RELATIVE_SIZE_SQUARED ||
       this->GetHexQualityMeasure() == VTK_QUALITY_SHAPE_AND_SIZE ||
       this->GetHexQualityMeasure() == VTK_QUALITY_SHEAR_AND_SIZE )
    {
    vtkDataArray* triAreaHint = in->GetFieldData()->GetArray( "TriArea" );
    vtkDataArray* quadAreaHint = in->GetFieldData()->GetArray( "QuadArea" );
    vtkDataArray* tetVolHint = in->GetFieldData()->GetArray( "TetVolume" );
    vtkDataArray* hexVolHint = in->GetFieldData()->GetArray( "HexVolume" );

    double triAreaTuple[5];
    double quadAreaTuple[5];
    double tetVolTuple[5];
    double hexVolTuple[5];

    if ( triAreaHint  &&  triAreaHint->GetNumberOfTuples() > 0 &&  triAreaHint->GetNumberOfComponents() == 5 &&
         quadAreaHint && quadAreaHint->GetNumberOfTuples() > 0 && quadAreaHint->GetNumberOfComponents() == 5 &&
         tetVolHint   &&   tetVolHint->GetNumberOfTuples() > 0 &&   tetVolHint->GetNumberOfComponents() == 5 &&
         hexVolHint   &&   hexVolHint->GetNumberOfTuples() > 0 &&   hexVolHint->GetNumberOfComponents() == 5 )
      {
      triAreaHint->GetTuple( 0, triAreaTuple );
      quadAreaHint->GetTuple( 0, quadAreaTuple );
      tetVolHint->GetTuple( 0, tetVolTuple );
      hexVolHint->GetTuple( 0, hexVolTuple );
      v_set_tri_size( triAreaTuple[1] / triAreaTuple[4] );
      v_set_quad_size( quadAreaTuple[1] / quadAreaTuple[4] );
      v_set_tet_size(  tetVolTuple[1] / tetVolTuple[4] );
      v_set_hex_size(  hexVolTuple[1] / hexVolTuple[4] );
      }
    else
      {
      for ( int i = 0; i < 5; ++i )
        {
        triAreaTuple[i]  = 0;
        quadAreaTuple[i] = 0;
        tetVolTuple[i]   = 0;
        hexVolTuple[i]   = 0;
        }
      for ( vtkIdType c = 0; c < N; ++c )
        {
        double a, v; // area and volume
        cell = out->GetCell( c );
        switch ( cell->GetCellType() )
          {
          case VTK_TRIANGLE:
            a = TriangleArea( cell );
            if ( a > triAreaTuple[2] )
              {
              if ( triAreaTuple[0] == triAreaTuple[2] )
                { // min == max => min has not been set
                triAreaTuple[0] = a;
                }
              triAreaTuple[2] = a;
              }
            else if ( a < triAreaTuple[0] )
              {
              triAreaTuple[0] = a;
              }
            triAreaTuple[1] += a;
            triAreaTuple[3] += a * a;
            ntri++;
            break;
          case VTK_QUAD:
            a = QuadArea( cell );
            if ( a > quadAreaTuple[2] )
              {
              if ( quadAreaTuple[0] == quadAreaTuple[2] )
                { // min == max => min has not been set
                quadAreaTuple[0] = a;
                }
              quadAreaTuple[2] = a;
              }
            else if ( a < quadAreaTuple[0] )
              {
              quadAreaTuple[0] = a;
              }
            quadAreaTuple[1] += a;
            quadAreaTuple[3] += a * a;
            nqua++;
            break;
          case VTK_TETRA:
            v = TetVolume( cell );
            if ( v > tetVolTuple[2] )
              {
              if ( tetVolTuple[0] == tetVolTuple[2] )
                { // min == max => min has not been set
                tetVolTuple[0] = v;
                }
              tetVolTuple[2] = v;
              }
            else if ( v < tetVolTuple[0] )
              {
              tetVolTuple[0] = v;
              }
            tetVolTuple[1] += v;
            tetVolTuple[3] += v * v;
            ntet++;
            break;
          case VTK_HEXAHEDRON:
            v = HexVolume( cell );
            if ( v > hexVolTuple[2] )
              {
              if ( hexVolTuple[0] == hexVolTuple[2] )
                { // min == max => min has not been set
                hexVolTuple[0] = v;
                }
              hexVolTuple[2] = v;
              }
            else if ( v < hexVolTuple[0] )
              {
              hexVolTuple[0] = v;
              }
            hexVolTuple[1] += v;
            hexVolTuple[3] += v * v;
            nhex++;
            break;
          }
        }
      triAreaTuple[4]  = ntri;
      quadAreaTuple[4] = nqua;
      tetVolTuple[4]   = ntet;
      hexVolTuple[4]   = nhex;
      v_set_tri_size( triAreaTuple[1] / triAreaTuple[4] );
      v_set_quad_size( quadAreaTuple[1] / quadAreaTuple[4] );
      v_set_tet_size( tetVolTuple[1] / tetVolTuple[4] );
      v_set_hex_size( hexVolTuple[1] / hexVolTuple[4] );
      progressNumer = 20;
      progressDenom = 40.;
      ntri = 0;
      nqua = 0;
      ntet = 0;
      nhex = 0;

      // Save info as field data for downstream filters
      triAreaHint = vtkDoubleArray::New();
      triAreaHint->SetName( "TriArea" );
      triAreaHint->SetNumberOfComponents( 5 );
      triAreaHint->InsertNextTuple( triAreaTuple );
      out->GetFieldData()->AddArray( triAreaHint );
      triAreaHint->Delete();

      quadAreaHint = vtkDoubleArray::New();
      quadAreaHint->SetName( "QuadArea" );
      quadAreaHint->SetNumberOfComponents( 5 );
      quadAreaHint->InsertNextTuple( quadAreaTuple );
      out->GetFieldData()->AddArray( quadAreaHint );
      quadAreaHint->Delete();

      tetVolHint = vtkDoubleArray::New();
      tetVolHint->SetName( "TetVolume" );
      tetVolHint->SetNumberOfComponents( 5 );
      tetVolHint->InsertNextTuple( tetVolTuple );
      out->GetFieldData()->AddArray( tetVolHint );
      tetVolHint->Delete();

      hexVolHint = vtkDoubleArray::New();
      hexVolHint->SetName( "HexVolume" );
      hexVolHint->SetNumberOfComponents( 5 );
      hexVolHint->InsertNextTuple( hexVolTuple );
      out->GetFieldData()->AddArray( hexVolHint );
      hexVolHint->Delete();
      }
    }

  int p;
  vtkIdType c = 0;
  vtkIdType sz = N / 20 + 1;
  vtkIdType inner;
  this->UpdateProgress( progressNumer/progressDenom + 0.01 );
  for ( p = 0; p < 20; ++p )
    {
    for ( inner = 0; (inner < sz && c < N); ++c, ++inner )
      {
      cell = out->GetCell( c );
      V = 0.;
      switch ( cell->GetCellType() )
        {
        case VTK_TRIANGLE:
          if ( this->CellNormals )
            this->CellNormals->GetTuple( c, vtkMeshQuality::CurrentTriNormal );
          q = TriangleQuality( cell );
          if ( q > qtriM )
            {
            if ( qtrim > qtriM )
              {
              qtrim = q;
              }
            qtriM = q;
            }
          else if ( q < qtrim )
            {
            qtrim = q;
            }
          Eqtri += q;
          Eqtri2 += q * q;
          ++ ntri;
          break;
        case VTK_QUAD:
          q = QuadQuality( cell );
          if ( q > qquaM )
            {
            if ( qquam > qquaM )
              {
              qquam = q;
              }
            qquaM = q;
            }
          else if ( q < qquam )
            {
            qquam = q;
            }
          Eqqua += q;
          Eqqua2 += q * q;
          ++ nqua;
          break;
        case VTK_TETRA:
          q = TetQuality( cell );
          if ( q > qtetM )
            {
            if ( qtetm > qtetM )
              {
              qtetm = q;
              }
            qtetM = q;
            }
          else if ( q < qtetm )
            {
            qtetm = q;
            }
          Eqtet += q;
          Eqtet2 += q * q;
          ++ ntet;
          if ( this->Volume )
            {
            V = TetVolume( cell );
            if ( ! this->CompatibilityMode )
              {
              volume->SetTuple1( 0, V );
              }
            }
          break;
        case VTK_HEXAHEDRON:
          q = HexQuality( cell );
          if ( q > qhexM )
            {
            if ( qhexm > qhexM )
              {
              qhexm = q;
              }
            qhexM = q;
            }
          else if ( q < qhexm )
            {
            qhexm = q;
            }
          Eqhex += q;
          Eqhex2 += q * q;
          ++ nhex;
          break;
        default:
          q = 0.;
        }

      if ( this->SaveCellQuality )
        {
        if ( this->CompatibilityMode && this->Volume )
          {
          quality->SetTuple2( c, V, q );
          }
        else
          {
          quality->SetTuple1( c, q );
          }
        }
      }
    this->UpdateProgress( double(p+1+progressNumer)/progressDenom );
    }

  if ( ntri )
    {
    Eqtri  /= static_cast<double>( ntri );
    double multFactor = 1. / static_cast<double>( ntri > 1 ? ntri - 1 : ntri );
    Eqtri2 = multFactor * ( Eqtri2 - static_cast<double>( ntri ) * Eqtri * Eqtri );
    }
  else
    {
    qtrim = Eqtri = qtriM = Eqtri2 = 0.;
    }
  
  if ( nqua )
    {
    Eqqua  /= static_cast<double>( nqua );
    double multFactor = 1. / static_cast<double>( nqua > 1 ? nqua - 1 : nqua );
    Eqqua2 = multFactor * ( Eqqua2 - static_cast<double>( nqua ) * Eqqua * Eqqua );
    }
  else
    {
    qquam = Eqqua = qquaM = Eqqua2 = 0.;
    }
  
  if ( ntet )
    {
    Eqtet  /= static_cast<double>( ntet );
    double multFactor = 1. / static_cast<double>( ntet > 1 ? ntet - 1 : ntet );
    Eqtet2 = multFactor * ( Eqtet2 - static_cast<double>( ntet ) * Eqtet * Eqtet );
    }
  else
    {
    qtetm = Eqtet = qtetM = Eqtet2 = 0.;
    }

  if ( nhex )
    {
    Eqhex  /= static_cast<double>( nhex );
    double multFactor = 1. / static_cast<double>( nhex > 1 ? nhex - 1 : nhex );
    Eqhex2 = multFactor * ( Eqhex2 - static_cast<double>( nhex ) * Eqhex * Eqhex );
    }
  else
    {
    qhexm = Eqhex = qhexM = Eqhex2 = 0.;
    }

  double tuple[5];
  quality = vtkDoubleArray::New();
  quality->SetName( "Mesh Triangle Quality" );
  quality->SetNumberOfComponents(5);
  tuple[0] = qtrim;
  tuple[1] = Eqtri;
  tuple[2] = qtriM;
  tuple[3] = Eqtri2;
  tuple[4] = ntri;
  quality->InsertNextTuple( tuple );
  out->GetFieldData()->AddArray( quality );
  quality->Delete();

  quality = vtkDoubleArray::New();
  quality->SetName( "Mesh Quadrilateral Quality" );
  quality->SetNumberOfComponents(5);
  tuple[0] = qquam;
  tuple[1] = Eqqua;
  tuple[2] = qquaM;
  tuple[3] = Eqqua2;
  tuple[4] = nqua;
  quality->InsertNextTuple( tuple );
  out->GetFieldData()->AddArray( quality );
  quality->Delete();

  quality = vtkDoubleArray::New();
  quality->SetName( "Mesh Tetrahedron Quality" );
  quality->SetNumberOfComponents(5);
  tuple[0] = qtetm;
  tuple[1] = Eqtet;
  tuple[2] = qtetM;
  tuple[3] = Eqtet2;
  tuple[4] = ntet;
  quality->InsertNextTuple( tuple );
  out->GetFieldData()->AddArray( quality );
  quality->Delete();

  quality = vtkDoubleArray::New();
  quality->SetName( "Mesh Hexahedron Quality" );
  quality->SetNumberOfComponents(5);
  tuple[0] = qhexm;
  tuple[1] = Eqhex;
  tuple[2] = qhexM;
  tuple[3] = Eqhex2;
  tuple[4] = nhex;
  quality->InsertNextTuple( tuple );
  out->GetFieldData()->AddArray( quality );
  quality->Delete();

  return 1;
}


// Triangle quality measure:
// edge collapse

double vtkMeshQualityExtended::TriangleEdgeCollapse( vtkCell* cell)
{
  double p0[3],p1[3],p2[3];

  vtkPoints *p = cell->GetPoints();
  p->GetPoint(0, p0);
  p->GetPoint(1, p1);
  p->GetPoint(2, p2);

  double a[3],b[3],c[3];
 
  a[0] = fabs( p2[0] - p0[0] );
  a[1] = fabs( p2[1] - p0[1] );
  a[2] = fabs( p2[2] - p0[2] );
 
  b[0] = fabs( p1[0] - p0[0] );
  b[1] = fabs( p1[1] - p0[1] );
  b[2] = fabs( p1[2] - p0[2] );
 
  c[0] = fabs( p2[0] - p1[0] );
  c[1] = fabs( p2[1] - p1[1] );
  c[2] = fabs( p2[2] - p1[2] );
 
  
  // now we have all the triangle edges, lets examine if any segments are length 0 and 
  // return low quality
  
  if ((a[0] == a[1]) && (a[1] == a[2]) && (a[2] == 0)) 
    return -1.0;
  else if ((b[0] == b[1]) && (b[1] == b[2]) && (b[2] == 0)) 
    return -1.0;
  else if ((c[0] == c[1]) && (c[1] == c[2]) && (c[2] == 0)) 
    return -1.0;
  else
    return 1.0;
}

// Quadraleral quality measure:
// edge collapse

double vtkMeshQualityExtended::QuadEdgeCollapse( vtkCell* cell)
{
  double p0[3],p1[3],p2[3],p3[3];

  vtkPoints *p = cell->GetPoints();
  p->GetPoint(0, p0);
  p->GetPoint(1, p1);
  p->GetPoint(2, p2);
  p->GetPoint(3, p3);

  double a[3],b[3],c[3],d[3];

  a[0] = fabs( p1[0] - p0[0] );
  a[1] = fabs( p1[1] - p0[1] );
  a[2] = fabs( p1[2] - p0[2] );
 
  b[0] = fabs( p2[0] - p1[0] );
  b[1] = fabs( p2[1] - p1[1] );
  b[2] = fabs( p2[2] - p1[2] );
 
  c[0] = fabs( p3[0] - p2[0] );
  c[1] = fabs( p3[1] - p2[1] );
  c[2] = fabs( p3[2] - p2[2] );
 
  d[0] = fabs( p0[0] - p3[0] );
  d[1] = fabs( p0[1] - p3[1] );
  d[2] = fabs( p0[2] - p3[2] );
  
  // now we have all the tet edges, lets examine if any segments are length 0 and 
  // return low quality (0.01) if any segments are zero length.  This should be modified for 
  // a zero tolerance test of "close-enough-to-zero"  
  
  if ((a[0] == a[1]) && (a[1] == a[2]) && (a[2] == 0)) 
    return -1.0;
  else if ((b[0] == b[1]) && (b[1] == b[2]) && (b[2] == 0)) 
    return -1.0;
  else if ((c[0] == c[1]) && (c[1] == c[2]) && (c[2] == 0)) 
    return -1.0;
  else if ((d[0] == d[1]) && (d[1] == d[2]) && (d[2] == 0)) 
    return -1.0;
  else
    return 1.0;
}

// Tetrahedron quality measure:
// edge collapse

double vtkMeshQualityExtended::TetEdgeCollapse( vtkCell* cell)
{
  double p0[3],p1[3],p2[3],p3[3];

  vtkPoints *p = cell->GetPoints();
  p->GetPoint(0, p0);
  p->GetPoint(1, p1);
  p->GetPoint(2, p2);
  p->GetPoint(3, p3);

  double a[3],b[3],c[3],d[3],e[3],f[3];

  a[0] = fabs( p3[0] - p0[0] );
  a[1] = fabs( p3[1] - p0[1] );
  a[2] = fabs( p3[2] - p0[2] );
 
  b[0] = fabs( p2[0] - p0[0] );
  b[1] = fabs( p2[1] - p0[1] );
  b[2] = fabs( p2[2] - p0[2] );
 
  c[0] = fabs( p1[0] - p0[0] );
  c[1] = fabs( p1[1] - p0[1] );
  c[2] = fabs( p1[2] - p0[2] );
 
  d[0] = fabs( p2[0] - p1[0] );
  d[1] = fabs( p2[1] - p1[1] );
  d[2] = fabs( p2[2] - p1[2] );
 
  e[0] = fabs( p3[0] - p1[0] );
  e[1] = fabs( p3[1] - p1[1] );
  e[2] = fabs( p3[2] - p1[2] );
 
  f[0] = fabs( p3[0] - p2[0] );
  f[1] = fabs( p3[1] - p2[1] );
  f[2] = fabs( p3[2] - p2[2] );
  
  // now we have all the tet edges, lets examine if any segments are length 0 and 
  // return low quality (0.01) if any segments are zero length.  This should be modified for 
  // a zero tolerance test of "close-enough-to-zero"  
  
  if ((a[0] == a[1]) && (a[1] == a[2]) && (a[2] == 0)) 
    return -1.0;
  else if ((b[0] == b[1]) && (b[1] == b[2]) && (b[2] == 0)) 
    return -1.0;
  else if ((c[0] == c[1]) && (c[1] == c[2]) && (c[2] == 0)) 
    return -1.0;
  else if ((d[0] == d[1]) && (d[1] == d[2]) && (d[2] == 0)) 
    return -1.0;
  else if ((e[0] == e[1]) && (e[1] == e[2]) && (e[2] == 0)) 
    return -1.0;
  else if ((f[0] == f[1]) && (f[1] == f[2]) && (f[2] == 0)) 
    return -1.0;
  else
    return 1.0;
}



// Hexahedron quality measure:
// edge collapse

double vtkMeshQualityExtended::HexEdgeCollapse( vtkCell* cell)
{
  double p0[3],p1[3],p2[3],p3[3];
  double p4[3],p5[3],p6[3],p7[3];

  vtkPoints *p = cell->GetPoints();
  p->GetPoint(0, p0);
  p->GetPoint(1, p1);
  p->GetPoint(2, p2);
  p->GetPoint(3, p3);
  p->GetPoint(4, p4);
  p->GetPoint(5, p5);
  p->GetPoint(6, p6);
  p->GetPoint(7, p7);

  double a[3],b[3],c[3],d[3],e[3],f[3];
  double g[3],h[3],i[3],j[3],k[3],l[3];

  a[0] = p1[0] - p0[0];
  a[1] = p1[1] - p0[1];
  a[2] = p1[2] - p0[2];
 
  b[0] = p2[0] - p1[0];
  b[1] = p2[1] - p1[1];
  b[2] = p2[2] - p1[2];
 
  c[0] = p3[0] - p2[0];
  c[1] = p3[1] - p2[1];
  c[2] = p3[2] - p2[2];
 
  d[0] = p0[0] - p3[0];
  d[1] = p0[1] - p3[1];
  d[2] = p0[2] - p3[2];
 
  e[0] = p4[0] - p0[0];
  e[1] = p4[1] - p0[1];
  e[2] = p4[2] - p0[2];
 
  f[0] = p5[0] - p1[0];
  f[1] = p5[1] - p1[1];
  f[2] = p5[2] - p1[2];

  g[0] = p6[0] - p2[0];
  g[1] = p6[1] - p2[1];
  g[2] = p6[2] - p2[2];

  h[0] = p7[0] - p3[0];
  h[1] = p7[1] - p3[1];
  h[2] = p7[2] - p3[2];

  i[0] = p5[0] - p4[0];
  i[1] = p5[1] - p4[1];
  i[2] = p5[2] - p4[2];
 
  j[0] = p6[0] - p5[0];
  j[1] = p6[1] - p5[1];
  j[2] = p6[2] - p5[2];

  k[0] = p7[0] - p6[0];
  k[1] = p7[1] - p6[1];
  k[2] = p7[2] - p6[2];

  l[0] = p4[0] - p7[0];
  l[1] = p4[1] - p7[1];
  l[2] = p4[2] - p7[2];
  
  // now we have all the hex edges, lets examine if any segments are length 0 and 
  // return low quality (0.01) if any segments are zero length.  This should be modified for 
  // a zero tolerance test of "close-enough-to-zero"
  
  //cout << "a[0]=" << a[0] << " a[1]=" << a[1] << " a[2]=" << a[2] << endl;
  //cout << "b[0]=" << b[0] << " b[1]=" << b[1] << " b[2]=" << b[2] << endl;
  //cout << "c[0]=" << c[0] << " c[1]=" << c[1] << " c[2]=" << c[2] << endl;
  //cout << "d[0]=" << d[0] << " d[1]=" << d[1] << " d[2]=" << d[2] << endl;
  //cout << "e[0]=" << e[0] << " e[1]=" << e[1] << " e[2]=" << e[2] << endl;
  //cout << "f[0]=" << f[0] << " f[1]=" << f[1] << " f[2]=" << f[2] << endl;
  //cout << "g[0]=" << g[0] << " g[1]=" << g[1] << " g[2]=" << g[2] << endl;
  //cout << "h[0]=" << h[0] << " h[1]=" << h[1] << " h[2]=" << h[2] << endl;
  //cout << "i[0]=" << i[0] << " i[1]=" << i[1] << " i[2]=" << i[2] << endl;
  //cout << "j[0]=" << j[0] << " j[1]=" << j[1] << " j[2]=" << j[2] << endl;
  //cout << "k[0]=" << k[0] << " k[1]=" << k[1] << " k[2]=" << k[2] << endl;
  //cout << "l[0]=" << l[0] << " l[1]=" << l[1] << " l[2]=" << l[2] << endl;
  
  if ((a[0] == a[1]) && (a[1] == a[2]) && (a[2] == 0)) 
    return -1.0;
  else if ((b[0] == b[1]) && (b[1] == b[2]) && (b[2] == 0)) 
    return -1.0;
  else if ((c[0] == c[1]) && (c[1] == c[2]) && (c[2] == 0)) 
    return -1.0;
  else if ((d[0] == d[1]) && (d[1] == d[2]) && (d[2] == 0)) 
    return -1.0;
  else if ((e[0] == e[1]) && (e[1] == e[2]) && (e[2] == 0)) 
    return -1.0;
  else if ((f[0] == f[1]) && (f[1] == f[2]) && (f[2] == 0)) 
    return -1.0;
  else if ((g[0] == g[1]) && (g[1] == g[2]) && (g[2] == 0)) 
    return -1.0;
  else if ((h[0] == h[1]) && (h[1] == h[2]) && (h[2] == 0)) 
    return -1.0;
  else if ((i[0] == i[1]) && (i[1] == i[2]) && (i[2] == 0)) 
    return -1.0;
  else if ((j[0] == j[1]) && (j[1] == j[2]) && (j[2] == 0)) 
    return -1.0;
  else if ((k[0] == k[1]) && (k[1] == k[2]) && (k[2] == 0)) 
    return -1.0;
  else if ((l[0] == l[1]) && (l[1] == l[2]) && (l[2] == 0)) 
    return -1.0;
  else
    return 1.0;
}


// Triangle quality measure:
// Angle Out of Bounds

double vtkMeshQualityExtended::TriangleAngleOutOfBounds( vtkCell* cell)
{
  double p0[3],p1[3],p2[3];
  double angle[3];

  vtkPoints *p = cell->GetPoints();
  p->GetPoint(0, p0);
  p->GetPoint(1, p1);
  p->GetPoint(2, p2);

  double a[3],b[3],c[3];

  a[0] = p1[0] - p0[0];
  a[1] = p1[1] - p0[1];
  a[2] = p1[2] - p0[2];
 
  b[0] = p2[0] - p1[0];
  b[1] = p2[1] - p1[1];
  b[2] = p2[2] - p1[2];
 
  c[0] = p0[0] - p2[0];
  c[1] = p0[1] - p2[1];
  c[2] = p0[2] - p2[2];
 

  //Cos 30 = 0.866025404
  //Cos 90 = 0 
  // now we have all the lines, so investigate the 24 angles made up by combining the 12 lines
  // two at a time.  Normalize the vectors, so the dot product will have the value of the cosine
  // of the angle between them.   
  vtkMath::Normalize(a);
  vtkMath::Normalize(b);
  vtkMath::Normalize(c);
  
  
  // to really get the angle, we need to do the inverse cosine. However, we are testing
  // for  < 30 or > 90, The Cos 30 = 0.866025404 and Cos 90 = 0 

  // corner 0 - by Point 0
  angle[0] = (vtkMath::Dot(a,c));
  
  // corner 1
  angle[1] = (vtkMath::Dot(b,a));
  
  // corner 2
  angle[2] = (vtkMath::Dot(c,b));
  

  for (int i=0; i<3; i++) 
    {
    // if the absolute value of the dot-product is greater than 0.707 then
    // the angle is either less than 45 or greater than 135
    if ( (angle[i] > 0.0) && (angle[i] < 0.866025404) )
      {
      // this is an element is out of the range 30-90
      return -1.0;
      }
    }
  // all angles were within range, so return a high-quality value for this element
  return 1.0;
}


// Quadraheral quality measure:
// Angle Out of Bounds

double vtkMeshQualityExtended::QuadAngleOutOfBounds( vtkCell* cell)
{
  double p0[3],p1[3],p2[3],p3[3];
  double angle[4];

  vtkPoints *p = cell->GetPoints();
  p->GetPoint(0, p0);
  p->GetPoint(1, p1);
  p->GetPoint(2, p2);
  p->GetPoint(3, p3);

  double a[3],b[3],c[3],d[3];

  a[0] = p1[0] - p0[0];
  a[1] = p1[1] - p0[1];
  a[2] = p1[2] - p0[2];
 
  b[0] = p2[0] - p1[0];
  b[1] = p2[1] - p1[1];
  b[2] = p2[2] - p1[2];
 
  c[0] = p3[0] - p2[0];
  c[1] = p3[1] - p2[1];
  c[2] = p3[2] - p2[2];
 
  d[0] = p0[0] - p3[0];
  d[1] = p0[1] - p3[1];
  d[2] = p0[2] - p3[2];
 
  // now we have all the lines, so investigate the 24 angles made up by combining the 12 lines
  // two at a time.  Normalize the vectors, so the dot product will have the value of the cosine
  // of the angle between them.   
  vtkMath::Normalize(a);
  vtkMath::Normalize(b);
  vtkMath::Normalize(c);
  vtkMath::Normalize(d);
  
  // to really get the angle, we need to do the inverse cosine. 
  // The angle test here is 45 and 135 which are both 0.707
  
  // corner 0 - by Point 0
  angle[0] = (vtkMath::Dot(a,d));
  
  // corner 1
  angle[1] = (vtkMath::Dot(b,a));
    
  //corner 2 by point 2 
  angle[2] = (vtkMath::Dot(c,b));
  
  // corner 3
  angle[3] = (vtkMath::Dot(d,c));
  

  for (int i=0; i<4; i++) 
    {
    // if the absolute value of the dot-product is greater than 0.707 then
    // the angle is either less than 45 or greater than 135
    if (fabs(angle[i]) > 0.707)
      {
      // this is an element that has low quality, so return 0
      return -1.0;
      }
    }
  // all angles were within range, so return a high-quality value for this element
  return 1.0;
}


// Tetrahedral quality measure:
// edge collapse

double vtkMeshQualityExtended::TetAngleOutOfBounds( vtkCell* cell)
{
  double p0[3],p1[3],p2[3],p3[3];
  double angle[9];

  vtkPoints *p = cell->GetPoints();
  p->GetPoint(0, p0);
  p->GetPoint(1, p1);
  p->GetPoint(2, p2);
  p->GetPoint(3, p3);

  double a[3],b[3],c[3],d[3],e[3],f[3];

  a[0] = p1[0] - p0[0];
  a[1] = p1[1] - p0[1];
  a[2] = p1[2] - p0[2];
 
  b[0] = p2[0] - p1[0];
  b[1] = p2[1] - p1[1];
  b[2] = p2[2] - p1[2];
 
  c[0] = p2[0] - p0[0];
  c[1] = p2[1] - p0[1];
  c[2] = p2[2] - p0[2];
  
  d[0] = p3[0] - p0[0];
  d[1] = p3[1] - p0[1];
  d[2] = p3[2] - p0[2];
 
  e[0] = p3[0] - p1[0];
  e[1] = p3[1] - p1[1];
  e[2] = p3[2] - p1[2];
 
  f[0] = p3[0] - p2[0];
  f[1] = p3[1] - p2[1];
  f[2] = p3[2] - p2[2];
 

  // now we have all the lines, so investigate the 12 angles made up by combining the 6 lines
  // two at a time.  Normalize the vectors, so the dot product will have the value of the cosine
  // of the angle between them.   
  vtkMath::Normalize(a);
  vtkMath::Normalize(b);
  vtkMath::Normalize(c);
  vtkMath::Normalize(d);
  vtkMath::Normalize(e);
  vtkMath::Normalize(f);
  
  // to really get the angle, we need to do the inverse cosine. However, we are testing
  // for  < 30 or > 90, The Cos 30 = 0.866025404 and Cos 90 = 0 
  
  // corner 0 - by Point 0
  angle[0] = (vtkMath::Dot(a,c));
  angle[1] = (vtkMath::Dot(a,d));
  angle[2] = (vtkMath::Dot(c,d));
  
  // corner 1
  angle[3] = (vtkMath::Dot(a,b));
  angle[4] = (vtkMath::Dot(e,b));
  angle[5] = (vtkMath::Dot(e,a));
    
  //corner 2 
  angle[6] = (vtkMath::Dot(b,c));
  angle[7] = (vtkMath::Dot(f,b));
  angle[8] = (vtkMath::Dot(f,c));

  for (int i=0; i<8; i++) 
    {
    std::cout << " Angle " << i << ": " << angle[i];
    }
  std::cout << std::endl;
  
  
  for (int i=0; i<8; i++) 
    {
    // if the absolute value of the dot-product is greater than 0.707 then
    // the angle is either less than 45 or greater than 135
    if ( (angle[i] > 0.0) && (angle[i] < 0.866025404) )
      {
      // this is an element that has low quality, so return 0
      return -1.0;
      }
    }
  // all angles were within range, so return a high-quality value for this element
  return 1.0;
}

// Hexahedron quality measure:
// Angle out of Bounds

double vtkMeshQualityExtended::HexAngleOutOfBounds( vtkCell* cell)
{
  double p0[3],p1[3],p2[3],p3[3];
  double p4[3],p5[3],p6[3],p7[3];
  double angle[24];

  vtkPoints *p = cell->GetPoints();
  p->GetPoint(0, p0);
  p->GetPoint(1, p1);
  p->GetPoint(2, p2);
  p->GetPoint(3, p3);
  p->GetPoint(4, p4);
  p->GetPoint(5, p5);
  p->GetPoint(6, p6);
  p->GetPoint(7, p7);

  double a[3],b[3],c[3],d[3],e[3],f[3];
  double g[3],h[3],i[3],j[3],k[3],l[3];

  a[0] = p1[0] - p0[0];
  a[1] = p1[1] - p0[1];
  a[2] = p1[2] - p0[2];
 
  b[0] = p2[0] - p1[0];
  b[1] = p2[1] - p1[1];
  b[2] = p2[2] - p1[2];
 
  c[0] = p3[0] - p2[0];
  c[1] = p3[1] - p2[1];
  c[2] = p3[2] - p2[2];
 
  d[0] = p0[0] - p3[0];
  d[1] = p0[1] - p3[1];
  d[2] = p0[2] - p3[2];
 
  e[0] = p4[0] - p0[0];
  e[1] = p4[1] - p0[1];
  e[2] = p4[2] - p0[2];
 
  f[0] = p5[0] - p1[0];
  f[1] = p5[1] - p1[1];
  f[2] = p5[2] - p1[2];

  g[0] = p6[0] - p2[0];
  g[1] = p6[1] - p2[1];
  g[2] = p6[2] - p2[2];

  h[0] = p7[0] - p3[0];
  h[1] = p7[1] - p3[1];
  h[2] = p7[2] - p3[2];

  i[0] = p5[0] - p4[0];
  i[1] = p5[1] - p4[1];
  i[2] = p5[2] - p4[2];
 
  j[0] = p6[0] - p5[0];
  j[1] = p6[1] - p5[1];
  j[2] = p6[2] - p5[2];

  k[0] = p7[0] - p6[0];
  k[1] = p7[1] - p6[1];
  k[2] = p7[2] - p6[2];

  l[0] = p4[0] - p7[0];
  l[1] = p4[1] - p7[1];
  l[2] = p4[2] - p7[2];


  // now we have all the lines, so investigate the 24 angles made up by combining the 12 lines
  // two at a time.  Normalize the vectors, so the dot product will have the value of the cosine
  // of the angle between them.   
  vtkMath::Normalize(a);
  vtkMath::Normalize(b);
  vtkMath::Normalize(c);
  vtkMath::Normalize(d);
  vtkMath::Normalize(e);
  vtkMath::Normalize(f);
  vtkMath::Normalize(g);  
  vtkMath::Normalize(h);
  vtkMath::Normalize(i);
  vtkMath::Normalize(j);
  vtkMath::Normalize(k);
  vtkMath::Normalize(l);
  
  // to really get the angle, we need to do the inverse cosine. However, we are testing
  // for  < 45 or > 135, both cases where cos=-0.707, so we can just check the absolute
  // value of the dot product to be > 0.707 for an angle that meets our criteria of too
  // small or too large
  
  // angle[0] = acos(vtkMath::Dot(a,e))= 45degrees   or angle[0]=(vtkMath::Dot(a,e)) = 0.707

  
  // corner 0 - by Point 0
  angle[0] = (vtkMath::Dot(a,e));
  angle[1] = (vtkMath::Dot(d,e));
  angle[2] = (vtkMath::Dot(a,d));
  
  // corner 1
  angle[3] = (vtkMath::Dot(b,a));
  angle[4] = (vtkMath::Dot(b,f));
  angle[5] = (vtkMath::Dot(f,a));
  
  //cout <<  "a0=" << angle[0]<< " a1=" << angle[1] << "a2= " << angle[2] << endl;
  //cout <<  "a3=" << angle[3]<< " a4=" << angle[4] << "a5= " << angle[5] << endl;
    
  //corner 2 by point 2 
  angle[6] = (vtkMath::Dot(c,b));
  angle[7] = (vtkMath::Dot(c,g));
  angle[8] = (vtkMath::Dot(b,g));
  
  // corner 3
  angle[9] = (vtkMath::Dot(c,d));
  angle[10] = (vtkMath::Dot(c,h));
  angle[11] = (vtkMath::Dot(d,h));
  
  // corner 4
  angle[12] = (vtkMath::Dot(i,e));
  angle[13] = (vtkMath::Dot(l,e));
  angle[14] = (vtkMath::Dot(l,i));

  // corner 5
  angle[15] = (vtkMath::Dot(i,f));
  angle[16] = (vtkMath::Dot(i,j));
  angle[17] = (vtkMath::Dot(f,j));

  // corner 6
  angle[18] = (vtkMath::Dot(j,g));
  angle[19] = (vtkMath::Dot(k,g));
  angle[20] = (vtkMath::Dot(k,j));

  // corner 7
  angle[21] = (vtkMath::Dot(l,k));
  angle[22] = (vtkMath::Dot(h,l));
  angle[23] = (vtkMath::Dot(h,k));

  for (int ii=0; ii<24; ii++) 
    {
    // if the absolute value of the dot-product is greater than 0.707 then
    // the angle is either less than 45 or greater than 135
    if (fabs(angle[ii]) > 0.707)
      {
      //cout << "Found a small / big angle !!" << endl;
      // this is an element that has low quality, so return 0
      return -1.0;
      }
    }
  // all angles were within range, so return a high-quality value for this element
  return 1.0;
}
