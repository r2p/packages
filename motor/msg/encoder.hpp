#pragma once

namespace r2p {

class QEIMsg: public Message {
public:
	int16_t delta;
}R2P_PACKED;

class EncoderMsg: public Message {
public:
	float delta;
}R2P_PACKED;

class Encoder2Msg: public Message {
public:
	float delta[2];
}R2P_PACKED;

class AbsoluteEncoder: public Message {
public:
	float position;
}R2P_PACKED;

}
