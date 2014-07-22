// Copyright © 2014 Wei Wang. All Rights Reserved.
// 2014-07-03 18:02

#include <google/protobuf/repeated_field.h>
#include <glog/logging.h>
#include "model/param.h"
using google::protobuf::RepeatedField;

namespace lapis {
void Param::Init(const ParamProto &proto) {
  const RepeatedField<int> shape = proto.shape();
  // currently only support vector and  matrix parameter
  if (shape.size() == 2) {
    int h = shape.Get(0);
    int w = shape.Get(1);
    content_.Reshape(h, w);
    grad_.Reshape(h, w);
    history_grad_.Reshape(h, w);
  } else {
    int l = shape.Get(0);
    content_.Reshape(l);
    grad_.Reshape(l);
    history_grad_.Reshape(l);
  }

  int length=content_.Length();
  float *val=content_.mutable_content();
  switch (proto.init_method()) {
    case ParamProto::kConstant: {
      for(int i=0;i<length;i++)
        val[i]=proto.value();
      break;
    }
    case ParamProto::kUniform: {
      FillUniformData(proto.low(), proto.high(), proto.value(), val);
      break;
    }
    case ParamProto::kUniformSqrtFanIn:{
      CHECK_EQ(shape.size(),2);
      FillUniformData(proto.low(), proto.high(),
                      proto.value()/sqrt(shape.Get(0)/3.0f), val);
      break;
    }
    case ParamProto::kUniformSqrtFanInOut: {
      CHECK_EQ(shape.size(),2);
      FillUniformData(proto.low(), proto.high(),
                      proto.value()/sqrt(shape.Get(0)+shape.Get(1)), val);
    }
    case ParamProto::kGaussain: {
      FillGaussainData(proto.mean(), proto.std(), proto.value(), val);
      break;
    }
    case ParamProto::kGaussainSqrtFanIn: {
      CHECK_EQ(shape.size(),2);
      FillGaussainData(proto.mean(), proto.std(),
                       proto.value()/sqrt(shape.Get(0)), val);
      break;
    }
    case ParamProto::kPretrained: {
      content_.set_content(proto.content().data());
      history_grad_.set_content(proto.history().data());
      break;
    }
    default: {
      LOG(ERROR)<<"Illegal parameter init method "<<proto.init_method();
    }
  }
  name_ = proto.name();
}

void Param::ToProto(ParamProto *proto) {
  // TODO(wangwei) store the proto as a member for easy ToProto.
}

void Param::FillGaussainData(float mean, float std, float factor, float *val) {
  std::normal_distribution<float> normal(mean,std);
  for (int i=0;i<Length();i++)
    val[i]=normal(Lapis::Get().rng())*factor;
}

void Param::FillUniformData(float low, float high, float factor, float *val) {
  LOG(INFO)<<low<<" "<<high;
  std::uniform_real_distribution<float> uniform(low,high);
  for (int i=0;i<Length();i++)
    val[i]=uniform(Lapis::Get().rng())*factor;
}


/**************************************************************************
 * Implementation for ParamInitFactory
 *************************************************************************/

ParamInitFactory *ParamInitFactory::Instance() {
  static ParamInitFactory factory;
  return &factory;
}

void ParamInitFactory::RegisterInitFunc(
  std::string id, const std::function<void(Param *)> &func) {
  map_[id] = func;
}

std::function<void(Param *)> &ParamInitFactory::Get(std::string id) {
  CHECK(map_.find(id) != map_.end()) << "The initialization function " << id
                                     << " has not been registered\n";
  return map_[id];
}
}  // namespace lapis
