/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkBIRNCardManager.cxx,v $
  Date:      $Date: 2006/05/26 19:59:40 $
  Version:   $Revision: 1.3 $

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

#include "vtkBIRNCardManager.h"
#include "vtkBIRNCard.h"


vtkCxxRevisionMacro(vtkBIRNCardManager, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkBIRNCardManager);


vtkBIRNCardManager::vtkBIRNCardManager()
{
    this->NumCards = 10;

    this->DirBase = NULL;
    this->SearchString = NULL;
}


vtkBIRNCardManager::~vtkBIRNCardManager() {
}


void vtkBIRNCardManager::SetDirBase(char *d) {
    // add a trailing '/' if needed (alloc an extra char for it)
    this->DirBase = (char *)malloc(strlen(d) + 2);
    strcpy(this->DirBase, d);
    int l = strlen(this->DirBase);

    if (this->DirBase[l-1] != '/' && this->DirBase[l-1] != '\\')
        this->DirBase = strcat(this->DirBase, "/");
}


vtkCard *vtkBIRNCardManager::NewCard() {
    return vtkBIRNCard::New();
}


// sub-classes override this to do specific card setup

void vtkBIRNCardManager::ExtraCardInit(vtkCard *c) {
    vtkBIRNCard *bc = (vtkBIRNCard *)c;
    bc->SetDirBase(this->DirBase);
    bc->SetSearchString(this->SearchString);
    bc->CreateCard();
}


// sub-classes override this to prapare to make specific kinds of cards

void vtkBIRNCardManager::ExtraLoadInit() {
    // TODO - string limits
    char searchcat[512];
    char searchFile[BIRN_PATH_LEN];

    // The trailing '/' is added ahead of time - since DirBase is protected, the accessor fn will ensure the trailing '/'

    sprintf(searchFile, "%sSearch.txt", this->DirBase);
    char *search = vtkCard::LoadString(searchFile);

    if (search != NULL) {
        sprintf(searchcat, "Search String: %s", search);
        SetSearchString(searchcat);
    }

    free (search);
}


void vtkBIRNCardManager::ActivateLink() {
    int l = (int)(this->CardZoom + 0.5);
    printf("vtkBIRNCardManager::ActivateLink: level %f,  %d,  %d\n", this->CardZoom, (int)(this->CardZoom), l);

    // NOTE - based on the list being backwards
    vtkBIRNCard *card = (vtkBIRNCard *)(this->Cards->GetItemAsObject(this->NumCards -1 - l));
    card->ActivateLink();
}


void vtkBIRNCardManager::PrintSelf(ostream& os, vtkIndent indent)
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

