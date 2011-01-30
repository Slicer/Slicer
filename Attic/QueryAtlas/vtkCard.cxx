/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkCard.cxx,v $
  Date:      $Date: 2006/05/26 19:59:41 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
/*===========================================================

Authors: Michael McKenna, David Small, Steve Pieper.
    Small Design Firm, Inc.,
    in association with Isomics, Inc.,
    and Partners Health Care.  MA, USA.
    June, 2004

===========================================================*/

#include "vtkCard.h"
#include "vtkRectangle.h"
#include "vtkTextureText.h"
#include "vtkEdgeGradient.h"

#include "vtkFollower.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkCamera.h"
#include "vtkRenderer.h"

#include "vtkLineSource.h"
#include "vtkCommand.h"


// TODO MED - filelength - not solaris stanard?  or just not in io.h?
//#include <io.h>


vtkCxxRevisionMacro(vtkCard, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkCard);


// TODO - these default values could be exposed w/ some methods
//static vtkFloatingPointType DefaultBoxOpacity = 0.75;
static vtkFloatingPointType DefaultBoxOpacity = 0.90;
static vtkFloatingPointType DefaultTextOpacity = 0.99;


vtkFloatingPointType vtkCard::DefaultBoxDistanceZ = 0.04;
//vtkFloatingPointType vtkCard::DefaultBorderWidth = 0.15;
vtkFloatingPointType vtkCard::DefaultBorderWidth = 0.25;


//vtkFloatingPointType vtkCard::DefaultBoxEdgeWidth = 0.1;
vtkFloatingPointType vtkCard::DefaultBoxEdgeWidth = 0.2;

//vtkFloatingPointType vtkCard::DefaultBoxEdgeBias = .02;
// v53 - edge was out a little too far - could see a small gap
//vtkFloatingPointType vtkCard::DefaultBoxEdgeBias = .175;
vtkFloatingPointType vtkCard::DefaultBoxEdgeBias = .16;


//vtkFloatingPointType vtkCard::DefaultBoxEdgeColor[3] = {0, 0, 0};
vtkFloatingPointType vtkCard::DefaultBoxEdgeColor[3] = {158. / 255., 11 / 255., 14 / 255.};

bool vtkCard::DefaultUseBoxEdge = 1;

vtkFloatingPointType vtkCard::DefaultBoxColor[3] = {1, 1, 1};


vtkCard::vtkCard()
{
    int i;
    this->Box = NULL;
    this->BoxRectangle = NULL;
    for (i = 0; i < 3; i++) this->BoxColor[i] = this->DefaultBoxColor[i];
    this->BoxOpacity = DefaultBoxOpacity;
    this->TextOpacity = DefaultTextOpacity;

    this->BoxEdgeCount = VTK_TEXT_CARD_EDGE_COUNT;

    for (i = 0; i < this->BoxEdgeCount; i++) {
        this->BoxEdge[i] = NULL;
        this->BoxEdgeRectangle[i] = NULL;
    }

    this->UnscaledBoxEdgeWidth = this->DefaultBoxEdgeWidth;
    this->BoxEdgeWidth = this->DefaultBoxEdgeWidth;
    for (i = 0; i < 3; i++) this->BoxEdgeColor[i] = this->DefaultBoxEdgeColor[i];

    this->UnscaledBoxEdgeBias = this->DefaultBoxEdgeBias;
    this->BoxEdgeBias = this->DefaultBoxEdgeBias;
    this->UseBoxEdge = vtkCard::DefaultUseBoxEdge;

    this->UseTexturedBoxEdge = 1;

    this->BoxWidthMinimum = 0;
    this->BoxHeightMinimum = 0;

    this->MainText = NULL;

    this->UnscaledBorderWidth = this->DefaultBorderWidth;
    this->BorderWidth = this->DefaultBorderWidth;

    this->UnscaledBoxDistanceZ = this->DefaultBoxDistanceZ;
    this->BoxDistanceZ = this->DefaultBoxDistanceZ;

    this->ScaleFactor = 1.;

    this->OthersVisibility = 1;

    this->LineActor = NULL;
    //this->LineEnd.set(0,0,0);
    this->LineSource = NULL;

    this->Dim = 0;

    this->Texture = NULL;

    //this->ImageFollower = NULL;

    this->OffsetActor = NULL;
    this->OffsetMarker = NULL;
    this->OffsetVector.set(0, 0, 0);
    this->OffsetMarkerVector.set(0, 0, 0);

    this->Renderer = NULL;

    this->OtherTexts = vtkCollection::New();
    this->ImageFollowers = vtkCollection::New();
    this->ImageRectangles = vtkCollection::New();

    this->LevelNum = 0;

}


vtkCard::~vtkCard() {

    // TODO - other deletes and cleanups (?)
}


vtkCollection *vtkCard::GetOtherTexts() {
    return this->OtherTexts;
}


vtkCollection *vtkCard::GetImageFollowers() {
    return this->ImageFollowers;
}


vtkCollection *vtkCard::GetImageRectangles() {
    return this->ImageRectangles;
}


void vtkCard::SetMainText(vtkTextureText *t) {
    this->MainText = t;

    //if (!this->Initialized) return;

    if (this->Box == NULL) CreateBox();
    else UpdateBox();
}


// This settings allows a text card to be placed automatically away from an actor (a), with a line 
//   drawn to a marker-actor (m), given a vector offset (off).
// Usually, you would only use one of the offset dimensions - e.g. Y axis, especially when you use the
//   same offset value for multiple cards, otherwise, the cards could start to get placed in the
//   same location.
// If the offset value is positive (e.g. 0, 5, 0), the card will be located to the positive side 
//   of the actor's bounding box, plus the offset.  If the marker's position is beyond the bounding
//   box, the card is extended further, so that there is a minimum of offset between the marker and
//   the card's origin.
// If the offset is negative, the card is placed to the negative side of the actor's bounding box.


// for tcl - don't use vtkFloatingPointType[3]
// for slicer fiducials - 2004-03 - set position, don't use fiducial marker
void vtkCard::SetOffsetActorAndMarker(vtkActor *a, vtkFloatingPointType markerX, vtkFloatingPointType markerY, vtkFloatingPointType markerZ, 
                                          vtkFloatingPointType offX, vtkFloatingPointType offY, vtkFloatingPointType offZ) {
    vtkFloatingPointType off[3];
    off[0] = offX;
    off[1] = offY;
    off[2] = offZ;
    OffsetMarkerVector.set(markerX, markerY, markerZ);
    OffsetMarker = NULL;

    OffsetActor = a;
    OffsetVector.set(off[0], off[1], off[2]);

    // TODO - we don't yet call Update all the updat-able times
    UpdateOffsetActorAndMarker();
}


// for tcl - don't use vtkFloatingPointType[3]
void vtkCard::SetOffsetActorAndMarker(vtkActor *a, vtkActor *m, vtkFloatingPointType offX, vtkFloatingPointType offY, vtkFloatingPointType offZ) {
    vtkFloatingPointType off[3];
    off[0] = offX;
    off[1] = offY;
    off[2] = offZ;
    SetOffsetActorAndMarker(a, m, off);
}

    
void vtkCard::SetOffsetActorAndMarker(vtkActor *a, vtkActor *m, vtkFloatingPointType off[3]) {
    OffsetActor = a;
    OffsetMarker = m;
    OffsetVector.set(off[0], off[1], off[2]);

    UpdateOffsetActorAndMarker();
}


void vtkCard::UpdateOffsetActorAndMarker() {
    int i;

    if (OffsetActor == NULL) return;
    // we can use OffsetMarkerVector instead of an actor
    //if (OffsetMarker == NULL) return;

    vtkFloatingPointType actorBounds[6];
    OffsetActor->GetBounds(actorBounds);

    vtkFloatingPointType markerPos[3];
    if (OffsetMarker == NULL) for (i = 0; i < 3; i++) markerPos[i] = OffsetMarkerVector[i];
    else OffsetMarker->GetPosition(markerPos);

    if (this->LineSource == NULL) CreateLine(markerPos[0], markerPos[1], markerPos[2]);
    else SetLinePoint2(markerPos[0], markerPos[1], markerPos[2]);

    vtkFloatingPointType pos[3];
    for (i = 0; i < 3; i++) pos[i] = markerPos[i];

    for (i = 0; i < 3; i++) {
        if (OffsetVector[i] != 0) {
            if (OffsetVector[i] < 0) {
                // go left/under/behind the actor
                pos[i] = actorBounds[i * 2];
                if (markerPos[i] < pos[i]) pos[i] = markerPos[i];
                pos[i] += OffsetVector[i];
            }
            else {
                // go right/above/front the actor
                pos[i] = actorBounds[i * 2 + 1];
                if (markerPos[i] > pos[i]) pos[i] = markerPos[i];
                pos[i] += OffsetVector[i];
            }
        }
    }

    SetPosition(pos[0], pos[1], pos[2]);
}


vtkFollower *vtkCard::CreateBox()
{
    if (this->MainText == NULL) return NULL;

    this->Box = vtkFollower::New();
    this->BoxRectangle = vtkRectangle::New();
    vtkPolyDataMapper *planeMapper = vtkPolyDataMapper::New();
    planeMapper->SetInput(this->BoxRectangle->GetOutput());
    this->Box->SetMapper(planeMapper);

    this->Box->GetProperty()->SetColor(this->BoxColor);
    this->Box->GetProperty()->SetOpacity(BoxOpacity);

    if (this->UseBoxEdge) CreateBoxEdge();

    UpdateBox();
    return this->Box;
}


void vtkCard::CreateBoxEdge() {
    int i;
    
    if (this->UseBoxEdge) {
        // edge + corner objects
        // NOTE - we will always make the corner objects with the edge object, even if we don't texture/gradient map them
        for (i = 0; i < this->BoxEdgeCount; i++) {
            this->BoxEdge[i] = vtkFollower::New();
            this->BoxEdgeRectangle[i] = vtkRectangle::New();
            vtkPolyDataMapper *planeMapper = vtkPolyDataMapper::New();
            planeMapper->SetInput(this->BoxEdgeRectangle[i]->GetOutput());
            this->BoxEdge[i]->SetMapper(planeMapper);

            this->BoxEdge[i]->GetProperty()->SetColor(this->BoxEdgeColor);
            this->BoxEdge[i]->GetProperty()->SetOpacity(BoxOpacity);

            // rotate texture maps on rects - need to match corner obj rotates to exactly align gradients
            // side 0: turn 0,  3 1,  2 2,  1 3

            if (i == 1) this->BoxEdgeRectangle[i]->SetTextureTurn(3);
            if (i == 2) this->BoxEdgeRectangle[i]->SetTextureTurn(2);
            if (i == 3) this->BoxEdgeRectangle[i]->SetTextureTurn(1);

            // TODO - cleanup & optimize - just one or two texture gradients should be needed - reuse them.

            if (this->UseTexturedBoxEdge) {
                vtkTexture *t = vtkTexture::New();
                t->InterpolateOn();   
                t->RepeatOff();

                vtkEdgeGradient *g = vtkEdgeGradient::New();
                if (i >= 4) {
                    // rotate texture maps to turn the gradients:  corner 7: 0 turn 6:  1, 5:  2,  4: 3
                    this->BoxEdgeRectangle[i]->SetTextureTurn(7 - i);
                    g->SetCorner(1);
                }

                g->Create();

                t->SetInput(g);
                this->BoxEdge[i]->SetTexture(t);
            }

        }
    }
    // to avoid redunant calls, callers should do this:
    //UpdateBox();
}


void vtkCard::RemoveBoxEdge() {
    int i;
    if (this->UseBoxEdge) return;

    for (i = 0; i < this->BoxEdgeCount; i++) {
        if (this->Renderer != NULL  &&  this->BoxEdge[i] != NULL) 
            this->Renderer->RemoveActor(this->BoxEdge[i]);
        this->BoxEdge[i] = NULL;
        this->BoxEdgeRectangle[i] = NULL;

        // TODO - there's probably more VTK cleanup we could do, to free up memory
    }
}


void vtkCard::SetUseBoxEdge(bool b) {
    if (b == this->UseBoxEdge) return;
    this->UseBoxEdge = b;

    if (b) {
        CreateBoxEdge();
        UpdateBox();
    }
    else {
        RemoveBoxEdge();
    }
}


void vtkCard::SetUseTexturedBoxEdge(bool b) {
    if (b == this->UseTexturedBoxEdge) return;
    this->UseTexturedBoxEdge = b;

    // TODO MED - needs updating for texture on/off!
    // so, for now, set this up before calling SetMainText() (which calls CreateBox())
/*
    if (b) {
        CreateBoxEdge();
        UpdateBox();
    }
    else {
        RemoveBoxEdge();
    }
    */
}


/*  
Box Edges
    0
  -----
 1|   |3  <- sides gradient->Turn = 1
  |   |
  -----
    2

Corners
  4   7
  o---o
  |   |
  |   |
  o---o
  5   6
*/



vtkFollower *vtkCard::UpdateBox() {
    if (this->Box == NULL) return NULL;
    if (this->BoxRectangle == NULL) return NULL;
    if (this->MainText == NULL) return NULL;

    vtkFloatingPointType pos[3];
    vtkFollower *fText = this->MainText->GetFollower();
    fText->GetPosition(pos);

    vtkFloatingPointType bo[6];
    fText->GetBounds(bo);

    this->Box->SetPosition(pos[0], pos[1], pos[2] - this->BoxDistanceZ);

    int i;
    if (this->OthersVisibility) {
        for (i = 0; i < OtherTexts->GetNumberOfItems(); i++) {
            vtkFloatingPointType bo2[6];
            ((vtkTextureText *)OtherTexts->GetItemAsObject(i))->GetFollower()->GetBounds(bo2);
            if (bo2[0] < bo[0]) bo[0] = bo2[0];
            if (bo2[1] > bo[1]) bo[1] = bo2[1];
            if (bo2[2] < bo[2]) bo[2] = bo2[2];
            if (bo2[3] > bo[3]) bo[3] = bo2[3];
        }

        //  images also follow the OthersVisibility rule
        for (i = 0; i < this->ImageFollowers->GetNumberOfItems(); i++) {
            vtkFloatingPointType bo2[6];
            ((vtkFollower *)ImageFollowers->GetItemAsObject(i))->GetBounds(bo2);
            if (bo2[0] < bo[0]) bo[0] = bo2[0];
            if (bo2[1] > bo[1]) bo[1] = bo2[1];
            if (bo2[2] < bo[2]) bo[2] = bo2[2];
            if (bo2[3] > bo[3]) bo[3] = bo2[3];
        }
    }


    vtkFloatingPointType w = bo[1] - bo[0] + this->BorderWidth * 2.;
    vtkFloatingPointType h = bo[3] - bo[2] + this->BorderWidth * 2.;
    // the text rects don't really start right at 0,0, and can descend below - need to use the lower XY
    vtkFloatingPointType l = -this->BorderWidth - (pos[0] - bo[0]);
    vtkFloatingPointType b = -this->BorderWidth - (pos[1] - bo[2]);

    // NOTE - because of the extra borderwidth, we should expand our 'Minimums' by this amount-
    //   it's really the minimum size of the inset text/images.
    // v46 - use current scale
    //vtkFloatingPointType m = this->BoxWidthMinimum + this->BorderWidth * 2.;
    vtkFloatingPointType m = this->BoxWidthMinimum * this->ScaleFactor + this->BorderWidth * 2.;
    if (w < m) w = m;
    m = this->BoxHeightMinimum;
    if (h < m) h = m;

    this->BoxRectangle->SetWidth(w);
    this->BoxRectangle->SetHeight(h);
    this->BoxRectangle->SetLeft(l);
    this->BoxRectangle->SetBottom(b);

    if (UseBoxEdge) {
        for (i = 0; i < this->BoxEdgeCount; i++) {
            if (this->BoxEdgeRectangle[i] != NULL) {
                this->BoxEdge[i]->SetPosition(pos[0], pos[1], pos[2] - this->BoxDistanceZ);
                vtkFloatingPointType wb, hb, lb, bb;

                if (i == 0 || i == 2) {
                    // bias the edges out a little bit - especially important since the left and right 
                    //   edges don't exactly match the underlying card, so sparkles would be seen on 
                    //   the outer edge
                    // also,with gradient corners, the sharp edge box corner will be seen, unless
                    //   the gradient edging is biased ourwards.

                    // TODO - this makes the outer biased edge a little different when semi-trans, 
                    //   since it doesn't blend with the card there.  
                    //   generally this os OK, and there's nothing really to do about it.

                    // leave space for the corners
                    wb = w - 2.0 * this->BoxEdgeWidth + 2.0 * this->BoxEdgeBias;
                    hb = this->BoxEdgeWidth;
                    lb = l + this->BoxEdgeWidth - this->BoxEdgeBias;
                    if (i == 0) bb = b + h - this->BoxEdgeWidth + this->BoxEdgeBias;
                    else bb = b - this->BoxEdgeBias;
                }

                else if (i == 1  ||  i == 3) {
                    // don't overlap at the corner
                    wb = this->BoxEdgeWidth;
                    hb = h - 2.0 * this->BoxEdgeWidth + 2.0 * this->BoxEdgeBias;
                    if (i == 1) lb = l - this->BoxEdgeBias;
                    else lb = l + w - this->BoxEdgeWidth + this->BoxEdgeBias;
                    bb = b + this->BoxEdgeWidth - this->BoxEdgeBias;
                }

                // corner objects
                else {
                    wb = this->BoxEdgeWidth;
                    hb = this->BoxEdgeWidth;
                    if (i == 4  ||  i == 5) lb = l - this->BoxEdgeBias;
                    else lb = l + w - this->BoxEdgeWidth + this->BoxEdgeBias;

                    if (i == 4  ||  i == 7) bb = b + h - this->BoxEdgeWidth + this->BoxEdgeBias;
                    else bb = b - this->BoxEdgeBias;
                }

                this->BoxEdgeRectangle[i]->SetWidth(wb);
                this->BoxEdgeRectangle[i]->SetHeight(hb);
                this->BoxEdgeRectangle[i]->SetLeft(lb);
                this->BoxEdgeRectangle[i]->SetBottom(bb);
            }
        }
    }


    // v22 - odd idea - set origin to center, so it's flatter against screen when following
    // nope - sends it's follower rotation way off
    //this->Box->GetBounds(bo);
    //this->Box->GetPosition(pos);
    //this->Box->SetOrigin(bo[1] - bo[0] + pos[0], bo[3] - bo[2] + pos[1], pos[2]);

    return this->Box;
}


// TODO - the 'unscaled' parameters are a pain to maintain (but they work).

void vtkCard::SetBorderWidth(vtkFloatingPointType b) {
    this->UnscaledBorderWidth = b;
    this->BorderWidth = b * ScaleFactor;
    UpdateBox();
}


void vtkCard::SetBoxEdgeWidth(vtkFloatingPointType b) {
    this->UnscaledBoxEdgeWidth = b;
    this->BoxEdgeWidth = b * ScaleFactor;
    UpdateBox();
}


void vtkCard::SetBoxEdgeBias(vtkFloatingPointType b) {
    this->UnscaledBoxEdgeBias = b;
    this->BoxEdgeBias = b * ScaleFactor;
    UpdateBox();
}


void vtkCard::SetBoxEdgeColor(vtkFloatingPointType r, vtkFloatingPointType g, vtkFloatingPointType b) {
    int i;
    this->BoxEdgeColor[0] = r;
    this->BoxEdgeColor[1] = g;
    this->BoxEdgeColor[2] = b;

    for (i = 0; i < this->BoxEdgeCount; i++) {
        if (this->BoxEdge[i] != NULL) 
            this->BoxEdge[i]->GetProperty()->SetColor(this->BoxEdgeColor);
    }
}


void vtkCard::SetDefaultBoxEdgeColor(vtkFloatingPointType r, vtkFloatingPointType g, vtkFloatingPointType b) {
    vtkCard::DefaultBoxEdgeColor[0] = r;
    vtkCard::DefaultBoxEdgeColor[1] = g;
    vtkCard::DefaultBoxEdgeColor[2] = b;
}


void vtkCard::SetBoxColor(vtkFloatingPointType r, vtkFloatingPointType g, vtkFloatingPointType b) {
    this->BoxColor[0] = r;
    this->BoxColor[1] = g;
    this->BoxColor[2] = b;

    if (this->Box != NULL) 
        this->Box->GetProperty()->SetColor(this->BoxColor);
}


void vtkCard::SetDefaultBoxColor(vtkFloatingPointType r, vtkFloatingPointType g, vtkFloatingPointType b) {
    vtkCard::DefaultBoxColor[0] = r;
    vtkCard::DefaultBoxColor[1] = g;
    vtkCard::DefaultBoxColor[2] = b;
}


void vtkCard::SetBoxDistanceZ(vtkFloatingPointType d) {
    this->UnscaledBoxDistanceZ = d;
    this->BoxDistanceZ = d * ScaleFactor;
    UpdateBox();
}


// SetPosition sets the start of the baseline of the MainText, which forms the card origin.
// This method uses regular SetPosition to movce all the objects around.  
// When the Follower mode is on, this can cause the objects to skew a little, and oddly
//   clip, especially when you get close to them.
// The PositionOffset methods move around the local vertices of the objects instead.


void vtkCard::SetPosition(vtkFloatingPointType x, vtkFloatingPointType y, vtkFloatingPointType z) {
    if (this->MainText == NULL) return;

    // TODO - this and other follower methods should use a generic list of followers (where possible), or figure out how Assembly could work (or make our own)

    vtkFloatingPointType pos[3];
    this->MainText->GetFollower()->GetPosition(pos);
    vtkFloatingPointType delta[3];
    delta[0] = x - pos[0];
    delta[1] = y - pos[1];
    delta[2] = z - pos[2];
    this->MainText->GetFollower()->SetPosition(x, y, z);
    int i, j;
    for (i = 0; i < OtherTexts->GetNumberOfItems(); i++) {
        ((vtkTextureText *)OtherTexts->GetItemAsObject(i))->GetFollower()->GetPosition(pos);
        for (j = 0; j < 3; j++) pos[j] += delta[j];
        ((vtkTextureText *)OtherTexts->GetItemAsObject(i))->GetFollower()->SetPosition(pos);
    }

    if (LineActor != NULL  &&  LineSource != NULL) LineSource->SetPoint1(x, y, z);

    for (i = 0; i < this->ImageFollowers->GetNumberOfItems(); i++) {
        ((vtkFollower *)ImageFollowers->GetItemAsObject(i))->GetPosition(pos);
        for (j = 0; j < 3; j++) pos[j] += delta[j];
        ((vtkFollower *)ImageFollowers->GetItemAsObject(i))->SetPosition(pos);
    }

    Modified();
}


// shift the objects' local point coordinates

void vtkCard::AddPositionOffset(vtkFloatingPointType x, vtkFloatingPointType y, vtkFloatingPointType z) {
    if (this->MainText == NULL) return;

    // TODO - bleh - another damn scale issue - need a new assembly + follower!

    vtkFloatingPointType s[3];
    this->MainText->GetFollower()->GetScale(s);
    this->MainText->AddPositionOffset(x / s[0], y / s[1], z / s[2]);

    // modify each individual SetPositionOffset - not a delta off of MainText only
    int i;
    for (i = 0; i < OtherTexts->GetNumberOfItems(); i++) {
        ((vtkTextureText *)OtherTexts->GetItemAsObject(i))->GetFollower()->GetScale(s);
        ((vtkTextureText *)OtherTexts->GetItemAsObject(i))->AddPositionOffset(x / s[0], y / s[1], z / s[2]);
    }

    // TODO MED - test the line offset
    if (LineActor != NULL  &&  LineSource != NULL) {
        vtkFloatingPointType lpos[3];
        LineSource->GetPoint1(lpos);
        lpos[0] += x;
        lpos[1] += y;
        lpos[2] += z;
        LineSource->SetPoint1(lpos);
    }

    for (i = 0; i < this->ImageFollowers->GetNumberOfItems(); i++) {
        ((vtkFollower *)ImageFollowers->GetItemAsObject(i))->GetScale(s);
        vtkRectangle *r = (vtkRectangle *)ImageRectangles->GetItemAsObject(i);
        r->AddPositionOffset(x / s[0], y / s[1], z / s[2]);
    }

    // NO - let's not modify the box at this point!
    //Modified();
}


void vtkCard::SetPosition (vtkFloatingPointType _arg[3]) {
    SetPosition(_arg[0], _arg[1], _arg[2]);
}


void vtkCard::GetPosition (vtkFloatingPointType data[3]) {
    this->MainText->GetFollower()->GetPosition(data);
}


void vtkCard::SetScale(vtkFloatingPointType s) {
    if (this->MainText == NULL) return;

    vtkTextureText *t = NULL;
    // base the new scale relative to the last scale.
    vtkFloatingPointType scaleChange = s / this->ScaleFactor;
    this->ScaleFactor = s;

    // NOTE - this will update these objects' scaled params.
    //   The "Unscaled" values maintain their original scale.
    SetBorderWidth(UnscaledBorderWidth);
    SetBoxEdgeWidth(UnscaledBoxEdgeWidth);
    SetBoxEdgeBias(UnscaledBoxEdgeBias);
    SetBoxDistanceZ(UnscaledBoxDistanceZ);    

    // The Box will auto-size itself - no need to explicitly Scale it.

    // NOTE - only using X [0] scale factor - but that's generally OK

    this->MainText->GetFollower()->SetScale(this->MainText->GetFollower()->GetScale()[0] * scaleChange);

    int i;
    for (i = 0; i < OtherTexts->GetNumberOfItems(); i++) {
        t = (vtkTextureText *)OtherTexts->GetItemAsObject(i);
        t->GetFollower()->SetScale(
            t->GetFollower()->GetScale()[0] * scaleChange);
    }


    // TODO MED - don't we need to compensate for the Line?
    if (LineActor != NULL  &&  LineSource != NULL) {
        vtkFloatingPointType p[3];
        LineSource->GetPoint1(p);
        int hh;
        for (hh= 0; hh< 0; hh++) p[hh] *= scaleChange;
        LineSource->SetPoint1(p);
    }

    // NOTE - for the images also, we're only using the X [0] scale factor -
    //   this might be more of an issue than for the text objects, but it should be fine.

    for (i = 0; i < this->ImageFollowers->GetNumberOfItems(); i++) {
        ((vtkFollower *)this->ImageFollowers->GetItemAsObject(i))->SetScale(
            ((vtkFollower *)this->ImageFollowers->GetItemAsObject(i))->GetScale()[0] * scaleChange);
    }

    Modified();
}


void vtkCard::Center() {
    // use current box, & center x/y

    // NOTE - this centers correctly - it's a bit confusing, tho...
    // SetPosition is on the MainText starting baseline, not box corner, 
    // so we take the difference between the mainText baseline origin and the box center.

    // You may want to use CenterOffset instead - to shift all the objects' local origins
    
    if (this->Box == NULL) return;

    vtkFloatingPointType bo[6];
    this->Box->GetBounds(bo);

    // center of box
    vtkFloatingPointType cx = ((bo[1] - bo[0]) / 2. + bo[0]);
    vtkFloatingPointType cy = ((bo[3] - bo[2]) / 2. + bo[2]);

    // offset from where the MainText is
    vtkFloatingPointType pos[3];
    this->MainText->GetFollower()->GetPosition(pos);
    pos[0] -= cx;
    pos[1] -= cy;

    // NOTE - we don't change the Z - we just pass through the original Text's Z

    this->SetPosition(pos[0], pos[1], pos[2]);
}


void vtkCard::CenterOffset() {
    if (this->Box == NULL) return;

    vtkFloatingPointType bo[6];
    this->Box->GetBounds(bo);

    // center of box in world-space
    vtkFloatingPointType cx = ((bo[1] - bo[0]) / 2. + bo[0]);
    vtkFloatingPointType cy = ((bo[3] - bo[2]) / 2. + bo[2]);

    vtkFloatingPointType pos[3];

    // offset the world-space bounds from the SetPosition for the objects - get back to the rectangle's Offset value.

    // TODO - test this when the SetPosition is not set to 0
    this->Box->GetPosition(pos);
    cx -= pos[0];
    cy -= pos[1];

    // only move the Card w/ PositionOffset in XY
    // we don't change the Z - we just pass through the original Text's Z
    this->AddPositionOffset(-cx, -cy, 0);
}


// TODO - OK - all this redundancy is ridiculous - need an Assembly-type thingie!
//  These simpler operations (SetVis) could be handled through a more general list of Followers/Actors.

void vtkCard::SetVisibility(bool v) {
    if (this->MainText != NULL) this->MainText->GetFollower()->SetVisibility(v);
    if (this->Box != NULL) this->Box->SetVisibility(v);
    int i;
    for (i = 0; i < this->BoxEdgeCount; i++) if (this->BoxEdge[i] != NULL) this->BoxEdge[i]->SetVisibility(v);

    // v47 - when the OthersVisibility flag is off, we need to respect it!
    if (this->OthersVisibility == 0) v = 0;

    for (i = 0; i < this->ImageFollowers->GetNumberOfItems(); i++) {
        ((vtkFollower *)ImageFollowers->GetItemAsObject(i))->SetVisibility(v);
    }

    for (i = 0; i < OtherTexts->GetNumberOfItems(); i++) {
        ((vtkTextureText *)OtherTexts->GetItemAsObject(i))->GetFollower()->SetVisibility(v);
    }
}


void vtkCard::SetCamera(vtkCamera *cam) {
    if (this->MainText != NULL) this->MainText->GetFollower()->SetCamera(cam);
    if (this->Box != NULL) this->Box->SetCamera(cam);
    int i;
    for (i = 0; i < this->BoxEdgeCount; i++) if (this->BoxEdge[i] != NULL) this->BoxEdge[i]->SetCamera(cam);

    for (i = 0; i < this->ImageFollowers->GetNumberOfItems(); i++) {
        ((vtkFollower *)ImageFollowers->GetItemAsObject(i))->SetCamera(cam);
    }

    for (i = 0; i < OtherTexts->GetNumberOfItems(); i++) {
        ((vtkTextureText *)OtherTexts->GetItemAsObject(i))->GetFollower()->SetCamera(cam);
    }
}


void vtkCard::RemoveActors() {
    this->RemoveActors(this->Renderer);
}


// This method, with the Renderer arg, is provided just to be more parallel to the standard calls.
//   The specified Renderer must match the member Renderer.  For simplicity just call the plain
//   RemoveActors().

void vtkCard::RemoveActors(vtkRenderer *r) {
    if (r == NULL) {
        vtkErrorMacro("vtkCard::RemoveActors : ERROR - NULL renderer - aborting.\n");
        return;
    }

    if (r != this->Renderer) {
        vtkErrorMacro("vtkCard::RemoveActors : ERROR - specified renderer does not match the member renderer - aborting.\n");
        return;
    }

    if (this->Box != NULL) r->RemoveActor(this->Box);
    int i;
    for (i = 0; i < this->BoxEdgeCount; i++) if (this->BoxEdge[i] != NULL) r->RemoveActor(this->BoxEdge[i]);
    if (this->MainText != NULL) r->RemoveActor(this->MainText->GetFollower());

    for (i = 0; i < OtherTexts->GetNumberOfItems(); i++) {
        r->RemoveActor(((vtkTextureText *)OtherTexts->GetItemAsObject(i))->GetFollower());
    }
    if (LineActor != NULL) r->RemoveActor(LineActor);
    for (i = 0; i < this->ImageFollowers->GetNumberOfItems(); i++) {
        r->RemoveActor(((vtkFollower *)this->ImageFollowers->GetItemAsObject(i)));
    }
}


void vtkCard::AddActors() {
    this->AddActors(this->Renderer);
}


void vtkCard::AddActors(vtkRenderer *r) {
    if (r == NULL) {
        vtkErrorMacro("vtkCard::AddActors : ERROR - NULL renderer - aborting.\n");
        return;
    }

    this->Renderer = r;
    if (this->Box != NULL) r->AddActor(this->Box);
    int i;
    for (i = 0; i < this->BoxEdgeCount; i++) if (this->BoxEdge[i] != NULL) r->AddActor(this->BoxEdge[i]);

    if (this->MainText != NULL) r->AddActor(this->MainText->GetFollower());

    for (i = 0; i < OtherTexts->GetNumberOfItems(); i++) {
        r->AddActor(((vtkTextureText *)OtherTexts->GetItemAsObject(i))->GetFollower());
        // v47 - hide if needed
        if (this->OthersVisibility == 0)
            ((vtkTextureText *)OtherTexts->GetItemAsObject(i))->GetFollower()->VisibilityOff();
    }
    if (LineActor != NULL) r->AddActor(LineActor);
    for (i = 0; i < this->ImageFollowers->GetNumberOfItems(); i++) {
        r->AddActor(((vtkFollower *)this->ImageFollowers->GetItemAsObject(i)));
    }
}


void vtkCard::Modified() {
    UpdateBox();
}


void vtkCard::SetOthersVisibility(bool v) {
    if (v) OthersVisibilityOn();
    else OthersVisibilityOff();
}


void vtkCard::OthersVisibilityOn() {
    this->OthersVisibility = 1;
    int i;
    for (i = 0; i < OtherTexts->GetNumberOfItems(); i++) {
        ((vtkTextureText *)OtherTexts->GetItemAsObject(i))->GetFollower()->VisibilityOn();
    }
    for (i = 0; i < this->ImageFollowers->GetNumberOfItems(); i++) {
        ((vtkFollower *)ImageFollowers->GetItemAsObject(i))->VisibilityOn();
    }

    Modified();
}


void vtkCard::OthersVisibilityOff() {
    this->OthersVisibility = 0;
    int i;
    for (i = 0; i < OtherTexts->GetNumberOfItems(); i++) {
        ((vtkTextureText *)OtherTexts->GetItemAsObject(i))->GetFollower()->SetVisibility(0);
        //printf("vtkCard::OthersVisibilityOff() - item %d off\n", i);
    }
    for (i = 0; i < this->ImageFollowers->GetNumberOfItems(); i++) {
        ((vtkFollower *)ImageFollowers->GetItemAsObject(i))->SetVisibility(0);
    }

    Modified();
}


vtkActor *vtkCard::CreateLine(vtkFloatingPointType x, vtkFloatingPointType y, vtkFloatingPointType z) {
    this->LineActor = vtkActor::New();
    this->LineSource = vtkLineSource::New();
    vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
    mapper->SetInput(this->LineSource->GetOutput());
    this->LineActor->SetMapper(mapper);

    vtkFloatingPointType pos[3];
    GetPosition(pos);
    this->LineSource->SetPoint1(pos);
    this->LineSource->SetPoint2(x, y, z);

    return this->LineActor;
}


// NOTE - when using a follower Card, any local point besides 0,0,0 will get offset while rotating the camera.

void vtkCard::SetLinePoint1(vtkFloatingPointType x, vtkFloatingPointType y, vtkFloatingPointType z) {
    if (this->LineSource == NULL) return;
    this->LineSource->SetPoint1(x, y, z);
}



// NOTE - when using a follower Card, any local point besides 0,0,0 will get offset while rotating the camera.

void vtkCard::SetLinePoint1Local(vtkFloatingPointType x, vtkFloatingPointType y, vtkFloatingPointType z) {
    if (this->LineSource == NULL) return;
    vtkFloatingPointType pos[3];

    this->MainText->GetFollower()->GetPosition(pos);

    x += pos[0];
    y += pos[1];
    z += pos[2];
    SetLinePoint1(x, y, z);
}


void vtkCard::SetLinePoint2(vtkFloatingPointType x, vtkFloatingPointType y, vtkFloatingPointType z) {
    if (this->LineSource == NULL) return;
    this->LineSource->SetPoint2(x, y, z);
}


void vtkCard::SetOpacityBase(vtkFloatingPointType o) {
    vtkFloatingPointType to = TextOpacity * o;
    vtkFloatingPointType bo = BoxOpacity * o;

    //printf("vtkCard::SetDim %d, MainText: %s, bo %g, to %g\n", d, this->MainText->GetText(), bo, to);

    if (this->MainText != NULL) this->MainText->GetFollower()->GetProperty()->SetOpacity(to);
    if (this->MainText != NULL) this->MainText->GetFollower()->GetProperty()->SetOpacity(to);
    if (this->Box != NULL)     this->Box->GetProperty()->SetOpacity(bo);

    int i;
    for (i = 0; i < this->BoxEdgeCount; i++) if (this->BoxEdge[i] != NULL) this->BoxEdge[i]->GetProperty()->SetOpacity(bo);

    for (i = 0; i < OtherTexts->GetNumberOfItems(); i++) {
        ((vtkTextureText *)OtherTexts->GetItemAsObject(i))->GetFollower()->GetProperty()->SetOpacity(to);
    }

    // NOTE - using the text opacity for the image also (default 0.99)
    for (i = 0; i < this->ImageFollowers->GetNumberOfItems(); i++) {
        ((vtkFollower *)this->ImageFollowers->GetItemAsObject(i))->GetProperty()->SetOpacity(to);
    }

    Modified();
}


void vtkCard::SetDim(bool d) {
    if (this->Dim == d) return;
    this->Dim = d;

    vtkFloatingPointType to = TextOpacity;
    vtkFloatingPointType bo = BoxOpacity;

    // TODO LOW - could use specific member values for the dimmer opacity
    if (d) {
        to /= 10.;
        bo /= 10.;
    }

    //printf("vtkCard::SetDim %d, MainText: %s, bo %g, to %g\n", d, this->MainText->GetText(), bo, to);

    if (this->MainText != NULL) this->MainText->GetFollower()->GetProperty()->SetOpacity(to);
    if (this->Box != NULL)     this->Box->GetProperty()->SetOpacity(bo);
    int i;
    for (i = 0; i < this->BoxEdgeCount; i++) if (this->BoxEdge[i] != NULL) this->BoxEdge[i]->GetProperty()->SetOpacity(bo);

    for (i = 0; i < OtherTexts->GetNumberOfItems(); i++) {
        ((vtkTextureText *)OtherTexts->GetItemAsObject(i))->GetFollower()->GetProperty()->SetOpacity(to);
    }

    // NOTE - using the text opacity for the image also
    for (i = 0; i < this->ImageFollowers->GetNumberOfItems(); i++) {
        ((vtkFollower *)this->ImageFollowers->GetItemAsObject(i))->GetProperty()->SetOpacity(to);
    }

    Modified();
}


// utility method - load text from file; malloc the string

char *vtkCard::LoadString(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf (stderr, "LoadString: error opening file: %s", filename);
        return false;
    }

    int ret = 0;

    // TODO MED - filelength - not Solaris standard?  or just not in io.h?

    // TODO - back to a hardcoded max file len
    //int f = fileno(fp);
    //long blen = filelength(f) + 1;
    long blen = 5000;
    char *buf = (char *)malloc(blen);

    ret = fread(buf, 1, blen-1, fp);
    buf[ret] = 0;

    fclose(fp);

    char *buf2 = strdup(buf);
    free(buf);

    //return buf;
    return buf2;
}


void vtkCard::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    /*  TODO - do print
    os << indent << "X Resolution: " << this->XResolution << "\n";
    os << indent << "Y Resolution: " << this->YResolution << "\n";

    os << indent << "Origin: (" << this->Origin[0] << ", "
    << this->Origin[1] << ", "
    << this->Origin[2] << ")\n";

    os << indent << "Point 1: (" << this->Point1[0] << ", "
    << this->Point1[1] << ", "
    << this->Point1[2] << ")\n";
    ...
    */

}
