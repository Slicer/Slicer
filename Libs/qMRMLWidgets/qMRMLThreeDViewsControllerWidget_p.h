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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLThreeDViewsControllerWidget_p_h
#define __qMRMLThreeDViewsControllerWidget_p_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLThreeDViewsControllerWidget.h"
#include "ui_qMRMLThreeDViewsControllerWidget.h"

class QAction;

//-----------------------------------------------------------------------------
class qMRMLThreeDViewsControllerWidgetPrivate: public QObject,
                                   public ctkPrivate<qMRMLThreeDViewsControllerWidget>,
                                   public Ui_qMRMLThreeDViewsControllerWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  CTK_DECLARE_PUBLIC(qMRMLThreeDViewsControllerWidget);
  qMRMLThreeDViewsControllerWidgetPrivate();
  ~qMRMLThreeDViewsControllerWidgetPrivate();

  void setupUi(qMRMLWidget* widget);

public:

  QAction* actionFitToWindow;
  
};

#endif
