#ifndef __vtkSlicermiAnnotationModuleLogic_h
#define __vtkSlicermiAnnotationModuleLogic_h

// Slicer Logic includes
#include "vtkSlicerModuleLogic.h"

// qCTK includes
#include <ctkPimpl.h>

#include "qSlicermiAnnotationModuleExport.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWUserInterfacePanel.h"
#include "vtkSlicerModuleGUI.h"

class vtkSlicerFiducialListWidget;
class vtkSlicermiAnnotationModuleLogicPrivate;
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
class vtkSlicerAnnotationFiducialManager;
class vtkMRMLAnnotationTextNode;
class vtkSlicerSeedWidgetClass;
class vtkTextWidget;
class vtkSlicerROIDisplayWidget;
class vtkSlicerAnnotationROIManager;
class vtkSlicerAnnotationSplineManager;
class vtkSlicerAnnotationBidimensionalManager;
class vtkSlicerAnnotationTextManager;


class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkSlicermiAnnotationModuleLogic :
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

  static vtkSlicermiAnnotationModuleLogic *New();
  vtkTypeRevisionMacro(vtkSlicermiAnnotationModuleLogic,vtkSlicerModuleLogic);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Fiducial Related Public Functions
  const char* AddFiducial();
  const char* AddFiducialPicked();
  double GetFiducialValue(const char* cpID);
  std::vector<double> GetFiducialPositionsByNodeID(const char* cpID);
  void SetFiducialPositionsByNodeID(const char* cpID, std::vector<double> positions);
  void RemoveFiducial(const char* id);
  vtkMRMLAnnotationFiducialNode* GetFiducialNodeByID(const char* id);
  void Update3DFiducial(vtkMRMLAnnotationFiducialNode *activeCPNode);
  void StartAddingFiducials();
  void StopAddingFiducials();

  // Ruler Related Public Functions
  const char* AddRuler();
  void RemoveRuler(const char* id);
  vtkMRMLAnnotationRulerNode* GetRulerNodeByID(const char* id);

  // Angle Related Public Functions
  const char* AddAngle();
  void RemoveAngle(const char* id);
  vtkMRMLAnnotationAngleNode* GetAngleNodeByID(const char* id);
  void AddAngleCompleted();

  // Sticky Node
  const char* AddStickyNode();

  // Text Node
  const char* AddTextNode();
  void AddTextNodeCompleted();
  vtkMRMLAnnotationTextNode* GetTextNodeByID(const char* id);
  
  // ROI Node
  const char* AddROINode();

  // BiDimentional Line Node
  const char* AddBidLineNode();

  // SplineNode
  const char* AddSplineNode();

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
  double GetAnnotationMeasurement(vtkMRMLNode* node);
  int GetNumberOfControlPoints(vtkMRMLNode* mrmlnode);
  double* GetAnnotationControlPointsCoordinate(vtkMRMLNode* mrmlnode, vtkIdType coordId);
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
  void ProcessLogicEvents(vtkObject *caller, unsigned long event, void *callData );
  vtkSlicerApplicationGUI* GetApplicationGUI();
  vtkImageData* SaveScreenShot();
  void SaveMRMLScene();
  const char* GetIconName(vtkMRMLNode* node, bool isEdit=false);

  void AddRulerNodeObserver(vtkMRMLAnnotationRulerNode* rnode) ;

  vtkSlicerAnnotationFiducialManager* GetFiducialManager(){ return m_FiducialManager; }

  int TestReceivedMessage; 

protected:
  vtkSlicermiAnnotationModuleLogic();
  ~vtkSlicermiAnnotationModuleLogic();
  // not implemented
  vtkSlicermiAnnotationModuleLogic(const vtkSlicermiAnnotationModuleLogic&);
  void operator=(const vtkSlicermiAnnotationModuleLogic&);


private:
  CTK_DECLARE_PRIVATE(vtkSlicermiAnnotationModuleLogic);

  // Fiducial Related Private Functions
  vtkSlicerFiducialListWidget* GetFiducialWidget(const char *nodeID);
  void AddFiducialWidget(vtkMRMLAnnotationFiducialNode *cpNode);
  void RemoveFiducialWidget(vtkMRMLAnnotationFiducialNode *cpNode);

  vtkSlicerAnnotationRulerManager *m_RulerManager;
  vtkSlicerAnnotationAngleManager *m_AngleManager;
  vtkSlicerAnnotationFiducialManager *m_FiducialManager;
  vtkSlicerAnnotationROIManager *m_ROIManager;
  vtkSlicerAnnotationSplineManager *m_SplineManager;
  vtkSlicerAnnotationBidimensionalManager *m_BidimensionalManager;
  vtkSlicerAnnotationTextManager *m_TextManager;

};

#endif
