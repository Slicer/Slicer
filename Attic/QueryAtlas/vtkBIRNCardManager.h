/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkBIRNCardManager.h,v $
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


// .NAME vtkBIRNCardManager - manage a set of vtkBIRNCards - load and animate them.
// .SECTION Description


#ifndef __vtkBIRNCardManager_h
#define __vtkBIRNCardManager_h

#include "vtkCardManager.h"

#include "vtkObject.h"
#include "vtkRenderer.h"
#include "vtkSorter.h"

#include <vtkQueryAtlasConfigure.h>


class vtkCollection;

class vtkBIRNCard;


class VTK_QUERYATLAS_EXPORT vtkBIRNCardManager : public vtkCardManager
{
public:
    void PrintSelf(ostream& os, vtkIndent indent);
    vtkTypeRevisionMacro(vtkBIRNCardManager,vtkCardManager);

    static vtkBIRNCardManager *New();

    virtual vtkCard *NewCard();
    virtual void ExtraCardInit(vtkCard *c);
    virtual void ExtraLoadInit();

    void ActivateLink();


    vtkGetStringMacro(DirBase);
    void SetDirBase(char *);

    vtkGetStringMacro(SearchString);
    vtkSetStringMacro(SearchString);


protected:
    vtkBIRNCardManager();
    ~vtkBIRNCardManager();


    char *DirBase;
    char *SearchString;


private:
    vtkBIRNCardManager(const vtkBIRNCardManager&);  // Not implemented.
    void operator=(const vtkBIRNCardManager&);  // Not implemented.
};

#endif
