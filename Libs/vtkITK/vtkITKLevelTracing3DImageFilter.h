
#ifndef __vtkITKLevelTracing3DImageFilter_h
#define __vtkITKLevelTracing3DImageFilter_h

#include "vtkITK.h"
#include "vtkImageAlgorithm.h"
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
class VTK_ITK_EXPORT vtkITKLevelTracing3DImageFilter : public vtkImageAlgorithm
{
public:
  static vtkITKLevelTracing3DImageFilter *New();
  vtkTypeMacro(vtkITKLevelTracing3DImageFilter, vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Methods to set/get seeds.  Seeds are specified in IJK (not XYZ).
  vtkSetVector3Macro(Seed, int);
  vtkGetVector3Macro(Seed, int);

protected:
  vtkITKLevelTracing3DImageFilter();
  ~vtkITKLevelTracing3DImageFilter() override;

  int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
  int FillInputPortInformation(int port, vtkInformation *info) override;

  int Seed[3];

private:
  vtkITKLevelTracing3DImageFilter(const vtkITKLevelTracing3DImageFilter&) = delete;
  void operator=(const vtkITKLevelTracing3DImageFilter&) = delete;
};

#endif
