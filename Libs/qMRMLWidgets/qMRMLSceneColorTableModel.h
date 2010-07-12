#ifndef __qMRMLSceneColorTableModel_h
#define __qMRMLSceneColorTableModel_h

#include "qMRMLSceneTreeModel.h"

class QMRML_WIDGETS_EXPORT qMRMLSceneColorTableModel : public qMRMLSceneTreeModel
{
  Q_OBJECT

public:
  qMRMLSceneColorTableModel(QObject *parent=0);
  virtual ~qMRMLSceneColorTableModel();

};

#endif
