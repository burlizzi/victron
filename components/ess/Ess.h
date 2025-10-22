#pragma once
#include <sstream>
#include "esphome/core/component.h"
#include "esphome/components/uart/uart_component_esp_idf.h"

namespace esphome
{
    namespace victron
    {
        static const char *const TAG = "Victron";
        class Ess : public Component
        {
        public:
            Ess(uart::UARTComponent *uart) : uart_(uart)
            {
            }
            void setup() override;
            float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }
            void loop() override;
            void on() { sendmsg(4, 0); }
            void off() { sendmsg(3, 0); }
            void power(short essPower) { desiredPower = essPower; }
            void dump_config() override;
            uart::IDFUARTComponent * getUart() { return static_cast<uart::IDFUARTComponent *>(this->uart_); }  
            void on_uart_data(int size)
            {
                // Handle the received data here
                ESP_LOGD(TAG, "Received %d bytes of data", size);
                // Process the data as needed
            }
        protected:
            uart::UARTComponent *uart_;

            int commandReplaceFAtoFF(uint8_t *outbuf, uint8_t *inbuf, int inlength);
            int destuffFAtoFF(uint8_t *outbuf, uint8_t *inbuf, int inlength);
            bool verifyChecksum(uint8_t *inbuf, int inlength);
            int appendChecksum(uint8_t *buf, int inlength);
            void sendmsg(int msgtype, short essPower);
            void decodeVEbusFrame(uint8_t *frame, int len);
            void multiplusCommandHandling();
            int cmdOnOff(uint8_t *outbuf, unsigned char desiredFrameNr, bool doon);
            int prepareESScommand(uint8_t *outbuf, short power, unsigned char desiredFrameNr);
            int preparecmd(uint8_t *outbuf, unsigned char desiredFrameNr);

            // Hardware
            const int VEBUS_RXD1 = 16, VEBUS_TXD1 = 17, VEBUS_DE1 = 4; // Victron Multiplus VE.bus RS485 gpio pins

            bool gotMP2data = false; // true if we got a valid frame from Multiplus
            bool syncrxed = false;   // true if we got a sync frame from Multiplus
            uint32_t synctime = 0;   // time of last sync frame received from Multiplus
            int chksmfault  = 0; // number of checksum errors received from Multiplus

            // other variables:
            uint8_t frbuf1[128]; // assembles one complete frame received by Multiplus
            uint8_t frbuf2[128]; // assembles one complete frame received by Multiplus
            uint8_t txbuf1[32];  // buffer for assembling bare command towards Multiplus (without replacements or checksum)
            uint8_t txbuf2[32];  // Multiplus output buffer containing the final command towards Multiplus, including replacements and checksum
            unsigned char rxnum;
            unsigned char frp = 0;     // Pointer into Multiplus framebuffer frbuf[] to store received frames.
            unsigned char frlen = 0;   // Pointer into Multiplus framebuffer
            unsigned char frameNr = 0; // Last frame number received from Multiplus. Own command has be be sent with frameNr+1, otherwise it will be ignored by Multiplus.

            // mp2 variables
            unsigned char masterMultiLED_LEDon;    // Bits 0..7 = mains on, absorption, bulk, float, inverter on, overload, low battery, temperature
            unsigned char masterMultiLED_LEDblink; //(LEDon=1 && LEDblink=1) = blinking; (LEDon=0 && LEDblink=1) = blinking_inverted
            unsigned char masterMultiLED_Status;   // 0=ok, 2=battery low
            unsigned char masterMultiLED_AcInputConfiguration;
            float masterMultiLED_MinimumInputCurrentLimit;
            float masterMultiLED_MaximumInputCurrentLimit;
            float masterMultiLED_ActualInputCurrentLimit;
            unsigned char masterMultiLED_SwitchRegister;
            float multiplusTemp;
            float multiplusDcCurrent;
            int16_t multiplusAh;
            unsigned char multiplusStatus80; // status from the charger/inverter frame 0x80: 0=ok, 2=battery low
            bool multiplusDcLevelAllowsInverting;
            bool acked = false; // acknowledge of last command sent to Multiplus
            float BatVolt;
            float multiplusAcFrequency;
            float multiplusPowerFactor;
            float multiplusAcPhase;
            float multiplusDcVoltage;
            int16_t ACPower;
            int16_t desiredPower;
        };
    }
}
