// Copyright © 2014 Wei Wang. All Rights Reserved.
// 2014-07-14 13:12

#ifndef INCLUDE_MODEL_NET_H_
#define INCLUDE_MODEL_NET_H_

#include <google/protobuf/repeated_field.h>
#include <vector>
#include "net/layer.h"
#include "net/edge.h"
#include "net/param.h"
#include "datasource/data_source.h"
#include "proto/model.pb.h"


namespace lapis {
/**
 * Forward declaration of Edge and Layer
 */
class Edge;
class Layer;
/**
 * The neural network consists of Layers and Edges.
 */
class Net {
 public:
  /**
   * construct the net structure, i.e., how edges and layers are connected
   */
  explicit Net(const NetProto &net_proto);
  /**
   * setup the net by init dary shapes,
   * then allocate memory(optional) and init parameters (optional)
   */
  void Setup(const char flag,int batchsize,
             const std::map<std::string, Shape> &shapes,
             const std::map<std::string, int> & stores);
  void Setup(const char flag,int batchsize,
             const std::map<std::string, Shape> &shapes);

  /**
   * set shapes of DArys
   */
  void SetShape(const int batchsize, const Record &record);
  /**
   * allocate memory for DArys
   */
  void AllocateMemory();
  /**
   * init parameters, must be called after InitDAryShape
   * if memory of parameters are not allocated, do memory allocation before
   * init parameters
   */
  void InitParameters();
  void ToProto(NetProto *net_proto);
  std::vector<InputLayer *> &input_layers() {
    return input_layers_;
  }
  OutputLayer* output_layer() {
    return output_layer_;
  }
  const std::vector<Param *> &params() {
    return params_;
  }
  ~Net();
 private:
  OutputLayer * output_layer_;
  std::vector<Layer *> layers_;
  std::vector<Layer *> input_layers_;
  std::vector<Edge *> edges_;
  std::vector<Param *> params_;
};

}  // namespace lapis
#endif  // INCLUDE_MODEL_NET_H_
