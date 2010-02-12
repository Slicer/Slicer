#ifndef __qCTKButtonGroup_h
#define __qCTKButtonGroup_h

// Qt includes
#include <QButtonGroup>

// qCTK includes
#include "qCTKPimpl.h"
#include "qCTKWidgetsExport.h"

class qCTKButtonGroupPrivate;

///
/// The qCTKButtonGroup class is a QButtonGroup that has a slightly
/// different behavior when exclusive. By default, a qCTKButtonGroup is 
/// exclusive. An exclusive qCTKButtonGroup switches off the button that
///  was clicked if it was already checked.
class QCTK_WIDGETS_EXPORT qCTKButtonGroup : public QButtonGroup
{
  Q_OBJECT
public:
  explicit qCTKButtonGroup(QObject *_parent = 0);

protected slots:
  void onButtonClicked(QAbstractButton * button);
  void onButtonPressed(QAbstractButton * button);

private:
  QCTK_DECLARE_PRIVATE(qCTKButtonGroup);
};

#endif
