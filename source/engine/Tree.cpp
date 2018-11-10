#include "reversi/engine/Tree.h"
#include <iostream>
#include <random>

namespace Reversi {
  static std::random_device random_device;
  static std::mt19937 random_generator(random_device());

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
    std::scoped_lock<std::shared_mutex> lock(this->cacheMutex);
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

  const std::vector<ChildNode> &Node::getChildren() const {
    return this->children;
  }

  std::optional<ChildNode> Node::getOptimalChild() const {
    return this->optimal;
  }

  std::optional<ChildNode> Node::build(std::size_t depth, const Strategy &strategy, bool randomize, bool useCache) {
    this->depth = depth;
    std::shared_ptr<NodeCache> cache = useCache ? std::make_shared<NodeCache>() : nullptr;
    this->traverse(depth, INT16_MIN, INT16_MAX, this->state.getPlayer() == Player::White ? 1 : -1, false, strategy, cache, !randomize);
    if (randomize) {
      this->randomizeOptimal();
    }
    return this->optimal;
  }

  std::optional<ChildNode> Node::build(std::size_t depth, const Strategy &strategy, FixedThreadPool &pool, bool randomize, bool useCache) {
    this->depth = depth;
    std::shared_ptr<NodeCache> cache = useCache ? std::make_shared<NodeCache>() : nullptr;
    this->traverse(depth, INT16_MIN, INT16_MAX, this->state.getPlayer() == Player::White ? 1 : -1, false, strategy, pool, cache, !randomize);
    if (randomize) {
      this->randomizeOptimal();
    }
    return this->optimal;
  }

  int32_t Node::traverse(std::size_t depth, int32_t alpha, int32_t beta, int color, bool abortOnNoMoves, const Strategy &strategy,
    std::shared_ptr<NodeCache> cache, bool useAlphaBeta) {
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
      std::shared_ptr<Node> best_child = nullptr;
      State base(this->state);
      for (Position position : moves) {
        base.apply(position);
        if (cache && cache->has(base, depth - 1)) {
          std::shared_ptr<Node> child = cache->get(base);
          this->children.push_back(ChildNode(position, child));
          if (child->getMetric() > this->metric) {
            this->metric = child->getMetric();
            bestMove = position;
            best_child = child;
          }
        } else {
          auto child_best = this->addChild(position, base, depth, alpha, beta, color, strategy, cache);
          if (child_best) {
            std::tie(bestMove, best_child) = child_best.value().asTuple();
          }
        }
        base = this->state;
        
        if (this->metric > alpha) {
          alpha = this->metric;
        }
        if (alpha >= beta && useAlphaBeta) {
          break;
        }
      }
      if (best_child) {
        this->optimal = ChildNode(bestMove, best_child);
      }
      return this->metric;
    }
  }

  int32_t Node::traverse(std::size_t depth, int32_t alpha, int32_t beta, int color, bool abortOnNoMoves, const Strategy &strategy, FixedThreadPool &pool,
    std::shared_ptr<NodeCache> cache, bool useAlphaBeta) {
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
      std::shared_ptr<Node> best_child = nullptr;
      for (std::size_t i = 0; i < nodeFutures.size(); i++) {
        auto nodeFuture = std::move(nodeFutures.at(i));
        Position position = moves.at(i);
        auto child_best = this->addChild(std::move(nodeFuture), position);
        if (child_best) {
          std::tie(bestMove, best_child) = child_best.value().asTuple();
        }

        if (this->metric > alpha) {
          alpha = this->metric;
        }
        if (alpha >= beta && useAlphaBeta) {
          break;
        }
      }

      if (best_child) {
        this->optimal = ChildNode(bestMove, best_child);
      }
      return this->metric;
    }
  }

  std::ostream &Node::dump(std::ostream &os, const Node &root, std::string separator, std::string prefix) {
    const std::vector<ChildNode> &children = root.getChildren();
    for (const auto &child : children) {
      os << prefix;
      if (child.move) {
        os << prefix << child.move.value() << ' ';
      }
      os << child.node->getMetric() << std::endl;
      Node::dump(os, *child.node, separator, prefix + separator);
    }
    return os;
  }


  int32_t Node::zeroDepth(int color, std::function<int32_t (const State &)> score_assess) {
    this->metric = color * score_assess(this->state);
    return this->metric;
  }

  int32_t Node::noMoves(std::size_t depth, int32_t alpha, int32_t beta, int color, bool abortOnNoMoves, const Strategy &strategy, std::shared_ptr<NodeCache> cache) {
    std::function<int32_t (const State &)> score_assess = static_cast<int>(Player::White) == color ? strategy.white : strategy.black;
    if (abortOnNoMoves) {
      this->metric = color * score_assess(this->state);
      return this->metric;
    } else {
      State base(this->state);
      base.next();
      std::shared_ptr<Node> child = std::make_shared<Node>(base);
      this->metric = -child->traverse(depth - 1, -beta, -alpha, -color, true, strategy, cache);
      this->optimal = ChildNode(Move(), child);
      this->children.push_back(ChildNode(Move(), child)); // TODO
      return this->metric;
    }
  }

  std::optional<ChildNode> Node::addChild(Position position, const State &base, std::size_t depth, int32_t alpha, int32_t beta, int color, const Strategy &strategy,
    std::shared_ptr<NodeCache> cache) {
    std::optional<ChildNode> best;
    std::shared_ptr<Node> child = std::make_shared<Node>(base);
    int32_t child_metric = -child->traverse(depth - 1, -beta, -alpha, -color, false, strategy, cache);
    if (child_metric > this->metric) {
      this->metric = child_metric;
      best = ChildNode(position, child);
    }
    if (cache) {
      cache->put(child);
    }
    this->children.push_back(ChildNode(position, child));
    return best;
  }

  void Node::generateFutures(std::vector<Position> &moves, std::vector<std::future<std::shared_ptr<Node>>> &nodeFutures,
    std::size_t depth, int32_t alpha, int32_t beta, int color, const Strategy &strategy, FixedThreadPool &pool, std::shared_ptr<NodeCache> cache) {
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

  std::optional<ChildNode> Node::addChild(std::future<std::shared_ptr<Node>> nodeFuture, Position position) {
    std::optional<ChildNode> best;
    std::shared_ptr<Node> child = nodeFuture.get();
    int32_t child_metric = -child->getMetric();
    if (child_metric > this->metric) {
      this->metric = child_metric;
      best = ChildNode(position, child);
    }
    this->children.push_back(ChildNode(position, child));
    return best;
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