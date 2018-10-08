#include "reversi/engine/Tree.h"
#include <iostream>

namespace Reversi {

  Node::Node(const State &state)
    : state(state), metric(INT32_MIN), optimal() {}

  int32_t Node::getMetric() const {
    return this->metric;
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

  std::optional<ChildNode> Node::build(std::size_t depth, const Strategy &strategy) {
    this->traverse(depth, INT16_MIN, INT16_MAX, this->state.getPlayer() == Player::White ? 1 : -1, false, strategy);
    return this->optimal;
  }

  std::optional<ChildNode> Node::build(std::size_t depth, const Strategy &strategy, FixedThreadPool &pool) {
    this->traverse(depth, INT16_MIN, INT16_MAX, this->state.getPlayer() == Player::White ? 1 : -1, false, strategy, pool);
    return this->optimal;
  }

  std::pair<int32_t, Node *> Node::traverse(std::size_t depth, int32_t alpha, int32_t beta, int color, bool abortOnNoMoves, const Strategy &strategy) {
    BoardReduceFunction reduce = static_cast<int>(Player::White) == color ? strategy.white : strategy.black;
    if (depth == 0) {
      return this->zeroDepth(color, reduce);
    } else {
      std::vector<Position> moves;
      this->state.getBoard().getMoves(moves, this->state.getPlayer());
      if (moves.empty()) {
        return this->noMoves(depth, alpha, beta, color, abortOnNoMoves, strategy);
      }

      this->metric = INT32_MIN;
      Move bestMove;
      Node *best_child = nullptr;
      State base(this->state);
      for (Position position : moves) {
        base.apply(position);
        auto child_best = this->addChild(position, base, depth, alpha, beta, color, strategy);
        if (child_best) {
          std::tie(bestMove, best_child) = child_best.value();
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

  std::pair<int32_t, Node *> Node::traverse(std::size_t depth, int32_t alpha, int32_t beta, int color, bool abortOnNoMoves, const Strategy &strategy, FixedThreadPool &pool) {
    BoardReduceFunction reduce = static_cast<int>(Player::White) == color ? strategy.white : strategy.black;
    if (depth == 0) {
      return this->zeroDepth(color, reduce);
    } else {
      std::vector<Position> moves;
      this->state.getBoard().getMoves(moves, this->state.getPlayer());
      if (moves.empty()) {
        return this->noMoves(depth, alpha, beta, color, abortOnNoMoves, strategy);
      }

      std::vector<std::future<std::unique_ptr<Node>>> nodeFutures;
      this->generateFutures(moves, nodeFutures, depth, alpha, beta, color, strategy, pool);
      

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


  std::pair<int32_t, Node *> Node::zeroDepth(int color, BoardReduceFunction reduce) {
    this->metric = color * this->state.getBoard().getMetric(reduce);
    return std::make_pair(this->metric, this);
  }

  std::pair<int32_t, Node *> Node::noMoves(std::size_t depth, int32_t alpha, int32_t beta, int color, bool abortOnNoMoves, const Strategy &strategy) {
    BoardReduceFunction reduce = static_cast<int>(Player::White) == color ? strategy.white : strategy.black;
    if (abortOnNoMoves) {
      this->metric = color * this->state.getBoard().getMetric(reduce);
      return std::make_pair(this->metric, this);
    } else {
      State base(this->state);
      base.next();
      std::unique_ptr<Node> child = std::make_unique<Node>(base);
      this->metric = -child->traverse(depth - 1, -beta, -alpha, -color, true, strategy).first;
      this->optimal = std::make_pair(Move(), child.get());
      this->children.push_back(std::make_pair(Move(), std::move(child))); // TODO
      return std::make_pair(this->metric, this->optimal.value().second);
    }
  }

  std::optional<std::pair<Position, Node *>> Node::addChild(Position position, const State &base, std::size_t depth, int32_t alpha, int32_t beta, int color, const Strategy &strategy) {
    std::optional<std::pair<Position, Node *>> best;
    std::unique_ptr<Node> child = std::make_unique<Node>(base);
    int32_t child_metric = -child->traverse(depth - 1, -beta, -alpha, -color, false, strategy).first;
    if (child_metric > this->metric) {
      this->metric = child_metric;
      best = std::make_pair(position, child.get());
    }
    this->children.push_back(std::make_pair(position, std::move(child)));
    return best;
  }

  void Node::generateFutures(std::vector<Position> &moves, std::vector<std::future<std::unique_ptr<Node>>> &nodeFutures,
    std::size_t depth, int32_t alpha, int32_t beta, int color, const Strategy &strategy, FixedThreadPool &pool) {
    for (Position position : moves) {
      State base(this->state);
      base.apply(position);
      nodeFutures.push_back(pool.submit([&, base, depth, alpha, beta, color, strategy]() {
        std::unique_ptr<Node> child = std::make_unique<Node>(base);
        child->traverse(depth - 1, -beta, -alpha, -color, false, strategy);
        return child;
      }));
    }
  }

  std::optional<std::pair<Position, Node *>> Node::addChild(std::future<std::unique_ptr<Node>> nodeFuture, Position position) {
    std::optional<std::pair<Position, Node *>> best;
    std::unique_ptr<Node> child = nodeFuture.get();
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