
#include "Ess.h"
#include "esphome/core/log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
namespace esphome
{
    namespace victron
    {

        
        void EssNumber::control(float timeout){
            this->parent_->power(static_cast<short>(timeout));
        }
        int prepareCommandReadRAMVar(unsigned char *outbuf, uint8_t desiredFrameNr)
        {
        uint8_t j=0;
        outbuf[j++] = 0x98;           //MK3 interface to Multiplus
        outbuf[j++] = 0xf7;           //MK3 interface to Multiplus
        outbuf[j++] = 0xfe;           //data frame
        outbuf[j++] = desiredFrameNr;
        outbuf[j++] = 0x00;           //our own ID
        outbuf[j++] = 0xe6;           //our own ID
        outbuf[j++] = 0x30;           //CommandReadRAMVar
        outbuf[j++] = 0;              //0=UmainsRMS             1st RAM ID (up to 6 possible)
        outbuf[j++] = 0x0f;             //15=PmainsFiltered       2nd RAM ID
        outbuf[j++] = 0x0e;             //14=PinverterFiltered    3rd RAM ID
        outbuf[j++] = 1;             //16=PoutputFiltered      4th RAM ID
        return j;
        }

        void Ess::uart_event_task(void *pvParameters)
        {
            Ess *self = static_cast<Ess *>(pvParameters);
            uart_event_t event;
            for (;;)
            {
                // Wait for UART event.
                //if (xQueueReceive(*self->getUart()->get_uart_event_queue(), (void *)&event, (TickType_t)portMAX_DELAY))
                {
                    if (event.type == UART_DATA)
                    {
                        ESP_LOGV(TAG, "UART data event, size=%d", event.size);
                        // Data received, call your callback or process data here

                        self->multiplusCommandHandling();
                    }
                    // Handle other event types if needed
                }
            }
        }
        void Ess::setup()
        {
            //xTaskCreate(uart_event_task, "uart_event_task", 4096, this, 12, NULL);
        }
        void Ess::loop()
        {
            
            multiplusCommandHandling();
        }
        void Ess::dump_config()
        {
            ESP_LOGCONFIG(TAG, "Victron ESS:");
            // ESP_LOG("victron.ess", "Ess", this);
        }
        int Ess::commandReplaceFAtoFF(uint8_t *outbuf, uint8_t *inbuf, int inlength)
        {
            int j = 0;
            // copy over the first 4 bytes of command, as there is no replacement
            for (int i = 0; i < 4; i++)
                outbuf[j++] = inbuf[i];

            // starting from 5th unsigned char, replace 0xFA..FF with double-byte character
            for (int i = 4; i < inlength; i++)
            {
                unsigned char c = inbuf[i];
                if (c >= 0xFA)
                {
                    outbuf[j++] = 0xFA;
                    outbuf[j++] = 0x70 | (c & 0x0F);
                }
                else
                    outbuf[j++] = c; // no replacement
            }
            return j; // new length of output frame
        }

        bool Ess::verifyChecksum(uint8_t *inbuf, int inlength)
        {
            unsigned char cs = 0;
            for (int i = 2; i < inlength; i++)
                cs += inbuf[i]; // sum over all bytes excluding the first two (address)
            if (cs == 0)
                return true;
            else
                return false;
        }

        int Ess::appendChecksum(uint8_t *buf, int inlength)
        {
            int j = 0;
            // calculate checksum starting from 3rd unsigned char
            unsigned char cs = 1;
            for (int i = 2; i < inlength; i++)
            {
                cs -= buf[i];
            }
            j = inlength;
            if (cs >= 0xFB) // EXCEPTION: Only replace starting from 0xFB
            {
                buf[j++] = 0xFA;
                buf[j++] = (cs - 0xFA);
            }
            else
            {
                buf[j++] = cs;
            }
            buf[j++] = 0xFF; // append End Of Frame symbol
            return j;        // new length of output frame
        }

        void Ess::sendmsg(int msgtype, short essPower)
        {
            int len;
            switch (msgtype)
            {
                case 1: // ESS power command
                    len = prepareESScommand(txbuf1, essPower, (frameNr + 1) & 0x7F);
                    break;
                case 2: // Read RAM variables
                    len = prepareCommandReadRAMVar(txbuf1, (frameNr + 1) & 0x7F);
                    break;
                case 3: // ESS off
                    len = cmdOnOff(txbuf1, (frameNr + 1) & 0x7F, false);
                    break;
                case 4: // ESS on
                    len = cmdOnOff(txbuf1, (frameNr + 1) & 0x7F, true);
                    break;
                default:
                    ESP_LOGE(TAG, "Unknown ESS command type %d", msgtype);
                    return;
            }
            len = commandReplaceFAtoFF(txbuf2, txbuf1, len);
            len = appendChecksum(txbuf2, len);
            // write command into Multiplus :-)
            //gpio_reset_pin(GPIO_NUM_18);
            //gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);

            //gpio_set_level(GPIO_NUM_18, 1);
            ESP_LOGV(TAG,"Sending ESS command, len=%d", len);
            uart_->write_array(txbuf2, len); // write command bytes to UART
            //gpio_set_level(GPIO_NUM_18, 0);
        }

        void Ess::decodeVEbusFrame(uint8_t *frame, int len)
        {
            ESP_LOGD(TAG, "Decoding VE.bus frame, len=%d", len);
            // data frame
            switch(frame[4])
            {
                case 0x80: // 80 = Condition of Charger/Inverter (Temp+Current)
                {
                    if ((frame[5] == 0x80) && ((frame[6] & 0xFE) == 0x12) && (frame[8] == 0x80) && ((frame[11] & 0x10) == 0x10))
                    {
                        multiplusStatus80 = frame[7];
                        multiplusDcLevelAllowsInverting = (frame[6] & 0x01);
                        int16_t t = 256 * frame[10] + frame[9];
                        multiplusDcCurrent = 0.1 * t;

                        //ESP_LOGI(TAG, "Multiplus current: %.1f", multiplusDcCurrent);
                        if ((frame[11] & 0xF0) == 0x30)
                        {
                            multiplusTemp = 0.1 * frame[15]*2-8; //pv-baxi's guess: (*2.0 - 8°C) as otherwise temperature is too low
                            ESP_LOGD(TAG, "Multiplus Temp: %.1f", multiplusTemp);
                        }
                            
                    }
                }
                break;
                case 0xE4: // E4 = AC phase information (comes with 50Hz)
                {
                    // 83 83 fe 51 e4  80 56 c3 c3 a6 4c be 8f d3 68 19 4b 7a 00  1a ff
                    // 83 83 fe 68 e4  2b 46 c3 9c 68 31 be 8f d8 68 19 4b 7a 00  e3 ff
                    // 83 83 fe 3d e4  13 5e c3 c4 dc 39 be 8f 7d 68 0b 4b 7a 00  d3 ff
                if ( (len==21) ) {
                    uint16_t ut = (frame[7]<<8) + frame[6];
                    multiplusAcFrequency = ut / 1000.0;
                    //multiplusE4_Timestamp = (frame[10]<<16) + (frame[9]<<8) + frame[8];
                    //multiplusE4_byte11 = frame[11];
                    //multiplusE4_byte12 = frame[12];
                    int16_t it = (frame[14]<<8) + frame[13];
                    multiplusPowerFactor = it / 32768.0;
                    multiplusAcPhase = frame[15];
                    ut = ((frame[17] & 0x0F)<<8) + frame[16];
                    multiplusDcVoltage = ut / 50.0;
                    if (battery_!=nullptr)
                        battery_->publish_state(multiplusDcVoltage);
                    static int count=0;
                    if(count++==100)//2sec
                    {
                        count=0;
                        ESP_LOGD(TAG, "Multiplus DC volt: %.1f ACfreq: %.1f out power:%d", multiplusDcVoltage,multiplusAcFrequency,multiplusPinverterFiltered);

                    }
                }
                }
                break;
                case 0x70: // 70 = DC capacity counter)
                {
                    // 83 83 fe 23 70  81 44 16 5e 01 c2 00 00  74 ff
                    // 83 83 fe 20 70  81 44 16 5e 01 c2 00 00  77 ff
                }
                break;
                case 0x41: // 41 = Multiplus mode / master led
                {
                    if ((len == 19) && (frame[5] == 0x10)) // frame[5] unknown byte
                    {
                        masterMultiLED_LEDon = frame[6];
                        masterMultiLED_LEDblink = frame[7];
                        masterMultiLED_Status = frame[8];
                        masterMultiLED_AcInputConfiguration = frame[9];
                        int16_t t = 256 * frame[11] + frame[10];
                        masterMultiLED_MinimumInputCurrentLimit = t / 10.0;
                        t = 256 * frame[13] + frame[12];
                        masterMultiLED_MaximumInputCurrentLimit = t / 10.0;
                        t = 256 * frame[15] + frame[14];
                        masterMultiLED_ActualInputCurrentLimit = t / 10.0;
                        masterMultiLED_SwitchRegister = frame[16];
                    }
                }
                break;
                case 0x38: // ?
                {
                    // 83 83 fe 05 38 01 c0 c0 45 ff
                    // for (int i=0;i<len;i++) extframe[i] = frame[i];
                    // extframelen = len;
                }
                break;
                case 0x00: // ack or response
                {
                    ESP_LOGV(TAG, "frame: %x %x %x", frame[4],frame[5],frame[6]);

                    if (frame[5] == 0xE6)
                    {
                        if (frame[6] == 0x87)
                            acked = true;
                        else if (frame[6] == 0x85)
                        {
                            gotMP2data = true;
                            int16_t v = 256 * frame[8] + frame[7];
                            BatVolt = 0.01 * float(v);
                            ACPower = 256 * frame[10] + frame[9];
                            multiplusPinverterFiltered = 256 * frame[12] + frame[11];
                            if (powerOut_ != nullptr) {
                                powerOut_->publish_state(float(multiplusPinverterFiltered));
                            }
                            if (powerAc_ != nullptr) {
                                powerAc_->publish_state(float(ACPower));
                            }
                            
                            ESP_LOGV(TAG,"mains volt: %.1f ACPower: %d InvPower: %d",BatVolt,ACPower,multiplusPinverterFiltered);
                            // MP2Soc  = 256*frame[10] + frame[9];
                            // ACPower = 256*frame[12] + frame[11];
                        }
                        else
                        {
                        }
                    }
                }
                break;
                default:
                            ESP_LOGV(TAG, "frame: %x", frame[4]);

                break;
            }
        }

        void Ess::multiplusCommandHandling()
        {            
            // Check for new bytes on UART
            while (uart_->available())
            {
                uint8_t c;
                uart_->read_byte(&c); // read one byte
                frbuf1[frp++] = c;   // store into framebuffer
                if (c == 0x55)
                {
                    if (frp == 5)
                        synctime = esphome::millis();
                }
                if (c == 0xFF) // in case current byte was EndOfFrame, interprete frame
                {
                    if ((frbuf1[2] == 0xFD) && (frbuf1[4] == 0x55)) // if it was a sync frame:
                    {
                        frameNr = frbuf1[3];
                        static int synccnt = 0;
                        synccnt++;
                        if (synccnt == 50) {   //every 5th sync frame, meaning 10 times per second, CommandGetRAMVarInfo is sent, NO re-send
                            synccnt = 0;    //reset counter
                            //build desired command
                            int len = 0;
                            //syslog.println("ask stuff");
                            len = prepareCommandReadRAMVar(txbuf1, (frameNr+1) & 0x7F);
                            //postprocess command
                            len = commandReplaceFAtoFF(txbuf2, txbuf1, len);
                            len = appendChecksum(txbuf2, len);
                            //write command into Multiplus :-)
                            uart_->write_array(txbuf2, len); // write command bytes to UART
                        }


                        syncrxed = true;
                        switch (command)
                        {
                            case 1:
                                ESP_LOGD(TAG, "ESS command: set power to %d W", desiredPower);
                                sendmsg(1,desiredPower);
                                break;
                            case 4:
                                ESP_LOGI(TAG, "ESS command: ON");
                                sendmsg(4, 0);
                                break;
                            case 3:
                                ESP_LOGI(TAG, "ESS command: OFF");
                                sendmsg(3, 0);
                                break;
                            default:
                                break;
                        }
                        command=1;

                    }
                    else if ((frbuf1[0] == 0x83) && (frbuf1[1] == 0x83) && (frbuf1[2] == 0xFE))
                    {
                        if (verifyChecksum(frbuf1, frp))
                        {
                            frlen = destuffFAtoFF(frbuf2, frbuf1, frp);
                            decodeVEbusFrame(frbuf2, frlen);
                        }
                        else
                            chksmfault++;
                    }
                    rxnum = frlen;
                    frp = 0;
                }
                else
                    syncrxed = false; // unexpected char received
            }
        }

        int Ess::prepareESScommand(uint8_t *outbuf, short power, unsigned char desiredFrameNr)
        {
            unsigned char j = 0;
            outbuf[j++] = 0x98; // MK3 interface to Multiplus
            outbuf[j++] = 0xf7; // MK3 interface to Multiplus
            outbuf[j++] = 0xfe; // data frame
            outbuf[j++] = desiredFrameNr;
            outbuf[j++] = 0x00;           // our own ID
            outbuf[j++] = 0xe6;           // our own ID
            outbuf[j++] = 0x37;           // CommandWriteViaID
            outbuf[j++] = 0x02;           // Flags, 0x02=RAMvar and no EEPROM
            outbuf[j++] = 0x83;           // ID = address of ESS power in assistand memory
            outbuf[j++] = (power & 0xFF); // Lo value of power (positive = into grid, negative = from grid)
            outbuf[j++] = (power >> 8);   // Hi value of power (positive = into grid, negative = from grid)
            return j;
        }

        int Ess::preparecmd(uint8_t *outbuf, unsigned char desiredFrameNr)
        {
            unsigned char j = 0;
            outbuf[j++] = 0x98; // MK3 interface to Multiplus
            outbuf[j++] = 0xf7; // MK3 interface to Multiplus
            outbuf[j++] = 0xfe; // data frame
            outbuf[j++] = desiredFrameNr;
            outbuf[j++] = 0x00; // our own ID
            outbuf[j++] = 0xe6; // our own ID
            outbuf[j++] = 0x30; // Command read ram
            outbuf[j++] = 0;              //0=UmainsRMS             1st RAM ID (up to 6 possible)
            outbuf[j++] = 0x0f;             //15=PmainsFiltered       2nd RAM ID
            outbuf[j++] = 0x0e;             //14=PinverterFiltered    3rd RAM ID
            outbuf[j++] = 1;             //16=PoutputFiltered      4th RAM ID
            // outbuf[j++] = 0x0D;           //13-SOC
            outbuf[j++] = 0x0E; // 14-AC Power
            return j;
        }

        int Ess::cmdOnOff(uint8_t *outbuf, unsigned char desiredFrameNr, bool doon)
        {
            // 98F7 FE 60 3F07 0000005DFF wakeup
            // 98F7 FE 6F 3F04 00000051FF sleep
            unsigned char j = 0;
            outbuf[j++] = 0x98; // MK3 interface to Multiplus
            outbuf[j++] = 0xf7; // MK3 interface to Multiplus
            outbuf[j++] = 0xfe; // data frame
            outbuf[j++] = desiredFrameNr;
            outbuf[j++] = 0x3F; // cmd
            if (doon)
                outbuf[j++] = chargeOnly_?0x05:0x07; // wakeup
            else
                outbuf[j++] = 0x04; // sleep
            outbuf[j++] = 0x00;
            outbuf[j++] = 0x00;
            outbuf[j++] = 0x00;
            return j;
        }
        int Ess::destuffFAtoFF(uint8_t *outbuf, uint8_t *inbuf, int inlength)
        {
            int j = 0;
            for (int i = 0; i < 4; i++)
                outbuf[j++] = inbuf[i];
            for (int i = 4; i < inlength; i++)
            {
                unsigned char c = inbuf[i];
                if (c == 0xFA)
                {
                    c = inbuf[++i];
                    if (c == 0xFF) // if 0xFA is the checksum, leave the FA and following FF (end of frame) in as it was.
                    {
                        outbuf[j++] = 0xFA;
                        outbuf[j++] = c;
                    }
                    else
                        outbuf[j++] = c + 0x80;
                }
                else
                    outbuf[j++] = c; // no replacement
            }
            return j; // new length of output frame
        }

    } // namespace victron
} // namespace esphome
