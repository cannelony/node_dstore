var dstore = require('../build/Release/dstore');
var fs = require('fs');

// read file
try {
  // var data = fs.readFileSync('triple/dbpedia.16.json', 'ascii');
  //console.log(data);
} catch (err) {
  console.error("There was an error opening the file:");
  console.log(err);
}

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
//var msg = dstore.insert(rdfJson);
// var msg = dstore.insert(JSON.parse(data));
//var then = new Date().getTime();
//console.log('Import took ' + (then - now) + ' ms!');
//console.log(msg);

//// test dstore.hasSubject
var benchmarkNow = process.hrtime();
var hasSubject = dstore.hasSubject("http://dbpedia.org/resource/%C3%87ukurova,_Adana", function(err, status) {
  var benchmarkThen = process.hrtime();
	console.log('HasSubject took ' + (benchmarkThen[1] - benchmarkNow[1])/1000000 + ' ms!');
	console.log("\nhttp://dbpedia.org/resource/%C3%87ukurova,_Adana - callback - hasSubject should be true: " + status + '\n');
});
//
console.log("\nhttp://dbpedia.org/resource/%C3%87ukurova,_Adana - hasSubject should be true: " + hasSubject + '\n');
//
//var hasSubject = dstore.hasSubject("http://foobar.boo/about");
//console.log("Should be true: " + hasSubject);

// test find
console.log('data.json@50%');
var benchmarkNow = process.hrtime();
var find = dstore.find("http://dbpedia.org/resource/%C3%87ukurova,_Adana", function(err, result) {
  var benchmarkThen = process.hrtime();
  console.log('Query took ' + (benchmarkThen[1] - benchmarkNow[1])/1000000 + ' ms!');
  console.log('ENTER FIND CALLBACK');
  console.log(result);
});

//console.log('return value of find: ' + find);

console.log('\ndata.json@99%');
var benchmarkNow = process.hrtime();
var find = dstore.find("http://dbpedia.org/resource/%C4%8Cetverored", function(err, result) {
  var benchmarkThen = process.hrtime();
  console.log('Query took ' + (benchmarkThen[1] - benchmarkNow[1])/1000000 + ' ms!');
  console.log('ENTER FIND CALLBACK');
  console.log(result);
});
	
console.log('\ndata.json@0%');
var benchmarkNow = process.hrtime();
var find = dstore.find("http://dbpedia.org/ontology/", function(err, result) {
  var benchmarkThen = process.hrtime();
  console.log('Query took ' + (benchmarkThen[1] - benchmarkNow[1])/1000000 + ' ms!');
  console.log('ENTER FIND CALLBACK');
  console.log(result);
});

console.log('\nrdfJSON');
var benchmarkNow = process.hrtime();
var find = dstore.find("http://example.org/about", function(err, result) {
  var benchmarkThen = process.hrtime();
  console.log('Query took ' + (benchmarkThen[1] - benchmarkNow[1])/1000000 + ' ms!');
  console.log('ENTER FIND CALLBACK');
  console.log(result);
//console.log('\nobjects for property http://purl.org/dc/elements/1.1/title\n');
//console.log(result['http://example.org/about']['http://purl.org/dc/elements/1.1/title'])
//console.log('\naccess the first object and return the value')
//console.log(result['http://example.org/about']['http://purl.org/dc/elements/1.1/title'][0].value);
//console.log('\nDONE FIND CALLBACK');
});

console.log('\naksw.json');
var benchmarkNow = process.hrtime();
var find = dstore.find("http://aksw.org/PhilippFrischmuth", function(err, result) {
  var benchmarkThen = process.hrtime();
  console.log('Query took ' + (benchmarkThen[1] - benchmarkNow[1])/1000000 + ' ms!');
  console.log('ENTER FIND CALLBACK');
  console.log(result);
});

	
//var msg = dstore.insert(rdfJson2)
