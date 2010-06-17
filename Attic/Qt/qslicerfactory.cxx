/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the documentation of Qt. It was originally
** published as part of Qt Quarterly.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include <QtGui>
#include <QtWebKit>
#include "qslicerfactory.h"

#include "vtkview.h"
#include "slicerview.h"
#include "vtkKWApplication.h"

QSlicerFactory::QSlicerFactory(QWebView *webView, QObject *parent, vtkKWApplication *kwapp)
    : QWebPluginFactory(parent)
{
    this->webView = webView;
    this->kwapp = kwapp;
};

QObject *QSlicerFactory::create(const QString &mimeType, const QUrl &url,
                            const QStringList &argumentNames,
                            const QStringList &argumentValues) const
{
    if (mimeType == "application/qslicer")
      {
      SlicerView *slicerview = new SlicerView(NULL, this->kwapp);
      return slicerview;
      }

    if (mimeType == "text/qvtk")
      {
      VTKView *vtkview = new VTKView;
      return vtkview;
      }

    return 0;
}

QList<QWebPluginFactory::Plugin> QSlicerFactory::plugins() const
{
    // VTK
    QWebPluginFactory::MimeType vtkmimeType;
    vtkmimeType.name = "text/qvtk";
    vtkmimeType.description = "qvtk render widget";
    vtkmimeType.fileExtensions = QStringList() << "qvtk";

    QWebPluginFactory::Plugin vtkplugin;
    vtkplugin.name = "QSlicer view viewer";
    vtkplugin.description = "A QSlicer Web plugin.";
    vtkplugin.mimeTypes = QList<MimeType>() << vtkmimeType;

    // Slicer
    QWebPluginFactory::MimeType slicermimeType;
    slicermimeType.name = "application/qslicer";
    slicermimeType.description = "qslicer render widget";
    slicermimeType.fileExtensions = QStringList() << "qslicer";

    QWebPluginFactory::Plugin slicerplugin;
    slicerplugin.name = "QSlicer view viewer";
    slicerplugin.description = "A QSlicer Web plugin.";
    slicerplugin.mimeTypes = QList<MimeType>() << slicermimeType;

    return QList<QWebPluginFactory::Plugin>() << vtkplugin << slicerplugin;
}
