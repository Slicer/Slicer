/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/


#ifndef __vtkSlicerAnnotationAngleManager_h
#define __vtkSlicerAnnotationAngleManager_h


#include "qSlicerWidget.h"
#include "qSlicerAnnotationModuleExport.h"

class vtkMRMLAnnotationAngleNode;
class vtkMeasurementsAngleWidgetClass;
class vtkSlicerViewerWidget;
class vtkCamera;
class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;


class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkSlicerAnnotationAngleManager : public qSlicerWidget
{
  
public:
    //static vtkSlicerAnnotationAngleManager* New();
    //vtkTypeRevisionMacro(vtkSlicerAnnotationAngleManager, vtkSlicerWidget);
    //void PrintSelf(ostream& os, vtkIndent indent);

    /// Getting the mrml Angle node id
    //vtkGetStringMacro(AngleNodeID);

    /// Set the fid list id, and update the widgets
    //void SetAngleNodeID(char *id);
  
    /// alternative method to propagate events generated in GUI to logic / mrml
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
    
    /// alternative method to propagate events generated in GUI to logic / mrml
    //virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
    /// add observers on node
    virtual void AddMRMLObservers ( );

    /// remove observers on node
    virtual void RemoveMRMLObservers ( );

    /// check scene to make sure that have a widget for each Angle node, and no extra widgets...
    void Update3DWidgetsFromMRML();

    /// Update the interactors on all the widgets
    void UpdateAngleWidgetInteractors();

    /// encapsulated 3d widgets for each Angle node
    //std::map<std::string, vtkMeasurementsAngleWidgetClass*> AngleWidgets;

    vtkMeasurementsAngleWidgetClass *GetAngleWidget(const char * nodeID);
    void AddAngleWidget(vtkMRMLAnnotationAngleNode *angleNode);
    void RemoveAngleWidget(vtkMRMLAnnotationAngleNode *angleNode);
    void RemoveAngleWidgets();

    /// Get/set the viewer widget so can add a the Angle widget to it
    //vtkGetObjectMacro(ViewerWidget, vtkSlicerViewerWidget);
    //virtual void SetViewerWidget(vtkSlicerViewerWidget *viewerWidget);

    void UpdateCamera();
    vtkCamera *GetActiveCamera();

    /// encapsulated text display node for each Angle node
    //std::map<std::string, std::string> TextDisplayNodes;
    //std::string GetTextDisplayNodeID(const char *nodeID);
    //std::string GetAngleNodeIDFromTextDisplayNode(const char *nodeID);
    void AddTextDiaplayNodeID(vtkMRMLAnnotationAngleNode *angleNode);
    void RemoveTextDisplayNodeID(vtkMRMLAnnotationAngleNode *angleNode);

    /// encapsulated point display node for each Angle node
    //std::map<std::string, std::string> PointDisplayNodes;
    //std::string GetPointDisplayNodeID(const char *nodeID);
    //std::string GetAngleNodeIDFromPointDisplayNode(const char *nodeID);
    void AddPointDiaplayNodeID(vtkMRMLAnnotationAngleNode *angleNode);
    void RemovePointDisplayNodeID(vtkMRMLAnnotationAngleNode *angleNode);

    /// encapsulated line display node for each Angle node
    //std::map<std::string, std::string> LineDisplayNodes;
    //std::string GetLineDisplayNodeID(const char *nodeID);
    //std::string GetAngleNodeIDFromLineDisplayNode(const char *nodeID);
    void AddLineDiaplayNodeID(vtkMRMLAnnotationAngleNode *angleNode);
    void RemoveLineDisplayNodeID(vtkMRMLAnnotationAngleNode *angleNode);

    void SetWidgetColor(vtkMRMLAnnotationAngleNode* node, int type, double* color);
    void UpdateAngleMeasurement(vtkMRMLAnnotationAngleNode *angleNode);

     void UpdateLockUnlock(vtkMRMLAnnotationAngleNode* angleNode);

protected:
    vtkSlicerAnnotationAngleManager();
    virtual ~vtkSlicerAnnotationAngleManager();

    /// update the widget GUI from the settings in the passed in AngleNode
    void UpdateWidget(vtkMRMLAnnotationAngleNode *angleNode);

    /// update the 3d distance widget from the settings in the passed in
    /// AngleNode
    void Update3DWidget(vtkMRMLAnnotationAngleNode *angleNode);

    /// update the display nodes for AngleNode
    void UpdateDisplayNode(vtkMRMLAnnotationAngleNode *angleNode);

    /// update the text property passed by the text display node
    void UpdateTextDisplayProperty(vtkMRMLAnnotationTextDisplayNode *node);

    /// update the point property passed by the point display node
    void UpdatePointDisplayProperty(vtkMRMLAnnotationPointDisplayNode *node);

    /// update the point property passed by the point display node
    void UpdateLineDisplayProperty(vtkMRMLAnnotationLineDisplayNode *node);

    /// the id of the mrml node currently displayed in the widget
    char* AngleNodeID;

    /// flag set to 1 when updating a 3d widget (todo: set it to the index of the
    /// Angle node / 3d widget once have more than one)
    int Updating3DAngle;

    /// pointer to the viewer widget so can add props, request renders 
    vtkSlicerViewerWidget *ViewerWidget;

    /// Sets Slicer's interaction mode to "pick and manipulate"
    /// when a widget is being manipulated, and
    void UpdateInteractionModeAtStartInteraction();
    ///
    /// Resets Slicer's interaction mode back to original
    /// interaction mode when manipulation is finished.
    void UpdateInteractionModeAtEndInteraction();

    ///  update the visibility of the 3d widget associated with the passed node
    void Update3DWidgetVisibility(vtkMRMLAnnotationAngleNode *activeAngleNode);


private:

    vtkSlicerAnnotationAngleManager(const vtkSlicerAnnotationAngleManager&); /// Not implemented
    void operator=(const vtkSlicerAnnotationAngleManager&); /// Not Implemented

};

#endif

