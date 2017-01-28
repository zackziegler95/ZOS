
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_awt_image_RGBImageFilter__
#define __java_awt_image_RGBImageFilter__

#pragma interface

#include <java/awt/image/ImageFilter.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace awt
    {
      namespace image
      {
          class ColorModel;
          class IndexColorModel;
          class RGBImageFilter;
      }
    }
  }
}

class java::awt::image::RGBImageFilter : public ::java::awt::image::ImageFilter
{

public:
  RGBImageFilter();
  virtual void setColorModel(::java::awt::image::ColorModel *);
  virtual void substituteColorModel(::java::awt::image::ColorModel *, ::java::awt::image::ColorModel *);
  virtual ::java::awt::image::IndexColorModel * filterIndexColorModel(::java::awt::image::IndexColorModel *);
  virtual void filterRGBPixels(jint, jint, jint, jint, JArray< jint > *, jint, jint);
  virtual void setPixels(jint, jint, jint, jint, ::java::awt::image::ColorModel *, JArray< jbyte > *, jint, jint);
  virtual void setPixels(jint, jint, jint, jint, ::java::awt::image::ColorModel *, JArray< jint > *, jint, jint);
  virtual jint filterRGB(jint, jint, jint) = 0;
public: // actually protected
  ::java::awt::image::ColorModel * __attribute__((aligned(__alignof__( ::java::awt::image::ImageFilter)))) origmodel;
  ::java::awt::image::ColorModel * newmodel;
  jboolean canFilterIndexColorModel;
public:
  static ::java::lang::Class class$;
};

#endif // __java_awt_image_RGBImageFilter__