/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTextureText.cxx,v $
  Date:      $Date: 2006/05/26 19:59:45 $
  Version:   $Revision: 1.5 $

=========================================================================auto=*/
/*
Authors: Michael McKenna, David Small, Steve Pieper.
    Small Design Firm, Inc.,
    in association with Isomics, Inc.,
    and Partners Health Care.  MA, USA.
    June, 2004

===========================================================*/


#include "vtkTextureText.h" 
#include "vtkTextureFont.h"

#include "vtkActor.h"
#include "vtkFollower.h"
#include "vtkTexture.h"
#include "vtkImageData.h"
#include "vtkUnsignedCharArray.h"
#include "vtkTextureTextPolys.h"
#include "vtkPolyDataMapper.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkFloatArray.h"
#include "vtkRenderer.h"
#include "vtkPlaneSource.h"
#include "vtkProperty.h"
#include "vtkRectangle.h"


short vtkTextureText::sDefaultTextAlignment = VTK_TEXT_ALIGNMENT_FLUSH_LEFT;



static vtkFloatingPointType DefaultTextOpacity = 0.99;


// characterPosition - small utility storage class

characterPosition::characterPosition(unsigned char c)
{
  character = c;
  mbox_pos.set(0.0, 0.0, 0.0);
}



vtkCxxRevisionMacro(vtkTextureText, "$Revision: 1.5 $");
vtkStandardNewMacro(vtkTextureText);



vtkTextureText::vtkTextureText() {
    this->Error = 0;

    this->TextureFont = NULL;
    this->FontParameters = vtkFontParameters::New();
    this->FontParameters->TextureText = this;

    this->mCharCount = 0;
    this->Indent = 0;
    this->mCharGsetCoords = NULL;
    this->mTexCoords = NULL;
    this->TextureTextPolys = NULL;
    this->mLineCount = 1;
    this->Leading = sAutoLeading;
    this->CharacterSpace = 0;
    this->baselineFunc = NULL;
    this->baselineArgs = NULL;
    this->Texture = NULL;
    this->Text = NULL;
    this->Follower = NULL;

    this->Alignment = sDefaultTextAlignment;
    this->Wrapped = 0;
    this->BoxHeight = 10;
    this->BoxWidth = 10;
    this->word_space_when_justified = 0.0f;
    this->PositionOffset.set(0, 0, 0);

    this->TextOpacity = DefaultTextOpacity;
    this->Dim = 0;

    buildQuads();

    this->Initialized = 0;
}


vtkTextureText::~vtkTextureText()
{
    // delete char list
    deleteAllChars();

    // TODO MED - what about removing these from objects that have them as a data member - eg follower in a renderer
    // texture need to removed from follower or anything?
    
    if (this->Texture != NULL) this->Texture->Delete();
    if (this->Follower != NULL) this->Follower->Delete();
    if (this->TextureTextPolys != NULL) this->TextureTextPolys->Delete();

    // TODO - delete the geometry - mCharGsetCoords, mTexCoords, this->CharacterArray, polyMapper, etc.
}


int vtkTextureText::CreateTextureText()
{
    this->TextureFont = this->FontParameters->RequestTextureFont();

    if (this->TextureFont == NULL) {
        vtkErrorMacro( "vtkTextureText::CreateTextureText: Couldn't create font.\n");
        this->Error = 1;

        // NOTE - bad empty quads cause a render loss, if the erronous TextureText is added to the Renderer
        //   So we stop the first Execute of the polys, then we're OK.   
        // BUT this is a bit of a kludge solution.
        // But - we also test for:
        //      if (this->mTextureText->GetError()) return;
        //   in the Poly's Execute, which also prevents the error.

        this->TextureTextPolys->SetTextureText(NULL);

        return this->Error;
    }

    this->Initialized = 1;
    Modified();

    return this->Error;
}


void vtkTextureText::SetBoxSize(vtkFloatingPointType w, vtkFloatingPointType h) {
    this->BoxWidth = w;
    this->BoxHeight = h;
    Modified();
}


void vtkTextureText::SetWrapped(bool wrpd)
{
    if (wrpd) WrappedOn();
    else WrappedOff();
}


void vtkTextureText::WrappedOn ()
{
    Wrapped = 1;
    if (BoxWidth <= 0.0f)  
        vtkErrorMacro( "vtkTextureText::WrappedOn(): Warning: BoxWidth = " << BoxWidth << " must be greater than 0\n");

    Modified(); 
}


void vtkTextureText::WrappedOff ()
{
    Wrapped = 0;
    Modified(); 
}


// set the actual text string
void vtkTextureText::SetText(char *string)
{
    unsigned char c;
    char *str;

    mCharCount=0;
    mLineCount=1;

    deleteAllChars();

    // v47 - a NULL string would crash w/o protection
    if (string == NULL) {
        this->Text = NULL;
        Modified();
        return;
    }

    this->Text = strdup(string);

    // make a charlist - divides the word into separate characters 
    // and makes a list of them
    // i is the index into the original string
    int i = 0;
    for(str = this->Text; str[i] != '\0'; i++) { 
        c = (int)str[i];
        if (c > 32 && c <= 127) {
            // normal character
            characterPosition *cl = new characterPosition(c);
            mCharacterPositions.push_back(cl);
            mCharCount++;
        }
        else if (c > 127 /*&& c <= 255*/) {
            // printf("Special char %d\n", c);
            characterPosition *cl = new characterPosition(c);
            mCharacterPositions.push_back(cl);
            mCharCount++;
        }
        else if (c == 10  ||  c == 13) {
            characterPosition *cl = new characterPosition(c);
            mCharacterPositions.push_back(cl);
            mLineCount++;
        }
        else if (c == ' ') {
            characterPosition *cl = new characterPosition(c);
            mCharacterPositions.push_back(cl);
        }
        else if (c == '\t') {
            characterPosition *cl = new characterPosition(' ');
            mCharacterPositions.push_back(cl);
        }
        else {
            vtkErrorMacro("vtkTextureText::SetText: Unknown character " << c);
        }
    }

    mCharGsetCoords = (Vector3D<vtkFloatingPointType> *) malloc (sizeof(Vector3D<vtkFloatingPointType>) * 4 * mCharCount);
    mTexCoords = (Vector2D<vtkFloatingPointType> *) malloc (sizeof(Vector2D<vtkFloatingPointType>) * 4 * mCharCount);
    this->CharacterArray = (char *)   malloc (sizeof(char) * (mCharCount+1));
    this->CharacterArray[mCharCount] = '\0';

    Modified();
}


void vtkTextureText::SetDefaultAlignment(short new_align)
{
    if(new_align > 5)
    {
        vtkErrorMacro("vtkTextureText::SetDefaultAlignment: invalid alignment " << new_align << " specified\n");
    } else
    {
        sDefaultTextAlignment = new_align;
    }
}


short vtkTextureText::GetDefaultAlignment()
{
    return sDefaultTextAlignment;
}


void vtkTextureText::SetBaselineFunction(vtkFloatingPointType* (*f)(vtkFloatingPointType *vec, void *args), void *args) {
    baselineArgs = args;
    baselineFunc = f;
    ApplyBaselineFunction();
}


// Call this is you have changed baseline function arguments, etc.
void vtkTextureText::UpdateBaseline()
{
    Modified();
}


void vtkTextureText::ApplyBaselineFunction()
{
    if (baselineFunc != NULL) {
        Vector3D<vtkFloatingPointType> *vertptr;

        vertptr = mCharGsetCoords;

        for (int i = 0; i < mCharCount; i++) {
            // what about the normal?

            (baselineFunc) (vertptr[0].values, baselineArgs);
            (baselineFunc) (vertptr[1].values, baselineArgs);
            (baselineFunc) (vertptr[2].values, baselineArgs);
            (baselineFunc) (vertptr[3].values, baselineArgs);

            vertptr += 4;    
        }
    }

    if (this->TextureTextPolys != NULL) this->TextureTextPolys->Modified();
}


// NOTE - this method used to be Format()
void vtkTextureText::Modified()
{
    int c, c2;
    Vector3D<vtkFloatingPointType> mbox_pos, tex_pos;
    int column = 0;
    int num_word, word_count, my_char_count;
    int use_indent = 0;

    if (!this->Initialized) return;

    // v47
    if (this->Text == NULL) return;
    if (strlen(this->Text) == 0) return;

    vtkFreetypeRasterizer *font;
    characterPosition *my_char_list, *cl;

    //vtkTextureFont *textureFont_start = TextureFont;
    font = TextureFont->GetFreetypeRasterizer();
    if ( font == NULL )
    {   vtkErrorMacro ("vtkTextureText::Modified: null font\n");
        mbox_pos.set(0, 0, 0);

        // TODO - clean up to not make mbox_pos
        //return mbox_pos;
        return;
    }

    my_char_list = mCharacterPositions[0];
    my_char_count = 0;
    mbox_pos.set(0, 0, 0);
    mbox_pos[0] = getLinePos(0) + this->Indent;

    // TODO - should getLineWordCount really be a vtkFloatingPointType?!  should we round() ?
    num_word = (int)(getLineWordCount(0, this->Indent));
    word_count=0;

    for (unsigned int i = 0; i < mCharacterPositions.size(); i++) {
        cl = mCharacterPositions[i];
        c = (int) cl->character;
        if (i != mCharacterPositions.size()-1) c2 = (int) mCharacterPositions[i+1]->character;
        else c2 = -1;

        //vtkRasterizerCharacter *rch = (vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(c - VTK_FONT_CHAR_OFFSET);
        if ((c > 32) && ((c-VTK_FONT_CHAR_OFFSET) < (int)font->mRasterizerCharacters->GetNumberOfItems())) {
            tex_pos[0] = mbox_pos[0] + (vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(c - VTK_FONT_CHAR_OFFSET))->mLeftExtent *
                font->GetRasterizerScale();
            tex_pos[1] = mbox_pos[1] + (vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(c - VTK_FONT_CHAR_OFFSET))->GetBottomExtent() *
                font->GetRasterizerScale();
            tex_pos[2] = mbox_pos[2];

            setCharCoords(mCharGsetCoords+(4*my_char_count), mTexCoords+(4*my_char_count), 
                TextureFont, c, tex_pos);
            this->CharacterArray[my_char_count] = c;

            cl->mbox_pos = mbox_pos;
            if (this->Alignment == VTK_TEXT_ALIGNMENT_MONOSPACE)
                mbox_pos[0] += 1.0 + this->CharacterSpace;
            else
                mbox_pos[0] += (font->GetAdvanceWidthKerned(c, c2) * font->GetRasterizerScale()) + this->CharacterSpace;
            my_char_count++;
            use_indent = 0;
        }      
        else if ((c == 10) || (c==13)) {
            tex_pos[0] = mbox_pos[0] + (vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_NLCHAR - VTK_FONT_CHAR_OFFSET))->mLeftExtent *
                font->GetRasterizerScale();
            tex_pos[1] = mbox_pos[1] + (vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_NLCHAR - VTK_FONT_CHAR_OFFSET))->GetBottomExtent() *
                font->GetRasterizerScale();
            tex_pos[2] = mbox_pos[2];
            cl->mbox_pos = mbox_pos;

            word_count++;
            use_indent = 1;
        }
        else if (c == ' ') {
            if(this->Alignment == VTK_TEXT_ALIGNMENT_JUSTIFIED) {
                tex_pos[0] = mbox_pos[0] + (vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET))->mLeftExtent *
                    font->GetRasterizerScale();
                tex_pos[1] = mbox_pos[1] + (vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET))->GetBottomExtent() *
                    font->GetRasterizerScale();
                tex_pos[2] = mbox_pos[2];
                cl->mbox_pos = mbox_pos;
                mbox_pos[0] += word_space_when_justified;
            }
            else if (this->Alignment == VTK_TEXT_ALIGNMENT_MONOSPACE)
                mbox_pos[0] += 1.0 + this->CharacterSpace;
            else {
                tex_pos[0] = mbox_pos[0] + (vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET))->mLeftExtent *
                    font->GetRasterizerScale();
                tex_pos[1] = mbox_pos[1] + (vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET))->GetBottomExtent() *
                    font->GetRasterizerScale();
                tex_pos[2] = mbox_pos[2];
                cl->mbox_pos = mbox_pos;
                mbox_pos[0] += ((vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET))->mAdvanceWidth * 
                    font->GetRasterizerScale()) + this->CharacterSpace;
            }
            word_count++;
        }
        else if (c == '\t') { 
            // TODO MED - sorry, tab is just a space now.
            if(this->Alignment == VTK_TEXT_ALIGNMENT_JUSTIFIED) {
                tex_pos[0] = mbox_pos[0] + (vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET))->mLeftExtent *
                    font->GetRasterizerScale();
                tex_pos[1] = mbox_pos[1] + (vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET))->GetBottomExtent() *
                    font->GetRasterizerScale();
                tex_pos[2] = mbox_pos[2];
                cl->mbox_pos = mbox_pos;
                mbox_pos[0] += word_space_when_justified;
            }
            else if (this->Alignment == VTK_TEXT_ALIGNMENT_MONOSPACE)
                mbox_pos[0] += 1.0 + this->CharacterSpace;
            else {
                tex_pos[0] = mbox_pos[0] + (vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET))->mLeftExtent * 
                    font->GetRasterizerScale();
                tex_pos[1] = mbox_pos[1] + (vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET))->GetBottomExtent() * 
                    font->GetRasterizerScale();
                tex_pos[2] = mbox_pos[2];
                cl->mbox_pos = mbox_pos;
                mbox_pos[1] += ((vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET))->mAdvanceWidth * 
                    font->GetRasterizerScale()) + this->CharacterSpace;
            }
        }
        else if (c == 0) {
            ;
        }
        else 
            vtkErrorMacro("vtkTextureText::Modified: Unknown character. " << c );

        if (num_word <= word_count) {
            mbox_pos[0] = getLinePos(i+1) + column*(BoxWidth+1);
            mbox_pos[1] -= this->Leading;
            mbox_pos[2] -= 0;

            if (Wrapped) {
                if (-mbox_pos[1] > this->BoxHeight) {
                    column++;
                    mbox_pos[0] = getLinePos(i+1) + column*(this->BoxWidth+1);
                    mbox_pos[1] = 0;
                    //printf("Wrapped column %d,  mbox_pos ", column);  mbox_pos.print();
                }
            }

            if (use_indent) {
                mbox_pos[0] += this->Indent;
                num_word = (int)(getLineWordCount(i+1, this->Indent));
            }
            else
                num_word = (int)(getLineWordCount(i+1));

            word_count = 0;
        }
    }

    if (baselineFunc != NULL) ApplyBaselineFunction();

    if (this->TextureTextPolys != NULL) this->TextureTextPolys->Modified();

    // need to make sure our teture map is still appropriate (after font or blur change in particular)
    UpdateTexture();

    DoPositionOffset();

    // TODO - OK to not return mbox, right?
    //return (mbox_pos);
}


// TODO - could add CalculateLineLength back later - will need more porting, esp. for FontManager
/*
vtkFloatingPointType vtkTextureText::CalculateLineLength(char *string, char *fontname)
{
    vtkFloatingPointType line_length=0.0;
    unsigned int count;
    int c, c2;
    vtkFreetypeRasterizer *font;
    vtkTextureFont *textureFont;

    if (((textureFont = vtkTextureFont::GetTextureFont(fontname, 0)) == NULL)) {
        fprintf(stderr, "vtkTextureText::CalculateLineLength: Couldn't find font (%s).\n", fontname);
        return(-1);
    }

    font = textureFont->GetFreetypeRasterizer();

    for (count = 0; count < strlen(string); count++) {
        c = string[count];
        if ((count+1) < strlen(string))
            c2 = string[count+1];
        else
            c2 = -1;

        if ((c > 32) && ((c-VTK_FONT_CHAR_OFFSET) < (int)font->mRasterizerCharacters->GetNumberOfItems())) {
            line_length += font->GetAdvanceWidthKerned(c, c2) * font->GetRasterizerScale();
        }
        else if ((c == 10) || (c == 13)) {
            break;
        }
        else if (c == ' ') {
            line_length += (vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET))->mAdvanceWidth * 
                font->GetRasterizerScale();
        }
        else if (c == '\t') {
            line_length += (vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET))->mAdvanceWidth * 
                font->GetRasterizerScale();
        }
        else 
            fprintf(stderr, "pffTextLineWordCount: Unknown character. (%c = %d).\n", c, c);
    }

    return line_length;
}
*/


// TODO - CalculateNumberLines doesn't seem to be used in the lib, but it could be useful to an app.
//  - shouldn't this result just be saved from Modified(), instead of recalculated?

int vtkTextureText::CalculateNumberLines()
{    
    int num_lines = 1;
    Vector3D<vtkFloatingPointType> *vertptr;

    vertptr = mCharGsetCoords; 
    vtkFloatingPointType prev = -10.0;
    for (int i = 0; i < mCharCount; i++) {
        if (vertptr[0].values[0] < prev)
            num_lines++, prev = -10;
        else
            prev = vertptr[0].values[0];

        vertptr += 4;
    }
    return num_lines;
}


void extendRange(Vector3D<vtkFloatingPointType> *min, Vector3D<vtkFloatingPointType> *max, Vector3D<vtkFloatingPointType> val) {
    for (int i = 0; i < 3; i++) {
        if (min->values[i] > val[i])
            min->values[i] = val[i];
        if (max->values[i] < val[i])
            max->values[i] = val[i];
    }
}


void vtkTextureText::CalculateRangeBox(Vector3D<vtkFloatingPointType> *min, Vector3D<vtkFloatingPointType> *max, int start, int end)
{
    min->set(100000, 100000, 100000);
    max->set(-100000, -100000, -100000);

    if (end < mCharCount) {
        for (int j = 0; j < 4; j++) {
            extendRange(min, max, mCharGsetCoords[4*start+j]);
            extendRange(min, max, mCharGsetCoords[4*end+j]);
        }
    }
}


// TODO - could be added back - can be a useful Calculate method
vtkFloatingPointType vtkTextureText::GetAdvanceWidth(char *string, char *fontname, bool space)
{
  return 0.0;
}

// TODO - could be added back - can be a useful Calculate method
/*
vtkFloatingPointType vtkTextureText::GetAdvanceWidth(char *string, char *fontname, bool space)
{
    vtkFloatingPointType xpos = 0.0;
    vtkTextureFont *textureFont;

    if((textureFont = vtkTextureFont::GetTextureFont(fontname)) == NULL) {
        fprintf(stderr, "vtkTextureText::GetAdvanceWidth: Couldn't find font (%s).\n", fontname);
        return(-1);
    }

    //int d = 1;
    //if (d) printf("vtkTextureText::GetAdvanceWidth: string: %s\nstrlen: %d\n", string, strlen(string));

    for (unsigned int i = 0; i < strlen(string); i++)
    {
        //if (d) printf("%d, '%c'  ", string[i], string[i]);
        if (string[i] == ' ') {
            xpos += textureFont->GetFreetypeRasterizer()->mRasterizerCharacters[VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET]->mAdvanceWidth *
                textureFont->GetFreetypeRasterizer()->GetRasterizerScale();
        }
        else {
            // v 1.73 - check for negative font index values
            if (string[i] - VTK_FONT_CHAR_OFFSET >= 0) {
                xpos += textureFont->GetFreetypeRasterizer()->mRasterizerCharacters[string[i] - VTK_FONT_CHAR_OFFSET]->mAdvanceWidth *
                    textureFont->GetFreetypeRasterizer()->GetRasterizerScale();
            }
        }
    }

    if (space) {
        xpos += textureFont->GetFreetypeRasterizer()->mRasterizerCharacters[VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET]->mAdvanceWidth *
            textureFont->GetFreetypeRasterizer()->GetRasterizerScale();
    }

    //if (d) printf("\n");

    return (xpos);
}
*/



// Low level
vtkFloatingPointType vtkTextureText::getLinePos(unsigned int _start)
{
    vtkFloatingPointType line_pos;
    vtkFloatingPointType line_length = 0, line_word_count;
    vtkFloatingPointType anchor_x = 0;

    if(this->Alignment != VTK_TEXT_ALIGNMENT_FLUSH_LEFT)
        line_length = getLineLength(_start);

    switch (this->Alignment)
    {
    case VTK_TEXT_ALIGNMENT_FLUSH_LEFT:
    case VTK_TEXT_ALIGNMENT_MONOSPACE:
        line_pos = anchor_x;
        break;

    case VTK_TEXT_ALIGNMENT_FLUSH_RIGHT:
        if(Wrapped) // if wrapped, the left edge becomes an anchor
            anchor_x += this->BoxWidth;
        line_pos = anchor_x - line_length;
        break;

    case VTK_TEXT_ALIGNMENT_JUSTIFIED:
        // this justification method only changes the word spacing
        line_pos = 0;
        if(Wrapped) {
            line_word_count = getLineWordCount(_start);
            if(line_word_count!=1)
                word_space_when_justified = (this->BoxWidth - line_length) / (line_word_count-1);
            else
                word_space_when_justified = 0.0f;
        }
        else {
            line_pos = anchor_x;
            fprintf(stderr, "Warning: vtkTextureText::getLinePos: couldn't justify.\n");
            fprintf(stderr, "         Not a wrapped text. Flush left is used instead.\n");
        }
        break;

    case VTK_TEXT_ALIGNMENT_CENTERED:
        if(Wrapped)
            anchor_x += (this->BoxWidth/2.0f);
        line_pos = anchor_x - (line_length / 2.0f);
        break;

    default:
        fprintf(stderr, "vtkTextureText::getLinePos: %d is not a type of alignment. Flush left is used instead.\n", this->Alignment);
        this->Alignment = VTK_TEXT_ALIGNMENT_FLUSH_LEFT;
        line_pos = 0;
        break;
    }
    return line_pos;
}


// Low level
vtkFloatingPointType vtkTextureText::getLineLength(unsigned int _start)
{
    vtkFloatingPointType line_length=0.0f, temp_length=0.0f;
    vtkFloatingPointType justified_line_length=0.0f;
    int c, c2;
    vtkFloatingPointType right_edge=0.0f;
    characterPosition *cl = NULL;
    vtkFreetypeRasterizer *font;

    font = TextureFont->GetFreetypeRasterizer();

    if (Wrapped) right_edge = this->BoxWidth;

    for(unsigned int i = _start; i < mCharacterPositions.size(); i++) {
        cl = mCharacterPositions[i];
        c = (int) cl->character;
        if (i != mCharacterPositions.size()-1) c2 = (int) mCharacterPositions[i+1]->character;
        else c2 = -1;

        if ((c > 32) && ((c-VTK_FONT_CHAR_OFFSET) < (int)font->mRasterizerCharacters->GetNumberOfItems())) {
            temp_length += font->GetAdvanceWidthKerned(c, c2) * font->GetRasterizerScale();
            if (Wrapped && temp_length >= right_edge)
                break;
        }
        else if ((c == 10) || (c == 13)) {
            line_length = temp_length;
            break;
        }
        else if (c == ' ') {
            // if justified, keep track of the length without spaces
            if(Wrapped && this->Alignment == VTK_TEXT_ALIGNMENT_JUSTIFIED)
                justified_line_length = temp_length;

            // count the length with spaces
            temp_length += (vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET))->mAdvanceWidth * 
                font->GetRasterizerScale();
            line_length = temp_length;
        }
        else if (c == '\t') {
            // if justified, keep track of the length without spaces
            if(Wrapped && this->Alignment == VTK_TEXT_ALIGNMENT_JUSTIFIED)
                justified_line_length = temp_length;

            temp_length += ((vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET))->mAdvanceWidth * 
                font->GetRasterizerScale()) + this->CharacterSpace;
            line_length = temp_length;
        }
        else {
            //fprintf(stderr, "vtkTextureText::getLineLength: Unknown character. (%c = %d).\n", c, c);
        }
    }

    if (cl == NULL) 
        line_length = temp_length;
    else if(Wrapped && this->Alignment == VTK_TEXT_ALIGNMENT_JUSTIFIED)
        line_length = justified_line_length;
    else
        line_length -= ((vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET))->mAdvanceWidth * 
        font->GetRasterizerScale()) + this->CharacterSpace;

    return line_length;
}


// getLineWordCount:
// This function counts the number of words from the current character[_start]
// to the last word in the line.

vtkFloatingPointType vtkTextureText::getLineWordCount(unsigned int _start, vtkFloatingPointType ind)
{
    vtkFloatingPointType line_length;
    int word_count;
    int c, c2;
    vtkFloatingPointType right_edge=0.0f;
    characterPosition *cl;
    vtkFreetypeRasterizer *font;  

    word_count = 0;
    font = TextureFont->GetFreetypeRasterizer();

    line_length = ind;

    if(Wrapped)
        right_edge = this->BoxWidth;

    unsigned int i;
    for(i = _start; i < mCharacterPositions.size(); i++) {
        cl = mCharacterPositions[i];
        c = (int) cl->character;
        if (i != mCharacterPositions.size()-1) c2 = (int) mCharacterPositions[i+1]->character;
        else c2 = -1;

        if ((c > 32) && ((c-VTK_FONT_CHAR_OFFSET) < (int)font->mRasterizerCharacters->GetNumberOfItems())) {
            line_length += (font->GetAdvanceWidthKerned(c, c2) * font->GetRasterizerScale()) + this->CharacterSpace;
            if (Wrapped && line_length >= right_edge)
                break;
        }
        else if ((c == 10) || (c == 13)) {
            word_count++;
            break;
        }
        else if (c == ' ') {
            word_count++;
            line_length += 
                ((vtkFloatingPointType)((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET))->mAdvanceWidth * 
                font->GetRasterizerScale()) + this->CharacterSpace;
        }
        else if (c == '\t') {
            word_count++;
            line_length += ((vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(VTK_TEXT_SPACECHAR - VTK_FONT_CHAR_OFFSET))->mAdvanceWidth * 
                font->GetRasterizerScale()) + this->CharacterSpace;
        }
        else if (c == 0) {
            ;
        }
        else 
            fprintf(stderr, "vtkTextureText::getLineWordCount: Unknown character. (%c = %d).\n", c, c);
    }

    if (i == mCharacterPositions.size()) word_count++;
    return word_count;
}



//
// VTK specific methods
//

void vtkTextureText::buildQuads()
{
    if (this->TextureTextPolys != NULL) {
        printf("vtkTextureText::buildQuads(): quads already built, returning.\n");
        return;
    }

    this->TextureTextPolys = vtkTextureTextPolys::New();
    this->TextureTextPolys->SetTextureText(this);

    vtkPolyDataMapper *planeMapper = vtkPolyDataMapper::New();
    planeMapper->SetInput(this->TextureTextPolys->GetOutput());

    this->Follower = vtkFollower::New();
    this->Follower->SetMapper(planeMapper);

    // make text 0.99 opaque by default, so that there are fewer depth/trans rendering artifacts
    // BUT - the render order must be done back-to-front in Z (use vtkSortCommand).
    this->Follower->GetProperty()->SetOpacity(TextOpacity);

    this->Texture = vtkTexture::New();
    this->Texture->InterpolateOn();   

    UpdateTexture();
}


void vtkTextureText::UpdateTexture() {
    if (this->TextureFont != NULL) {
        this->Texture->SetInput(this->TextureFont);
        GetFollower()->SetTexture(this->Texture);
    }
}


vtkTextureTextPolys *vtkTextureText::GetPolyDataSource() {
    return this->TextureTextPolys;
}


// used by SetText
void vtkTextureText::deleteAllChars()
{
    if (mCharGsetCoords != NULL) free(mCharGsetCoords);
    mCharGsetCoords = NULL;

    if (mTexCoords != NULL) free(mTexCoords);
    mTexCoords = NULL;

    if (this->Text != NULL) free(this->Text);
    this->Text = NULL;

    for (unsigned int i = 0; i < mCharacterPositions.size(); i++) 
        delete mCharacterPositions[i];
    mCharacterPositions.clear();

    mLineCount  = 1;
    mCharCount  = 0;
}


int vtkTextureText::getCharLine(unsigned int _start)
{
    int line_count=1;
    characterPosition *cl;
    vtkFloatingPointType right_edge=0.0f;

    if(Wrapped)
        right_edge = this->BoxWidth;

    for(unsigned int i = _start; i < mCharacterPositions.size(); i++) {
        cl = mCharacterPositions[i];
        if(cl->character == '\n' || (Wrapped && cl->mbox_pos[0] >= right_edge ))
            line_count++;
    }

    return line_count;
}


void vtkTextureText::setCharCoords(Vector3D<vtkFloatingPointType> *char_gset_coords, Vector2D<vtkFloatingPointType> *tex_coords, 
                            vtkTextureFont *textureFont, int the_char,  Vector3D<vtkFloatingPointType> text_pos)
{
    int c;
    vtkFloatingPointType wd, ht;
    vtkFreetypeRasterizer *font;
    Vector2D<vtkFloatingPointType> tex_pos, tex_size;
    vtkFloatingPointType xpos,ypos,zpos;

    font = textureFont->GetFreetypeRasterizer();  

    c = the_char - VTK_FONT_CHAR_OFFSET;
    if (c >= font->mRasterizerCharacters->GetNumberOfItems()) c = font->mRasterizerCharacters->GetNumberOfItems() - 1;

    wd = (vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(c))->mWidth * font->GetRasterizerScale();
    ht = (vtkFloatingPointType) ((vtkRasterizerCharacter *)font->mRasterizerCharacters->GetItemAsObject(c))->mHeight * font->GetRasterizerScale();

    xpos = text_pos[0];
    ypos = text_pos[1];
    zpos = text_pos[2];
    char_gset_coords[VTK_TEXT_BOTTOM_LEFT ].set( xpos,      ypos,      zpos);
    char_gset_coords[VTK_TEXT_BOTTOM_RIGHT].set( xpos + wd, ypos,      zpos);
    char_gset_coords[VTK_TEXT_TOP_RIGHT   ].set( xpos + wd, ypos + ht, zpos);
    char_gset_coords[VTK_TEXT_TOP_LEFT    ].set( xpos,      ypos + ht, zpos);

    tex_pos = textureFont->GetChararacterPosition(c);
    tex_size = textureFont->GetChararacterSize(c);

    tex_coords[VTK_TEXT_BOTTOM_LEFT ].set( tex_pos[0]              , tex_pos[1] + tex_size[1]);
    tex_coords[VTK_TEXT_BOTTOM_RIGHT].set( tex_pos[0] + tex_size[0], tex_pos[1] + tex_size[1]);
    tex_coords[VTK_TEXT_TOP_RIGHT   ].set( tex_pos[0] + tex_size[0], tex_pos[1]); 
    tex_coords[VTK_TEXT_TOP_LEFT    ].set( tex_pos[0]              , tex_pos[1]);

    if (this->TextureTextPolys != NULL) this->TextureTextPolys->Modified();
}


// adjust text locations, so that they can join with other texts, maintaining a common origin, especially for using camera followers
void vtkTextureText::SetPositionOffset(vtkFloatingPointType pos[3]) {
    this->SetPositionOffset(pos[0], pos[1], pos[2]);
}


void vtkTextureText::SetPositionOffset(vtkFloatingPointType x, vtkFloatingPointType y, vtkFloatingPointType z) {
    this->PositionOffset.set(x, y, z);
    Modified();
}


void vtkTextureText::GetPositionOffset(vtkFloatingPointType pos[3]) {
    for (int i = 0; i < 3; i++) pos[i] = this->PositionOffset[i];
}


void vtkTextureText::AddPositionOffset(vtkFloatingPointType x, vtkFloatingPointType y, vtkFloatingPointType z) {
    vtkFloatingPointType pos[3];
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
    this->AddPositionOffset(pos);
}


// adjust text locations, by adding a new bias into the PositionOffset
void vtkTextureText::AddPositionOffset(vtkFloatingPointType posNew[3]) {
    vtkFloatingPointType posCur[3];
    this->GetPositionOffset(posCur);
    for (int i = 0; i < 3; i++) posCur[i] += posNew[i];

    this->SetPositionOffset(posCur[0], posCur[1], posCur[2]);
}


// PositionOffset works in the local vertex space, so we compen

void vtkTextureText::DoPositionOffset() {
    if (this->PositionOffset[0] == 0  &&  this->PositionOffset[1] == 0  &&  this->PositionOffset[2] == 0) return;

    // v52 - Compensating for the scale seems wrong.
    //   We should just be working in our local vertex space - which we do now.
    //   Previously, we tried to use a world space PositionOffset, but this was problematic -
    //   Modified would always need to  be called (but wasn't being) - a current scale would 
    //   have to be correct, didn't match the more straightforward vtkRectangle's local space 
    //   for PositionOffset.

    //vtkFloatingPointType s[3];
    //GetFollower()->GetScale(s);

    for (int i = 0; i < mCharCount; i++) {
        for (int j = 0; j < 4; j++ ) {
            for (int k = 0; k < 3; k++) {
                //mCharGsetCoords[i * 4 + j].values[k] += this->PositionOffset[k] / s[k];
                mCharGsetCoords[i * 4 + j].values[k] += this->PositionOffset[k];
            }
        }
    }
}


void vtkTextureText::SetDim(int d) {
    if (this->Dim == d) return;
    this->Dim = d;

    vtkFloatingPointType to = TextOpacity;

    // TODO LOW - could use specific member values for the dimmer opacity
    if (d) to /= 10.;

    //printf("vtkTextureText::SetDim %d, Text: %s, to %g\n", d, this->GetText(), to);

    if (this->GetFollower() != NULL) this->GetFollower()->GetProperty()->SetOpacity(to);
    Modified();
}


void vtkTextureText::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    /*  TODO - do print
    os << indent << "X Resolution: " << this->XResolution << "\n";
    os << indent << "Y Resolution: " << this->YResolution << "\n";
    ...
    */
}
