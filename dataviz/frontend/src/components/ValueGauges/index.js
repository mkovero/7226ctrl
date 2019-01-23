import React, { Component } from 'react';

export default class ValueGauges extends Component {
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
    );
  }
}
