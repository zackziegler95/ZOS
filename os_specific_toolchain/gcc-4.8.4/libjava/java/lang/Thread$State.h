
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_lang_Thread$State__
#define __java_lang_Thread$State__

#pragma interface

#include <java/lang/Enum.h>
#include <gcj/array.h>


class java::lang::Thread$State : public ::java::lang::Enum
{

  Thread$State(::java::lang::String *, jint);
public:
  static JArray< ::java::lang::Thread$State * > * values();
  static ::java::lang::Thread$State * valueOf(::java::lang::String *);
  static ::java::lang::Thread$State * BLOCKED;
  static ::java::lang::Thread$State * NEW;
  static ::java::lang::Thread$State * RUNNABLE;
  static ::java::lang::Thread$State * TERMINATED;
  static ::java::lang::Thread$State * TIMED_WAITING;
  static ::java::lang::Thread$State * WAITING;
private:
  static JArray< ::java::lang::Thread$State * > * ENUM$VALUES;
public:
  static ::java::lang::Class class$;
};

#endif // __java_lang_Thread$State__
