/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

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
