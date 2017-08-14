/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QCLEANLOOKSSTYLE_H
#define QCLEANLOOKSSTYLE_H

#include <QtWidgets/qproxystyle.h>
#include <QtCore/qelapsedtimer.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QProgressBar;

class QCleanlooksStyle : public QProxyStyle
{
    Q_OBJECT

public:
    QCleanlooksStyle();
    ~QCleanlooksStyle();

    QPalette standardPalette () const Q_DECL_OVERRIDE;
    void drawPrimitive(PrimitiveElement elem,
                        const QStyleOption *option,
                        QPainter *painter, const QWidget *widget = 0) const Q_DECL_OVERRIDE;
    void drawControl(ControlElement ce, const QStyleOption *option, QPainter *painter,
                                const QWidget *widget) const Q_DECL_OVERRIDE;
    int pixelMetric(PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const Q_DECL_OVERRIDE;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
                            QPainter *painter, const QWidget *widget) const Q_DECL_OVERRIDE;
    QRect subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget = 0) const Q_DECL_OVERRIDE;
    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
                           const QSize &size, const QWidget *widget) const Q_DECL_OVERRIDE;
    SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                               const QPoint &pt, const QWidget *w = 0) const Q_DECL_OVERRIDE;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
                         SubControl sc, const QWidget *widget) const Q_DECL_OVERRIDE;
    QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap,
                                        const QStyleOption *opt) const Q_DECL_OVERRIDE;
    int styleHint(StyleHint hint, const QStyleOption *option = 0, const QWidget *widget = 0,
                  QStyleHintReturn *returnData = 0) const Q_DECL_OVERRIDE;
    QRect itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const Q_DECL_OVERRIDE;
    QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option = 0,
                       const QWidget *widget = 0) const Q_DECL_OVERRIDE;
    QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt,
                           const QWidget *widget = 0) const Q_DECL_OVERRIDE;
    void drawItemPixmap(QPainter *painter, const QRect &rect,
                        int alignment, const QPixmap &pixmap) const Q_DECL_OVERRIDE;
    void drawItemText(QPainter *painter, const QRect &rect,
                              int flags, const QPalette &pal, bool enabled,
                              const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const Q_DECL_OVERRIDE;
    void polish(QWidget *widget) Q_DECL_OVERRIDE;
    void polish(QApplication *app) Q_DECL_OVERRIDE;
    void polish(QPalette &pal) Q_DECL_OVERRIDE;
    void unpolish(QWidget *widget) Q_DECL_OVERRIDE;
    void unpolish(QApplication *app) Q_DECL_OVERRIDE;


protected:
    void timerEvent(QTimerEvent *) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *o, QEvent *e) Q_DECL_OVERRIDE;
    void startProgressAnimation(QObject *o, QProgressBar *bar);
    void stopProgressAnimation(QObject *o, QProgressBar *bar);

private:
    int animateStep;
    int animateTimer;
    QElapsedTimer startTime;
    QList<QProgressBar *> animatedProgressBars;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QCLEANLOOKSSTYLE_H
