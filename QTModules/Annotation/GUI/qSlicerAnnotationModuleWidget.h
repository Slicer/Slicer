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
class qSlicerAnnotationModuleSnapShotDialog;
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

/// \ingroup Slicer_QtModules_Annotation
class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT qSlicerAnnotationModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
    typedef qSlicerAbstractModuleWidget Superclass;
    qSlicerAnnotationModuleWidget(QWidget *parent=0);
    ~qSlicerAnnotationModuleWidget();

    void refreshTree();

    /// User clicked on property edit button
    void propertyEditButtonClicked(QString mrmlId);


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
        Screenshot,
      };

protected:

protected slots:

    // Table and Property Modify
    void moveDownSelected();
    void moveUpSelected();  


    // Save
    void onSaveMRMLSceneButtonClicked();
    void onSaveAnnotationButtonClicked();


  //------------------------------------------------------------------
  // Daniel's approved code starting here

  /// Add a new hierarchy.
  void onAddHierarchyButtonClicked();

  /// Restore the view to when the selected annotation was created.
  void onRestoreViewButtonClicked();

  /// Select all annotations
  void selectAllButtonClicked();

  /// Toggle the visibility of selected Annotations
  void visibleSelectedButtonClicked();

  /// Un-/Lock selected Annotations
  void lockSelectedButtonClicked();

  /// Delete selected Annotations
  void deleteSelectedButtonClicked();

  // Property dialog
  void propertyRestored();
  void propertyAccepted();

  // Snapshot dialog
  void snapshotRejected();
  void snapshotAccepted();
  void onSnapShotButtonClicked();

  // Report dialog
  void reportDialogRejected();
  void reportDialogAccepted();
  void onReportButtonClicked();

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

  // ROI Node
  void onROINodeButtonClicked();

protected:
  QScopedPointer<qSlicerAnnotationModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerAnnotationModuleWidget);
  Q_DISABLE_COPY(qSlicerAnnotationModuleWidget);

  virtual void setup();

  qSlicerAnnotationModulePropertyDialog* m_PropertyDialog;
  qSlicerAnnotationModuleReportDialog* m_ReportDialog;
  qSlicerAnnotationModuleSnapShotDialog* m_SnapShotDialog;

  /// Type of current Annotations - described by enum
  int m_CurrentAnnotationType;

};

#endif
