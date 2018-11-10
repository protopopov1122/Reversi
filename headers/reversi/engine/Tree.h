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
   private:
    std::shared_mutex cacheMutex;
    std::map<State, std::shared_ptr<Node>> cache;
  };

  class Node {
   public:
    Node(const State &);
    std::optional<ChildNode> build(std::size_t, const Strategy &, bool = false, bool = true);
    std::optional<ChildNode> build(std::size_t, const Strategy &, FixedThreadPool &, bool = false, bool = true);
    int32_t getMetric() const;
    std::size_t getDepth() const;
    const State &getState() const;
    std::optional<ChildNode> getOptimalChild() const;

    friend std::ostream &operator<<(std::ostream &, const Node &);
    static std::ostream &dump(std::ostream &, const Node &, std::string = "\t", std::string = "");
   protected:
    const std::vector<ChildNode> &getChildren() const;
    int32_t traverse(std::size_t, int32_t, int32_t, int, bool, const Strategy &, std::shared_ptr<NodeCache> = nullptr, bool = true);
    int32_t traverse(std::size_t, int32_t, int32_t, int, bool, const Strategy &, FixedThreadPool &, std::shared_ptr<NodeCache> = nullptr, bool = true);
   private:
    int32_t zeroDepth(int, std::function<int32_t (const State &)>);
    int32_t noMoves(std::size_t, int32_t, int32_t, int, bool, const Strategy &, std::shared_ptr<NodeCache>);
    std::optional<ChildNode> addChild(Position, const State &, std::size_t, int32_t, int32_t, int, const Strategy &, std::shared_ptr<NodeCache>);
    void generateFutures(std::vector<Position> &, std::vector<std::future<std::shared_ptr<Node>>> &,
      std::size_t, int32_t, int32_t, int, const Strategy &, FixedThreadPool &, std::shared_ptr<NodeCache>);
    std::optional<ChildNode> addChild(std::future<std::shared_ptr<Node>>, Position);
    void randomizeOptimal();

    State state;
    std::size_t depth;
    int32_t metric;
    std::vector<ChildNode> children;
    std::optional<ChildNode> optimal;
  };
}

#endif