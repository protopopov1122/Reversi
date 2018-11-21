/*
  Copyright 2018 Jevgenijs Protopopovs

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
  in the documentation and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef REVERSI_ENGINE_TREE_H_
#define REVERSI_ENGINE_TREE_H_

#include <optional>
#include <memory>
#include <map>
#include <iosfwd>
#include <shared_mutex>
#include "reversi/engine/Threads.h"
#include "reversi/engine/State.h"

namespace Reversi {

  class Node; // Forward referencing

  using Move = std::optional<Position>;
  using MoveState = std::pair<Move, State>;

  struct ChildNode {
    ChildNode(Move move, std::shared_ptr<Node> child)
      : move(move), node(child) {}
    ChildNode(Position move, std::shared_ptr<Node> child)
      : move(move), node(child) {}
    ChildNode(const ChildNode &node)
      : move(node.move), node(node.node) {}
    ChildNode(ChildNode &&node) {
      this->move = std::move(node.move);
      this->node = std::move(node.node);
      node.node = nullptr;
    }
    
    ChildNode &operator=(const ChildNode &node) {
      this->move = node.move;
      this->node = node.node;
      return *this;
    }

    std::pair<Move, std::shared_ptr<Node>> asTuple() const {
      return std::make_pair(this->move, this->node);
    }
    Move move;
    std::shared_ptr<Node> node;
  };

  struct Strategy {
    std::function<int32_t (const State &)> white;
    std::function<int32_t (const State &)> black;
  };

  class NodeCache {
   public:
     bool has(const State &, std::size_t);
     std::shared_ptr<Node> get(const State &);
     void put(std::shared_ptr<Node>);
     std::size_t size() const;
   private:
    std::shared_mutex cacheMutex;
    std::map<State, std::shared_ptr<Node>> cache;
  };

  class ChildNodeIterator {
   public:
    virtual ~ChildNodeIterator() = default;
    virtual bool hasNext() = 0;
    virtual std::optional<ChildNode> next(int32_t, int32_t) = 0;
  };

  class Node {
   public:
    Node(const State &);
    std::optional<ChildNode> build(std::size_t, const Strategy &, FixedThreadPool &, bool = false, std::shared_ptr<NodeCache> = nullptr);
    int32_t getMetric() const;
    std::size_t getDepth() const;
    const State &getState() const;
    std::optional<ChildNode> getOptimalChild() const;

    friend std::ostream &operator<<(std::ostream &, const Node &);
    static std::ostream &dump(std::ostream &, const Node &, std::string = "\t", std::string = "");
   protected:
    const std::vector<ChildNode> &getChildren() const;
    int32_t traverseSequential(std::size_t, int32_t, int32_t, int, const Strategy &, std::shared_ptr<NodeCache> = nullptr);
   private:
    int32_t traverseParallel(std::size_t, int32_t, int32_t, int, const Strategy &, FixedThreadPool &, std::shared_ptr<NodeCache> = nullptr);
    int32_t directMetric(int, std::function<int32_t (const State &)>);
    int32_t evaluateMetricSequential(int32_t, int32_t, int, const Strategy &, std::shared_ptr<NodeCache>);
    int32_t evaluateMetricParallel(int32_t, int32_t, int, const Strategy &, FixedThreadPool &, std::shared_ptr<NodeCache>);
    int32_t evaluateMetric(ChildNodeIterator &, int32_t, int32_t);
    ChildNode getChildFromCache(Move, const State &, std::shared_ptr<NodeCache>);
    ChildNode buildChild(Move, const State &, std::function<void (std::shared_ptr<Node>, int32_t, int32_t)>, std::shared_ptr<NodeCache>,
      int32_t, int32_t);
    void generateFutures(std::vector<std::future<ChildNode>> &, std::function<void (std::shared_ptr<Node>)>, FixedThreadPool &);
    void randomizeOptimal();
    void selectOptimal(const Strategy &);

    State state;
    std::size_t depth;
    int32_t metric;
    std::vector<ChildNode> children;
    std::optional<ChildNode> optimal;
  };
}

#endif