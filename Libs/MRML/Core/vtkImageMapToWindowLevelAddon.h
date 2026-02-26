/*==============================================================================

  Copyright (c) TBD

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Alex Allphin, PhD at Revvity.

==============================================================================*/
/**
 * @class   vtkImageMapToWindowLevelAddon
 * @brief   Subclass of vtkImageMapToWindowLevelColors that adds the option to compress values within the window logarithmically.
 *
 * This class is a subclass of vtkImageMapToWindowLevelColors that adds optional logarithmic window compression.
 *
 * In the long term, this class will be removed and the functionality should be added to vtkImageMapToWindowLevelColors, if it is found to be useful.
 */

#ifndef vtkImageMapToWindowLevelAddon_h
#define vtkImageMapToWindowLevelAddon_h

// VTK includes
#include "vtkImageMapToWindowLevelColors.h"

// MRML includes
#include "vtkMRML.h"

class VTK_MRML_EXPORT vtkImageMapToWindowLevelAddon : public vtkImageMapToWindowLevelColors
{
public:
  vtkTypeMacro(vtkImageMapToWindowLevelAddon, vtkImageMapToWindowLevelColors);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkImageMapToWindowLevelAddon* New();

  /// Specifies whether values should be compressed within the window
  vtkBooleanMacro(LogCompressWindow, int);
  vtkGetMacro(LogCompressWindow, int);
  vtkSetMacro(LogCompressWindow, int);

protected:
  vtkImageMapToWindowLevelAddon();
  ~vtkImageMapToWindowLevelAddon() override;

  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  void ThreadedRequestData(vtkInformation* request,
                           vtkInformationVector** inputVector,
                           vtkInformationVector* outputVector,
                           vtkImageData*** inData,
                           vtkImageData** outData,
                           int outExt[6],
                           int id) override;
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;

  int LogCompressWindow;

private:
  vtkImageMapToWindowLevelAddon(const vtkImageMapToWindowLevelAddon&) = delete;
  void operator=(const vtkImageMapToWindowLevelAddon&) = delete;
};
#endif
