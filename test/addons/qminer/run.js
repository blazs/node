var qm = require('./build/Release/qminer.node');

console.log("Creating new TVec vector");

var v = new qm.TVec();
v.push(3.2);
console.log(v.length);
v.push(2.3);
console.log(v.length);

function assert(cond, msg) {
   if (!(cond)) { throw "Assert failed"; }
}

function testVec(n) {
   var v = new qm.TVec();
   var curr = 1, prev = 2;
   
   // sum of terms of the sequence 
   var sum = 1;
   
   v.push(0); v.push(1);
   for (var i = 0; i < n; ++i) {
      var tmp = curr+prev;
      v.push(tmp);
      prev = curr;
      curr = tmp;
      sum += prev;
   }
   return sum;
}

for (var i = 0; i < v.length; ++i) {
   console.log("v[" + (i+1) + "]=" + v.at(i));
}

/*
var v = new qm.TVec();
for (var i = 0; i < 1000000; ++i) {
   v.push(testVec(40));
   v.push(testVec(40));
}
console.log(v.length());
*/

var v = new qm.TVec();
for (var i = 0; i < 10; ++i) {
   v.push(7*i % 10);
}

console.log("Contents:");
for (var i = 0; i < v.length; ++i) {
   console.log(v.at(i));
}

v.put(0, 100);
var w = v.sort(true);

console.log("Contents:");
for (var i = 0; i < v.length; ++i) {
   console.log(v.at(i));
}

console.log("len="+v.length);
console.log("sum = "+v.sum());
console.log("index of max el: "+v.getMaxIdx());
console.log("max = "+v.at(v.getMaxIdx()));

/*
 TODO (jan):
   (*) TJsVec: TInt in TFlt 
   (*) qm.newTVec javascript wrap
   (*) V8: what's new 
   (*) TJsRecCMp::operator(): a se da poenostavit s tanovim APIjem?
   (*) google test 
   (*) spMat.js, TJsSPMat: New() dobi pot do prototipa iz spMat.js; a se da poenostavit? 
   (*) Kako vracat nov objekt v Javascript 
*/

