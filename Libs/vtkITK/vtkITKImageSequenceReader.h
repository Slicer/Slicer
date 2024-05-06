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
  by the grant GRT-00000485 of Children's Hospital of Philadeplhia, USA.

==========================================================================*/

#ifndef __vtkITKImageSequenceReader_h
#define __vtkITKImageSequenceReader_h

#include "vtkImageAlgorithm.h"

#include "vtkITK.h"

class VTK_ITK_EXPORT vtkITKImageSequenceReader : public vtkImageAlgorithm
{
 public:
  static vtkITKImageSequenceReader *New();
  vtkTypeMacro(vtkITKImageSequenceReader, vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///
  /// Specify file name for the image file.
  void SetFileName(const char *);

  char* GetFileName()
  {
    return FileName;
  }

  ///
  /// The main interface which triggers the reader to start.
  void Read();

 protected:
  vtkITKImageSequenceReader();
  ~vtkITKImageSequenceReader() override;

  //void ExecuteDataWithInformation(vtkDataObject *output, vtkInformation *outInfo) override;
  //static void ReadProgressCallback(itk::Object* obj, const itk::EventObject&, void* data);

protected:
  char* FileName;

private:
  vtkITKImageSequenceReader(const vtkITKImageSequenceReader&) = delete;
  void operator=(const vtkITKImageSequenceReader&) = delete;
};

#endif
