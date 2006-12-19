/*=auto=========================================================================

  Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================auto=*/

// .NAME vtkSlicerIGTDemoLogic - slicer logic class for Locator module 
// .SECTION Description
// This class manages the logic associated with tracking device for 
// IGT. 


#ifndef __vtkSlicerIGTDemoLogic_h
#define __vtkSlicerIGTDemoLogic_h

#include <vtkSlicerConfigure.h>
#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerLogic.h"
#include "vtkMRMLModelNode.h"

#include "vtkPoints.h"
#include "vtkMatrix4x4.h"

#ifdef USE_OPENTRACKER
#include "OpenTracker/OpenTracker.h"
#include "OpenTracker/common/CallbackModule.h"
using namespace ot;
#endif



class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerIGTDemoLogic : public vtkSlicerLogic 
{
public:

    // The Usual vtk class functions
    static vtkSlicerIGTDemoLogic *New();
    vtkTypeRevisionMacro(vtkSlicerIGTDemoLogic,vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);


    vtkGetObjectMacro(LocatorMatrix,vtkMatrix4x4);
    vtkGetObjectMacro(LandmarkTransformMatrix,vtkMatrix4x4);
    vtkGetObjectMacro(LocatorNormalTransform,vtkTransform);


    vtkSetMacro(UseRegistration,int);
    vtkGetMacro(UseRegistration,int);

    vtkGetMacro(NumberOfPoints,int);

    void Init(char *configfile);
    void CloseConnection();
    void PollRealtime();

#ifdef USE_OPENTRACKER
    static void callbackF(const Node&, const Event &event, void *data);
#endif

    // t1, t2, t3: target landmarks 
    // s1, s2, s3: source landmarks 
    void AddPoint(int id, float t1, float t2, float t3, float s1, float s2, float s3);
    int DoRegistration();

    void SetNumberOfPoints(int no);

    // Description:
    // Description:
    // Update logic state when MRML scene chenges
    void ProcessMRMLEvents(vtkObject * caller, unsigned long event, void * callData);

    void SetLocatorTransforms();
    void UpdateSliceImages();

protected:

    vtkSlicerIGTDemoLogic();
    ~vtkSlicerIGTDemoLogic();
    vtkSlicerIGTDemoLogic(const vtkSlicerIGTDemoLogic&);
    void operator=(const vtkSlicerIGTDemoLogic&);

#ifdef USE_OPENTRACKER
    Context *context;
#endif

    int NumberOfPoints;
    int UseRegistration;
    vtkPoints *SourceLandmarks;
    vtkPoints *TargetLandmarks;
    vtkMatrix4x4 *LandmarkTransformMatrix;
    vtkTransform *LocatorNormalTransform;

//BTX
    float p[3], n[3];
    float pOld[3], nOld[3];
//ETX


    void quaternion2xyz(float* orientation, float *normal, float *transnormal); 
    vtkMatrix4x4 *LocatorMatrix;

    void ApplyTransform(float *position, float *norm, float *transnorm);

    void Normalize(float *a);
    void Cross(float *a, float *b, float *c);

};

#endif


  
