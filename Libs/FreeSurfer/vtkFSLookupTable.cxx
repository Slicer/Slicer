/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFSLookupTable.cxx,v $
  Date:      $Date: 2006/02/28 20:50:07 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
#include "vtkFSLookupTable.h"

#include "vtkObjectFactory.h"

vtkFSLookupTable* vtkFSLookupTable::New()
{
     // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkFSLookupTable");
  if(ret)
  {
      return (vtkFSLookupTable*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkFSLookupTable;
}

//------------------------------------------------------------------------------
vtkFSLookupTable::vtkFSLookupTable()
{
    this->LowThresh = -10000.0;
    this->HiThresh =   10000.0;
    this->LutType = this->FSLUTHEAT;
    this->Reverse = 0;
    this->Truncate = 0;
    this->Offset = 0.25;
    this->Slope = 1.0;
    this->Blufact = 1.0;
    this->FMid = 0.0;
    this->NumberOfColors = 256;
}

//------------------------------------------------------------------------------
vtkFSLookupTable::~vtkFSLookupTable()
{
    // delete any allocated memory
}

//------------------------------------------------------------------------------
void vtkFSLookupTable::PrintSelf(ostream& os, vtkIndent indent)
{
    Superclass::PrintSelf(os, indent);

    os << indent << "LowThres: " << this->LowThresh << endl;
    os << indent << "HiThresh: " << this->HiThresh << endl;
    os << indent << "Look up table type: " << this->GetLutTypeString() << endl;
    os << indent << "Reverse: " << this->Reverse << endl;
    os << indent << "Truncate: " << this->Truncate << endl;
    os << indent << "Offset: " << this->Offset << endl;
    os << indent << "Slope: " << this->Slope << endl;
    os << indent << "Blufact: " << this->Blufact << endl;
    os << indent << "Slope mid point FMid: " << this->FMid << endl;
}

//------------------------------------------------------------------------------
const char *vtkFSLookupTable::GetLutTypeString ()
{
    switch (this->LutType) {
    case FSLUTLABELS:
      return "FreeSurferLabels";
      break;
    case FSLUTHEAT:
      return "Heat"; 
      break;
    case FSLUTGREENRED:
      return "GreenRed";
      break;
    case FSLUTREDGREEN:
        return "RedGreen";
        break;
    case FSLUTBLUERED:
        return "BlueRed";
        break;
    case FSLUTREDBLUE:
        return "RedBlue";
        break;
    default:
        return "Unknown";
    }
}

//------------------------------------------------------------------------------
void vtkFSLookupTable::SetLutTypeToLabels()
{
  this->LutType = this->FSLUTLABELS;
}

//------------------------------------------------------------------------------
void vtkFSLookupTable::SetLutTypeToHeat()
{
    this->LutType = this->FSLUTHEAT;   
    this->FMid = 2.0;
    this->LowThresh = -100.0;
    this->Reverse = 0;
    this->Truncate = 0;
    this->Offset = 2.0;
    this->Slope = 1.5;
    this->FMid = 2.0;
}

//------------------------------------------------------------------------------
void vtkFSLookupTable::SetLutTypeToBlueRed()
{
    this->LutType = this->FSLUTBLUERED;
    this->LowThresh = -10000.0;
    this->HiThresh =   10000.0;
    this->Reverse = 0;
    this->Truncate = 0;
    this->Offset = 0.25;
    this->Slope = 1.0;
    this->Blufact = 1.0;
    this->FMid = 0.0;
    this->NumberOfColors = 256;
}

//------------------------------------------------------------------------------
void vtkFSLookupTable::SetLutTypeToRedBlue()
{
    this->LutType = this->FSLUTREDBLUE;
    this->LowThresh = -10000.0;
    this->HiThresh =   10000.0;
    this->Reverse = 1;
    this->Truncate = 0;
    this->Offset = 0.25;
    this->Slope = 1.0;
    this->Blufact = 1.0;
    this->FMid = 0.0;
    this->NumberOfColors = 256;
}

//------------------------------------------------------------------------------
void vtkFSLookupTable::SetLutTypeToRedGreen()
{
    this->LutType = this->FSLUTREDGREEN;
    this->LowThresh = -10000.0;
    this->HiThresh =   10000.0;
    this->Reverse = 0;
    this->Truncate = 0;
    this->Offset = 0.25;
    this->Slope = 1.0;
    this->Blufact = 1.0;
    this->FMid = 0.0;
    this->NumberOfColors = 256;
}

//------------------------------------------------------------------------------
void vtkFSLookupTable::SetLutTypeToGreenRed()
{
    this->LutType = this->FSLUTGREENRED;
    this->LowThresh = -10000.0;
    this->HiThresh =   10000.0;
    this->Reverse = 0;
    this->Truncate = 0;
    this->Offset = 0.25;
    this->Slope = 1.0;
    this->Blufact = 1.0;
    this->FMid = 0.0;
    this->NumberOfColors = 256;
}

//------------------------------------------------------------------------------
// What a strange code ...
double *vtkFSLookupTable::GetRange()
{
    double range[2];

    range[0] = this->LowThresh;
    range[1] = this->HiThresh;

    return (double *)NULL;
    
}

//------------------------------------------------------------------------------
void vtkFSLookupTable::SetRange(double /*lo*/, double /*hi*/)
{
    // don't do anything as it's overriding the LUTs low threshold with the
    // scalar values' lowest value
//    this->SetLowThresh(lo);
//    this->SetHiThresh(hi);
}

//------------------------------------------------------------------------------
unsigned char *vtkFSLookupTable::MapValue(double val)
{
    /// variables for the heat colour scale
    float f, ftmp, c1, c2, fcurv;
    /// variables for the green red colour scale
    float curv;
    
    /// the final calculated values, red, green, blue, alpha 
    float r, g, b, a;

    r = 0.0;
    g = 0.0;
    b = 0.0;
    a = 1.0;

    switch (this->LutType) {
    case FSLUTHEAT:
    case FSLUTBLUERED:
    case FSLUTREDBLUE:
      // temp solution for red/blue, blue/red
      if (this->LutType == FSLUTREDBLUE)
        {
        this->ReverseOn();
        }
        /// functional overlay heat scale
        f = val;
        fcurv = this->Slope;
        if (this->Reverse)
        {
            f = -f;
        }
        if (this->Truncate && f < 0)
        {
            f = 0;
        }
        if (fabs(f) > this->LowThresh && fabs(f) < this->FMid)
        {
            ftmp = fabs(f);
            c1 = 1.0/(this->FMid - this->LowThresh);
            if (fcurv != 1.0)
            {
                c2 = (this->FMid - this->LowThresh - fcurv*c1*sqrt(this->FMid - this->LowThresh))/((1 - fcurv)*(this->FMid - this->LowThresh));
            }
            else
            {
                c2 = 0;
            }
            ftmp = fcurv*c1*sqrt(ftmp - this->LowThresh) + c2*(1 - fcurv)*(ftmp - this->LowThresh) + this->LowThresh;
            f = (f<0)?-ftmp:ftmp;
        }
        if (f>=0)
        {
            r = 
                this->Offset*((f < this->LowThresh)?1:(f < this->FMid)?1 - (f - this->LowThresh)/(this->FMid - this->LowThresh):0) +
                ((f < this->LowThresh)?0:(f < this->FMid)?(f - this->LowThresh)/(this->FMid - this->LowThresh):1);
            g = 
                this->Offset*((f < this->LowThresh)?1:(f < this->FMid)?1 - (f - this->LowThresh)/(this->FMid - this->LowThresh):0) +
                ((f < this->FMid)?0:(f < this->FMid + 1.00/this->Slope)?1*(f - this->FMid)*this->Slope:1);
            b = 
                this->Offset*((f < this->LowThresh)?1:(f < this->FMid)?1 - (f - this->LowThresh)/(this->FMid - this->LowThresh):0);
        }
        else
        {
            f = -f;
            b = 
                this->Offset*((f < this->LowThresh)?1:(f < this->FMid)?1 - (f - this->LowThresh)/(this->FMid - this->LowThresh):0) +
                ((f < this->LowThresh)?0:(f < this->FMid)?(f - this->LowThresh)/(this->FMid - this->LowThresh):1);
            g = 
                this->Offset*((f < this->LowThresh)?1:(f < this->FMid)?1 - (f - this->LowThresh)/(this->FMid - this->LowThresh):0) +
                ((f < this->FMid)?0:(f < this->FMid + 1.00/this->Slope)?1*(f - this->FMid)*this->Slope:1);
            r = 
                this->Offset*((f < this->LowThresh)?1:(f < this->FMid)?1 - (f - this->LowThresh)/(this->FMid - this->LowThresh):0);
        }

        this->RGBA[0] = (unsigned char)(r * 255.0);
        this->RGBA[1] = (unsigned char)(g * 255.0);
        this->RGBA[2] = (unsigned char)(b * 255.0);
        this->RGBA[3] = (unsigned char)(a * 255.0);

        break;
    case FSLUTGREENRED:
        // 
        curv = val;
        if (curv < this->LowThresh)
        {
            b = 255 * (this->Offset/this->Blufact + 0.95*(1-this->Offset/this->Blufact)); /* yellow */
            r = g = 0 ;
        }
        else if (curv > this->HiThresh)
        {
            r = g = 255 * (this->Offset/this->Blufact + 0.95*(1-this->Offset/this->Blufact)); /* yellow */
            b = 0 ;
        }
        else
        {
            f = tanh(this->Slope*(curv-this->FMid));
            if (f>0) {
                r = 255 * (this->Offset/this->Blufact + 0.95*(1-this->Offset/this->Blufact)*fabs(f));
                g = 255 * (this->Offset/this->Blufact*(1 - fabs(f)));
            }
            else {
                r = 255 * (this->Offset/this->Blufact*(1 - fabs(f)));
                g = 255 * (this->Offset/this->Blufact + 0.95*(1-this->Offset/this->Blufact)*fabs(f));
            }
            b = 255 * (this->Offset*this->Blufact*(1 - fabs(f)));
        }

        this->RGBA[0] = (unsigned char)(r);
        this->RGBA[1] = (unsigned char)(g);
        this->RGBA[2] = (unsigned char)(b);
        this->RGBA[3] = (unsigned char)(a);

        break;
    case FSLUTREDGREEN:
//        vtkErrorMacro(<<"Not doing anything for Red Green just yet\n");
        // from GREENRED, just switch green and red values
        curv = val;
        if (curv < this->LowThresh)
        {
            b = 255 * (this->Offset/this->Blufact + 0.95*(1-this->Offset/this->Blufact)); /* yellow */
            r = g = 0 ;
        }
        else if (curv > this->HiThresh)
        {
            r = g = 255 * (this->Offset/this->Blufact + 0.95*(1-this->Offset/this->Blufact)); /* yellow */
            b = 0 ;
        }
        else
        {
            f = tanh(this->Slope*(curv-this->FMid));
            if (f>0) {
                g = 255 * (this->Offset/this->Blufact + 0.95*(1-this->Offset/this->Blufact)*fabs(f));
                r = 255 * (this->Offset/this->Blufact*(1 - fabs(f)));
            }
            else {
                g = 255 * (this->Offset/this->Blufact*(1 - fabs(f)));
                r = 255 * (this->Offset/this->Blufact + 0.95*(1-this->Offset/this->Blufact)*fabs(f));
            }
            b = 255 * (this->Offset*this->Blufact*(1 - fabs(f)));
        }

        this->RGBA[0] = (unsigned char)(r);
        this->RGBA[1] = (unsigned char)(g);
        this->RGBA[2] = (unsigned char)(b);
        this->RGBA[3] = (unsigned char)(a);

        break;
        /*
    case FSLUTBLUERED:
//        vtkErrorMacro(<<"Not doing anything for Blue Red just yet\n");
        break;
    case FSLUTREDBLUE:
//        vtkErrorMacro(<<"Not doing anything for  Red Blue just yet\n");
        break;
        */
    default:
        vtkErrorMacro(<<"Unknown look up table type " << this->LutType);
    }

//    vtkDebugMacro(<<"R = " << this->RGBA[1] << ", G = " <<  this->RGBA[2] << ", B = " <<  this->RGBA[3] << endl);
    
    return this->RGBA;
}

//------------------------------------------------------------------------------
void vtkFSLookupTable::GetColor(double val, double rgb[3])
{
    unsigned char *rgb8 = this->MapValue(val);
    
    rgb[0] = rgb8[0]/255.0;
    rgb[1] = rgb8[1]/255.0;
    rgb[2] = rgb8[2]/255.0;
}

//------------------------------------------------------------------------------
void vtkFSLookupTable::MapScalarsThroughTable2(void *input, unsigned char *output,
                               int inputDataType, int numberOfValues,
                               int inputIncrement, int outputIncrement)
{
    int n;
    double rgb[3];
    double val;
    float *inPtr;
    unsigned char *ucPtr;
    unsigned char ucVal;
    
    
    vtkDebugMacro( << "MapScalarsThroughTable2:\n");
    vtkDebugMacro( << "\tinputDataType = " << inputDataType << ", number of vals = " << numberOfValues << ", input incr = " << inputIncrement << ",\noutput incr = " << outputIncrement << ", VTK_RGBA data type = "<< VTK_RGBA << ", lut type = " << this->LutType << endl);

    if (input == NULL)
      {
      vtkErrorMacro(<<"Input scalars are null!");
      return;
      }
    if (output == NULL)
      {
      vtkErrorMacro(<<"Output array is null!");
      return;
      }
    if (this->LutType == 0)
      {
      // labels, just call the superclass
      vtkDebugMacro("Probably have labels, calling the superclass to map through the table.");
      Superclass::MapScalarsThroughTable2(input, output, inputDataType, numberOfValues, inputIncrement, outputIncrement);
      return;
      }
    
    
    if (outputIncrement != VTK_RGBA &&
        outputIncrement != VTK_RGB)
      {
      vtkErrorMacro(<<"Output increment " << outputIncrement << " doesn't match VTK_RGBA data type ("<< VTK_RGBA << ") nor VTK_RGB data type ("<< VTK_RGB << "), returning");
      return;
      }
    
    switch (inputDataType)
      {
      case VTK_FLOAT:
        vtkDebugMacro(<<"Input data type is float.");
        for (n = 0; n < numberOfValues; n++)
          {
          inPtr = static_cast<float *>(input);
          //inPtr += n*inputIncrement*sizeof(float);
          inPtr += n*inputIncrement;
          val = *inPtr;
          this->GetColor(val, rgb);
          if (n < 100) // (n % 10000 == 0)
            {
            vtkDebugMacro( << n << ": val = " << val << ", rgb = " << rgb[0] << "," << rgb[1] << "," << rgb[2] << "\noutput array index = " << n*outputIncrement*sizeof(unsigned char));
            }
          if (rgb[0] < 0.0 || rgb[0] > 1.0 ||
              rgb[1] < 0.0 || rgb[1] > 1.0 ||
              rgb[2] < 0.0 || rgb[2] > 1.0)
            {
            // invalid colour values, use 0
            vtkDebugMacro(<<"Val " << val << " resulted in invalid rgb, out of range 0-1, using 0 0 0 instead of " << rgb[0] << " " << rgb[1] << " " << rgb[2] );
            rgb[0] = 0.0;
            rgb[1] = 0.0;
            rgb[2] = 0.0;
            }
          // now save it to the output - should loop 0 to
          // outputIncrement
          
          output[n*outputIncrement*sizeof(unsigned char)] = (unsigned char)(rgb[0]*255.0);
          output[n*outputIncrement*sizeof(unsigned char) + 1] = (unsigned char)(rgb[1]*255.0);
          output[n*outputIncrement*sizeof(unsigned char) + 2] = (unsigned char)(rgb[2]*255.0);
          if (outputIncrement == VTK_RGBA)
            {
            // opacity set to be always 1
            output[n*outputIncrement*sizeof(unsigned char) + 3] = (unsigned char)(255);
            }
          }
        break;
      case VTK_UNSIGNED_CHAR:
        vtkDebugMacro("Input data type is unsigned char.");
        for (n = 0; n < numberOfValues; n++)
          {
          ucPtr = static_cast<unsigned char*>(input);
          ucPtr += n*inputIncrement;
          ucVal = *ucPtr;
          unsigned char *rgb8 = this->MapValue((double)ucVal);
          
          // now save it to the output - unsigned char return guarantees it's 0-255
          output[n*outputIncrement*sizeof(unsigned char)] = rgb8[0];
          output[n*outputIncrement*sizeof(unsigned char) + 1] = rgb8[1];
          output[n*outputIncrement*sizeof(unsigned char) + 2] = (unsigned char)rgb8[2];
          if (outputIncrement == VTK_RGBA)
            {
            // opacity set to be always 1
            output[n*outputIncrement*sizeof(unsigned char) + 3] = (unsigned char)(255);
            }
          }
        break;
      default:
        vtkErrorMacro(<<"MapScalarsThroughTable2: Have no idea how to deal with this input type " << inputDataType);
      }
}
