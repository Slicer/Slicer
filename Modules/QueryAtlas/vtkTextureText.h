/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTextureText.h,v $
  Date:      $Date: 2006/05/26 19:59:45 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
/*===========================================================

Authors: Michael McKenna, David Small, Steve Pieper.
    Small Design Firm, Inc.,
    in association with Isomics, Inc.,
    and Partners Health Care.  MA, USA.
    June, 2004

===========================================================*/


#ifndef __vtkTextureText_h
#define __vtkTextureText_h

#include "vtkObject.h"
#include "vtkFreetypeRasterizer.h"
#include "vtkTextureFont.h"
#include <vtkQueryAtlasConfigure.h>
#include <vtkFollower.h>

#include "vtkTextureTextPolys.h"
#include "vtkFontParameters.h"

#include <vector>
#include <vector>


class vtkRenderer;


// Alignment - must be one of the following 
#define VTK_TEXT_ALIGNMENT_FLUSH_LEFT       (0)
#define VTK_TEXT_ALIGNMENT_FLUSH_RIGHT      (1)
#define VTK_TEXT_ALIGNMENT_JUSTIFIED        (2)
#define VTK_TEXT_ALIGNMENT_CENTERED         (3)
#define VTK_TEXT_ALIGNMENT_MONOSPACE        (4)

#define VTK_TEXT_NLCHAR      'n'
#define VTK_TEXT_SPACECHAR   'i'

#define VTK_TEXT_BOTTOM_LEFT   (0)
#define VTK_TEXT_BOTTOM_RIGHT  (1)
#define VTK_TEXT_TOP_RIGHT     (2)
#define VTK_TEXT_TOP_LEFT      (3)

static vtkFloatingPointType sAutoLeading = 1.0;  // 120 %  ?

// characterPosition - small utility storage class

//BTX
class characterPosition
{
public:
    characterPosition(unsigned char c);
    ~characterPosition() {};

    // lower left x, y, z of the mbox
    Vector3D<vtkFloatingPointType> mbox_pos;
    char character;
};
//ETX


class VTK_QUERYATLAS_EXPORT vtkTextureText : public vtkObject
{
public:
     void PrintSelf(ostream& os, vtkIndent indent);
    vtkTypeRevisionMacro(vtkTextureText,vtkObject);

    static vtkTextureText *New();

    int CreateTextureText();

    int mCharCount;

//BTX
    Vector3D<vtkFloatingPointType> *mCharGsetCoords;
    Vector2D<vtkFloatingPointType> *mTexCoords;

    vtkFloatingPointType* (*baselineFunc) (vtkFloatingPointType *vec, void *args);
    void *baselineArgs;

    std::vector<characterPosition *>mCharacterPositions;
//ETX

    void buildQuads();
    void UpdateTexture();

    vtkGetObjectMacro(Texture, vtkTexture);
    vtkGetObjectMacro(Follower, vtkFollower);

    vtkGetObjectMacro(TextureTextPolys, vtkTextureTextPolys);
    // this method also returns TextureTextPolys, but using more standard VTK naming
    vtkTextureTextPolys *GetPolyDataSource();

    vtkGetObjectMacro(TextureFont, vtkTextureFont);
    vtkSetObjectMacro(TextureFont, vtkTextureFont);

    vtkGetObjectMacro(FontParameters, vtkFontParameters);

    vtkGetStringMacro(CharacterArray);


    // dimensions for wrapped text    
    // The text goes to the next line when the x position of the next word goes outside the 
    // box width. When the height is exceeded, the text moves to new column.
    vtkGetMacro(BoxWidth, vtkFloatingPointType);
    vtkSetMacro(BoxWidth, vtkFloatingPointType);
    vtkGetMacro(BoxHeight, vtkFloatingPointType);
    vtkSetMacro(BoxHeight, vtkFloatingPointType);

    void SetBoxSize(vtkFloatingPointType w, vtkFloatingPointType h);

    void SetWrapped(bool wrpd);
    
    vtkGetMacro(Wrapped, bool);
    
    void WrappedOn();
    void WrappedOff();

    vtkGetMacro(Leading, vtkFloatingPointType);
    vtkSetMacro(Leading, vtkFloatingPointType);

    vtkGetMacro(Indent, vtkFloatingPointType);
    vtkSetMacro(Indent, vtkFloatingPointType);

    vtkGetMacro(CharacterSpace, vtkFloatingPointType);
    vtkSetMacro(CharacterSpace, vtkFloatingPointType);

    vtkGetMacro(Alignment, short);
    vtkSetMacro(Alignment, short);

    // set/replace an existing string with <string> 
    void SetText(char *string);
    vtkGetStringMacro(Text);

    void SetDefaultAlignment(short alignment);
    short GetDefaultAlignment();

    vtkGetMacro(Initialized, int);
    vtkGetMacro(Error, int);

//BTX
    void SetBaselineFunction(vtkFloatingPointType* (*f) (vtkFloatingPointType *vec, void *args), void *args);
    void UpdateBaseline();
    void ApplyBaselineFunction();

    void CalculateRangeBox(Vector3D<vtkFloatingPointType> *min, Vector3D<vtkFloatingPointType> *max, int start, int end);
//ETX

    //static vtkFloatingPointType CalculateLineLength(char *string, char *fontname);

    // v12 - this doesn't seem to be used internally, but can be a useful utility...
    static vtkFloatingPointType GetAdvanceWidth(char *string, char *fontname, bool space);
    int CalculateNumberLines();

    void AddPositionOffset(vtkFloatingPointType pos[3]);
    void AddPositionOffset(vtkFloatingPointType x, vtkFloatingPointType y, vtkFloatingPointType z);

    void SetPositionOffset(vtkFloatingPointType pos[3]);
    void SetPositionOffset(vtkFloatingPointType x, vtkFloatingPointType y, vtkFloatingPointType z);
    void GetPositionOffset(vtkFloatingPointType pos[3]);

    void DoPositionOffset();

    void Modified();

    void SetDim(int d);
    

protected:
    vtkTextureText();
    ~vtkTextureText(); 

    int Initialized;
    int Error;


    // space between lines
    vtkFloatingPointType                   Leading;
    vtkFloatingPointType                    Indent;
    int                     mLineCount;
    // extra space between characters
    vtkFloatingPointType                   CharacterSpace;

    bool                    Wrapped;
    vtkFloatingPointType                   BoxWidth;
    vtkFloatingPointType                   BoxHeight;

    // TODO MED - v12 - Alignment isn't working right for every line, 
    //   w/ VTK_TEXT_ALIGNMENT_JUSTIFIED, RIGHT, CENTERED
    //  Right, Centered seem to work when charSpace and indent are off.  Justified is still a bit off.
    // VTK_TEXT_ALIGNMENT_MONOSPACE - seems to correctly monospace, but too spread out, and w/ multiple columns, they're overlapping w/o wrapping soon enough
    //   especially messed up if SetCharacterSpace() if used - also when indent used
    // Centered needs a linefeed at the end of the text to work right.
    short                   Alignment;
    static short            sDefaultTextAlignment;
    vtkFloatingPointType                   word_space_when_justified;

    char                    *CharacterArray;
    char *Text;
//BTX
    Vector3D<vtkFloatingPointType> PositionOffset;
//ETX
    vtkFontParameters *FontParameters;
    vtkTexture *Texture;
    vtkFollower *Follower;
    vtkTextureFont *TextureFont;
    vtkTextureTextPolys *TextureTextPolys;

    vtkFloatingPointType TextOpacity;
    int Dim;

    void deleteAllChars();

    // low level
    vtkFloatingPointType getLinePos(unsigned int _start);
    vtkFloatingPointType getLineLength(unsigned int _start);
    int getCharLine(unsigned int _start);
    vtkFloatingPointType getLineWordCount(unsigned int _start, vtkFloatingPointType ind = 0);

//BTX
    void  setCharCoords(Vector3D<vtkFloatingPointType> *char_gset_coords, Vector2D<vtkFloatingPointType> *tex_coords, 
        vtkTextureFont *textureFont, int the_char, Vector3D<vtkFloatingPointType> text_pos);
//ETX

};

#endif
