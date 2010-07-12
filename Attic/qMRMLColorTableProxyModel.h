#ifndef __qMRMLColorTableProxyModel_h
#define __qMRMLColorTableProxyModel_h

#include "qMRMLTreeProxyModel.h"

class qMRMLColorTableProxyModelPrivate;

class QMRML_WIDGETS_EXPORT qMRMLColorTableProxyModel : public qMRMLTreeProxyModel
{
  Q_OBJECT

public:
  qMRMLColorTableProxyModel(QObject *parent=0);
  virtual ~qMRMLColorTableProxyModel();

protected:
  virtual qMRMLAbstractItemHelperFactory* itemFactory()const;

private:
  CTK_DECLARE_PRIVATE(qMRMLColorTableProxyModel);
};

#endif
