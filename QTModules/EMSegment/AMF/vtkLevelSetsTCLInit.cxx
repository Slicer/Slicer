/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkLevelSetsTCLInit.cxx,v $
  Date:      $Date: 2006/01/06 17:57:55 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
/*  ==================================================
    Module : vtkLevelSets
    Authors: Karl Krissian (from initial code by Liana Lorigo and Renaud Keriven)
    Email  : karl@bwh.harvard.edu

    This module implements a Active Contour evolution
    for segmentation of 2D and 3D images.
    It implements a 'codimension 2' levelsets as an
    option for the smoothing term.
    It comes with a Tcl/Tk interface for the '3D Slicer'.
    ==================================================
    Copyright (C) 2003  LMI, Laboratory of Mathematics in Imaging, 
    Brigham and Women's Hospital, Boston MA USA

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    ================================================== 
   The full GNU Lesser General Public License file is in vtkLevelSets/LesserGPL_license.txt
*/

#include "vtkTclUtil.h"
int vtkLevelSetFastMarchingCommand(ClientData cd, Tcl_Interp *interp,
             int argc, char *argv[]);
ClientData vtkLevelSetFastMarchingNewCommand();
int vtkImageIsoContourDistCommand(ClientData cd, Tcl_Interp *interp,
             int argc, char *argv[]);
ClientData vtkImageIsoContourDistNewCommand();
int vtkImageFastSignedChamferCommand(ClientData cd, Tcl_Interp *interp,
             int argc, char *argv[]);
ClientData vtkImageFastSignedChamferNewCommand();
int vtkLevelSetsCommand(ClientData cd, Tcl_Interp *interp,
             int argc, char *argv[]);
ClientData vtkLevelSetsNewCommand();

extern Tcl_HashTable vtkInstanceLookup;
extern Tcl_HashTable vtkPointerLookup;
extern Tcl_HashTable vtkCommandLookup;
extern void vtkTclDeleteObjectFromHash(void *);
extern void vtkTclListInstances(Tcl_Interp *interp, ClientData arg);


extern "C" {int VTK_EXPORT Vtklevelsetstcl_SafeInit(Tcl_Interp *interp);}

extern "C" {int VTK_EXPORT Vtklevelsetstcl_Init(Tcl_Interp *interp);}

extern void vtkTclGenericDeleteObject(ClientData cd);


int VTK_EXPORT Vtklevelsetstcl_SafeInit(Tcl_Interp *interp)
{
  return Vtklevelsetstcl_Init(interp);
}


int VTK_EXPORT Vtklevelsetstcl_Init(Tcl_Interp *interp)
{

  vtkTclCreateNew(interp,(char *) "vtkLevelSetFastMarching", vtkLevelSetFastMarchingNewCommand,
                  vtkLevelSetFastMarchingCommand);
  vtkTclCreateNew(interp,(char *) "vtkImageIsoContourDist", vtkImageIsoContourDistNewCommand,
                  vtkImageIsoContourDistCommand);
  vtkTclCreateNew(interp,(char *) "vtkImageFastSignedChamfer", vtkImageFastSignedChamferNewCommand,
                  vtkImageFastSignedChamferCommand);
  vtkTclCreateNew(interp,(char *) "vtkLevelSets", vtkLevelSetsNewCommand,
                  vtkLevelSetsCommand);
  return TCL_OK;
}
