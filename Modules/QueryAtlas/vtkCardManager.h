/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkCardManager.h,v $
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


// .NAME vtkCardManager - manage a set of vtkCards - load and animate them.
// .SECTION Description


#ifndef __vtkCardManager_h
#define __vtkCardManager_h


#include "vtkObject.h"
#include "vtkRenderer.h"
#include "vtkSorter.h"

#include <vtkQueryAtlasConfigure.h>


class vtkCollection;

class vtkCard;


class VTK_QUERYATLAS_EXPORT vtkCardManager : public vtkObject
{
public:
    void PrintSelf(ostream& os, vtkIndent indent);
    vtkTypeRevisionMacro(vtkCardManager,vtkObject);

    static vtkCardManager *New();

    virtual vtkCard *NewCard();
    virtual void ExtraCardInit(vtkCard *c);
    virtual void ExtraLoadInit();

    virtual void LoadSet();
    void LoadSet(vtkRenderer *ren);
    //void LoadSet(char *dirBase, vtkRenderer *ren);

    //void ActivateLink();

    void SetVisibility(int v);

    vtkCollection *Cards;

    vtkGetMacro(CardZoom, vtkFloatingPointType);
    virtual void SetCardZoom(vtkFloatingPointType c);

    vtkGetMacro(NumCards, int);
    vtkSetMacro(NumCards, int);

    vtkGetObjectMacro(Renderer, vtkRenderer);
    vtkSetObjectMacro(Renderer, vtkRenderer);

    vtkGetObjectMacro(Sorter, vtkSorter);
    vtkSetObjectMacro(Sorter, vtkSorter);

    vtkGetMacro(CardSpacing, vtkFloatingPointType);
    vtkSetMacro(CardSpacing, vtkFloatingPointType);

    vtkGetMacro(ScaleCards, vtkFloatingPointType);
    vtkSetMacro(ScaleCards, vtkFloatingPointType);

    vtkGetMacro(ScaleDownPower, vtkFloatingPointType);
    vtkSetMacro(ScaleDownPower, vtkFloatingPointType);

    vtkGetMacro(ScaleDownFlag, int);
    vtkSetMacro(ScaleDownFlag, int);

    vtkGetMacro(UseFollower, int);
    vtkSetMacro(UseFollower, int);

    //vtkGetStringMacro(DirBase);
    //void SetDirBase(char *);

    //vtkGetStringMacro(SearchString);
    //vtkSetStringMacro(SearchString);



protected:
    vtkCardManager();
    ~vtkCardManager();


    vtkFloatingPointType CardSpacing;
    vtkFloatingPointType CardZoom;
    int NumCards;
    vtkRenderer *Renderer;

    // NOTE - set the Sorter before loading the card set w/ LoadSet()
    vtkSorter *Sorter;

    // param to set before LoadSet to scale all the cards
    vtkFloatingPointType ScaleCards;
    vtkFloatingPointType ScaleDownPower;
    int ScaleDownFlag;

    //char *DirBase;
    //char *SearchString;

    int UseFollower;


private:
    vtkCardManager(const vtkCardManager&);  // Not implemented.
    void operator=(const vtkCardManager&);  // Not implemented.
};

#endif
