#include "Image.h"

//namespace ot {

  Image::Image(){
    this->SetSize(256,256,2);
  };


  Image::Image(int x,int y, int p,void* pixel_data)
  {
    xsize=x;
    pixelsize=p;
    ysize=y;
    image_ptr=(void*)malloc(x*y*p);
    //void* tmp=(void*)calloc(x*y*p,1);
    //memcpy(image_ptr,tmp,x*y*p);
    memcpy(image_ptr,pixel_data,x*y*p);
  }


  void Image::SetSize(int x,int y,int p)
  {
    xsize=x;
    ysize=y;
    pixelsize=p;
  }

  std::ostream& operator<<(std::ostream& os, const Image& object)
  {

    os << "[" << object.xsize << ":" << object.ysize << ":" << object.pixelsize << ":";

    short *tmp_short;
    long *tmp_long;




    if(object.pixelsize==2)
    {
      tmp_short=(short*)malloc(object.size());
      memcpy(tmp_short,object.image_ptr,object.size());
      for(int i=0;i<object.xsize*object.ysize;i++)
        tmp_short[i]=htons(tmp_short[i]);
      os.write((char*)tmp_short,object.size());
      free(tmp_short);
    }

    if(object.pixelsize==4)
    {
      tmp_long=(long*)malloc(object.size());
      memcpy(tmp_long,object.image_ptr,object.size());
      for(int i=0;i<object.xsize*object.ysize;i++)
         tmp_long[i]=htonl(tmp_long[i]);
      os.write((char*)tmp_long,object.size());
      free(tmp_long);
    }

    os << "]";

    ///todo need to serialize the xdim, ydim and pixel size as well

    return os;
  };


  std::istream& operator>>(std::istream& is, Image& object)
  {
    char c;
    int size;

    if (!(is >> c) || c != '['
        || !(is >> object.xsize)
        || !(is >> c) || c != ':'
        || !(is >> object.ysize)
        || !(is >> c) || c != ':'
        || !(is >> object.pixelsize)
        || !(is >> c) || c != ':')
      {
        is.setstate(std::ios_base::failbit);
        return is;
      }

    size = object.xsize*object.ysize*object.pixelsize;
    object.image_ptr=(void*)malloc(size);
    is.read((char*)object.image_ptr,size);

    short *tmp_short;
    int *tmp_long;


    tmp_short=(short*)object.image_ptr;
    tmp_long=(int*)object.image_ptr;

    if(object.pixelsize==2)
      {
        for(int i=0;i<object.size()/2;i++)
          tmp_short[i]=ntohs(tmp_short[i]);
      }

    if(object.pixelsize==4)
      {
        for(int i=0;i<object.size()/2;i++)
        tmp_long[i]=ntohl(tmp_long[i]);
      }

    if (!(is >> c) || c != ']')
      {
        is.setstate(std::ios_base::failbit);
        return is;
      }



    return is;
  };

//}
