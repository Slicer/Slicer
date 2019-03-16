/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

  This file is a modified version of vtkPolyDataToImageStencil.h

==============================================================================*/

#ifndef vtkPolyDataToFractionalLabelmapFilter_h
#define vtkPolyDataToFractionalLabelmapFilter_h


// VTK includes
#include <vtkPolyDataToImageStencil.h>
#include <vtkSmartPointer.h>
#include <vtkCellArray.h>
#include <vtkSetGet.h>
#include <vtkMatrix4x4.h>
#include <vtkCellLocator.h>

// Segmentations includes
#include <vtkOrientedImageData.h>

// std includes
#include <map>

#include "vtkSegmentationCoreConfigure.h"

// Define the datatype and fractional constants for fractional labelmap conversion based on the value of VTK_FRACTIONAL_DATA_TYPE
#define VTK_FRACTIONAL_DATA_TYPE VTK_CHAR

#if VTK_FRACTIONAL_DATA_TYPE == VTK_UNSIGNED_CHAR
  #define FRACTIONAL_DATA_TYPE VTK_TYPE_NAME_UNSIGNED_CHAR
  #define FRACTIONAL_MIN 0
  #define FRACTIONAL_MAX 216
  #define FRACTIONAL_STEP_SIZE 1
#elif VTK_FRACTIONAL_DATA_TYPE == VTK_CHAR
  #define FRACTIONAL_DATA_TYPE VTK_TYPE_NAME_CHAR
  #define FRACTIONAL_MIN -108
  #define FRACTIONAL_MAX 108
  #define FRACTIONAL_STEP_SIZE 1
#elif VTK_FRACTIONAL_DATA_TYPE == VTK_FLOAT
  #define FRACTIONAL_DATA_TYPE VTK_TYPE_NAME_FLOAT
  #define FRACTIONAL_MIN 0.0
  #define FRACTIONAL_MAX 1.0
  #define FRACTIONAL_STEP_SIZE (1.0/216.0)
#endif

class vtkSegmentationCore_EXPORT vtkPolyDataToFractionalLabelmapFilter :
  public vtkPolyDataToImageStencil
{
private:
  std::map<double, vtkSmartPointer<vtkCellArray> > LinesCache;
  std::map<double, vtkSmartPointer<vtkPolyData> > SliceCache;
  std::map<double, vtkIdType*> PointIdsCache;
  std::map<double, vtkIdType> NptsCache;
  std::map<double,  vtkSmartPointer<vtkIdTypeArray> > PointNeighborCountsCache;

  vtkCellLocator* CellLocator;

  vtkOrientedImageData* OutputImageTransformData;
  int NumberOfOffsets;

public:
  static vtkPolyDataToFractionalLabelmapFilter* New();
  vtkTypeMacro(vtkPolyDataToFractionalLabelmapFilter, vtkPolyDataToImageStencil);

  virtual vtkOrientedImageData* GetOutput();
  virtual void SetOutput(vtkOrientedImageData* output);

  void SetOutputImageToWorldMatrix(vtkMatrix4x4* imageToWorldMatrix);
  void GetOutputImageToWorldMatrix(vtkMatrix4x4* imageToWorldMatrix);

  using Superclass::GetOutputOrigin;
  double* GetOutputOrigin() override;
  void GetOutputOrigin(double origin[3]) override;

  void SetOutputOrigin(double origin[3]) override;
  void SetOutputOrigin(double x, double y, double z) override;

  using Superclass::GetOutputSpacing;
  double* GetOutputSpacing() override;
  void GetOutputSpacing(double spacing[3]) override;

  void SetOutputSpacing(double spacing[3]) override;
  void SetOutputSpacing(double x, double y, double z) override;


  /// This method deletes the currently stored cache variables
  void DeleteCache();

  vtkSetMacro(NumberOfOffsets, int);
  vtkGetMacro(NumberOfOffsets, int);

protected:
  vtkPolyDataToFractionalLabelmapFilter();
  ~vtkPolyDataToFractionalLabelmapFilter() override;

  int RequestData(vtkInformation *, vtkInformationVector **,
                          vtkInformationVector *) override;
  vtkOrientedImageData *AllocateOutputData(vtkDataObject *out, int* updateExt);
  int FillOutputPortInformation(int, vtkInformation*) override;

  /// Create a binary image stencil for the closed surface within the current extent
  /// This method is a modified version of vtkPolyDataToImageStencil::ThreadedExecute
  /// \param output Output stencil data
  /// \param closedSurface The input surface to be converted
  /// \param extent The extent region that is being converted
  void FillImageStencilData(vtkImageStencilData *output, vtkPolyData* closedSurface, int extent[6]);

  /// Add the values of the binary labelmap to the fractional labelmap.
  /// \param binaryLabelMap Binary labelmap that will be added to the fractional labelmap
  /// \param fractionalLabelMap The fractional labelmap that the binary labelmap is added to
  void AddBinaryLabelMapToFractionalLabelMap(vtkImageData* binaryLabelMap, vtkImageData* fractionalLabelMap);

  /// Clip the polydata at the specified z coordinate to create a planar contour.
  /// This method is a modified version of vtkPolyDataToImageStencil::PolyDataCutter to decrease execution time
  /// \param input The closed surface that is being cut
  /// \param output Polydata containing the contour lines
  /// \param z The z coordinate for the cutting plane
  void PolyDataCutter(vtkPolyData *input, vtkPolyData *output,
                             double z);

private:
  vtkPolyDataToFractionalLabelmapFilter(const vtkPolyDataToFractionalLabelmapFilter&) = delete;
  void operator=(const vtkPolyDataToFractionalLabelmapFilter&) = delete;
};

#endif
