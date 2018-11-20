#ifndef REVERSI_ENGINE_LIBRARY_H_
#define REVERSI_ENGINe_LIBRARY_H_

#include "reversi/engine/State.h"
#include <map>
#include <optional>
#include <vector>
#include <initializer_list>

namespace Reversi {

  class StateBuilder {
   public:
    StateBuilder();
    StateBuilder(const State &);
    StateBuilder &apply(Position);
    StateBuilder &apply(char, unsigned int);
    operator const State &() {
      return this->state;
    };
   private:
    State state;
  };

  class MoveLibrary {
   public:
    bool hasMove(const State &) const;
    std::optional<Position> getMove(const State &) const;
   protected:
    void addMoves(const State &, std::initializer_list<Position>, bool);
    void addMove(const State &, Position);
   private:
    std::map<State, std::vector<Position>> library;
  };

  class OpeningLibrary : public MoveLibrary {
   public:
    OpeningLibrary();
    static const OpeningLibrary Openings;
  };
}

#endif