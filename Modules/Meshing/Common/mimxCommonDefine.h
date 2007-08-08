/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: mimxCommonDefine.h,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.4 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __mimxCommonDefine_h
#define __mimxCommonDefine_h

#include "vtkCommon.h"

#define ACTOR_NONE   0
#define ACTOR_POLYDATA_LINE  1
#define ACTOR_POLYDATA_SURFACE 2
#define ACTOR_STRUCTURED_PLANAR_GRID 3
#define ACTOR_STRUCTURED_SOLID_GRID 4
#define ACTOR_BOUNDING_BOX 5
#define ACTOR_FE_MESH 6

// different filters being used
#define FILTER_CREATE_BOUNDING_BOX_FROM_BOUNDS 1
#define FILTER_EDIT_BOUNDING_BOX_GEOMETRY 2
#define FILTER_CREATE_MESH_SEED 3
#define FILTER_CREATE_FEMESH_FROM_BOUNDING_BOX 4
#define FILTER_WRITE_VTK_FILE_FORMAT 5
#define FILTER_LAPLACIAN_SMOOTHING 6
#define FILTER_RECALCULATE_INTERIOR_NODES 7
#define FILTER_APPLY_NODE_ELEMENT_NUMBERS 8
#define FILTER_EDIT_BOUNDING_BOX_MESH_SEED 9
#define FILTER_WRITE_ABAQUS_FILE_FORMAT 10
#define FILTER_MERGE_FE_MESH 11

// Cell and Point Data Labels
#define MIMX_NODE_NUMBER_LABEL "Node_Numbers"
#define MIMX_ELEMENT_NUMBER_LABEL "Element_Numbers"

#endif
