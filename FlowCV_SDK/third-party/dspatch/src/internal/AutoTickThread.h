/******************************************************************************
DSPatch - The Refreshingly Simple C++ Dataflow Framework
Copyright (c) 2021, Marcus Tomlinson

BSD 2-Clause License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#pragma once

#include <dspatch/Circuit.h>
#include <dspatch/Common.h>

#include <condition_variable>
#include <thread>

namespace DSPatch
{
namespace internal
{

/// Thread class for auto-ticking a circuit

/**
An AutoTickThread is responsible for ticking a circuit continuously in a free-running thread. Upon
initialisation, a reference to the circuit must be provided for the thread's _Run() method to use.
Once Start() has been called, the thread will begin, repeatedly calling the circuit's Tick()
method until instructed to Pause() or Stop().
*/

class AutoTickThread final
{
public:
    NONCOPYABLE( AutoTickThread );
    DEFINE_PTRS( AutoTickThread );

    AutoTickThread();
    ~AutoTickThread();

    DSPatch::Component::TickMode Mode();

    [[nodiscard]] bool IsStopped() const;
    [[nodiscard]] bool IsPaused() const;

    void Start( DSPatch::Circuit* circuit, DSPatch::Component::TickMode mode );
    void Stop();
    void Pause();
    void Resume();

private:
    void _Run();

private:
    DSPatch::Component::TickMode _mode;
    std::thread _thread;
    DSPatch::Circuit* _circuit = nullptr;
    bool _stop = false;
    bool _pause = false;
    bool _stopped = true;
    std::mutex _resumeMutex;
    std::condition_variable _resumeCondt, _pauseCondt;
};

}  // namespace internal
}  // namespace DSPatch
