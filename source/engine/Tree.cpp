#include "reversi/engine/Tree.h"
#include <iostream>

namespace Reversi {

  bool NodeCache::has(const State &state, std::size_t depth) {
    std::shared_lock<std::shared_mutex> lock(this->cacheMutex);
    return this->cache.count(state) > 0 &&
      this->cache[state]->getDepth() >= depth;
  }

  std::shared_ptr<Node> NodeCache::get(const State &state) {
    std::shared_lock<std::shared_mutex> lock(this->cacheMutex);
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

  bool Node::isTerminal() const {
    return this->children.empty();
  }

  bool Node::isBuilt() const {
    return this->metric == INT32_MIN;
  }

  void Node::getChildren(std::vector<ChildNode> &children) const {
    for (const auto &child : this->children) {
      children.push_back(std::make_pair(child.first, child.second.get()));
    }
  }

  std::optional<ChildNode> Node::getOptimalChild() const {
    return this->optimal;
  }

  std::optional<ChildNode> Node::build(std::size_t depth, const Strategy &strategy, bool useCache) {
    this->depth = depth;
    NodeCache cache;
    this->traverse(depth, INT16_MIN, INT16_MAX, this->state.getPlayer() == Player::White ? 1 : -1, false, strategy, useCache ? &cache : nullptr);
    return this->optimal;
  }

  std::optional<ChildNode> Node::build(std::size_t depth, const Strategy &strategy, FixedThreadPool &pool, bool useCache) {
    this->depth = depth;
    NodeCache cache;
    this->traverse(depth, INT16_MIN, INT16_MAX, this->state.getPlayer() == Player::White ? 1 : -1, false, strategy, pool, useCache ? &cache : nullptr);
    return this->optimal;
  }

  std::pair<int32_t, Node *> Node::traverse(std::size_t depth, int32_t alpha, int32_t beta, int color, bool abortOnNoMoves, const Strategy &strategy,
    NodeCache *cache) {
    this->depth = depth;
    std::function<int32_t (const State &)> score_assess = static_cast<int>(Player::White) == color ? strategy.white : strategy.black;
    if (depth == 0) {
      return this->zeroDepth(color, score_assess);
    } else {
      std::vector<Position> moves;
      this->state.getBoard().getMoves(moves, this->state.getPlayer());
      if (moves.empty()) {
        return this->noMoves(depth, alpha, beta, color, abortOnNoMoves, strategy, cache);
      }

      this->metric = INT32_MIN;
      Move bestMove;
      Node *best_child = nullptr;
      State base(this->state);
      for (Position position : moves) {
        base.apply(position);
        if (cache && cache->has(base, depth - 1)) {
          std::shared_ptr<Node> child = cache->get(base);
          this->children.push_back(std::make_pair(position, child));
          if (child->getMetric() > this->metric) {
            this->metric = child->getMetric();
            bestMove = position;
            best_child = child.get();
          }
        } else {
          auto child_best = this->addChild(position, base, depth, alpha, beta, color, strategy, cache);
          if (child_best) {
            std::tie(bestMove, best_child) = child_best.value();
          }
        }
        base = this->state;
        
        if (this->metric > alpha) {
          alpha = this->metric;
        }
        if (alpha >= beta) {
          break;
        }
      }
      if (best_child) {
        this->optimal = std::make_pair(bestMove, best_child);
      }
      return std::make_pair(this->metric, best_child);
    }
  }

  std::pair<int32_t, Node *> Node::traverse(std::size_t depth, int32_t alpha, int32_t beta, int color, bool abortOnNoMoves, const Strategy &strategy, FixedThreadPool &pool,
    NodeCache *cache) {
    this->depth = depth;
    std::function<int32_t (const State &)> score_assess = static_cast<int>(Player::White) == color ? strategy.white : strategy.black;
    if (depth == 0) {
      return this->zeroDepth(color, score_assess);
    } else {
      std::vector<Position> moves;
      this->state.getBoard().getMoves(moves, this->state.getPlayer());
      if (moves.empty()) {
        return this->noMoves(depth, alpha, beta, color, abortOnNoMoves, strategy, cache);
      }

      std::vector<std::future<std::shared_ptr<Node>>> nodeFutures;
      this->generateFutures(moves, nodeFutures, depth, alpha, beta, color, strategy, pool, cache);
      

      this->metric = INT32_MIN;
      Move bestMove;
      Node *best_child = nullptr;
      for (std::size_t i = 0; i < nodeFutures.size(); i++) {
        auto nodeFuture = std::move(nodeFutures.at(i));
        Position position = moves.at(i);
        auto child_best = this->addChild(std::move(nodeFuture), position);
        if (child_best) {
          std::tie(bestMove, best_child) = child_best.value();
        }

        if (this->metric > alpha) {
          alpha = this->metric;
        }
        if (alpha >= beta) {
          break;
        }
      }

      if (best_child) {
        this->optimal = std::make_pair(bestMove, best_child);
      }
      return std::make_pair(this->metric, best_child);
    }
  }

  std::ostream &Node::dump(std::ostream &os, const Node &root, std::string separator, std::string prefix) {
    std::vector<std::pair<Move, Node *>> children;
    root.getChildren(children);
    for (const auto &child : children) {
      os << prefix;
      if (child.first) {
        os << prefix << child.first.value() << ' ';
      }
      os << child.second->getMetric() << std::endl;
      Node::dump(os, *child.second, separator, prefix + separator);
    }
    return os;
  }


  std::pair<int32_t, Node *> Node::zeroDepth(int color, std::function<int32_t (const State &)> score_assess) {
    this->metric = color * score_assess(this->state);
    return std::make_pair(this->metric, this);
  }

  std::pair<int32_t, Node *> Node::noMoves(std::size_t depth, int32_t alpha, int32_t beta, int color, bool abortOnNoMoves, const Strategy &strategy, NodeCache *cache) {
    std::function<int32_t (const State &)> score_assess = static_cast<int>(Player::White) == color ? strategy.white : strategy.black;
    if (abortOnNoMoves) {
      this->metric = color * score_assess(this->state);
      return std::make_pair(this->metric, this);
    } else {
      State base(this->state);
      base.next();
      std::shared_ptr<Node> child = std::make_shared<Node>(base);
      this->metric = -child->traverse(depth - 1, -beta, -alpha, -color, true, strategy, cache).first;
      this->optimal = std::make_pair(Move(), child.get());
      this->children.push_back(std::make_pair(Move(), std::move(child))); // TODO
      return std::make_pair(this->metric, this->optimal.value().second);
    }
  }

  std::optional<std::pair<Position, Node *>> Node::addChild(Position position, const State &base, std::size_t depth, int32_t alpha, int32_t beta, int color, const Strategy &strategy,
    NodeCache *cache) {
    std::optional<std::pair<Position, Node *>> best;
    std::shared_ptr<Node> child = std::make_shared<Node>(base);
    int32_t child_metric = -child->traverse(depth - 1, -beta, -alpha, -color, false, strategy, cache).first;
    if (child_metric > this->metric) {
      this->metric = child_metric;
      best = std::make_pair(position, child.get());
    }
    if (cache) {
      cache->put(child);
    }
    this->children.push_back(std::make_pair(position, child));
    return best;
  }

  void Node::generateFutures(std::vector<Position> &moves, std::vector<std::future<std::shared_ptr<Node>>> &nodeFutures,
    std::size_t depth, int32_t alpha, int32_t beta, int color, const Strategy &strategy, FixedThreadPool &pool, NodeCache *cache) {
    for (Position position : moves) {
      State base(this->state);
      base.apply(position);
      nodeFutures.push_back(pool.submit([=]() {
        std::shared_ptr<Node> child = std::make_shared<Node>(base);
        child->traverse(depth - 1, -beta, -alpha, -color, false, strategy, cache);
        return child;
      }));
    }
  }

  std::optional<std::pair<Position, Node *>> Node::addChild(std::future<std::shared_ptr<Node>> nodeFuture, Position position) {
    std::optional<std::pair<Position, Node *>> best;
    std::shared_ptr<Node> child = nodeFuture.get();
    int32_t child_metric = -child->getMetric();
    if (child_metric > this->metric) {
      this->metric = child_metric;
      best = std::make_pair(position, child.get());
    }
    this->children.push_back(std::make_pair(position, std::move(child)));
    return best;
  }

  std::ostream &operator<<(std::ostream &os, const Node &root) {
    std::optional<std::pair<Move, Node *>> optimal = root.getOptimalChild();
    if (optimal) {
      if (optimal.value().first) {
        os << optimal.value().first.value() << ' ';
      }
      os << root.getMetric() << std::endl;
    }
    return Node::dump(os, root, "\t", "\t");
  }
}