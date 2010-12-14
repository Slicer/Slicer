#ifndef __qSlicerSceneViewsModuleWidget_h
#define __qSlicerSceneViewsModuleWidget_h

// SlicerQT includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerSceneViewsModuleExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>


class qSlicerSceneViewsModuleDialog;
class vtkMRMLSceneViewNode;
class qSlicerSceneViewsModuleWidgetPrivate;

class vtkMRMLNode;


class Q_SLICER_QTMODULES_SCENEVIEWS_EXPORT qSlicerSceneViewsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
    typedef qSlicerAbstractModuleWidget Superclass;
    qSlicerSceneViewsModuleWidget(QWidget *parent=0);
    ~qSlicerSceneViewsModuleWidget();

    void refreshTree();

    /// User clicked on property edit button
    void propertyEditButtonClicked(QString mrmlId);

    /// User clicked on restore button
    void restoreButtonClicked(QString mrmlId);

protected:

protected slots:

  void moveDownSelected();
  void moveUpSelected();

  /// Delete selected Annotations
  void deleteSelectedButtonClicked();

  // Snapshot dialog
  void snapshotRejected();
  void snapshotAccepted();
  void onSnapShotButtonClicked();

protected:
  QScopedPointer<qSlicerSceneViewsModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSceneViewsModuleWidget);
  Q_DISABLE_COPY(qSlicerSceneViewsModuleWidget);

  virtual void setup();

  qSlicerSceneViewsModuleDialog* m_SnapShotDialog;

};

#endif
