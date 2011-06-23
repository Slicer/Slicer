/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/


#ifndef __vtkSlicerAnnotationRulerManager_h
#define __vtkSlicerAnnotationRulerManager_h


#include "qSlicerWidget.h"
#include "qSlicerAnnotationModuleExport.h"

class vtkMRMLAnnotationRulerNode;
//class vtkMeasurementsDistanceWidgetClass;
//class vtkSlicerViewerWidget;
class vtkCamera;
class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkSlicerSeedWidgetClass;


class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkSlicerAnnotationRulerManager : public qSlicerWidget
{
  
public:
    /*static vtkSlicerAnnotationRulerManager* New();
    vtkTypeRevisionMacro(vtkSlicerAnnotationRulerManager, vtkSlicerWidget);
    void PrintSelf(ostream& os, vtkIndent indent);
*/
    /// Getting the mrml ruler node id
    //vtkGetStringMacro(RulerNodeID);

    /// Set the fid list id, and update the widgets
    //void SetRulerNodeID(char *id);
  
    /// alternative method to propagate events generated in GUI to logic / mrml
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
    
    /// alternative method to propagate events generated in GUI to logic / mrml
    //virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
    /// add observers on node
    virtual void AddMRMLObservers ( );

    /// remove observers on node
    virtual void RemoveMRMLObservers ( );

    /// check scene to make sure that have a widget for each ruler node, and no extra widgets...
    void Update3DWidgetsFromMRML();

    /// Update the interactors on all the widgets
    void UpdateRulerWidgetInteractors();

    /// encapsulated 3d widgets for each ruler node
   // std::map<std::string, vtkMeasurementsDistanceWidgetClass *> DistanceWidgets;
    //vtkMeasurementsDistanceWidgetClass *GetDistanceWidget(const char * nodeID);
    void AddDistanceWidget(vtkMRMLAnnotationRulerNode *rulerNode);
    void RemoveDistanceWidget(vtkMRMLAnnotationRulerNode *rulerNode);
    void RemoveDistanceWidgets();

    /// Get/set the viewer widget so can add a the ruler widget to it
    //vtkGetObjectMacro(ViewerWidget, vtkSlicerViewerWidget);
    //virtual void SetViewerWidget(vtkSlicerViewerWidget *viewerWidget);

    /// Update the camera
    void UpdateCamera();

    /// get the currently active camera
    vtkCamera *GetActiveCamera();

    /// encapsulated text display node for each ruler node
    //std::map<std::string, std::string> TextDisplayNodes;
    std::string GetTextDisplayNodeID(const char *nodeID);
    std::string GetRulerNodeIDFromTextDisplayNode(const char *nodeID);
    void AddTextDiaplayNodeID(vtkMRMLAnnotationRulerNode *rulerNode);
    void RemoveTextDisplayNodeID(vtkMRMLAnnotationRulerNode *rulerNode);

    /// encapsulated point display node for each ruler node
    //std::map<std::string, std::string> PointDisplayNodes;
    std::string GetPointDisplayNodeID(const char *nodeID);
    std::string GetRulerNodeIDFromPointDisplayNode(const char *nodeID);
    void AddPointDiaplayNodeID(vtkMRMLAnnotationRulerNode *rulerNode);
    void RemovePointDisplayNodeID(vtkMRMLAnnotationRulerNode *rulerNode);

    /// encapsulated line display node for each ruler node
    //std::map<std::string, std::string> LineDisplayNodes;
    std::string GetLineDisplayNodeID(const char *nodeID);
    std::string GetRulerNodeIDFromLineDisplayNode(const char *nodeID);
    void AddLineDiaplayNodeID(vtkMRMLAnnotationRulerNode *rulerNode);
    void RemoveLineDisplayNodeID(vtkMRMLAnnotationRulerNode *rulerNode);

    void UpdateDistanceMeasurement(vtkMRMLAnnotationRulerNode* rulerNode);


    /// encapsulated 3d widgets for each seed node
    //std::map<std::string, vtkSlicerSeedWidgetClass *> SeedWidgets;
    vtkSlicerSeedWidgetClass *GetSeedWidget(const char * nodeID);
    void AddSeedWidget(vtkMRMLAnnotationRulerNode* rulerNode);
    void RemoveSeedWidget(vtkMRMLAnnotationRulerNode* rulerNode);

     void UpdateLockUnlock(vtkMRMLAnnotationRulerNode* rulerNode);

protected:
    vtkSlicerAnnotationRulerManager();
    virtual ~vtkSlicerAnnotationRulerManager();

    /// update the widget GUI from the settings in the passed in rulerNode
    void UpdateWidget(vtkMRMLAnnotationRulerNode *rulerNode);

    /// update the 3d distance widget from the settings in the passed in
    /// rulerNode
    void Update3DWidget(vtkMRMLAnnotationRulerNode *rulerNode);

    /// update the display nodes for rulerNode
    void UpdateDisplayNode(vtkMRMLAnnotationRulerNode *rulerNode);

    /// update the text property passed by the text display node
    void UpdateTextDisplayProperty(vtkMRMLAnnotationTextDisplayNode *node);

    /// update the point property passed by the point display node
    void UpdatePointDisplayProperty(vtkMRMLAnnotationPointDisplayNode *node);

    /// update the point property passed by the point display node
    void UpdateLineDisplayProperty(vtkMRMLAnnotationLineDisplayNode *node);

    /// the id of the mrml node currently displayed in the widget
    char* RulerNodeID;

    /// flag set to 1 when updating a 3d widget (todo: set it to the index of the
    /// ruler node / 3d widget once have more than one)
    int Updating3DRuler;

    /// pointer to the viewer widget so can add props, request renders 
    //vtkSlicerViewerWidget *ViewerWidget;

    /// Sets Slicer's interaction mode to "pick and manipulate"
    /// when a widget is being manipulated, and
    void UpdateInteractionModeAtStartInteraction();
    ///
    /// Resets Slicer's interaction mode back to original
    /// interaction mode when manipulation is finished.
    void UpdateInteractionModeAtEndInteraction();

    ///  update the visibility of the 3d widget associated with the passed node
    void Update3DWidgetVisibility(vtkMRMLAnnotationRulerNode *activeRulerNode);


private:

    vtkSlicerAnnotationRulerManager(const vtkSlicerAnnotationRulerManager&); /// Not implemented
    void operator=(const vtkSlicerAnnotationRulerManager&); /// Not Implemented

};

#endif

