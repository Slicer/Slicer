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

#ifndef __vtkITKImageSequenceWriter_h
#define __vtkITKImageSequenceWriter_h

#include "vtkImageAlgorithm.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkMatrix4x4.h"

#include "vtkITK.h"
#include "itkImageIOBase.h"

class vtkStringArray;

class VTK_ITK_EXPORT vtkITKImageSequenceWriter : public vtkImageAlgorithm
{
public:
  static vtkITKImageSequenceWriter *New();
  vtkTypeMacro(vtkITKImageSequenceWriter, vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  enum
    {
    VoxelVectorTypeUndefined,
    VoxelVectorTypeSpatial,
    VoxelVectorTypeColorRGB,
    VoxelVectorTypeColorRGBA,
    VoxelVectorType_Last // must be last
    };

  ///
  /// Specify file name for the image file. You should specify either
  /// a FileName or a FilePrefix. Use FilePrefix if the data is stored
  /// in multiple files.
  void SetFileName(const char *);

  char* GetFileName()
  {
    return FileName;
  }

  ///
  /// Use compression if possible
  vtkGetMacro(UseCompression, int);
  vtkSetMacro(UseCompression, int);
  vtkBooleanMacro(UseCompression, int);

  ///
  /// Set/Get the ImageIO class name.
  vtkGetStringMacro(ImageIOClassName);
  vtkSetStringMacro(ImageIOClassName);

  ///
  /// The main interface which triggers the writer to start.
  void Write();

  /// Set orientation matrix
  void SetRasToIJKMatrix(vtkMatrix4x4* mat)
  {
    RasToIJKMatrix = mat;
  }

  /// Set orientation matrix
  void SetMeasurementFrameMatrix(vtkMatrix4x4* mat)
  {
    MeasurementFrameMatrix = mat;
  }

  /// Defines how to interpret voxel components
  vtkSetMacro(VoxelVectorType, int);
  vtkGetMacro(VoxelVectorType, int);

protected:
  vtkITKImageSequenceWriter();
  ~vtkITKImageSequenceWriter() override;

  /**
   * Fill the input port information objects for this algorithm.  This
   * is invoked by the first call to GetInputPortInformation for each
   * port so subclasses can specify what they can handle.
   */
  int FillInputPortInformation(int port, vtkInformation* info) override;

protected:
  char* FileName;
  vtkMatrix4x4* RasToIJKMatrix;
  vtkMatrix4x4* MeasurementFrameMatrix;
  int UseCompression;
  char* ImageIOClassName;
  int VoxelVectorType;

private:
  vtkITKImageSequenceWriter(const vtkITKImageSequenceWriter&) = delete;
  void operator=(const vtkITKImageSequenceWriter&) = delete;
};

#endif
