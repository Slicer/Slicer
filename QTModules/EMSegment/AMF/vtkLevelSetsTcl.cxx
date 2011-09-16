/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkLevelSetsTcl.cxx,v $
  Date:      $Date: 2006/01/06 17:57:55 $
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

// tcl wrapper for vtkLevelSets object
//
#include "vtkSystemIncludes.h"
#include "vtkLevelSets.h"

#include "vtkTclUtil.h"

ClientData vtkLevelSetsNewCommand()
{
  vtkLevelSets *temp = vtkLevelSets::New();
  return ((ClientData)temp);
}

int vtkImageToImageFilterCppCommand(vtkImageToImageFilter *op, Tcl_Interp *interp,
             int argc, char *argv[]);
int VTKTCL_EXPORT vtkLevelSetsCppCommand(vtkLevelSets *op, Tcl_Interp *interp,
             int argc, char *argv[]);

int VTKTCL_EXPORT vtkLevelSetsCommand(ClientData cd, Tcl_Interp *interp,
             int argc, char *argv[])
{
  if ((argc == 2)&&(!strcmp("Delete",argv[1]))&& !vtkTclInDelete(interp))
    {
    Tcl_DeleteCommand(interp,argv[0]);
    return TCL_OK;
    }
   return vtkLevelSetsCppCommand((vtkLevelSets *)(((vtkTclCommandArgStruct *)cd)->Pointer),interp, argc, argv);
}

int VTKTCL_EXPORT vtkLevelSetsCppCommand(vtkLevelSets *op, Tcl_Interp *interp,
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
      if (!strcmp("vtkLevelSets",argv[1]))
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
    vtkLevelSets  *temp20;
    int vtkLevelSetsCommand(ClientData, Tcl_Interp *, int, char *[]);
    error = 0;

    if (!error)
      {
      temp20 = (op)->New();
      vtkTclGetObjectFromPointer(interp,(void *)temp20,vtkLevelSetsCommand);
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
  if ((!strcmp("SetRescaleImage",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->SetRescaleImage(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetRescaleImage",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetRescaleImage();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("RescaleImageOn",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->RescaleImageOn();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("RescaleImageOff",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->RescaleImageOff();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetInitThreshold",argv[1]))&&(argc == 3))
    {
    float    temp0;
    error = 0;

    if (Tcl_GetDouble(interp,argv[2],&tempd) != TCL_OK) error = 1;
    temp0 = tempd;
    if (!error)
      {
      op->SetInitThreshold(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetInitThreshold",argv[1]))&&(argc == 2))
    {
    float    temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetInitThreshold();
      char tempResult[1024];
      sprintf(tempResult,"%g",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetUseLowThreshold",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->SetUseLowThreshold(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetUseLowThreshold",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetUseLowThreshold();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("UseLowThresholdOn",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->UseLowThresholdOn();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("UseLowThresholdOff",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->UseLowThresholdOff();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetLowThreshold",argv[1]))&&(argc == 3))
    {
    float    temp0;
    error = 0;

    if (Tcl_GetDouble(interp,argv[2],&tempd) != TCL_OK) error = 1;
    temp0 = tempd;
    if (!error)
      {
      op->SetLowThreshold(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetLowThreshold",argv[1]))&&(argc == 2))
    {
    float    temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetLowThreshold();
      char tempResult[1024];
      sprintf(tempResult,"%g",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetUseHighThreshold",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->SetUseHighThreshold(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetUseHighThreshold",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetUseHighThreshold();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("UseHighThresholdOn",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->UseHighThresholdOn();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("UseHighThresholdOff",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->UseHighThresholdOff();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetHighThreshold",argv[1]))&&(argc == 3))
    {
    float    temp0;
    error = 0;

    if (Tcl_GetDouble(interp,argv[2],&tempd) != TCL_OK) error = 1;
    temp0 = tempd;
    if (!error)
      {
      op->SetHighThreshold(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetHighThreshold",argv[1]))&&(argc == 2))
    {
    float    temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetHighThreshold();
      char tempResult[1024];
      sprintf(tempResult,"%g",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetProbabilityHighThreshold",argv[1]))&&(argc == 3))
    {
    float    temp0;
    error = 0;

    if (Tcl_GetDouble(interp,argv[2],&tempd) != TCL_OK) error = 1;
    temp0 = tempd;
    if (!error)
      {
      op->SetProbabilityHighThreshold(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetProbabilityHighThreshold",argv[1]))&&(argc == 2))
    {
    float    temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetProbabilityHighThreshold();
      char tempResult[1024];
      sprintf(tempResult,"%g",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetStepDt",argv[1]))&&(argc == 3))
    {
    float    temp0;
    error = 0;

    if (Tcl_GetDouble(interp,argv[2],&tempd) != TCL_OK) error = 1;
    temp0 = tempd;
    if (!error)
      {
      op->SetStepDt(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetStepDt",argv[1]))&&(argc == 2))
    {
    float    temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetStepDt();
      char tempResult[1024];
      sprintf(tempResult,"%g",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetCheckFreq",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->SetCheckFreq(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetCheckFreq",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetCheckFreq();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetNumIters",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->SetNumIters(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetNumIters",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetNumIters();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetReinitFreq",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->SetReinitFreq(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetReinitFreq",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetReinitFreq();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetConvergedThresh",argv[1]))&&(argc == 3))
    {
    float    temp0;
    error = 0;

    if (Tcl_GetDouble(interp,argv[2],&tempd) != TCL_OK) error = 1;
    temp0 = tempd;
    if (!error)
      {
      op->SetConvergedThresh(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetConvergedThresh",argv[1]))&&(argc == 2))
    {
    float    temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetConvergedThresh();
      char tempResult[1024];
      sprintf(tempResult,"%g",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetAdvectionCoeff",argv[1]))&&(argc == 3))
    {
    float    temp0;
    error = 0;

    if (Tcl_GetDouble(interp,argv[2],&tempd) != TCL_OK) error = 1;
    temp0 = tempd;
    if (!error)
      {
      op->SetAdvectionCoeff(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetAdvectionCoeff",argv[1]))&&(argc == 2))
    {
    float    temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetAdvectionCoeff();
      char tempResult[1024];
      sprintf(tempResult,"%g",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetDoMean",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->SetDoMean(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetDoMean",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetDoMean();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("DoMeanOn",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->DoMeanOn();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("DoMeanOff",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->DoMeanOff();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetBand",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->SetBand(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetBand",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetBand();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetTube",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->SetTube(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetTube",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetTube();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetDMmethod",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->SetDMmethod(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetDMmethod",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetDMmethod();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetEvolveThreads",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->SetEvolveThreads(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetEvolveThreads",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetEvolveThreads();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("Setsavedistmap",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->Setsavedistmap(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("Getsavedistmap",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->Getsavedistmap();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("savedistmapOn",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->savedistmapOn();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("savedistmapOff",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->savedistmapOff();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("Setsavesecdergrad",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->Setsavesecdergrad(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("Getsavesecdergrad",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->Getsavesecdergrad();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("savesecdergradOn",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->savesecdergradOn();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("savesecdergradOff",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->savesecdergradOff();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("Setadvection_scheme",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->Setadvection_scheme(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("Getadvection_scheme",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->Getadvection_scheme();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("advection_schemeOn",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->advection_schemeOn();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("advection_schemeOff",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->advection_schemeOff();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetUseCosTerm",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->SetUseCosTerm(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetUseCosTerm",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetUseCosTerm();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("UseCosTermOn",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->UseCosTermOn();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("UseCosTermOff",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->UseCosTermOff();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetIsoContourBin",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->SetIsoContourBin(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetIsoContourBin",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetIsoContourBin();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("IsoContourBinOn",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->IsoContourBinOn();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("IsoContourBinOff",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->IsoContourBinOff();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetDimension",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->SetDimension(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetDimension",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetDimension();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetSliceNum",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->SetSliceNum(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetSliceNum",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetSliceNum();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("Setvelocity",argv[1]))&&(argc == 3))
    {
    vtkImageData  *temp0;
    error = 0;

    temp0 = (vtkImageData *)(vtkTclGetPointerFromObject(argv[2],(char *) "vtkImageData",interp,error));
    if (!error)
      {
      op->Setvelocity(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("Getvelocity",argv[1]))&&(argc == 2))
    {
    vtkImageData  *temp20;
    int vtkImageDataCommand(ClientData, Tcl_Interp *, int, char *[]);
    error = 0;

    if (!error)
      {
      temp20 = (op)->Getvelocity();
      vtkTclGetObjectFromPointer(interp,(void *)temp20,vtkImageDataCommand);
      return TCL_OK;
      }
    }
  if ((!strcmp("Setcoeff_velocity",argv[1]))&&(argc == 3))
    {
    float    temp0;
    error = 0;

    if (Tcl_GetDouble(interp,argv[2],&tempd) != TCL_OK) error = 1;
    temp0 = tempd;
    if (!error)
      {
      op->Setcoeff_velocity(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("Getcoeff_velocity",argv[1]))&&(argc == 2))
    {
    float    temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->Getcoeff_velocity();
      char tempResult[1024];
      sprintf(tempResult,"%g",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("Setballoon_coeff",argv[1]))&&(argc == 3))
    {
    float    temp0;
    error = 0;

    if (Tcl_GetDouble(interp,argv[2],&tempd) != TCL_OK) error = 1;
    temp0 = tempd;
    if (!error)
      {
      op->Setballoon_coeff(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("Getballoon_coeff",argv[1]))&&(argc == 2))
    {
    float    temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->Getballoon_coeff();
      char tempResult[1024];
      sprintf(tempResult,"%g",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("Setballoon_image",argv[1]))&&(argc == 3))
    {
    vtkImageData  *temp0;
    error = 0;

    temp0 = (vtkImageData *)(vtkTclGetPointerFromObject(argv[2],(char *) "vtkImageData",interp,error));
    if (!error)
      {
      op->Setballoon_image(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("Getballoon_image",argv[1]))&&(argc == 2))
    {
    vtkImageData  *temp20;
    int vtkImageDataCommand(ClientData, Tcl_Interp *, int, char *[]);
    error = 0;

    if (!error)
      {
      temp20 = (op)->Getballoon_image();
      vtkTclGetObjectFromPointer(interp,(void *)temp20,vtkImageDataCommand);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetinitImage",argv[1]))&&(argc == 3))
    {
    vtkImageData  *temp0;
    error = 0;

    temp0 = (vtkImageData *)(vtkTclGetPointerFromObject(argv[2],(char *) "vtkImageData",interp,error));
    if (!error)
      {
      op->SetinitImage(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetinitImage",argv[1]))&&(argc == 2))
    {
    vtkImageData  *temp20;
    int vtkImageDataCommand(ClientData, Tcl_Interp *, int, char *[]);
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetinitImage();
      vtkTclGetObjectFromPointer(interp,(void *)temp20,vtkImageDataCommand);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetHistoGradThreshold",argv[1]))&&(argc == 3))
    {
    float    temp0;
    error = 0;

    if (Tcl_GetDouble(interp,argv[2],&tempd) != TCL_OK) error = 1;
    temp0 = tempd;
    if (!error)
      {
      op->SetHistoGradThreshold(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetHistoGradThreshold",argv[1]))&&(argc == 2))
    {
    float    temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetHistoGradThreshold();
      char tempResult[1024];
      sprintf(tempResult,"%g",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("Setcoeff_curvature",argv[1]))&&(argc == 3))
    {
    float    temp0;
    error = 0;

    if (Tcl_GetDouble(interp,argv[2],&tempd) != TCL_OK) error = 1;
    temp0 = tempd;
    if (!error)
      {
      op->Setcoeff_curvature(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("Getcoeff_curvature",argv[1]))&&(argc == 2))
    {
    float    temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->Getcoeff_curvature();
      char tempResult[1024];
      sprintf(tempResult,"%g",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetProbabilityThreshold",argv[1]))&&(argc == 3))
    {
    float    temp0;
    error = 0;

    if (Tcl_GetDouble(interp,argv[2],&tempd) != TCL_OK) error = 1;
    temp0 = tempd;
    if (!error)
      {
      op->SetProbabilityThreshold(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("GetProbabilityThreshold",argv[1]))&&(argc == 2))
    {
    float    temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->GetProbabilityThreshold();
      char tempResult[1024];
      sprintf(tempResult,"%g",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetNumInitPoints",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->SetNumInitPoints(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetInitPoint",argv[1]))&&(argc == 7))
    {
    int      temp0;
    int      temp1;
    int      temp2;
    int      temp3;
    int      temp4;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (Tcl_GetInt(interp,argv[3],&tempi) != TCL_OK) error = 1;
    temp1 = tempi;
    if (Tcl_GetInt(interp,argv[4],&tempi) != TCL_OK) error = 1;
    temp2 = tempi;
    if (Tcl_GetInt(interp,argv[5],&tempi) != TCL_OK) error = 1;
    temp3 = tempi;
    if (Tcl_GetInt(interp,argv[6],&tempi) != TCL_OK) error = 1;
    temp4 = tempi;
    if (!error)
      {
      op->SetInitPoint(temp0,temp1,temp2,temp3,temp4);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetNumGaussians",argv[1]))&&(argc == 3))
    {
    int      temp0;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (!error)
      {
      op->SetNumGaussians(temp0);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("SetGaussian",argv[1]))&&(argc == 5))
    {
    int      temp0;
    float    temp1;
    float    temp2;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (Tcl_GetDouble(interp,argv[3],&tempd) != TCL_OK) error = 1;
    temp1 = tempd;
    if (Tcl_GetDouble(interp,argv[4],&tempd) != TCL_OK) error = 1;
    temp2 = tempd;
    if (!error)
      {
      op->SetGaussian(temp0,temp1,temp2);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("DistanceMap",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->DistanceMap();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("DistanceMap0",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->DistanceMap0();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("DistanceMap1Old",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->DistanceMap1Old();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("DistanceMap1",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->DistanceMap1();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("DistanceMap2",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->DistanceMap2();
      Tcl_ResetResult(interp);
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
  if ((!strcmp("ResizeBand",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->ResizeBand();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("InitEvolution",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->InitEvolution();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("Iterate",argv[1]))&&(argc == 2))
    {
    int      temp20;
    error = 0;

    if (!error)
      {
      temp20 = (op)->Iterate();
      char tempResult[1024];
      sprintf(tempResult,"%i",temp20);
      Tcl_SetResult(interp, tempResult, TCL_VOLATILE);
      return TCL_OK;
      }
    }
  if ((!strcmp("EndEvolution",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->EndEvolution();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("Evolve3D",argv[1]))&&(argc == 4))
    {
    int      temp0;
    int      temp1;
    error = 0;

    if (Tcl_GetInt(interp,argv[2],&tempi) != TCL_OK) error = 1;
    temp0 = tempi;
    if (Tcl_GetInt(interp,argv[3],&tempi) != TCL_OK) error = 1;
    temp1 = tempi;
    if (!error)
      {
      op->Evolve3D(temp0,temp1);
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }
  if ((!strcmp("PrintParameters",argv[1]))&&(argc == 2))
    {
    error = 0;

    if (!error)
      {
      op->PrintParameters();
      Tcl_ResetResult(interp);
      return TCL_OK;
      }
    }

  if (!strcmp("ListInstances",argv[1]))
    {
    vtkTclListInstances(interp,(ClientData)vtkLevelSetsCommand);
    return TCL_OK;
    }

  if (!strcmp("ListMethods",argv[1]))
    {
    vtkImageToImageFilterCppCommand(op,interp,argc,argv);
    Tcl_AppendResult(interp,"Methods from vtkLevelSets:\n",NULL);
    Tcl_AppendResult(interp,"  GetSuperClassName\n",NULL);
    Tcl_AppendResult(interp,"  New\n",NULL);
    Tcl_AppendResult(interp,"  GetClassName\n",NULL);
    Tcl_AppendResult(interp,"  IsA\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  SetRescaleImage\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetRescaleImage\n",NULL);
    Tcl_AppendResult(interp,"  RescaleImageOn\n",NULL);
    Tcl_AppendResult(interp,"  RescaleImageOff\n",NULL);
    Tcl_AppendResult(interp,"  SetInitThreshold\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetInitThreshold\n",NULL);
    Tcl_AppendResult(interp,"  SetUseLowThreshold\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetUseLowThreshold\n",NULL);
    Tcl_AppendResult(interp,"  UseLowThresholdOn\n",NULL);
    Tcl_AppendResult(interp,"  UseLowThresholdOff\n",NULL);
    Tcl_AppendResult(interp,"  SetLowThreshold\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetLowThreshold\n",NULL);
    Tcl_AppendResult(interp,"  SetUseHighThreshold\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetUseHighThreshold\n",NULL);
    Tcl_AppendResult(interp,"  UseHighThresholdOn\n",NULL);
    Tcl_AppendResult(interp,"  UseHighThresholdOff\n",NULL);
    Tcl_AppendResult(interp,"  SetHighThreshold\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetHighThreshold\n",NULL);
    Tcl_AppendResult(interp,"  SetProbabilityHighThreshold\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetProbabilityHighThreshold\n",NULL);
    Tcl_AppendResult(interp,"  SetStepDt\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetStepDt\n",NULL);
    Tcl_AppendResult(interp,"  SetCheckFreq\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetCheckFreq\n",NULL);
    Tcl_AppendResult(interp,"  SetNumIters\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetNumIters\n",NULL);
    Tcl_AppendResult(interp,"  SetReinitFreq\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetReinitFreq\n",NULL);
    Tcl_AppendResult(interp,"  SetConvergedThresh\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetConvergedThresh\n",NULL);
    Tcl_AppendResult(interp,"  SetAdvectionCoeff\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetAdvectionCoeff\n",NULL);
    Tcl_AppendResult(interp,"  SetDoMean\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetDoMean\n",NULL);
    Tcl_AppendResult(interp,"  DoMeanOn\n",NULL);
    Tcl_AppendResult(interp,"  DoMeanOff\n",NULL);
    Tcl_AppendResult(interp,"  SetBand\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetBand\n",NULL);
    Tcl_AppendResult(interp,"  SetTube\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetTube\n",NULL);
    Tcl_AppendResult(interp,"  SetDMmethod\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetDMmethod\n",NULL);
    Tcl_AppendResult(interp,"  SetEvolveThreads\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetEvolveThreads\n",NULL);
    Tcl_AppendResult(interp,"  Setsavedistmap\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  Getsavedistmap\n",NULL);
    Tcl_AppendResult(interp,"  savedistmapOn\n",NULL);
    Tcl_AppendResult(interp,"  savedistmapOff\n",NULL);
    Tcl_AppendResult(interp,"  Setsavesecdergrad\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  Getsavesecdergrad\n",NULL);
    Tcl_AppendResult(interp,"  savesecdergradOn\n",NULL);
    Tcl_AppendResult(interp,"  savesecdergradOff\n",NULL);
    Tcl_AppendResult(interp,"  Setadvection_scheme\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  Getadvection_scheme\n",NULL);
    Tcl_AppendResult(interp,"  advection_schemeOn\n",NULL);
    Tcl_AppendResult(interp,"  advection_schemeOff\n",NULL);
    Tcl_AppendResult(interp,"  SetUseCosTerm\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetUseCosTerm\n",NULL);
    Tcl_AppendResult(interp,"  UseCosTermOn\n",NULL);
    Tcl_AppendResult(interp,"  UseCosTermOff\n",NULL);
    Tcl_AppendResult(interp,"  SetIsoContourBin\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetIsoContourBin\n",NULL);
    Tcl_AppendResult(interp,"  IsoContourBinOn\n",NULL);
    Tcl_AppendResult(interp,"  IsoContourBinOff\n",NULL);
    Tcl_AppendResult(interp,"  SetDimension\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetDimension\n",NULL);
    Tcl_AppendResult(interp,"  SetSliceNum\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetSliceNum\n",NULL);
    Tcl_AppendResult(interp,"  Setvelocity\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  Getvelocity\n",NULL);
    Tcl_AppendResult(interp,"  Setcoeff_velocity\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  Getcoeff_velocity\n",NULL);
    Tcl_AppendResult(interp,"  Setballoon_coeff\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  Getballoon_coeff\n",NULL);
    Tcl_AppendResult(interp,"  Setballoon_image\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  Getballoon_image\n",NULL);
    Tcl_AppendResult(interp,"  SetinitImage\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetinitImage\n",NULL);
    Tcl_AppendResult(interp,"  SetHistoGradThreshold\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetHistoGradThreshold\n",NULL);
    Tcl_AppendResult(interp,"  Setcoeff_curvature\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  Getcoeff_curvature\n",NULL);
    Tcl_AppendResult(interp,"  SetProbabilityThreshold\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  GetProbabilityThreshold\n",NULL);
    Tcl_AppendResult(interp,"  SetNumInitPoints\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  SetInitPoint\t with 5 args\n",NULL);
    Tcl_AppendResult(interp,"  SetNumGaussians\t with 1 arg\n",NULL);
    Tcl_AppendResult(interp,"  SetGaussian\t with 3 args\n",NULL);
    Tcl_AppendResult(interp,"  DistanceMap\n",NULL);
    Tcl_AppendResult(interp,"  DistanceMap0\n",NULL);
    Tcl_AppendResult(interp,"  DistanceMap1Old\n",NULL);
    Tcl_AppendResult(interp,"  DistanceMap1\n",NULL);
    Tcl_AppendResult(interp,"  DistanceMap2\n",NULL);
    Tcl_AppendResult(interp,"  InitParam\t with 2 args\n",NULL);
    Tcl_AppendResult(interp,"  ResizeBand\n",NULL);
    Tcl_AppendResult(interp,"  InitEvolution\n",NULL);
    Tcl_AppendResult(interp,"  Iterate\n",NULL);
    Tcl_AppendResult(interp,"  EndEvolution\n",NULL);
    Tcl_AppendResult(interp,"  Evolve3D\t with 2 args\n",NULL);
    Tcl_AppendResult(interp,"  PrintParameters\n",NULL);
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
