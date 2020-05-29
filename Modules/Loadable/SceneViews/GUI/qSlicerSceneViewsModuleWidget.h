#ifndef __qSlicerSceneViewsModuleWidget_h
#define __qSlicerSceneViewsModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerSceneViewsModuleExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>


class qSlicerSceneViewsModuleDialog;
class vtkMRMLSceneViewNode;
class qSlicerSceneViewsModuleWidgetPrivate;

class vtkMRMLNode;

class QUrl;

/// \ingroup Slicer_QtModules_SceneViews
class Q_SLICER_QTMODULES_SCENEVIEWS_EXPORT qSlicerSceneViewsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
    typedef qSlicerAbstractModuleWidget Superclass;
    qSlicerSceneViewsModuleWidget(QWidget *parent=nullptr);
    ~qSlicerSceneViewsModuleWidget() override;

  /// Set up the GUI from mrml when entering
  void enter() override;
  /// Disconnect from scene when exiting
  void exit() override;

  bool setEditedNode(vtkMRMLNode* node, QString role = QString(), QString context = QString()) override;

public slots:
    /// a public slot allowing other modules to open up the scene view capture
    /// dialog (get the module manager, get the module sceneviews, get the
    /// widget representation, then invoke this method, see qSlicerIOManager openSceneViewsDialog
    void showSceneViewDialog();

    /// User clicked on restore button
    void restoreSceneView(const QString& mrmlId);

    /// User clicked on property edit button
    void editSceneView(const QString& mrmlId);

    /// scene was closed or imported or restored or finished batch
    /// processing, reset as necessary
    void onMRMLSceneReset();

protected slots:

  void onSceneViewDoubleClicked(int row, int column);

  void onRestoreButtonClicked();
  void onEditButtonClicked();
  void onDeleteButtonClicked();

  void moveDownSelected(QString mrmlId);
  void moveUpSelected(QString mrmlId);

  /// Respond to scene events
  void onMRMLSceneEvent(vtkObject*, vtkObject* node);

  /// respond to mrml events
  void updateFromMRMLScene();

protected:
  QScopedPointer<qSlicerSceneViewsModuleWidgetPrivate> d_ptr;

  void setup() override;

private:
  Q_DECLARE_PRIVATE(qSlicerSceneViewsModuleWidget);
  Q_DISABLE_COPY(qSlicerSceneViewsModuleWidget);
};

#endif
