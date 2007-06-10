
// .NAME vtkNRRDWriter - Writes PNG files.
// .SECTION Description
// vtkNRRDWriter writes NRRD files.

// .SECTION See Also
// vtkNRRDReader

#ifndef __vtkNRRDWriter_h
#define __vtkNRRDWriter_h

#include "vtkWriter.h"

#include "vtkMatrix4x4.h"
#include "vtkDoubleArray.h"
#include "teem/nrrd.h"

#include "vtkTeemConfigure.h"

class vtkImageData;

class VTK_TEEM_EXPORT vtkNRRDWriter : public vtkWriter
{
public:

  vtkTypeRevisionMacro(vtkNRRDWriter,vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkNRRDWriter *New();

  // Description:
  // Get the input to this writer.
  vtkImageData* GetInput();
  vtkImageData* GetInput(int port);



  // Description:
  // Specify file name of vtk polygon data file to write.
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
  
  vtkSetClampMacro(FileType,int,VTK_ASCII,VTK_BINARY);
  vtkGetMacro(FileType,int);
  void SetFileTypeToASCII() {this->SetFileType(VTK_ASCII);};
  void SetFileTypeToBinary() {this->SetFileType(VTK_BINARY);};

  vtkBooleanMacro(WriteError, int);
  vtkSetMacro(WriteError, int);
  vtkGetMacro(WriteError, int);
  
protected:
  vtkNRRDWriter();
  ~vtkNRRDWriter();

  virtual int FillInputPortInformation(int port, vtkInformation *info);

  // Description:
  // Write method. It is called by vtkWriter::Write();
  void WriteData();

  // Description:
  // Flag to set to on when a write error occured
  int WriteError;
  
  char *FileName;
  
  vtkDoubleArray *BValues;
  vtkDoubleArray *DiffusionGradients;

  vtkMatrix4x4 *IJKToRASMatrix;
  vtkMatrix4x4 *MeasurementFrameMatrix;

  int UseCompression;
  int FileType;
  
  
private:
  vtkNRRDWriter(const vtkNRRDWriter&);  // Not implemented.
  void operator=(const vtkNRRDWriter&);  // Not implemented.
  //BTX
  void vtkImageDataInfoToNrrdInfo(vtkImageData *in, int &nrrdKind, size_t &numComp, int &vtkType, void **buffer);
  //ETX
  int VTKToNrrdPixelType( const int vtkPixelType );
  int DiffusionWeigthedData;
};

#endif


