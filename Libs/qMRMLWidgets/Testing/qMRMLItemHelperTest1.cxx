#include <qMRMLItemHelper.h>
#include <QApplication>
#include <QDebug>
#include <QSharedPointer>

// STD includes
#include <stdlib.h>
#include <iostream>

class qMRMLRootItemHelper2: public qMRMLRootItemHelper
{
public: 
  qMRMLRootItemHelper2() : qMRMLRootItemHelper(0, 0)
  {
qDebug() << "constructor";
  
  }
  virtual ~qMRMLRootItemHelper2()
  {
    qDebug() << "destructor";
  }
};

int qMRMLItemHelperTest1( int , char * [] )
{
  QSharedPointer<qMRMLAbstractItemHelper> t = 
    QSharedPointer<qMRMLAbstractItemHelper>(
      new qMRMLRootItemHelper2);
  
  return EXIT_SUCCESS;
}
