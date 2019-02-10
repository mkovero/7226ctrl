import React, { Component } from 'react';
import ReactSpeedometer from 'react-d3-speedometer';
import { scaleLinear } from 'd3-scale';
import './styles.css';
let BarChart = require('react-d3-components').BarChart;

export default class Meters extends Component {
  constructor(props) {
    super(props);
    this.state = {
      vehicleSpeed: 0,
      rpm: 0,
      tps: 0,
      oilTemp: 0,
      ATFTemp: 0,
      load: 0,
      boost: 0,
      express: 0,
      battery: 0,
    }
  }

  render() {
    
    return (
      <div>
        <table className="Stats">
          <tr><td>Vehicle speed:</td><td>{this.props.data[0]}</td></tr>
          <tr><td>RPM:</td><td>{this.props.data[1]}</td></tr>
          <tr><td>TPS:</td><td>{this.props.data[2]}</td></tr>
          <tr><td>Oil Temp:</td><td>{this.props.data[3]}</td></tr>
          <tr><td>ATF Temp:</td><td>{this.props.data[4]}</td></tr>
          <tr><td>Load:</td><td>{this.props.data[5]}</td></tr>
          <tr><td>Boost Sensor:</td><td>{this.props.data[6]}</td></tr>
          <tr><td>Exhaust temp:</td><td>{this.props.data[7]}</td></tr>
          <tr><td>Battery voltage:</td><td>{this.props.data[8]}</td></tr>
        </table>
        <div class="container">
          <div class="meter-row row">
            <ReactSpeedometer
              height={220}
              width={280}
              value={this.props.data[0]}
              minValue={0}
              maxValue={300}
              segments={15}
              needleColor="steelblue"
              needleTransitionDuration={4000}
              needleTransition="easeElastic"
              startColor="#33CC33"
              endColor="#FF471A"
              textColor="#e5e5e5"
              ringWidth={10}
              currentValueText="Speed"
            />
            <ReactSpeedometer
              height={220}
              width={280}
              value={this.props.data[1] / 1000}
              minValue={0}
              maxValue={7}
              segments={7}
              needleColor="steelblue"
              needleTransitionDuration={4000}
              needleTransition="easeElastic"
              startColor="#33CC33"
              endColor="#FF471A"
              textColor="#e5e5e5"
              ringWidth={10}
              currentValueText="KRPM"
            />
            <ReactSpeedometer
              height={220}
              width={280}
              value={this.props.data[6] / 100}
              minValue={0}
              maxValue={3}
              segments={6}
              needleColor="steelblue"
              needleTransitionDuration={4000}
              needleTransition="easeElastic"
              startColor="#33CC33"
              endColor="#FF471A"
              textColor="#e5e5e5"
              ringWidth={10}
              currentValueText="Boost"
            />
          </div>
          <BarChart
            yAxis={{tickArguments: [10]}}
            yScale={scaleLinear().domain([200, 0]).range([0, 400 - 70])}
            data={[{
              label: 'Temperatures',
              values: [
                {x: 'Oil', y: this.props.data[3]},
                {x: 'ATF', y: this.props.data[4]}
              ]
          }]}
            width={400}
            height={400}
            margin={{top: 10, bottom: 50, left: 50, right: 10}}
          />
        </div>
      </div>
    );
  }
}
