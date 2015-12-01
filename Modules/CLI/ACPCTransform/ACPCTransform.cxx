/*=========================================================================

  Program:   Realign Volumes
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#include "ACPCTransformCLP.h"
#include "vtkPluginFilterWatcher.h"
#include "vtkPrincipalAxesAlign.h"

// MRML includes
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkTransform.h>

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

} // end of anonymous namespace

//-----------------------------------------------------------------------------
int main(int argc, char * argv[])
{
  PARSE_ARGS;

  if( debugSwitch )
    {
    std::cout << "Trying to get transforms out...\n";
    std::cout << "OutputTransform = " << OutputTransform.c_str() << std::endl;
    }

  // tease apart the scene files and the node ids
  std::string::size_type loc;
  std::string            OutputTransformFilename;
  std::string            OutputTransformID;

  loc = OutputTransform.find_last_of("#");
  if( loc != std::string::npos )
    {
    OutputTransformFilename = std::string(OutputTransform.begin(),
                                          OutputTransform.begin() + loc);
    loc++;

    OutputTransformID = std::string(OutputTransform.begin() + loc, OutputTransform.end() );
    }

  if( debugSwitch )
    {
    std::cout << "The ACPC fiducial list is " <<  std::endl;
    ::size_t i;
    for( i = 0; i < ACPC.size(); i++ )
      {
      std::cout << i << ": " << ACPC[i][0] << ", " << ACPC[i][1] << ", " << ACPC[i][2] << std::endl;
      }
    std::cout << "The midline fiducial list is: " << std::endl;
    for( i = 0; i < Midline.size(); i++ )
      {
      std::cout << i << ": " << Midline[i][0] << ", " << Midline[i][1] << ", " << Midline[i][2] << std::endl;
      }
    std::cout << "OutputTransform filename: " << OutputTransformFilename << std::endl;
    std::cout << "OutputTransform ID: " << OutputTransformID << std::endl;
    std::cout << "\nStarting..." << std::endl;
    }

  // if have input lists and an output transform, calculate
  if( ACPC.size() > 0 && Midline.size() > 0 &&
      OutputTransformID.length() > 0 )
    {
    }

  vtkMRMLLinearTransformNode *outNode = NULL;

  // read in the scene, output and input transform file names should be the same
  vtkNew<vtkMRMLScene> scene;

  scene->SetURL( OutputTransformFilename.c_str() );
  scene->Import();
  // get the output transform
  vtkMRMLNode *node = scene->GetNodeByID( OutputTransformID );
  if( node )
    {
    outNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
    if( !outNode )
      {
      std::cout << "No output transform node found. Specified output transform ID = " << OutputTransformID << "."
                << std::endl;
      }
    }

  // increment after each filter is run
  float currentFilterOffset = 0.0;
  float numFilterSteps = 2.0;

  // fill in this transform with either the output or input matrix
  vtkNew<vtkTransform> transformToApply;
  transformToApply->Identity();
  transformToApply->PostMultiply();

  if( Midline.size() > 0 )
    {
    if( debugSwitch )
      {
      std::cout << "Doing Midline..." << std::endl;
      }
    vtkNew<vtkMath>     math;
    vtkNew<vtkPolyData> polydata;
    vtkNew<vtkPolyData> output;
    vtkNew<vtkPoints>   points;
    points->SetDataTypeToDouble();
    size_t x = Midline.size();
    if( debugSwitch )
      {
      std::cout << "Total number of midline points " << x << ", points data type = " << points->GetDataType()
                << std::endl;
      }
    points->SetNumberOfPoints(x);
    for( size_t i = 0; i < x; ++i )
      {
      points->SetPoint(i, Midline[i][0], Midline[i][1], Midline[i][2]);
      if( debugSwitch )
        {
        double pt[3];
        points->GetPoint(i, pt);
        std::cout << "Set midline point " << i << " to " << pt[0] << ", " << pt[1] << "," << pt[2] << "\n";
        }
      }
    polydata->SetPoints(points.GetPointer());

    vtkNew<vtkPrincipalAxesAlign> pa;
    if( debugSwitch )
      {
      std::cout << "Set Input to PrincipalAxesAlign\n";
      }
    pa->SetInputData(polydata.GetPointer());
    if( debugSwitch )
      {
      std::cout << "Executing PrincipalAxesAlign\n";
      }
    vtkPluginFilterWatcher watchPA(pa.GetPointer(),
                                   "Principle Axes Align",
                                   CLPProcessInformation,
                                   1.0 / numFilterSteps,
                                   currentFilterOffset / numFilterSteps);
    currentFilterOffset++;
    pa->Update();

    double *normal = pa->GetZAxis();
    double  nx = normal[0];
    double  ny = normal[1];
    double  nz = normal[2];
    if( debugSwitch )
      {
      std::cout << "Normal " << nx << " " << ny << " " << nz << std::endl;
      }

    double Max = nx;
    if( ny * ny > Max * Max )
      {
      Max = ny;
      }
    if( nz * nz > Max * Max )
      {
      Max = nz;
      }
    double sign = 1.0;
    if( Max < 0 )
      {
      sign = -1.0;
      }

    // prepare the rotation matrix
    vtkNew<vtkMatrix4x4> mat;
    mat->Identity();
    for( size_t p = 0; p < 4; p++ )
      {
      double point = normal[p];
      mat->SetElement(static_cast<int>(p), 0, (sign * point) );
      }
    double oneAndAlpha = 1.0 + mat->GetElement(0, 0);
    mat->SetElement(0, 1, -1.0 * mat->GetElement(1, 0) );
    mat->SetElement(0, 2, (-1.0 * (mat->GetElement(2, 0) ) ) );
    mat->SetElement(2, 1, (-1.0 * (mat->GetElement(1, 0) * (mat->GetElement(2, 0) / oneAndAlpha) ) ) );
    mat->SetElement(1, 2, (-1.0 * (mat->GetElement(1, 0) * (mat->GetElement(2, 0) / oneAndAlpha) ) ) );
    mat->SetElement(1, 1, (1.0  - (mat->GetElement(1, 0) * (mat->GetElement(1, 0) / oneAndAlpha) ) ) );
    mat->SetElement(2, 2, (1.0  - (mat->GetElement(2, 0) * (mat->GetElement(2, 0) / oneAndAlpha) ) ) );

    // Check the sign of the determinant
    double det = mat->Determinant();
    if( debugSwitch )
      {
      std::cout << "Determinant " << det << endl;
      }
    vtkNew<vtkMatrix4x4> matInverse;
    matInverse->DeepCopy(mat.GetPointer());
    matInverse->Invert();
    transformToApply->SetMatrix(matInverse.GetPointer());
    }

  // need at least two points
  if( ACPC.size() > 1 )
    {
    if( debugSwitch )
      {
      std::cout << "Doing ACPC, size = " << ACPC.size() << "\n";
      }
    double top = ACPC[0][2] - ACPC[1][2];
    double bot = ACPC[0][1] - ACPC[1][1];
    double tangent = atan(top / bot) * (180.0 / (4.0 * atan(1.0) ) );
    if( debugSwitch )
      {
      std::cout << "Tangent (top = " << top << ", bot = " << bot << ") = " << tangent << endl;
      }
    transformToApply->RotateX(tangent * -1.0);
    }

  // clean up
  if( outNode )
    {
    if( debugSwitch )
      {
      std::cout << "setting matix on outnode, and committing scene " << OutputTransformFilename.c_str() << std::endl;
      }
    outNode->SetMatrixTransformToParent(transformToApply->GetMatrix() );
    scene->Commit( OutputTransformFilename.c_str() );
    }
  return EXIT_SUCCESS;
}
