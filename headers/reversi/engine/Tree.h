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
  using ChildNode = std::pair<Move, Node *>;
  using ChildNodeUnique = std::pair<Move, std::shared_ptr<Node>>;

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
    std::optional<ChildNode> build(std::size_t, const Strategy &, bool = true);
    std::optional<ChildNode> build(std::size_t, const Strategy &, FixedThreadPool &, bool = true);
    int32_t getMetric() const;
    std::size_t getDepth() const;
    const State &getState() const;
    bool isTerminal() const;
    bool isBuilt() const;
    std::optional<ChildNode> getOptimalChild() const;
    void getChildren(std::vector<ChildNode> &) const;

    friend std::ostream &operator<<(std::ostream &, const Node &);
    static std::ostream &dump(std::ostream &, const Node &, std::string = "\t", std::string = "");
   protected:
    std::pair<int32_t, Node *> traverse(std::size_t, int32_t, int32_t, int, bool, const Strategy &, NodeCache * = nullptr);
    std::pair<int32_t, Node *> traverse(std::size_t, int32_t, int32_t, int, bool, const Strategy &, FixedThreadPool &, NodeCache * = nullptr);
   private:
    std::pair<int32_t, Node *> zeroDepth(int, std::function<int32_t (const State &)>);
    std::pair<int32_t, Node *> noMoves(std::size_t, int32_t, int32_t, int, bool, const Strategy &, NodeCache *);
    std::optional<std::pair<Position, Node *>> addChild(Position, const State &, std::size_t, int32_t, int32_t, int, const Strategy &, NodeCache *);
    void generateFutures(std::vector<Position> &, std::vector<std::future<std::shared_ptr<Node>>> &,
      std::size_t, int32_t, int32_t, int, const Strategy &, FixedThreadPool &, NodeCache *);
    std::optional<std::pair<Position, Node *>> addChild(std::future<std::shared_ptr<Node>>, Position);

    State state;
    std::size_t depth;
    int32_t metric;
    std::vector<ChildNodeUnique> children;
    std::optional<ChildNode> optimal;
  };
}

#endif