#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

// in milliseconds
#define SHORTEST_LIGHT_CHANGE_TIME 4000
#define LONGEST_LIGHT_CHANGE_TIME 6000

#include <mutex>
#include <thread>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Vehicle;

enum TrafficLightPhase {
    kRed, kGreen
};

template <class T>
class MessageQueue
{
public:
    T receive();
    void send(T &&msg);

private:
    std::mutex _mutex;
    std::condition_variable _cond;
    std::deque<T> _queue;
};

class TrafficLight : public TrafficObject
{
public:
    // constructor
    TrafficLight();

    // getters / setters
    TrafficLightPhase getCurrentPhase() { return _currentPhase; } 
    void setPhase(TrafficLightPhase phase) { _currentPhase = phase; }

    // typical behaviour methods
    void togglePhase();
    void waitForGreen();
    void simulate();

private:
    // typical behaviour methods
    void cycleThroughPhases();

    // private members
    std::condition_variable _condition;
    std::mutex _mutex;

    std::unique_ptr<MessageQueue<TrafficLightPhase>> _que;
    TrafficLightPhase _currentPhase;
};

#endif