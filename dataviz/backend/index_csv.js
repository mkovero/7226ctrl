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

setInterval(() => {
  line = lines.shift().split(';').map(datapoint => parseFloat(datapoint))
  line = stripData(line)
  console.log('Sending data:', line)
  if (ws && ws.readyState === 1) {
    ws.send(JSON.stringify(line))
  }
}, 200)
