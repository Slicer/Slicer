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

  The signed distance field computation (jump flooding) and the ray casting
  are based on SlicerLive by Steve Pieper (https://github.com/pieper/SlicerLive,
  Apache License 2.0).

==============================================================================*/

#ifndef __vtkSegmentationLabelmapSurfaceMapper_h
#define __vtkSegmentationLabelmapSurfaceMapper_h

#include "vtkSlicerSegmentationsModuleMRMLDisplayableManagerExport.h"

// MRMLDisplayableManager includes
#include <vtkMRMLRayCastMapper.h>

class vtkOrientedImageData;

/// \brief Render a labelmap as smooth surfaces, computed on the GPU.
///
/// The mapper computes a signed distance field of the labelmap on the GPU
/// (jump flooding algorithm, in fragment shader passes that write 3D texture slices,
/// followed by Gaussian smoothing) and ray casts its zero level set.
/// No polygonal mesh is generated, so the surface is updated quickly when the labelmap changes.
///
/// Labels that are not added by SetLabelColor are treated as background.
/// Model coordinates of the mapper are the IJK coordinates of the labelmap
/// (the actor's matrix is expected to map them to world coordinates).
///
/// Uses only OpenGL features that are available in both OpenGL 3.2 and OpenGL ES 3.0 (WebGL2),
/// with 16-bit floating-point render targets (EXT_color_buffer_float on WebGL2).
class VTK_SLICER_SEGMENTATIONS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkSegmentationLabelmapSurfaceMapper : public vtkMRMLRayCastMapper
{
public:
  static vtkSegmentationLabelmapSurfaceMapper* New();
  vtkTypeMacro(vtkSegmentationLabelmapSurfaceMapper, vtkMRMLRayCastMapper);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Labelmap to render. Geometry (spacing, directions) is taken into account when computing distances.
  void SetLabelmap(vtkOrientedImageData* labelmap);
  vtkOrientedImageData* GetLabelmap();

  /// Show the voxels that have the specified label value with the specified color and opacity.
  /// Surfaces of segments that touch each other are only shown where they touch if they are in different mappers.
  void SetLabelColor(int labelValue, double r, double g, double b, double opacity = 1.0);
  /// Remove all label colors (all labels are hidden).
  void RemoveAllLabelColors();

  /// Smoothing of the surface, between 0 (no smoothing, surface follows voxel boundaries) and 1 (strong smoothing).
  /// Same meaning as the "Smoothing factor" conversion parameter of closed surface representation.
  /// The distance field is smoothed by a Gaussian (standard deviation is 1.7 * SmoothingFactor times the largest voxel size,
  /// the same in all directions in physical space), combined with the result of applying it twice ("twicing"),
  /// so that curved surfaces do not shrink and thin structures are preserved.
  /// Gaps between segments that are narrower than the standard deviation are filled.
  vtkSetClampMacro(SmoothingFactor, double, 0.0, 1.0);
  vtkGetMacro(SmoothingFactor, double);

  /// The labelmap is downsampled if its distance field would have more voxels than this.
  /// Larger values preserve more details, but need more GPU memory: about 11 bytes per voxel while the distance field
  /// is computed, 3 bytes per voxel after that. Each mapper instance holds its own distance field and color textures,
  /// so the memory is multiplied by the number of mappers that show the same labelmap (the segmentations displayable
  /// manager uses one mapper for the opaque segments of a segmentation and one for each distinct translucent opacity).
  /// A downsampled voxel is shown if any of its labelmap voxels is shown, which is checked in at most 4 voxels along
  /// each axis: if the downsampling factor is above 4 along an axis (labelmap larger than about 1300^3 voxels at the
  /// default limit), the block is sampled at evenly spaced voxels and small structures between them may be missed.
  vtkSetClampMacro(MaximumNumberOfVoxels, vtkIdType, 4096, VTK_ID_MAX);
  vtkGetMacro(MaximumNumberOfVoxels, vtkIdType);

  /// Show the surface where clipping planes cut segments (enabled by default).
  /// Clipping planes are set by SetClippingPlanes, in world coordinates; the kept region is where all plane functions are
  /// positive (as for other VTK mappers), or where any of them is, see KeepWhereAnyClippingPlaneKeeps.
  vtkSetMacro(CapClippedSurface, bool);
  vtkGetMacro(CapClippedSurface, bool);
  vtkBooleanMacro(CapClippedSurface, bool);

  /// Opacity of the cap surface, relative to the opacity of the segment.
  vtkSetClampMacro(CapOpacity, double, 0.0, 1.0);
  vtkGetMacro(CapOpacity, double);

  /// Keep the region where any clipping plane function is positive, instead of where all of them are (disabled by default).
  /// This is the ClipIntersection clip type of vtkMRMLClipNode (which clips away the intersection of the clipped spaces).
  vtkSetMacro(KeepWhereAnyClippingPlaneKeeps, bool);
  vtkGetMacro(KeepWhereAnyClippingPlaneKeeps, bool);
  vtkBooleanMacro(KeepWhereAnyClippingPlaneKeeps, bool);

  /// Show the outline of the segments on clipping planes: the curve where the planes cut the surface (disabled by default).
  vtkSetMacro(ClippingOutline, bool);
  vtkGetMacro(ClippingOutline, bool);
  vtkBooleanMacro(ClippingOutline, bool);

  /// Color of the clipping outline.
  vtkSetVector3Macro(OutlineColor, double);
  vtkGetVector3Macro(OutlineColor, double);

  /// Width of the clipping outline, in pixels.
  vtkSetClampMacro(OutlineWidth, double, 0.5, 100.0);
  vtkGetMacro(OutlineWidth, double);

  /// Rays are cast for every n-th pixel across and down (1: every pixel), and the image is scaled up to the viewport,
  /// as with ImageSampleDistance of vtkGPUVolumeRayCastMapper. Larger values make rendering faster at the expense of
  /// detail, for example while the camera is moving.
  vtkSetClampMacro(ImageSampleDistance, double, 1.0, 8.0);
  vtkGetMacro(ImageSampleDistance, double);

  /// Number of times the signed distance field has been computed (for testing).
  vtkGetMacro(NumberOfDistanceFieldComputations, int);

  /// Intersect a ray (in IJK coordinates of the labelmap, between P1 + T1 * (P2 - P1) and P1 + T2 * (P2 - P1)) with
  /// the surface of the shown segments (computed from the labelmap on the CPU, the surface follows the boundary of
  /// voxels, not smoothed). Returns true if there is an intersection, and its parametric coordinate, position and normal.
  /// The ray is clipped as the mapper clips what it renders, in all of its clipping modes (KeepWhereAnyClippingPlaneKeeps):
  /// what the clipping planes clip away is not hit, and where they cut a segment the cap is hit if it is shown
  /// (CapClippedSurface, with a CapOpacity above 0). toWorld is the matrix of the actor (IJK to world), which the
  /// clipping planes are transformed with; if it is nullptr then the clipping planes are ignored.
  /// vtkMRMLAccuratePicker picks the surface with this.
  bool IntersectWithRay(const double p1[3], const double p2[3], double t1, double t2, vtkMatrix4x4* toWorld, double& t, double xyz[3], double n[3]) override;

  /// Get the label of the shown segment that is at the specified position (in IJK coordinates of the labelmap),
  /// or the nearest one within the specified distance (in voxels). Returns 0 if there is no shown segment there.
  int GetShownLabelAtPosition(const double ijk[3], double maximumDistance = 1.5);

  void Render(vtkRenderer* ren, vtkActor* actor) override;
  void ReleaseGraphicsResources(vtkWindow* window) override;
  using Superclass::GetBounds;
  double* GetBounds() VTK_SIZEHINT(6) override;

protected:
  vtkSegmentationLabelmapSurfaceMapper();
  ~vtkSegmentationLabelmapSurfaceMapper() override;

  int FillInputPortInformation(int port, vtkInformation* info) override;

  double SmoothingFactor{ 0.5 };
  double ImageSampleDistance{ 1.0 };
  bool CapClippedSurface{ true };
  double CapOpacity{ 1.0 };
  bool KeepWhereAnyClippingPlaneKeeps{ false };
  bool ClippingOutline{ false };
  double OutlineColor[3]{ 0.0, 0.0, 0.0 };
  double OutlineWidth{ 1.0 };
  vtkIdType MaximumNumberOfVoxels{ 32 * 1024 * 1024 };
  int NumberOfDistanceFieldComputations{ 0 };

private:
  vtkSegmentationLabelmapSurfaceMapper(const vtkSegmentationLabelmapSurfaceMapper&) = delete;
  void operator=(const vtkSegmentationLabelmapSurfaceMapper&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
