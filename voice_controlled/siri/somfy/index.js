'use strict';

const net = require('net');
const crypto = require('crypto');
var request = require("request");

var Service, Characteristic;

module.exports = function (homebridge) {
  Service = homebridge.hap.Service;
  Characteristic = homebridge.hap.Characteristic;

  homebridge.registerAccessory('homebridge-real-fake-garage-doors', 'RealFakeGarageDoors', RealFakeGarageDoorsAccessory);
};

function RealFakeGarageDoorsAccessory (log, config) {
  this.log = log;

  this.host = config['host'];
  this.port = config['port'];
  this.key = config['key'];
  this.open_url = config['open_url'];
  this.close_url = config['close_url'];
  this.my_url = config['my_url'];
  this.method = config["http_method"];
  if (this.method === undefined) this.method = "GET";

  this.closingLock = false;
  this.openingLock = false;
  this.stoppedFlag = false;

  this.openCommand = config['openCommand'];
  this.closeAfter = config['closeAfter'];
  this.lastOpened = new Date();

  this.simulateTimeOpening = config['simulateTimeOpening'];
  this.simulateTimeOpen = config['simulateTimeOpen'];
  this.simulateTimeClosing = config['simulateTimeClosing'];

  this.service = new Service.GarageDoorOpener(config['name'], config['name']);
  this.setupGarageDoorOpenerService(this.service);

  this.informationService = new Service.AccessoryInformation();
  this.informationService
    .setCharacteristic(Characteristic.Manufacturer, 'm-ro based on Plumbus & Kimble Inc.')
    .setCharacteristic(Characteristic.Model, 'Somfy Remote')
    .setCharacteristic(Characteristic.SerialNumber, '24-12-2016');
}

RealFakeGarageDoorsAccessory.prototype = {
  getServices: function () {
    return [this.informationService, this.service];
  },

  setupGarageDoorOpenerService: function (service) {
    this.service.setCharacteristic(Characteristic.TargetDoorState, Characteristic.TargetDoorState.CLOSED);
    this.service.setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.CLOSED);

    service.getCharacteristic(Characteristic.TargetDoorState)
      .on('get', (callback) => {
        var tds = service.getCharacteristic(Characteristic.TargetDoorState).value;
        var cds = service.getCharacteristic(Characteristic.CurrentDoorState).value;
        //if ((tds === Characteristic.TargetDoorState.OPEN) && (cds === Characteristic.CurrentDoorState.OPEN)){
        if ((cds === Characteristic.CurrentDoorState.OPEN)){
           this.log('Setting TargetDoorState to CLOSED');
           callback(null, Characteristic.TargetDoorState.OPEN);
        //}else if ((tds === Characteristic.TargetDoorState.CLOSED) && (cds === Characteristic.CurrentDoorState.CLOSED)){
        }else if (cds === Characteristic.CurrentDoorState.CLOSED){
           this.log('Setting TargetDoorState to OPEN');
           callback(null, Characteristic.TargetDoorState.CLOSED);
        //}else if ((tds === Characteristic.TargetDoorState.CLOSED) && ((cds === Characteristic.CurrentDoorState.OPENING)||(cds === Characteristic.CurrentDoorState.CLOSING)||(cds === Characteristic.CurrentDoorState.STOPPED))){
        //   //this.service.setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.CLOSED);
        //   this.log('Setting TargetDoorState to CLSED');
        //   callback(null, Characteristic.TargetDoorState.CLOSED);
        //}else if ((tds === Characteristic.TargetDoorState.OPEN) && ((cds === Characteristic.CurrentDoorState.OPENING)||(cds === Characteristic.CurrentDoorState.CLOSING))){
        //   this.log('Setting TargetDoorState to CLSED');
        //   //this.service.setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.OPEN);
        //   callback(null, Characteristic.TargetDoorState.OPEN);
        }else{
             this.log('in null tds');
          callback(null, tds);
        }
//        if (tds === Characteristic.TargetDoorState.OPEN &&
//          (((new Date()) - this.lastOpened) >= (this.closeAfter * 1000))) {
//          this.log('Setting TargetDoorState to CLOSED');
//          callback(null, Characteristic.TargetDoorState.CLOSED);
//        } else {
//          callback(null, tds);
//        }
      })
      .on('set', (value, callback) => {
        if ((value === Characteristic.TargetDoorState.OPEN)) {
//        this.lastOpened = new Date();
          switch (service.getCharacteristic(Characteristic.CurrentDoorState).value) {
            case Characteristic.CurrentDoorState.CLOSING:
            case Characteristic.CurrentDoorState.OPENING:
//              this.lastStopped = new Date();
              this.stoppedFlag = true;
              this.stopDoor(callback);
              this.service.setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.STOPPED);
              break;
            case Characteristic.CurrentDoorState.STOPPED:
            case Characteristic.CurrentDoorState.CLOSED:
            case Characteristic.CurrentDoorState.OPEN:
              this.stoppedFlag = false;
              this.openDoor(callback);
              break;
            default:
              this.log('in default of target open');
              callback();
          }
        }else {//targetDoorState.CLOSED
//          this.lastClosed = new Date();
          switch (service.getCharacteristic(Characteristic.CurrentDoorState).value) {
            case Characteristic.CurrentDoorState.CLOSING:
            case Characteristic.CurrentDoorState.OPENING:
//              this.lastStopped = new Date();
              this.stoppedFlag = true;
              this.stopDoor(callback);
              this.service.setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.STOPPED);
              break;
            case Characteristic.CurrentDoorState.STOPPED:
            case Characteristic.CurrentDoorState.CLOSED:
            case Characteristic.CurrentDoorState.OPEN:
              this.stoppedFlag = false;
              this.closeDoor(callback);
              break;
            default:
              this.log('in default of target closed');
              callback();
         }
        }
      });
  },

  openDoor: function (callback) {
    //var client = this.sendCommand(this.openCommand);
    var callbackCalled = false;

    //client.on('data', (data) => {
    //  client.end();
    //  var dataString = data.toString('ascii');
    //  if (+dataString === +this.openCommand) {
        this.log('Opening Rollo should be called here!');
    //    this.simulateDoorOpening();
    //    callbackCalled = true;
    //    callback();

    if (this.open_url !== undefined) {
        request.get({
                url: this.open_url,
                        }, function(error, response, body) {
                        this.log("Status_URL: %s", this.open_url);
                        if (!error && response.statusCode == 200) {
                        if (body !== undefined) {
                                if (!isNaN(parseFloat(body)) && isFinite(body)) {
                                    this.lastState = (parseInt(body) > 0);
                                        this.log("Got Status %s",this.lastState);

                                } else {
                                        this.log("Warning, status returned isn't numeric: %s",body);
                                        this.simulateDoorOpening();
                                }
                        } else {
                                this.log("Warning, data returned isn't defined");
                        }
                        //this.log("callback CurrentState: %s", this.lastState);
                        callback();
                        //callback(null, service.getCharacteristic(Characteristic.TargetDoorState).value);
                        }
        }.bind(this));
    } else {
                callback();
                //callback(null, service.getCharacteristic(Characteristic.TargetDoorState).value);
    };
    callbackCalled = true;


  },

  closeDoor: function (callback) {
        var callbackCalled = false;
        this.log('Closing Rollo should be called here!');

    if (this.close_url !== undefined) {
    	request.get({
    		url: this.close_url,
  			}, function(error, response, body) {
  			this.log("Status_URL: %s", this.open_url);
  			if (!error && response.statusCode == 200) {
    			if (body !== undefined) {
    				if (!isNaN(parseFloat(body)) && isFinite(body)) {
	    			    this.lastState = (parseInt(body) > 0);	
	    				this.log("Got Status %s",this.lastState);

	    			} else {
	    				this.log("Warning, status returned isn't numeric: %s",body);
                                        this.simulateDoorClosing();
	    			}
    			} else {
    				this.log("Warning, data returned isn't defined");
    			}  	
    		        //this.log("callback CurrentState: %s", this.lastState);      				
  			callback();
                        //callback(null, service.getCharacteristic(Characteristic.TargetDoorState).value);
  			}
    	}.bind(this));
    } else {
  		callback();
                //callback(null, service.getCharacteristic(Characteristic.TargetDoorState).value);
    };
    callbackCalled = true;
  },

  stopDoor: function (callback) {
        var callbackCalled = false;

        this.log('Stoping Rollo should be called here!');
        //this.simulateDoorClosing();
        //callbackCalled = true;
        //callback();


    if (this.my_url !== undefined) {
        request.get({
                url: this.my_url,
                        }, function(error, response, body) {
                        this.log("Status_URL: %s", this.my_url);
                        if (!error && response.statusCode == 200) {
                        if (body !== undefined) {
                                if (!isNaN(parseFloat(body)) && isFinite(body)) {
                                    this.lastState = (parseInt(body) > 0);
                                        this.log("Got Status %s",this.lastState);

                                } else {
                                        this.log("Warning, status returned isn't numeric: %s",body);
                                        //this.service.setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.STOPPED);

                                }
                        } else {
                                this.log("Warning, data returned isn't defined");
                        }
                        //this.log("callback CurrentState: %s", this.lastState);
                        //callback();
                        callback();
                        }
        }.bind(this));
    } else {
                //callback();
                callback();
    };
    callbackCalled = true;


  },

  sendCommand: function (command) {
    const hmac = crypto.createHmac('sha256', this.key);
    var client = new net.Socket();
    client.connect(this.port, this.host, () => {
      var epochTime = Math.floor((new Date()).getTime() / 1000);
      hmac.update(epochTime + '', 'ascii');
      client.write(hmac.digest());
      client.write(new Buffer([command]));
    });
    client.on('error', (err) => {
      client.end();
      this.log(err);
    });
    return client;
  },

  simulateDoorOpening: function () {
    this.service.setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.OPENING);
    setTimeout(() => {
      if (this.stoppedFlag == false){this.service.setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.OPEN)};
    }, this.simulateTimeOpening * 1000);
  },

  simulateDoorClosing: function () {
    this.service.setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.CLOSING);
    setTimeout(() => {
       if (this.stoppedFlag == false){this.service.setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.CLOSED)};
    }, this.simulateTimeClosing * 1000);
  },

  simulateDoorOpeningClosing: function () {
    this.service.setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.OPENING);
    setTimeout(() => {
      this.service.setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.OPEN);
      setTimeout(() => {
        this.service.setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.CLOSING);
        this.service.setCharacteristic(Characteristic.TargetDoorState, Characteristic.TargetDoorState.CLOSED);
        setTimeout(() => {
          this.service.setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.CLOSED);
        }, this.simulateTimeClosing * 1000);
      }, this.simulateTimeOpen * 1000);
    }, this.simulateTimeOpening * 1000);
  }

};
