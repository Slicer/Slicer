/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/


#ifndef __vtkSlicerAnnotationROIManager_h
#define __vtkSlicerAnnotationROIManager_h


#include "qSlicerWidget.h"
#include "qSlicerAnnotationModuleExport.h"

class vtkMRMLAnnotationROINode;
//class vtkSlicerViewerWidget;
class vtkCamera;
class vtSlicerAnnotationROIWidgetCallback;
//class vtkSlicerBoxWidget2;


class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkSlicerAnnotationROIManager : public qSlicerWidget
{
  
public:
    /*static vtkSlicerAnnotationROIManager* New();
    vtkTypeRevisionMacro(vtkSlicerAnnotationROIManager, vtkSlicerWidget);
    void PrintSelf(ostream& os, vtkIndent indent);
*/
  /// add mrml scene observers 
  virtual void AddMRMLObservers();

  /// removes observers on widgets in the class
  virtual void RemoveMRMLObservers();

    /// alternative method to propagate events generated in GUI to logic / mrml
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
   
    /// Get/set the viewer widget so can add a the ruler widget to it
 //   vtkGetObjectMacro(ViewerWidget, vtkSlicerViewerWidget);
 //   virtual void SetViewerWidget(vtkSlicerViewerWidget *viewerWidget);

   /// Post a request for a render -- won't be done until the system is
   /// idle, and then only once....
   void RequestRender();

   /// Actually do a render (don't wait for idle)
   void Render();

   /// Remove all the box widgets from the scene and memory
   void RemoveBoxWidgets();

   /// Remove the box widget associated with this id
   void RemoveBoxWidget(const char *pointID);

   /// Remove the box widget
   //void RemoveBoxWidget(vtkSlicerBoxWidget2* boxWidget);

   void SetProcessingWidgetEvent(int flag)
   {
     this->ProcessingWidgetEvent = flag;
   };

   /// return the current box widget corresponding to a given MRML ID
   //vtkSlicerBoxWidget2 *GetBoxWidgetByID (const char *id);

   /// return the current box widget callback corresponding to a given MRML ID
   vtSlicerAnnotationROIWidgetCallback* GetBoxWidgetCallbackByID (const char *id);

   /// Updates all roi's based on mrml nodes
   void UpdateFromMRML();

   /// Updates an roi's based on mrml nodes
   void UpdateROIFromMRML(vtkMRMLAnnotationROINode *roi);

   /// Updates an roi's based on transform of the ROI to World
   void UpdateROITransform(vtkMRMLAnnotationROINode *roi);

   void UpdateROIWidgetInteractors();

   //void UpdateROIMeasurement(vtkMRMLAnnotationROINode* node, vtkSlicerBoxWidget2* widget);

protected:
    vtkSlicerAnnotationROIManager();
    virtual ~vtkSlicerAnnotationROIManager();

  /// Add observers for this MRML node
  void AddMRMLROIObservers(vtkMRMLAnnotationROINode *roi);

  /// Remove observers for all MRML nodes
  void RemoveMRMLROIObservers();

  /// Remove observers for this MRML node
  void RemoveMRMLROINodeObservers(vtkMRMLAnnotationROINode *roi);

  void UpdateLockUnlock(vtkMRMLAnnotationROINode* roiNode);

    /// the id of the mrml node currently displayed in the widget
    char* NodeID;

    /// flag set to 1 when updating a 3d widget (todo: set it to the index of the
    /// ruler node / 3d widget once have more than one)
    int Updating3DNode;

  int ProcessingMRMLEvent;
  int ProcessingWidgetEvent;

  //std::map<std::string, vtkSlicerBoxWidget2*> DisplayedBoxWidgets;
  //std::map<std::string, vtSlicerAnnotationROIWidgetCallback*> DisplayedBoxWidgetCallbacks;

  /// pointer to the viewer widget so can add props, request renders 
  //vtkSlicerViewerWidget *ViewerWidget;


private:

    vtkSlicerAnnotationROIManager(const vtkSlicerAnnotationROIManager&); /// Not implemented
    void operator=(const vtkSlicerAnnotationROIManager&); /// Not Implemented

};

#endif

