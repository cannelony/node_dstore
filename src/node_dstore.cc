#include <node/v8.h>
#include <node/node.h>

#include "DStore.h"
#include "Dictionary.h"
#include "Element.h";
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <cstddef>

#define TYPE "type"
#define VALUE "value"
#define LANG "lang"
#define DATATYPE "datatype"
#define FIRSTELEMENT 0
#define TRUE 1
#define FALSE 0

////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> Insert(const v8::Arguments& args)
{
  Dictionary dictionary("dict.bin");
  DStore dstore(dictionary);

  v8::HandleScope scope;
  if (args[0]->IsObject()) {
    v8::Handle<v8::Object> rdfJson = args[0]->ToObject();
    for (unsigned int i=0; i<rdfJson->GetPropertyNames()->Length(); i++) {
      Element dstoreSubject = dstore.createElement();
      const v8::Handle<v8::Value> key = rdfJson->GetPropertyNames()->Get(i);
      // convert to std::string
      v8::String::Utf8Value subject(key->ToString());
      std::string jsonSubject(*subject);
      // set subject to dstore element
      dstoreSubject.setSubject(jsonSubject);
      // iterate through properties
      v8::Handle<v8::Object> subjectObject = v8::Handle<v8::Object>::Cast(rdfJson->Get(key));
      // access property keys of http://example.org/about
      // ["http://purl.org/dc/elements/1.1/creator", "http://purl.org/dc/elements/1.1/title", "http://xmlns.com/foaf/0.1/maker"]
      v8::Handle<v8::Array> propertyKeys = v8::Handle<v8::Array>::Cast(subjectObject->GetPropertyNames());
      // access one property of http://example.org/about (http://purl.org/dc/elements/1.1/creator)
      // [ { "value" : "Anna Wilder", "type" : "literal" } ]
      for (int j=0; j<propertyKeys->Length(); j++) {
        // get property name
        const v8::Handle<v8::Value> propertyKey = subjectObject->GetPropertyNames()->Get(j);
        // convert to std::string
        v8::String::Utf8Value property(propertyKey->ToString());
        std::string jsonProperty(*property);
        // iterate through one property
        v8::Handle<v8::Array> propertyArray = v8::Handle<v8::Array>::Cast(subjectObject->Get(propertyKeys->Get(v8::Number::New(j))));
        // access one propertyValue of http://purl.org/dc/elements/1.1/creator
        for (int k=0; k<propertyArray->Length(); k++) {
          v8::Handle<v8::Object> propertyValueObject = v8::Handle<v8::Object>::Cast(propertyArray->Get(v8::Number::New(k)));
          // access value, type, lang
          v8::Handle<v8::String> propertyValueObjectValue = v8::Handle<v8::String>::Cast(propertyValueObject->Get(v8::String::New(VALUE)));
          v8::Handle<v8::String> propertyValueObjectType = v8::Handle<v8::String>::Cast(propertyValueObject->Get(v8::String::New(TYPE)));
          v8::Handle<v8::String> propertyValueObjectLang = v8::Handle<v8::String>::Cast(propertyValueObject->Get(v8::String::New(LANG)));
          v8::Handle<v8::String> propertyValueObjectDatatype = v8::Handle<v8::String>::Cast(propertyValueObject->Get(v8::String::New(DATATYPE)));

          // cast
          v8::String::Utf8Value propertyValue(propertyValueObjectValue->ToString());
          v8::String::Utf8Value propertyType(propertyValueObjectType->ToString());
          v8::String::Utf8Value propertyLang(propertyValueObjectLang->ToString());
          v8::String::Utf8Value propertyDatatype(propertyValueObjectDatatype->ToString());
          std::string jsonObjectValue(*propertyValue);
          std::string jsonObjectType(*propertyType);
          std::string jsonObjectLang = (*propertyLang);
          std::string jsonObjectDatatype(*propertyDatatype);
//          std::cout << "<" << jsonSubject << "> <" << jsonProperty << "> \"" << jsonObjectValue << "\" - Type: " << jsonObjectType << ", Lang: " << jsonObjectLang << ", Datatype: " << jsonObjectDatatype << std::endl;

          // add property to dstore element
          dstoreSubject.addProperty(jsonProperty, jsonObjectType, jsonObjectValue, jsonObjectLang, jsonObjectDatatype);
        }
      }
//      std::cout << "DSTORE element size " << dstoreSubject.getSize() << std::endl;
      dstore.insert(dstoreSubject);
    }

    // test
//    std::cout << "Dstore has subject http://example.org/about: " << dstore.hasSubject(dictionary.HasKey("http://example.org/about")) << std::endl;
//    std::cout << "Dstore has subject http://example.org/about4: " << dstore.hasSubject(dictionary.HasKey("http://example.org/about4")) << std::endl;

    return scope.Close(v8::Boolean::New(TRUE));
  } else {
    return scope.Close(v8::Boolean::New(FALSE));
  }
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
  Dictionary dictionary("dict.bin");
  // init dstore
  DStore dstore(dictionary);
  v8::HandleScope scope;
  // v8::String holds first argument
  v8::String::Utf8Value subject_(args[0]->ToString());
  // due to no ToFunction(), use a Cast instead
  v8::Local<v8::Function> callback_ = v8::Local<v8::Function>::Cast(args[1]);
  std::string subject = std::string(*subject_);
  DStore::SubjectID id = dictionary.HasKey(subject);
  // run equivalent to DStore method and convert result to v8 type
  Element dstoreElement = dstore.find(id);
//  std::cout << "FIND size of element " << dstoreElement.getSize() << std::endl;
  // second argument has to be a function
  if (!args[1]->IsFunction()) {
    // throw exception if no callback was set
    // return v8::ThrowException(v8::Exception::TypeError(
                                // v8::String::New("Second argument must be a callback function")));
  } else {
    // create json object
    v8::Handle<v8::Object> rdfJson = v8::Object::New();
    // create property object
    v8::Handle<v8::Object> rdfJsonPropertyObject = v8::Object::New();

    for (auto property : dstoreElement.getProperties()) {
      // create property array
      v8::Handle<v8::Array> rdfJsonObjects = v8::Array::New();

      // count property objects
      int propertyCount = 0;
      for (auto object : property.second) {
        // create property object which will be inserted into property array
        v8::Handle<v8::Object> rdfJsonObject = v8::Object::New();

        // convert DictIDs to std::string to v8::String
        std::string type;
        if (object.t == DStore::LITERAL) {
          type = "literal";
        } else if (object.t == DStore::URI) {
          type = "uri";
        } else if (object.t == DStore::BNODE) {
          type = "bnode";
        }

        std::string value(dictionary.Find(object.v));
        std::string lang(dictionary.Find(object.l));
        std::string datatype(dictionary.Find(object.d));

        // hack: empty value when undefined. should be fixed during inserting subject
        if (lang == "undefined") { lang = ""; };
        if (datatype == "undefined") { datatype = ""; };

        // output
//        std::cout << "TYPE " << type << std::endl;
//        std::cout << "VALUE " << value << std::endl;
//        std::cout << "LANGUAGE " << lang << std::endl;
//        std::cout << "DATATYPE " << datatype << std::endl;

        // write values for property object
        rdfJsonObject->Set(v8::String::New(TYPE), v8::String::New(type.c_str()));
        rdfJsonObject->Set(v8::String::New(VALUE), v8::String::New(value.c_str()));
        rdfJsonObject->Set(v8::String::New(LANG), v8::String::New(lang.c_str()));
        rdfJsonObject->Set(v8::String::New(DATATYPE), v8::String::New(datatype.c_str()));

        // insert into property array
        rdfJsonObjects->Set(v8::Number::New(propertyCount), rdfJsonObject);

        // increase counter, needed for inserting into property array above
        propertyCount++;
      }

      std::string propertyKey(dictionary.Find(property.first));
      rdfJsonPropertyObject->Set(v8::String::New(propertyKey.c_str()), rdfJsonObjects);
//      std::cout << "property key " << dictionary.Find(property.first) << std::endl;
    }

    rdfJson->Set(args[0]->ToString(), rdfJsonPropertyObject);

    v8::Local<v8::Value> result = v8::Local<v8::Value>::New(rdfJson);
    // prepare error handling
    v8::Local<v8::Value> err = v8::Exception::Error(v8::String::New("No Error!"));
    // prepare parameter of callback
    const unsigned argc = 2;
    v8::Local<v8::Value> argv[argc] = { err, result };
    // call callback
    callback_->Call(v8::Context::GetCurrent()->Global(), argc, argv);
    return scope.Close(v8::Boolean::New(TRUE));
  }

  return scope.Close(v8::Boolean::New(FALSE));
}

////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> HasSubject(const v8::Arguments& args)
{
  Dictionary dictionary("dict.bin");
  // init dstore
  DStore dstore(dictionary);
  v8::HandleScope scope;
  // v8::String holds first argument
  v8::String::Utf8Value subject_(args[0]->ToString());
  // due to no ToFunction(), use a Cast instead
  v8::Local<v8::Function> callback_ = v8::Local<v8::Function>::Cast(args[1]);
  std::string subject = std::string(*subject_);
  DStore::SubjectID id = dictionary.HasKey(subject);
  // run equivalent to DStore method and convert result to v8 type
  bool dsHasSubject = dstore.hasSubject(id);
  v8::Handle<v8::Boolean> hasSubject_ = v8::Boolean::New(dsHasSubject);
  // second argument has to be a function
  if (!args[1]->IsFunction()) {
    // throw exception if no callback was set
    // return v8::ThrowException(v8::Exception::TypeError(
                                // v8::String::New("Second argument must be a callback function")));
  } else {
    v8::Local<v8::Value> status = v8::Local<v8::Value>::New(hasSubject_);
    // prepare error handling
    v8::Local<v8::Value> err = v8::Exception::Error(v8::String::New("Error in callback!"));
    // prepare parameter of callback
    const unsigned argc = 2;
    v8::Local<v8::Value> argv[argc] = { err, status };
    // call callback
    callback_->Call(v8::Context::GetCurrent()->Global(), argc, argv);
  }
  return scope.Close(hasSubject_);
}

////////////////////////////////////////////////////////////////////

v8::Handle<v8::Value> ParseJson(v8::Handle<v8::Value> jsonString) {
  v8::HandleScope scope;

  v8::Handle<v8::Context> context = v8::Context::GetCurrent();
  v8::Handle<v8::Object> global = context->Global();

  v8::Handle<v8::Object> JSON = global->Get(v8::String::New("JSON"))->ToObject();
  v8::Handle<v8::Function> JSON_parse = v8::Handle<v8::Function>::Cast(JSON->Get(v8::String::New("parse")));

  // return JSON.parse.apply(JSON, jsonString);
  return scope.Close(JSON_parse->Call(JSON, 1, &jsonString));
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
  NODE_SET_METHOD(target, "helloworld", Helloworld);
  NODE_SET_METHOD(target, "insert", Insert);
  NODE_SET_METHOD(target, "insertFile", InsertFile);
  NODE_SET_METHOD(target, "insertMultiple", InsertMultiple);
  NODE_SET_METHOD(target, "find", Find);
  NODE_SET_METHOD(target, "hasSubject", HasSubject);
}

NODE_MODULE(dstore, RegisterModule);
