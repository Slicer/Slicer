/*==============================================================================

  Program: 3D Slicer

  Copyright(c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkMRMLScalarVectorDWIVolumeResampler_h
#define __vtkMRMLScalarVectorDWIVolumeResampler_h

#include "qSlicerGeneralizedReformatModuleExport.h"

// GeneralizedReformat MRML includes
#include "vtkMRMLAbstractVolumeResampler.h"

// MRML includes
class vtkMRMLVolumeNode;
class vtkMRMLTransformNode;

class Q_SLICER_QTMODULES_GENERALIZEDREFORMAT_EXPORT vtkMRMLScalarVectorDWIVolumeResampler : public vtkMRMLAbstractVolumeResampler
{
public:
  static vtkMRMLScalarVectorDWIVolumeResampler *New();
  vtkTypeMacro(vtkMRMLScalarVectorDWIVolumeResampler, vtkMRMLAbstractVolumeResampler);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  virtual bool Resample(vtkMRMLVolumeNode* inputVolume,
                        vtkMRMLVolumeNode* outputVolume,
                        vtkMRMLTransformNode* resamplingTransform,
                        vtkMRMLVolumeNode* referenceVolume,
                        int interpolationType,
                        const vtkMRMLAbstractVolumeResampler::ResamplingParameters& resamplingParameter) override;

protected:
  vtkMRMLScalarVectorDWIVolumeResampler() = default;
  ~vtkMRMLScalarVectorDWIVolumeResampler() override = default;
  vtkMRMLScalarVectorDWIVolumeResampler(const vtkMRMLScalarVectorDWIVolumeResampler&) = delete;
  void operator=(const vtkMRMLScalarVectorDWIVolumeResampler&) = delete;
};

#endif
