#ifndef __qSlicerAnnotationModuleWidget_h
#define __qSlicerAnnotationModuleWidget_h

// SlicerQT includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerAnnotationModuleExport.h"
#include "GUI/qSlicerAnnotationModulePropertyDialog.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>


class qSlicerAnnotationModuleReportDialog;
class qSlicerAnnotationModuleScreenShotDialog;
class vtkSlicerNodeSelectorWidget;
class vtkMRMLAnnotationRulerNode;
class vtkMRMLAnnotationAngleNode;
class qSlicerAnnotationModuleWidgetPrivate;
class QTableWidgetItem;
class QLineEdit;
class QTextDocument;
class QGridLayout;
class vtkMRMLAnnotationLinesNode;
class vtkMRMLAnnotationControlPointsNode;
class vtkMRMLAnnotationNode;
class vtkMRMLAnnotationFiducialNode;
class vtkMRMLNode;
class vtkMRMLAnnotationDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;

class vtkMRMLInteractionNode;
class vtkObserverManager;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT qSlicerAnnotationModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
    typedef qSlicerAbstractModuleWidget Superclass;
    qSlicerAnnotationModuleWidget(QWidget *parent=0);
    ~qSlicerAnnotationModuleWidget();


    // Add a new annotation node to the list
    void addNodeToTable(const char* newNodeID);

    // Update an existing annotation in the list
    void updateAnnotationInTableByID(const char* id, std::vector<double> value, const char* format);

    //BTX
    /// Different Annotation Types
    enum
      {
        TextNode = 1000,
        FiducialNode,
        AngleNode,
        StickyNode,
        SplineNode,
        RulerNode,
        BidimensionalNode,
        ROINode,
      };


protected:


protected slots:

    // Fiducial
    void StartAddingFiducials();
    void StopAddingFiducials();
    void onAddFiducialsButtonToggled(bool toggle);
    void onFiducialNodeAdded(vtkObject* fiducialList);
    void onFiducialNodeModified(vtkObject* fiducialList, void* call_data);
    void updateFiducialTable(int index = -1);
    void onFiducialNodeRemoved(vtkObject* fiducialList);
  void AddFiducialCompleted(vtkObject* object, void* call_data);

    // Ruler
    void onCreateMeasurementRulerButtonClicked();

    // Angle
    void onCreateMeasurementAngleButtonToggled(bool toggle);
    void AddAngleCompleted(vtkObject* object, void* call_data);



  // ROI Node
  void onROINodeButtonClicked();


    // Table and Property Modify
    void moveDownSelected();
    void moveUpSelected();  
    void selectedAllButtonClicked();
    void onLockUnlockAllButtonClicked();
    void visibleSelectedButtonClicked();
    void lockSelectedButtonClicked();
    void deleteSelectedButtonClicked();


    void annotationTextChanged(QString text, char* nodeId);
    void annotationCoordinateChanged(QString valueString, char* nodeId);
    void selectRowByIndex(int index);
    void updateAnnotationTable(int index, std::vector<double> thevalue, const char* format);

    int getIndexByNodeID(const char* nodeID);
    void updateAnnotationText(int row, int col);
    void onItemSelectionChanged();

    // Save
    void onSaveMRMLSceneButtonClicked();
    void onSaveAnnotationButtonClicked();
    void onScreenShotButtonClicked();
    void onGenerateReportButtonClicked();
    bool saveAnnotationReport();
    bool saveScreenShot();

    //void AddTextNodeCompleted(vtkObject* object, void* call_data);

  //------------------------------------------------------------------
  // Daniel's approved code starting here

  // Property dialog
  void propertyRestored();
  void propertyAccepted();
  void propertyEditButtonClicked();

  // Mouse Mode Operation
  void enableMouseModeButtons();
  void disableMouseModeButtons();

  void onResumeButtonClicked();
  void onPauseButtonClicked();
  void onCancelButtonClicked();
  void onDoneButtonClicked();

  void enableAllAnnotationTools();
  void disableAllAnnotationTools();
  void resetAllAnnotationTools();
  void cancelOrRemoveLastAddedAnnotationNode();

  // Sticky Node (semi-approved)
  void onStickyNodeButtonClicked();

  // Angle Node
  void onAngleNodeButtonClicked();

  // Text Node
  void onTextNodeButtonClicked();

  // Fiducial Node
  void onFiducialNodeButtonClicked();

  // Spline Node
  void onSplineNodeButtonClicked();

  // Ruler Node
  void onRulerNodeButtonClicked();

  // Bidimensional Node
  void onBidimensionalNodeButtonClicked();

private:
  CTK_DECLARE_PRIVATE(qSlicerAnnotationModuleWidget);

  virtual void setup();
  QString getAnnotationIconName(int index, bool isEdit=false);

  qSlicerAnnotationModulePropertyDialog* m_PropertyDialog;
  qSlicerAnnotationModuleReportDialog* m_ReportDialog;
  qSlicerAnnotationModuleScreenShotDialog *m_ScreenShotDialog;
  QString m_report;
  int m_index;
  int m_lastAddedIndex;
  std::vector<const char*> m_IDs;
  QPixmap m_screenshot;
  QStringList m_screenshotList;


  /// Type of current Annotations - described by enum
  int m_CurrentAnnotationType;

};

#endif
