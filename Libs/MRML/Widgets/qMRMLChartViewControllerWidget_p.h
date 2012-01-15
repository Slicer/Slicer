#ifndef __qMRMLChartViewControllerWidget_p_h
#define __qMRMLChartViewControllerWidget_p_h

// qMRML includes
#include "qMRMLChartViewControllerWidget.h"
#include "qMRMLViewControllerBar_p.h"
#include "ui_qMRMLChartViewControllerWidget.h"

// VTK includes
#include <vtkWeakPointer.h>

class QAction;
class ctkButtonGroup;
class ctkSignalMapper;
class qMRMLSceneViewMenu;
class vtkMRMLChartViewNode;
class vtkMRMLChartNode;
class QString;

//-----------------------------------------------------------------------------
class qMRMLChartViewControllerWidgetPrivate
  : public qMRMLViewControllerBarPrivate
  , public Ui_qMRMLChartViewControllerWidget
{
  Q_DECLARE_PUBLIC(qMRMLChartViewControllerWidget);
public:
  typedef qMRMLViewControllerBarPrivate Superclass;
  qMRMLChartViewControllerWidgetPrivate(qMRMLChartViewControllerWidget& object);
  virtual ~qMRMLChartViewControllerWidgetPrivate();

  virtual void init();

  vtkWeakPointer<vtkMRMLChartViewNode>  ChartViewNode;
  qMRMLChartView*                       ChartView;

  ctkSignalMapper*                 StereoTypesMapper;
  ctkButtonGroup*                  AnimateViewButtonGroup;
  
  QString                          ChartViewLabel;

public slots:
  /// Called after a chart node is selected
  /// using the associated qMRMLNodeComboBox
  void onChartNodeSelected(vtkMRMLNode* node);
  

protected:
  virtual void setupPopupUi();

public:
  vtkMRMLChartNode*        MRMLChartNode;
};

#endif
