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

#include <dspatch/Component.h>

#include <internal/ComponentThread.h>
#include <internal/Wire.h>

#include <condition_variable>
#include <unordered_set>
#include <utility>

using namespace DSPatch;

namespace DSPatch
{
namespace internal
{

class Component
{
public:
    enum class TickStatus
    {
        NotTicked,
        TickStarted,
        Ticking
    };

    Component( DSPatch::Component::ProcessOrder processOrder )
        : processOrder( processOrder )
    {
    }

    void WaitForRelease( int threadNo );
    void ReleaseThread( int threadNo );

    void GetOutput( int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus, DSPatch::Component::TickMode mode );

    void IncRefs( int output );
    void DecRefs( int output );

    const DSPatch::Component::ProcessOrder processOrder;

    int bufferCount = 0;

    std::vector<DSPatch::SignalBus> inputBuses;
    std::vector<DSPatch::SignalBus> outputBuses;

    std::vector<std::vector<std::pair<int, int>>> refs;  // ref_total:ref_counter per output, per buffer
    std::vector<std::vector<std::unique_ptr<std::mutex>>> refMutexes;

    std::vector<Wire> inputWires;

    std::vector<ComponentThread::UPtr> componentThreads;
    std::vector<std::unordered_set<Wire*>> feedbackWires;

    std::vector<TickStatus> tickStatuses;
    std::vector<bool> gotReleases;
    std::vector<std::unique_ptr<std::mutex>> releaseMutexes;
    std::vector<std::unique_ptr<std::condition_variable>> releaseCondts;

    std::vector<std::string> inputNames;
    std::vector<std::string> outputNames;
    std::vector<IoType> inputTypes;
    std::vector<IoType> outputTypes;
};

}  // namespace internal
}  // namespace DSPatch

Component::Component( ProcessOrder processOrder )
    : p( new internal::Component( processOrder ) )
{
    isEnabled_ = true;
    SetBufferCount( 1 );
    instance_name_ = name_;
    instance_name_ += std::to_string(instance_count_);
}

Component::~Component()
{
    DisconnectAllInputs();
}

bool Component::ConnectInput( Component::SPtr const& fromComponent, int fromOutput, int toInput )
{
    if ( fromOutput >= fromComponent->GetOutputCount() || toInput >= p->inputBuses[0].GetSignalCount() )
    {
        return false;
    }

    // first make sure there are no wires already connected to this input
    DisconnectInput( toInput );

    p->inputWires.emplace_back( fromComponent, fromOutput, toInput );

    // update source output's reference count
    fromComponent->p->IncRefs( fromOutput );

    return true;
}

void Component::DisconnectInput( int inputNo )
{
    // remove wires connected to inputNo from inputWires
    for ( auto it = p->inputWires.begin(); it != p->inputWires.end(); ++it )
    {
        if ( it->toInput == inputNo )
        {
            // update source output's reference count
            it->fromComponent->p->DecRefs( it->fromOutput );

            p->inputWires.erase( it );
            break;
        }
    }
}

void Component::DisconnectInput( Component::SCPtr const& fromComponent )
{
    // remove fromComponent from inputWires
    for ( auto it = p->inputWires.begin(); it != p->inputWires.end(); )
    {
        if ( it->fromComponent == fromComponent )
        {
            // update source output's reference count
            fromComponent->p->DecRefs( it->fromOutput );

            it = p->inputWires.erase( it );
        }
        else
        {
            ++it;
        }
    }
}

void Component::DisconnectAllInputs()
{
    // remove all wires from inputWires
    for ( int i = 0; i < p->inputBuses[0].GetSignalCount(); ++i )
    {
        DisconnectInput( i );
    }
}

int Component::GetInputCount() const
{
    return p->inputBuses[0].GetSignalCount();
}

int Component::GetOutputCount() const
{
    return p->outputBuses[0].GetSignalCount();
}

std::string Component::GetInputName( int inputNo ) const
{
    if ( inputNo < (int)p->inputNames.size() )
    {
        return p->inputNames[inputNo];
    }
    return "";
}

std::string Component::GetOutputName( int outputNo ) const
{
    if ( outputNo < (int)p->outputNames.size() )
    {
        return p->outputNames[outputNo];
    }
    return "";
}

IoType Component::GetInputType( int inputNo ) const
{
    if ( inputNo < (int)p->inputTypes.size() )
    {
        return p->inputTypes[inputNo];
    }
    return IoType::Io_Type_Unspecified;
}

IoType Component::GetOutputType( int outputNo ) const
{
    if ( outputNo < (int)p->outputTypes.size() )
    {
        return p->outputTypes[outputNo];
    }
    return IoType::Io_Type_Unspecified;
}

void Component::SetBufferCount( int bufferCount )
{
    // p->bufferCount is the current thread count / bufferCount is new thread count

    if ( bufferCount <= 0 )
    {
        bufferCount = 1;  // there needs to be at least 1 buffer
    }

    // resize vectors
    p->componentThreads.resize( bufferCount );
    p->feedbackWires.resize( bufferCount );

    p->tickStatuses.resize( bufferCount );

    p->inputBuses.resize( bufferCount );
    p->outputBuses.resize( bufferCount );

    p->gotReleases.resize( bufferCount );
    p->releaseMutexes.resize( bufferCount );
    p->releaseCondts.resize( bufferCount );

    p->refs.resize( bufferCount );
    p->refMutexes.resize( bufferCount );

    // init new vector values
    for ( int i = p->bufferCount; i < bufferCount; ++i )
    {
        p->componentThreads[i] = std::unique_ptr<internal::ComponentThread>( new internal::ComponentThread() );

        p->tickStatuses[i] = internal::Component::TickStatus::NotTicked;

        p->inputBuses[i].SetSignalCount( p->inputBuses[0].GetSignalCount() );
        p->outputBuses[i].SetSignalCount( p->outputBuses[0].GetSignalCount() );

        p->gotReleases[i] = false;
        p->releaseMutexes[i] = std::unique_ptr<std::mutex>( new std::mutex() );
        p->releaseCondts[i] = std::unique_ptr<std::condition_variable>( new std::condition_variable() );

        p->refs[i].resize( p->refs[0].size() );
        for ( size_t j = 0; j < p->refs[0].size(); ++j )
        {
            // sync output reference counts
            p->refs[i][j] = p->refs[0][j];
        }

        p->refMutexes[i].resize( p->refMutexes[0].size() );
        for ( size_t j = 0; j < p->refs[0].size(); ++j )
        {
            // construct new output reference mutexes
            p->refMutexes[i][j] = std::unique_ptr<std::mutex>( new std::mutex() );
        }
    }

    p->gotReleases[0] = true;

    p->bufferCount = bufferCount;
}

int Component::GetBufferCount() const
{
    return p->inputBuses.size();
}

bool Component::Tick( Component::TickMode mode, int bufferNo )
{
    // continue only if this component has not already been ticked
    if ( p->tickStatuses[bufferNo] == internal::Component::TickStatus::NotTicked )
    {
        // 1. set tickStatus -> TickStarted
        p->tickStatuses[bufferNo] = internal::Component::TickStatus::TickStarted;

        // 2. tick incoming components
        for ( auto& wire : p->inputWires )
        {
            if ( mode == TickMode::Series )
            {
                wire.fromComponent->Tick( mode, bufferNo );
            }
            else if ( mode == TickMode::Parallel )
            {
                if ( !wire.fromComponent->Tick( mode, bufferNo ) )
                {
                    p->feedbackWires[bufferNo].emplace( &wire );
                }
            }
        }

        // 3. set tickStatus -> Ticking
        p->tickStatuses[bufferNo] = internal::Component::TickStatus::Ticking;

        auto tick = [this, mode, bufferNo]() {
            // 4. get new inputs from incoming components
            for ( auto& wire : p->inputWires )
            {
                if ( mode == TickMode::Parallel )
                {
                    // wait for non-feedback incoming components to finish ticking
                    auto wireIndex = p->feedbackWires[bufferNo].find( &wire );
                    if ( wireIndex == p->feedbackWires[bufferNo].end() )
                    {
                        wire.fromComponent->p->componentThreads[bufferNo]->Sync();
                    }
                    else
                    {
                        p->feedbackWires[bufferNo].erase( wireIndex );
                    }
                }

                wire.fromComponent->p->GetOutput( bufferNo, wire.fromOutput, wire.toInput, p->inputBuses[bufferNo], mode );
            }

            // You might be thinking: Why not clear the outputs in Reset()?

            // This is because we need components to hold onto their outputs long enough for any
            // loopback wires to grab them during the next tick. The same applies to how we handle
            // output reference counting in internal::Component::GetOutput(), reseting the counter upon
            // the final request rather than in Reset().

            // 5. clear outputs
            p->outputBuses[bufferNo].ClearAllValues();

            if ( p->processOrder == ProcessOrder::InOrder && p->bufferCount > 1 )
            {
                // 6. wait for our turn to process
                p->WaitForRelease( bufferNo );

                // 7. call Process_() with newly aquired inputs
                Process_( p->inputBuses[bufferNo], p->outputBuses[bufferNo] );

                // 8. signal that we're done processing
                p->ReleaseThread( bufferNo );
            }
            else
            {
                // 6. call Process_() with newly aquired inputs
                Process_( p->inputBuses[bufferNo], p->outputBuses[bufferNo] );
            }
        };

        // do tick
        if ( mode == TickMode::Series )
        {
            tick();
        }
        else if ( mode == TickMode::Parallel )
        {
            p->componentThreads[bufferNo]->Resume( tick );
        }
    }
    else if ( p->tickStatuses[bufferNo] == internal::Component::TickStatus::TickStarted )
    {
        // return false to indicate that we have already started a tick, and hence, are a feedback component.
        return false;
    }

    // return true to indicate that we are now in "Ticking" state.
    return true;
}

void Component::Reset( int bufferNo )
{
    // wait for ticking to complete
    p->componentThreads[bufferNo]->Sync();

    // clear inputs
    p->inputBuses[bufferNo].ClearAllValues();

    // reset tickStatus
    p->tickStatuses[bufferNo] = internal::Component::TickStatus::NotTicked;
}

void Component::SetInputCount_( int inputCount, std::vector<std::string> const& inputNames, std::vector<IoType> const& inputTypes )
{
    p->inputNames = inputNames;
    p->inputTypes = inputTypes;

    for ( auto& inputBus : p->inputBuses )
    {
        inputBus.SetSignalCount( inputCount );
    }
}

void Component::SetOutputCount_( int outputCount, std::vector<std::string> const& outputNames, std::vector<IoType> const& outputTypes )
{
    p->outputNames = outputNames;
    p->outputTypes = outputTypes;

    for ( auto& outputBus : p->outputBuses )
    {
        outputBus.SetSignalCount( outputCount );
    }

    // add reference counters for our new outputs
    for ( auto& ref : p->refs )
    {
        ref.resize( outputCount );
    }
    for ( auto& refMutexes : p->refMutexes )
    {
        refMutexes.resize( outputCount );
        for ( auto& refMutex : refMutexes )
        {
            // construct new output reference mutexes
            if ( !refMutex )
            {
                refMutex = std::unique_ptr<std::mutex>( new std::mutex() );
            }
        }
    }
}

void Component::SetComponentName_(std::string component_name)
{
    name_ = std::move(component_name);
    instance_name_ = name_;
    instance_name_ += std::to_string(instance_count_);
}

void Component::SetComponentCategory_(Category component_category)
{
    category_ = component_category;
}

void Component::SetComponentAuthor_(std::string component_author)
{
    author_ = std::move(component_author);
}

void Component::SetComponentVersion_(std::string component_version)
{
    version_ = std::move(component_version);
}

void Component::SetInstanceCount(int num) {
    instance_count_ = num;
    instance_name_ = name_;
    instance_name_ += std::to_string(instance_count_);
}

std::string Component::GetComponentName() const
{
    return name_;
}

const char* Component::GetInstanceName() const
{
    return instance_name_.c_str();
}

Category Component::GetComponentCategory() const
{
    return category_;
}

std::string Component::GetComponentAuthor() const
{
    return author_;
}

std::string Component::GetComponentVersion() const
{
    return version_;
}

int Component::GetInstanceCount() const
{
    return instance_count_;
}

void Component::SetEnabled(bool enabled)
{
    isEnabled_ = enabled;
}

bool Component::IsEnabled()
{
    return isEnabled_;
}

void internal::Component::WaitForRelease( int threadNo )
{
    std::unique_lock<std::mutex> lock( *releaseMutexes[threadNo] );

    if ( !gotReleases[threadNo] )
    {
        releaseCondts[threadNo]->wait( lock );  // wait for resume
    }
    gotReleases[threadNo] = false;  // reset the release flag
}

void internal::Component::ReleaseThread( int threadNo )
{
    threadNo = threadNo + 1 == bufferCount ? 0 : threadNo + 1;  // we're actually releasing the next available thread

    std::lock_guard<std::mutex> lock( *releaseMutexes[threadNo] );

    gotReleases[threadNo] = true;
    releaseCondts[threadNo]->notify_all();
}

void internal::Component::GetOutput(
    int bufferNo, int fromOutput, int toInput, DSPatch::SignalBus& toBus, DSPatch::Component::TickMode mode )
{
    if ( !outputBuses[bufferNo].HasValue( fromOutput ) )
    {
        return;
    }

    auto& signal = outputBuses[bufferNo].GetSignal( fromOutput );

    if ( mode == DSPatch::Component::TickMode::Parallel && refs[bufferNo][fromOutput].first > 1 )
    {
        refMutexes[bufferNo][fromOutput]->lock();
    }

    if ( ++refs[bufferNo][fromOutput].second == refs[bufferNo][fromOutput].first )
    {
        // this is the final reference, reset the counter, move the signal
        refs[bufferNo][fromOutput].second = 0;

        toBus.MoveSignal( toInput, signal );
    }
    else
    {
        // otherwise, copy the signal
        toBus.CopySignal( toInput, signal );
    }

    if ( mode == DSPatch::Component::TickMode::Parallel && refs[bufferNo][fromOutput].first > 1 )
    {
        refMutexes[bufferNo][fromOutput]->unlock();
    }
}

void internal::Component::IncRefs( int output )
{
    for ( auto& ref : refs )
    {
        ++ref[output].first;
    }
}

void internal::Component::DecRefs( int output )
{
    for ( auto& ref : refs )
    {
        --ref[output].first;
    }
}
