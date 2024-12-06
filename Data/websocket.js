
var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
var first = true;

connection.onopen = function () {
  first = true;
  // connection.send('connected'); // Get value
};

connection.onerror = function (error) {
  console.log('WebSocket Error ', error);
};

connection.onclose = function () {
  console.log('WebSocket connection closed');
};

// Function that receives the message from the ESP32 with the readings
connection.onmessage = function (evt) {
  console.log('Server: ', evt.data);
  var myObj = JSON.parse(evt.data);
  var keys = Object.keys(myObj);
  document.getElementById('toggleClock').checked = myObj['toggleClock'];
  document.getElementById('Utc').value = parseInt(myObj['Utc']);
  document.getElementById('toggleText').checked = myObj['toggleText'];
  document.getElementById('text').innerHTML = myObj['text'];
  document.getElementById('textDot').innerHTML = myObj['textDot'];
};

// Send value to Json format
function sendValue () {
  first = false;
  var valstr = '{';
  var tgc = document.getElementById('toggleClock').checked;
  var utc = document.getElementById('Utc').value;
  valstr = valstr + '\"toggleClock\":' + tgc.toString() +',\"Utc\":\"' + utc.toString() + '\",';
  var tgt = document.getElementById('toggleText').checked;
  var txt = document.getElementById('text').value;
  valstr = valstr + '\"toggleText\":' + tgt.toString() +',\"text\":\"' + txt.toString() + '\"'; 
  valstr += '}';  
  console.log('sendValue: ' + valstr);
  connection.send(valstr);
}
