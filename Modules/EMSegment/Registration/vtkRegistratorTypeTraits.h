#ifndef __vtkRegistratorTypeTraits_h
#define __vtkRegistratorTypeTraits_h

//BTX
//
// We limit the number of instantiations of registration code (and
// thus code bloat) by explicit specialization of this primary
// template.  Only double, float, short, and unsigned short are
// currently instantiated.  All other voxel types are converted to one
// of these three.
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
  typedef unsigned short RegistrationVoxelType;
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
  typedef unsigned short RegistrationVoxelType;
};

template<>
class RegistrationVoxelTypeTraits<short>
{
public:
  typedef short RegistrationVoxelType;
};
//ETX

#endif // __vtkRegistratorTypeTraits_h
