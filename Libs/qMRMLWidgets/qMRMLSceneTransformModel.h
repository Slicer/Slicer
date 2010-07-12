#ifndef __qMRMLSceneTransformModel_h
#define __qMRMLSceneTransformModel_h

#include "qMRMLSceneTreeModel.h"

class QMRML_WIDGETS_EXPORT qMRMLSceneTransformModel : public qMRMLSceneTreeModel
{
  Q_OBJECT

public:
  qMRMLSceneTransformModel(QObject *parent=0);
  virtual ~qMRMLSceneTransformModel();
};

#endif
