/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
///  vtkImageFillROI - Paints on top of an image.
///
/// vtkImageFillROI will draw a polygon, line, or points in an
/// image.  As opposed to vtkImageDrawROI, which draws only outlines
/// around these shapes, this filter will also fill the inside of
/// polygons.
/// This filter is used when applying image editing effects,
/// while vtkImageDrawROI is used while the user is actually
/// drawing (for temporary interactive display).  So it is
/// important that the output from this filter (vtkImageFIllROI)
/// correspond to that of the vtkImageDrawROI filter!
//

#ifndef __vtkImageFillROI_h
#define __vtkImageFillROI_h

#include "vtkSlicerBaseLogic.h"

// VTK includes
#include <vtkImageAlgorithm.h>

#define SHAPE_POLYGON 1
#define SHAPE_LINES 2
#define SHAPE_POINTS 3

class vtkPoints;

class VTK_SLICER_BASE_LOGIC_EXPORT vtkImageFillROI : public vtkImageAlgorithm
{
public:
  static vtkImageFillROI* New();
  vtkTypeMacro(vtkImageFillROI, vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkSetMacro(Value, double);
  vtkGetMacro(Value, double);

  void SetShapeToPolygon() { this->Shape = SHAPE_POLYGON; };
  void SetShapeToLines() { this->Shape = SHAPE_LINES; };
  void SetShapeToPoints() { this->Shape = SHAPE_POINTS; };
  void SetShape(int s) { this->Shape = s; };
  int GetShape() { return this->Shape; };

  const char* GetShapeString()
  {
    switch (this->Shape)
    {
      case SHAPE_POLYGON:
        return "Polygon";
      case SHAPE_LINES:
        return "Lines";
      case SHAPE_POINTS:
        return "Points";
      default:
        return "None";
    }
  }

  void SetShapeString(const char* str)
  {
    if (strcmp(str, "Polygon") == 0)
      this->SetShapeToPolygon();
    else if (strcmp(str, "Lines") == 0)
      this->SetShapeToLines();
    else
      this->SetShapeToPoints();
  }

  vtkSetMacro(Radius, int);
  vtkGetMacro(Radius, int);

  virtual void SetPoints(vtkPoints*);
  vtkGetObjectMacro(Points, vtkPoints);

protected:
  vtkImageFillROI();
  ~vtkImageFillROI() override;

  vtkPoints* Points;
  double Value;
  int Radius;
  int Shape;

  /// Reimplemented.
  /// Not threaded because too simple a filter
  int RequestData(vtkInformation* request,
                  vtkInformationVector** inputVectors,
                  vtkInformationVector* outputVector) override;

private:
  vtkImageFillROI(const vtkImageFillROI&) = delete;
  void operator=(const vtkImageFillROI&) = delete;
};

#endif
