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

#include "reversi/engine/Tree.h"
#include "reversi/engine/Engine.h"
#include <iostream>
#include <iomanip>
#include <random>

namespace Reversi {
  static std::random_device random_device;
  static std::mt19937 random_generator(random_device());

  bool NodeCache::has(const State &state, std::size_t depth) {
    std::lock_guard<std::shared_mutex> lock(this->cacheMutex);
    return this->cache.count(state) > 0 &&
      this->cache[state]->getDepth() >= depth;
  }

  std::shared_ptr<Node> NodeCache::get(const State &state) {
    std::lock_guard<std::shared_mutex> lock(this->cacheMutex);
    if (this->cache.count(state) != 0) {
      return this->cache[state];
    } else {
      return nullptr;
    }
  }

  void NodeCache::put(std::shared_ptr<Node> node) {
    std::lock_guard<std::shared_mutex> lock(this->cacheMutex);
    if (node &&
      (this->cache.count(node->getState()) == 0 ||
      this->cache[node->getState()]->getDepth() < node->getDepth())) {
      this->cache[node->getState()] = node;
    }
  }



  void deriveSubstates(const State &state, std::vector<MoveState> &children) {
    std::vector<Position> moves;
    state.getBoard().getMoves(moves, state.getPlayer());
    State base(state);
    if (moves.empty()) {
      base.next();
      children.push_back(std::make_pair(Move(), base));
    } else {
      for (Position move : moves) {
        base.apply(move);
        children.push_back(std::make_pair(move, base));
        base = state;
      }
    }
  }

  using ChildNodeGenerator = std::function<ChildNode(Move, const State &, int32_t, int32_t)>;
  using ChildNodeCallback = std::function<void (ChildNode &)>;

  class ChildNodeSequentialIterator : public ChildNodeIterator {
   public:
    ChildNodeSequentialIterator(const State &base, ChildNodeGenerator gen) : index(0), gen(gen) {
      deriveSubstates(base, this->substates);
    }

    bool hasNext() override {
      return this->index < this->substates.size();
    }

    std::optional<ChildNode> next(int32_t alpha, int32_t beta) override {
      if (this->index >= this->substates.size()) {
        return std::optional<ChildNode>();
      } else {
        const MoveState &moveState = this->substates.at(this->index++);
        return this->gen(moveState.first, moveState.second, alpha, beta);
      }
    }
   private:
    std::vector<MoveState> substates;
    std::size_t index;
    ChildNodeGenerator gen;
  };

  class ChildNodeParallelIterator : public ChildNodeIterator {
   public:
    ChildNodeParallelIterator(std::vector<std::future<ChildNode>> &futures, ChildNodeCallback cb)
      : futures(futures), index(0), cb(cb) {}

    bool hasNext() override {
      return this->index < this->futures.size();
    }

    std::optional<ChildNode> next(int32_t alpha, int32_t beta) override {
      if (this->index >= this->futures.size()) {
        return std::optional<ChildNode>();
      } else {
        ChildNode node = this->futures.at(this->index++).get();
        cb(node);
        return node;
      }
    }
   private:
    std::vector<std::future<ChildNode>> &futures;
    std::size_t index;
    ChildNodeCallback cb;
  };

  Node::Node(const State &state)
    : state(state), depth(0), metric(INT32_MIN), optimal() {}

  int32_t Node::getMetric() const {
    return this->metric;
  }

  std::size_t Node::getDepth() const {
    return this->depth;
  }

  const State &Node::getState() const {
    return this->state;
  }

  const std::vector<ChildNode> &Node::getChildren() const {
    return this->children;
  }

  std::optional<ChildNode> Node::getOptimalChild() const {
    return this->optimal;
  }

  std::size_t Node::getSubNodeCount() const {
    std::size_t count = 0;
    for (const auto &child : this->children) {
      count += 1 + child.node->getSubNodeCount();
    }
    return count;
  }

  std::optional<ChildNode> Node::build(std::size_t depth, const Strategy &strategy, FixedThreadPool &pool, bool randomize) {
    std::shared_ptr<NodeCache> cache = ENABLE_TREE_NODE_CACHE ? std::make_shared<NodeCache>() : nullptr;
    this->traverseParallel(depth, INT16_MIN, INT16_MAX, this->state.getPlayer() == Player::White ? 1 : -1, strategy, pool, cache);
    if (randomize) {
      this->randomizeOptimal();
    } else {
      this->selectOptimal(strategy);
    }
    return this->optimal;
  }

  int32_t Node::traverseSequential(std::size_t depth, int32_t alpha, int32_t beta, int color, const Strategy &strategy,
    std::shared_ptr<NodeCache> cache) {

    this->depth = depth;
    std::function<int32_t (const State &)> score_assess = static_cast<int>(Player::White) == color ? strategy.white : strategy.black;

    if (depth == 0 || StateHelpers::isGameFinished(this->state)) {
      return this->directMetric(color, score_assess);
    } else {
      return this->evaluateMetricSequential(alpha, beta, color, strategy, cache);
    }
  }

  int32_t Node::traverseParallel(std::size_t depth, int32_t alpha, int32_t beta, int color, const Strategy &strategy, FixedThreadPool &pool,
    std::shared_ptr<NodeCache> cache) {
    this->depth = depth;
    std::function<int32_t (const State &)> score_assess = static_cast<int>(Player::White) == color ? strategy.white : strategy.black;
    if (depth == 0 || StateHelpers::isGameFinished(this->state)) {
      return this->directMetric(color, score_assess);
    } else {
      return this->evaluateMetricParallel(alpha, beta, color, strategy, pool, cache);
    }
  }


  int32_t Node::directMetric(int color, std::function<int32_t (const State &)> score_assess) {
    this->metric = color * score_assess(this->state);
    return this->metric;
  }

  int32_t Node::evaluateMetricParallel(int32_t alpha, int32_t beta, int color,
                               const Strategy &strategy, FixedThreadPool &pool, std::shared_ptr<NodeCache> cache) {
    std::function<void (std::shared_ptr<Node>)> childTraversal = [&](std::shared_ptr<Node> child) {
      child->traverseSequential(this->depth - 1, -beta, -alpha, -color, strategy, cache);
    };
    std::vector<std::future<ChildNode>> nodeFutures;
    this->generateFutures(nodeFutures, childTraversal, pool);
    ChildNodeParallelIterator iter(nodeFutures, [&](ChildNode &node) {
      this->children.push_back(node);
    });
    return this->evaluateMetric(iter, alpha, beta);
  }

  int32_t Node::evaluateMetricSequential(int32_t alpha, int32_t beta, int color, const Strategy &strategy, std::shared_ptr<NodeCache> cache) {
    auto childTraversal = [&](std::shared_ptr<Node> child, int32_t cAlpha, int32_t cBeta) {
      child->traverseSequential(this->depth - 1, -cBeta, -cAlpha, -color, strategy, cache);
    };
    ChildNodeSequentialIterator iter(this->state, [&](Move position, const State &base, int32_t iAlpha, int32_t iBeta) {
      if (cache && cache->has(base, this->depth - 1)) {
        return this->getChildFromCache(position, base, cache);
      } else {
        return this->buildChild(position, base, childTraversal, cache, iAlpha, iBeta);
      }
    });
    return this->evaluateMetric(iter, alpha, beta);
  }

  int32_t Node::evaluateMetric(ChildNodeIterator &iter, int32_t alpha, int32_t beta) {
    this->metric = INT32_MIN;
    std::optional<ChildNode> bestChild;
    while (iter.hasNext()) {
      std::optional<ChildNode> childNodeOpt = iter.next(alpha, beta);
      if (!childNodeOpt.has_value()) {
        continue;
      }
      ChildNode childNode = childNodeOpt.value();
      
      int32_t child_metric = -childNode.node->getMetric();
      if (child_metric > this->metric) {
        this->metric = child_metric;
        bestChild = childNode;
      }
      
      if (this->metric > alpha) {
        alpha = this->metric;
      }
      if (alpha >= beta) {
        break;
      }
    }
    if (bestChild) {
      this->optimal = bestChild;
    }
    return this->metric;
  }

  ChildNode Node::getChildFromCache(Move move, const State &base, std::shared_ptr<NodeCache> cache) {
    std::shared_ptr<Node> child = cache->get(base);
    ChildNode childNode(move, child);
    this->children.push_back(childNode);
    return childNode;
  }

  ChildNode Node::buildChild(Move position, const State &base, std::function<void (std::shared_ptr<Node>, int32_t, int32_t)> traverse,
                                          std::shared_ptr<NodeCache> cache,
                                          int32_t alpha, int32_t beta) {
    std::shared_ptr<Node> child = std::make_shared<Node>(base);
    traverse(child, alpha, beta);
    if (cache) {
      cache->put(child);
    }
    ChildNode childNode(position, child);
    this->children.push_back(childNode);
    return childNode;
  }

  void Node::generateFutures(std::vector<std::future<ChildNode>> &nodeFutures, std::function<void (std::shared_ptr<Node>)> traverse,
    FixedThreadPool &pool) {
    
    std::vector<MoveState> children;
    deriveSubstates(this->state, children);
    for (const auto &childState : children) {
      nodeFutures.push_back(pool.submit([=]() {
        std::shared_ptr<Node> child = std::make_shared<Node>(childState.second);
        traverse(child);
        return ChildNode(childState.first, child);
      }));
    }
  }

  void Node::randomizeOptimal() {
    if (!this->optimal.has_value()) {
      return;
    }
    int32_t metric = this->optimal.value().node->getMetric();
    std::vector<ChildNode> best_children;
    for (const auto &child : this->children) {
      if (child.node->getMetric() == metric) {
        best_children.push_back(child);
      }
    }
    if (best_children.size() > 1) {
      std::uniform_int_distribution<> distribution(0, best_children.size() - 1);
      this->optimal = best_children.at(distribution(random_generator));
    }
  }

  void Node::selectOptimal(const Strategy &strategy) {
    std::function<int32_t (const State &)> score_assess = this->state.getPlayer() == Player::White ? strategy.white : strategy.black;
    if (this->optimal) {
      int32_t max_own_metric = score_assess(this->optimal.value().node->getState());
      for (const auto &child : this->children) {
        int32_t own_metric = score_assess(child.node->getState());
        if (child.node->getMetric() == this->metric && own_metric > max_own_metric) {
          max_own_metric = own_metric;
          this->optimal = child;
        }
      }
    }
  }

  std::ostream &Node::dump(std::ostream &os, const Node &root, std::string separator, std::string prefix) {
    const std::vector<ChildNode> &children = root.getChildren();
    for (const auto &child : children) {
      os << prefix;
      if (child.move) {
        os << prefix << child.move.value() << ' ';
      }
      os << static_cast<int>(child.node->getState().getPlayer()) * child.node->getMetric() << std::endl;
      Node::dump(os, *child.node, separator, prefix + separator);
    }
    return os;
  }

  std::ostream &operator<<(std::ostream &os, const Node &root) {
    std::optional<ChildNode> optimal = root.getOptimalChild();
    if (optimal) {
      if (optimal.value().move) {
        os << optimal.value().move.value() << ' ';
      }
      os << root.getMetric() << std::endl;
    }
    return Node::dump(os, root, "\t", "\t");
  }
}