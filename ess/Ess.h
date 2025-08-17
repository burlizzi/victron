#pragma once
#include <sstream>
#include "esphome/core/component.h"
#include <VEBusDefinition.h>
#include <VEBus.h>

namespace esphome
{
    namespace victron
    {
        static const char *const TAG = "Victron";
        class Ess : public Component
        {
        public:
            void setup() override;
            float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }
            void loop() override;
            void dump_config() override;
        protected:
            VEBus _vEBus;
        };
    }
}
