// Copyright © 2014 Wei Wang. All Rights Reserved.
// 2014-07-14 11:42

#include <glog/logging.h>
#include "model/edge.h"
#include "model/sgd_trainer.h"
#include "utils/lapis.h"


namespace lapis {
void Edge::Init(const EdgeProto &proto,
    const std::map<std::string, Layer *> &layer_map){
  name_ = proto.name();
  CHECK(layer_map.find(proto.top())!=layer_map.end());
  CHECK(layer_map.find(proto.bottom())!=layer_map.end());
  top_=layer_map.at(proto.top());
  bottom_=layer_map.at(proto.bottom());
  if(proto.directed()) {
      top_->add_in_edge(this);
      bottom_->add_out_edge(this);
  }else{
      top_->add_out_edge(this);
      bottom_->add_out_edge(this);
      top_->add_in_edge(this);
      bottom_->add_in_edge(this);
  }
}

void Edge::ToProto(EdgeProto *proto) {
  proto->set_name(name_);
}

void Edge::Setup(bool set_param) {
  LOG(INFO)<<"Not implemented";
}

void Edge::ComputeParamUpdates(const Trainer &trainer) {
  const SGDTrainer* sgd=reinterpret_cast<const SGDTrainer*> (&trainer);
  float momentum=sgd->momentum();
  float weight_decay=sgd->weight_decay();
  float learning_rate=sgd->learning_rate();
  for (Param* param : params_) {
    Blob2& history=param->mutable_history();
    const Blbo2& gradient=param->gradient();
    const Blbo2& data=param->content();
    momentum*=param->momentum();
    weight_decay*=param->weight_decay();
    learning_rate*=param->learning_rate();
    history=history*momentum-(gradient+weight_decay*data)*learning_rate;
  }
}

/*****************************************************************************
 * Edge Factory Implementation
 *****************************************************************************/
#define CreateEdge(EdgeClass) [](void)->Edge* {return new EdgeClass();}
std::shared_ptr<EdgeFactory> EdgeFactory::Instance() {
  if(!instance_.get()){
    instance_.reset(new EdgeFactory());
 }
  return instance_;
}

EdgeFactory::EdgeFactory() {
  RegisterCreateFunction("ConvEdge", CreateEdge(ConvEdge));
  RegisterCreateFunction("InnerProductEdge", CreateEdge(InnerProductEdge));
  RegisterCreateFunction("LRNEdge", CreateEdge(LRNEdge));
  RegisterCreateFunction("PoolingEdge", CreateEdge(PoolingEdge));
  RegisterCreateFunction("SoftmaxLossEdge", CreateEdge(SoftmaxLossEdge));
}

void EdgeFactory::RegisterCreateFunction(
  const std::string id,
  std::function<Edge*(void)> create_function) {
  edge_map_[id] = create_function;
}

Edge *EdgeFactory::Create(const std::string id) {
  CHECK(edge_map_.find(id) != edge_map_.end())
      << "The initialization function " << id << " has not been registered";
  return layer_map_[id]();
}
}  // namespace lapis
