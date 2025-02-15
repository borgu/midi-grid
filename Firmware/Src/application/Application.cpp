#include "application/Application.hpp"
#include "ThreadConfigurations.h"

namespace application
{

template <class InputSource, class InputType>
InputHandler<InputSource, InputType>::InputHandler( ApplicationController& applicationController, InputSource inputSource ):
    Thread( "InputHandler", kInputSource.stackDepth, kInputSource.priority ),
    applicationController_( applicationController ),
    inputSource_( inputSource )
{
    Start();
    Suspend();
}

template <class InputSource, class InputType>
void InputHandler<InputSource, InputType>::enable()
{
    inputSource_.discardPendingInput();
    Resume();
}

template <class InputSource, class InputType>
void InputHandler<InputSource, InputType>::disable()
{
    Suspend();
}

template <class InputSource, class InputType>
void InputHandler<InputSource, InputType>::Run()
{
    while (true)
    {
        InputType input = {};

        if (inputSource_.waitForInput( input ))
        {
            applicationController_.handleInput( input );
        }
    }
}

ApplicationThread::ApplicationThread( ApplicationController& applicationController ):
    Thread( "ApplicationThread", kApplicationThread.stackDepth, kApplicationThread.priority ),
    applicationController_( applicationController )
{
    Start();
    Suspend();
}

void ApplicationThread::enable()
{
    Resume();
}

void ApplicationThread::disable()
{
    Suspend();
}

void ApplicationThread::run()
{
    continueApplication_.Give();
}

void ApplicationThread::Run()
{
    while (true)
    {
        continueApplication_.Take();
        applicationController_.runApplicationThread( *this );
    }
}

Application::Application( ApplicationController& applicationController ):
    applicationController_( applicationController )
{
}

Application::~Application()
{
}

void Application::switchApplication( const ApplicationIndex applicationIndex )
{
    applicationController_.selectApplication( applicationIndex );
}

void Application::enableAdditionalButtonInputHandler()
{
    applicationController_.enableAdditionalButtonInputHandler();
}

void Application::enableGridInputHandler()
{
    applicationController_.enableGridInputHandler();
}

void Application::enableRotaryControlInputHandler()
{
    applicationController_.enableRotaryControlInputHandler();
}

void Application::enableMidiInputAvailableHandler()
{
    applicationController_.enableMidiInputAvailableHandler();
}

void Application::enableMidiInputHandler()
{
    applicationController_.enableMidiInputHandler();
}

void Application::run( ApplicationThread& thread )
{
    // do nothing by default, this method is to be overridden
}

void Application::handleAdditionalButtonEvent( const AdditionalButtons::Event event )
{
    // do nothing by default, this method is to be overridden
}

void Application::handleGridButtonEvent( const grid::Grid::ButtonEvent event )
{
    // do nothing by default, this method is to be overridden
}

void Application::handleRotaryControlEvent( const RotaryControls::Event event )
{
    // do nothing by default, this method is to be overridden
}

void Application::handleMidiPacketAvailable()
{
    // do nothing by default, this method is to be overridden
}

void Application::handleMidiPacket( const midi::MidiPacket packet )
{
    // do nothing by default, this method is to be overridden
}

ApplicationController::ApplicationController( AdditionalButtons& additionalButtons, grid::Grid& grid,
    RotaryControls& rotaryControls, midi::UsbMidi& usbMidi ):
        Thread( "ApplicationController", kApplicationController.stackDepth, kApplicationController.priority ),
        currentlyOpenApplication_( NULL ),
        nextApplication_( freertos::Queue( 2, sizeof( ApplicationIndex ) ) ),
        additionalButtonInputHandler_( InputHandler<AdditionalButtons&, AdditionalButtons::Event>( *this, additionalButtons ) ),
        gridInputHandler_( InputHandler<grid::Grid&, grid::Grid::ButtonEvent>( *this, grid ) ),
        rotaryControlInputHandler_( InputHandler<RotaryControls&, RotaryControls::Event>( *this, rotaryControls ) ),
        midiInputAvailableHandler_( InputHandler<midi::UsbMidi&, bool>( *this, usbMidi ) ),
        midiInputHandler_( InputHandler<midi::UsbMidi&, midi::MidiPacket>( *this, usbMidi ) ),
        applicationThread_( ApplicationThread( *this ) )
{
}

void ApplicationController::initialize( Application** const applicationList )
{
    for (uint8_t index = 0; index < kNumberOfApplications; index++)
    {
        application_[index] = applicationList[index];
    }

    currentlyOpenApplication_ = application_[ApplicationIndex_STARTUP];
    Start();
}

void ApplicationController::selectApplication( const ApplicationIndex applicationIndex )
{
    Application* applicationBeingClosed = currentlyOpenApplication_;
    currentlyOpenApplication_ = application_[applicationIndex];
    application_[ApplicationIndex_PREVIOUS] = applicationBeingClosed;
    Notify();
}

void ApplicationController::Run()
{
    // open Startup application at first
    currentlyOpenApplication_ = application_[ApplicationIndex_STARTUP];
    applicationThread_.enable();

    while (true)
    {
        applicationThread_.run();
        WaitForNotification(); // block until notification from application
    }
}

void ApplicationController::enableAdditionalButtonInputHandler()
{
    additionalButtonInputHandler_.enable();
}

void ApplicationController::enableGridInputHandler()
{
    gridInputHandler_.enable();
}

void ApplicationController::enableRotaryControlInputHandler()
{
    rotaryControlInputHandler_.enable();
}

void ApplicationController::enableMidiInputAvailableHandler()
{
    midiInputAvailableHandler_.enable();
}

void ApplicationController::enableMidiInputHandler()
{
    midiInputHandler_.enable();
}

void ApplicationController::disableAllHandlers()
{
    additionalButtonInputHandler_.disable();
    gridInputHandler_.disable();
    rotaryControlInputHandler_.disable();
    midiInputAvailableHandler_.disable();
    midiInputHandler_.disable();
}

void ApplicationController::handleInput( const bool dummy )
{
    currentlyOpenApplication_->handleMidiPacketAvailable();
}

void ApplicationController::handleInput( const AdditionalButtons::Event event )
{
    currentlyOpenApplication_->handleAdditionalButtonEvent( event );
}

void ApplicationController::handleInput( const grid::Grid::ButtonEvent event )
{
    currentlyOpenApplication_->handleGridButtonEvent( event );
}

void ApplicationController::handleInput( const RotaryControls::Event event )
{
    currentlyOpenApplication_->handleRotaryControlEvent( event );
}

void ApplicationController::handleInput( const midi::MidiPacket packet )
{
    currentlyOpenApplication_->handleMidiPacket( packet );
}

void ApplicationController::runApplicationThread( ApplicationThread& thread )
{
    currentlyOpenApplication_->run( thread );
}

}
