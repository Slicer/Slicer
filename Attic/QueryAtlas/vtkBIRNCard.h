/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkBIRNCard.h,v $
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


// .NAME vtkBIRNCard - create an array of quadrilaterals located in a plane
// .SECTION Description


#ifndef __vtkBIRNCard_h
#define __vtkBIRNCard_h

#include "vtkCard.h"

#include "vtkQueryAtlasWin32Header.h"


// TODO - limit to filepath char length
#define BIRN_PATH_LEN (512)


class VTK_QUERYATLAS_EXPORT vtkBIRNCard : public vtkCard
{
public:
    void PrintSelf(ostream& os, vtkIndent indent);
    vtkTypeRevisionMacro(vtkBIRNCard,vtkCard);

    // Description:
    // Manage a set of vtkTextureText objects with a surrounding rectangle-box.
    static vtkBIRNCard *New();

    void CreateCard();
    void ActivateLink();

    // TODO - remake card objects if the set-values change?  use Modified() ?  how much to track & cleanup?

    vtkGetStringMacro(SearchString);
    vtkSetStringMacro(SearchString);

    vtkGetStringMacro(DirBase);
    vtkSetStringMacro(DirBase);


protected:
    vtkBIRNCard();
    ~vtkBIRNCard();

    char *SearchString;
    char *DirBase;
    char *DirName;


private:
    vtkBIRNCard(const vtkBIRNCard&);  // Not implemented.
    void operator=(const vtkBIRNCard&);  // Not implemented.
};

#endif


