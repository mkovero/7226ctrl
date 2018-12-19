const SerialPort = require('serialport');
const WebSocket = require('ws');


const wss = new WebSocket.Server({ port: 8080 });
var ws;

var serial = new SerialPort('/dev/tty.usbmodem14201', {
  baudRate: 115200
});


wss.on('connection', function connection(ws_) {
  console.log('New connection!');
  ws_.on('message', function incoming(message) {
    console.log('received: %s', message);
  });
  ws = ws_;
});
  
serial.open(() => {
  console.log("Opening serial")
  let buffer = "";
  serial.on('data', (data) => {
    buffer += String(data);
    while (buffer.match(/\r\n/)) {
      match = buffer.match(/\r\n/);
      let values = buffer.slice(0, match.index).split(';').map(val => parseFloat(val));
      buffer = buffer.slice(match.index + 2);
      console.log(values);
      if (ws && ws.readyState === 1 && values.length === 12) {
        ws.send(JSON.stringify(values));
      }
    }
  });
});