
#include "ess.h"

namespace esphome
{
    namespace victron
    {
            void Ess::setup() 
            {

            }
            void Ess::loop() 
            {
                
            }
            void Ess::dump_config() 
            {
                ESP_LOGCONFIG(TAG, "Victron ESS:");
                LOG_COMPONENT("victron.ess", "Ess", this);
            }
    }
}
