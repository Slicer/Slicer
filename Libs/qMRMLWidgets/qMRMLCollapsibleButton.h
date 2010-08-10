#ifndef __qMRMLCollapsibleButton_h
#define __qMRMLCollapsibleButton_h

// CTK includes
#include <ctkCollapsibleButton.h>

// qMRMLWidget includes
#include "qMRMLWidgetsExport.h"

class QMRML_WIDGETS_EXPORT qMRMLCollapsibleButton : public ctkCollapsibleButton
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef ctkCollapsibleButton Superclass;
  
  /// Constructors
  explicit qMRMLCollapsibleButton(QWidget* parent = 0);
  virtual ~qMRMLCollapsibleButton(){}
protected:
  virtual void changeEvent(QEvent* event);
  void computePalette();
};

#endif
