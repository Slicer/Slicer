

#ifndef __qMRMLChartWidget_h
#define __qMRMLChartWidget_h

// Qt includes
#include <QWidget>
class QResizeEvent;

// qMRMLWidget includes
#include "qMRMLWidget.h"
class qMRMLChartViewControllerWidget;
class qMRMLChartView;
class qMRMLChartWidgetPrivate;

// MRML includes
class vtkMRMLChartViewNode;
class vtkMRMLColorLogic;
class vtkMRMLScene;

class QMRML_WIDGETS_EXPORT qMRMLChartWidget : public qMRMLWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;
  
  /// Constructors
  explicit qMRMLChartWidget(QWidget* parent = 0);
  virtual ~qMRMLChartWidget();

  /// Get the chart node observed by view.
  vtkMRMLChartViewNode* mrmlChartViewNode()const;

  /// Get a reference to the underlying Chart View
  /// Becareful if you change the ChartView, you might
  /// unsynchronize the view from the nodes/logics.
  Q_INVOKABLE qMRMLChartView* chartView()const;

  /// \sa qMRMLChartView::addDisplayableManager
  void addDisplayableManager(const QString& displayableManager);

  /// \sa qMRMLChartControllerWidget::chartViewLabel()
  /// \sa setChartViewLabel()
  QString viewLabel()const;

  /// \sa qMRMLChartControllerWidget::chartViewLabel()
  /// \sa chartViewLabel()
  void setViewLabel(const QString& newChartViewLabel);

  /// Set the color logic that is used by the view.
  void setColorLogic(vtkMRMLColorLogic* colorLogic);
  vtkMRMLColorLogic* colorLogic()const;

public slots:
  /// Set the current \a viewNode to observe
  void setMRMLChartViewNode(vtkMRMLChartViewNode* newChartViewNode);

protected:
  QScopedPointer<qMRMLChartWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLChartWidget);
  Q_DISABLE_COPY(qMRMLChartWidget);
};

#endif
