/*==========================================================================

  Copyright (c) Ebatinca S.L., Las Palmas de Gran Canaria, Spain

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, Ebatinca, funded
  by the grant GRT-00000485 of Children's Hospital of Philadelphia, USA.

==========================================================================*/

#ifndef __vtkITKImageSequenceReader_h
#define __vtkITKImageSequenceReader_h

#include "vtkMedicalImageReader2.h"

#include "vtkMatrix4x4.h"

#include "vtkITK.h"

class VTK_ITK_EXPORT vtkITKImageSequenceReader : public vtkMedicalImageReader2
{
 public:
  static vtkITKImageSequenceReader *New();
  vtkTypeMacro(vtkITKImageSequenceReader, vtkMedicalImageReader2);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///
  /// Specify file name for the image file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  ///
  /// Current frame index that is extracted from the sequence image to the output port.
  vtkSetMacro(CurrentFrameIndex, unsigned int);
  vtkGetMacro(CurrentFrameIndex, unsigned int);

  /// Get number of frames in recently read image. Set in first Update. Read only.
  vtkSetMacro(NumberOfFrames, unsigned int);
  vtkGetMacro(NumberOfFrames, unsigned int);

  /// Get RAS to IJK matrix. It is null until the first reading is done.
  vtkSetObjectMacro(RasToIjkMatrix, vtkMatrix4x4);
  vtkGetObjectMacro(RasToIjkMatrix, vtkMatrix4x4);

 protected:
  vtkITKImageSequenceReader();
  ~vtkITKImageSequenceReader() override;

  //void ExecuteInformation() override;
  void ExecuteDataWithInformation(vtkDataObject *output, vtkInformation* outInfo) override;

protected:
  /// File name for the image to read.
  char* FileName{nullptr};

  /// Current frame index that is extracted from the sequence image to the output port.
  unsigned int CurrentFrameIndex{0};

  /// Number of frames in recently read image. Set in first Update. Read only.
  unsigned int NumberOfFrames{0};

  /// RAS to IJK matrix
  vtkMatrix4x4* RasToIjkMatrix{nullptr};

private:
  vtkITKImageSequenceReader(const vtkITKImageSequenceReader&) = delete;
  void operator=(const vtkITKImageSequenceReader&) = delete;
};

#endif
