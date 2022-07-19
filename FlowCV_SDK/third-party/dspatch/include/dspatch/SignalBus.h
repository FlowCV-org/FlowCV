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

#include <dspatch/Signal.h>

#include <vector>

namespace DSPatch
{

namespace internal
{
class SignalBus;
}

/// Signal container

/**
A SignalBus contains Signals (see Signal). Via the Process_() method, a Component receives signals
into it's "inputs" SignalBus and provides signals to it's "outputs" SignalBus. The SignalBus class
provides public getters and setters for manipulating it's internal Signal values directly,
abstracting the need to retrieve and interface with the contained Signals themself.
*/

class DLLEXPORT SignalBus final
{
public:
    NONCOPYABLE( SignalBus );
    DEFINE_PTRS( SignalBus );

    SignalBus();
    SignalBus( SignalBus&& );
    ~SignalBus();

    void SetSignalCount( int signalCount );
    [[nodiscard]] int GetSignalCount() const;

    [[nodiscard]] Signal::SPtr const& GetSignal( int signalIndex ) const;

    [[nodiscard]] bool HasValue( int signalIndex ) const;

    template <class ValueType>
    ValueType* GetValue( int signalIndex ) const;

    template <class ValueType>
    bool SetValue( int signalIndex, ValueType const& newValue );

    bool CopySignal( int toSignalIndex, Signal::SPtr const& fromSignal );
    bool MoveSignal( int toSignalIndex, Signal::SPtr const& fromSignal );

    void ClearAllValues();

    [[nodiscard]] std::type_info const& GetType( int signalIndex ) const;

private:
    std::vector<Signal::SPtr> _signals;

    std::unique_ptr<internal::SignalBus> p;
};

template <class ValueType>
ValueType* SignalBus::GetValue( int signalIndex ) const
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        return _signals[signalIndex]->GetValue<ValueType>();
    }
    else
    {
        return nullptr;
    }
}

template <class ValueType>
bool SignalBus::SetValue( int signalIndex, ValueType const& newValue )
{
    if ( (size_t)signalIndex < _signals.size() )
    {
        _signals[signalIndex]->SetValue( newValue );
        return true;
    }
    else
    {
        return false;
    }
}

}  // namespace DSPatch
