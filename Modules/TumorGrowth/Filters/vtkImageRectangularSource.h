// .NAME vtkImageRectangularSource - Create a binary image of an rectangular.
// .SECTION Description
// vtkImageRectangularSource creates a binary image of a ellipsoid.  It was created
// as an example of a simple source, and to test the mask filter.
// It is also used internally in vtkImageDilateErode3D.

#ifndef __vtkImageRectangularSource_h
#define __vtkImageRectangularSource_h

#include "vtkImageSource.h"
#include "vtkTumorGrowth.h"

class  VTK_TUMORGROWTH_EXPORT vtkImageRectangularSource : public vtkImageSource
{
public:
  static vtkImageRectangularSource *New();
  vtkTypeRevisionMacro(vtkImageRectangularSource,vtkImageSource);
  void PrintSelf(ostream& os, vtkIndent indent);   
  
  // Description:
  // Set/Get the extent of the whole output image.
  void SetWholeExtent(int extent[6]);
  void SetWholeExtent(int minX, int maxX, int minY, int maxY, 
                            int minZ, int maxZ);
  void GetWholeExtent(int extent[6]);
  int *GetWholeExtent() {return this->WholeExtent;}
  
  // Description:
  // Set/Get the center of the ellipsoid.
  vtkSetVector3Macro(Center, int);
  vtkGetVector3Macro(Center, int);
  
  // Description:
  // Set/Get the radius of the ellipsoid.
  vtkSetVector3Macro(Size, int);
  vtkGetVector3Macro(Size, int);

  // Description:
  // Set/Get the inside pixel values.
  vtkSetMacro(InValue,float);
  vtkGetMacro(InValue,float);

  // Description:
  // Set/Get the outside pixel values.
  vtkSetMacro(OutValue,float);
  vtkGetMacro(OutValue,float);

  // Description:
  // If the flag is set the inside of the rectangular slowly adjust its intensity from outside to inside
  vtkSetMacro(InsideGraySlopeFlag,int);
  vtkGetMacro(InsideGraySlopeFlag,int);

  // Description:
  // Set what type of scalar data this source should generate.
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

  // If you do no want to define a rectangular with edges of 90 degree   
  void SetCorners(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);

protected:
  vtkImageRectangularSource();
  ~vtkImageRectangularSource();

  int **Corners;

  int WholeExtent[6];
  int Center[3];
  int Size[3];
  float InValue;
  float OutValue;
  int OutputScalarType;
  int InsideGraySlopeFlag;
  
  virtual void ExecuteInformation();
  virtual void ExecuteData(vtkDataObject *outData);
private:
  vtkImageRectangularSource(const vtkImageRectangularSource&);  // Not implemented.
  void operator=(const vtkImageRectangularSource&);  // Not implemented.
};


#endif


