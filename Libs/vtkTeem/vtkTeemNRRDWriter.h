
#ifndef __vtkTeemNRRDWriter_h
#define __vtkTeemNRRDWriter_h

#include "vtkWriter.h"

#include "vtkDoubleArray.h"
#include "vtkMatrix4x4.h"
#include "vtkSmartPointer.h"

#include "vtkTeemConfigure.h"

class vtkImageData;
class AttributeMapType;
class AxisInfoMapType;

/// \brief Writes NRRD files using the teem library.
///
/// This class was originally developed for writing DWI images and it is still the recommended writer for this purpose.
/// The class was later extended to allow writing vector images, but it has some limitations:
/// It can only write 3D images with voxel component axis kind scalar, RGB, RGBA, list, or a manually specified kind.
///
/// vtkITKImageWriter is a more general-purpose writer - it can be used for writing 3D image data with voxels containing
/// scalars, RGB, RGBA, spatial vectors (displacement, speed, etc.), or generic list components.
/// vtkITKImageSequenceWriter can be used for writing time sequence data (e.g., time sequence of displacement fields, RGB volumes, etc.).
///
/// \sa vtkTeemNRRDReader vtkITKImageWriter vtkITKImageSequenceWriter
class VTK_Teem_EXPORT vtkTeemNRRDWriter : public vtkWriter
{
public:
  vtkTypeMacro(vtkTeemNRRDWriter, vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static vtkTeemNRRDWriter* New();

  ///
  /// Get the input to this writer.
  vtkImageData* GetInput();
  vtkImageData* GetInput(int port);

  ///
  /// Specify file name of vtk polygon data file to write.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  vtkSetObjectMacro(DiffusionGradients, vtkDoubleArray);
  vtkGetObjectMacro(DiffusionGradients, vtkDoubleArray);

  vtkSetObjectMacro(BValues, vtkDoubleArray);
  vtkGetObjectMacro(BValues, vtkDoubleArray);

  vtkSetObjectMacro(IJKToRASMatrix, vtkMatrix4x4);
  vtkGetObjectMacro(IJKToRASMatrix, vtkMatrix4x4);

  vtkSetObjectMacro(MeasurementFrameMatrix, vtkMatrix4x4);
  vtkGetObjectMacro(MeasurementFrameMatrix, vtkMatrix4x4);

  vtkSetMacro(UseCompression, int);
  vtkGetMacro(UseCompression, int);
  vtkBooleanMacro(UseCompression, int);

  vtkSetClampMacro(CompressionLevel, int, 0, 9);
  vtkGetMacro(CompressionLevel, int);

  vtkSetClampMacro(FileType, int, VTK_ASCII, VTK_BINARY);
  vtkGetMacro(FileType, int);
  void SetFileTypeToASCII() { this->SetFileType(VTK_ASCII); };
  void SetFileTypeToBinary() { this->SetFileType(VTK_BINARY); };

  /// Flag that is set by WriteData() if writing fails.
  vtkBooleanMacro(WriteError, int);
  vtkSetMacro(WriteError, int);
  vtkGetMacro(WriteError, int);

  /// Method to set an attribute that will be passed into the NRRD
  /// file on write
  void SetAttribute(const std::string& name, const std::string& value);

  /// Method to set label for each axis
  void SetAxisLabel(unsigned int axis, const char* label);

  /// Method to set unit for each axis
  void SetAxisUnit(unsigned int axis, const char* unit);

  /// Method to force axis kind for the non-spatial axis (4th dimension).
  /// If not set (or set to nrrdKindUnknown) then axis kind is guessed
  /// from the number of components and scalar type.
  void SetVectorAxisKind(int kind);

  /// Method to set the coordinate system written to the NRRD file.
  /// Currently the only valid coordinate systems are: RAS, RAST, LPS, and LPST.
  vtkSetMacro(Space, int);
  vtkGetMacro(Space, int);

  /// Set coordinate system to RAS
  void vtkSetSpaceToRAS();
  void vtkSetSpaceToRAST();

  /// Set coordinate system to LPS
  void vtkSetSpaceToLPS();
  void vtkSetSpaceToLPST();

  /// Force the addition of a range axis, even when the size of the first image dimension (components, or frame list) is 1.
  /// This is useful when attempting to write an image sequence with a single frame, as otherwise the range dimension would be omitted.
  vtkSetMacro(ForceRangeAxis, bool);
  vtkGetMacro(ForceRangeAxis, bool);
  vtkBooleanMacro(ForceRangeAxis, bool);

  /// Utility function to return image as a Nrrd*
  void* MakeNRRD();

protected:
  vtkTeemNRRDWriter();
  ~vtkTeemNRRDWriter() override;

  int FillInputPortInformation(int port, vtkInformation* info) override;

  ///
  /// Write method. It is called by vtkWriter::Write();
  void WriteData() override;

  ///
  /// Flag to set to on when a write error occurred
  int WriteError;

  char* FileName;

  vtkDoubleArray* BValues;
  vtkDoubleArray* DiffusionGradients;

  vtkMatrix4x4* IJKToRASMatrix;
  vtkMatrix4x4* MeasurementFrameMatrix;

  int UseCompression;
  int CompressionLevel;
  int FileType;

  AttributeMapType* Attributes;
  AxisInfoMapType* AxisLabels;
  AxisInfoMapType* AxisUnits;
  int VectorAxisKind;
  int Space;

  bool ForceRangeAxis;

private:
  vtkTeemNRRDWriter(const vtkTeemNRRDWriter&) = delete;
  void operator=(const vtkTeemNRRDWriter&) = delete;
  void vtkImageDataInfoToNrrdInfo(vtkImageData* in, int& nrrdKind, size_t& numComp, int& vtkType, void** buffer);
  int VTKToNrrdPixelType(const int vtkPixelType);
  int DiffusionWeightedData;
};

#endif
