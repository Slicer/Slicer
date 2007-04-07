/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTextureFont.cxx,v $
  Date:      $Date: 2006/05/26 19:59:44 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
/*

Authors: Michael McKenna, David Small, Steve Pieper.
    Small Design Firm, Inc.,
    in association with Isomics, Inc.,
    and Partners Health Care.  MA, USA.
    June, 2004

===========================================================*/


#include <math.h>

#include "vtkTextureFont.h"

#include "vtkTexture.h"
#include "vtkActor.h"
#include "vtkImageData.h"
#include "vtkUnsignedCharArray.h"
#include "vtkPointData.h"
#include "vtkCollection.h"


vtkCxxRevisionMacro(vtkTextureFont, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkTextureFont);


vtkTextureFont::vtkTextureFont()
{
    mCharPositions = NULL;
    mCharSizes = NULL;

    this->FreetypeRasterizer = NULL;

    this->Width = 0;
    this->Height = 0;

    this->FontParameters = vtkFontParameters::New();
    this->FontParameters->TextureFont = this;

    this->Error = 0;
    this->Initialized = 0;
}


int vtkTextureFont::CreateRasterizer(char *fontpath) {
    if (this->FontParameters == NULL) {
        vtkErrorMacro("vtkTextureFont::CreateRasterizer - ERROR - NULL FontParameters.\n");
        return -1;
    }

    this->FreetypeRasterizer = vtkFreetypeRasterizer::New();
    this->FreetypeRasterizer->SetResolution(this->FontParameters->GetResolution());
    this->FreetypeRasterizer->LoadFont(fontpath);

    if (this->FreetypeRasterizer->mError) {
        this->FreetypeRasterizer->Delete();
        this->FreetypeRasterizer = NULL;
        return -1;
    }
    return 0;
}


int vtkTextureFont::CreateFont()
{
    int c;
    unsigned char *charBitmap, *texptr;
    int charBitmap_wd, charBitmap_ht, xpos, ypos, x, y;
    int tex_width, tex_height, max_height;
    int n = 0;
    int gap = 0;

    // TODO - CHAR LIMIT
    char fontpath[512];

    this->Initialized = 0;

    vtkFontParameters *params = this->FontParameters;

    if (params == NULL) {
        vtkErrorMacro("vtkTextureFont::CreateFont() - ERROR - FontParemeters has not been set.\n");
        this->Error = 1;
        return this->Error;
    }

    if (params->GetFontFileName() == NULL) {
        vtkErrorMacro("vtkTextureFont::CreateFont() - ERROR - FontFileName has not been set.\n");
        this->Error = 1;
        return this->Error;
    }

    // FontManager is actually going to call w/ at least the default dir.
    if (params->GetFontDirectory() == NULL) {
        vtkErrorMacro("vtkTextureFont::CreateFont() - ERROR - FontDirectory has not been set.\n");
        this->Error = 1;
        return this->Error;
    }


    gap = params->GetBlur() + 1;
    int charExtend = params->GetBlur();

    if (params->GetStyle()) {
        gap += params->GetStyleBorderSize();
        charExtend += params->GetStyleBorderSize();
    }

    sprintf(fontpath, "%s/%s", params->GetFontDirectory(), params->GetFontFileName());

    int lerror = CreateRasterizer(fontpath);

    if (lerror) {
        vtkErrorMacro("vtkTextureFont::CreateFont error: the font: " <<
            params->GetFontFileName() << " could not be loaded. aborting.\n")
        this->Error = 1;
        return this->Initialized;
    }

    // Set up font image buffer
    
    // find the real size it needs
    // Num_chars(font) is the # of characters in font, should be no more than 222
    tex_width = tex_height = max_height = gap;
    for (c = 0; c < (int)this->FreetypeRasterizer->mRasterizerCharacters->GetNumberOfItems(); c++) {
        vtkRasterizerCharacter *ch = ((vtkRasterizerCharacter *)this->FreetypeRasterizer->mRasterizerCharacters->GetItemAsObject(c));
        if (max_height < ch->mHeight) max_height = ch->mHeight;
        if ((tex_width + ch->mWidth + gap * 2) < 1024) tex_width += ch->mWidth + gap * 2;
        else {
            tex_height += max_height + gap *2 ;
            max_height = 0;
            // TODO - is this supposed to be "gap * 2"?
            tex_width = ch->mWidth + gap * 3;
        }
    }
    tex_height += max_height + gap * 2;
    
    while (static_cast<int>(pow(static_cast<double>(2.), n)) < tex_height) n++;
    
    // TODO - should we bother w/ the pow 2 texture size?
    tex_height = static_cast<int>( pow( static_cast<double>(2.), n ) );
    // 1024 width so that the buffer doesn't make an extra-wide texture map, which can be inefficient in some systems
    tex_width = 1024;

    this->Width = tex_width;
    this->Height = tex_height;
    
    // buffer is the entire texture map where all the letters will be temporarily stored.  
    // It is a 2D picture whose height is 64 and length is a power of 2.
    unsigned char *buffer = (unsigned char *) malloc(sizeof(unsigned char) * tex_width * tex_height);

    zero_array(buffer, tex_width * tex_height);
    
    // setup mCharPositions and mCharSizes
    mCharPositions  = (Vector2D<vtkFloatingPointType> *) malloc (sizeof(Vector2D<vtkFloatingPointType>) * this->FreetypeRasterizer->mRasterizerCharacters->GetNumberOfItems());  // The x,y position of each character in the texture map (buffer).
    mCharSizes = (Vector2D<vtkFloatingPointType> *) malloc (sizeof(Vector2D<vtkFloatingPointType>) * this->FreetypeRasterizer->mRasterizerCharacters->GetNumberOfItems());  // The size (width,height) of the letter in the texture map.
    
    ypos = xpos = gap;
    max_height = 0;
    
    for (c = 0; c < (int)this->FreetypeRasterizer->mRasterizerCharacters->GetNumberOfItems(); c++) {
        vtkRasterizerCharacter *ch = ((vtkRasterizerCharacter *)this->FreetypeRasterizer->mRasterizerCharacters->GetItemAsObject(c));
        charBitmap = ch->mBitmap;
        charBitmap_wd = ch->mWidth;
        charBitmap_ht = ch->mHeight;
        
        if (max_height < charBitmap_ht)
            max_height = charBitmap_ht;
        
        if (xpos + charBitmap_wd + (gap*2) > 1024)
            xpos = gap, ypos += max_height+(gap*2), max_height = 0;
        
        texptr = buffer + ((ypos * tex_width) + xpos);
        
        for(y=0; y<charBitmap_ht; y ++) 
        {
            for(x=0;x<charBitmap_wd;x++) 
            {
                // This assigns the current index of texptr to be 
                // the current index of bitmap and increments both.
                *texptr++ = *charBitmap++;
            }
            texptr += (tex_width - charBitmap_wd); 
            // This makes texptr go to the next level of the bitmap
        }
        
        ch->mHeight += charExtend * 2;
        ch->mWidth += charExtend * 2;
        ch->mTopExtent += charExtend;
        ch->mLeftExtent -= charExtend;
        
        //mCharPositions[c][0] = (vtkFloatingPointType)(xpos - (charExtend * 2)) / (vtkFloatingPointType)tex_width;
        //mCharPositions[c][1] = (vtkFloatingPointType)(ypos - (charExtend * 2)) / (vtkFloatingPointType)tex_height;

        //mCharPositions[c][0] = (vtkFloatingPointType)(xpos - (charExtend)) / (vtkFloatingPointType)tex_width;
        //mCharPositions[c][1] = (vtkFloatingPointType)(ypos - (charExtend)) / (vtkFloatingPointType)tex_height;

        // confused - why don't we have to back out the char pos by the shadow outline dist also?
        //   just the blur works, tho.  hmm... because xpos already has it built in, via gap...
        //mCharPositions[c][0] = (vtkFloatingPointType)(xpos - (this->GetBlur() * 2)) / (vtkFloatingPointType)tex_width;
        //mCharPositions[c][1] = (vtkFloatingPointType)(ypos - (this->GetBlur() * 2)) / (vtkFloatingPointType)tex_height;

        // ahh - need a combination of 1 this->GetBlur() and 1 charExtend, instead of 2x of either one.
        mCharPositions[c][0] = (vtkFloatingPointType)(xpos - (params->GetBlur() + charExtend)) / (vtkFloatingPointType)tex_width;
        mCharPositions[c][1] = (vtkFloatingPointType)(ypos - (params->GetBlur() + charExtend)) / (vtkFloatingPointType)tex_height;


        mCharSizes[c][0] = (vtkFloatingPointType)ch->mWidth / (vtkFloatingPointType)tex_width;
        mCharSizes[c][1] = (vtkFloatingPointType)ch->mHeight / (vtkFloatingPointType)tex_height;
        
        xpos += charBitmap_wd + (gap * 2);
    }
    
    // If there is a blur, filter the image
    if (params->GetBlur() > 0) {
        unsigned char *filtered;
        filtered = (unsigned char *) malloc(sizeof(unsigned char) * tex_width * tex_height);
        zero_array(filtered, tex_width * tex_height); 
        filter_image(buffer, filtered, tex_width, tex_height, params->GetBlur());

        // TODO - shouldn't we just update buffer pointer, and not do a copy
        copy_array(buffer, filtered, tex_width * tex_height);
        free(filtered);
    }
    

    if (params->GetStyle() == VTK_TEXTURE_TEXT_STYLE_SHADOW) {
        unsigned char *bgIntensity, *bgAlpha;
        bgIntensity = (unsigned char *) malloc(sizeof(unsigned char) * tex_width * tex_height);
        zero_array(bgIntensity, tex_width * tex_height);

        bgAlpha = (unsigned char *) malloc(sizeof(unsigned char) * tex_width * tex_height);
        zero_array(bgAlpha, tex_width * tex_height);

        copyArrayOffset(bgAlpha, buffer, tex_width, tex_height, params->GetStyleBorderSize(), params->GetStyleBorderSize());

        mattArrayOffset(bgAlpha, bgIntensity, buffer, 255, tex_width, tex_height, 0, 0);

        MakeImageData(bgAlpha, bgIntensity);
        free(bgIntensity);
        free(bgAlpha);
    }
    else if (params->GetStyle() == VTK_TEXTURE_TEXT_STYLE_OUTLINE) {
        unsigned char *bgIntensity, *bgAlpha;
        bgIntensity = (unsigned char *) malloc(sizeof(unsigned char) * tex_width * tex_height);
        zero_array(bgIntensity, tex_width * tex_height);
        bgAlpha = (unsigned char *) malloc(sizeof(unsigned char) * tex_width * tex_height);
        zero_array(bgAlpha, tex_width * tex_height);

        //copyArrayOffset(bgAlpha, buffer, tex_width, tex_height, 0, 0);
        for (int x = -params->GetStyleBorderSize(); x <= params->GetStyleBorderSize(); x++) {
            for (int y = -params->GetStyleBorderSize(); y <= params->GetStyleBorderSize(); y++) {
                mattArrayOffset(bgAlpha, bgIntensity, buffer, 0, tex_width, tex_height, x, y);
            }
        }

        mattArrayOffset(bgAlpha, bgIntensity, buffer, 255, tex_width, tex_height, 0, 0);

        MakeImageData(bgAlpha, bgIntensity);
        free(bgIntensity);
        free(bgAlpha);
    }
    else MakeImageData(buffer, NULL);

    free(buffer);

    this->Initialized = 1;

    return this->Initialized;
}


vtkTextureFont::~vtkTextureFont()
{
    if (mCharPositions != NULL) free(mCharPositions);
    if (mCharSizes != NULL) free(mCharSizes);
    if (this->FreetypeRasterizer != NULL) this->FreetypeRasterizer->Delete();

    // TODO - delete the other (vtk) stuff?  free FontFileName/Dir?
}


// set up the info for 'this' vtkImageData
void vtkTextureFont::MakeImageData(unsigned char *alpha, unsigned char *intensity) {
    SetDimensions(GetWidth(), GetHeight(), 1);

    vtkUnsignedCharArray *scalars = vtkUnsignedCharArray::New();
    // only 2 components - luminance and alpha (not 4 RGBA)
    scalars->SetNumberOfComponents(2);
    //scalars->SetNumberOfComponents(1);

    int p = 0;
    for (int y = 0; y < GetHeight(); y++) {
        for (int x = 0; x < GetWidth(); x++) {
            if (intensity == NULL) scalars->InsertNextValue(255);
            else scalars->InsertNextValue(intensity[p]);
            scalars->InsertNextValue(alpha[p]);
            p+=1;
        }
    }

    GetPointData()->SetScalars(scalars);
    SetScalarTypeToUnsignedChar();
}


vtkTextureFont *vtkTextureFont::filter_image(unsigned char *src, unsigned char *dst, int wd, int ht, int filter)
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


Vector2D<vtkFloatingPointType> vtkTextureFont::GetChararacterPosition(int c) 
{ 
    return (mCharPositions[c]); 
}


Vector2D<vtkFloatingPointType> vtkTextureFont::GetChararacterSize(int c) 
{ 
    return (mCharSizes[c]); 
}


// helper methods
void vtkTextureFont::zero_array(unsigned char *ptr, int size)
{
    for (int i = 0; i < size; i++) *ptr++ = 0;
}

void vtkTextureFont::copy_array(unsigned char *dst, unsigned char *src, int size)
{
    for (int i = 0; i < size; i++) *dst++ = *src++;
}


void vtkTextureFont::copyArrayOffset(unsigned char *dst, 
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


void vtkTextureFont::mattArrayOffset(unsigned char *bgAlpha, unsigned char *bgI, 
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



void vtkTextureFont::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    /*  TODO - do print
    os << indent << "X Resolution: " << this->XResolution << "\n";
    os << indent << "Y Resolution: " << this->YResolution << "\n";
    */
}
