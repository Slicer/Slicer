
#ifndef __vtkITKWandImageFilter_h
#define __vtkITKWandImageFilter_h

#include "vtkITK.h"
#include "vtkSimpleImageToImageFilter.h"

/// \brief Wand tool implemented using connected threshold.
///
/// itk::WandImageFilter
/// WandImageFilter selects all pixels connected to a seed point that
/// are within a specified intensity difference of the seed point.
class VTK_ITK_EXPORT vtkITKWandImageFilter : public vtkSimpleImageToImageFilter
{
public:
  static vtkITKWandImageFilter *New();
  vtkTypeMacro(vtkITKWandImageFilter, vtkSimpleImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Methods to set/get seeds.  Seeds are specified in IJK (not XYZ).
  vtkSetVector3Macro(Seed, int);
  vtkGetVector3Macro(Seed, int);

  /// Set/Get the intensity difference to connect as a function of the
  /// dynamic range
  vtkSetClampMacro(DynamicRangePercentage, double, 0.0, 1.0);
  vtkGetMacro(DynamicRangePercentage, double);

protected:
  vtkITKWandImageFilter();
  ~vtkITKWandImageFilter() override;

  void SimpleExecute(vtkImageData* input, vtkImageData* output) override;

  int Seed[3];
  double DynamicRangePercentage;

private:
  vtkITKWandImageFilter(const vtkITKWandImageFilter&) = delete;
  void operator=(const vtkITKWandImageFilter&) = delete;
};

#endif
