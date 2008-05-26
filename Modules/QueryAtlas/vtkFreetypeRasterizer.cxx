/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFreetypeRasterizer.cxx,v $
  Date:      $Date: 2006/05/26 19:59:43 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
/*
Authors: Michael McKenna, David Small, Steve Pieper.
    Small Design Firm, Inc.,
    in association with Isomics, Inc.,
    and Partners Health Care.  MA, USA.
    June, 2004

===========================================================*/


#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include "vtkFreetypeRasterizer.h"

//#include <ft2build.h> // can't include that directly
#include "vtk_freetype.h"
//#include <FTFont.h>

#include FT_FREETYPE_H


// NOTE - we could make a list of cached rasterizers, as we do with TextureFont.
//   A cashe could be a bit more efficient potentially, but since we cache the Fonts,
//   it wouldn't help a bunch to also cache this.

// If we start to cache this, TextureFont::Delete should not delete the Rasterizer.


// overall freetype library
static FT_Library sLibrary = NULL;

// handle to face object
static FT_Face sFace = NULL;
static int sError = 0;

int vtkFreetypeRasterizer::sDefaultRasterizerResolution = 48;


vtkCxxRevisionMacro(vtkFreetypeRasterizer, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkFreetypeRasterizer);



vtkFreetypeRasterizer::vtkFreetypeRasterizer()
{
    this->Resolution = vtkFreetypeRasterizer::sDefaultRasterizerResolution;
    this->mIsKerning = 0;
    this->mError = 0;
    this->mRasterizerCharacters = vtkCollection::New();
}


int vtkFreetypeRasterizer::LoadFont(char *fontname)
{
    int i;
    unsigned int bmapsize;

    // check if freetype library has been initialized
    if (sLibrary == NULL) {
        int sError = FT_Init_FreeType(&sLibrary);
        if (sError) {
            vtkErrorMacro("vtkFreetypeRasterizer: ERROR: during freetype library initialization.\n");
            this->mError = 1;
            return this->mError;
        }
    }
    // check to see if face exists
    sError = FT_New_Face(sLibrary, fontname, 0, &sFace);

    if (sError == FT_Err_Unknown_File_Format) {
        vtkErrorMacro("vtkFreetypeRasterizer: ERROR: the font file could be opened and read, but it appears that its font format is unsupported, file: " << 
            fontname);
        this->mError = 1;
        return this->mError;
    }
    else if (sError) {
        vtkErrorMacro("vtkFreetypeRasterizer: file not found: " <<  fontname);
        this->mError = 1;
        return this->mError;
    }

    //printf("vtkFreetypeRasterizer: Loading FreeType %s ...\n", fontname);

    //set the size for our render
    sError = FT_Set_Char_Size(
        sFace,    // handle to face object          
        0,       // char_width in 1/64th of points 
        this->Resolution*64,   // char_height in 1/64th of points
        64,     // horizontal device resolution   
        64 );   // vertical device resolution     

    // v14 - use VTK's built in freetype lib - different #define - lowercase
    //sError = FT_Select_Charmap(sFace, FT_ENCODING_APPLE_ROMAN);
    sError = FT_Select_Charmap(sFace, ft_encoding_apple_roman);
    
    if (sError) {
        vtkErrorMacro("vtkFreetypeRasterizer: Adobe encoding not found for: " << fontname);
    }

    FT_GlyphSlot  slot = sFace->glyph;  // a small shortcut
    FT_UInt  glyph_index;

    int numChars = VTK_FONT_CHAR_NUM;

    //vtkErrorMacro("vtkFreetypeRasterizer: Font Data: \n numChars [%d], mNumBits[%d], mMboxX[%d], Mbox_y[%d], Base_ht[%d]\n\n",
    //       numChars, mNumBits(font), mMboxX(font), Mbox_y(font), Base_ht(font));

    for (i = 0; i < numChars; i++) {
        vtkRasterizerCharacter *ch = vtkRasterizerCharacter::New();
        this->mRasterizerCharacters->AddItem(ch);

        // retrieve glyph index from character code
        glyph_index = FT_Get_Char_Index( sFace, i + VTK_FONT_CHAR_OFFSET);

        // load glyph image into the slot (erase previous one)
        sError = FT_Load_Glyph(sFace, glyph_index, FT_LOAD_DEFAULT);
        if (sError) continue;  // ignore errors

        // convert to an anti-aliased bitmap
        // v14 - use VTK's built in freetype lib - different #define - lowercase
        sError = FT_Render_Glyph( sFace->glyph, ft_render_mode_normal );
        if (sError) continue;

        ch->mAsciiValue = i + VTK_FONT_CHAR_OFFSET;
        ch->mHeight = slot->bitmap.rows;
        ch->mWidth = slot->bitmap.width;

        // NOTE - the bit shift doesn't result in any data truncation - advance.x is a mult of 64 - alwats true?
        ch->mAdvanceWidth = slot->advance.x >> 6;
        //printf("%c: %f  ", i, ch->mAdvanceWidth);

        ch->mTopExtent = slot->bitmap_top;
        ch->mLeftExtent = slot->bitmap_left;

        
        //printf("vtkFreetypeRasterizer: loading %c: wd %d mHeight %d advance %d\n", Ascii_value(this, i), Char_ht(this, i), Char_wd(this, i), Set_width(this, i));

        bmapsize = ch->mHeight * ch->mWidth;

        if ((ch->mBitmap = (unsigned char *) malloc (sizeof (unsigned char) * bmapsize)) == NULL) {
            vtkErrorMacro ("vtkFreetypeRasterizer: malloc failed, char bitmap " << i);
            this->mError = 1;
            return this->mError;
        }

        memcpy(ch->mBitmap, slot->bitmap.buffer, bmapsize);
    }

    // Do all the Kerning stuff
    this->mIsKerning = 1;

    for (i = 0; i < VTK_FONT_CHAR_NUM; i++) {
        for (int j = 0; j < VTK_FONT_CHAR_NUM; j++) {
            this->mKernPairs[i][j] = 0;
        }
    }    

    FT_Bool use_kerning = (FT_Bool)FT_HAS_KERNING(sFace);
    FT_UInt glyph_index_r, glyph_index_l;

    for (i = 0; i < VTK_FONT_CHAR_NUM; i++) {
        // convert character code to glyph index    
        glyph_index_r = FT_Get_Char_Index( sFace, i );

        for (int j = 0; j < VTK_FONT_CHAR_NUM; j++) {
            // convert character code to glyph index    
            glyph_index_l = FT_Get_Char_Index( sFace, j );

            // retrieve kerning distance and move pen position    
            if (use_kerning  &&  glyph_index_l  &&  glyph_index_r) {
                FT_Vector  delta;

                FT_Get_Kerning( sFace, glyph_index_l, glyph_index_r,
                    //FT_KERNING_DEFAULT, &delta );
                    ft_kerning_default, &delta );

                //mKernPairs[i][j] = (int)(delta.x * 1000. / (vtkFloatingPointType)Resolution) >> 6;
                // NOTE - the bit shift doesn't result in any data truncation - delta.x is a mult of 64
                this->mKernPairs[i][j] = delta.x >> 6;
                //if (delta.x != 0) printf("%c-%c:%d  ", i, j, mKernPairs[i][j]);
            }
        }
        //printf("\nChar %c:", i);
    }
    return this->mError;
}


vtkFreetypeRasterizer::~vtkFreetypeRasterizer() {
    //remove & delete all elements in mFontChars vector
    for (int i = 0; i < this->mRasterizerCharacters->GetNumberOfItems(); i++) {
        vtkRasterizerCharacter *ch = (vtkRasterizerCharacter *)this->mRasterizerCharacters->GetItemAsObject(i);
        ch->Delete();
    }
    // TODO - any other deletes?
}


// the master resolution, that a new reader will to use rasterize
void vtkFreetypeRasterizer::SetDefaultRasterizerResolution(int res)
{
    sDefaultRasterizerResolution = res;
}


// the master resolution, that a new reader will to use rasterize
int vtkFreetypeRasterizer::GetDefaultRasterizerResolution()
{
    return sDefaultRasterizerResolution;
}


vtkFloatingPointType vtkFreetypeRasterizer::GetAdvanceWidthKerned(int c, int c2)
{
    vtkFloatingPointType sw = (vtkFloatingPointType) ((vtkRasterizerCharacter *)this->mRasterizerCharacters->GetItemAsObject(c - VTK_FONT_CHAR_OFFSET))->mAdvanceWidth;

    if (this->mIsKerning) {
        if ((c < VTK_FONT_CHAR_NUM) && (c2 < VTK_FONT_CHAR_NUM))
            //sw += mKernPairs[c][c2] * (vtkFloatingPointType)Resolution / 1000.0f;    
            sw += (vtkFloatingPointType)this->mKernPairs[c][c2];    
    }
    return (sw);
}


vtkFloatingPointType vtkFreetypeRasterizer::GetRasterizerScale() {
    return (1.0f/((vtkFloatingPointType)this->Resolution));
}


void vtkFreetypeRasterizer::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    /*  TODO - do print
    os << indent << "X Resolution: " << this->XResolution << "\n";
    os << indent << "Y Resolution: " << this->YResolution << "\n";
    ...
    */
}
