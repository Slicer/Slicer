/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkLevelSetsPythonInit.cxx,v $
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

#include <string.h>
#include "Python.h"

extern  "C" {PyObject *PyVTKClass_vtkLevelSetFastMarchingNew(char *); }
extern  "C" {PyObject *PyVTKClass_vtkImageIsoContourDistNew(char *); }
extern  "C" {PyObject *PyVTKClass_vtkImageFastSignedChamferNew(char *); }
extern  "C" {PyObject *PyVTKClass_vtkImageCURVESNew(char *); }

static PyMethodDef PyvtkLevelSetsPython_ClassMethods[] = {
{NULL, NULL}};

extern  "C" {void initlibvtkLevelSetsPython();}

void initlibvtkLevelSetsPython()
{
  PyObject *m, *d, *c;

  static char modulename[] = "libvtkLevelSetsPython";
  m = Py_InitModule(modulename, PyvtkLevelSetsPython_ClassMethods);
  d = PyModule_GetDict(m);
  if (!d) Py_FatalError("can't get dictionary for module vtkLevelSetsPython!");

  if ((c = PyVTKClass_vtkLevelSetFastMarchingNew(modulename)))
    if (-1 == PyDict_SetItemString(d, "vtkLevelSetFastMarching", c))
      Py_FatalError("can't add class vtkLevelSetFastMarching to dictionary!");

  if ((c = PyVTKClass_vtkImageIsoContourDistNew(modulename)))
    if (-1 == PyDict_SetItemString(d, "vtkImageIsoContourDist", c))
      Py_FatalError("can't add class vtkImageIsoContourDist to dictionary!");

  if ((c = PyVTKClass_vtkImageFastSignedChamferNew(modulename)))
    if (-1 == PyDict_SetItemString(d, "vtkImageFastSignedChamfer", c))
      Py_FatalError("can't add class vtkImageFastSignedChamfer to dictionary!");

  if ((c = PyVTKClass_vtkImageCURVESNew(modulename)))
    if (-1 == PyDict_SetItemString(d, "vtkImageCURVES", c))
      Py_FatalError("can't add class vtkImageCURVES to dictionary!");

}

