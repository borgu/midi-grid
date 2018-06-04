/**
  ******************************************************************************
  * @file           : usbd_midi_if.c
  * @brief          :
  ******************************************************************************

    (CC at)2016 by D.F.Mac. @TripArts Music

*/

/* Includes ------------------------------------------------------------------*/
#include "usb/UsbMidi.h"

#include "stm32f4xx_hal.h"
#include "queue32.h"

// basic midi rx/tx functions
static uint16_t MIDI_DataRx(uint8_t *msg, uint16_t length);
static uint16_t MIDI_DataTx(uint8_t *message, uint16_t length);

// from mi:muz (Internal)
stB4Arrq rxq;

USBD_MIDI_ItfTypeDef USBD_Interface_fops_FS =
{
    MIDI_DataRx,
    MIDI_DataTx
};

static uint16_t MIDI_DataRx(uint8_t *msg, uint16_t length)
{
    uint16_t cnt;
    uint16_t msgs = length / 4;
    uint16_t chk = length % 4;
    volatile uint32_t value;
    if(chk == 0)
    {
        for(cnt = 0;cnt < msgs;cnt ++)
        {
            value = *(((uint32_t *)msg)+cnt);
            b4arrq_push(&rxq,((uint32_t *)msg)+cnt);
        }
    }
    return 0;
}

static uint16_t MIDI_DataTx(uint8_t *message, uint16_t length)
{
    uint32_t i = 0;
    while (i < length)
    {
        APP_Rx_Buffer[APP_Rx_ptr_in] = *(message + i);
        APP_Rx_ptr_in++;
        i++;
        if (APP_RX_DATA_SIZE == APP_Rx_ptr_in)
        {
            APP_Rx_ptr_in = 0;
        }
    }
    return USBD_OK;
}

namespace midi
{

UsbMidi::UsbMidi()
{
}

UsbMidi::~UsbMidi()
{
}

void UsbMidi::sendNoteOn( const uint8_t channel, const uint8_t note, const uint8_t velocity )
{
    uint8_t buffer[4];
    buffer[0] = 0x09;
    buffer[1] = 0x90 | channel;
    buffer[2] = 0x7F & note;
    buffer[3] = 0x7F & velocity;

    MIDI_DataTx(buffer,4);
    USBD_MIDI_SendPacket();
}

void UsbMidi::sendNoteOff( const uint8_t channel, const uint8_t note )
{
    uint8_t buffer[4];
    buffer[0] = 0x08;
    buffer[1] = 0x80 | channel;
    buffer[2] = 0x7F & note;
    buffer[3] = 0;

    MIDI_DataTx(buffer,4);
    USBD_MIDI_SendPacket();
}

void UsbMidi::sendControlChange( const uint8_t channel, const uint8_t control, const uint8_t value )
{
    uint8_t buffer[4];
    buffer[0] = 0x0B;
    buffer[1] = 0xB0 | channel;
    buffer[2] = 0x7F & control;
    buffer[3] = 0x7F & value;

    MIDI_DataTx(buffer,4);
    USBD_MIDI_SendPacket();
}


void UsbMidi::sendSystemExclussive( const uint8_t* const data, const uint8_t length )
{
    uint8_t buffer[4];
    uint8_t bytesRemaining;
    uint8_t i = 0;
    while (i<length)
    {
        bytesRemaining = length - i;
        switch (bytesRemaining)
        {
            case 3:
                buffer[0] = 0x07;
                buffer[1] = data[i++];
                buffer[2] = data[i++];
                buffer[3] = data[i++];
                break;
            case 2:
                buffer[0] = 0x06;
                buffer[1] = data[i++];
                buffer[2] = data[i++];
                buffer[3] = 0;
                break;
            case 1:
                buffer[0] = 0x05;
                buffer[1] = data[i++];
                buffer[2] = 0;
                buffer[3] = 0;
                break;
            default: // more than 3 bytes left
                buffer[0] = 0x04;
                buffer[1] = data[i++];
                buffer[2] = data[i++];
                buffer[3] = data[i++];
                break;
        }
        MIDI_DataTx(buffer,4);
        USBD_MIDI_SendPacket();
    }
}

bool UsbMidi::getPacket( MidiPacket& packet)
{
    MidiInput midiInput;
    bool packetAvailable = false;
    if (0 != rxq.num)
    {
        midiInput.input = *b4arrq_pop(&rxq);
        packet = midiInput.packet;
        packetAvailable = true;
    }
    return packetAvailable;
}

bool UsbMidi::isPacketAvailable()
{
    return (0 != rxq.num);
}

} // namespace
