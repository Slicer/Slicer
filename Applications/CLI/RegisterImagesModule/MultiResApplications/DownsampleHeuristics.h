#ifndef DownsampleHeuristics_h_
#define DownsampleHeuristics_h_

#include "itkArray2D.h"
#include <vnl/vnl_vector.h>

template<class PyramidFilterType>
void
scheduleImagePyramid(PyramidFilterType* pyramid)
{
  typedef typename PyramidFilterType::InputImageType ImageType;
  typename ImageType::ConstPointer image = pyramid->GetInput();

  typedef typename ImageType::RegionType RegionType;
  
  RegionType region = image->GetLargestPossibleRegion();
  
  // concept check 3D image
  
  typedef typename RegionType::SizeType SizeType;
  typedef typename RegionType::SizeValueType SizeValueType;
  
  const SizeType size = region.GetSize();
  
  typedef typename ImageType::SpacingType SpacingType;
  const SpacingType spacing = image->GetSpacing();

  unsigned int ninplanelevels = 0;
  unsigned int nalllevels = 0;
  
  typedef typename ImageType::SpacingValueType SpacingValueType;

  SpacingValueType max = spacing.GetVnlVector().max_value(); 
  unsigned int maxind = 0;
  if(spacing[1] == max)
    {
    maxind = 1;
    }
  else if(spacing[2] == max)
    {
    maxind = 2;
    }

  // Assuming the other two spacings are approximately equal by averaging them.
  SpacingValueType nonmax = (spacing[0] + spacing[1] + spacing[2] - max)/2.0;

  // downsample the other two dimensions until there average is as close to min
  // without going under min.  That is as long as we can multiply the larger
  // spacing by 2.
  while(nonmax * 2.0 <= max)
    {
    nonmax *= 2.0;
    ninplanelevels++;
    }

  SizeType afterinplanesize = size;
  for(unsigned int s = 0; s < 3; ++s)
    {
    if(s != maxind)
      {
      afterinplanesize[s] /= 2;
      }
    }  
  
  SpacingValueType minsize = size[0];
  if(size[1] < minsize)
    minsize = size[1];
  if(size[2] < minsize)
    minsize = size[2];

  // Hard-coded constant is bad
  while((minsize / 2.0) >= 25)
    {
    minsize /= 2.0;
    nalllevels++;
    }

  const unsigned int nlevels = ninplanelevels + nalllevels+1;
  
  pyramid->SetNumberOfLevels(nlevels);
  itk::Array2D<unsigned int> schedule = pyramid->GetSchedule();
  for(int i = ninplanelevels-1; i >= 0; --i)
    {
    for(unsigned int j = 0; j < 3; ++j)
      {
      if(j == maxind)
        {
        schedule(i,j) = schedule(i+1,j);
        }     
      }
    }

  pyramid->SetSchedule(schedule);
}

#endif
