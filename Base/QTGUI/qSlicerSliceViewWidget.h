#ifndef __qSlicerSliceViewWidget_h
#define __qSlicerSliceViewWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

// qMRMLWidget includes
#include <qMRMLSliceViewWidget.h>

#include "qSlicerBaseQTGUIExport.h"

class qSlicerSliceViewWidgetPrivate;
class vtkMRMLScene;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerSliceViewWidget : public qMRMLSliceViewWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLSliceViewWidget Superclass;
  
  /// Constructors
  explicit qSlicerSliceViewWidget(const QString& sliceViewName, QWidget* parent = 0);
  virtual ~qSlicerSliceViewWidget(){}

  //sgui->GetLogic()->FitSliceToAll ( w, h );

public slots:
  /// 
  /// Set the MRML \a scene that should be listened for events
  virtual void setMRMLScene(vtkMRMLScene* scene);

  ///
  /// Set \a sliceNode
  virtual void setMRMLSliceNode(vtkMRMLSliceNode* sliceNode);

protected slots:
  virtual void onSliceOffsetValueChanged(double value);

protected:
  //void resizeEvent(QResizeEvent * event);

private:
  CTK_DECLARE_PRIVATE(qSlicerSliceViewWidget);
};

#endif

