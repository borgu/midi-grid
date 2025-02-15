#ifndef APPLICATION_HPP_
#define APPLICATION_HPP_

#include "thread.hpp"
#include "semaphore.hpp"

#include "io/AdditionalButtons.hpp"
#include "io/grid/Grid.hpp"
#include "io/RotaryControls.hpp"
#include "io/usb/UsbMidi.hpp"

namespace application
{

enum ApplicationIndex : uint8_t
{
    ApplicationIndex_PREVIOUS = 0,
    ApplicationIndex_STARTUP,
    ApplicationIndex_GRID_TEST,
    ApplicationIndex_INTERNAL_MENU,
    ApplicationIndex_LAUNCHPAD,
    ApplicationIndex_SNAKE,
    kNumberOfApplications
};

class Application;
class ApplicationController;

template <class InputSource, class InputType>
class InputHandler : private freertos::Thread
{
public:
    InputHandler( ApplicationController& applicationController, InputSource inputSource );

    void enable();
    void disable();

    void Run();

private:
    ApplicationController& applicationController_;
    InputSource inputSource_;
};

class ApplicationThread : public freertos::Thread
{
public:
    ApplicationThread( ApplicationController& applicationController );

    void enable();
    void disable();
    void run();

private:
    void Run();

    ApplicationController& applicationController_;
    freertos::BinarySemaphore continueApplication_;
};

class Application
{
public:
    Application( ApplicationController& applicationController );
    virtual ~Application();

    virtual void run( ApplicationThread& thread );
    virtual void handleAdditionalButtonEvent( const AdditionalButtons::Event event );
    virtual void handleGridButtonEvent( const grid::Grid::ButtonEvent event );
    virtual void handleRotaryControlEvent( const RotaryControls::Event event );
    virtual void handleMidiPacket( const midi::MidiPacket packet );
    virtual void handleMidiPacketAvailable();

protected:
    void switchApplication( const ApplicationIndex application );

    void enableAdditionalButtonInputHandler();
    void enableGridInputHandler();
    void enableRotaryControlInputHandler();
    void enableMidiInputAvailableHandler();
    void enableMidiInputHandler();

private:
    ApplicationController& applicationController_;
};

class ApplicationController : private freertos::Thread
{
public:
    ApplicationController( AdditionalButtons& additionalButtons, grid::Grid& grid, RotaryControls& rotaryControls,
        midi::UsbMidi& usbMidi );

    void initialize( Application** const applicationList );

    void selectApplication( const ApplicationIndex applicationIndex );

    void enableAdditionalButtonInputHandler();
    void enableGridInputHandler();
    void enableRotaryControlInputHandler();
    void enableMidiInputAvailableHandler();
    void enableMidiInputHandler();
    void disableAllHandlers();

    void handleInput( const bool dummy );
    void handleInput( const AdditionalButtons::Event event );
    void handleInput( const grid::Grid::ButtonEvent event );
    void handleInput( const RotaryControls::Event event );
    void handleInput( const midi::MidiPacket packet );
    void runApplicationThread( ApplicationThread& thread );

private:
    void Run();

    Application* application_[kNumberOfApplications];
    Application* currentlyOpenApplication_;
    freertos::Queue nextApplication_;
    static bool applicationFinished_;

    InputHandler<AdditionalButtons&, AdditionalButtons::Event> additionalButtonInputHandler_;
    InputHandler<grid::Grid&, grid::Grid::ButtonEvent> gridInputHandler_;
    InputHandler<RotaryControls&, RotaryControls::Event> rotaryControlInputHandler_;
    InputHandler<midi::UsbMidi&, bool> midiInputAvailableHandler_;
    InputHandler<midi::UsbMidi&, midi::MidiPacket> midiInputHandler_;
    ApplicationThread applicationThread_;
};

} // namespace

#endif // APPLICATION_HPP_
