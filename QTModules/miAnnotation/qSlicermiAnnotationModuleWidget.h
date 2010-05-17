#ifndef __qSlicermiAnnotationModuleWidget_h
#define __qSlicermiAnnotationModuleWidget_h

// SlicerQT includes
#include "qSlicerAbstractModuleWidget.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

#include "qSlicermiAnnotationModuleExport.h"
#include "qSlicermiAnnotationModuleAnnotationPropertyDialog.h"

class qSlicermiAnnotationModuleReportDialog;
class qSlicermiAnnotationModuleScreenShotDialog;
class vtkSlicerNodeSelectorWidget;
class vtkMRMLAnnotationRulerNode;
class vtkMRMLAnnotationAngleNode;
class qSlicermiAnnotationModuleWidgetPrivate;
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

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT qSlicermiAnnotationModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
    typedef qSlicerAbstractModuleWidget Superclass;
    qSlicermiAnnotationModuleWidget(QWidget *parent=0);
    ~qSlicermiAnnotationModuleWidget();
    typedef enum { FiducialPoint=0, Angle=1, Ruler=2 } AnnotationType;

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

    // Sticky Node
    void onStickyNodeButtonClicked();

  // Text Node
  void onTextNodeButtonClicked();
  void AddTextNodeCompleted(vtkObject* object, void* call_data);

  // ROI Node
  void onROINodeButtonClicked();

  // Polyline
  void onPolylineButtonClicked();

  // Spline
  void onSplineButtonClicked();

    // Table and Property Modify
    void moveDownSelected();
    void moveUpSelected();  
    void selectedAllButtonClicked();
    void onLockUnlockAllButtonClicked();
    void visibleSelectedButtonClicked();
    void lockSelectedButtonClicked();
    void deleteSelectedButtonClicked();
    void propertyEditButtonClicked();
    void propertyRestored(char* nodeID);
    void propertyAccepted(char* nodeID, QString text);
    void annotationTextChanged(QString text, char* nodeId);
    void annotationCoordinateChanged(QString valueString, char* nodeId);
    void selectRowByIndex(int index);
    void updateAnnotationTable(int index, double thevalue, const char* format);
    void updateValue(vtkObject* annotationNode, void* call_data);
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

    // Mouse Mode Operation
    void onPauseButtonToggled(bool toggle);

private:
  CTK_DECLARE_PRIVATE(qSlicermiAnnotationModuleWidget);

  qSlicermiAnnotationModuleAnnotationPropertyDialog* GetPropertyDialog(const char* id);
  void RemovePropertyDialog(const char* id);
  virtual void setup();
  QString getAnnotationIconName(int index, bool isEdit=false);

  std::map<std::string, qSlicermiAnnotationModuleAnnotationPropertyDialog*> m_PropertyDialogs;
  qSlicermiAnnotationModuleReportDialog* m_ReportDialog;
  qSlicermiAnnotationModuleScreenShotDialog *m_ScreenShotDialog;
  QString m_report;
  int m_index;
  std::vector<const char*> m_IDs;
  QPixmap m_screenshot;
  QStringList m_screenshotList;

};

#endif
