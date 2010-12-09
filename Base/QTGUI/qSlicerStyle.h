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

#ifndef __qSlicerStyle_h
#define __qSlicerStyle_h

// Qt includes
#include <QProxyStyle>

#include "qSlicerBaseQTGUIExport.h"


class Q_SLICER_BASE_QTGUI_EXPORT qSlicerStyle : public QProxyStyle
{
public:
  /// Superclass typedef
  typedef QProxyStyle Superclass;

  /// Constructors
  qSlicerStyle();
  virtual ~qSlicerStyle();

  virtual int pixelMetric(PixelMetric metric, const QStyleOption * option = 0,
                          const QWidget * widget = 0)const;

  virtual QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                               SubControl subControl, const QWidget *widget) const;
  virtual QPalette standardPalette()const;
};

#endif

