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
 * @class   vtkImageMathematicsAddon
 * @brief   Subclass of vtkImageMathematics that adds the option to multiply an image by another image which has its
 * scalar values scaled.
 *
 * This class is a subclass of vtkImageMathematics that adds an additional operation:
 * - MultiplyByScaledRange: Multiply an image by the value of the second image, scaled using the specified scalar range.
 *
 * In the long term, this class will be removed and the functionality should be added to vtkImageMathematics, if it is
 * found to be useful.
 */

#ifndef vtkImageMathematicsAddon_h
#define vtkImageMathematicsAddon_h

#define VTK_MULTIPLYBYSCALEDRANGE 100

// VTK includes
#include "vtkImageMathematics.h"

// MRML includes
#include "vtkMRML.h"

class VTK_MRML_EXPORT vtkImageMathematicsAddon : public vtkImageMathematics
{
public:
  vtkTypeMacro(vtkImageMathematicsAddon, vtkImageMathematics);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkImageMathematicsAddon* New();

  void SetOperationToMultiplyByScaledRange() { this->SetOperation(VTK_MULTIPLYBYSCALEDRANGE); }

  vtkSetVector2Macro(Range, double);
  vtkGetVector2Macro(Range, double);

protected:
  vtkImageMathematicsAddon();
  ~vtkImageMathematicsAddon() override;

  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  void ThreadedRequestData(vtkInformation* request,
                           vtkInformationVector** inputVector,
                           vtkInformationVector* outputVector,
                           vtkImageData*** inData,
                           vtkImageData** outData,
                           int outExt[6],
                           int threadId) override;

  double Range[2] = { 0.0, 1.0 };

private:
  vtkImageMathematicsAddon(const vtkImageMathematicsAddon&) = delete;
  void operator=(const vtkImageMathematicsAddon&) = delete;
};
#endif
