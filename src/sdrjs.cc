#include <nan.h>
#include <rtl-sdr.h>

#include "device.h"

using namespace v8;

void GetDevices(const Nan::FunctionCallbackInfo<Value>& info) {
  Isolate* isolate = info.GetIsolate();
  
  int count = rtlsdr_get_device_count();
  Local<Array> devices = Array::New(isolate, count);
  
  for (int i = 0; i < count; i++) {
    Local<Value> device = Device::NewInstance();
    Nan::ObjectWrap::Unwrap<Device>(device->ToObject())->initWithIndex(i);
    devices->Set(i, device);
  }
  
  info.GetReturnValue().Set(devices);
}

NAN_MODULE_INIT(InitAll) {  
  Device::Init(target);
  
  Nan::Set(target, Nan::New<String>("getDevices").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(GetDevices)).ToLocalChecked());
}

NODE_MODULE(sdrjs, InitAll)
