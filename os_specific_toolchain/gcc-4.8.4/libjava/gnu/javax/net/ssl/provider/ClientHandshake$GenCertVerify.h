
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_net_ssl_provider_ClientHandshake$GenCertVerify__
#define __gnu_javax_net_ssl_provider_ClientHandshake$GenCertVerify__

#pragma interface

#include <gnu/javax/net/ssl/provider/DelegatedTask.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace net
      {
        namespace ssl
        {
          namespace provider
          {
              class ClientHandshake;
              class ClientHandshake$GenCertVerify;
          }
        }
      }
    }
  }
  namespace java
  {
    namespace security
    {
        class MessageDigest;
    }
  }
}

class gnu::javax::net::ssl::provider::ClientHandshake$GenCertVerify : public ::gnu::javax::net::ssl::provider::DelegatedTask
{

public: // actually package-private
  ClientHandshake$GenCertVerify(::gnu::javax::net::ssl::provider::ClientHandshake *, ::java::security::MessageDigest *, ::java::security::MessageDigest *);
public:
  virtual void implRun();
public: // actually package-private
  virtual JArray< jbyte > * signed$();
private:
  ::java::security::MessageDigest * __attribute__((aligned(__alignof__( ::gnu::javax::net::ssl::provider::DelegatedTask)))) md5;
  ::java::security::MessageDigest * sha;
  JArray< jbyte > * signed$__;
public: // actually package-private
  ::gnu::javax::net::ssl::provider::ClientHandshake * this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_net_ssl_provider_ClientHandshake$GenCertVerify__