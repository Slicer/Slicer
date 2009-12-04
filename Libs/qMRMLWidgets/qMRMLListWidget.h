#ifndef __qMRMLListWidget_h
#define __qMRMLListWidget_h

#include <QListView>
#include "qCTKPimpl.h"
#include "qMRMLWidgetsWin32Header.h"

class qMRMLListWidgetPrivate;
class vtkMRMLScene;

class QMRML_WIDGETS_EXPORT qMRMLListWidget : public QListView
{
  Q_OBJECT
public:
  qMRMLListWidget(QWidget *parent=0);
  virtual ~qMRMLListWidget();
                           
  vtkMRMLScene* mrmlScene()const;

public slots:
  void setMRMLScene(vtkMRMLScene* scene);

private:
  QCTK_DECLARE_PRIVATE(qMRMLListWidget);
};

#endif
