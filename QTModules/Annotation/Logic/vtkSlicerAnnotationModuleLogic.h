#ifndef __vtkSlicerAnnotationModuleLogic_h
#define __vtkSlicerAnnotationModuleLogic_h

// Annotation QT includes
#include "GUI/qSlicerAnnotationModuleWidget.h"

// Slicer Logic includes
#include "vtkSlicerModuleLogic.h"

// qCTK includes
#include <ctkPimpl.h>

#include "qSlicerAnnotationModuleExport.h"

class vtkSlicerFiducialListWidget;
class vtkSlicerAnnotationModuleLogicPrivate;
class vtkMeasurementsAngleWidget;
class vtkMeasurementsRulerWidget;
class vtkMRMLMeasurementsAngleNode;
class vtkMRMLAnnotationRulerNode;
class vtkMRMLAnnotationAngleNode;
class vtkSlicerViewerWidget;
class vtkSlicerApplicationGUI;
class vtkMeasurementsDistanceWidgetClass;
class vtkMeasurementsAngleWidgetClass;
class vtkMRMLAnnotationFiducialNode;
class vtkMRMLAnnotationFiducialNode;
class vtkMRMLAnnotationNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkMRMLAnnotationLinesNode;
class vtkMRMLAnnotationControlPointsNode;
class vtkMRMLAnnotationDisplayNode;
class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationStickyNode;
class vtkSlicerAnnotationRulerManager;
class vtkSlicerAnnotationAngleManager;
class vtkMRMLAnnotationFiducialDisplayableManager;
class vtkMRMLAnnotationTextNode;
//class vtkSlicerSeedWidgetClass;
class vtkTextWidget;
class vtkSlicerROIDisplayWidget;
class vtkSlicerAnnotationROIManager;
class vtkSlicerAnnotationSplineManager;
class vtkSlicerAnnotationBidimensionalManager;
class vtkMRMLAnnotationTextDisplayableManager;


class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkSlicerAnnotationModuleLogic :
  public vtkSlicerModuleLogic
{
public:
  typedef enum { VALUE=0, 
         TEXT, 
         LOCK,      
         TEXT_COLOR, 
         TEXT_SELECTED_COLOR, 
         TEXT_OPACITY, 
         TEXT_AMBIENT, 
         TEXT_DIFFUSE, 
         TEXT_SPECULAR,
         TEXT_SCALE, 
         POINT_COLOR, 
         POINT_SELECTED_COLOR, 
         POINT_OPACITY, 
         POINT_AMBIENT, 
         POINT_DIFFUSE, 
         POINT_SPECULAR,
         POINT_SIZE, 
         LINE_COLOR, 
         LINE_SELECTED_COLOR, 
         LINE_OPACITY, 
         LINE_AMBIENT, 
         LINE_DIFFUSE, 
         LINE_SPECULAR,
         LINE_WIDTH} PropertyType;

  enum
  {
      AddAngleCompletedEvent = 19020,
    AddTextNodeCompletedEvent,
  };

  // starting Daniel approved code
  static vtkSlicerAnnotationModuleLogic *New();
  vtkTypeRevisionMacro(vtkSlicerAnnotationModuleLogic,vtkSlicerModuleLogic);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Start the place mode for annotations
  void StartPlaceMode();

  // Exit the place mode for annotations
  void StopPlaceMode();

  // Start adding a new annotation Node
  void AddAnnotationNode(const char * nodeDescriptor);

  // After a node was added, propagate to widget
  void AddNodeCompleted(vtkMRMLAnnotationNode* node);

  // Cancel the current annotation placement or remove last annotation node
  void CancelCurrentOrRemoveLastAddedAnnotationNode();

  // Register the widget
  void SetAndObserveWidget(qSlicerAnnotationModuleWidget* widget);

  // MRML events
  void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData );
  void OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node);
  void OnMRMLAnnotationNodeModifiedEvent(vtkMRMLNode* node);

  //
  // Annotation Properties (interface to MRML)
  //
  /// Get the name of an Annotation MRML node
  const char * GetAnnotationName(const char * id);

  /// Return the text of an Annotation MRML node
  vtkStdString GetAnnotationText(const char* id);
  /// Set the text of an Annotation MRML node
  void SetAnnotationText(const char* id, const char * newtext);

  /// Get the text scale of an Annotation MRML node
  double GetAnnotationTextScale(const char* id);
  /// Set the text scale of an Annotation MRML node
  void SetAnnotationTextScale(const char* id, double textScale);

  /// Get the measurement value of an Annotation MRML node
  const char * GetAnnotationMeasurement(const char * id, bool showUnits);

  /// Get the icon name of an Annotation MRML node
  const char * GetAnnotationIcon(const char * id);

  /// Toggle the lock flag of an Annotation MRML node and return the updated flag
  int ToggleAnnotationLockUnlock(const char * id);


  /// Backup an Annotation MRML node
  void BackupAnnotationNode(const char * id);
  /// Restore a backup of an Annotation MRML node
  void RestoreAnnotationNode(const char * id);

  // ^^^^ end of Daniel approved code










  // Common Widget Public Function
  void ModifyPropertiesAndWidget(vtkMRMLNode* node, int type, void*data);
  vtkSlicerViewerWidget* GetViewerWidget();
  void SetAnnotationSelectedByIDs(std::vector<const char*> selectedIDs, std::vector<const char*> allIDs);
  void RemoveAnnotationByID(const char* id);

  // Common MRML Related Public Functions
  void SetAnnotationControlPointsProperties(vtkMRMLAnnotationControlPointsNode* node, int type, void* data);
  void SetAnnotationLinesProperties(vtkMRMLAnnotationLinesNode* node, int type, void* data);
  void SetAnnotationProperties(vtkMRMLAnnotationNode* node, int type, void* data);
  int SetAnnotationDisplayProperties(vtkMRMLAnnotationDisplayNode* node, int type, void* data);
  int SetAnnotationTextDisplayProperties(vtkMRMLAnnotationTextDisplayNode* node, int type, void* data);
  int SetAnnotationPointDisplayProperties(vtkMRMLAnnotationPointDisplayNode* node, int type, void* data);
  int SetAnnotationLineDisplayProperties(vtkMRMLAnnotationLineDisplayNode* node, int type, void* data);
  vtkStdString GetAnnotationTextProperty(vtkMRMLNode* node);
  const char* GetAnnotationTextFormatProperty(vtkMRMLNode* node);


  int SetAnnotationControlPointsCoordinate(vtkMRMLNode* mrmlnode, double* pos, vtkIdType coordId);

  int GetAnnotationLinesPropertiesDouble(vtkMRMLNode* node, int type, double &result);
  int GetAnnotationLineDisplayPropertiesDouble(vtkMRMLAnnotationLineDisplayNode* node, int type, double& result);
  int GetAnnotationControlPointsPropertiesDouble(vtkMRMLAnnotationControlPointsNode* node, int type, double &result);
  int GetAnnotationPropertiesDouble(vtkMRMLAnnotationNode* node, int type, double& result);
  int GetAnnotationDisplayPropertiesDouble(vtkMRMLAnnotationDisplayNode* node, int type, double& result);
  int GetAnnotationTextDisplayPropertiesDouble(vtkMRMLAnnotationTextDisplayNode* node, int type, double& result);
  int GetAnnotationPointDisplayPropertiesDouble(vtkMRMLAnnotationPointDisplayNode* node, int type, double& result);

  double* GetAnnotationLinesPropertiesColor(vtkMRMLNode* mrmlnode, int type);
  double* GetAnnotationLineDisplayPropertiesColor(vtkMRMLAnnotationLineDisplayNode* node, int type);
  double* GetAnnotationControlPointsPropertiesColor(vtkMRMLAnnotationControlPointsNode* node, int type);
  double* GetAnnotationPropertiesColor(vtkMRMLAnnotationNode* node, int type);
  double* GetAnnotationDisplayPropertiesColor(vtkMRMLAnnotationDisplayNode* node, int type);
  double* GetAnnotationTextDisplayPropertiesColor(vtkMRMLAnnotationTextDisplayNode* node, int type);
  double* GetAnnotationPointDisplayPropertiesColor(vtkMRMLAnnotationPointDisplayNode* node, int type);

  // Other Public Functions

  vtkSlicerApplicationGUI* GetApplicationGUI();
  //vtkImageData* SaveScreenShot();
  void SaveMRMLScene();
  const char* GetIconName(vtkMRMLNode* node, bool isEdit=false);

  void AddRulerNodeObserver(vtkMRMLAnnotationRulerNode* rnode) ;

  int TestReceivedMessage; 



  /// Return the number of Control Points
  int GetNumberOfControlPoints(vtkMRMLNode* mrmlnode);
  int GetNumberOfControlPointsByID(const char * id);

  /// Return the Control Points of an Annotation MRML node
  double* GetAnnotationControlPointsCoordinate(vtkMRMLNode* mrmlnode, vtkIdType coordId);
  double* GetAnnotationControlPointsCoordinateByID(const char * id, vtkIdType coordId);

protected:
  vtkSlicerAnnotationModuleLogic();
  ~vtkSlicerAnnotationModuleLogic();
  // not implemented
  vtkSlicerAnnotationModuleLogic(const vtkSlicerAnnotationModuleLogic&);
  void operator=(const vtkSlicerAnnotationModuleLogic&);

private:
  CTK_DECLARE_PRIVATE(vtkSlicerAnnotationModuleLogic);

  qSlicerAnnotationModuleWidget *m_Widget;

  vtkMRMLAnnotationNode *m_LastAddedAnnotationNode;

};

#endif
