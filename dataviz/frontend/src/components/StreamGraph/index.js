import React, { Component } from 'react';
import { select, selectAll } from 'd3-selection';
import { scaleLinear } from 'd3-scale';
import { stack, stackOffsetSilhouette, area, curveNatural } from 'd3-shape';
import { range, max, min, transpose } from 'd3-array';
import './styles.css';

// nopeus;rpm;tps;vaihde;oiltemp;atftemp;load;boost;express;boostlim;pressdif;n2;n3;evalgear;ratio;slip;battery 
var n = 9, // number of layers
    m = 150; // number of samples per layer
const data = new Array(n).fill(null).map(() => new Array(m).fill(0));

var y;

const colors = [
    "rgb(192,174,0)",
    "rgb(192,155,0)",
    "rgb(192,135,0)",
    "rgb(192,0,0)",
    "rgb(255,0,0)",
    "rgb(192,0,0)",
    "rgb(192,135,0)",
    "rgb(192,155,0)",
    "rgb(192,174,0)",
]

function streamGraph() {

    var stack_ = stack().keys(range(n)).offset(stackOffsetSilhouette),
        layers = stack_(transpose(data));

    var svg = select("#streamChart");

    const   width = +svg.attr("width"),
            height = +svg.attr("height");

    var x = scaleLinear()
        .domain([0, m - 1])
        .range([0, width-1]);

    y = scaleLinear()
        //.domain([min(layers, stackMin), max(layers, stackMax)])
        .domain([-7000, 7000])
        .range([height, 0])

    var area_ = area()
        .x(function(d, i) { return x(i); })
        .y0(function(d) { return y(d[0]); })
        .y1(function(d) { return y(d[1]); });

    svg.selectAll(".streamPath")
        .data(layers)
        .enter().append("path")
            .attr("class", "streamPath")
            .attr("d", area_)
            .attr("fill", (d, i) => colors[i]);

    function stackMax(layer) {
        return max(layer, function(d) { return d[1]; });
    }

    function stackMin(layer) {
        return min(layer, function(d) { return d[0]; });
    }

    function transition(newData) {
        if (newData) {
            newData.forEach((value, idx) => {
                data[idx].push(value);
                data[idx].shift();
            });
        }

        let d = stack_(transpose(data));

        var area_ = area()
            .x(function(d, i) { return x(i); })
            .y0(function(d) { return y(d[0]); })
            .y1(function(d) { return y(d[1]); })
            .curve(curveNatural);

        selectAll(".streamPath")
            .data(d)
            .attr("d", area_);
}
    return transition;
}

function scaleData(line) {
    let scaleFactor = 1500;
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

export default class StreamGraph extends Component {
  constructor(props) {
    super(props);
    this.state = {
      callback: () => 0,
    }
  }

  componentDidMount() {
    var cb = streamGraph();
    this.setState({callback: cb}, () => this.forceUpdate());
  }

  componentWillReceiveProps({ data }) {
    // duplicate array and scale values
    let dataCopy = Array(data.length); 
    let i = data.length;
    while(i--) dataCopy[i] = data[i];
    scaleData(dataCopy)
    this.state.callback(dataCopy);
  }

  shouldComponentUpdate() {
    return false;
  }
  
  render() {
    
    return (
      <div>
        <svg id="streamChart" width="1480" height="1480"></svg>
      </div>
    );
  }
}
