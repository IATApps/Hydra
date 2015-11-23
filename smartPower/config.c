/*
 * config.c
 *
 * Created: 2/3/2013 5:03:23 PM
 *  Author: Caleb
 */ 

#include "asf.h"
#include "config.h"
#include "util.h"
#include "MCP4451.h"
#include "USART.h"

volatile supply_config g_config;
volatile supply_data g_data;

uint32_t EEMEM ROM_config_settings[CONFIG_ARRAY_SIZE];

// Preinitialize EEPROM
uint8_t g_supply_name[SUPPLY_NAME_LENGTH];

uint8_t g_outputs_disabled_on_startup;

char* g_status_on_cv_string = "ON - CV";
char* g_status_off_string = "OFF";
char* g_status_on_cc_string = "ON - CC";
char* g_status_fault_pot_string = "*FAULT* C1";
char* g_status_fault_V_string = "*FAULT* C2";

char* g_supply1_status;
char* g_supply2_status;
char* g_supply3_status;

/*******************************************************************************
* Function Name  : set_global_defaults
* Input          : None
* Output         : None
* Return         : void
* Description    :

Sets hard-coded global default settings

*******************************************************************************/
void set_global_defaults()
{
	// Default POT taps
	g_config.POT1_taps = 1024;
	g_config.POT2_taps = 1024;
	g_config.POT3_taps = 1024;
	
	g_config.V1_C1 = DEFAULT_C1;
	g_config.V1_C2 = DEFAULT_C2;
	g_config.V1_C3 = DEFAULT_C3;
	g_config.V2_C1 = DEFAULT_C1;
	g_config.V2_C2 = DEFAULT_C2;
	g_config.V2_C3 = DEFAULT_C3;
	g_config.V3_C1 = DEFAULT_C1;
	g_config.V3_C2 = DEFAULT_C2;
	g_config.V3_C3 = DEFAULT_C3;
	
	// Default target output voltages
	g_config.V1_target = get_MV_from_taps(g_config.POT1_taps, 1);
	g_config.V2_target = get_MV_from_taps(g_config.POT2_taps, 2);
	g_config.V3_target = get_MV_from_taps(g_config.POT3_taps, 3);
	
	// Default current limit in milliamps
	g_config.C1_limit = 2500;
	g_config.C2_limit = 2500;
	g_config.C3_limit = 2500;
	
	g_config.Vin_cutoff = 4000;
	g_config.V1_cutoff_disabled = 0;
	g_config.V2_cutoff_disabled = 0;
	g_config.V3_cutoff_disabled = 0;
	
	// Supplies disabled by default
	g_config.V1_enabled = 0;
	g_config.V2_enabled = 0;
	g_config.V3_enabled = 0;
	
	// Low-current mode disabled by default
	g_config.V1_low_current = 0;
	g_config.V2_low_current = 0;
	g_config.V3_low_current = 0;
	
	save_config();
}

/*******************************************************************************
* Function Name  : save_config
* Input          : None
* Output         : None
* Return         : void
* Description    :

Saves the current configuration

*******************************************************************************/
void save_config()
{
	copy_local_to_config();
	save_config_to_ROM();
}

/*******************************************************************************
* Function Name  : save_config_to_ROM
* Input          : None
* Output         : None
* Return         : void
* Description    :

Saves the current configuration register settings to ROM

*******************************************************************************/
__attribute__((optimize("Os")))
void save_config_to_ROM()
{
	int index;
	
	// Load settings from EEPROM config array
	for( index = 0; index < CONFIG_ARRAY_SIZE; index++ )
	{
		eeprom_write_dword((uint32_t*)&ROM_config_settings[index], g_config.r[index]);
	}
}

/*******************************************************************************
* Function Name  : load_global_settings
* Input          : None
* Output         : None
* Return         : void
* Description    :

Loads stored configuration settings from the EEPROM

*******************************************************************************/
__attribute__((optimize("Os")))
void load_global_settings()
 {
	int index;
	
	// Load settings from EEPROM config array
	for( index = 0; index < CONFIG_ARRAY_SIZE; index++ )
	{
		g_config.r[index-CONFIG_REG_START_ADDRESS] = eeprom_read_dword((const uint32_t*)&ROM_config_settings[index]);
	}
	
	// If the register hasn't been initialized, load defaults.  This should only run once.
	if( g_config.r[REG_V1_SETTINGS-CONFIG_REG_START_ADDRESS] == 0xFFFFFFFF )
	{
		set_global_defaults();

		save_config();
	}
	else
	{
		// Copy into global variables for convenience
		copy_config_to_local();
	}
}

/*******************************************************************************
* Function Name  : update_outputs_from_config
* Input          : None
* Output         : None
* Return         : uint8_t
* Description    :

Takes the current configuration settings and sets up the power supply outputs
to correspond with the configuration.  This is used after a configuration packet
has been written to the Hydra, allowing a quick, easy update of all outputs to
reflect changes.  It is also used on startup to set up the outputs after loading
the configuration from the EEPROM.
*******************************************************************************/
__attribute__((optimize("Os")))
uint8_t update_outputs_from_config()
{
	// Set the digital potentiometers to their proper values.
	// If unable to set POT value, indicate that there was a fault and
	// make sure the supply doesn't turn on automatically
	// Also, only do the update if the supply is NOT in current control mode.
	// If in current control mode, the new voltage setting will be used automatically
	// in the correct way.
	update_output_voltage(1);
	update_output_voltage(2);
	update_output_voltage(3);

	// Set PWM/SYNC outputs
	if( g_config.V1_low_current == 1 )
	{
		set_low_current_mode(1);
	}
	else
	{
		set_high_current_mode(1);
	}
	
	if( g_config.V2_low_current == 1 )
	{
		set_low_current_mode(2);
	}
	else
	{
		set_high_current_mode(2);
	}
	
	if( g_config.V3_low_current == 1 )
	{
		set_low_current_mode(3);
	}
	else
	{
		set_high_current_mode(3);
	}
	
	// Enable supplies if they should be turned on
	// Only execute this code if we aren't in input voltage cutoff mode
	// Entering voltage cutoff mode requires a restart to turn power back on
	if( !g_data.voltage_cutoff_active || (g_data.voltage_cutoff_active && g_config.V1_cutoff_disabled) )
	{
		if( g_config.V1_enabled )
		{
			enable_output(1);
		}
		else
		{
			disable_output(1);
		}
	}
	
	if( !g_data.voltage_cutoff_active || (g_data.voltage_cutoff_active && g_config.V2_cutoff_disabled) )
	{
		if( g_config.V2_enabled )
		{
			enable_output(2);
		}
		else
		{
			disable_output(2);
		}
	}
		
	if( !g_data.voltage_cutoff_active || (g_data.voltage_cutoff_active && g_config.V3_cutoff_disabled) )
	{
		if( g_config.V3_enabled )
		{
			enable_output(3);
		}
		else
		{
			disable_output(3);
		}
	}

	return 0;
}

/*******************************************************************************
* Function Name  : update_output_voltage
* Input          : None
* Output         : None
* Return         : void
* Description    :

Updates the output voltage for the specified supply.  Uses the g_config.POTn_taps
variable to set the supply

*******************************************************************************/
__attribute__((optimize("Os")))
uint8_t update_output_voltage(uint8_t supply)
{	
	if( supply == 1 )
	{
		if( set_digital_pot( PWR1_SLAVE_ADDRESS, g_config.POT1_taps ) )
		{
			g_data.out1_fault = POT_FAULT;
			copy_data_to_output();
			
			g_supply1_status = g_status_fault_pot_string;
			
			return 1;
		}
		else
		{
			g_data.out1_fault = 0;
		}
	}
	else if( supply == 2 )
	{
		if( set_digital_pot( PWR2_SLAVE_ADDRESS, g_config.POT2_taps ) )
		{
			g_data.out2_fault = POT_FAULT;
			copy_data_to_output();
			
			g_supply2_status = g_status_fault_pot_string;
			
			return 1;
		}
		else
		{
			g_data.out2_fault = 0;
		}
	}
	else if( supply == 3 )
	{
		if( set_digital_pot( PWR3_SLAVE_ADDRESS, g_config.POT3_taps ) )
		{
			g_data.out3_fault = POT_FAULT;
			copy_data_to_output();
			
			g_supply3_status = g_status_fault_pot_string;
			
			return 1;
		}
		else
		{
			g_data.out3_fault = 0;
		}
	}
	else
	{
		return 1;
	}
	
	return 0;
}

/*******************************************************************************
* Function Name  : copy_config_to_local
* Input          : None
* Output         : None
* Return         : void
* Description    :

Uses the g_config data array to extract convenient local variables to indicate
what the power supply configuration is.

*******************************************************************************/
void copy_config_to_local( )
{
	// Get target voltages
	g_config.V1_target = ((g_config.r[REG_V1_SETTINGS-CONFIG_REG_START_ADDRESS] >> VTARGET_START_BIT) & VTARGET_MASK);
	g_config.V2_target = ((g_config.r[REG_V2_SETTINGS-CONFIG_REG_START_ADDRESS] >> VTARGET_START_BIT) & VTARGET_MASK);
	g_config.V3_target = ((g_config.r[REG_V3_SETTINGS-CONFIG_REG_START_ADDRESS] >> VTARGET_START_BIT) & VTARGET_MASK);
	
	// Input voltage cutoff
	g_config.Vin_cutoff = ((g_config.r[REG_VIN_SETTINGS-CONFIG_REG_START_ADDRESS] >> VIN_VOLTAGE_START_BIT) & VIN_VOLTAGE_MASK);
	if( g_config.r[REG_VIN_SETTINGS-CONFIG_REG_START_ADDRESS] & ((uint32_t)1 << V1_CUTOFF_DISABLE_BIT) )
	{
		g_config.V1_cutoff_disabled = 1;
	}
	else
	{
		g_config.V1_cutoff_disabled = 0;
	}
	
	if( g_config.r[REG_VIN_SETTINGS-CONFIG_REG_START_ADDRESS] & ((uint32_t)1 << V2_CUTOFF_DISABLE_BIT) )
	{
		g_config.V2_cutoff_disabled = 1;
	}
	else
	{
		g_config.V2_cutoff_disabled = 0;
	}
	
	if( g_config.r[REG_VIN_SETTINGS-CONFIG_REG_START_ADDRESS] & ((uint32_t)1 << V3_CUTOFF_DISABLE_BIT) )
	{
		g_config.V3_cutoff_disabled = 1;
	}
	else
	{
		g_config.V3_cutoff_disabled = 0;
	}
	
	// Calibration coefficients
	g_config.V1_C1 = (int32_t)g_config.r[REG_V1_C1-CONFIG_REG_START_ADDRESS];
	g_config.V1_C2 = (int32_t)g_config.r[REG_V1_C2-CONFIG_REG_START_ADDRESS];
	g_config.V1_C3 = (int32_t)g_config.r[REG_V1_C3-CONFIG_REG_START_ADDRESS];
	
	g_config.V2_C1 = (int32_t)g_config.r[REG_V2_C1-CONFIG_REG_START_ADDRESS];
	g_config.V2_C2 = (int32_t)g_config.r[REG_V2_C2-CONFIG_REG_START_ADDRESS];
	g_config.V2_C3 = (int32_t)g_config.r[REG_V2_C3-CONFIG_REG_START_ADDRESS];
	
	g_config.V3_C1 = (int32_t)g_config.r[REG_V3_C1-CONFIG_REG_START_ADDRESS];
	g_config.V3_C2 = (int32_t)g_config.r[REG_V3_C2-CONFIG_REG_START_ADDRESS];
	g_config.V3_C3 = (int32_t)g_config.r[REG_V3_C3-CONFIG_REG_START_ADDRESS];
	
	// Get POT taps
	g_config.POT1_taps = get_POT_taps( g_config.V1_target, 1 );
	g_config.POT2_taps = get_POT_taps( g_config.V2_target, 2 );
	g_config.POT3_taps = get_POT_taps( g_config.V3_target, 3 );
	
	// Get current limits
	g_config.C1_limit = ((g_config.r[REG_V1_SETTINGS-CONFIG_REG_START_ADDRESS] >> IOUT_MAX_START_BIT) & IOUT_MAX_MASK);
	g_config.C2_limit = ((g_config.r[REG_V2_SETTINGS-CONFIG_REG_START_ADDRESS] >> IOUT_MAX_START_BIT) & IOUT_MAX_MASK);
	g_config.C3_limit = ((g_config.r[REG_V3_SETTINGS-CONFIG_REG_START_ADDRESS] >> IOUT_MAX_START_BIT) & IOUT_MAX_MASK);
	
	// Get enabled flags
	if( g_config.r[REG_V1_SETTINGS-CONFIG_REG_START_ADDRESS] & OUTPUT_ENABLED_BIT )
	{
		g_config.V1_enabled = 1;
	}
	else
	{
		g_config.V1_enabled = 0;
	}
	
	if( g_config.r[REG_V2_SETTINGS-CONFIG_REG_START_ADDRESS] & OUTPUT_ENABLED_BIT )
	{
		g_config.V2_enabled = 1;
	}
	else
	{
		g_config.V2_enabled = 0;
	}
	
	if( g_config.r[REG_V3_SETTINGS-CONFIG_REG_START_ADDRESS] & OUTPUT_ENABLED_BIT )
	{
		g_config.V3_enabled = 1;
	}
	else
	{
		g_config.V3_enabled = 0;
	}
	
	// Get low-current flags
	if( g_config.r[REG_V1_SETTINGS-CONFIG_REG_START_ADDRESS] & LOW_CURRENT_MODE_BIT )
	{
		g_config.V1_low_current = 1;
	}
	else
	{
		g_config.V1_low_current = 0;
	}
	
	if( g_config.r[REG_V2_SETTINGS-CONFIG_REG_START_ADDRESS] & LOW_CURRENT_MODE_BIT )
	{
		g_config.V2_low_current = 1;
	}
	else
	{
		g_config.V2_low_current = 0;
	}
	
	if( g_config.r[REG_V3_SETTINGS-CONFIG_REG_START_ADDRESS] & LOW_CURRENT_MODE_BIT )
	{
		g_config.V3_low_current = 1;
	}
	else
	{
		g_config.V3_low_current = 0;
	}
}

/*******************************************************************************
* Function Name  : copy_local_to_config
* Input          : None
* Output         : None
* Return         : void
* Description    :

Uses global configuration variables to set the g_config array for easy COM
access to configuration settings.

*******************************************************************************/
void copy_local_to_config( )
{
	// Settings for supply 1
	g_config.r[REG_V1_SETTINGS-CONFIG_REG_START_ADDRESS] = (((uint32_t)g_config.V1_target & VTARGET_MASK) << VTARGET_START_BIT) | (((uint32_t)g_config.C1_limit & IOUT_MAX_MASK) << IOUT_MAX_START_BIT);
	if( g_config.V1_enabled )
	{
		g_config.r[REG_V1_SETTINGS-CONFIG_REG_START_ADDRESS] |= OUTPUT_ENABLED_BIT;
	}
	if( g_config.V1_low_current )
	{
		g_config.r[REG_V1_SETTINGS-CONFIG_REG_START_ADDRESS] |= LOW_CURRENT_MODE_BIT;
	}
	
	g_config.r[REG_V1_C1-CONFIG_REG_START_ADDRESS] = g_config.V1_C1;
	g_config.r[REG_V1_C2-CONFIG_REG_START_ADDRESS] = g_config.V1_C2;
	g_config.r[REG_V1_C3-CONFIG_REG_START_ADDRESS] = g_config.V1_C3;
	
	// Settings for supply 2
	g_config.r[REG_V2_SETTINGS-CONFIG_REG_START_ADDRESS] = (((uint32_t)g_config.V2_target & VTARGET_MASK) << VTARGET_START_BIT) | (((uint32_t)g_config.C2_limit & IOUT_MAX_MASK) << IOUT_MAX_START_BIT);
	if( g_config.V2_enabled )
	{
		g_config.r[REG_V2_SETTINGS-CONFIG_REG_START_ADDRESS] |= OUTPUT_ENABLED_BIT;
	}
	if( g_config.V2_low_current )
	{
		g_config.r[REG_V2_SETTINGS-CONFIG_REG_START_ADDRESS] |= LOW_CURRENT_MODE_BIT;
	}
	
	g_config.r[REG_V2_C1-CONFIG_REG_START_ADDRESS] = g_config.V2_C1;
	g_config.r[REG_V2_C2-CONFIG_REG_START_ADDRESS] = g_config.V2_C2;
	g_config.r[REG_V2_C3-CONFIG_REG_START_ADDRESS] = g_config.V2_C3;
	
	// Settings for supply 3
	g_config.r[REG_V3_SETTINGS-CONFIG_REG_START_ADDRESS] = (((uint32_t)g_config.V3_target & VTARGET_MASK) << VTARGET_START_BIT) | (((uint32_t)g_config.C3_limit & IOUT_MAX_MASK) << IOUT_MAX_START_BIT);
	if( g_config.V3_enabled )
	{
		g_config.r[REG_V3_SETTINGS-CONFIG_REG_START_ADDRESS] |= OUTPUT_ENABLED_BIT;
	}
	if( g_config.V3_low_current )
	{
		g_config.r[REG_V3_SETTINGS-CONFIG_REG_START_ADDRESS] |= LOW_CURRENT_MODE_BIT;
	}
	
	g_config.r[REG_V3_C1-CONFIG_REG_START_ADDRESS] = g_config.V3_C1;
	g_config.r[REG_V3_C2-CONFIG_REG_START_ADDRESS] = g_config.V3_C2;
	g_config.r[REG_V3_C3-CONFIG_REG_START_ADDRESS] = g_config.V3_C3;
	
	// Voltage input settings
	g_config.r[REG_VIN_SETTINGS-CONFIG_REG_START_ADDRESS] = ((uint32_t)g_config.Vin_cutoff >> VIN_VOLTAGE_START_BIT) & VIN_VOLTAGE_MASK;
	
	if( g_config.V1_cutoff_disabled )
	{
		g_config.r[REG_VIN_SETTINGS-CONFIG_REG_START_ADDRESS] |= ((uint32_t)1 << V1_CUTOFF_DISABLE_BIT);
	}
	
	if( g_config.V2_cutoff_disabled )
	{
		g_config.r[REG_VIN_SETTINGS-CONFIG_REG_START_ADDRESS] |= ((uint32_t)1 << V2_CUTOFF_DISABLE_BIT);
	}
	
	if( g_config.V3_cutoff_disabled )
	{
		g_config.r[REG_VIN_SETTINGS-CONFIG_REG_START_ADDRESS] |= ((uint32_t)1 << V3_CUTOFF_DISABLE_BIT);
	}
}

/*******************************************************************************
* Function Name  : copy_data_to_output
* Input          : None
* Output         : None
* Return         : void
* Description    : 

Copies data to the output data array so that it can be transmitted easily
when in binary data mode

*******************************************************************************/
void copy_data_to_output( )
{
	g_data.r[V1_STATUS-DATA_REG_START_ADDRESS] = (((uint32_t)g_data.vout1 & VOLTAGE_MASK) << VOLTAGE_START_BIT) | ((uint32_t)((uint32_t)g_data.iout1 & CURRENT_MASK) << CURRENT_START_BIT);
	g_data.r[V2_STATUS-DATA_REG_START_ADDRESS] = (((uint32_t)g_data.vout2 & VOLTAGE_MASK) << VOLTAGE_START_BIT) | ((uint32_t)((uint32_t)g_data.iout2 & CURRENT_MASK) << CURRENT_START_BIT);
	g_data.r[V3_STATUS-DATA_REG_START_ADDRESS] = (((uint32_t)g_data.vout3 & VOLTAGE_MASK) << VOLTAGE_START_BIT) | ((uint32_t)((uint32_t)g_data.iout3 & CURRENT_MASK) << CURRENT_START_BIT);
	g_data.r[VIN_STATUS-DATA_REG_START_ADDRESS] = ((uint32_t)g_data.vin & VIN_VOLTAGE_MASK) << VIN_VOLTAGE_START_BIT;
	
	// If voltage cutoff is active, set the bit
	if( g_data.voltage_cutoff_active )
	{
		g_data.r[VIN_STATUS-DATA_REG_START_ADDRESS] |= ((uint32_t)1 << VOLTAGE_CUTOFF_ACTIVE_BIT);
	}
	
	// Set CV and CC bits for each supply
	if( g_config.V1_enabled )
	{
		if( g_data.out1_cc )
		{
			g_data.r[V1_STATUS-DATA_REG_START_ADDRESS] |= (uint32_t)CC_BIT;
		}
		else
		{
			g_data.r[V1_STATUS-DATA_REG_START_ADDRESS] |= (uint32_t)CV_BIT;
		}
	}
	
	if( g_config.V2_enabled )
	{
		if( g_data.out2_cc )
		{
			g_data.r[V2_STATUS-DATA_REG_START_ADDRESS] |= (uint32_t)CC_BIT;
		}
		else
		{
			g_data.r[V2_STATUS-DATA_REG_START_ADDRESS] |= (uint32_t)CV_BIT;
		}
	}
	
	if( g_config.V3_enabled )
	{
		if( g_data.out3_cc )
		{
			g_data.r[V3_STATUS-DATA_REG_START_ADDRESS] |= (uint32_t)CC_BIT;
		}
		else
		{
			g_data.r[V3_STATUS-DATA_REG_START_ADDRESS] |= (uint32_t)CV_BIT;
		}
	}
	
	// Clear/set fault bits for each supply
	if( g_data.out1_fault )
	{
		g_data.r[V1_STATUS-DATA_REG_START_ADDRESS] |= (uint32_t)((uint32_t)(g_data.out1_fault & FAULT_MASK) << FAULT_START_BIT);
	}
	
	if( g_data.out2_fault )
	{
		g_data.r[V2_STATUS-DATA_REG_START_ADDRESS] |= (uint32_t)((uint32_t)(g_data.out2_fault & FAULT_MASK) << FAULT_START_BIT);
	}
	
	if( g_data.out3_fault )
	{
		g_data.r[V3_STATUS-DATA_REG_START_ADDRESS] |= (uint32_t)((uint32_t)(g_data.out3_fault & FAULT_MASK) << FAULT_START_BIT);
	}
	
	// Copy the local configuration out as well
	copy_local_to_config();
}

/*******************************************************************************
* Function Name  : get_POT_taps
* Input          : int32_t voltageMV
* Output         : None
* Return         : pot taps required to get desired voltage
* Description    :

Converts the desired voltage into a POT_taps setting to get the desired voltage

*******************************************************************************/
uint16_t get_POT_taps( int32_t voltageMV, uint8_t supply_number )
{
	int32_t C1,C2,C3;
	
	get_coefficients(supply_number,&C1,&C2,&C3);
	
	// Make sure voltageMV is in the supported voltage range
	if( voltageMV < MIN_OUTPUT_VOLTAGE_MV )
	{
		voltageMV = MIN_OUTPUT_VOLTAGE_MV;
	}
	if( voltageMV > MAX_OUTPUT_VOLTAGE_MV )
	{
		voltageMV = MAX_OUTPUT_VOLTAGE_MV;
	}
	
	int32_t temp = C1/((int32_t)voltageMV+C2);
	
	if( (temp + C3) < 0 )
	{
		return (uint16_t)0;
	}
	
	temp += C3;
	
	if( temp > 1024 )
	{
		return 1024;
	}
	
	return (uint16_t)temp;
}

/*******************************************************************************
* Function Name  : get_MV_from_taps
* Input          : uint16_t POT_taps
* Output         : None
* Return         : Estimated voltage in millivolts
* Description    :

// Function takes the POT taps and computes the expected output voltage in millivolts

*******************************************************************************/
int32_t get_MV_from_taps( uint16_t POT_taps, uint8_t supply_number )
{
	int32_t C1,C2,C3;
	
	get_coefficients(supply_number,&C1,&C2,&C3);
	
	// return (int32_t)(2867200/(POT_taps + 256) + 800);
	return (int32_t)(C1/(POT_taps - C3) - C2);
}

/*******************************************************************************
* Function Name  : get_coefficients
* Input          : void
* Output         : None
* Return         : 0 on success, 1 on failure
* Description    :

Gets the coefficients setting the output voltage on the specified supply

*******************************************************************************/
void get_coefficients(uint8_t supply_number, int32_t* C1, int32_t* C2, int32_t* C3 )
{
	if( supply_number == 1 )
	{
		*C1 = g_config.V1_C1;
		*C2 = g_config.V1_C2;
		*C3 = g_config.V1_C3;
	}
	else if( supply_number == 2 )
	{
		*C1 = g_config.V2_C1;
		*C2 = g_config.V2_C2;
		*C3 = g_config.V2_C3;
	}
	else if( supply_number == 3 )
	{
		*C1 = g_config.V3_C1;
		*C2 = g_config.V3_C2;
		*C3 = g_config.V3_C3;
	}
	else
	{
		*C1 = 0;
		*C2 = 0;
		*C3 = 0;
	}

}

/*******************************************************************************
* Function Name  : enable_output
* Input          :  uint8_t output_number
* Output         : None
* Return         : None
* Description    :

Enables the specified supply.  The input output_number should be 1, 2, or 3

*******************************************************************************/
void enable_output( uint8_t output_number )
{
	switch(output_number)
	{
		case 1:
			RUN1_PORT |= RUN1_MASK;
			
			g_config.V1_enabled = 1;
			g_data.out1_cc = 0;
			g_supply1_status = g_status_on_cv_string;
		break;
		
		case 2:
			RUN2_PORT |= RUN2_MASK;
			
			g_config.V2_enabled = 1;
			g_data.out2_cc = 0;
			g_supply2_status = g_status_on_cv_string;
		break;
		
		case 3:
			RUN3_PORT |= RUN3_MASK;
			
			g_config.V3_enabled = 1;
			g_data.out3_cc = 0;
			g_supply3_status = g_status_on_cv_string;
		break;
	}
}

/*******************************************************************************
* Function Name  : disable_output
* Input          :  uint8_t output_number
* Output         : None
* Return         : None
* Description    :

Disables the specified supply.  The input output_number should be 1, 2, or 3

*******************************************************************************/
void disable_output( uint8_t output_number )
{
	switch(output_number)
	{
		case 1:
			RUN1_PORT &= ~RUN1_MASK;
			
			g_config.V1_enabled = 0;
			g_data.out1_cc = 0;			
			g_supply1_status = g_status_off_string;
		break;
		
		case 2:
			RUN2_PORT &= ~RUN2_MASK;
			
			g_config.V2_enabled = 0;
			g_data.out2_cc = 0;
			g_supply2_status = g_status_off_string;
		break;
		
		case 3:
			RUN3_PORT &= ~RUN3_MASK;
			
			g_config.V3_enabled = 0;
			g_data.out3_cc = 0;
			g_supply3_status = g_status_off_string;
		break;
	}
}

/*******************************************************************************
* Function Name  : disable_all
* Input          :  None
* Output         : None
* Return         : None
* Description    :

Disables all power supply outputs

*******************************************************************************/
void disable_all( void )
{
	disable_output(1);
	disable_output(2);
	disable_output(3);
}

/*******************************************************************************
* Function Name  : disable_all_temp
* Input          :  None
* Output         : None
* Return         : None
* Description    :

Disables all power supply outputs without writing the changes to configuration.
This is used when we need to disable the outputs on an input voltage cutoff
condition, so that when the power is cycled, the enabled supplies are still enabled.

As far as the rest of the code is concerned, these supplies are still enabled...

*******************************************************************************/
void disable_all_temp( void )
{
	disable_output_temp(1);
	disable_output_temp(2);
	disable_output_temp(3);
}

/*******************************************************************************
* Function Name  : disable_output_temp
* Input          :  uint8_t output_number
* Output         : None
* Return         : None
* Description    :


Disables the specified power supply output without writing the changes to configuration.
This is used when we need to disable the outputs on an input voltage cutoff
condition, so that when the power is cycled, the enabled supplies are still enabled.

As far as the rest of the code is concerned, these supplies are still enabled...

*******************************************************************************/
void disable_output_temp( uint8_t output_number )
{
	switch(output_number)
	{
		case 1:
		RUN1_PORT &= ~RUN1_MASK;

		g_data.out1_cc = 0;
		g_supply1_status = g_status_off_string;
		break;
		
		case 2:
		RUN2_PORT &= ~RUN2_MASK;

		g_data.out2_cc = 0;
		g_supply2_status = g_status_off_string;
		break;
		
		case 3:
		RUN3_PORT &= ~RUN3_MASK;

		g_data.out3_cc = 0;
		g_supply3_status = g_status_off_string;
		break;
	}
}

/*******************************************************************************
* Function Name  : set_low_current_mode
* Input          :  None
* Output         : None
* Return         : None
* Description    :

Turns on low current mode for the specified supply (1, 2, or 3)

*******************************************************************************/
void set_low_current_mode( uint8_t output_number )
{
	switch(output_number)
	{
		case 1:
			SYNC1_PORT &= ~SYNC1_MASK;
		break;
		
		case 2:
			SYNC2_PORT &= ~SYNC2_MASK;
		break;
		
		case 3:
			SYNC3_PORT &= ~SYNC3_MASK;
		break;
	}
}

/*******************************************************************************
* Function Name  : set_high_current_mode
* Input          :  None
* Output         : None
* Return         : None
* Description    :

Turns off low current mode for the specified supply (1, 2, or 3)

*******************************************************************************/
void set_high_current_mode( uint8_t output_number )
{
	switch(output_number)
	{
		case 1:
			SYNC1_PORT |= SYNC1_MASK;
		break;
		
		case 2:
			SYNC2_PORT |= SYNC2_MASK;
		break;
		
		case 3:
			SYNC3_PORT |= SYNC3_MASK;
		break;
	}
}
