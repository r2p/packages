#pragma once

/*
 * PID configuration
 */

struct pid_conf_t {
	float k;
	float ti;
	float td;
	float ts;
};

struct pid_node_conf_t {
	const char * name;
	int motor_id;
	const char * setpoint_topic;
	const char * encoder_topic;
	const char * config_topic;
	pid_conf_t * pid_conf;
};

msg_t pid_node(void * arg);
