#include "device.h"

#include <iostream>

using namespace v8;

Device::Device() { 
  _started = false;
  _devicePtr = NULL;
  _gains = NULL;
  _reader = NULL;
  
  _bufferNumber = 32;
  _bufferLength = 16 * 16384;
}

Device::~Device() {
  if (_devicePtr != NULL) {
    rtlsdr_cancel_async(_devicePtr);
    rtlsdr_close(_devicePtr);
    _devicePtr = NULL;
  }
  
  if (_gains != NULL) {
    delete _gains;
    _gains = NULL;
  }
  
  if (_reader != NULL) {
    delete _reader;
    _reader = NULL;
  }
}

Nan::Persistent<FunctionTemplate> Device::_constructor;

NAN_MODULE_INIT(Device::Init) {
	Local<FunctionTemplate> tpl =
		Nan::New<FunctionTemplate>(Device::New);
	_constructor.Reset(tpl);
	tpl->SetClassName(Nan::New("Device").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Local<ObjectTemplate> proto = tpl->PrototypeTemplate();

  Nan::SetAccessor(proto, 
    Nan::New("deviceName").ToLocalChecked(), GetProperty);
  Nan::SetAccessor(proto, 
    Nan::New("productName").ToLocalChecked(), GetProperty);
  Nan::SetAccessor(proto, 
    Nan::New("manufacturer").ToLocalChecked(), GetProperty);
  Nan::SetAccessor(proto, 
    Nan::New("serialNumber").ToLocalChecked(), GetProperty);
  Nan::SetAccessor(proto, 
    Nan::New("validGains").ToLocalChecked(), GetProperty);
  Nan::SetAccessor(proto, 
    Nan::New("tunerType").ToLocalChecked(), GetProperty);
    
  Nan::SetAccessor(proto, 
    Nan::New("sampleRate").ToLocalChecked(), 
    GetSampleRate, SetSampleRate);
  Nan::SetAccessor(proto, 
    Nan::New("centerFrequency").ToLocalChecked(), 
    GetCenterFrequency, SetCenterFrequency);
  Nan::SetAccessor(proto, 
    Nan::New("frequencyCorrection").ToLocalChecked(), 
    GetFrequencyCorrection, SetFrequencyCorrection);
  Nan::SetAccessor(proto, 
    Nan::New("bufferNumber").ToLocalChecked(), 
    GetBufferNumber, SetBufferNumber);
  Nan::SetAccessor(proto, 
    Nan::New("bufferLength").ToLocalChecked(), 
    GetBufferLength, SetBufferLength);
  Nan::SetAccessor(proto, 
    Nan::New("rtlOscillatorFrequency").ToLocalChecked(), 
    GetRtlOscillatorFrequency, SetRtlOscillatorFrequency);
  Nan::SetAccessor(proto, 
    Nan::New("tunerOscillatorFrequency").ToLocalChecked(), 
    GetTunerOscillatorFrequency, SetTunerOscillatorFrequency);
  Nan::SetAccessor(proto, 
    Nan::New("tunerGain").ToLocalChecked(), 
    GetTunerGain, SetTunerGain);
  Nan::SetAccessor(proto, 
    Nan::New("directSampling").ToLocalChecked(), 
    GetDirectSampling, SetDirectSampling);
  Nan::SetAccessor(proto, 
    Nan::New("offsetTuning").ToLocalChecked(), 
    GetOffsetTuning, SetOffsetTuning);

  Nan::SetPrototypeMethod(tpl, "open", Open);
  Nan::SetPrototypeMethod(tpl, "close", Close);
  Nan::SetPrototypeMethod(tpl, "start", Start);
  Nan::SetPrototypeMethod(tpl, "stop", Stop);

  Nan::SetPrototypeMethod(
    tpl, "enableManualTunerGain", EnableManualTunerGain);
  Nan::SetPrototypeMethod(
    tpl, "disableManualTunerGain", DisableManualTunerGain);
  Nan::SetPrototypeMethod(
    tpl, "setIntermediateFrequencyGain", SetIntermediateFrequencyGain);
  Nan::SetPrototypeMethod(tpl, "enableAGC", EnableAGC);
  Nan::SetPrototypeMethod(tpl, "disableAGC", DisableAGC);
  Nan::SetPrototypeMethod(tpl, "enableTestMode", EnableTestMode);
  Nan::SetPrototypeMethod(tpl, "disableTestMode", DisableTestMode);
  
  Nan::SetPrototypeMethod(tpl, "on", Emitter::On);
  Nan::SetPrototypeMethod(tpl, "once", Emitter::Once);
  Nan::SetPrototypeMethod(tpl, "off", Emitter::Off);
}

Local<Value> Device::NewInstance() {
  Nan::EscapableHandleScope scope;
  Local<FunctionTemplate> ctorHandle = Nan::New(_constructor);
  Local<Function> ctor = Nan::GetFunction(ctorHandle).ToLocalChecked();
  Local<Object> device = Nan::NewInstance(ctor).ToLocalChecked();
  
  return scope.Escape(device);
}

NAN_METHOD(Device::New) {
	Device* device = new Device();
	device->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
}

void Device::initWithIndex(int i) {
  _index = i; 
  
  if(rtlsdr_get_device_usb_strings(
      _index, _manufacturer, _productname, _serialnumber)) {
    Nan::ThrowError("Could not get device data!");
    return;
  }
  
  _devicename = rtlsdr_get_device_name(_index);
}

void dataCallback(unsigned char *buf, uint32_t len, void *ctx) {
  Device* device = (Device*)ctx;
  if (device != NULL && buf != NULL && len > 0) {
    device->Emit("data", [buf, len](huron::Dictionary& dict) {
      dict.Set("buffer", Nan::CopyBuffer((char*)buf, len).ToLocalChecked());
      dict.Set("length", len);
    });
  }
}

void Device::asyncData() {
  if (this->_started && rtlsdr_read_async(
      this->_devicePtr, &dataCallback, 
      (void*)this, _bufferNumber, _bufferLength)) {
    Nan::ThrowError("Could not start device.");
    return;
  }
  
  Emit("stopped", [](huron::Dictionary& dict) {});
}

NAN_GETTER(Device::GetProperty) {
  Isolate* isolate = info.GetIsolate();
  
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  std::string prop(*String::Utf8Value(property));
  Local<Value> value;
  
  if (prop == "deviceName") {
    value = Nan::New<String>(device->_devicename).ToLocalChecked();
  } else if (prop == "productName") {
    value = Nan::New<String>(device->_productname).ToLocalChecked();
  } else if (prop == "manufacturer") {
    value = Nan::New<String>(device->_manufacturer).ToLocalChecked();
  } else if (prop == "serialNumber") {
    value = Nan::New<String>(device->_serialnumber).ToLocalChecked();
  } else if (prop == "validGains") {
    int length = 0;
    if (device->_gains != NULL) length = device->_gains->size();
    Local<Array> gains = Array::New(isolate, length);
    
    for (int i = 0; i < length; i++) {
      gains->Set(i, Nan::New(device->_gains->at(i)));
    }
    
    value = gains;
  } else if (prop == "tunerType") {
    switch (rtlsdr_get_tuner_type(device->_devicePtr)) {
      case RTLSDR_TUNER_E4000:
        value = Nan::New<String>("E4000").ToLocalChecked();
      break;
      case RTLSDR_TUNER_FC0012:
        value = Nan::New<String>("FC0012").ToLocalChecked();
      break;
      case RTLSDR_TUNER_FC0013:
        value = Nan::New<String>("FC0013").ToLocalChecked();
      break;
      case RTLSDR_TUNER_FC2580:
        value = Nan::New<String>("FC2580").ToLocalChecked();
      break;
      case RTLSDR_TUNER_R820T:
        value = Nan::New<String>("R820T").ToLocalChecked();
      break;
      case RTLSDR_TUNER_R828D:
        value = Nan::New<String>("R828D").ToLocalChecked();
      break;
      default:
        value = Nan::New<String>("Unknown").ToLocalChecked();
      break;
    }
  }
  
  info.GetReturnValue().Set(value);
}

NAN_METHOD(Device::Open) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if(rtlsdr_open(&(device->_devicePtr), device->_index) < 0) {
    Nan::ThrowError("Could not open device.");
  }
  
  int count = rtlsdr_get_tuner_gains(device->_devicePtr, NULL);
  device->_gains = new std::vector<int>(count);
  rtlsdr_get_tuner_gains(device->_devicePtr, device->_gains->data());
  
	info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Device::Close) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    rtlsdr_close(device->_devicePtr);
    device->_devicePtr = NULL;
  }
  
  if (device->_gains != NULL) {
    delete device->_gains;
    device->_gains = NULL;
  }
  
  if (device->_reader != NULL) {
    delete device->_reader;
    device->_reader = NULL;
  }
  
	info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Device::Start) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    if(rtlsdr_reset_buffer(device->_devicePtr)) {
      Nan::ThrowError("Failed to reset buffer.");
    }
    device->_started = true;
    device->_reader = new std::thread(&Device::asyncData, device);
    device->_reader->detach();
  } else {
    Nan::ThrowError("Device not open!");
  }
  
	info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Device::Stop) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    rtlsdr_cancel_async(device->_devicePtr);
    device->_started = false;
  } else {
    Nan::ThrowError("Device not open!");
  }
  
	info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Device::EnableManualTunerGain) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    if (rtlsdr_set_tuner_gain_mode(device->_devicePtr, 1) < 0) {
      Nan::ThrowError("Could not enable manual gain mode.");
    }
  } else {
    Nan::ThrowError("Device not open!");
  }
  
	info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Device::DisableManualTunerGain) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    if (rtlsdr_set_tuner_gain_mode(device->_devicePtr, 0) < 0) {
      Nan::ThrowError("Could not disable manual gain mode.");
    }
  } else {
    Nan::ThrowError("Device not open!");
  }
  
	info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Device::SetIntermediateFrequencyGain) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    int32_t if_gain;
    if (info[0]->IsInt32()) {
      if_gain = info[0]->Int32Value();
    } else {
      Nan::ThrowError(
        "You must give a whole number as intermediate frequency gain!");
      
      info.GetReturnValue().Set(info.This());
      return;
    }
    
    int32_t stage = 1;
    if (info[1]->IsInt32()) {
      stage = info[1]->Int32Value();
    }
    
    if (rtlsdr_set_tuner_if_gain(device->_devicePtr, stage, if_gain) < 0) {
      Nan::ThrowError("Could not set intermediate frequency gain.");
    }
  } else {
    Nan::ThrowError("Device not open!");
  }
  
	info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Device::EnableAGC) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    if (rtlsdr_set_agc_mode(device->_devicePtr, 1) < 0) {
      Nan::ThrowError("Could not enable AGC.");
    }
  } else {
    Nan::ThrowError("Device not open!");
  }
  
	info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Device::DisableAGC) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    if (rtlsdr_set_agc_mode(device->_devicePtr, 0) < 0) {
      Nan::ThrowError("Could not disable AGC.");
    }
  } else {
    Nan::ThrowError("Device not open!");
  }
  
	info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Device::EnableTestMode) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    if (rtlsdr_set_testmode(device->_devicePtr, 1) < 0) {
      Nan::ThrowError("Could not enable test mode.");
    }
  } else {
    Nan::ThrowError("Device not open!");
  }
  
	info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Device::DisableTestMode) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    if (rtlsdr_set_testmode(device->_devicePtr, 0) < 0) {
      Nan::ThrowError("Could not disable test mode.");
    }
  } else {
    Nan::ThrowError("Device not open!");
  }
  
	info.GetReturnValue().Set(info.This());
}

NAN_GETTER(Device::GetSampleRate) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    Local<Uint32> value = Nan::New(
      rtlsdr_get_sample_rate(device->_devicePtr)
    );
  	info.GetReturnValue().Set(value);
  } else {
    Nan::ThrowError("Device not open!");
    info.GetReturnValue().Set(Undefined(info.GetIsolate()));
  }
}

NAN_SETTER(Device::SetSampleRate) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    if (value->IsUint32()) {
      uint32_t sampleRate = value->Uint32Value();
      
      if(rtlsdr_set_sample_rate(device->_devicePtr, sampleRate) < 0) {
        Nan::ThrowError("Could not set sample rate.");
      }
    } else {
      Nan::ThrowError("Sample rate must be a positive whole number");
    }
  	info.GetReturnValue().Set(info.This());
  }
  
  info.GetReturnValue().Set(Undefined(info.GetIsolate()));
}

NAN_GETTER(Device::GetCenterFrequency) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    Local<Uint32> value = Nan::New(
      rtlsdr_get_center_freq(device->_devicePtr)
    );
  	info.GetReturnValue().Set(value);
  } else {
    Nan::ThrowError("Device not open!");
    info.GetReturnValue().Set(Undefined(info.GetIsolate()));
  }
}

NAN_SETTER(Device::SetCenterFrequency) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    if (value->IsUint32()) {
      uint32_t centerFreq = value->Uint32Value();
      
      if(rtlsdr_set_center_freq(device->_devicePtr, centerFreq) < 0) {
        Nan::ThrowError("Could not set center frequency.");
      }
    } else {
      Nan::ThrowError("Center frequency must be a positive whole number");
    }
  	info.GetReturnValue().Set(info.This());
  }
  
  info.GetReturnValue().Set(Undefined(info.GetIsolate()));
}

NAN_GETTER(Device::GetFrequencyCorrection) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    Local<Integer> value = Nan::New(
      rtlsdr_get_freq_correction(device->_devicePtr)
    );
  	info.GetReturnValue().Set(value);
  } else {
    Nan::ThrowError("Device not open!");
    info.GetReturnValue().Set(Undefined(info.GetIsolate()));
  }
}

NAN_SETTER(Device::SetFrequencyCorrection) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    if (value->IsInt32()) {
      int32_t freqCorrection = value->Int32Value();
      
      if(rtlsdr_set_freq_correction(device->_devicePtr, freqCorrection) < 0) {
        Nan::ThrowError("Could not set frequency correction.");
      }
    } else {
      Nan::ThrowError("Frequency correction must be a whole number");
    }
  }
  
	info.GetReturnValue().Set(info.This());
}

NAN_GETTER(Device::GetBufferNumber) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  Local<Integer> value = Nan::New(device->_bufferNumber);
	info.GetReturnValue().Set(value);
}

NAN_SETTER(Device::SetBufferNumber) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (value->IsInt32()) {    
    device->_bufferNumber = value->Int32Value();
  } else {
    Nan::ThrowError("Buffer number must be a whole number");
  }
  
	info.GetReturnValue().Set(info.This());
}

NAN_GETTER(Device::GetBufferLength) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  Local<Integer> value = Nan::New(device->_bufferLength);
	info.GetReturnValue().Set(value);
}

NAN_SETTER(Device::SetBufferLength) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (value->IsInt32()) {    
    device->_bufferLength = value->Int32Value();
  } else {
    Nan::ThrowError("Buffer length must be a whole number");
  }
  
	info.GetReturnValue().Set(info.This());
}

NAN_GETTER(Device::GetRtlOscillatorFrequency) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    uint32_t rtl_freq, tuner_freq;
    rtlsdr_get_xtal_freq(device->_devicePtr, &rtl_freq, &tuner_freq);
    
  	info.GetReturnValue().Set(Nan::New(rtl_freq));
  } else {
    Nan::ThrowError("Device not open!");
    info.GetReturnValue().Set(Undefined(info.GetIsolate()));
  }
}

NAN_SETTER(Device::SetRtlOscillatorFrequency) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    if (value->IsUint32()) {
      uint32_t rtl_freq, tuner_freq;
      rtlsdr_get_xtal_freq(device->_devicePtr, &rtl_freq, &tuner_freq);
      rtl_freq = value->Uint32Value();
      
      if(rtlsdr_set_xtal_freq(device->_devicePtr, rtl_freq, tuner_freq) < 0) {
        Nan::ThrowError("Could not set RTL oscillator frequency.");
      }
    } else {
      Nan::ThrowError(
        "RTL oscillator frequency must be a positive whole number");
    }
  }
  
	info.GetReturnValue().Set(info.This());
}

NAN_GETTER(Device::GetTunerOscillatorFrequency) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    uint32_t rtl_freq, tuner_freq;
    rtlsdr_get_xtal_freq(device->_devicePtr, &rtl_freq, &tuner_freq);
    
  	info.GetReturnValue().Set(Nan::New(tuner_freq));
  } else {
    Nan::ThrowError("Device not open!");
    info.GetReturnValue().Set(Undefined(info.GetIsolate()));
  }
}

NAN_SETTER(Device::SetTunerOscillatorFrequency) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    if (value->IsUint32()) {
      uint32_t rtl_freq, tuner_freq;
      rtlsdr_get_xtal_freq(device->_devicePtr, &rtl_freq, &tuner_freq);
      tuner_freq = value->Uint32Value();
      
      if(rtlsdr_set_xtal_freq(device->_devicePtr, rtl_freq, tuner_freq) < 0) {
        Nan::ThrowError("Could not set tuner oscillator frequency.");
      }
    } else {
      Nan::ThrowError(
        "Tuner oscillator frequency must be a positive whole number");
    }
  }
  
	info.GetReturnValue().Set(info.This());
}

NAN_GETTER(Device::GetTunerGain) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    Local<Integer> value = Nan::New(
      rtlsdr_get_tuner_gain(device->_devicePtr)
    );
  	info.GetReturnValue().Set(value);
  } else {
    Nan::ThrowError("Device not open!");
    info.GetReturnValue().Set(Undefined(info.GetIsolate()));
  }
}

NAN_SETTER(Device::SetTunerGain) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    if (value->IsInt32()) {
      int32_t tunerGain = value->Int32Value();
      
      if(rtlsdr_set_tuner_gain(device->_devicePtr, tunerGain) < 0) {
        Nan::ThrowError("Could not set tuner gain.");
      }
    } else {
      Nan::ThrowError("Tuner gain must be a whole number");
    }
  }
  
	info.GetReturnValue().Set(info.This());
}

NAN_GETTER(Device::GetDirectSampling) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    int val = rtlsdr_get_direct_sampling(device->_devicePtr);
    if (val < 0) {
      Nan::ThrowError("Could not get direct sampling.");
      info.GetReturnValue().Set(Undefined(info.GetIsolate()));
      return;
    } else {
      Local<String> value;
      
      switch (val) {
        case 0:
          value = Nan::New<String>("disabled").ToLocalChecked();
        break;
        case 1:
          value = Nan::New<String>("I-ADC").ToLocalChecked();
        break;
        case 2:
          value = Nan::New<String>("Q-ADC").ToLocalChecked();
        break;
      }
      
    	info.GetReturnValue().Set(value);
    }
    
  } else {
    Nan::ThrowError("Device not open!");
    info.GetReturnValue().Set(Undefined(info.GetIsolate()));
  }
}

NAN_SETTER(Device::SetDirectSampling) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    if (value->IsString()) {
      std::string directSampling(*String::Utf8Value(value));
      int val = 0;
      
      if (directSampling == "disabled") {
        val = 0;
      } else if (directSampling == "I-ADC") {
        val = 1;
      } else if (directSampling == "Q-ADC") {
        val = 2;
      } else {
        Nan::ThrowError("Direct sampling must be disabled, I-ADC or Q-ADC");
        info.GetReturnValue().Set(info.This());
        return;
      }
      
      if(rtlsdr_set_direct_sampling(device->_devicePtr, val) < 0) {
        Nan::ThrowError("Could not set direct sampling.");
      }
    } else {
      Nan::ThrowError("Direct sampling must be a string");
    }
  }
  
	info.GetReturnValue().Set(info.This());
}

NAN_GETTER(Device::GetOffsetTuning) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    Local<Boolean> value = Nan::New<Boolean>(
      rtlsdr_get_offset_tuning(device->_devicePtr)
    );
  	info.GetReturnValue().Set(value);
  } else {
    Nan::ThrowError("Device not open!");
    info.GetReturnValue().Set(Undefined(info.GetIsolate()));
  }
}

NAN_SETTER(Device::SetOffsetTuning) {
  Device* device = Nan::ObjectWrap::Unwrap<Device>(info.This());
  
  if (device->_devicePtr != NULL) {
    if (value->IsBoolean()) {
      bool ot = value->BooleanValue();
      
      if(rtlsdr_set_offset_tuning(device->_devicePtr, ot ? 1 : 0) < 0) {
        Nan::ThrowError("Could not set offset tuning.");
      }
    } else {
      Nan::ThrowError("Offset tuning must be a boolean value");
    }
  }
  
	info.GetReturnValue().Set(info.This());
}