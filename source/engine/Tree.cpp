#include "reversi/engine/Tree.h"
#include <iostream>

namespace Reversi {

  Node::Node(const State &state)
    : state(state), metric(INT32_MIN), optimal(nullptr) {}

  int32_t Node::getMetric() const {
    return this->metric;
  }

  const State &Node::getState() const {
    return this->state;
  }

  bool Node::isTerminal() const {
    return this->children.empty();
  }

  void Node::getChildren(std::vector<std::pair<Position, Node *>> &children) const {
    for (const auto &child : this->children) {
      children.push_back(std::make_pair(child.first, child.second.get()));
    }
  }

  std::optional<std::pair<Position, Node *>> Node::getOptimalChild() const {
    std::optional<std::pair<Position, Node *>> result;
    for (const auto &child : this->children) {
      if (child.second.get() == this->optimal) {
        result = std::make_pair(child.first, child.second.get());
        break;
      }
    }
    return result;
  }

  std::optional<std::pair<Position, Node *>> Node::build(std::size_t depth) {
    Node *best_child = this->traverse(depth, INT16_MIN, INT16_MAX, this->state.getPlayer() == Player::White ? 1 : -1, false).second;
    std::optional<std::pair<Position, Node *>> result;
    for (const auto &child : this->children) {
      if (child.second.get() == best_child) {
        result = std::make_pair(child.first, child.second.get());
        break;
      }
    }
    return result;
  }

  std::pair<int32_t, Node *> Node::traverse(std::size_t depth, int32_t alpha, int32_t beta, int color, bool abortOnNoMoves) {
    if (depth == 0) {
      this->metric = color * this->state.getBoard().getPlainMetric();
      return std::make_pair(this->metric, this);
    } else {
      std::vector<Position> moves;
      this->state.getBoard().getMoves(moves, this->state.getPlayer());
      if (moves.empty()) {  // TODO
        this->metric = color * this->state.getBoard().getPlainMetric();
        return std::make_pair(this->metric, this);
      }

      this->metric = INT32_MIN;
      Node *best_child = nullptr;
      State base(this->state);
      for (Position position : moves) {
        base.apply(position);
        std::unique_ptr<Node> child = std::make_unique<Node>(base);
        base = this->state;
        int32_t child_metric = -child->traverse(depth - 1, -beta, -alpha, -color, false).first;
        if (child_metric > this->metric) {
          this->metric = child_metric;
          best_child = child.get();
        }
        this->children.push_back(std::make_pair(position, std::move(child)));
        
        if (this->metric > alpha) {
          alpha = this->metric;
        }
        if (alpha >= beta) {
          break;
        }
      }
      if (best_child) {
        this->optimal = best_child;
      }
      return std::make_pair(this->metric, best_child);
    }
  }

  std::ostream &Node::dump(std::ostream &os, const Node &root, std::string separator, std::string prefix) {
    std::vector<std::pair<Position, Node *>> children;
    root.getChildren(children);
    for (const auto &child : children) {
      os << prefix << child.first << " " << child.second->getMetric() << std::endl;
      Node::dump(os, *child.second, separator, prefix + separator);
    }
    return os;
  }

  std::ostream &operator<<(std::ostream &os, const Node &root) {
    std::optional<std::pair<Position, Node *>> optimal = root.getOptimalChild();
    if (optimal) {
      os << optimal.value().first << " " << root.getMetric() << std::endl;
    }
    return Node::dump(os, root, "\t", "\t");
  }
}