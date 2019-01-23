import React, { Component } from 'react';
import ValueGauges from './components/ValueGauges';
import StreamGraph from './components/StreamGraph';
import SwipeableViews from 'react-swipeable-views';
import './App.css';

class App extends Component {
    state = {
        currData: new Array(12).fill(0)
    }

  componentDidMount() {
    var socket = new WebSocket("ws://localhost:8080");
    socket.onmessage = (event) => {
        let newData = JSON.parse(event.data);
        this.setState({currData: newData})
      }
  }
  
  render() {
    return (
      <div className="App">
        <SwipeableViews>
          <ValueGauges data={this.state.currData} />
          <StreamGraph data={this.state.currData} />
        </SwipeableViews>
      </div>
    );
  }
}

export default App;
