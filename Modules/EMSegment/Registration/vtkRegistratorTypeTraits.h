/*=auto=======================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkRegistratorTypeTraits.h,v $
  Date:      $Date: 2006/03/17 15:10:10 $
  Version:   $Revision: 1.2 $
  Author:    $Sylvain Jaume (MIT)$

=======================================================================auto=*/

#ifndef __vtkRegistratorTypeTraits_h
#define __vtkRegistratorTypeTraits_h

//BTX
//
// We limit the number of instantiations of registration code (and
// thus code bloat) by explicit specialization of this primary
// template.  Only double, float, short, and unsigned short are
// currently instantiated.  All other voxel types are converted to one
// of these four.
template <class CommonVoxelType>
class RegistrationVoxelTypeTraits
{
public:
  typedef float RegistrationVoxelType;
};

template<>
class RegistrationVoxelTypeTraits<double>
{
public:
  typedef double RegistrationVoxelType;
};

template<>
class RegistrationVoxelTypeTraits<unsigned char>
{
public:
  typedef short RegistrationVoxelType;
};

template<>
class RegistrationVoxelTypeTraits<char>
{
public:
  typedef short RegistrationVoxelType;
};

template<>
class RegistrationVoxelTypeTraits<signed char>
{
public:
  typedef short RegistrationVoxelType;
};

template<>
class RegistrationVoxelTypeTraits<unsigned short>
{
public:
  typedef int RegistrationVoxelType;
};

template<>
class RegistrationVoxelTypeTraits<short>
{
public:
  typedef short RegistrationVoxelType;
};
//ETX

#endif // __vtkRegistratorTypeTraits_h

