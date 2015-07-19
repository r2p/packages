#pragma once

namespace r2p {

class PWMMsg: public Message {
public:
	uint16_t value;
	uint16_t dummy;
}R2P_PACKED;

class PWM2Msg: public Message {
public:
	int16_t value[2];
}R2P_PACKED;

class PWM3Msg: public Message {
public:
	int16_t value[3];
}R2P_PACKED;

}
