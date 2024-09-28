/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __qSlicerWebPythonProxy_p_h
#define __qSlicerWebPythonProxy_p_h

#include "qSlicerBaseQTGUIExport.h"

//------------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerWebPythonProxyPrivate
{
public:
  virtual ~qSlicerWebPythonProxyPrivate();

  /// Keep track of user response to avoid going through ctk dialog to check
  bool isPythonEvaluationAllowed();

  bool PythonEvaluationAllowed{ false };
  bool Verbose{ false };
};

#endif
