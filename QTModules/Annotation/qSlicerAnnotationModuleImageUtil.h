/*=========================================================================

   Program: ParaView
   Module:    $RCSfile: pqImageUtil.h,v $

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#ifndef _qSlicerAnnotationModuleImageUtil_h
#define _qSlicerAnnotationModuleImageUtil_h

#include "qSlicerAnnotationModuleExport.h"

class QImage;
class vtkImageData;
class QString;

/// Utility class to convert VTK images to Qt images and vice versa
class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT qSlicerAnnotationModuleImageUtil
{
public:
  /// convert a QImage to vtkImageData
  static bool toImageData(const QImage& img, vtkImageData* vtkimage);

  /// convert vtkImageData to QImage
  static bool fromImageData(vtkImageData* vtkimage, QImage& img);

  /// Save an image to a file. Determines the type of the file using the file
  /// extension. Returns the vtkErrorCode on error (vtkErrorCode::NoError i.e. 0
  /// if file is successfully saved).
  /// quality [0,100] -- 0 = low, 100=high, -1=default
  static int saveImage(vtkImageData* vtkimage, const QString& filename, int quality=-1);

  /// Save an image to a file. Determines the type of the file using the file
  /// extension. Returns the vtkErrorCode on error (vtkErrorCode::NoError i.e. 0
  /// if file is successfully saved).
  /// quality [0,100] -- 0 = low, 100=high, -1=default
  static int saveImage(const QImage& image, const QString& filename, int quality=-1);

  static int SaveImage(vtkImageData* image, const char* filename, int quality);
  static int SaveImage(vtkImageData* image, const char* filename)
  { return qSlicerAnnotationModuleImageUtil::SaveImage(image, filename, -1); }

};

#endif

