/*
 * (C) Copyright 2004-Present
 * Peter Chen <peterc@socle-tech.com.tw>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include "aspeed_i2c.h"

void i2c_init(int speed, int slaveadd)
{
	unsigned long SCURegister;

	/* I2C Reset */
	SCURegister = inl(SCU_BASE + SCU_RESET_CONTROL);
	outl(SCURegister & ~(0x04), SCU_BASE + SCU_RESET_CONTROL);

	/* I2C Multi-Pin */
	SCURegister = inl(SCU_BASE + SCU_MULTIFUNCTION_PIN_CTL5_REG);
	outl((SCURegister | 0x30000), SCU_BASE + SCU_MULTIFUNCTION_PIN_CTL5_REG);

	/* Reset */
	outl(0, I2C_FUNCTION_CONTROL_REGISTER);

	/* Set AC Timing, we use fix AC timing for eeprom in u-boot */
	outl(AC_TIMING, I2C_AC_TIMING_REGISTER_1);
	outl(0, I2C_AC_TIMING_REGISTER_2);

	/* Clear Interrupt */
	outl(ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);

	/* Enable Master Mode */
	outl(MASTER_ENABLE, I2C_FUNCTION_CONTROL_REGISTER);

	/* Enable Interrupt, STOP Interrupt has bug in AST2000 */
	outl(0xAF, I2C_INTERRUPT_CONTROL_REGISTER);

	/* Set Slave address, should not use for eeprom */
	outl(slaveadd, I2C_DEVICE_ADDRESS_REGISTER);
}

static int i2c_check_address(u16 regoffset) {
	u32 status, count = 0;

	/* Send Device Register Offset (HIGH BYTE) */
	outl((regoffset & 0xFF00) >> 8, I2C_BYTE_BUFFER_REGISTER);
	outl(MASTER_TX_COMMAND, I2C_COMMAND_REGISTER);
	/* Wait Tx ACK */
	do {
		status = (inl(I2C_INTERRUPT_STATUS_REGISTER) & TX_EITHER);
		count++;
		if (count == LOOP_COUNT) {
			printf("Send Device Register Offset not ACKed\n");
			return 1;
		}
	} while (status != TX_ACK);
	/* Clear Interrupt */
	outl(ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
	return 0;
}

static int i2c_stop(void) {
	u32 status, count = 0;
	/* Clear Interrupt */
	outl(ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
	/* Enable Interrupt + Stop Interrupt */
	outl(0xBF, I2C_INTERRUPT_CONTROL_REGISTER);
	/* Issue Stop Command */
	outl(MASTER_STOP_COMMAND, I2C_COMMAND_REGISTER);
	/* Wait Stop */
	do {
		status = (inl(I2C_INTERRUPT_STATUS_REGISTER) & STOP_DONE);
		count++;
		if (count == LOOP_COUNT) {
			printf("Can't get STOP back\n");
			return 1;
		}
	} while (status != STOP_DONE);

	/* Disable Stop Interrupt */
	outl(0xAF, I2C_INTERRUPT_CONTROL_REGISTER);
	/* Clear Interrupt */
	outl(ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
	return 0;
}

static void i2c_start(u8 devaddr) {
	/* Start and Send Device Address */
	outl(devaddr, I2C_BYTE_BUFFER_REGISTER);
	outl(MASTER_START_COMMAND | MASTER_TX_COMMAND, I2C_COMMAND_REGISTER);
}

static int i2c_read_byte(u8 devaddr, u16 regoffset, u8 *value, int alen)
{
	int i2c_error = 0;
	u32 status, count = 0;

	i2c_start(devaddr);
	/* Wait Tx ACK */
	do {
		status = (inl(I2C_INTERRUPT_STATUS_REGISTER) & TX_EITHER);
		count++;
		if (count == LOOP_COUNT) {
			i2c_error = 1;
			printf("Start and Send Device Address not ACKed\n");
			return i2c_error;
		}
	} while (status != TX_ACK);
	count = 0;

	/* Clear Interrupt */
	outl(ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
	/* Check if address length equals to 16bits */
	if (alen != 1 && i2c_check_address(regoffset) != 0) {
		return 1;
	}

	/* Send Device Register Offset(LOW) */
	outl(regoffset & 0xFF, I2C_BYTE_BUFFER_REGISTER);
	outl(MASTER_TX_COMMAND, I2C_COMMAND_REGISTER);
	/* Wait Tx ACK */
	do {
		status = (inl(I2C_INTERRUPT_STATUS_REGISTER) & TX_EITHER);
		count++;
		if (count == LOOP_COUNT) {
			i2c_error = 1;
			printf("Send Device Register Offset no ACK\n");
			return i2c_error;
		}
	} while (status != TX_ACK);
	count = 0;

	/* Clear Interrupt */
	outl(ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
	/* Start, Send Device Address + 1 (Read Mode), Receive Data */
	outl(devaddr + 1, I2C_BYTE_BUFFER_REGISTER);
	outl(MASTER_START_COMMAND | MASTER_TX_COMMAND | MASTER_RX_COMMAND |
		RX_COMMAND_LIST, I2C_COMMAND_REGISTER);

	/* Wait Rx Done */
	do {
		status = (inl(I2C_INTERRUPT_STATUS_REGISTER) & RX_DONE);
		count++;
		if (count == LOOP_COUNT) {
			i2c_error = 1;
			printf("Can't get RX_DONE back\n");
			return i2c_error;
		}
	} while (status != RX_DONE);
	count = 0;

	if (i2c_stop() != 0) {
		return 1;
	}
	/* Read Received Data */
	*value = ((inl(I2C_BYTE_BUFFER_REGISTER) & 0xFF00) >> 8);
	return i2c_error;
}

static void i2c_write_retry(u8 devaddr) {
	/* Clear Interrupt */
	outl(ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
	/* Re-send Start and Send Device Address while NACK return */
	outl(devaddr, I2C_BYTE_BUFFER_REGISTER);
	outl(MASTER_START_COMMAND | MASTER_TX_COMMAND, I2C_COMMAND_REGISTER);
}

static int i2c_write_byte(u8 devaddr, u16 regoffset, u8 value, int alen)
{
	int i2c_error = 0;
	u32 status, count = 0;

	i2c_start(devaddr);
	/* Wait Tx ACK */
	do {
		status = (inl(I2C_INTERRUPT_STATUS_REGISTER) & TX_EITHER);
		count++;
		if (status == TX_NACK) {
			i2c_write_retry(devaddr);
		} else if (count == LOOP_COUNT) {
			i2c_error = 1;
			printf("Start and Send Device Address not ACKed\n");
			return i2c_error;
		}
	} while (status != TX_ACK);
	count = 0;

	/* Clear Interrupt */
	outl(ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
	/* Check if address length equals to 16bits */
	if (alen != 1 && i2c_check_address(regoffset) != 0) {
		return 1;
	}

	/* Send Device Register Offset */
	outl(regoffset & 0xFF, I2C_BYTE_BUFFER_REGISTER);
	outl(MASTER_TX_COMMAND, I2C_COMMAND_REGISTER);

	/* Wait Tx ACK */
	do {
		status = (inl(I2C_INTERRUPT_STATUS_REGISTER) & TX_EITHER);
		count++;
		if (count == LOOP_COUNT) {
			i2c_error = 1;
			printf("Send Device Register Offset not ACKed\n");
			return i2c_error;
		}
	} while (status != TX_ACK);
	count = 0;

	/* Clear Interrupt */
	outl(ALL_CLEAR, I2C_INTERRUPT_STATUS_REGISTER);
	/* Send Device Register Value */
	outl(value, I2C_BYTE_BUFFER_REGISTER);
	outl(MASTER_TX_COMMAND, I2C_COMMAND_REGISTER);

	/* Wait Tx ACK */
	do {
		status = (inl(I2C_INTERRUPT_STATUS_REGISTER) & TX_EITHER);
		count++;
		if (count == LOOP_COUNT) {
			i2c_error = 1;
			printf("Send Device Register Value not ACKed\n");
			return i2c_error;
		}
	} while (status != TX_ACK);
	count = 0;

	if (i2c_stop() != 0) {
		return 1;
	}
	return i2c_error;
}

int i2c_probe(uchar chip)
{
	/* Suppose IP is always on chip */
	int res = 0;
	return res;
}

int i2c_read(uchar device_addr, uint offset, int alen, uchar *buffer, int len)
{
	int i;

	if ((alen == 1) && ((offset + len) > 256)) {
		printf("Register index overflow\n");
	}

	for (i = 0; i < len; i++) {
		if (i2c_read_byte(device_addr, offset + i, &buffer[i], alen)) {
			printf("I2C read: I/O error\n");
			i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
			return 1;
		}
	}

	return 0;
}

int i2c_write(uchar device_addr, uint offset, int alen, uchar *buffer, int len)
{
	int i;

	if ((alen == 1) && ((offset + len) > 256)) {
		printf("Register index overflow\n");
	}

	for (i = 0; i < len; i++) {
		if (i2c_write_byte(device_addr, offset + i, buffer[i], alen)) {
			printf("I2C read: I/O error\n");
			i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
			return 1;
		}
	}

	return 0;
}
