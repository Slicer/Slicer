#include <vtkImageRectangularSource.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkImageRectangularSource);

//----------------------------------------------------------------------------
vtkImageRectangularSource::vtkImageRectangularSource()
{
  this->WholeExtent[0] = 0;
  this->WholeExtent[1] = 255;
  this->WholeExtent[2] = 0;
  this->WholeExtent[3] = 255;
  this->WholeExtent[4] = 0;
  this->WholeExtent[5] = 0;
  this->Center[0] = 128;
  this->Center[1] = 128;
  this->Center[2] = 0;
  this->Size[0] = 70;
  this->Size[1] = 70;
  this->Size[2] = 70;
  this->InValue = 255.0;
  this->OutValue = 0.0;
  this->InsideGraySlopeFlag = 0;

  this->Corners = nullptr;

  this->OutputScalarType = VTK_UNSIGNED_CHAR;
}

//----------------------------------------------------------------------------
vtkImageRectangularSource::~vtkImageRectangularSource()
{
  if (this->Corners) {
    for (int i = 0; i < 4 ; i++) delete[] this->Corners[i];
    delete[] this->Corners;
    this->Corners = nullptr;
  }


}

//----------------------------------------------------------------------------
void vtkImageRectangularSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Center: (" << this->Center[0] << ", "
     << this->Center[1] << ", " << this->Center[2] << ")\n";

  os << indent << "Size: (" << this->Size[0] << ", "
     << this->Size[1] << ", " << this->Size[2] << ")\n";

  os << indent << "InValue: " << this->InValue << "\n";
  os << indent << "OutValue: " << this->OutValue << "\n";
  os << indent << "InsideGraySlopeFlag: " << this->InsideGraySlopeFlag << "\n";
  os << indent << "OutputScalarType: " << this->OutputScalarType << "\n";
}
//----------------------------------------------------------------------------
void vtkImageRectangularSource::SetWholeExtent(int extent[6])
{
  int idx;

  for (idx = 0; idx < 6; ++idx)
    {
    if (this->WholeExtent[idx] != extent[idx])
      {
      this->WholeExtent[idx] = extent[idx];
      this->Modified();
      }
    }
}

//----------------------------------------------------------------------------
void vtkImageRectangularSource::SetWholeExtent(int minX, int maxX,
                                            int minY, int maxY,
                                            int minZ, int maxZ)
{
  int extent[6];

  extent[0] = minX;  extent[1] = maxX;
  extent[2] = minY;  extent[3] = maxY;
  extent[4] = minZ;  extent[5] = maxZ;
  this->SetWholeExtent(extent);
}


//----------------------------------------------------------------------------
void vtkImageRectangularSource::GetWholeExtent(int extent[6])
{
  int idx;

  for (idx = 0; idx < 6; ++idx)
    {
    extent[idx] = this->WholeExtent[idx];
    }
}

//----------------------------------------------------------------------------
int vtkImageRectangularSource::RequestInformation(
  vtkInformation * vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  double spacing[3] = {1.0, 1.0, 1.0};
  outInfo->Set(vtkDataObject::SPACING(), spacing, 3);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), this->WholeExtent, 6);
  vtkDataObject::SetPointDataActiveScalarInfo(outInfo, this->OutputScalarType, 1);
  return 1;
}

//----------------------------------------------------------------------------
void vtkImageRectangularSource::SetCorners(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) {
  // Check if it is of dimension 4x2
  // Only set it once

  assert(!this->Corners);
  this->Corners = new int*[4];
  for (int i = 0; i < 4 ; i++) this->Corners[i] = new int[2];
  this->Corners[0][0] = x1;
  this->Corners[0][1] = y1;

  this->Corners[1][0] = x2;
  this->Corners[1][1] = y2;

  this->Corners[2][0] = x3;
  this->Corners[2][1] = y3;

  this->Corners[3][0] = x4;
  this->Corners[3][1] = y4;

}





template <class T>
void vtkImageRectangularSourceExecute(vtkImageRectangularSource *self,
                                    vtkImageData *data, int ext[6], T *ptr)
{
  int idx0, idx1, idx2;
  vtkIdType inc0, inc1, inc2;
  char InFlag[3];
  T outVal, inVal;
  double grad;
  int *center, Min[3], Max[3];
  unsigned long count = 0;
  unsigned long target;
  int InsideGraySlopeFlag;
  int *size;

  outVal = (T)(self->GetOutValue());
  inVal = (T)(self->GetInValue());
  center = self->GetCenter();
  InsideGraySlopeFlag = self->GetInsideGraySlopeFlag();
  size = self->GetSize();
  for (int i = 0; i <3; i++) {
    Min[i] =  center[i] -  size[i]/2;
    if (Min[i] <= ext[2*i]) Min[i] = ext[2*i];
    Max[i] =  center[i] +  size[i]/2 +1;
  }
  data->GetContinuousIncrements(ext, inc0, inc1, inc2);

  target = (unsigned long)((ext[5]-ext[4]+1)*(ext[3]-ext[2]+1)/50.0);
  target++;
  // z direction
  InFlag[2] = 0;
  for (idx2 = ext[4]; idx2 <= ext[5]; ++idx2) {
    if (idx2 == Min[2]) InFlag[2] = 1;
    else if (idx2 == Max[2]) InFlag[2] = 0;
    InFlag[1] = 0;
    for (idx1 = ext[2]; !self->AbortExecute && idx1 <= ext[3]; ++idx1)
      {
      if (!(count%target)) self->UpdateProgress(count/(50.0*target));
      count++;
      if (idx1 == Min[1]) InFlag[1] = InFlag[2];
      else if (idx1 == Max[1]) InFlag[1] = 0;
      InFlag[0] = 0;
      for (idx0 =  ext[0]; idx0 <=  ext[1]; ++idx0) {
        // handle divide by zero
        if (idx0 == Min[0]) InFlag[0] = InFlag[1];
        else if (idx0 == Max[0]) InFlag[0] = 0;
        if (InFlag[0])  {
          if (InsideGraySlopeFlag && size[0]) {
            grad = (2.0*abs(idx0 - center[0])) / double(size[0]);
            *ptr = T((1.0 - grad)*double(inVal)) + T(grad*double(outVal));
          }
          else *ptr = inVal;
        } else *ptr = outVal;
        ++ptr;
        // inc0 is 0
        }
      ptr += inc1;
      }
    ptr += inc2;
    }
}

// Beginining
namespace RectSource
{

// Define in a tirangle the min and max
int Min(int x1, int x2) {return (x1 < x2 ? x1 : x2);}
int Min(int x1, int x2, int x3) {return Min(Min(x1,x2),x3);}
int Min(int x1, int x2, int x3, int x4) {return Min(Min(x1,x2,x3),x4);}

int Max(int x1, int x2) {return (x1 > x2 ? x1 : x2);}
int Max(int x1, int x2, int x3) {return Max(Max(x1,x2),x3); }
int Max(int x1, int x2, int x3, int x4) {return Max(Max(x1,x2,x3),x4); }

void Sort(int x1, int x2, int &xMin, int &xMax) {
  if (x1 < x2) { xMin = x1; xMax = x2;
  } else { xMin = x2;  xMax = x1; }
}

void Sort(int x1, int x2, int x3,int &xMin, int &xMean, int &xMax) {
  Sort(x1,x2,xMin,xMean);
  xMax = xMean;
  int xBlub;
  Sort(x3,xMin,xMin,xBlub);
  Sort(xBlub,xMean,xMean,xBlub);
  Sort(xMax,xBlub,xBlub,xMax);
}

int DefineX(int *c1, int *c2, int y) {
  assert(c1[1] != c2[1]);
  if ((Min(c1[1],c2[1]) > y )|| (Max(c1[1],c2[1]) < y )) return -1;

  double lenY = c2[1] - c1[1];
  int lenX    = c2[0] - c1[0];
  double Scale =  double(lenX)/double(lenY);

  return int(c1[0] + double(y - c1[1])*Scale);
}

template <class T> T CalculateGraySlope(int length, double center, int pos, T inVal, T outVal) {
  double grad = (2.0*fabs(float(pos - center))) / double(length);
  return  T((1.0 - grad)*double(inVal)) + T(grad*double(outVal));
}

template <class T>
void DefineLine(int MinInX, int MaxInX,int LineLength, T inVal, T outVal, int InsideGraySlopeFlag,  T* Result) {
  if (MinInX >= LineLength || MaxInX < 0 || MinInX > MaxInX) MinInX = LineLength;
  for (int x = 0; x < MinInX; x ++) *Result ++ = outVal;
  // We already went through entire image
  if (MinInX == LineLength) return;


  if (MinInX < 0) MinInX  = 0;
  if (MaxInX >= LineLength) MaxInX = LineLength -1;

  int inLength = MaxInX - MinInX + 1;
  double inCenter = double(MinInX) + double(inLength)/2.0;


  // Otherwise everything is just black
  if (inLength < 2)  InsideGraySlopeFlag = 0;

  for (int x = MinInX; x <= MaxInX; x ++) {
    if (InsideGraySlopeFlag) {
      *Result ++ = CalculateGraySlope(inLength, inCenter, x, inVal, outVal);
    } else *Result ++ = inVal;
  }

  for (int x = MaxInX + 1; x <  LineLength; x ++) *Result ++ = outVal;

}


  void DefineXMinMaxInTriangleNormal(int *c1, int *c2, int *c3, int y, int &MinX, int &MaxX) {
    // Otherwise does not define a triangle;
    assert((c1[1] != c2[1]) || (c2[1] != c3[1]));
    // It is not part of the triangle
    if ((Min(c1[1],c2[1],c3[1]) > y ) || (Max(c1[1],c2[1],c3[1]) < y)) {
      MaxX = -1;
      MinX = 0;
      return;
    }
    int x1 , x2 , x3;
    // Check if it is on the line
    if (c1[1] == c2[1]) {
      if (c1[1] == y) {
        MaxX = Max(c1[0],c2[0]);
        MinX = Min(c1[0],c2[0]);
        return;
      } else x1 = -1;
    } else x1 = DefineX(c1, c2, y);

    if (c2[1] == c3[1]) {
      if (c2[1] == y) {
        MaxX = Max(c2[0],c3[0]);
        MinX = Min(c2[0],c3[0]);
        return;
      } else x2 = -1;
    } else x2 = DefineX(c2, c3, y);

    if (c1[1] == c3[1]) {
      if (c1[1] == y) {
        MaxX = Max(c1[0],c3[0]);
        MinX = Min(c1[0],c3[0]);
        return;
      } else x3 = -1;
    } else x3 = DefineX(c1, c3, y);
    // One of them has to be -1
    int xBlub;

    Sort(x1,x2,x3,xBlub,MinX,MaxX);

    // Special Case when all three lines have input
    if ((MinX == MaxX) && (xBlub > -1)) MinX = xBlub;
  }

void DefineXMinMaxInTriangle(int *c1, int *c2, int *c3, int y, int &MinX, int &MaxX) {
  DefineXMinMaxInTriangleNormal(c1, c2, c3, y, MinX, MaxX);
  if (MinX != MaxX) return;

  // When we are at a corners make sure that the piece is connected
  int MinX1, MaxX1;
  DefineXMinMaxInTriangleNormal(c1, c2, c3, y +1, MinX1, MaxX1);
  if (MaxX1 < 0) DefineXMinMaxInTriangleNormal(c1, c2, c3, y -1, MinX1, MaxX1);
  if (MaxX1 < 0) return;
  if (!(MaxX1 - MinX1)) return;

  if (MinX < MinX1) {
    MaxX = MinX1  + 1;
    return;
  }

  if (MaxX > MaxX1) MinX = MaxX1 - 1;
  return;
}

template <class T>
void DefineSlice(int **Corners, int RowLength, int LineLength, T inVal, T outVal, int InsideGraySlopeFlag, T* Result, int IncResultY) {
  int  MinInY = Min(Corners[0][1],Corners[1][1], Corners[2][1], Corners[3][1]);
  int  MaxInY = Max(Corners[0][1],Corners[1][1], Corners[2][1], Corners[3][1]);

  if (MinInY >= RowLength || MaxInY < 0 || MinInY > MaxInY) MinInY = RowLength;
  // Outside
  for (int y = 0; y < MinInY; y ++) {
    DefineLine(LineLength, LineLength, LineLength, inVal, outVal, InsideGraySlopeFlag, Result);
    Result += RowLength + IncResultY;
  }
  if (MinInY == RowLength) return;


  if (MinInY < 0) MinInY = 0;
  if (MaxInY >= RowLength) MaxInY = RowLength -1;

  int inMinX1, inMinX2, inMaxX1, inMaxX2;

  // Area of interest
  for (int y = MinInY; y <= MaxInY; y ++) {
    // Split Rectangual in two triangles
    DefineXMinMaxInTriangle(Corners[0], Corners[1], Corners[3], y, inMinX1, inMaxX1);
    DefineXMinMaxInTriangle(Corners[1], Corners[2], Corners[3], y, inMinX2, inMaxX2);
    if (inMaxX1 < 0) DefineLine(inMinX2, inMaxX2,LineLength, inVal, outVal, InsideGraySlopeFlag,Result);
    else if (inMaxX2 < 0) DefineLine(inMinX1, inMaxX1,LineLength, inVal, outVal, InsideGraySlopeFlag,Result);
    else {

      if ((inMinX1 <= inMinX2) && (inMaxX2 <= inMaxX1)) {
        DefineLine(inMinX1, inMinX2-1, LineLength, inVal, outVal, InsideGraySlopeFlag,Result);
        int inMaxX3 = inMaxX1 - inMaxX2 -1 ;
        if (inMaxX3 > -1) DefineLine(0, inMaxX3, LineLength, inVal, outVal, InsideGraySlopeFlag,Result + (inMaxX2  + 1));
      } else if ((inMinX2 <= inMinX1) && (inMaxX1 <= inMaxX2)) {
        DefineLine(inMinX2, inMinX1 - 1, LineLength, inVal, outVal, InsideGraySlopeFlag,Result);
        int inMaxX3 = inMaxX2 - inMaxX1 -1 ;
        if (inMaxX3 > -1) DefineLine(0, inMaxX3, LineLength, inVal, outVal, InsideGraySlopeFlag,Result + (inMaxX1  + 1));
      } else {
        DefineLine(Min(inMinX1,inMinX2), Max(inMaxX1,inMaxX2), LineLength, inVal, outVal, InsideGraySlopeFlag,Result);
      }

    }
    Result += RowLength + IncResultY;
  }

  // Outside
  for (int y =  MaxInY +1; y <  RowLength; y ++) {
    DefineLine(LineLength, LineLength, LineLength, inVal, outVal, InsideGraySlopeFlag, Result);
    Result += RowLength + IncResultY;
  }
}

}

template <class T>
void vtkImageRectangularSource_GeneralExecute(vtkImageRectangularSource *self, vtkImageData *data, int ext[6], int** Corners, T *ptr)
{
  // Currently only defined in 2D
  assert(!(ext[5] - ext[4]));

  vtkIdType inc0, inc1, inc2;

  data->GetContinuousIncrements(ext, inc0, inc1, inc2);
  RectSource::DefineSlice(Corners, ext[3]-ext[2]+1, ext[1]-ext[0]+1, (T)(self->GetInValue()), (T)(self->GetOutValue()), self->GetInsideGraySlopeFlag(), ptr, inc1);
}


//----------------------------------------------------------------------------
void vtkImageRectangularSource::ExecuteDataWithInformation(vtkDataObject *output, vtkInformation* outInfo)
{
  int *extent;
  void *ptr;

  vtkImageData *data = this->AllocateOutputData(output, outInfo);
  extent = this->GetUpdateExtent();
  ptr = data->GetScalarPointerForExtent(extent);
  
  if (this->Corners ) {
    switch (data->GetScalarType()) {
      vtkTemplateMacro(vtkImageRectangularSource_GeneralExecute(this, data, extent, this->Corners, (VTK_TT *)ptr));
    default:
      vtkErrorMacro("Execute: Unknown output ScalarType");
    }
  } else {
    switch (data->GetScalarType()) {
      vtkTemplateMacro(vtkImageRectangularSourceExecute(this, data, extent, static_cast<VTK_TT*>(ptr)));
    default:
      vtkErrorMacro("Execute: Unknown output ScalarType");
    }
  }
}







