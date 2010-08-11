/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/


#ifndef __vtkSlicerAnnotationSplineManager_h
#define __vtkSlicerAnnotationSplineManager_h


#include "qSlicerWidget.h"
#include "qSlicerAnnotationModuleExport.h"

class vtkMRMLAnnotationSplineNode;
//class vtkSlicerViewerWidget;
class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkSplineWidget;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkSlicerAnnotationSplineManager : public qSlicerWidget
{
  
public:
    /*
    static vtkSlicerAnnotationSplineManager* New();
    vtkTypeRevisionMacro(vtkSlicerAnnotationSplineManager, vtkSlicerWidget);
    void PrintSelf(ostream& os, vtkIndent indent);
    */
    /// alternative method to propagate events generated in GUI to logic / mrml
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
    
    /// alternative method to propagate events generated in GUI to logic / mrml
    //virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
    /// add observers on node
    virtual void AddMRMLObservers ( );

    /// remove observers on node
    virtual void RemoveMRMLObservers ( );

    /// check scene to make sure that have a widget for each ruler node, and no extra widgets...
    void Update3DWidget(vtkMRMLAnnotationSplineNode *activeNode);

    /// encapsulated 3d widgets for each ruler node
    //std::map<std::string, vtkSplineWidget *> SplineWidgets;
    //vtkSplineWidget *GetSplineWidget(const char * nodeID);
    void AddSplineWidget(vtkMRMLAnnotationSplineNode *node);
    void RemoveSplineWidget(vtkMRMLAnnotationSplineNode *node);
    void RemoveSplineWidgets();

    /// Get/set the viewer widget so can add a the ruler widget to it
    //vtkGetObjectMacro(ViewerWidget, vtkSlicerViewerWidget);
    //virtual void SetViewerWidget(vtkSlicerViewerWidget *viewerWidget);

  void UpdateVisibility(vtkMRMLAnnotationSplineNode *activeNode);

  void UpdateSplineMeasurement(vtkMRMLAnnotationSplineNode* node, vtkSplineWidget* widget);

  void RequestRender();
  void Render();

protected:
    vtkSlicerAnnotationSplineManager();
    virtual ~vtkSlicerAnnotationSplineManager();

  void UpdateSplineWidgetInteractors();
  void UpdateLockUnlock(vtkMRMLAnnotationSplineNode* node);

    /// the id of the mrml node currently displayed in the widget
    char* NodeID;

    /// flag set to 1 when updating a 3d widget (todo: set it to the index of the
    /// ruler node / 3d widget once have more than one)
    int Updating3DWidget;

    /// pointer to the viewer widget so can add props, request renders 
    //vtkSlicerViewerWidget *ViewerWidget;
  int ProcessingMRMLEvent;


private:

    vtkSlicerAnnotationSplineManager(const vtkSlicerAnnotationSplineManager&); /// Not implemented
    void operator=(const vtkSlicerAnnotationSplineManager&); /// Not Implemented

};

#endif

