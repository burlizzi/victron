public methods:
void set_power_sensor(sensor::Sensor* powerOut) { powerOut_ = powerOut; }
void set_ac_power_sensor(sensor::Sensor* acPower) { acPower_ = acPower; }
void set_power_number(EssNumber* powerReq) { powerReq_ = powerReq; powerReq->set_parent(this); }

member variables:
sensor::Sensor* powerOut_=nullptr;
sensor::Sensor* acPower_=nullptr;
sensor::Sensor* battery_=nullptr;