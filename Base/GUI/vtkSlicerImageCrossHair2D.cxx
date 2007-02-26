
#include "vtkSlicerImageCrossHair2D.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLargeInteger.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkPointData.h"

#define SET_PIXEL(x, y, color) { ptr = &outPtr[(y)*nxnc + (x)*nc]; \
    ptr[0] = color[0]; ptr[1] = color[1]; ptr[2] = color[2];}


//------------------------------------------------------------------------------

vtkSlicerImageCrossHair2D* vtkSlicerImageCrossHair2D::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkSlicerImageCrossHair2D");
  if(ret)
    {
    return (vtkSlicerImageCrossHair2D*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkSlicerImageCrossHair2D;
}

//----------------------------------------------------------------------------
// Description:
// Constructor sets default values
vtkSlicerImageCrossHair2D::vtkSlicerImageCrossHair2D()
{
  this->ShowCursorOn();
  this->IntersectCrossOff();
  this->BullsEyeOff();

  this->Cursor[0] = 0;
  this->Cursor[1] = 0;

  this->CursorColor[0] = 1;
  this->CursorColor[1] = 1;
  this->CursorColor[2] = 0;

  this->NumHashes = 5;
  this->Magnification = 1.0;
  this->HashLength = 5.0;
  this->HashGap = 5.0;
  this->BullsEyeWidth = 10;
}

//----------------------------------------------------------------------------

static void ConvertColor(vtkFloatingPointType *f, unsigned char *c)
{
  c[0] = (int)(f[0] * 255.0);
  c[1] = (int)(f[1] * 255.0);
  c[2] = (int)(f[2] * 255.0);
}

//----------------------------------------------------------------------------

// Draw line including first, AND second end point
static void DrawLine(int x1, int y1, int x2, int y2, unsigned char color[3],
  unsigned char *outPtr, int nxnc, int nc)
{
  unsigned char *ptr;
  long dx = abs(x2 - x1);
  long dy = abs(y2 - y1);
  long dy2, dx2, p, dydx2;
  long x, y, xEnd, yInc, yEnd, xInc;

  // Horizontal and vertical lines don't need to be special cases,
  // but it will make them faster.

  // Horizontal
  if (dy == 0)
    {
    y = y1;
    if (x1 < x2) {
      xEnd = x2;
      for (x=x1; x <= xEnd; x++)
        SET_PIXEL(x, y, color);
    } else {
      xEnd = x1;
      for (x=x2; x <= xEnd; x++)
        SET_PIXEL(x, y, color);
    }
    }
  // Vertical
  else if (dx == 0)
    {
    x = x1;
    if (y1 < y2) {
      yEnd = y2;
      for (y=y1; y <= yEnd; y++)
        SET_PIXEL(x, y, color);
    } else {
      yEnd = y1;
      for (y=y2; y <= yEnd; y++)
        SET_PIXEL(x, y, color);
    }
    }
  // < 45 degree slope
  else if (dy <= dx)
    {
    dydx2 = (dy-dx) << 1;
    dy2 = dy << 1;
    p = dy2 - dx;

    if (x1 < x2) {
      x = x1;
      y = y1;
      xEnd = x2;
      if (y1 < y2)
        yInc = 1;
      else
        yInc = -1;
    } else {
      x = x2;
      y = y2;
      xEnd = x1;
      if (y2 < y1)
        yInc = 1;
      else
        yInc = -1;
    }

    // Draw first point
    SET_PIXEL(x, y, color);

    // Draw up to end point
    while (x < xEnd)
      {
      x++;
      if (p <= 0)
        p += dy2;
      else {
        y += yInc;
        p += dydx2;
      }
      SET_PIXEL(x, y, color);
      }
    // Draw end point
    SET_PIXEL(x, y, color);
    }
  // > 45 degree slope
  else
    {
    dx2 = dx << 1;
    p = dx2 - dy;
    dydx2 = (dx-dy) << 1;

    if (y1 < y2) {
      x = x1;
      y = y1;
      yEnd = y2;
      if (x1 < x2)
        xInc = 1;
      else
        xInc = -1;
    } else {
      x = x2;
      y = y2;
      yEnd = y1;
      if (x2 < x1)
        xInc = 1;
      else
        xInc = -1;
    }

    // Draw first point
    SET_PIXEL(x, y, color);

    // Draw up to end point
    while (y < yEnd)
      {
      y++;
      if (p <= 0)
        p += dx2;
      else {
        x += xInc;
        p += dydx2;
      }
      SET_PIXEL(x, y, color);
      }
    // Draw end point
    SET_PIXEL(x, y, color);
    }
}

//----------------------------------------------------------------------------

void vtkSlicerImageCrossHair2D::DrawCursor(vtkImageData *outData, int outExt[6])
{
  unsigned char color[3];
  unsigned char *outPtr = (unsigned char *)
    outData->GetScalarPointerForExtent(outExt);
  int x, y, nx, ny, nx1, ny1, nxnc, nc, numHash, h, d, eye, e;
  int xMin, xMax, yMin, yMax;
  int gapx, gapy,lenx,leny,lenx2,leny2;
  vtkFloatingPointType mmLen,  mmGap, mag;
  vtkFloatingPointType mmSpacing[3];
    
  // Get Extent and Number of components(nc)
  xMin = outExt[0];
  xMax = outExt[1];
  yMin = outExt[2];
  yMax = outExt[3];
  nx = outExt[1] - outExt[0] + 1;
  ny = outExt[3] - outExt[2] + 1;
  nc = outData->GetNumberOfScalarComponents();
  nxnc = nx*nc;
  nx1 = nx-1;
  ny1 = ny-1;

  // Hash Mark number and length.
  numHash = this->NumHashes;
  mag = this->Magnification;
  mmLen = this->HashLength;
  mmGap = this->HashGap;

  // Convert Color from rgb in the rang 0..1 to 0..255
  ConvertColor(this->CursorColor, color);

  // Cursor Position.
  x = this->Cursor[0];
  y = this->Cursor[1];

  outData->GetSpacing(mmSpacing);
  // Conversions between mm and pixels:
  //   mm = pix * mmSpacing / mag
  //   pix = mm / mmSpacing * mag

  // Spacing and length of hash marks in pixels. 
  gapx = (int)(mmGap / mmSpacing[0] * mag + 0.49);
  lenx = (int)(mmLen / mmSpacing[0] * mag + 0.49);
  lenx2 = lenx / 2;

  gapy = (int)(mmGap / mmSpacing[1] * mag + 0.49);
  leny = (int)(mmLen / mmSpacing[1] * mag + 0.49);
  leny2 = leny / 2;


  // Check bounds
  if (x < xMin || x > xMax || y < yMin || y > yMax)
    {
    return;
    }

  // Draw horizontal cross
  if (this->IntersectCross)
    {
    DrawLine(0, y, nx1, y, color, outPtr, nxnc, nc);
    }
  else 
    {
    if (x-gapx >= xMin)
      {
      DrawLine(0, y, x-gapx, y, color, outPtr, nxnc, nc);
      }
    if (x+gapx <= xMax)
      {
      DrawLine(x+gapx, y, nx1, y, color, outPtr, nxnc, nc);
      }
    }

  // Draw vertical cross
  if (this->IntersectCross)
    {
    DrawLine(x, 0, x, ny1, color, outPtr, nxnc, nc);
    }
  else 
    {
    if (y-gapy >= yMin)
      {
      DrawLine(x, 0, x, y-gapy, color, outPtr, nxnc, nc);
      }
    if (y+gapy <= yMax)
      {
      DrawLine(x, y+gapy, x, ny1, color, outPtr, nxnc, nc);
      }
    }

  // Hashes along horizontal cross
  d = x;
  for (h=0; h < numHash; h++)
    {
    d += gapx;
    if (d >= xMin && d <= xMax && y-leny2 >= yMin && y+leny2 <= yMax)
      {
      DrawLine(d, y-leny2, d, y+leny2, color, outPtr, nxnc, nc);
      }
    }
  d = x;
  for (h=0; h < numHash; h++)
    {
    d -= gapx;
    if (d >= xMin && d <= xMax && y-leny2 >= yMin && y+leny2 <= yMax)
      {
      DrawLine(d, y-leny2, d, y+leny2, color, outPtr, nxnc, nc);
      }
    }

  // Hashes along vertical cross
  d = y;
  for (h=0; h < numHash; h++)
    {
    d += gapy;
    if (d >= yMin && d <= yMax && x-lenx2 >= xMin && x+lenx2 <= xMax)
      {
      DrawLine(x-lenx2, d, x+lenx2, d, color, outPtr, nxnc, nc);
      }
    }
  d = y;
  for (h=0; h < numHash; h++)
    {
    d -= gapy;
    if (d >= yMin && d <= yMax && x-lenx2 >= xMin && x+lenx2 <= xMax)
      {
      DrawLine(x-lenx2, d, x+lenx2, d, color, outPtr, nxnc, nc);
      }
    }

  // Bullseye around center pixel
  if (this->BullsEye)
    {
    eye = this->BullsEyeWidth;
    e = eye / 2;
    if (x-e >= xMin && x+e <= xMax && y-e >= yMin && y+e <= yMax) 
      {
      DrawLine(x-e, y-e, x+e, y-e, color, outPtr, nxnc, nc); //bottom
      DrawLine(x+e, y-e, x+e, y+e, color, outPtr, nxnc, nc); //right
      DrawLine(x+e, y+e, x-e, y+e, color, outPtr, nxnc, nc); //top
      DrawLine(x-e, y+e, x-e, y-e, color, outPtr, nxnc, nc); //left
      }
    }
}


//----------------------------------------------------------------------------

int vtkSlicerImageCrossHair2D::RequestData(
  vtkInformation* vtkNotUsed( request ),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the data object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkImageData *output = 
    vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkImageData *input = 
    vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

  int *inExt, *outExt;
  inExt = inInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT());
  outExt = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT());

  // Ensure outtput is 2D
  if (inExt[5] != inExt[4]) 
    {
    vtkErrorMacro("Input must be 2D.");
    return 0;
    }

  // if the total size of the data is the same then can be in place
  vtkLargeInteger inSize; 
  vtkLargeInteger outSize; 
  inSize = (inExt[1] - inExt[0] + 1);
  inSize = inSize * (inExt[3] - inExt[2] + 1);
  inSize = inSize * (inExt[5] - inExt[4] + 1);
  outSize = (outExt[1] - outExt[0] + 1);
  outSize = outSize * (outExt[3] - outExt[2] + 1);
  outSize = outSize * (outExt[5] - outExt[4] + 1);
  if (inSize == outSize &&
      this->GetInput()->ShouldIReleaseData())
    {
    // pass the data
    output->GetPointData()->PassData(input->GetPointData());
    output->SetExtent(outExt);
    }
  else
    {
    output->SetExtent(outExt);
    output->AllocateScalars();
    this->CopyData(input,output);
    }

  //--
  
  if (!this->ShowCursor)
    {
    // don't bother with the error checks if the cursor is off
    return 0;
    }

  if ( this->GetInput()->GetDataObjectType() != VTK_IMAGE_DATA )
    {
    vtkWarningMacro ("was sent non-image data data object");
    return 0;
    }

  vtkImageData *outData = this->GetOutput();

  // ensure 3 component data
  int x1 = outData->GetNumberOfScalarComponents();
  if (x1 != 3 && x1 != 4) 
    {
    vtkErrorMacro(<<"Input has "<<x1<<" instead of 3 or 4 components.");
    return 0;
    }

  // Ensure input is unsigned char
  x1 = outData->GetScalarType();
  if (x1 != VTK_UNSIGNED_CHAR)
    {
    vtkErrorMacro("Input is type "<<x1<<" instead of unsigned char.");
    return 0;
    }

  // Draw cursor
  if (this->ShowCursor)
    {
    this->DrawCursor(outData, outExt);
    }

  return 1;
}



//---------------------------------------------------------------------------- 
void vtkSlicerImageCrossHair2D::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "ShowCursor:               " << this->ShowCursor << "\n";
  os << indent << "NumHashes:                " << this->NumHashes << "\n";
  os << indent << "BullsEyeWidth in mm:      " << this->BullsEyeWidth << "\n";
  os << indent << "BullsEye On/Off:          " << this->BullsEye << "\n";
  os << indent << "Length of a Hash Mark:    " << this->HashLength << "\n";
  os << indent << "Cursor Position:          " << this->Cursor[0] << "," << this->Cursor[1] << "\n";
  os << indent << "Does the Cross Intersect? " << this->IntersectCross << "\n";
  os << indent << "Magnification:            " << this->Magnification << "\n";
  os << indent << "HashGap:                  " << this->HashGap << "\n";
  os << indent << "HashLength:               " << this->HashLength << "\n";
  os << indent << "Cursor Color:             " 
    << this->CursorColor[0] << "," << this->CursorColor[2] << "," << this->CursorColor[3] << "\n";
}
