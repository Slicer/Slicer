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
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women’s Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __vtkITKLabelShapeStatistics_h
#define __vtkITKLabelShapeStatistics_h

#include "vtkITK.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>
#include <vtkTableAlgorithm.h>
#include <vtkVector.h>

// vtkAddon includes
#include <vtkAddonSetGet.h>

// std includes
#include <vector>

class vtkPoints;

/// \brief ITK-based utilities for calculating label statistics.
/// Utilizes itk::LabelImageToShapeLabelMapFilter to calcualte label shape statistics
/// (https://itk.org/Doxygen/html/classitk_1_1LabelImageToShapeLabelMapFilter.html)
/// Label centroid and flatness are the only statistics calculated by default.
/// For a list of availiable parameters, see: vtkITKLabelShapeStatistics::ShapeStatistic
/// Calculated statistics can be changed using the SetComputeShapeStatistic/ComputeShapeStatisticOn/ComputeShapeStatisticOff methods.
/// Output statistics are represented in a vtkTable where each column represents a statistic and each row is a different label value.
class VTK_ITK_EXPORT vtkITKLabelShapeStatistics : public vtkTableAlgorithm
{
public:
  static vtkITKLabelShapeStatistics *New();
  vtkTypeMacro(vtkITKLabelShapeStatistics, vtkTableAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Shape statistic parameters
  /// See parameter definitions and formulas here: http://hdl.handle.net/1926/584
  enum ShapeStatistic
  {
    /// Location of the center of mass of the label
    Centroid,
    /// Oriented bounding box of the label
    OrientedBoundingBox,
    /// Diameter of the sphere that contains the label
    FeretDiameter,
    /// Surface area of the label
    Perimeter,
    /// Ratio of the area of the hypersphere by the actual area. A value of 1 represents a spherical structure
    Roundness,
    /// Square root of the ratio of the second smallest principal moment by the smallest. A value of 0 represents a flat structure
    Flatness,
    /// Square root of the ratio of the second largest principal moment by the second smallest
    Elongation,
    /// Principal moments of inertia for the principal axes
    PrincipalMoments,
    // Principal axes of rotation
    PrincipalAxes,
    ShapeStatistic_Last,
  };

  static std::string GetShapeStatisticAsString(ShapeStatistic statistic);
  static ShapeStatistic GetShapeStatisticFromString(std::string statisticName);

  vtkSetObjectMacro(Directions, vtkMatrix4x4);
  vtkSetStdVectorMacro(ComputedStatistics, std::vector<std::string>);
  vtkGetStdVectorMacro(ComputedStatistics, std::vector<std::string>);

  /// Set/Get if the the specified statistic should be computed.
  /// Label centroid and flatness are computed by default.
  /// Other statistics are listed in the ShapeStatistic enum.
  void SetComputeShapeStatistic(std::string statisticName, bool state);
  bool GetComputeShapeStatistic(std::string statisticName);
  void ComputeShapeStatisticOn(std::string statisticName);
  void ComputeShapeStatisticOff(std::string statisticName);

protected:
  vtkITKLabelShapeStatistics();
  ~vtkITKLabelShapeStatistics() override;

  int FillInputPortInformation(int vtkNotUsed(port), vtkInformation* info) override;
  int RequestData(vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector) override;

protected:
  std::vector<std::string> ComputedStatistics;
  vtkMatrix4x4* Directions;

private:
  vtkITKLabelShapeStatistics(const vtkITKLabelShapeStatistics&) = delete;
  void operator=(const vtkITKLabelShapeStatistics&) = delete;
};

#endif
