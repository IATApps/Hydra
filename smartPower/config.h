/*
 * config.h
 *
 * Created: 2/3/2013 5:03:33 PM
 *  Author: Caleb
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

#include "asf.h"

// Define the firmware revision
#define	FIRMWARE_REVISION			(uint32_t)(('H' << 24) | ('P' << 16) | ('0' << 8) | 'A')
// NOTE: Because the optimizer is stupid, it appears to be truncating this data.  In packet_handler.c,
// the firmware revision is hard-coded.  You have to change that, too, if you want to change the revision.

// CONFIG_ARRAY_SIZE and DATA_ARRAY_SIZE specify the number of 32 bit configuration and data registers used by the firmware
// (Note: The term "register" is used loosely here.  These "registers" are not actually registers in the same sense of a
// microcontroller register.  They are simply index locations into arrays stored in global memory.  Data and configuration
// parameters are stored in arrays because it allows a common communication protocol to be used to access all data and
// configuration.  The software communicating with the sensor needs only specify the register address, and the communication
// software running on the sensor knows exactly where to find it - it needn't know what the data is.  The software communicatin
// with the sensor, on the other hand, needs to know what it is asking for (naturally...)
// This setup makes it easy to make more data immediately available when needed - simply increase the array size, add code in
// the firmware that writes data to the new array location, and then make updates to the firmware definition on the PC side.
#define	CONFIG_ARRAY_SIZE			13
#define	DATA_ARRAY_SIZE				4
#define	COMMAND_COUNT				2

#define	CONFIG_REG_START_ADDRESS	0
#define	DATA_REG_START_ADDRESS		85
#define	COMMAND_START_ADDRESS		170

#define	COM_BAD_CHECKSUM			253								// Sent if the UM6 receives a packet with a bad checksum
#define	COM_UNKNOWN_ADDRESS			254								// Sent if the UM6 receives a packet with an unknown address
#define	COM_INVALID_BATCH_SIZE		255								// Sent if a requested batch read or write operation would go beyond the bounds of the config or data array

// Definitions for configuration settings register
#define REG_V1_SETTINGS				0
#define REG_V2_SETTINGS				1
#define REG_V3_SETTINGS				2
#define REG_VIN_SETTINGS			3
#define REG_V1_C1					4
#define REG_V1_C2					5
#define REG_V1_C3					6
#define REG_V2_C1					7
#define REG_V2_C2					8
#define REG_V2_C3					9
#define REG_V3_C1					10
#define REG_V3_C2					11
#define REG_V3_C3					12

#define OUTPUT_ENABLED_BIT			((uint32_t)1 << 31)
#define LOW_CURRENT_MODE_BIT		((uint32_t)1 << 30)
#define IOUT_MAX_START_BIT			16
#define IOUT_MAX_MASK				0x0FFF
#define VTARGET_START_BIT			0
#define VTARGET_MASK				0x0FFFF

#define V1_CUTOFF_DISABLE_BIT		31
#define V2_CUTOFF_DISABLE_BIT		30
#define V3_CUTOFF_DISABLE_BIT		29	
#define V_CUTOFF_START_BIT			0
#define V_CUTOFF_MASK				0x0FFFF

// Data register definitions
#define V1_STATUS					DATA_REG_START_ADDRESS + 0
#define V2_STATUS					DATA_REG_START_ADDRESS + 1
#define V3_STATUS					DATA_REG_START_ADDRESS + 2
#define VIN_STATUS					DATA_REG_START_ADDRESS + 3

// Voltage output status register bits
#define CV_BIT						((uint32_t)1 << 31)
#define CC_BIT						((uint32_t)1 << 30)
#define POT_FAULT_BIT				((uint32_t)1 << 29)
#define V_FAULT_BIT					((uint32_t)1 << 28)
#define FAULT_START_BIT				28
#define FAULT_MASK					0x03
#define CURRENT_START_BIT			16
#define CURRENT_MASK				0x0FFF
#define VOLTAGE_START_BIT			0
#define VOLTAGE_MASK				0x0FFFF

#define POT_FAULT					2
#define V_FAULT						1

// Vin  status register bits
#define VOLTAGE_CUTOFF_ACTIVE_BIT		31
#define VIN_VOLTAGE_START_BIT			0
#define VIN_VOLTAGE_MASK				0x0FFFF

// Definitions for commands
#define	GET_FW_VERSION				COMMAND_START_ADDRESS			// Causes the UM6 to report the firmware revision
#define WRITE_TO_FLASH				(COMMAND_START_ADDRESS + 1)

// Number of values to average to reduce noise on sample inputs
#define AVG_SIZE					32		// Make this a power of two

#define SUPPLY_NAME_LENGTH			50

// Constant defines scale factor for converting Vout ADC measurements to tenths of millivolts
#define VOUT_RAW_TO_V_SCALE			146
// Constant defines scale factor for converting Iout ADC measurements to tenths of milliamps
#define IOUT_RAW_TO_I_SCALE			56

// Delay in milliseconds that the source should use to allow voltage output to stabilize during calibration
#define SUPPLY_SETTLE_TIME			200

// Default calibration terms for converting POT taps to output voltages
#define DEFAULT_C1					2150400
#define DEFAULT_C2					-800
#define DEFAULT_C3					-162

// Min and Max outputs voltages
#define MIN_OUTPUT_VOLTAGE_MV		2500
#define MAX_OUTPUT_VOLTAGE_MV		14000

typedef struct __supply_config
{
	uint32_t r[CONFIG_ARRAY_SIZE];
	
	uint16_t POT1_taps;
	uint16_t POT2_taps;
	uint16_t POT3_taps;

	uint16_t V1_target;
	uint16_t V2_target;
	uint16_t V3_target;
	
	uint16_t Vin_cutoff;
	uint8_t V1_cutoff_disabled;
	uint8_t V2_cutoff_disabled;
	uint8_t V3_cutoff_disabled;

	uint16_t C1_limit;
	uint16_t C2_limit;
	uint16_t C3_limit;

	uint8_t V1_enabled;
	uint8_t V2_enabled;
	uint8_t V3_enabled;

	uint8_t V1_low_current;
	uint8_t V2_low_current;
	uint8_t V3_low_current;
	
	int32_t V1_C1;
	int32_t V1_C2;
	int32_t V1_C3;
	
	int32_t V2_C1;
	int32_t V2_C2;
	int32_t V2_C3;
	
	int32_t V3_C1;
	int32_t V3_C2;
	int32_t V3_C3;
	
} supply_config;

typedef struct __supply_data
{
	uint32_t r[DATA_ARRAY_SIZE];
	
	// Stores measured voltages in millivolts and currents in milliamps
	uint16_t vin;
	uint16_t vout1;
	uint16_t vout2;
	uint16_t vout3;
	uint16_t iout1;
	uint16_t iout2;
	uint16_t iout3;
	
	// Stores raw ADC measurements before conversion to milliamps and millivolts
	uint32_t vin_raw;
	uint32_t vout1_raw;
	uint32_t vout2_raw;
	uint32_t vout3_raw;
	uint32_t iout1_raw;
	uint32_t iout2_raw;
	uint32_t iout3_raw;
	
	uint32_t vout1_sum;
	uint32_t vout2_sum;
	uint32_t vout3_sum;
	
	uint32_t vin_sum;
	uint32_t iout1_sum;
	uint32_t iout2_sum;
	uint32_t iout3_sum;
	
	uint8_t vin_sample_count;
	uint8_t vout1_sample_count;
	uint8_t vout2_sample_count;
	uint8_t vout3_sample_count;
	uint8_t iout1_sample_count;
	uint8_t iout2_sample_count;
	uint8_t iout3_sample_count;
	
	// Flags indicating whether the supplies are in CC or CV modes
	uint8_t out1_cc;
	uint8_t out2_cc;
	uint8_t out3_cc;	
	
	// Flags indicating supply output fault
	uint8_t out1_fault;
	uint8_t out2_fault;
	uint8_t out3_fault;
	
	// Flag indicating that input voltage cutoff is set
	uint8_t voltage_cutoff_active;
	
	uint8_t new_vin;
	uint8_t new_vout1;
	uint8_t new_vout2;
	uint8_t new_vout3;
	uint8_t new_iout1;
	uint8_t new_iout2;
	uint8_t new_iout3;	
} supply_data;

extern volatile supply_config g_config;
extern volatile supply_data g_data;

extern uint32_t EEMEM ROM_config_settings[CONFIG_ARRAY_SIZE];

extern char g_refreshing_display;

extern char* g_status_on_cv_string;
extern char* g_status_off_string;
extern char* g_status_on_cc_string;
extern char* g_status_fault_pot_string;
extern char* g_status_fault_V_string;

extern char* g_supply1_status;
extern char* g_supply2_status;
extern char* g_supply3_status;

void copy_data_to_output( void );
void load_global_settings( void );
void set_global_defaults( void );
void save_config( void );
void save_config_to_ROM( void );

void copy_config_to_local( void );
void copy_local_to_config( void );
uint8_t update_outputs_from_config( void );

uint8_t update_output_voltage(uint8_t supply);

void enable_output( uint8_t output_number );
void disable_output( uint8_t output_number );
void disable_all( void );
void disable_all_temp( void );
void disable_output_temp( uint8_t output_number );
void set_low_current_mode( uint8_t output_number );
void set_high_current_mode( uint8_t output_number );

uint16_t get_POT_taps( int32_t voltageMV, uint8_t supply_number );
int32_t get_MV_from_taps( uint16_t POT_taps, uint8_t supply_number );

void get_coefficients(uint8_t supply_number, int32_t* C1, int32_t* C2, int32_t* C3 );

#endif /* CONFIG_H_ */
