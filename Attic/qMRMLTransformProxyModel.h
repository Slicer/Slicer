#ifndef __qMRMLTransformProxyModel_h
#define __qMRMLTransformProxyModel_h

#include "qMRMLTreeProxyModel.h"

class qMRMLTransformProxyModelPrivate;

class QMRML_WIDGETS_EXPORT qMRMLTransformProxyModel : public qMRMLTreeProxyModel
{
  Q_OBJECT

public:
  qMRMLTransformProxyModel(QObject *parent=0);
  virtual ~qMRMLTransformProxyModel();

protected:
  virtual qMRMLAbstractItemHelperFactory* itemFactory()const;

private:
  CTK_DECLARE_PRIVATE(qMRMLTransformProxyModel);
};

#endif
