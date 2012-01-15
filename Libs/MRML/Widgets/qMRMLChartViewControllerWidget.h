

#ifndef __qMRMLChartViewControllerWidget_h
#define __qMRMLChartViewControllerWidget_h

// CTK includes
#include <ctkAxesWidget.h>
#include <ctkVTKObject.h>

// qMRMLWidget includes
#include "qMRMLViewControllerBar.h"
class qMRMLChartViewControllerWidgetPrivate;
class qMRMLChartView;

// MRML includes
class vtkMRMLChartViewNode;

class QMRML_WIDGETS_EXPORT qMRMLChartViewControllerWidget
  : public qMRMLViewControllerBar
{
  Q_OBJECT
  QVTK_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLViewControllerBar Superclass;

  /// Constructors
  explicit qMRMLChartViewControllerWidget(QWidget* parent = 0);
  virtual ~qMRMLChartViewControllerWidget();

  /// Set the label for the 3D view (abbreviation for the view
  /// name)
  void setViewLabel(const QString& newViewLabel);

  /// Get the label for the view (abbreviation for the view name)
  QString viewLabel()const;

public slots:
  void setChartView(qMRMLChartView* ChartView);
  void setMRMLChartViewNode(vtkMRMLChartViewNode* chartViewNode);

protected slots:
  void updateWidgetFromMRML();

private:
  Q_DECLARE_PRIVATE(qMRMLChartViewControllerWidget);
  Q_DISABLE_COPY(qMRMLChartViewControllerWidget);
};

#endif
