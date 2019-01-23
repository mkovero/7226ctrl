const SerialPort = require('serialport');
const WebSocket = require('ws');


const wss = new WebSocket.Server({ port: 8080 });
var ws;

var serial = new SerialPort('/dev/ttyACM0', {
  baudRate: 115200
});


wss.on('connection', function connection(ws_) {
  console.log('New connection!');
  ws_.on('message', function incoming(message) {
    console.log('received: %s', message);
  });
  ws = ws_;
});

function stripData(line) {
    let newLine = []
    let indexArray = [0, 1, 2, 4, 5, 6, 7, 8, 16]
    indexArray.forEach(idx => newLine.push(line[idx]))
    return newLine
}

function scaleData(line) {
  let scaleFactor = 3000;
  line[0] = (line[0]/300) * scaleFactor
  line[1] = (line[1]/10/650) * scaleFactor
  line[2] = (line[2]/100) * scaleFactor
  line[3] = (line[3]/255) * scaleFactor
  line[4] = (line[4]/255) * scaleFactor 
  line[5] = (line[5]/100) * scaleFactor
  line[6] = (line[6]/700) * scaleFactor 
  line[7] = (line[7]/700) * scaleFactor 
  line[8] = (line[8]/100000) * scaleFactor
}

serial.open(() => {
  console.log("Opening serial")
  let buffer = "";
  serial.on('data', (data) => {
    buffer += String(data);
    while (buffer.match(/\r\n/)) {
      match = buffer.match(/\r\n/);
      let values = buffer.slice(0, match.index).split(';').map(val => parseFloat(val));
      buffer = buffer.slice(match.index + 2);
      values = stripData(values)
      scaleData(values)
      console.log(values);
      if (ws && ws.readyState === 1 && values.length === 9) {
        ws.send(JSON.stringify(values));
      }
    }
  });
});