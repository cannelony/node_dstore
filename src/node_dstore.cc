#include <node/v8.h>
#include <node/node.h>

#include "DStore.h"
#include <cmath>
#include <string>

////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> Insert(const v8::Arguments& args)
{
  v8::HandleScope scope;
  if (args[0]->IsObject()) {
    return scope.Close(v8::String::New("is an object"));
  }
  return v8::Undefined();
}

////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> InsertFile(const v8::Arguments& args)
{
  v8::HandleScope scope;
  return v8::Undefined();
}

////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> InsertMultiple(const v8::Arguments& args)
{
  v8::HandleScope scope;
  return v8::Undefined();
}

////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> Find(const v8::Arguments& args)
{
  v8::HandleScope scope;
  return v8::Undefined();
}

////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> HasSubject(const v8::Arguments& args)
{
  // init dstore
  DStore dstore;
  v8::HandleScope scope;
  // v8::String holds first argument
  v8::String::Utf8Value subject_(args[0]->ToString());
  // due to no ToFunction(), use a Cast instead
  v8::Local<v8::Function> callback_ = v8::Local<v8::Function>::Cast(args[1]);
  // run equivalent to DStore method and convert result to v8 type
  bool dsHasSubject = dstore.hasSubject(std::string(*subject_));
  v8::Handle<v8::Boolean> hasSubject_ = v8::Boolean::New(dsHasSubject);
  // second argument has to be a function
  if (!args[1]->IsFunction()) {
    // throw exception if no callback was set
    // return v8::ThrowException(v8::Exception::TypeError(
                                // v8::String::New("Second argument must be a callback function")));
  } else {
    // prepare error handling
    v8::Local<v8::Value> err = v8::Exception::Error(v8::String::New("Error in callback!"));
    // prepare parameter of callback
    const unsigned argc = 1;
    v8::Local<v8::Value> argv[argc] = { err };
    // call callback
    callback_->Call(v8::Context::GetCurrent()->Global(), argc, argv);
  }
  return scope.Close(hasSubject_);
}

////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> Testfunction(const v8::Arguments& args)
{
  v8::HandleScope scope;
  int num = 133.7f / M_PI;
  v8::Local<v8::Integer> number = v8::Integer::New(num);
  return scope.Close(number);
}

////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> Helloworld(const v8::Arguments& args)
{
  v8::HandleScope scope;
  v8::Local<v8::String> string = v8::String::New("Hello World!");
  return scope.Close(string);
}

////////////////////////////////////////////////////////////////////

void RegisterModule(v8::Handle<v8::Object> target)
{
  NODE_SET_METHOD(target, "testfunction", Testfunction);
  NODE_SET_METHOD(target, "helloworld", Helloworld);
  NODE_SET_METHOD(target, "insert", Insert);
  NODE_SET_METHOD(target, "hasSubject", HasSubject);
}

NODE_MODULE(dstore, RegisterModule);
