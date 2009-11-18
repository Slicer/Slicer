#ifndef __qMRMLNodeTreeSelector_h
#define __qMRMLNodeTreeSelector_h

#include "qMRMLNodeSelector.h"
#include "qMRMLWidgetsWin32Header.h"

class QMRML_WIDGETS_EXPORT qMRMLNodeTreeSelector : public qMRMLNodeSelector
{
  Q_OBJECT
public:
  // Superclass typedef
  typedef qMRMLNodeSelector Superclass;
  
  // Constructors
  qMRMLNodeTreeSelector(QWidget* parent = 0);
  virtual ~qMRMLNodeTreeSelector();

protected:
  virtual void addNodeInternal(vtkMRMLNode* mrmlNode);
private:
  void addItemInternal(int index, const QIcon &icon, 
                       const QString &text, const QVariant &userData);

  struct qInternal; 
  qInternal * Internal;
};

#endif
