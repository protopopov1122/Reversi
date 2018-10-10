#ifndef REVERSI_ENGINE_ENGINE_H_
#define REVERSI_ENGINE_ENGINE_H_

#include "reversi/engine/Tree.h"
#include <algorithm>


namespace Reversi {

  using PlayerMove = std::pair<Player, Position>;

  template <typename T>
  class EventListener {
   public:
    virtual ~EventListener() = default;
    virtual void receiveEvent(const T &) = 0;
  };

  template <typename T>
  class FunctionEventListener : public EventListener<T> {
   public:
    FunctionEventListener()
      : callback([](const T &t) {}) {}
    FunctionEventListener(std::function<void (const T &)> fn)
      : callback(fn) {}

    void setCallback(std::function<void (const T &)> fn) {
      this->callback = fn;
    }
    void receiveEvent(const T &evt) override {
      this->callback(evt);
    }
   private:
    std::function<void(const T &)> callback;
  };

  template <typename T>
  class EventSource {
   public:
    virtual ~EventSource() = default;
    void addEventListener(EventListener<T> &listener) {
      this->listeners.push_back(&listener);
    }
    void removeEventListener(EventListener<T> &listener) {
      this->listeners.erase(std::remove(this->listeners.begin(), this->listeners.end(), &listener), this->listeners.end());
    }
   protected:
    void triggerEvent(const T &value) {
      for (const auto &listener : this->listeners) {
        listener->receiveEvent(value);
      }
    }
   private:
    std::vector<EventListener<T> *> listeners;
  };

  class GameEngine : public EventListener<PlayerMove>, public EventSource<State> {
   public:
    GameEngine();
    GameEngine(const State &);
    virtual ~GameEngine() = default;
    void setState(const State &);
    const State &getState() const;
   protected:
    void stateUpdated();
  
    State state;
  };

  class DefaultGameEngine : public GameEngine {
   public:
    DefaultGameEngine();
    DefaultGameEngine(const State &);
    void receiveEvent(const PlayerMove &) override;
   protected:
    bool hasMoves(Player) const;
    bool isMovePossible(Player, Position) const;
    bool isGameFinished() const;
  };
}

#endif