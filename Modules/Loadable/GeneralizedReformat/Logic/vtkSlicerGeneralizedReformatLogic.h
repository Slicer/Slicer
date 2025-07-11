/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// .NAME vtkSlicerGeneralizedReformatLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes

#ifndef __vtkSlicerGeneralizedReformatLogic_h
#define __vtkSlicerGeneralizedReformatLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// GeneralizedReformat MRML includes
#include <vtkMRMLAbstractVolumeResampler.h>

// MRML includes
class vtkMRMLMarkupsCurveNode;
class vtkMRMLScalarVolumeNode;

// VTK includes
class vtkMatrix4x4;
class vtkPoints;
#include <vtkSmartPointer.h>

// STD includes
#include <cstdlib>
#include <map>

#include "vtkSlicerGeneralizedReformatModuleLogicExport.h"

class VTK_SLICER_GENERALIZEDREFORMAT_MODULE_LOGIC_EXPORT vtkSlicerGeneralizedReformatLogic : public vtkSlicerModuleLogic
{
public:
  static vtkSlicerGeneralizedReformatLogic* New();
  vtkTypeMacro(vtkSlicerGeneralizedReformatLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Projects input points onto a specified plane.
  ///
  /// \param pointsArrayIn Input points in world coordinates.
  /// \param transformWorldToPlane Transformation matrix from world to plane coordinates.
  /// \param pointsArrayOut Output points projected onto the plane.
  ///
  /// \return True if projection is successful, false if input parameters are null.
  ///
  /// This function transforms input points into the plane's coordinate system, where the plane's
  /// normal aligns with the Z-axis. The projection is performed by setting the Z coordinate of
  /// each point to zero, effectively flattening it onto the plane. The points are then transformed
  /// back into world coordinates and stored in `pointsArrayOut`.
  bool GetPointsProjectedToPlane(vtkPoints* pointsArrayIn, vtkMatrix4x4* transformWorldToPlane, vtkPoints* pointsArrayOut);

  /// Compute transform that straightens volume (useful for example for visualization of curved vessels)
  ///
  /// \param transformToStraightenedNode the computed transform
  /// \param curveNode curve that defines the planar reformation
  /// \param sliceSizeMm dimensions of slice that is produced
  /// \param outputSpacingMm spacing to be used along the curve
  /// \param stretching if True then stretching transform will be computed, otherwise straightening
  /// \param rotationDeg number of degrees to rotate the output
  /// \param reslicingPlanesModelNode
  ///
  /// \return True if the transform computation is successful, false if input parameters are null.
  ///
  /// \sa GetPointsProjectedToPlane()
  bool ComputeStraighteningTransform(vtkMRMLTransformNode* transformToStraightenedNode,
                                     vtkMRMLMarkupsCurveNode* curveNode,
                                     const double sliceSizeMm[2],
                                     double outputSpacingMm,
                                     bool stretching = false,
                                     double rotationDeg = 0.0,
                                     vtkMRMLModelNode* reslicingPlanesModelNode = nullptr);

  /// Straighten volume for curved planar reformation
  ///
  /// \param outputStraightenedVolume result of straightening computation
  /// \param volumeNode input volume to be straightened
  /// \param outputStraightenedVolumeSpacing spacing in the output volume
  /// \param straighteningTransformNode transform to be applied
  ///
  /// \return True if the volume computation is successful, false if input parameters are null.
  ///
  /// \sa vtkMRMLApplicationLogic::IsVolumeResamplerRegistered()
  /// \sa ResampleVolume()
  bool StraightenVolume(vtkMRMLScalarVolumeNode* outputStraightenedVolume,
                        vtkMRMLScalarVolumeNode* volumeNode,
                        const double outputStraightenedVolumeSpacing[3],
                        vtkMRMLTransformNode* straighteningTransformNode);

  /// Create panoramic volume by mean intensity projection along an axis of the straightened volume
  ///
  /// \param outputProjectedVolume resulting 2-dimensional projection
  /// \param inputStraightenedVolume input volume to be projected
  /// \param projectionAxisIndex axis to perform the projection on
  ///
  /// \return True if the projection volume computation is successful, false if input parameters are null.
  bool ProjectVolume(vtkMRMLScalarVolumeNode* outputProjectedVolume, vtkMRMLScalarVolumeNode* inputStraightenedVolume, int projectionAxisIndex = 0);

  /// Resample volume using the registered resampler.
  ///
  /// \param resamplerName the name as registered with vtkMRMLApplicationLogic
  /// \param inputVolume the volume to be resampled
  /// \param outputVolume the resulting volume
  /// \param resamplingTransform the transform to apply
  /// \param referenceVolume
  /// \param interpolationType see enum in vtkMRMLAbstractVolumeResampler.h
  /// \param resamplingParameters reserved for future use
  ///
  /// \return True if the volume resampling computation is successful, false if input parameters are null.
  ///
  /// \sa vtkMRMLApplicationLogic::GetVolumeResampler()
  bool ResampleVolume(std::string& resamplerName,
                      vtkMRMLVolumeNode* inputVolume,
                      vtkMRMLVolumeNode* outputVolume,
                      vtkMRMLTransformNode* resamplingTransform,
                      vtkMRMLVolumeNode* referenceVolume = nullptr,
                      int interpolationType = vtkMRMLAbstractVolumeResampler::InterpolationTypeLinear,
                      const vtkMRMLAbstractVolumeResampler::ResamplingParameters& resamplingParameters = vtkMRMLAbstractVolumeResampler::ResamplingParameters());

  /// @{
  /// Register/unregister resampler.
  void RegisterVolumeResampler(const std::string& resamplerName, vtkMRMLAbstractVolumeResampler* resampler);
  void UnregisterVolumeResampler(const std::string& resamplerName);
  bool IsVolumeResamplerRegistered(const std::string& resamplerName);
  vtkMRMLAbstractVolumeResampler* GetVolumeResampler(const std::string& resamplerName);
  /// @}

protected:
  vtkSlicerGeneralizedReformatLogic();
  ~vtkSlicerGeneralizedReformatLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;

  double TransformSpacingFactor{ 5.0 };

  std::map<std::string, vtkSmartPointer<vtkMRMLAbstractVolumeResampler>> Resamplers;

private:
  vtkSlicerGeneralizedReformatLogic(const vtkSlicerGeneralizedReformatLogic&); // Not implemented
  void operator=(const vtkSlicerGeneralizedReformatLogic&);                    // Not implemented
};

#endif
