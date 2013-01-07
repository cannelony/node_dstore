var dstore = require('../build/Release/dstore');
var fs = require('fs');

// read file
try {
  var data = fs.readFileSync('data2.json', 'ascii');
  console.log('Input: ', data);
} catch (err) {
  console.error("There was an error while opening the file:");
  console.log(err);
}


// insert the read data (turtle)
var msg = dstore.insert(JSON.parse(data));
console.log('Successfully insert: ', msg);

// hasSubject
var start = process.hrtime();
var hasSubject = dstore.hasSubject("http://example.org/about", function(err, status) {
  var stop = process.hrtime();
	console.log('HasSubject took ' + (stop[1] - start[1])/1000000 + ' ms!');
	console.log("\nhttp://example.org/about is already inserted:  " + status + '\n');
});

// find
var start = process.hrtime();
var find = dstore.find("http://example.org/about", function(err, result) {
  var stop = process.hrtime();
  console.log('Enter callback \"find\"');
  console.log('Query took ' + (stop[1] - start[1])/1000000 + ' ms!');
  console.log('Output: ', result);
});

