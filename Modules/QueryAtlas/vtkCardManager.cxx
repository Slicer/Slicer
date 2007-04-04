/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkCardManager.cxx,v $
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


#include "vtkRenderer.h"
#include "vtkObject.h"
#include "vtkCamera.h"
#include "vtkProperty.h"

#include "vtkCardManager.h"
#include "vtkCard.h"


vtkCxxRevisionMacro(vtkCardManager, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkCardManager);


vtkCardManager::vtkCardManager()
{
    this->CardSpacing = 30.0;
    this->NumCards = 0;
    this->CardZoom = 0.0;
    this->ScaleCards = 1.0;
    this->ScaleDownPower = 9;
    this->ScaleDownFlag = 0;
    this->Renderer = NULL;
    this->Sorter = NULL;
    this->Cards = vtkCollection::New();
    this->UseFollower = 1;
}


vtkCardManager::~vtkCardManager() {
}


// sub-classes override this to make a specific kind of card

vtkCard *vtkCardManager::NewCard() {
    return vtkCard::New();
}


// sub-classes override this to do specific card setup

void vtkCardManager::ExtraCardInit(vtkCard *c) {
    // a default, very basic card w/ a single text element
    char name[100];
    sprintf(name, "Level %d", c->GetLevelNum());

    vtkTextureText *tt = vtkTextureText::New();
    tt->GetFontParameters()->SetBlur(2);
    tt->SetText(name);
    tt->GetFollower()->GetProperty()->SetColor(0, 0, 0);
    tt->CreateTextureText();
    c->SetMainText(tt);
}


// sub-classes override this to prapare to make specific kinds of cards

void vtkCardManager::ExtraLoadInit() {
}


//void vtkCardManager::LoadSet(char *dirBase, vtkRenderer *ren) {
void vtkCardManager::LoadSet(vtkRenderer *ren) {
    this->Renderer = ren;
    LoadSet();
}


void vtkCardManager::LoadSet() {
    int level = 0;

    vtkCollection *tmpCards = vtkCollection::New();

    ExtraLoadInit();

    // NOTE - load cards in reverse order so that we don't normally need to depth sort.
    //   We'll reverse the list afterwards (to forward order).
    for (level = this->NumCards - 1; level >= 0; level--) {
        vtkCard *c = NewCard();

        c->SetLevelNum(level);
        
        c->SetRenderer(this->Renderer);

        ExtraCardInit(c);

        if (this->Sorter != NULL) this->Sorter->GetCards()->AddItem(c);

        // the SetCardZoom below will setup the positions

        // v47 - optionally scale down the cards as the level rises
        if (this->ScaleDownFlag) {
            vtkFloatingPointType sc = pow(((vtkFloatingPointType)level + 1.0), ScaleDownPower);
            c->SetScale(this->ScaleCards / sc);
        }
        else {
            c->SetScale(this->ScaleCards);
        }

        // v52 - create card w/ correct box, then center card vertices
        c->UpdateBox();

        // Use an initial CenterOffset to center the cards' local origins
        //c->Center();
        c->CenterOffset();

        // v52 - add flag for Follower, and move its activation here (from BIRNCard)
        if (this->UseFollower) c->SetCamera(this->Renderer->GetActiveCamera());

        tmpCards->AddItem(c);
    }

    // now, reverse the list, so the list goes in the right order
    for (level = 0; level < this->NumCards ; level++) {
        vtkObject *o = tmpCards->GetItemAsObject(this->NumCards - 1 - level);
        this->Cards->AddItem(o);
    }

    tmpCards->RemoveAllItems();
    tmpCards->Delete();

    SetCardZoom(0);
}


void vtkCardManager::SetVisibility(int v) {
    int i;
    bool vb = true;
    if (v == 0) 
    {
        vb = false;
    }
    for (i = 0; i < this->Cards->GetNumberOfItems(); i++) {
        vtkCard *card = (vtkCard *)(this->Cards->GetItemAsObject(i));
        card->SetVisibility(vb);
    }
}


void vtkCardManager::SetCardZoom(vtkFloatingPointType c) {
    if (c < 0) c = 0;
    if (c > this->NumCards - 1) c = this->NumCards - 1;

    this->CardZoom = c;

    if (this->Renderer == NULL) return;

    vtkFloatingPointType posZ = c * this->CardSpacing;
    
    int i;
    for (i = 0; i < this->Cards->GetNumberOfItems(); i++) {
        vtkCard *card = (vtkCard *)(this->Cards->GetItemAsObject(i));

        // NOTE - get an odd vtkFloatingPointType-render jump, if a GetPosition is used before the SetPosition + Center
        //  - SetPosition + Center is fine w/o it, tho
        vtkFloatingPointType pos[3];
        pos[0] = pos[1] = pos[2] = 0;

        // v52 - use GetPosition for relative positioning - now it seems to work fine...
        card->GetPosition(pos);

        card->SetPosition(pos[0], pos[1], (vtkFloatingPointType)card->GetLevelNum() * (-this->CardSpacing) + posZ);
        
        // do the card centering only durint setup
        //card->Center();
        //card->CenterOffset();

        int cFloor = (int)(floor(c));
        if (card->GetLevelNum() >= cFloor) {
            card->SetVisibility(1);
            card->SetOpacityBase(1.0);

            // the card we're passing becomes semi-trans
            if (c - card->GetLevelNum() >= 0) card->SetOpacityBase(1.0 - (c - card->GetLevelNum()));
        }
        else {
            card->SetVisibility(0);
        }
    }

    // v47 - if zooming in, set the camera to achieve the zoom and keep focus on the scaling-down cards
    if (this->ScaleDownFlag) {
        vtkFloatingPointType sc = pow(((vtkFloatingPointType)c + 1.0), ScaleDownPower);
        vtkFloatingPointType fov = 30.0 / sc;

        //printf("angle scale: %g,  fov: %g\n", sc, fov);

        this->Renderer->GetActiveCamera()->SetViewAngle(fov);
        this->Renderer->GetActiveCamera()->SetPosition(0, 0, 40);
        this->Renderer->GetActiveCamera()->SetFocalPoint(0, 0, 0);
    }
}


void vtkCardManager::PrintSelf(ostream& os, vtkIndent indent)
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
