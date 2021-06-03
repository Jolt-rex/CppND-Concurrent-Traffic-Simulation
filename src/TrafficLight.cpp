#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */
template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> u_lock(_mutex);
    _cond.wait(u_lock, [this] { return !_queue.empty(); });

    T msg = std::move(_queue.back());
    _queue.pop_back();
    return std::move(msg);
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> u_lock(_mutex);

    // clear the que first for intersections that are not used often
    _queue.clear();

    // add message to the que
    _queue.emplace_back(msg);
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */
// Initialise unique pointer to the message que, and start traffic phase as red
TrafficLight::TrafficLight()
{
    _que = std::make_unique<MessageQueue<TrafficLightPhase>>();
    _currentPhase = TrafficLightPhase::kRed;
}

void TrafficLight::togglePhase() {
    if(_currentPhase == TrafficLightPhase::kRed) { _currentPhase = TrafficLightPhase::kGreen; }
    else { _currentPhase = TrafficLightPhase::kRed; }
}

// function will block flow of execution until response from the _que is received
void TrafficLight::waitForGreen()
{
    while(true) {
        TrafficLightPhase current_phase = _que->receive();
        if(current_phase == TrafficLightPhase::kGreen) return;
    }
}

// add a new thread to process the light cycle of this traffic light
// thread is added to base class vector, which also handles the thread join
void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // get a random duration for this traffic light cycle
    int duration = rand()%(LONGEST_LIGHT_CHANGE_TIME - SHORTEST_LIGHT_CHANGE_TIME + 1) + SHORTEST_LIGHT_CHANGE_TIME;

    auto t_start = std::chrono::high_resolution_clock::now();
    auto t_now = std::chrono::high_resolution_clock::now();
    auto t_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t_now - t_start).count();

    while(true) {
        // wait 1 millisecond to reduce load on CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        t_now = std::chrono::high_resolution_clock::now();

        // calculate the elapsed time between loop iterations
        t_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t_now - t_start).count();

        // if we have passed the random duration time, toggle the phase
        // and send the current phase to the que
        // reset the start time for the next phase cycle
        if(t_elapsed > duration) {
            togglePhase();
            _que->send(std::move(_currentPhase));

            t_start = std::chrono::high_resolution_clock::now();
        }
    }
}