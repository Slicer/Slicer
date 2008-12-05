/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMeshQualityExtended.h,v $
Language:  C++
Date:      $Date: 2008/06/28 01:24:43 $
Version:   $Revision: 1.2 $

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
// .NAME vtkMeshQualityExtended - Calculate measures of quality of a mesh
//
// .SECTION Description
// vtkMeshQualityExtended extends the metrics provided by vtkMeshQuality
// to also include an edge collaspe and angle out of bound metrics for 
// Hexahedron cells. 
//
// .SECTION Caveats
// 

#ifndef __vtkMeshQualityExtended_h
#define __vtkMeshQualityExtended_h

#include "vtkMeshQuality.h"
#include "vtkMimxFilterWin32Header.h"

class vtkCell;
class vtkDataArray;

/* Added Mesh Quality Metrics by Curt Lisle */
#define VTK_QUALITY_EDGE_COLLAPSE 30
#define VTK_QUALITY_ANGLE_OUT_OF_BOUNDS 31

class VTK_MIMXFILTER_EXPORT vtkMeshQualityExtended : public vtkMeshQuality
{
public:
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeRevisionMacro(vtkMeshQualityExtended,vtkMeshQuality);
  static vtkMeshQualityExtended* New();

  
  // Description:
  // Set/Get the particular estimator used to measure the quality of hexahedra.
  // Two aditional metrics are now provided:
  //   VTK_QUALITY_EDGE_COLLAPSE and VTK_QUALITY_ANGLE_OUT_OF_BOUNDS

  void SetHexQualityMeasureToEdgeCollapse()
    {
    this->SetHexQualityMeasure( VTK_QUALITY_EDGE_COLLAPSE );
    }
  void SetHexQualityMeasureToAngleOutOfBounds()
    {
    this->SetHexQualityMeasure( VTK_QUALITY_ANGLE_OUT_OF_BOUNDS );
    }

  void SetTetQualityMeasureToEdgeCollapse()
    {
    this->SetTetQualityMeasure( VTK_QUALITY_EDGE_COLLAPSE );
    }
  void SetTetQualityMeasureToAngleOutOfBounds()
    {
    this->SetTetQualityMeasure( VTK_QUALITY_ANGLE_OUT_OF_BOUNDS );
    }

  void SetQuadQualityMeasureToEdgeCollapse()
    {
    this->SetQuadQualityMeasure( VTK_QUALITY_EDGE_COLLAPSE );
    }
  void SetQuadQualityMeasureToAngleOutOfBounds()
    {
    this->SetQuadQualityMeasure( VTK_QUALITY_ANGLE_OUT_OF_BOUNDS );
    }
  
  void SetTriangleQualityMeasureToEdgeCollapse()
    {
    this->SetTriangleQualityMeasure( VTK_QUALITY_EDGE_COLLAPSE );
    }
  void SetTriangleQualityMeasureToAngleOutOfBounds()
    {
    this->SetTriangleQualityMeasure( VTK_QUALITY_ANGLE_OUT_OF_BOUNDS );
    }

  vtkSetMacro(MetricTolerance, double);
  vtkGetMacro(MetricTolerance, double);
   
  // Description:
  // This is a static function used to calculate if any of the edges of the hex have collapsed
  // to zero length.  This returns a 1 if collapse occured and a 0 otherwise -- no type checking is
  // performed because this method is called from the inner loop of the Execute()
  // member function.
  // HISTORY:  Added for Iowa FE project April 2007
  
  static double HexEdgeCollapse( vtkCell* cell );
  static double HexAngleOutOfBounds(vtkCell* cell);
  static double TriangleEdgeCollapse( vtkCell* cell);
  static double QuadEdgeCollapse( vtkCell* cell);
  static double TetEdgeCollapse( vtkCell* cell);
  static double TriangleAngleOutOfBounds( vtkCell* cell);
  static double QuadAngleOutOfBounds( vtkCell* cell);
  static double TetAngleOutOfBounds( vtkCell* cell);

protected:
  vtkMeshQualityExtended();
  ~vtkMeshQualityExtended();

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  vtkMeshQualityExtended( const vtkMeshQualityExtended& ); // Not implemented.
  void operator = ( const vtkMeshQualityExtended& ); // Not implemented.
  
  double MetricTolerance;
};

#endif // vtkMeshQualityExtended_h
