#include "qSlicerIOManager.h"

#include <QString>
#include <QFileDialog>

//-----------------------------------------------------------------------------
class qSlicerIOManagerPrivate: public qCTKPrivate<qSlicerIOManager>
{
public:
  QCTK_DECLARE_PUBLIC(qSlicerIOManager);
};

//-----------------------------------------------------------------------------
qSlicerIOManager::qSlicerIOManager():Superclass()
{
  QCTK_INIT_PRIVATE(qSlicerIOManager);
}

//-----------------------------------------------------------------------------
qSlicerIOManager::~qSlicerIOManager()
{
}

//-----------------------------------------------------------------------------
QString qSlicerIOManager::getOpenFileName(QWidget* widget)const
{
  return QFileDialog::getOpenFileName(widget);
}
