/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: QtGADModule.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME QtGADModule - menu to select volumes from current mrml scene
// .SECTION Description
// Inherits most behavior from kw widget, but is specialized to observe
// the current mrml scene and update the entries of the pop up menu to correspond
// to the currently available volumes.  This widget also has a notion of the current selection
// that can be observed or set externally
//


#ifndef __QtGADModule_h
#define __QtGADModule_h

//BTX

class QtSlicerNodeSelectorWidget;
class vtkGradientAnisotropicDiffusionFilterLogic;

#include <QWidget>
#include "QtSlider.h"


#include "vtkGradientAnisotropicDiffusionFilter.h"
#include "vtkMRMLGradientAnisotropicDiffusionFilterNode.h"

#include "vtkMRMLScene.h"

class VTK_GRADIENTANISOTROPICDIFFUSIONFILTER_EXPORT QtGADModule : public QWidget 
{
  Q_OBJECT
public:

  QtGADModule();
  virtual ~QtGADModule();
    
  vtkMRMLScene* GetMRMLScene() 
    {
      return this->MRMLScene;
    };
  
  void SetMRMLScene(vtkMRMLScene *scene)
    {
      this->MRMLScene = scene;
    };

  void BuildGUI();
  
public slots:

  void apply();

  void parameterValueChanged(double value);

  void parameterSelected(const QString& id);

  void inputSelected(const QString& id);

  void outputSelected(const QString& id);

  void updateGUI();

  void updateMRML();

//Q_SIGNALS:
 

protected:

  vtkMRMLScene               *MRMLScene;

  QtSlicerNodeSelectorWidget *GADNodeSelector;

  QtSlicerNodeSelectorWidget *VolumeSelector;

  QtSlicerNodeSelectorWidget *OutVolumeSelector;

  vtkGradientAnisotropicDiffusionFilterLogic *Logic;

  QtSlider  *ConductanceSlider;
  QtSlider  *TimeStepSlider;
  QtSlider  *IterationsSlider;


  vtkMRMLGradientAnisotropicDiffusionFilterNode* CreateParameterNode();

  int CreatingParametersNode;

  int UpdatingGUI;

  int UpdatingMRML;

  QtGADModule(const QtGADModule&); // Not implemented
  void operator=(const QtGADModule&); // Not Implemented
};
//ETX
#endif

