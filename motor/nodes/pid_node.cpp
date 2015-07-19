#include "ch.h"
#include "hal.h"

#include "r2p/Middleware.hpp"

#include "pid_node.hpp"
#include "pid.hpp"
#include "motor/msg/encoder.hpp"
#include "motor/msg/speed.hpp"
#include "motor/msg/pid.hpp"

static PID speed_pid;
static int16_t pwm = 0;

/*
 * PWM cyclic callback.
 */
static void pwmcb(PWMDriver *pwmp) {

	(void) pwmp;
	chSysLockFromIsr()
	;
	if (pwm >= 0) {
		pwm_lld_enable_channel(&PWMD1, 0, pwm);
		pwm_lld_enable_channel(&PWMD1, 1, 0);
	} else {
		pwm_lld_enable_channel(&PWMD1, 0, 0);
		pwm_lld_enable_channel(&PWMD1, 1, -pwm);
	}
	chSysUnlockFromIsr();
}

/*
 * PWM configuration.
 */
static PWMConfig pwmcfg = { 36000000, /* 72MHz PWM clock frequency.   */
4096, /* 12-bit PWM, 17KHz frequency. */
pwmcb, { { PWM_OUTPUT_ACTIVE_HIGH, NULL }, { PWM_OUTPUT_ACTIVE_HIGH, NULL }, { PWM_OUTPUT_DISABLED, NULL }, {
	PWM_OUTPUT_DISABLED, NULL } }, 0, 0};

/*
 * PID node
 */

bool enc_callback(const r2p::EncoderMsg &msg) {

	pwm = speed_pid.update(msg.delta * 50); // delta_rad to rad/s

	chSysLock();

	if (pwm > 0) {
		pwm_lld_enable_channel(&PWM_DRIVER, 1, pwm);
		pwm_lld_enable_channel(&PWM_DRIVER, 0, 0);
	} else {
		pwm_lld_enable_channel(&PWM_DRIVER, 1, 0);
		pwm_lld_enable_channel(&PWM_DRIVER, 0, -pwm);
	}
	chSysUnlock();

	return true;
}

msg_t pid_node(void * arg) {
	pid_node_conf_t * conf = reinterpret_cast<pid_node_conf_t *>(arg);
	pid_conf_t * pid_conf = conf->pid_conf;
	r2p::Node node(conf->name);
	r2p::Subscriber<r2p::Speed2Msg, 5> speed_sub;
	r2p::Subscriber<r2p::EncoderMsg, 5> enc_sub(enc_callback);
	r2p::Subscriber<r2p::PIDCfgMsg, 5> cfg_sub;
	r2p::Speed2Msg * msgp;
	r2p::PIDCfgMsg *cfgp;
	r2p::Time last_setpoint(0);

	(void) arg;
	chRegSetThreadName(conf->name);

	speed_pid.config(pid_conf->k, pid_conf->ti, pid_conf->td, pid_conf->ts, -4095.0, 4095.0);

	/* Enable the h-bridge. */
	palSetPad(GPIOB, GPIOB_MOTOR_ENABLE); palClearPad(GPIOA, GPIOA_MOTOR_D1);
	chThdSleepMilliseconds(500);
	pwmStart(&PWM_DRIVER, &pwmcfg);

	node.subscribe(speed_sub, conf->setpoint_topic);
	node.subscribe(enc_sub, conf->encoder_topic);
	node.subscribe(cfg_sub, conf->config_topic);

	for (;;) {
		// Check for PID configuration parameters
		if (cfg_sub.fetch(cfgp)) {
			speed_pid.config(cfgp->k, cfgp->ti, cfgp->td, 0.02, -4095, 4095);
			cfg_sub.release(*cfgp);
		}

		if (node.spin(r2p::Time::ms(1000))) {
			// Check for speed setpoint
			if (speed_sub.fetch(msgp)) {
				speed_pid.set(msgp->value[conf->motor_id]);
				last_setpoint = r2p::Time::now();
				speed_sub.release(*msgp);
			} else if (r2p::Time::now() - last_setpoint > r2p::Time::ms(1000)) {
				// Set speed to 0 if no speed messages for 1000 ms
				speed_pid.set(0);
			}
		} else {
			// Stop motor if no encoder messages for 1000 ms
			pwm_lld_disable_channel(&PWM_DRIVER, 0);
			pwm_lld_disable_channel(&PWM_DRIVER, 1);
		}
	}

	return CH_SUCCESS;
}
