/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkEdgeGradient.cxx,v $
  Date:      $Date: 2006/05/26 19:59:41 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
/*===========================================================

Authors: Michael McKenna, David Small, Steve Pieper.
    Small Design Firm, Inc.,
    in association with Isomics, Inc.,
    and Partners Health Care.  MA, USA.
    June, 2004

===========================================================*/


#include <math.h>

#include "vtkEdgeGradient.h"

#include "vtkObjectFactory.h"
#include "vtkTexture.h"
#include "vtkActor.h"
#include "vtkImageData.h"
#include "vtkUnsignedCharArray.h"
#include "vtkPointData.h"
#include "vtkCollection.h"


vtkCxxRevisionMacro(vtkEdgeGradient, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkEdgeGradient);


vtkEdgeGradient::vtkEdgeGradient()
{
    this->Width = 32;
    this->Height = 32;

    this->Error = 0;
    this->Initialized = 0;

    //this->Turn = 0;
    this->Corner = 0;
}


vtkEdgeGradient::~vtkEdgeGradient()
{
    // TODO - anything to clean/delete?
}


void vtkEdgeGradient::Create() {
    MakeImageData(NULL, NULL);
}


// set up the info for 'this' vtkImageData
void vtkEdgeGradient::MakeImageData(unsigned char *alpha, unsigned char *intensity) {

    SetDimensions(GetWidth(), GetHeight(), 1);

    vtkUnsignedCharArray *scalars = vtkUnsignedCharArray::New();
    // only 2 components - luminance and alpha (not 4 RGBA)
    scalars->SetNumberOfComponents(2);
    //scalars->SetNumberOfComponents(1);

    int p = 0;

    if (this->Corner) {
        for (int y = 0; y < GetHeight(); y++) {
            for (int x = 0; x < GetWidth(); x++) {
                if (intensity == NULL) scalars->InsertNextValue(255);
                else scalars->InsertNextValue(intensity[p]);

                // NOTE - assuming square map
                if (alpha == NULL) {
                    int a = 255;

                    // distance from the corner of the rect, to form a semi-circle
                    vtkFloatingPointType d = sqrt((float)(x*x + y*y)) / (vtkFloatingPointType)GetWidth();
                    if (d < .25) a = (int)(255. * d * 4.);
                    else if (d > .75) {
                        if ( d > 1.) d = 1.;
                        a = (int)(255. * (1.0 - d) * 4.);
                    }

                    scalars->InsertNextValue(a);
                }

                else scalars->InsertNextValue(alpha[p]);
                p+=1;
            }
        }
    }

    // NOTE - eliminated the Turn option - we do this via texture coords in Rectangle now.
    // also, the repeating this code block was silly.

    //else if (this->Turn == 0) {

    for (int y = 0; y < GetHeight(); y++) {
        for (int x = 0; x < GetWidth(); x++) {
            if (intensity == NULL) scalars->InsertNextValue(255);
            else scalars->InsertNextValue(intensity[p]);

            if (alpha == NULL) {
                int a = 255;
                // first quarter
                if (y < GetHeight() / 4) 
                    a = (int)(255. / (vtkFloatingPointType)GetHeight() * (vtkFloatingPointType)y * 4.);
                // last quarter
                if (y > GetHeight() / 4 * 3)
                    a = (int)(255. / (vtkFloatingPointType)GetHeight() * (vtkFloatingPointType)(GetHeight() - y) * 4.);

                scalars->InsertNextValue(a);
            }

            else scalars->InsertNextValue(alpha[p]);
            p+=1;
        }
    }

    /*
    else {  // Turn == 1
        for (int y = 0; y < GetHeight(); y++) {
            for (int x = 0; x < GetWidth(); x++) {
                if (intensity == NULL) scalars->InsertNextValue(255);
                else scalars->InsertNextValue(intensity[p]);
                if (alpha == NULL) {
                    int a = 255;
                    // first quarter
                    if (x < GetWidth() / 4) 
                        a = (int)(255. / (vtkFloatingPointType)GetWidth() * (vtkFloatingPointType)x * 4.);
                    // last quarter
                    if (x > GetWidth() / 4 * 3)
                        a = (int)(255. / (vtkFloatingPointType)GetWidth() * (vtkFloatingPointType)(GetWidth() - x) * 4.);

                    scalars->InsertNextValue(a);
                }
                else scalars->InsertNextValue(alpha[p]);
                p+=1;
            } } }
*/

    GetPointData()->SetScalars(scalars);
    SetScalarTypeToUnsignedChar();
}


vtkEdgeGradient *vtkEdgeGradient::filter_image(unsigned char *src, unsigned char *dst, int wd, int ht, int filter)
{
    int x, y, xx, yy;
    int cum, div;
    unsigned char *ptr, *dstptr;
    int maxdist;
    int max_filter = 10;
    int kernal[450], *kernal_ptr;
    int count;
    
    if (filter > max_filter) filter = max_filter;
    
    maxdist = (int) (2.0*sqrt((float)(filter*filter + filter*filter)));
    
    div = 0;
    count = 0;
    kernal_ptr = kernal;
    
    for(yy=-filter; yy<filter; yy++) {
        for(xx=-filter; xx<filter; xx++) {
            count++;
            *kernal_ptr = maxdist - (int) (2.0*sqrt((float)(xx*xx + yy*yy)));
            div += *kernal_ptr;
            kernal_ptr++;
        }
    } 
    //printf( "Filtering - Kernal size is %d, Divisor is %d....\n", count, div);
    
    dstptr = dst;
    for(y=filter; y<ht-filter; y ++) {
        for(x=filter;x<wd-filter;x++) {
            cum = 0;
            ptr = src + (x-filter)+(y-filter)*wd;
            kernal_ptr = kernal;
            for(yy=-filter; yy<filter; yy++) {
                for(xx=-filter; xx<filter; xx++) {
                    cum += *ptr++ * *kernal_ptr++;
                }
                ptr += wd - (2*filter);
            }
            *dstptr++ = (unsigned char)(cum/div); 
        }
        dstptr += filter*2;
    }
    //printf("done\n");
    return this;
}


// helper methods
void vtkEdgeGradient::zero_array(unsigned char *ptr, int size)
{
    for (int i = 0; i < size; i++) *ptr++ = 0;
}

void vtkEdgeGradient::copy_array(unsigned char *dst, unsigned char *src, int size)
{
    for (int i = 0; i < size; i++) *dst++ = *src++;
}


void vtkEdgeGradient::copyArrayOffset(unsigned char *dst, 
                                     unsigned char *src,
                                     int w, int h, int xOff, int yOff)
{
    int xStart = 0;
    if (xOff < 0) xStart = -xOff;
    int yStart = 0;
    if (yOff < 0) yStart = -yOff;

    int xEnd = w;
    if (xOff > 0) xEnd = w - xOff;
    int yEnd = h;
    if (yOff > 0) yEnd = h - yOff;

    for (int y = yStart; y < yEnd; y++) {
        for (int x = xStart; x < xEnd; x++) {
            dst[x + xOff + (y + yOff) * w] = src[x + y * w];
        }
    }
}


void vtkEdgeGradient::mattArrayOffset(unsigned char *bgAlpha, unsigned char *bgI, 
                                     unsigned char *fgAlpha, unsigned char fgLevel, 
                                     int w, int h, int xOff, int yOff)
{
    int xStart = 0;
    if (xOff < 0) xStart = -xOff;
    int yStart = 0;
    if (yOff < 0) yStart = -yOff;

    int xEnd = w;
    if (xOff > 0) xEnd = w - xOff;
    int yEnd = h;
    if (yOff > 0) yEnd = h - yOff;


    bool over = 0;
    bool assoc = 1;

    // just one intensity value for the source (eg. all white image)
    for (int y = yStart; y < yEnd; y++) {
        for (int x = xStart; x < xEnd; x++) {
            /*
            http://astronomy.swin.edu.au/~pbourke/colour/composite/

            Perhaps the most common blending function was first promoted by Alvy Smith and Ed Catnull around 1977 and is often called the "over" operator.
            
            R = R1 (1 - A2) + R2 A2
            G = G1 (1 - A2) + G2 A2
            B = B1 (1 - A2) + B2 A2
            A = A1 (1 - A2) + A2 A2
        
            For some application there is a serious problem with the above blending function, it is not associative. That is, if we have 4 layers to composite 
            the result from compositing (A & B & C & D) is not the same as (A & B) & (C & D), indeed this is true for the vast majority of the blending functions. 
            Having a blending function that is associative is often required (eg: distributed volume rendering) and in other cases can lead to improved compositing performance. 
            Bruce Wallace and Marc Levoy, and later, Tim Porter and Tom Duff proposed the following blending function that is associative. Note that the following 
            expression can be simplified (and compositing performance increased) by premultiplying each colour by its alpha value. 

            R = R1 A1 (1 - A2) + R2 A2
            G = G1 A1 (1 - A2) + G2 A2
            B = B1 A1 (1 - A2) + B2 A2
            A = A1 (1 - A2) + A2  
            */

            int i = 0;
            if (over) {
                i = bgI[x + xOff + (y + yOff) * w] * (255 - fgAlpha[x + y * w]) +
                    fgLevel * fgAlpha[x + y * w];  

                // makes white text border better, but black text gets white border
                //i = bgI[x + xOff + (y + yOff) * w] * (255 - fgAlpha[x + y * w]);
                //int l = (fgAlpha[x + y * w] + (255 - bgAlpha[x + xOff + (y + yOff) * w]));
                //if (l > 255) l = 255;
                //i += fgLevel * l;                    
            }
            if (assoc) {
                i = bgI[x + xOff + (y + yOff) * w] * bgAlpha[x + xOff + (y + yOff) * w] * (255 - fgAlpha[x + y * w]) +
                    fgLevel * fgAlpha[x + y * w];
            }

            i /= 255;
            if (i < 0) {
                printf("i < 0 : %d\n", i);
                i = 0;
            }
            if (i > 255) {
                printf("i > 255 : %d\n", i);
                i = 255;
            }
            bgI[x + xOff + (y + yOff) * w] = (unsigned char)i;

            int a = 0;
            if (over) {
                a = bgAlpha[x + xOff + (y + yOff) * w] * (255 - fgAlpha[x + y * w]) +
                    fgAlpha[x + y * w] * fgAlpha[x + y * w];
                a /= 255;
            }
            if (assoc) {
                a = bgAlpha[x + xOff + (y + yOff) * w] * (255 - fgAlpha[x + y * w]);
                a /= 255;
                a += fgAlpha[x + y * w];
            }

            if (a < 0) {
                printf("a < 0 : %d\n", a);
                a = 0;
            }
            if (a > 255) {
                printf("a > 255 : %d\n", a);
                a = 255;
            }
            bgAlpha[x + xOff + (y + yOff) * w] = (unsigned char)a;
        }
    }
}



void vtkEdgeGradient::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    /*  TODO - do print
    os << indent << "X Resolution: " << this->XResolution << "\n";
    os << indent << "Y Resolution: " << this->YResolution << "\n";
    */
}
