

#ifndef __qMRMLChartView_p_h
#define __qMRMLChartView_p_h

// Qt includes
class QToolButton;

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>
class ctkPopupWidget;

// qMRML includes
#include "qMRMLChartView.h"

class vtkMRMLChartViewNode;
class vtkMRMLChartNode;
class vtkMRMLColorLogic;
class vtkMRMLColorNode;
class vtkMRMLDoubleArrayNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qMRMLChartViewPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLChartView);
protected:
  qMRMLChartView* const q_ptr;
public:
  qMRMLChartViewPrivate(qMRMLChartView& object);
  ~qMRMLChartViewPrivate();

  virtual void init();

  void setMRMLScene(vtkMRMLScene* scene);
  vtkMRMLScene *mrmlScene();

public slots:
  /// Handle MRML scene event
  void startProcessing();
  void endProcessing();

  void updateWidgetFromMRML();

  /// slot when the view is configured to look at a different chart node
  void onChartNodeChanged(); 

  // slot when a data point is clicked
  void onDataPointClicked(int series, int pointidx, double x, double y);

protected:

  // Convert the colors in a color node to a string that can be used
  // as the "seriesColors" field for the chart. This is used to assign
  // colors to multiple series.
  QString seriesColorsString(vtkMRMLColorNode *);

  // Convert the colors in ta color node to a string that can be used
  // as the "seriesColors" field for a specific series.  This requires
  // evaluating the lookup table for the values (X component) actually
  // used in the order they are used. This method is only used with
  // bar (and pie?)  charts to color each bar of series a different
  // color.
  QString seriesColorsString(vtkMRMLColorNode*, vtkMRMLDoubleArrayNode*);

  // Convert a data array into a string that can be passed as the data
  // for a series.
  QString seriesDataString(vtkMRMLDoubleArrayNode*);

  // Convert a data array into a string that can be passed as the data
  // for a series. This version will use values in the ArrayNode to
  // lookup names in a ColorNode.
  QString seriesDataString(vtkMRMLDoubleArrayNode*, vtkMRMLColorNode*);
  

  vtkMRMLScene*                      MRMLScene;
  vtkMRMLChartViewNode*              MRMLChartViewNode;
  vtkMRMLChartNode*                  MRMLChartNode;

  vtkMRMLColorLogic*                 ColorLogic;
  
  QToolButton*                       PinButton;
  ctkPopupWidget*                    PopupWidget;
};

#endif
