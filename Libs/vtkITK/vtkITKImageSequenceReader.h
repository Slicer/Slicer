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
#include "vtkITKImageWriter.h"

class VTK_ITK_EXPORT vtkITKImageSequenceReader : public vtkMedicalImageReader2
{
public:
  static vtkITKImageSequenceReader* New();
  vtkTypeMacro(vtkITKImageSequenceReader, vtkMedicalImageReader2);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Specify file name for the image file.
  vtkSetStringMacro(FileName);
  /// Get file name for the image file.
  vtkGetStringMacro(FileName);

  /// Set current frame index that is extracted from the sequence image to the output port.
  vtkSetMacro(CurrentFrameIndex, unsigned int);
  /// Get current frame index that is extracted from the sequence image to the output port.
  vtkGetMacro(CurrentFrameIndex, unsigned int);

  /// Set number of frames in recently read image. Set in first Update. Read only.
  vtkSetMacro(NumberOfFrames, unsigned int);
  /// Get number of frames in recently read image. Set in first Update. Read only.
  vtkGetMacro(NumberOfFrames, unsigned int);

  /// Get RAS to IJK matrix. It is null until the first reading is done.
  vtkSetObjectMacro(RasToIjkMatrix, vtkMatrix4x4);
  vtkGetObjectMacro(RasToIjkMatrix, vtkMatrix4x4);

  /// Defines how to interpret voxel components
  vtkSetMacro(VoxelVectorType, int);
  vtkGetMacro(VoxelVectorType, int);

  /// Get the list of keys in the header.
  const std::vector<std::string> GetHeaderKeysVector();
  /// Get the map of keys in the header.
  const std::map<std::string, std::string> GetHeaderKeysMap();
  /// Get a value given a key in the header
  const char* GetHeaderValue(const char* key);

  /// Get label for specified axis
  const char* GetAxisLabel(unsigned int axis);

  /// Get unit for specified axis
  const char* GetAxisUnit(unsigned int axis);

  vtkGetMacro(SequenceAxisLabel, std::string);
  vtkGetMacro(SequenceAxisUnit, std::string);

  unsigned int GetNumberOfCachedImages();
  vtkImageData* GetCachedImage(unsigned int index);
  void ClearCachedImages();

protected:
  vtkITKImageSequenceReader();
  ~vtkITKImageSequenceReader() override;

  // void ExecuteInformation() override;
  void ExecuteDataWithInformation(vtkDataObject* output, vtkInformation* outInfo) override;

protected:
  /// File name for the image to read.
  char* FileName{ nullptr };

  /// Current frame index that is extracted from the sequence image to the output port.
  unsigned int CurrentFrameIndex{ 0 };

  /// Number of frames in recently read image. Set in first Update. Read only.
  unsigned int NumberOfFrames{ 0 };

  /// RAS to IJK matrix
  vtkMatrix4x4* RasToIjkMatrix{ nullptr };

  int VoxelVectorType{ vtkITKImageWriter::VoxelVectorTypeUndefined };

  /// Key/value pairs read from the header.
  std::map<std::string, std::string> HeaderKeyValueMap;

  // Axis labels read from the header.
  std::map<unsigned int, std::string> AxisLabels;
  // Axis units read from the header.
  std::map<unsigned int, std::string> AxisUnits;
  // Sequence axis label read from the header.
  std::string SequenceAxisLabel;
  // Sequence axis unit read from the header.
  std::string SequenceAxisUnit;

  std::vector<vtkSmartPointer<vtkImageData>> CachedImages;

private:
  vtkITKImageSequenceReader(const vtkITKImageSequenceReader&) = delete;
  void operator=(const vtkITKImageSequenceReader&) = delete;
};

#endif
