#ifndef __qMRMLChartView_p_h
#define __qMRMLChartView_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Qt includes
class QToolButton;

// VTK includes
#include <vtkWeakPointer.h>

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
class vtkStringArray;

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
  ~qMRMLChartViewPrivate() override;

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

  // slot when mouse is over a point
  void onDataMouseOver(int series, int pointidx, double x, double y);

  // slot when a data point is clicked
  void onDataPointClicked(int series, int pointidx, double x, double y);


protected:

  // Convert the colors in a color node to a string that can be used
  // as the "seriesColors" field for the chart. This is used to assign
  // colors to multiple series.
  QString seriesColorsString(vtkMRMLColorNode *);

  // Convert the colors in the color node to a string that can be used
  // as the "seriesColors" field for a specific series.  This requires
  // evaluating the lookup table for the values (X component) actually
  // used in the order they are used. This method is only used with
  // bar (and pie?) charts to color each bar of series a different
  // color.
  QString seriesColorsString(vtkMRMLColorNode*, vtkMRMLDoubleArrayNode*);

  // Convert a data array into a string that can be passed as the data
  // for a series.
  QString seriesDataString(vtkMRMLDoubleArrayNode*);

  // Convert a data array into a string that can be passed as the data
  // for a series. This version will use values in the ArrayNode to
  // lookup names in a ColorNode.
  QString seriesLabelDataString(vtkMRMLDoubleArrayNode*, vtkMRMLColorNode*);

  // Convert a data array into a string by only taking the dependent
  // variables. This method is used when categorical or date indexed
  // data where the independent variables will be specified using a
  // separate call to *Ticks*String().
  QString seriesDependentDataString(vtkMRMLDoubleArrayNode*);

  // Convert a data array containing values into a string for a box
  // plot. This method calculates the min, q1, median, q3, and max of
  // the dependent variables.
  QString seriesBoxDataString(vtkMRMLDoubleArrayNode*, unsigned int idx);

  // Convert a data array into a string that can be passed as tick locations
  // for a series. This uses just the independent variables.
  QString seriesTicksString(vtkMRMLDoubleArrayNode*);

  // Convert a data array containing dates into a string that can be
  // passed as tick locations for a series.
  QString seriesDateTicksString(vtkMRMLDoubleArrayNode*);

  // Convert a data array into a string with labels based on the color node
  QString seriesLabelTicksString(vtkMRMLDoubleArrayNode*, vtkMRMLColorNode*);

  // Convert the array names (for all the arrays) into a string that
  // can be passed as tick locations for a chart, e.g. Box chart
  QString arrayTicksString(vtkStringArray*);

  // Convert the data in all the arrays into a structure suitable for
  // plotting as lines.
  QString lineData(vtkMRMLChartNode*);

  // Generate x-axis tick locations for lines. Generates ticks only
  // for categorical and date axes. Defaults to jqPlot for
  // quantitative axes.
  QString lineXAxisTicks(vtkMRMLChartNode*);

  // Generate a string of options for a line chart
  QString lineOptions(vtkMRMLChartNode*);

  // Convert the data in the all the arrays into a structure suitable
  // for a scatter plot
  QString scatterData(vtkMRMLChartNode*);

  // Generate x-axis tick location for a scatter plot. Generates ticks
  // only for categorical and data axes. Defaults to jqPlot for
  // quantitative axes.
  QString scatterXAxisTicks(vtkMRMLChartNode*);

  // Generate a string of options for a scatter chart
  QString scatterOptions(vtkMRMLChartNode*);

  // Convert the data in all the arrays into a structure suitable for
  // bar chart. String will vary depending on whether the x-axis is
  // quantitative, categorical or date.
  QString barData(vtkMRMLChartNode*);

  // Generate x-axis tick locations for bars. Used for categorical and
  // data axes.  If an array has a color table and a categorical
  // x-axis, this method will lookup tissue names from the color table
  // to use as tick labels.
  QString barXAxisTicks(vtkMRMLChartNode*);

  // Generate a string of options for a bar chart
  QString barOptions(vtkMRMLChartNode*);

  // Convert the data in all the arrays into a box chart.  Unlike
  // line, scatter, and bar chart types, this method summarizes the
  // data from each array, with each summary forming one entry in the
  // chart, i.e. several series are summarized into a single series,
  // with one entry per original series.
  QString boxData(vtkMRMLChartNode*);

  // Generate the x-axis tick locations for a bar chart. Used for
  // categorical and data axes.
  QString boxXAxisTicks(vtkMRMLChartNode*);

  // Generate a string of options for a bar chart
  QString boxOptions(vtkMRMLChartNode*);

  // Generate a string containing the options that are common to all
  // char types
  QString genericOptions(vtkMRMLChartNode*, bool rotateXTickLabels = false);


  vtkMRMLScene*                      MRMLScene;
  vtkMRMLChartViewNode*              MRMLChartViewNode;
  vtkMRMLChartNode*                  MRMLChartNode;

  vtkWeakPointer<vtkMRMLColorLogic>  ColorLogic;

  QToolButton*                       PinButton;
  ctkPopupWidget*                    PopupWidget;
};

#endif
