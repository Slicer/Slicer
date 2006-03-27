/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkGradientAnisotropicDiffusionFilterModule.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkGradientAnisotropicDiffusionFilterModule_h
#define __vtkGradientAnisotropicDiffusionFilterModule_h

#include "vtkModule.h"

class VTK_GRADIENTANIOTROPICDIFFUSIONFILTER_EXPORT vtkGradientAnisotropicDiffusionFilterModule : public vtkModule
{
  public:
  static vtkGradientAnisotropicDiffusionFilterModule *New();
  vtkTypeMacro(vtkGradientAnisotropicDiffusionFilterModule,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void BuildGUI(vtkKWFrame* frame);


  //virtual void ProcessMRMLSceneModified(vtkMRMLScene *scene);
  
protected:
  vtkGradientAnisotropicDiffusionFilterModule();
  ~vtkGradientAnisotropicDiffusionFilterModule();
  vtkGradientAnisotropicDiffusionFilterModule(const vtkGradientAnisotropicDiffusionFilterModule&);
  void operator=(const vtkGradientAnisotropicDiffusionFilterModule&);

  vtkCallbackCommand *GUICallbackCommand;

};

#endif

