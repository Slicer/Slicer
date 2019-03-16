///  vtkImageRectangularSource - Create a binary image of an rectangular.
///
/// vtkImageRectangularSource creates a binary image of a ellipsoid.  It was created
/// as an example of a simple source, and to test the mask filter.
/// It is also used internally in vtkImageDilateErode3D.

#ifndef __vtkImageRectangularSource_h
#define __vtkImageRectangularSource_h

#include "vtkImageAlgorithm.h"
#include "vtkSlicerBaseLogic.h"
#include <vtkVersion.h>

class  VTK_SLICER_BASE_LOGIC_EXPORT vtkImageRectangularSource : public vtkImageAlgorithm
{
public:
  static vtkImageRectangularSource *New();
  vtkTypeMacro(vtkImageRectangularSource,vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///
  /// Set/Get the extent of the whole output image.
  void SetWholeExtent(int extent[6]);
  void SetWholeExtent(int minX, int maxX, int minY, int maxY,
                            int minZ, int maxZ);
  void GetWholeExtent(int extent[6]);
  int *GetWholeExtent() {return this->WholeExtent;}

  ///
  /// Set/Get the center of the ellipsoid.
  vtkSetVector3Macro(Center, int);
  vtkGetVector3Macro(Center, int);

  ///
  /// Set/Get the radius of the ellipsoid.
  vtkSetVector3Macro(Size, int);
  vtkGetVector3Macro(Size, int);

  ///
  /// Set/Get the inside pixel values.
  vtkSetMacro(InValue,float);
  vtkGetMacro(InValue,float);

  ///
  /// Set/Get the outside pixel values.
  vtkSetMacro(OutValue,float);
  vtkGetMacro(OutValue,float);

  ///
  /// If the flag is set the inside of the rectangular slowly adjust its intensity from outside to inside
  vtkSetMacro(InsideGraySlopeFlag,int);
  vtkGetMacro(InsideGraySlopeFlag,int);

  ///
  /// Set what type of scalar data this source should generate.
  vtkSetMacro(OutputScalarType,int);
  vtkGetMacro(OutputScalarType,int);
  void SetOutputScalarTypeToDouble()
    {this->SetOutputScalarType(VTK_DOUBLE);}
  void SetOutputScalarTypeToFloat()
    {this->SetOutputScalarType(VTK_FLOAT);}
  void SetOutputScalarTypeToLong()
    {this->SetOutputScalarType(VTK_LONG);}
  void SetOutputScalarTypeToUnsignedLong()
    {this->SetOutputScalarType(VTK_UNSIGNED_LONG);};
  void SetOutputScalarTypeToInt()
    {this->SetOutputScalarType(VTK_INT);}
  void SetOutputScalarTypeToUnsignedInt()
    {this->SetOutputScalarType(VTK_UNSIGNED_INT);}
  void SetOutputScalarTypeToShort()
    {this->SetOutputScalarType(VTK_SHORT);}
  void SetOutputScalarTypeToUnsignedShort()
    {this->SetOutputScalarType(VTK_UNSIGNED_SHORT);}
  void SetOutputScalarTypeToChar()
    {this->SetOutputScalarType(VTK_CHAR);}
  void SetOutputScalarTypeToUnsignedChar()
    {this->SetOutputScalarType(VTK_UNSIGNED_CHAR);}

  /// If you do no want to define a rectangular with edges of 90 degree
  void SetCorners(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);

protected:
  vtkImageRectangularSource();
  ~vtkImageRectangularSource() override;

  int **Corners;

  int WholeExtent[6];
  int Center[3];
  int Size[3];
  float InValue;
  float OutValue;
  int OutputScalarType;
  int InsideGraySlopeFlag;

  int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
  void ExecuteDataWithInformation(vtkDataObject *output, vtkInformation* outInfo) override;
private:
  vtkImageRectangularSource(const vtkImageRectangularSource&) = delete;
  void operator=(const vtkImageRectangularSource&) = delete;
};


#endif


