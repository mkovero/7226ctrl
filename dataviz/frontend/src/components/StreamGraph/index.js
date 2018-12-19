import React, { Component } from 'react';
import { select, selectAll } from 'd3-selection';
import { scaleLinear } from 'd3-scale';
import { interpolateCool } from 'd3-scale-chromatic';
import { stack, stackOffsetSilhouette, area, curveNatural } from 'd3-shape';
import { range, max, min, transpose } from 'd3-array';
import { transition } from 'd3-transition';

// nopeus;rpm;tps;vaihde;oiltemp;atftemp;load;boost;express;boostlim;pressdif;n2;n3;evalgear;ratio;slip;battery 
var n = 9, // number of layers
    m = 150; // number of samples per layer
const data = new Array(n).fill(null).map(() => new Array(m).fill(0));

var y;

function streamGraph() {

    var stack_ = stack().keys(range(n)).offset(stackOffsetSilhouette),
        layers = stack_(transpose(data));

    var svg = select("svg");

    const   width = +svg.attr("width"),
            height = +svg.attr("height");

    var x = scaleLinear()
        .domain([0, m - 1])
        .range([0, width-1]);

    y = scaleLinear()
        //.domain([min(layers, stackMin), max(layers, stackMax)])
        .domain([-7000, 7000])
        .range([height, 0])

    var z = interpolateCool;

    var area_ = area()
        .x(function(d, i) { return x(i); })
        .y0(function(d) { return y(d[0]); })
        .y1(function(d) { return y(d[1]); });

    svg.selectAll("path")
        .data(layers)
        .enter().append("path")
            .attr("d", area_)
            .attr("fill", function() { return z(Math.random()); });

    function stackMax(layer) {
        return max(layer, function(d) { return d[1]; });
    }

    function stackMin(layer) {
        return min(layer, function(d) { return d[0]; });
    }

    function transition(newData) {
        //console.log("Transition called");
        if (newData) {
            newData.forEach((value, idx) => {
                //console.log(typeof value);
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

        selectAll("path")
            .data(d)
            .attr("d", area_);
}
    return transition;
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
    this.state.callback(data);
  }

  shouldComponentUpdate() {
    return false;
  }
  
  render() {
    
    return (
      <div>
        <button onClick={() => console.log(data)}>Print data</button>
        <svg id="chart" width="720" height="1280"></svg>
      </div>
    );
  }
}
