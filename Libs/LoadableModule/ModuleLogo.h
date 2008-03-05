/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   Module Description Parser
  Module:    $HeadURL: http://www.na-mic.org/svn/Slicer3/trunk/Libs/ModuleDescriptionParser/ModuleDescription.h $
  Date:      $Date: 2006-11-10 15:42:21 -0500 (Fri, 10 Nov 2006) $
  Version:   $Revision: 1562 $

==========================================================================*/

#ifndef __ModuleLogo_h
#define __ModuleLogo_h

#include "ModuleDescriptionParserWin32Header.h"
#include <string>

class ModuleDescriptionParser_EXPORT ModuleLogo
{
public:
  ModuleLogo();
  ModuleLogo(const ModuleLogo &logo);
  virtual ~ModuleLogo();
  
  void operator=(const ModuleLogo &logo);

  void SetLogo(unsigned char* logo, int width, int height, int pixelSize, unsigned long bufferLength, int options);

  int GetWidth() const;
  int GetHeight() const;
  int GetPixelSize() const;
  unsigned long GetBufferLength() const;
  int GetOptions() const;
  const unsigned char* GetLogo() const;

private:
  int Width;
  int Height;
  int PixelSize;
  unsigned long BufferLength;
  int Options;
  std::string Logo;
};

#endif
