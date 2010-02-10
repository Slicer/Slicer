// Qt includes
#include <QObject>

// qCTK includes
#include <qCTKPimpl.h>

// qVTK includes
#include "qVTKObject.h"
#include "qVTKWidgetsExport.h"

class qVTKObjectTestPrivate;

class QVTK_WIDGETS_EXPORT qVTKObjectTest: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
public:
  qVTKObjectTest();

  bool test();

  bool isPublicSlotCalled()const;
  bool isProtectedSlotCalled()const;
  bool isPrivateSlotCalled()const;

  void resetSlotCalls();

public slots:
  void onVTKObjectModifiedPublic();

protected slots:
  void onVTKObjectModifiedProtected();

private slots:
  void onVTKObjectModifiedPrivate();

signals:
  void signalEmitted();

private:
  QCTK_DECLARE_PRIVATE(qVTKObjectTest);
};

