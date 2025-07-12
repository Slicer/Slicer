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
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __vtkITKGrowCut_h
#define __vtkITKGrowCut_h

#include "vtkITK.h"

// VTK includes
#include "vtkImageAlgorithm.h"
#include "vtkImageData.h"

class vtkInformation;
class vtkInformationVector;

/// \brief ITK-based utilities for manipulating connected regions in label maps.
/// Limitation: The filter does not work correctly with input volume that has
/// unsigned long scalar type on Linux and macOS.
///
class VTK_ITK_EXPORT vtkITKGrowCut : public vtkImageAlgorithm
{
public:
  static vtkITKGrowCut* New();
  vtkTypeMacro(vtkITKGrowCut, vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Set input grayscale volume (input 0)
  void SetIntensityVolume(vtkImageData* intensityVolume) { this->SetInputData(0, intensityVolume); }

  /// Set input seed label volume (input 1)
  void SetSeedLabelVolume(vtkImageData* labelImage) { this->SetInputData(1, labelImage); }

  /// Set mask volume (input 2). Optional.
  /// If this volume is specified then only those regions outside the mask (where mask has zero value)
  /// will be included in the segmentation result. Regions outside the mask will not be used
  /// for region growing either (growing will not start from or cross through masked region).
  void SetMaskVolume(vtkImageData* labelImage) { this->SetInputData(2, labelImage); }

  /// Reset to initial state. This forces full recomputation of the result label volume.
  /// This method has to be called if intensity volume changes or if seeds are deleted after initial computation.
  void Reset();

  /// Spatial regularization factor, which can force growing in nearby regions.
  /// For each physical unit distance, this much intensity level difference is simulated.
  /// By default = 0, which means spatial distance does not play a role in the region growing, only intensity value
  /// similarity.
  vtkGetMacro(DistancePenalty, double);
  void SetDistancePenalty(double distancePenalty);

protected:
  vtkITKGrowCut();
  ~vtkITKGrowCut() override;

  int FillInputPortInformation(int port, vtkInformation* info) override;
  void ExecuteDataWithInformation(vtkDataObject* outData, vtkInformation* outInfo) override;

  double DistancePenalty{ 0.0 };

private:
  vtkITKGrowCut(const vtkITKGrowCut&) = delete;
  void operator=(const vtkITKGrowCut&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
