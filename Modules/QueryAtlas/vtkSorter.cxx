/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSorter.cxx,v $
  Date:      $Date: 2006/05/26 19:59:44 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

/*
Authors: Michael McKenna, David Small, Steve Pieper.
    Small Design Firm, Inc.,
    in association with Isomics, Inc.,
    and Partners Health Care.  MA, USA.
    June, 2004

===========================================================*/


#include "vtkSorter.h"
#include "vtkObject.h"
#include "vtkRenderer.h"
#include "vtkActorCollection.h"
//#include "vtkRendererCollection.h"
#include "vtkActor.h"
//#include "vtkRenderWindow.h"
#include "vtkMapper.h"
#include "vtkCamera.h"
#include "vtkMath.h"
#include "vtkFollower.h"

#include "vtkCard.h"
#include "vtkTextureText.h"


vtkCxxRevisionMacro(vtkSorter, "$Revision: 1.3 $");
vtkStandardNewMacro(vtkSorter);


vtkSorter::vtkSorter()
{
    this->LastDepth = NULL;
    this->LastDepthNum = 0;
    this->AlwaysResort = 0;
    this->SelectedItem = NULL;
    this->Renderer = NULL;
    this->Cards = vtkCollection::New();
    this->TextureTexts = vtkCollection::New();
}


vtkSorter::~vtkSorter() { 
}


// originally from: vtk 4.2:  void vtkDepthSortPolyData::ComputeProjectionVector(vtkFloatingPointType vector[3], vtkFloatingPointType origin[3])
//   There are more options for the class in general in the original vtkDepthSortPolyData - so look there for more stuff.

void vtkSorter::ComputeProjectionVector(vtkCamera *c, vtkFloatingPointType vector[3], vtkFloatingPointType origin[3])
{
    vtkFloatingPointType *focalPoint = c->GetFocalPoint();
    vtkFloatingPointType *position = c->GetPosition();

    for(int i=0; i<3; i++)
    { 
        vector[i] = focalPoint[i] - position[i];
        origin[i] = position[i];
    }
}



// refs to vtkDepthSortPolyData

extern "C" 
{  
    int vtkCompareBackToFront(const void *val1, const void *val2)
    {
        if (((vtkSorterValues *)val1)->z > ((vtkSorterValues *)val2)->z)
        {
            return (-1);
        }
        else if (((vtkSorterValues *)val1)->z < ((vtkSorterValues *)val2)->z)
        {
            return (1);
        }
        else
        {
            return (0);
        }
    }
}


void vtkSorter::SetSelectedItem(vtkObject *o) {
    if (o == NULL) SelectedItem = NULL;
    else if (o->IsA("vtkCard")) SelectedItem = o;
    else if (o->IsA("vtkTextureText")) SelectedItem = o;
    else vtkErrorMacro("vtkSorter::SetSelectedItem: ERROR - incorrect object type\n");
}


void vtkSorter::DepthSort() {
    //printf("vtkCard::DepthSort\n");

    // Specified text cards and plain text items are sorted

    if ( this->Renderer == NULL )
    {   return;
    }

    vtkRenderer *r = this->Renderer;
    vtkCollection *cards = this->Cards;
    vtkCollection *texts = this->TextureTexts;

    // TODO - we could add in more of the sorting options that are in vtkDepthSortPolyData - but i think this is really all we need.
    // stripped down vtk 4.2: void vtkDepthSortPolyData::Execute() :

    vtkFloatingPointType vector[3], origin[3];
    ComputeProjectionVector(r->GetActiveCamera(), vector, origin);

    int numCards = cards->GetNumberOfItems();
    int numTexts = texts->GetNumberOfItems();
    int numItems = numCards + numTexts;

    vtkSorterValues *depth = new vtkSorterValues[numItems];

    // TODO - now that i'm using vtkCollection instead of vector, i think i could consolidate the confusing vtkFloatingPointType list thing

    // get cards positions
    for ( int cellId = 0; cellId < numCards; cellId++ )
    {
        vtkFloatingPointType x[3];
        vtkFloatingPointType pos[3];
        ((vtkCard *)cards->GetItemAsObject(cellId))->GetMainText()->GetFollower()->GetPosition(pos);
        x[0] = pos[0] - origin[0];
        x[1] = pos[1] - origin[1];
        x[2] = pos[2] - origin[2];
        depth[cellId].z = vtkMath::Dot(x,vector);
        depth[cellId].cellId = cellId;
        // TODO - enum cellType
        depth[cellId].cellType = 0;
    }

    // get texts positions
    for ( int cellId = 0; cellId < numTexts; cellId++ )
    {
        vtkFloatingPointType x[3];
        vtkFloatingPointType pos[3];
        ((vtkTextureText *)texts->GetItemAsObject(cellId))->GetFollower()->GetPosition(pos);
        x[0] = pos[0] - origin[0];
        x[1] = pos[1] - origin[1];
        x[2] = pos[2] - origin[2];
        // compensate for first set added : numCards
        depth[cellId + numCards].z = vtkMath::Dot(x,vector);
        depth[cellId + numCards].cellId = cellId + numCards;
        depth[cellId + numCards].cellType = 1;
    }

    qsort((void *)depth, numItems, sizeof(vtkSorterValues), vtkCompareBackToFront);

    //for (int i = 0; i < numItems; i++) printf("sorted: %d\n", depth[i].cellId);

    // always sort will give you a slower, but consistant frame-rate
    if (!AlwaysResort) {
        // compare new sort to last sort - if it's not changed, don't remove/add, which slows things down
        int same = 0;
        if (LastDepth != NULL) {
            if (LastDepthNum == numItems) {
                for (int i = 0; i < numItems; i++) {
                    if (LastDepth[i].cellId == depth[i].cellId) same++;
                }
            }
        }
        if (same == numItems) {
            // no change in depth, don't re-sort.  no need to re-assign LastDepth
            delete [] depth;
            return;
        }
    }

    int i;
    //for (i = 0; i < numCards; i++) ((vtkCard *)cards->GetItemAsObject(i))->RemoveActors(r);
    for (i = 0; i < numCards; i++) ((vtkCard *)cards->GetItemAsObject(i))->RemoveActors();
    for (i = 0; i < numTexts; i++) r->RemoveActor(((vtkTextureText *)texts->GetItemAsObject(i))->GetFollower());

    for (i = 0; i < numItems; i++) {
        if (depth[i].cellType == 0) ((vtkCard *)cards->GetItemAsObject(depth[i].cellId))->AddActors(r);
        else if (depth[i].cellType == 1) {
            r->AddActor(((vtkTextureText *)texts->GetItemAsObject(depth[i].cellId - numCards))->GetFollower());
        }
    }

    // if there is a 'selected' item, dim out the objects in front of it

    int selectedI = 1000000;
    if (this->SelectedItem != NULL) {
        for (i = 0; i < numItems; i++) {
            // Cards
            if (depth[i].cellType == 0) {
                if (SelectedItem->IsA("vtkCard")) {
                    if ((vtkCard *)cards->GetItemAsObject(depth[i].cellId) == (vtkCard *)SelectedItem) {
                        selectedI = i;
                    }
                }
            }
            // TextureTexts
            else if (depth[i].cellType == 1) {
                if (SelectedItem->IsA("vtkTextureText")) {
                    if ((vtkTextureText *)texts->GetItemAsObject(depth[i].cellId - numCards) == (vtkTextureText *)SelectedItem) {
                        selectedI = i;
                    }
                }
            }
        }
    }


    // now set the dimming forward of the selected layer
    for (i = 0; i < numItems; i++) {
        // Cards
        if (depth[i].cellType == 0) {
            if (i <= selectedI) ((vtkCard *)cards->GetItemAsObject(depth[i].cellId))->SetDim(0);
            else ((vtkCard *)cards->GetItemAsObject(depth[i].cellId))->SetDim(1);
        }
        // TextureTexts
        else if (depth[i].cellType == 1) {
            if (i <= selectedI) ((vtkTextureText *)texts->GetItemAsObject(depth[i].cellId - numCards))->SetDim(0);
            else ((vtkTextureText *)texts->GetItemAsObject(depth[i].cellId - numCards))->SetDim(1);
        }
    }


    if (AlwaysResort) {
        delete [] depth;
        LastDepth = NULL;
    }
    else {
        if (LastDepth != NULL) delete [] LastDepth;
        LastDepth = depth;
        LastDepthNum = numItems;
    }
}


void vtkSorter::PrintSelf(ostream& os, vtkIndent indent)
{
    //  this->Superclass::PrintSelf(os,indent);

    /*  TODO - do print
    os << indent << "X Resolution: " << this->XResolution << "\n";
    os << indent << "Y Resolution: " << this->YResolution << "\n";
    ...
    */

}
