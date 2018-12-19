const SerialPort = require('serialport');
const WebSocket = require('ws');

const wss = new WebSocket.Server({ port: 8080 });
var ws;

wss.on('connection', function connection(ws_) {
  console.log('New connection!');
  ws_.on('message', function incoming(message) {
    console.log('received: %s', message);
  });
  ws = ws_;
});

let lines = require('fs').readFileSync('ajo.csv').toString().split('\n')

function stripData(line) {
  let newLine = []
  let indexArray = [0, 1, 2, 4, 5, 6, 7, 8, 16]
  indexArray.forEach(idx => newLine.push(line[idx]))
  return newLine
}

function scaleData(line) {
  let scaleFactor = 5000;
  line[0] = (line[0]/300) * scaleFactor
  line[1] = (line[1]/10/650) * scaleFactor
  line[2] = (line[2]/100) * scaleFactor
  line[3] = (line[3]/255) * scaleFactor
  line[4] = (line[4]/255) * scaleFactor 
  line[5] = (line[5]/100) * scaleFactor
  line[6] = (line[6]/700) * scaleFactor 
  line[7] = (line[7]/700) * scaleFactor 
  line[8] = (line[8]/100/160) * scaleFactor 
}

setInterval(() => {
  line = lines.shift().split(';').map(datapoint => parseFloat(datapoint))
  line = stripData(line)
  scaleData(line)
  console.log('Sending data:', line)
  if (ws && ws.readyState === 1) {
    ws.send(JSON.stringify(line))
  }
}, 200)
