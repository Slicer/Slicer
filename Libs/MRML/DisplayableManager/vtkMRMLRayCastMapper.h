/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef vtkMRMLRayCastMapper_h
#define vtkMRMLRayCastMapper_h

// MRMLDisplayableManager includes
#include "vtkMRMLDisplayableManagerExport.h"

// VTK includes
#include <vtkMapper.h>

/// \brief Base class of mappers that render surfaces without geometry that a cell picker could intersect.
///
/// Such mappers (for example surfaces that are ray cast on the GPU) implement IntersectWithRay, so that
/// vtkMRMLAccuratePicker can pick them: the picker asks the mapper where the pick ray hits the surface instead of
/// intersecting the cells of the input data set.
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLRayCastMapper : public vtkMapper
{
public:
  vtkAbstractTypeMacro(vtkMRMLRayCastMapper, vtkMapper);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Intersect a ray with the rendered surface. The ray is given in the model coordinates of the prop
  /// (the coordinate system of the input of the mapper), from p1 to p2; intersections are accepted between
  /// parametric coordinates t1 and t2 (0 at p1, 1 at p2).
  /// \param t Parametric coordinate of the first intersection along the ray
  /// \param position Position of the first intersection, in model coordinates
  /// \param normal Surface normal at the intersection, in model coordinates
  /// \return True if the ray hits the surface between t1 and t2
  virtual bool IntersectWithRay(const double p1[3], const double p2[3], double t1, double t2, double& t, double position[3], double normal[3]) = 0;

protected:
  vtkMRMLRayCastMapper() = default;
  ~vtkMRMLRayCastMapper() override = default;

private:
  vtkMRMLRayCastMapper(const vtkMRMLRayCastMapper&) = delete;
  void operator=(const vtkMRMLRayCastMapper&) = delete;
};

#endif
