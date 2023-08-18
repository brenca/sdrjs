const addon = require('bindings')('sdrjs')

let devices = addon.getDevices()
for (var i = 0; i < devices.length; i++) {
  let device = devices[i]
  
  device.open()
  device.centerFrequency = 1090000000
  device.sampleRate = 2000000
  device.bufferNumber = 12
  device.enableAGC()
  
  device.start()
  
  device.on("data", event => {
    console.log(event)
  })
  
  device.on("stopped", event => {
    device.close()
    console.log("closed")
  })
  
  setTimeout(() => {
    device.stop()
  },1000)
}