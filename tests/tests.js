const assert = require('assert')
const sdrjs = require('../addon.js')

describe('sdrjs', function() {
  describe('getDevices', function() {
    it('should return array of devices', (done) => {
      if (sdrjs.getDevices().constructor == Array) done()
      else done(Error("not an array"))
    })
  })
  
  describe('Device', function() {
    const devices = sdrjs.getDevices()
    
    it('should be connected', function(done) {
      if (devices.length > 0) done()
      else done(Error("No device connected"))
    })
    
    if (devices.length > 0) {    
      beforeEach(function(done) {
        this.timeout(10000)
        device.close()
        done()
      })
      
      const device = devices[0]
      
      describe('open() and close()', function() {
        it('should open the device and then close it', function(done) {
          try {
            device.open()
            device.close()
            done()
          } catch(e) {
            done(e)
          }
        })
      })
      
      describe('start() and stop()', function() {
        it('should start reading data and then stop reading', function(done) {
          try {
            device.open()
            device.start()
            
            device.stop()
            device.close()
            done()
          } catch(e) {
            done(e)
          }
        })
      })
      
      describe('stopped event', function() {
        it('should close the device once it\'s stopped', function(done) {
          try {
            device.open()
            device.start()
            
            device.once("stopped", () => {
              device.close()
              done()
            })
            
            device.stop()
          } catch(e) {
            done(e)
          }
        })
      })
      
      describe('data event', function() {
        it('should return a buffer read from the device', function(done) {
          this.timeout(20000)
          let val
          
          try {
            device.open()            
            device.start()
            
            let count = 0
            let datacb = (e) => {
              if (e.length == 0) {
                val = Error("Data was 0 length.")
              }
              
              if (count++ > 50) {
                device.off("data", datacb)
                device.stop()
              }
            }
            
            device.once("stopped", () => {
              device.close()
              done(val)
            })
            
            device.on("data", datacb)
          } catch(e) {
            done(e)
          }
        })
      })
      
      describe('.validGains', function() {
        it('should open the device and then close it', function(done) {
          try {
            device.open()
            let gains = device.validGains
            device.close()
            
            if (gains.length > 0) done()
            else done(Error("No valid gain values supplied by device."))
          } catch(e) {
            done(e)
          }
        })
      })
      
      describe('.centerFrequency', function() {
        it('should set the center frequency to the specified value', 
            function(done) {          
          try {
            device.open()
            device.centerFrequency = 1090000000
            if (device.centerFrequency == 1090000000) done()
            else done(Error("Center frequency was not set!"))
            device.close()
          } catch(e) {
            done(e)
          }
        })
      })
      
      describe('.sampleRate', function() {
        it('should set the sample rate to the specified value', 
            function(done) {          
          try {
            device.open()
            device.sampleRate = 2000000
            if (device.sampleRate == 2000000) done()
            else done(Error("Sample rate was not set!"))
            device.close()
          } catch(e) {
            done(e)
          }
        })
      })
      
      describe('.frequencyCorrection', function() {
        it('should set the frequency correction to the specified value', 
            function(done) {          
          try {
            device.open()
            device.frequencyCorrection = 200
            if (device.frequencyCorrection == 200) done()
            else done(Error("Frequency correction was not set!"))
            device.close()
          } catch(e) {
            done(e)
          }
        })
      })
      
      describe('.bufferNumber', function() {
        it('should set the buffer number to the specified value', 
            function(done) {          
          try {
            device.open()
            device.bufferNumber = 12
            if (device.bufferNumber == 12) done()
            else done(Error("Buffer number was not set!"))
            device.close()
          } catch(e) {
            done(e)
          }
        })
      })
      
      describe('.bufferLength', function() {
        it('should set the buffer length to the specified value', 
            function(done) {          
          try {
            device.open()
            device.bufferLength = 10000
            if (device.bufferLength == 10000) done()
            else done(Error("Buffer length was not set!"))
            device.close()
          } catch(e) {
            done(e)
          }
        })
      })
      
      describe('.rtlOscillatorFrequency', function() {
        it('should set the RTL oscillator frequency to the specified value', 
            function(done) {          
          try {
            device.open()
            let freq = device.rtlOscillatorFrequency
            device.rtlOscillatorFrequency = freq + 100
            if (device.rtlOscillatorFrequency == freq + 100) done()
            else done(Error("RTL oscillator frequency was not set!"))
            device.close()
          } catch(e) {
            done(e)
          }
        })
      })
      
      describe('.tunerOscillatorFrequency', function() {
        it('should set the tuner oscillator frequency to the specified value', 
            function(done) {          
          try {
            device.open()
            let freq = device.tunerOscillatorFrequency
            device.tunerOscillatorFrequency = freq + 100
            if (device.tunerOscillatorFrequency == freq + 100) done()
            else done(Error("Tuner oscillator frequency was not set!"))
            device.close()
          } catch(e) {
            done(e)
          }
        })
      })
      
      describe('.tunerGain', function() {
        it('should set the tuner gain to the specified value', 
            function(done) {          
          try {
            device.open()
            device.tunerGain = 10
            if (device.tunerGain == 10) done()
            else done(Error("Tuner gain was not set!"))
            device.close()
          } catch(e) {
            done(e)
          }
        })
      })
      
      describe('.directSampling', function() {
        it('should set the direct sampling to the specified value', 
            function(done) {
          let notset = Error("Direct sampling was not set!")
          
          try {
            device.open()
            device.directSampling = "disabled"
            if (device.directSampling != "disabled") done(notset)
            
            device.directSampling = "I-ADC"
            if (device.directSampling != "I-ADC") done(notset)
            
            device.directSampling = "Q-ADC"
            if (device.directSampling != "Q-ADC") done(notset)
            
            try {
              device.directSampling = "test"
              device.close()
              done(Error("Direct sampling was set to invalid value!"))
            } catch(err) {
              device.close()
              done()
            }
          } catch(e) {
            done(e)
          }
        })
      })
      
      describe('enable/disable manualTunerGain', function() {
        it('should toggle manual tuner gain mode', 
            function(done) {
          try {
            device.open()
            
            device.enableManualTunerGain()
            device.disableManualTunerGain()
            
            device.close()
            done()
          } catch(e) {
            done(e)
          }
        })
      })
      
      describe('setIntermediateFrequencyGain', function() {
        it('should set intermediate frequency gain to the specified value', 
            function(done) {
          try {
            device.open()
            
            device.setIntermediateFrequencyGain(-20)
            
            device.close()
            done()
          } catch(e) {
            done(e)
          }
        })
      })
      
      describe('enable/disable AGC', function() {
        it('should toggle AGC', 
            function(done) {
          try {
            device.open()
            
            device.enableAGC()
            device.disableAGC()
            
            device.close()
            done()
          } catch(e) {
            done(e)
          }
        })
      })
      
      describe('enable/disable testMode', function() {
        it('should toggle test mode', 
            function(done) {
          this.timeout(15000)
          let val
          try {
            device.open()
            
            device.enableTestMode()
            
            device.start()
            
            device.once("stopped", () => {
              device.disableTestMode()
              device.close()
              done(val)
            })
            
            device.once("data", e => {
              if (e.buffer[2] + 1 != e.buffer[3] || 
                  e.buffer[3] + 1 != e.buffer[4])
                val = Error(
                  "Test mode seems not to be working " +
                  "(data is not counter values)")
              device.stop()
            })
          } catch(e) {
            done(e)
          }
        })
      })
    }
  })
})