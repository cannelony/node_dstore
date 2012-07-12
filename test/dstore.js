var dstore = require('../build/Release/dstore');

var rdfJson = {
  "http://example.org/about" : 
    {
    "http://purl.org/dc/elements/1.1/creator" : [ { "value" : "Anna Wilder", "type" : "literal" } ],
    "http://purl.org/dc/elements/1.1/title"   : [ { "value" : "Anna's Homepage", "type" : "literal", "lang" : "en", "datatype" : "xsd:string" }, { "value" : "Die Homepage von Anna", "type" : "literal", "lang" : "en" } ] ,
    "http://xmlns.com/foaf/0.1/maker"         : [ { "value" : "_:person", "type" : "bnode" } ]
    },
  "http://example.org/about2" : 
    {
    "http://purl.org/dc/elements/1.1/creator" : [ { "value" : "Anna Wilder", "type" : "literal" } ],
    "http://purl.org/dc/elements/1.1/title"   : [ { "value" : "Anna's Homepage", "type" : "literal", "lang" : "en" } ] ,
    "http://xmlns.com/foaf/0.1/maker"         : [ { "value" : "_:person", "type" : "bnode" } ]
    },
  "http://example.org/about3" : 
    {
    "http://purl.org/dc/elements/1.1/creator" : [ { "value" : "Anna Wilder", "type" : "literal" } ],
    "http://purl.org/dc/elements/1.1/title"   : [ { "value" : "Anna's Homepage", "type" : "literal", "lang" : "en" } ] ,
    "http://xmlns.com/foaf/0.1/maker"         : [ { "value" : "_:person", "type" : "bnode" } ]
    }
};

var rdfJson2 = {
    "http://foobar.boo/about" : 
      {
      "http://purl.org/dc/elements/1.1/creator" : [ { "value" : "Anna Wilder", "type" : "literal" } ],
      "http://purl.org/dc/elements/1.1/title"   : [ { "value" : "Anna's Homepage", "type" : "literal", "lang" : "en", "datatype" : "xsd:string" }, { "value" : "Die Homepage von Anna", "type" : "literal", "lang" : "en" } ] ,
      "http://xmlns.com/foaf/0.1/maker"         : [ { "value" : "_:person", "type" : "bnode" } ]
      },
    "http://foobar.boo/about2" : 
      {
      "http://purl.org/dc/elements/1.1/creator" : [ { "value" : "Anna Wilder", "type" : "literal" } ],
      "http://purl.org/dc/elements/1.1/title"   : [ { "value" : "Anna's Homepage", "type" : "literal", "lang" : "en" } ] ,
      "http://xmlns.com/foaf/0.1/maker"         : [ { "value" : "_:person", "type" : "bnode" } ]
      },
    "http://foobar.boo/about3" : 
      {
      "http://purl.org/dc/elements/1.1/creator" : [ { "value" : "Anna Wilder", "type" : "literal" } ],
      "http://purl.org/dc/elements/1.1/title"   : [ { "value" : "Anna's Homepage", "type" : "literal", "lang" : "en" } ] ,
      "http://xmlns.com/foaf/0.1/maker"         : [ { "value" : "_:person", "type" : "bnode" } ]
      }
  };

// test insert
var msg = dstore.insert(rdfJson);
//console.log(msg);

//// test dstore.hasSubject
var hasSubject = dstore.hasSubject("http://example.org/about", function(err, status) {
	console.log("\nhttp://example.org/about - callback - hasSubject should be true: " + status + '\n');
});
//
console.log("\nhttp://example.org/about - hasSubject should be true: " + hasSubject + '\n');
//
//var hasSubject = dstore.hasSubject("http://foobar.boo/about");
//console.log("Should be true: " + hasSubject);

// test find
var find = dstore.find("http://example.org/about", function(err, result) {
  console.log('ENTER FIND CALLBACK');
  console.log(result);
  console.log('\nobjects for property http://purl.org/dc/elements/1.1/title\n');
  console.log(result['http://example.org/about']['http://purl.org/dc/elements/1.1/title'])
  console.log('\naccess the first object and return the value')
  console.log(result['http://example.org/about']['http://purl.org/dc/elements/1.1/title'][0].value);
  console.log('\nDONE FIND CALLBACK');
});

console.log('return value of find' + find);

//var msg = dstore.insert(rdfJson2)