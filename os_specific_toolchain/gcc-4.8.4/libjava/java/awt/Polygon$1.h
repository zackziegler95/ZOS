
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_awt_Polygon$1__
#define __java_awt_Polygon$1__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class Polygon;
        class Polygon$1;
      namespace geom
      {
          class AffineTransform;
      }
    }
  }
}

class java::awt::Polygon$1 : public ::java::lang::Object
{

public: // actually package-private
  Polygon$1(::java::awt::Polygon *, ::java::awt::geom::AffineTransform *);
public:
  virtual jint getWindingRule();
  virtual jboolean isDone();
  virtual void next();
  virtual jint currentSegment(JArray< jfloat > *);
  virtual jint currentSegment(JArray< jdouble > *);
private:
  jint __attribute__((aligned(__alignof__( ::java::lang::Object)))) vertex;
public: // actually package-private
  ::java::awt::Polygon * this$0;
private:
  ::java::awt::geom::AffineTransform * val$transform;
public:
  static ::java::lang::Class class$;
};

#endif // __java_awt_Polygon$1__