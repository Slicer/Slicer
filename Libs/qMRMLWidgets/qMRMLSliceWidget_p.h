#ifndef __qMRMLSliceWidget_p_h
#define __qMRMLSliceWidget_p_h

/// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLSliceWidget.h"
#include "ui_qMRMLSliceWidget.h"

class vtkMRMLDisplayableManagerGroup;
class QResizeEvent;

//-----------------------------------------------------------------------------
class qMRMLSliceWidgetPrivate: public QObject,
                                   public ctkPrivate<qMRMLSliceWidget>,
                                   public Ui_qMRMLSliceWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  CTK_DECLARE_PUBLIC(qMRMLSliceWidget);
  qMRMLSliceWidgetPrivate();
  ~qMRMLSliceWidgetPrivate();

public slots:

  /// Handle MRML scene event
  void onSceneAboutToBeClosedEvent();
  void onSceneClosedEvent();
  void onSceneAboutToBeImportedEvent();
  void onSceneImportedEvent();
  void onSceneRestoredEvent();

  /// Connected with the imageDataModified signal invoked by qMRMLSliceControllerWidget
  /// \sa qMRMLSliceControllerWidget::imageDataModified
  void onImageDataModified(vtkImageData * imageData);

  void updateWidgetFromMRMLSliceNode();

public:

  vtkMRMLDisplayableManagerGroup*    DisplayableManagerGroup;
  QString                            ScriptDisplayableManagerDirectory;
  vtkMRMLSliceNode*                  MRMLSliceNode;
};

#endif
