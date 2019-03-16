
#ifndef __vtkITKLevelTracingImageFilter_h
#define __vtkITKLevelTracingImageFilter_h

#include "vtkITK.h"
#include "vtkPolyDataAlgorithm.h"
#include "vtkObjectFactory.h"

/// \brief Wrapper class around itk::LevelTracingImageFilterImageFilter.
///
/// itk::LevelTracingImageFilter
/// LevelTracingImageFilter traces a level curve (or surface) from a
/// seed point.  The pixels on this level curve "boundary" are labeled
/// as 1. Does nothing if seed is in uniform area.
///
/// This filter is specialized to volumes. If you are interested in
/// contouring other types of data, use the general vtkContourFilter. If you
/// want to contour an image (i.e., a volume slice), use vtkMarchingSquares.
class VTK_ITK_EXPORT vtkITKLevelTracingImageFilter : public vtkPolyDataAlgorithm
{
public:
  static vtkITKLevelTracingImageFilter *New();
  vtkTypeMacro(vtkITKLevelTracingImageFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Methods to set/get seeds.  Seeds are specified in IJK (not XYZ).
  vtkSetVector3Macro(Seed, int);
  vtkGetVector3Macro(Seed, int);

  /// Method to set the plane (IJ=2, IK=1, JK=0)
  vtkSetMacro(Plane, int);
  vtkGetMacro(Plane, int);

  void SetPlaneToIJ() {this->SetPlane(2);}
  void SetPlaneToIK() {this->SetPlane(1);}
  void SetPlaneToJK() {this->SetPlane(0);}

protected:
  vtkITKLevelTracingImageFilter();
  ~vtkITKLevelTracingImageFilter() override;

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
  int FillInputPortInformation(int port, vtkInformation *info) override;

  int Seed[3];
  int Plane;

private:
  vtkITKLevelTracingImageFilter(const vtkITKLevelTracingImageFilter&) = delete;
  void operator=(const vtkITKLevelTracingImageFilter&) = delete;
};

#endif
