# sdrjs
A wrapper around [librtlsdr](http://sdr.osmocom.org/trac/wiki/rtl-sdr) for node.js

This module depends on `librtlsdr`, which you can install with most package managers.

Currently only tested on `Linux`

## sdrjs
sdrjs has the following functions:
#### getDevices()
 * Returns: `deviceArray` Array
Queries and returns the available `Device`s that are compatible with `librtlsdr`

## Device: Emitter
### Properties
Device has the following properties:

#### `Device.deviceName` String _readonly_
The name of the device, for example: `Generic RTL2832U OEM`

#### `Device.productName` String _readonly_
The product name of the device, for example: `RTL2838UHIDIR`

#### `Device.manufacturer` String _readonly_
The name of the product's manufacturer, for example: `Realtek`

#### `Device.serialNumber` String _readonly_
The serial number of the device, for example: `00000001`

#### `Device.tunerType` String _readonly_
The tuner's type, possible values: `Unknown`, `R828D`, `R820T`, `FC2580`, `FC0013`, `FC0012`, `E4000`

#### `Device.validGains` Array _readonly_
An array of valid gain values that the device will accept in manual gain mode.

#### `Device.sampleRate` Integer
The device's sample rate in Hz. Also sets the baseband filters according to the 
requested sample rate for tuners where this is possible. Possible values: 
`225001 - 300000 Hz`, `900001 - 3200000 Hz`.

Note form `librtlsdr`: _Sample loss is to be expected for rates > `2400000 Hz`._

#### `Device.centerFrequency` Integer
The device's center frequency in Hz (the frequency the device is tuned to).

#### `Device.frequencyCorrection` Integer
The device's frequency correction value in parts per million (ppm).

#### `Device.rtlOscillatorFrequency` Integer
The device's oscillator frequency in Hz.

#### `Device.tunerOscillatorFrequency` Integer
The tuner's oscillator frequency in Hz.

Note from `librtlsdr`: _Usually both ICs use the same clock. 
Changing the clock may make sense if you are applying an external clock to the tuner or 
to compensate the frequency (and samplerate) error caused by the original (cheap) crystal._

#### `Device.bufferNumber` Integer
The buffer number parameter that is used when reading from the device. Set to 0 to reset to default (15).

#### `Device.bufferLength` Integer
The buffer length parameter that is used when reading from the device. 
Must be a multiple of 512, should be a multiple of 16384 (URB size). 
Set to 0 to reset to default (16 * 32 * 512).

Note from `librtlsdr`: _bufferNumber * bufferLength = overall buffer size_

#### `Device.tunerGain` Integer
The tuner's gain in tenths of a dB (115 means 11.5 dB). 
Manual gain mode must be enabled for this to work.
Valid values can be read from `Device.validGains`.

#### `Device.directSampling` Enum (String)
Enable or disable the direct sampling mode. When enabled, the IF mode 
of the RTL2832 is activated, and rtlsdr_set_center_freq() will control
the IF-frequency of the DDC, which can be used to tune from 0 to 28.8 MHz
(oscillator frequency of the RTL2832).
Possible values: `disabled`, `I-ADC`, `Q-ADC`.

#### `Device.offsetTuning` Boolean
Enable or disable offset tuning for zero-IF tuners, which allows to avoid 
problems caused by the DC offset of the ADCs and 1/f noise.

### Methods
Device has the following methods:

#### `Device.open()` Device
Opens and initializes the device for usage.

#### `Device.close()` Device
Closes the device, which is now no longer usable.

#### `Device.start()` Device
Starts the thread that reads from the device. While the device is started, setting parameters might not take effect.

#### `Device.stop()` Device
Stops the thread that reads from the device.

#### `Device.enableManualTunerGain()` Device
Enables the manual tuner gain mode.

#### `Device.disableManualTunerGain()` Device
Disables the manual tuner gain mode.

#### `Device.enableTestMode()` Device
Enables the test mode. In this mode, an internal counter's value is returned in the buffer instead of real data.

#### `Device.disableTestMode()` Device
Disables the test mode.

#### `Device.enableAGC()` Device
Enables the internal digital AGC of the RTL2832.

#### `Device.disableAGC()` Device
Disables the internal digital AGC of the RTL2832.

#### `Device.setIntermediateFrequencyGain(gain)` Device
* `gain` Integer - the gain in tenths of a dB. (-30 means -3.0 dB)

Sets the intermediate frequency gain for the device.

### Events
Device emits the following events:

#### Event: 'data'
Returns: 
 * `bufferData` Object
  * `buffer` Buffer - the raw data read by `librtlsdr`
  * `length` Integer - the length of the buffer

Emitted when data is read from the device

#### Event: 'stopped'
Emitted when the thread that reads from the device stops
