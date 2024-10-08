// Copyright 2022-present 650 Industries. All rights reserved.

#include "JSIUtils.h"
#include "LazyObject.h"

namespace expo {

LazyObject::LazyObject(LazyObjectInitializer initializer) : initializer(std::move(initializer)) {}

LazyObject::~LazyObject() {
  backedObject = nullptr;
}

jsi::Value LazyObject::get(jsi::Runtime &runtime, const jsi::PropNameID &name) {
  if (!backedObject) {
    if (name.utf8(runtime) == "$$typeof") {
      // React Native asks for this property for some reason, we can just ignore it.
      return jsi::Value::undefined();
    }
    initializeBackedObject(runtime);
  }
  return backedObject ? backedObject->getProperty(runtime, name) : jsi::Value::undefined();
}

void LazyObject::set(jsi::Runtime &runtime, const jsi::PropNameID &name, const jsi::Value &value) {
  if (!backedObject) {
    initializeBackedObject(runtime);
  }
  if (backedObject) {
    backedObject->setProperty(runtime, name, value);
  }
}

std::vector<jsi::PropNameID> LazyObject::getPropertyNames(jsi::Runtime &runtime) {
  if (!backedObject) {
    initializeBackedObject(runtime);
  }
  if (backedObject) {
    jsi::Array propertyNames = backedObject->getPropertyNames(runtime);
    return common::jsiArrayToPropNameIdsVector(runtime, propertyNames);
  }
  return {};
}

const jsi::Object &LazyObject::unwrapObjectIfNecessary(jsi::Runtime &runtime, const jsi::Object &object) {
  if (object.isHostObject<LazyObject>(runtime)) {
    LazyObject::Shared lazyObject = object.getHostObject<LazyObject>(runtime);

    if (!lazyObject->backedObject) {
      lazyObject->initializeBackedObject(runtime);
    }
    return *lazyObject->backedObject;
  }
  return object;
}

} // namespace expo
