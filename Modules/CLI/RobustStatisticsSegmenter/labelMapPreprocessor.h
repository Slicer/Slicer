#ifndef LABELMAPPREPROCESSOR_H
#define LABELMAPPREPROCESSOR_H

#include "itkImage.h"
#include "itkImageRegionIterator.h"

#include <algorithm>

template <typename pixel_t>
typename itk::Image<pixel_t, 3>::Pointer
preprocessLabelMap(typename itk::Image<pixel_t, 3>::Pointer originalLabelMap, pixel_t desiredLabel)
{
  /*
    If there is a single non-zero label in the originalLabelMap, then
    use that as the label map to feed the segmentor.

    If originalLabelMap contains multiple labels, extract
    desiredLabel, forms a new label map containing only desiredLabel
    to feed the segmentor.

    If originalLabelMap contains multiple labels, but do NOT contain
    desiredLabel, then use all the non-zero labels as a single label
    and output label value is desiredLabel.

    1. count number of different labels in the originalLabelMap

    2. if #= 1, return it

    3. if #= 2, if desiredLabel is not there, then return originalLableMap itself

    4. if #= 2, go thru originalLabelMap and fill in a new label map
       with 1 where the label matches.


   */

  typedef itk::Image<pixel_t, 3> image_t;

  typedef itk::ImageRegionIterator<image_t> imageRegionIterator_t;

  // 1.
  imageRegionIterator_t iter(originalLabelMap, originalLabelMap->GetLargestPossibleRegion() );
  iter.GoToBegin();

  typename image_t::SizeType sz = originalLabelMap->GetLargestPossibleRegion().GetSize();

  std::vector<pixel_t> uniqueLabels(sz[0] * sz[1] * sz[2]);
  long                 i = 0;
  for( ; !iter.IsAtEnd(); ++iter )
    {
    uniqueLabels[i++] = iter.Get();
    }

  std::sort(uniqueLabels.begin(), uniqueLabels.end() );
  typename std::vector<pixel_t>::iterator itl = std::unique(uniqueLabels.begin(), uniqueLabels.end() );
  uniqueLabels.resize( itl - uniqueLabels.begin() );

  if( uniqueLabels[0] != 0 )
    {
    std::cerr << "Error: least label is not 0? no background?\n";
    raise(SIGABRT);
    }

  short numOfLabels = uniqueLabels.size() - 1; // 0 not count

  // 2.
  if( 1 == numOfLabels )
    {
    return originalLabelMap;
    }

  // 3.
  if( !std::binary_search(uniqueLabels.begin(), uniqueLabels.end(), desiredLabel) )
    {
    return originalLabelMap;
    }

  // 4.
  typename image_t::Pointer newLabelMap = image_t::New();
  newLabelMap->CopyInformation(originalLabelMap);
  newLabelMap->SetRegions( originalLabelMap->GetLargestPossibleRegion() );
  newLabelMap->Allocate();
  newLabelMap->FillBuffer(0);

  imageRegionIterator_t iterNew(newLabelMap, newLabelMap->GetLargestPossibleRegion() );
  iterNew.GoToBegin();
  iter.GoToBegin();
  for( ; !iter.IsAtEnd(); ++iter, ++iterNew )
    {
    if( iter.Get() == desiredLabel )
      {
      iterNew.Set(1);
      }
    }

  return newLabelMap;
}

#endif
