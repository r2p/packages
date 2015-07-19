#pragma once

namespace r2p {

class SpeedMsg: public Message {
public:
	float value;
}R2P_PACKED;

class Speed2Msg: public Message {
public:
	float value[2];
}R2P_PACKED;

class Speed3Msg: public Message {
public:
	float value[3];
}R2P_PACKED;

}
