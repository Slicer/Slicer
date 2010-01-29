#ifndef __qMRMLNodeTreeSelector_h
#define __qMRMLNodeTreeSelector_h

/// qMRML includes
#include "qMRMLNodeSelector.h"

/// qCTK includes
#include <qCTKPimpl.h>

#include "qMRMLWidgetsExport.h"

class qMRMLNodeTreeSelectorPrivate;

class QMRML_WIDGETS_EXPORT qMRMLNodeTreeSelector : public qMRMLNodeSelector
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLNodeSelector Superclass;
  
  /// Constructors
  explicit qMRMLNodeTreeSelector(QWidget* parent = 0);
  virtual ~qMRMLNodeTreeSelector(){}

protected:
  virtual void addNodeInternal(vtkMRMLNode* mrmlNode);
  
private:
  QCTK_DECLARE_PRIVATE(qMRMLNodeTreeSelector);
};

#endif
