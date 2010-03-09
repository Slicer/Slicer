#include <qMRMLItemHelper.h>
#include <QApplication>
#include <QDebug>
#include <QSharedPointer>

// STD includes
#include <stdlib.h>
#include <iostream>

class qMRMLRootItemHelper: public qMRMLAbstractRootItemHelper
{
public: 
  qMRMLRootItemHelper() : qMRMLAbstractRootItemHelper(0, 0)
  {
qDebug() << "constructor";
  
  }
  virtual ~qMRMLRootItemHelper()
  {
    qDebug() << "destructor";
  }
};

int qMRMLItemHelperTest1( int , char * [] )
{
  QSharedPointer<qMRMLAbstractItemHelper> t = 
    QSharedPointer<qMRMLAbstractItemHelper>(
      new qMRMLRootItemHelper);
  
  return EXIT_SUCCESS;
}
