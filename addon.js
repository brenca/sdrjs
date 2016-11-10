const EventEmitter = require('events')
const sdr = require('bindings')('sdrjs')

class Device extends EventEmitter {
  constructor (device) {
    super()
    this.device = device
    
    device.setCallbacks(
      (buffer) => { 
        this.emit("data", { buffer: buffer, length: buffer.length }) 
      },
      () => { 
        this.emit("stopped") 
      }
    )
  }
  
  get deviceName ()   { return this.device.deviceName }
  get productName ()  { return this.device.productName }
  get manufacturer () { return this.device.manufacturer }
  get serialNumber () { return this.device.serialNumber }
  get validGains ()   { return this.device.validGains }
  get tunerType ()    { return this.device.tunerType }
  
  get sampleRate () { 
    return this.device.sampleRate 
  }
  set sampleRate (value) { 
    this.device.sampleRate = value 
  }
  get centerFrequency () { 
    return this.device.centerFrequency 
  }
  set centerFrequency (value) { 
    this.device.centerFrequency = value 
  }
  get frequencyCorrection () { 
    return this.device.frequencyCorrection 
  }
  set frequencyCorrection (value) { 
    this.device.frequencyCorrection = value 
  }
  get bufferNumber () { 
    return this.device.bufferNumber 
  }
  set bufferNumber (value) { 
    this.device.bufferNumber = value 
  }
  get bufferLength () { 
    return this.device.bufferLength 
  }
  set bufferLength (value) { 
    this.device.bufferLength = value 
  }
  get rtlOscillatorFrequency () { 
    return this.device.rtlOscillatorFrequency 
  }
  set rtlOscillatorFrequency (value) { 
    this.device.rtlOscillatorFrequency = value 
  }
  get tunerOscillatorFrequency () { 
    return this.device.tunerOscillatorFrequency 
  }
  set tunerOscillatorFrequency (value) { 
    this.device.tunerOscillatorFrequency = value 
  }
  get tunerGain () { 
    return this.device.tunerGain 
  }
  set tunerGain (value) { 
    this.device.tunerGain = value 
  }
  get directSampling () { 
    return this.device.directSampling 
  }
  set directSampling (value) { 
    this.device.directSampling = value 
  }
  get offsetTuning () { 
    return this.device.offsetTuning 
  }
  set offsetTuning (value) { 
    this.device.offsetTuning = value 
  }
  
  open () {
    this.device.open()
  }
  close () {
    this.device.close()
  }
  start () {
    this.device.start()
  }
  stop () {
    this.device.stop()
  }
  
  enableManualTunerGain () {
    this.device.enableManualTunerGain()
  }
  disableManualTunerGain () {
    this.device.disableManualTunerGain()
  }
  setIntermediateFrequencyGain (value) {
    this.device.setIntermediateFrequencyGain(value)
  }
  enableAGC () {
    this.device.enableAGC()
  }
  disableAGC () {
    this.device.disableAGC()
  }
  enableTestMode () {
    this.device.enableTestMode()
  }
  disableTestMode () {
    this.device.disableTestMode()
  }
}

module.exports = {  
  getDevices () {
    let devices = []
    let d = sdr.getDevices()
    
    for (let i = 0; i < d.length; i++)
      devices.push(new Device(d[i]))
      
    return devices
  }
}