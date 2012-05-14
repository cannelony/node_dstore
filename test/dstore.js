var dstore = require('../build/Release/dstore');

console.warn(dstore);
console.warn(dstore.testfunction());
console.warn(dstore.helloworld());

// test insert
var msg = dstore.insert({ 'test': true});
console.warn(msg);

// test dstore.hasSubject
var hasSubject = dstore.hasSubject("http://foo.bar/foo", function(err) {
    console.warn("run callback");
    console.warn(err);
});

console.warn(hasSubject);

var hasSubject = dstore.hasSubject("http://foo.bar/bar");
console.log(hasSubject);
