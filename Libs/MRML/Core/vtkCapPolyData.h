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
  and was supported in part through NIH grant R01 HL153166.

==============================================================================*/
/**
 * @class   vtkCapPolyData
 * @brief   Filter to generate an end cap for a polydata cut with the specified function.
 *
 * This class is a subclass of vtkPolyDataAlgorithm that will generate an end cap for a polydata cut with the specified function.
 * The end cap is generated only for vtkPlane defined in the vtkImplicitFunction. The planes can be stored in vtkPlanes or nested vtkImplicitBoolean functions,
 * however it cannot extract planes in vtkImplicitSum.
 *
 * The end cap is generated by generating cross sections of the input polydata with the planes defined in the vtkImplicitFunction, which are then triangulated.
 * The triangulated contours are appended together and clipped with the implicit function to generate the end caps.
 */

#ifndef vtkCapPolyData_h
#define vtkCapPolyData_h

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkImplicitFunction.h>
#include <vtkPolyDataAlgorithm.h>

// MRML includes
#include "vtkMRML.h"

class vtkPlaneCollection;

class VTK_MRML_EXPORT vtkCapPolyData : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkCapPolyData, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkCapPolyData* New();

  //@{
  /**
   * Specify the implicit function with which to perform the
   * clipping. If you do not define an implicit function, then the input
   * scalar data will be used for clipping.
   */
  vtkSetMacro(ClipFunction, vtkSmartPointer<vtkImplicitFunction>);
  vtkGetMacro(ClipFunction, vtkSmartPointer<vtkImplicitFunction>);
  //@}

  /// Return the mtime also considering the locator and clip function.
  vtkMTimeType GetMTime() override;

  /// Get the list of planes from the implicit function
  static void GetPlanes(vtkImplicitFunction* function, vtkPlaneCollection* planes, vtkAbstractTransform* parentTransform = nullptr);

  //@{
  /**
   * Set/Get whether to generate an outline of the cap.
   * Default is on.
   */
  vtkSetMacro(GenerateOutline, bool);
  vtkGetMacro(GenerateOutline, bool);
  //@}

  //@{
  /**
   * Set/Get whether to generate cell type scalars.
   * The cell type scalars can be used to color the mesh by cell type.
   * Ex. Different colors for polygons and lines.
   * Default is on.
   */
  vtkSetMacro(GenerateCellTypeScalars, bool);
  vtkGetMacro(GenerateCellTypeScalars, bool);
  //@}

protected:
  vtkCapPolyData();
  ~vtkCapPolyData() override;

  /// Generate the end cap for the input polydata cut using planes in the cutFunction.
  void CreateEndCap(vtkPlaneCollection* planes, vtkPolyData* originalPolyData, vtkImplicitFunction* cutFunction, vtkPolyData* outputEndCap);

  /// Updates the polydata cell scalar array to reflect the cell type.
  void UpdateCellTypeArray(vtkPolyData* polyData);

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

protected:
  vtkSmartPointer<vtkImplicitFunction> ClipFunction;

  bool GenerateOutline{ true };
  bool GenerateCellTypeScalars{ true };

private:
  vtkCapPolyData(const vtkCapPolyData&) = delete;
  void operator=(const vtkCapPolyData&) = delete;
};
#endif
