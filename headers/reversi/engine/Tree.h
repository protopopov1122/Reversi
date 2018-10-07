#ifndef REVERSI_ENGINE_TREE_H_
#define REVERSI_ENGINE_TREE_H_

#include <optional>
#include <memory>
#include <iosfwd>
#include "reversi/engine/State.h"

namespace Reversi {

  class Node {
   public:
    Node(const State &);
    std::optional<std::pair<Position, Node *>> build(std::size_t);
    int32_t getMetric() const;
    const State &getState() const;
    bool isTerminal() const;
    std::optional<std::pair<Position, Node *>> getOptimalChild() const;
    void getChildren(std::vector<std::pair<Position, Node *>> &) const;


    friend std::ostream &operator<<(std::ostream &, const Node &);
    static std::ostream &dump(std::ostream &, const Node &, std::string = "\t", std::string = "");
   protected:
    std::pair<int32_t, Node *> traverse(std::size_t, int32_t, int32_t, int, bool);
   private:
    State state;
    int32_t metric;
    std::vector<std::pair<Position, std::unique_ptr<Node>>> children;
    Node *optimal;
  };
}

#endif