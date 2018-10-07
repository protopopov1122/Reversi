#ifndef REVERSI_ENGINE_TREE_H_
#define REVERSI_ENGINE_TREE_H_

#include <optional>
#include <memory>
#include <iosfwd>
#include "reversi/engine/State.h"

namespace Reversi {

  class Node; // Forward referencing

  using Move = std::optional<Position>;
  using ChildNode = std::pair<Move, Node *>;
  using ChildNodeUnique = std::pair<Move, std::unique_ptr<Node>>;

  struct Strategy {
    BoardReduceFunction white;
    BoardReduceFunction black;
  };

  class Node {
   public:
    Node(const State &);
    std::optional<ChildNode> build(std::size_t, const Strategy &);
    int32_t getMetric() const;
    const State &getState() const;
    bool isTerminal() const;
    bool isBuilt() const;
    std::optional<ChildNode> getOptimalChild() const;
    void getChildren(std::vector<ChildNode> &) const;

    friend std::ostream &operator<<(std::ostream &, const Node &);
    static std::ostream &dump(std::ostream &, const Node &, std::string = "\t", std::string = "");
   protected:
    std::pair<int32_t, Node *> traverse(std::size_t, int32_t, int32_t, int, bool, const Strategy &);
   private:
    State state;
    int32_t metric;
    std::vector<ChildNodeUnique> children;
    Node *optimal;
  };
}

#endif