#ifndef DEVICE_H
#define DEVICE_H

#include <nan.h>
#include <rtl-sdr.h>
#include <thread>
#include <vector>

class Device : public Nan::ObjectWrap
{
public:
  static NAN_MODULE_INIT(Init);
  static v8::Local<v8::Value> NewInstance();
  static NAN_METHOD(New);
  
  static NAN_METHOD(SetCallbacks);
  static NAN_GETTER(GetProperty);
  static NAN_METHOD(Open);
  static NAN_METHOD(Close);
  static NAN_METHOD(Start);
  static NAN_METHOD(Stop);
  static NAN_METHOD(EnableManualTunerGain);
  static NAN_METHOD(DisableManualTunerGain);
  static NAN_METHOD(SetIntermediateFrequencyGain);
  static NAN_METHOD(EnableAGC);
  static NAN_METHOD(DisableAGC);
  static NAN_METHOD(EnableTestMode);
  static NAN_METHOD(DisableTestMode);
  static NAN_GETTER(GetSampleRate);
  static NAN_SETTER(SetSampleRate);
  static NAN_GETTER(GetCenterFrequency);
  static NAN_SETTER(SetCenterFrequency);
  static NAN_GETTER(GetFrequencyCorrection);
  static NAN_SETTER(SetFrequencyCorrection);
  static NAN_GETTER(GetBufferNumber);
  static NAN_SETTER(SetBufferNumber);
  static NAN_GETTER(GetBufferLength);
  static NAN_SETTER(SetBufferLength);
  static NAN_GETTER(GetRtlOscillatorFrequency);
  static NAN_SETTER(SetRtlOscillatorFrequency);
  static NAN_GETTER(GetTunerOscillatorFrequency);
  static NAN_SETTER(SetTunerOscillatorFrequency);
  static NAN_GETTER(GetTunerGain);
  static NAN_SETTER(SetTunerGain);
  static NAN_GETTER(GetDirectSampling);
  static NAN_SETTER(SetDirectSampling);
  static NAN_GETTER(GetOffsetTuning);
  static NAN_SETTER(SetOffsetTuning);
  
  void initWithIndex(int i);
  void asyncData();

  uv_async_t* _async;
  
  Nan::Callback* _dataCb;
  Nan::Callback* _stoppedCb;
private:
  explicit Device();
  ~Device();
  
  int _index = -1;
  char _manufacturer[256];
  char _productname [256];
  char _serialnumber[256];
  const char* _devicename;
  
  bool _started;
  
  rtlsdr_dev_t* _devicePtr;
  
  uint32_t _bufferNumber;
  uint32_t _bufferLength;
  
  std::vector<int>* _gains;
  
  std::thread* _reader;
  
  static Nan::Persistent<v8::FunctionTemplate> _constructor;
};

struct AsyncMessage {
  Device* self;
  char* buffer;
  uint32_t length;
  bool stopped;
};

#endif