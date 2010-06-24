/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/


#ifndef __vtkSlicerAnnotationBidimensionalManager_h
#define __vtkSlicerAnnotationBidimensionalManager_h


#include "qSlicerWidget.h"
#include "qSlicerAnnotationModuleExport.h"

class vtkMRMLAnnotationBidimensionalNode;
//class vtkSlicerViewerWidget;
class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkBiDimensionalWidget;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkSlicerAnnotationBidimensionalManager : public qSlicerWidget
{
  
public:
/*    static vtkSlicerAnnotationBidimensionalManager* New();
    vtkTypeRevisionMacro(vtkSlicerAnnotationBidimensionalManager, vtkSlicerWidget);
    void PrintSelf(ostream& os, vtkIndent indent);
*/
    /// alternative method to propagate events generated in GUI to logic / mrml
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
    
    /// add observers on node
    virtual void AddMRMLObservers ( );

    /// remove observers on node
    virtual void RemoveMRMLObservers ( );

    /// check scene to make sure that have a widget for each ruler node, and no extra widgets...
    void Update3DWidget(vtkMRMLAnnotationBidimensionalNode *activeNode);

    /// encapsulated 3d widgets for each ruler node
    //std::map<std::string, vtkBiDimensionalWidget *> BidimensionalWidgets;
    vtkBiDimensionalWidget *GetBidimensionalWidget(const char * nodeID);
    void AddBidimensionalWidget(vtkMRMLAnnotationBidimensionalNode *node);
    void RemoveBidimensionalWidget(vtkMRMLAnnotationBidimensionalNode *node);
    void RemoveBidimensionalWidgets();

    /// Get/set the viewer widget so can add a the ruler widget to it
  //  vtkGetObjectMacro(ViewerWidget, vtkSlicerViewerWidget);
   // virtual void SetViewerWidget(vtkSlicerViewerWidget *viewerWidget);

  void UpdateVisibility(vtkMRMLAnnotationBidimensionalNode *activeNode);

  void UpdateBidimensionalMeasurement(vtkMRMLAnnotationBidimensionalNode* node, vtkBiDimensionalWidget* widget);

  void RequestRender();
  void Render();

protected:
    vtkSlicerAnnotationBidimensionalManager();
    virtual ~vtkSlicerAnnotationBidimensionalManager();

  void UpdateWidgetInteractors();
  void UpdateLockUnlock(vtkMRMLAnnotationBidimensionalNode* node);

    /// the id of the mrml node currently displayed in the widget
    char* NodeID;

    /// flag set to 1 when updating a 3d widget (todo: set it to the index of the
    /// ruler node / 3d widget once have more than one)
    int Updating3DWidget;

    /// pointer to the viewer widget so can add props, request renders 
    //vtkSlicerViewerWidget *ViewerWidget;
  int ProcessingMRMLEvent;


private:

    vtkSlicerAnnotationBidimensionalManager(const vtkSlicerAnnotationBidimensionalManager&); /// Not implemented
    void operator=(const vtkSlicerAnnotationBidimensionalManager&); /// Not Implemented

};

#endif

