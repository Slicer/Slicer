/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageFastSignedChamferTcl.cxx,v $
  Date:      $Date: 2006/01/06 17:57:54 $
  Version:   $Revision: 1.2 $

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

// tcl wrapper for vtkImageFastSignedChamfer object
//
#include "vtkSystemIncludes.h"
#include "vtkImageFastSignedChamfer.h"

#include "vtkTclUtil.h"

ClientData vtkImageFastSignedChamferNewCommand()
{
  vtkImageFastSignedChamfer *temp = vtkImageFastSignedChamfer::New();
  return ((ClientData)temp);
}

int vtkImageToImageFilterCppCommand(vtkImageToImageFilter *op, Tcl_Interp *interp,
             int argc, char *argv[]);
int VTKTCL_EXPORT vtkImageFastSignedChamferCppCommand(vtkImageFastSignedChamfer *op, Tcl_Interp *interp,
             int argc, char *argv[]);

int VTKTCL_EXPORT vtkImageFastSignedChamferCommand(ClientData cd, Tcl_Interp *interp,
             int argc, char *argv[])
{
  if ((argc == 2)&&(!strcmp("Delete",argv[1]))&& !vtkTclInDelete(interp))
    {
    Tcl_DeleteCommand(interp,argv[0]);
    return TCL_OK;
    }
   return vtkImageFastSignedChamferCppCommand((vtkImageFastSignedChamfer *)(((vtkTclCommandArgStruct *)cd)->Pointer),interp, argc, argv);
}

int VTKTCL_EXPORT vtkImageFastSignedChamferCppCommand(vtkImageFastSignedChamfer *op, Tcl_Interp *interp,
             int argc, char *argv[])
{
  int    tempi;
  double tempd;
  static char temps[80];
  int    error;

  error = 0; error = error;
  tempi = 0; tempi = tempi;
  tempd = 0; tempd = tempd;
  temps[0] = 0; temps[0] = temps[0];

  if (argc < 2)
    {
    Tcl_SetResult(interp, (char *) "Could not find requested method.", TCL_VOLATILE);
    return TCL_ERROR;
    }
  if (!interp)
    {
    if (!strcmp("DoTypecasting",argv[0]))
      {
      if (!strcmp("vtkImageFastSignedChamfer",argv[1]))
        {
        argv[2] = (char *)((void *)op);
        return TCL_OK;
        }
      if (vtkImageToImageFilterCppCommand((vtkImageToImageFilter *)op,interp,argc,argv) == TCL_OK)
        {
        return TCL_OK;
        }
      }
    return TCL_ERROR;
    }

  if (!strcmp("GetSuperClassName",argv[1]))
    {
    Tcl_SetResult(interp,(char *) "vtkImageToImageFilter", TCL_VOLATILE);
    return TCL_OK;
    }

  if ((!strcmp("New",argv[1]))&&(argc == 2))
    {
    vtkImageFastSignedChamfer  *temp20;
    int vtkImageFastSignedChamferCommand(ClientData, Tcl_Interp *, int, char *[]);
    error = 0;

    if (!error)
      {
      temp20 = (op)->New();
      vtkTclGetObjectFromPointer(interp,(void *)temp20,vtkImageFastSignedChamferCommand);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetClassName",argv[1]))&&(argc == 2))
    {
    const char    *temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetClassName();
      if (temp20)
        {
        Tcl_SetResult(interp, (char*)temp20, TCL_VOLATILE);
        }
      else
        {
        Tcl_ResetResult(interp);
        }
      return TCL_OK;
      }
    }
  if ((!strcmp("IsA",argv[1]))&&(argc == 3))
    {
    char    *temp0;
    int      temp20;
    error = 0;

    temp0 = argv[2];
    if (!error)
      {
      temp20 = (op)->IsA(temp0);
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("Setmaxdist",argv[1]))&&(argc == 3))
    {
    float    temp0;
    error = 0;

    if (Tcl_GetDouble(interp,argv[2],&tempd) != TCL_OK) error = 1;
    temp0 = tempd;
    if (!error)
      {
      op->Setmaxdist(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("Getmaxdist",argv[1]))&&(argc == 2))
    {
    float    temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->Getmaxdist();
      char tempResult[1024];
      sprintf(tempResult,"%g",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("Setcoeff_a",argv[1]))&&(argc == 3))
    {
    float    temp0;
    error = 0;

    if (Tcl_GetDouble(interp,argv[2],&tempd) != TCL_OK) error = 1;
    temp0 = tempd;
    if (!error)
      {
      op->Setcoeff_a(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("Getcoeff_a",argv[1]))&&(argc == 2))
    {
    float    temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->Getcoeff_a();
      char tempResult[1024];
      sprintf(tempResult,"%g",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("Setcoeff_b",argv[1]))&&(argc == 3))
    {
    float    temp0;
    error = 0;

    if (Tcl_GetDouble(interp,argv[2],&tempd) != TCL_OK) error = 1;
    temp0 = tempd;
    if (!error)
      {
      op->Setcoeff_b(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("Getcoeff_b",argv[1]))&&(argc == 2))
    {
    float    temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->Getcoeff_b();
      char tempResult[1024];
      sprintf(tempResult,"%g",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("Setcoeff_c",argv[1]))&&(argc == 3))
    {
    float    temp0;
    error = 0;

    if (Tcl_GetDouble(interp,argv[2],&tempd) != TCL_OK) error = 1;
    temp0 = tempd;
    if (!error)
      {
      op->Setcoeff_c(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("Getcoeff_c",argv[1]))&&(argc == 2))
    {
    float    temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->Getcoeff_c();
      char tempResult[1024];
      sprintf(tempResult,"%g",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("Setnoborder",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->Setnoborder(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("Getnoborder",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->Getnoborder();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("InitParam",argv[1]))&&(argc == 4))
    {
    vtkImageData  *temp0;
    vtkImageData  *temp1;
    error = 0;

    temp0 = (vtkImageData *)(vtkTclGetPointerFromObject(argv[2],(char *) "vtkImageData",interp,error));
    temp1 = (vtkImageData *)(vtkTclGetPointerFromObject(argv[3],(char *) "vtkImageData",interp,error));
    if (!error)
      {
      op->InitParam(temp0,temp1);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }

  if (!strcmp("ListInstances",argv[1]))
    {
    vtkTclListInstances(interp,(ClientData)vtkImageFastSignedChamferCommand);
    return TCL_OK;
    }

  if (!strcmp("ListMethods",argv[1]))
    {
    vtkImageToImageFilterCppCommand(op,interp,argc,argv);
    Tcl_AppendResult(interp,"Methods from vtkImageFastSignedChamfer:\n",NULL);
    Tcl_AppendResult(interp,"  GetSuperClassName\n",NULL);
    Tcl_AppendResult(interp,"  New\n",NULL);
    Tcl_AppendResult(interp,"  GetClassName\n",NULL);
    Tcl_AppendResult(interp,"  IsA\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  Setmaxdist\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  Getmaxdist\n",NULL);
    Tcl_AppendResult(interp,"  Setcoeff_a\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  Getcoeff_a\n",NULL);
    Tcl_AppendResult(interp,"  Setcoeff_b\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  Getcoeff_b\n",NULL);
    Tcl_AppendResult(interp,"  Setcoeff_c\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  Getcoeff_c\n",NULL);
    Tcl_AppendResult(interp,"  Setnoborder\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  Getnoborder\n",NULL);
    Tcl_AppendResult(interp,"  InitParam\t with 2 args\n",NULL);
    return TCL_OK;
    }

  if (vtkImageToImageFilterCppCommand((vtkImageToImageFilter *)op,interp,argc,argv) == TCL_OK)
    {
    return TCL_OK;
    }

  if ((argc >= 2)&&(!strstr(interp->result,"Object named:")))
    {
    char temps2[256];
    sprintf(temps2,"Object named: %s, could not find requested method: %s\nor the method was called with incorrect arguments.\n",argv[0],argv[1]);
    Tcl_AppendResult(interp,temps2,NULL);
    }
  return TCL_ERROR;
}
