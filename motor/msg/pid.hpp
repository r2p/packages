#pragma once

namespace r2p {

struct PIDCfgMsg: public Message {
	float k;
	float ti;
	float td;
} R2P_PACKED;

}
