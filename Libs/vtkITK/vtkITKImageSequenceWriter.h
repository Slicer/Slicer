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

#ifndef __vtkITKImageSequenceWriter_h
#define __vtkITKImageSequenceWriter_h

#include "vtkImageAlgorithm.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkMatrix4x4.h"

#include "vtkITK.h"
#include "vtkITKImageWriter.h"
#include "itkImageIOBase.h"

class vtkStringArray;
class AttributeMapType;
class AxisInfoMapType;

class VTK_ITK_EXPORT vtkITKImageSequenceWriter : public vtkImageAlgorithm
{
public:
  static vtkITKImageSequenceWriter* New();
  vtkTypeMacro(vtkITKImageSequenceWriter, vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///
  /// Use compression if possible
  vtkGetMacro(UseCompression, int);
  vtkSetMacro(UseCompression, int);
  vtkBooleanMacro(UseCompression, int);

  ///
  /// Set/Get the file name.
  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

  ///
  /// Set/Get the ImageIO class name.
  vtkGetStringMacro(ImageIOClassName);
  vtkSetStringMacro(ImageIOClassName);

  /// The main interface which triggers the writer to start.
  void Write();

  /// Set orientation matrix
  void SetRasToIJKMatrix(vtkMatrix4x4* mat) { RasToIJKMatrix = mat; }

  /// Defines how to interpret voxel components
  /// Enumerated values are specified in vtkITKImageWriter.
  vtkSetMacro(VoxelVectorType, int);
  vtkGetMacro(VoxelVectorType, int);

  /// Intent code identifying the type of data in the file
  vtkSetStringMacro(IntentCode);
  vtkGetStringMacro(IntentCode);

  /// Spacing along the sequence axis (4th image axis) that was used in the most recent Write().
  /// For NIfTI file format it is computed from the index values and unit of axis 3 (stored in pixdim[4]),
  /// for other file formats it is 1.0.
  vtkGetMacro(SequenceAxisSpacing, double);

  /// Origin of the sequence axis (4th image axis) that was used in the most recent Write().
  /// For NIfTI file format it is computed from the index values and unit of axis 3 (stored in toffset),
  /// for other file formats it is 0.0.
  vtkGetMacro(SequenceAxisOrigin, double);

  /// Method to set an attribute that will be passed into the NRRD file on write
  void SetAttribute(const std::string& name, const std::string& value);
  /// Get the attributes map
  AttributeMapType* GetAttributes() const { return this->Attributes; }

  /// Method to set label for each axis
  void SetAxisLabel(unsigned int axis, const char* label);
  /// Get the axis labels map
  AxisInfoMapType* GetAxisLabels() const { return this->AxisLabels; }

  /// Method to set unit for each axis
  void SetAxisUnit(unsigned int axis, const char* unit);
  /// Get the axis units map
  AxisInfoMapType* GetAxisUnits() const { return this->AxisUnits; }

protected:
  vtkITKImageSequenceWriter();
  ~vtkITKImageSequenceWriter() override;

  /// Fill the input port information objects for this algorithm.  This
  /// is invoked by the first call to GetInputPortInformation for each
  /// port so subclasses can specify what they can handle.
  int FillInputPortInformation(int port, vtkInformation* info) override;

  /// Returns true if the image is written in NIfTI file format.
  bool IsNiftiFile();

  /// Compute sequence axis spacing and origin from the index values and unit of axis 3
  /// and report sequence properties that cannot be stored in NIfTI file format.
  void UpdateNiftiSequenceAxis(int numberOfFrames);

protected:
  char* FileName{ nullptr };
  vtkMatrix4x4* RasToIJKMatrix{ nullptr };
  int UseCompression{ 0 };
  char* ImageIOClassName{ nullptr };
  int VoxelVectorType{ vtkITKImageWriter::VoxelVectorTypeUndefined };
  char* IntentCode{ nullptr };
  double SequenceAxisSpacing{ 1.0 };
  double SequenceAxisOrigin{ 0.0 };

  AttributeMapType* Attributes;
  AxisInfoMapType* AxisLabels;
  AxisInfoMapType* AxisUnits;

private:
  vtkITKImageSequenceWriter(const vtkITKImageSequenceWriter&) = delete;
  void operator=(const vtkITKImageSequenceWriter&) = delete;
};

#endif
