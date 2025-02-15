#include "io/RotaryControls.hpp"

#include "grid/GridDriver.hpp"
#include "ThreadConfigurations.h"

#include "ticks.hpp"

#include <cstdlib>

static const uint8_t kNumberOfControls = 2;
static const uint8_t kNumberOfTimeSteps = 10;
static const int8_t kNumberOfMicrostepsInStep = 4;
static const uint8_t kInputEventQueueSize = 8;

static const int8_t kEncoderState[16] = { 0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0,-1, 1, 0 };

RotaryControls::RotaryControls( grid::GridDriver& gridDriver ) :
        Thread( "RotaryControls", kRotaryControls.stackDepth, kRotaryControls.priority ),
        gridDriver_( gridDriver ),
        inputEvents_( freertos::Queue( kInputEventQueueSize, sizeof( Event ) ) )
{
    gridDriver_.addThreadToNotify( this );
    Start();
}

RotaryControls::~RotaryControls()
{
}

void RotaryControls::discardAllPendingEvents()
{
    inputEvents_.Flush();
}

bool RotaryControls::waitForEvent( Event& event )
{
    const bool eventAvailable = inputEvents_.Dequeue( &event ); // block until event
    return eventAvailable;
}

void RotaryControls::Run()
{
    while (true)
    {
        WaitForNotification();
        for (uint8_t encoder = 0; encoder < kNumberOfControls; encoder++)
        {
            static int8_t microstep[2] = {0, 0};
            static uint8_t previousEncoderValue[2] = {0, 0};

            for (uint8_t timeStep = 0; timeStep < kNumberOfTimeSteps; timeStep++)
            {
                previousEncoderValue[encoder] <<= 2;
                previousEncoderValue[encoder] |= gridDriver_.getRotaryEncodersInput( encoder, timeStep );
                previousEncoderValue[encoder] &= 0x0F;
                microstep[encoder] += kEncoderState[previousEncoderValue[encoder]];
            }

            // only respond every 4 microsteps (1 physical step)
            if (std::abs( microstep[encoder] ) >= kNumberOfMicrostepsInStep)
            {
                static uint32_t previousEventTime[2] = {0, 0};
                const uint32_t interval = freertos::Ticks::TicksToMs( freertos::Ticks::GetTicks() ) - previousEventTime[encoder];
                const int8_t velocityMultiplier = calculateVelocityMultiplier( interval );

                Event event = {};
                event.steps = (microstep[encoder] / kNumberOfMicrostepsInStep) * velocityMultiplier;
                event.control = encoder;
                microstep[encoder] %= kNumberOfMicrostepsInStep;
                
                inputEvents_.Enqueue( &event );
            }
        }
    }
}

int8_t RotaryControls::calculateVelocityMultiplier( const uint32_t intervalMs ) const
{
    uint32_t multiplier = 0;
    if (intervalMs > 500)
    {
        multiplier = 1;
    }
    else if (intervalMs > 200)
    {
        multiplier = 2;
    }
    else if (intervalMs > 50)
    {
        multiplier = 4;
    }
    else
    {
        multiplier = 8;
    }

    return multiplier;
}
