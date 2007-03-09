#ifndef _IMAGE_H
#define _IMAGE_H

#include "iostream"
//#include "OpenTracker/OpenTracker.h"

//namespace ot {

class VTK_REALTIMEIMAGING_EXPORT Image
{
public:
  Image();
  Image(int x,int y, int p,void* pixel_data);

  int size() const
  {
    return xsize*ysize*pixelsize;
  }

  void SetSize(int x,int y,int p);

  void *image_ptr;
private:
  int xsize;
  int ysize;
  int pixelsize;

  friend std::ostream& operator<<(std::ostream& os, const Image& object);
  friend std::istream& operator>>(std::istream& is, Image& object);

};

  std::ostream& operator<<(std::ostream& os, const Image& object);
  std::istream& operator>>(std::istream& is, Image& object);



//}
#endif
