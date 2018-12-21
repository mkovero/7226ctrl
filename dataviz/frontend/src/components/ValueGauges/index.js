import React, { Component } from 'react';

export default class ValueGauges extends Component {
  constructor(props) {
    super(props);
    this.state = {
      vehicleSpeed: 0,
      rpm: 0,
      tps: 0,
      gear: 0,
      oilTemp: 0,
      ATFTemp: 0,
      load: 0,
      boost: 0,
      boostLimit: 0,
      lastSPC: 0,
      modVal: 0,
      looptime: 0,
    }
  }

  render() {
    
    return (
        <div className="Stats">
          <div>Vehicle speed: {this.props.data[0]}</div>
          <div>RPM: {this.props.data[1]}</div>
          <div>TPS: {this.props.data[2]}</div>
          <div>Gear: {this.props.data[3]}</div>
          <div>Oil Temp: {this.props.data[4]}</div>
          <div>ATF Temp: {this.props.data[5]}</div>
          <div>Load: {this.props.data[6]}</div>
          <div>Boost Sensor: {this.props.data[7]}</div>
          <div>Boost Limit: {this.props.data[8]}</div>
          <div>Last SPC: {this.props.data[9]}</div>
          <div>ModVal: {this.props.data[10]}</div>
          <div>Looptime: {this.props.data[11]}</div>
        </div>
    );
  }
}
