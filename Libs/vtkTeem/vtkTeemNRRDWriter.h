
#ifndef __vtkTeemNRRDWriter_h
#define __vtkTeemNRRDWriter_h

#include "vtkWriter.h"

#include "vtkDoubleArray.h"
#include "vtkMatrix4x4.h"
#include "vtkSmartPointer.h"
#include "teem/nrrd.h"

#include "vtkTeemConfigure.h"

class vtkImageData;
class AttributeMapType;
class AxisInfoMapType;

/// \brief Writes PNG files.
///
/// vtkTeemNRRDWriter writes NRRD files.
///
/// \sa vtkTeemNRRDReader
class VTK_Teem_EXPORT vtkTeemNRRDWriter : public vtkWriter
{
public:

  vtkTypeMacro(vtkTeemNRRDWriter,vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static vtkTeemNRRDWriter *New();

  ///
  /// Get the input to this writer.
  vtkImageData* GetInput();
  vtkImageData* GetInput(int port);

  ///
  /// Specify file name of vtk polygon data file to write.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  vtkSetObjectMacro(DiffusionGradients,vtkDoubleArray);
  vtkGetObjectMacro(DiffusionGradients,vtkDoubleArray);

  vtkSetObjectMacro(BValues,vtkDoubleArray);
  vtkGetObjectMacro(BValues,vtkDoubleArray);

  vtkSetObjectMacro(IJKToRASMatrix,vtkMatrix4x4);
  vtkGetObjectMacro(IJKToRASMatrix,vtkMatrix4x4);

  vtkSetObjectMacro(MeasurementFrameMatrix,vtkMatrix4x4);
  vtkGetObjectMacro(MeasurementFrameMatrix,vtkMatrix4x4);

  vtkSetMacro(UseCompression,int);
  vtkGetMacro(UseCompression,int);
  vtkBooleanMacro(UseCompression,int);

  vtkSetClampMacro(CompressionLevel, int, 0, 9);
  vtkGetMacro(CompressionLevel, int);

  vtkSetClampMacro(FileType,int,VTK_ASCII,VTK_BINARY);
  vtkGetMacro(FileType,int);
  void SetFileTypeToASCII() {this->SetFileType(VTK_ASCII);};
  void SetFileTypeToBinary() {this->SetFileType(VTK_BINARY);};

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
  void vtkSetSpaceToRAS()  { this->SetSpace(nrrdSpaceRightAnteriorSuperior);  };
  void vtkSetSpaceToRAST() { this->SetSpace(nrrdSpaceRightAnteriorSuperiorTime);  };

  /// Set coordinate system to LPS
  void vtkSetSpaceToLPS()  { this->SetSpace(nrrdSpaceLeftPosteriorSuperior); };
  void vtkSetSpaceToLPST() { this->SetSpace(nrrdSpaceLeftPosteriorSuperiorTime); };

  /// Utility function to return image as a Nrrd*
  void* MakeNRRD();

protected:
  vtkTeemNRRDWriter();
  ~vtkTeemNRRDWriter() override;

  int FillInputPortInformation(int port, vtkInformation *info) override;

  ///
  /// Write method. It is called by vtkWriter::Write();
  void WriteData() override;

  ///
  /// Flag to set to on when a write error occurred
  int WriteError;

  char *FileName;

  vtkDoubleArray* BValues;
  vtkDoubleArray* DiffusionGradients;

  vtkMatrix4x4* IJKToRASMatrix;
  vtkMatrix4x4* MeasurementFrameMatrix;

  int UseCompression;
  int CompressionLevel;
  int FileType;

  AttributeMapType *Attributes;
  AxisInfoMapType *AxisLabels;
  AxisInfoMapType *AxisUnits;
  int VectorAxisKind;
  int Space;

private:
  vtkTeemNRRDWriter(const vtkTeemNRRDWriter&) = delete;
  void operator=(const vtkTeemNRRDWriter&) = delete;
  void vtkImageDataInfoToNrrdInfo(vtkImageData *in, int &nrrdKind, size_t &numComp, int &vtkType, void **buffer);
  int VTKToNrrdPixelType( const int vtkPixelType );
  int DiffusionWeightedData;
};

#endif
