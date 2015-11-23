#pragma once

#define IOUT_REG_SCALE		0.001f
#define VOUT_REG_SCALE		0.001f

#define V1_SETTINGS_REG_ADDRESS		0
#define V2_SETTINGS_REG_ADDRESS		1
#define V3_SETTINGS_REG_ADDRESS		2
#define VIN_SETTINGS_REG_ADDRESS	3

#define CHANGE_PENDING_COLOR		System::Drawing::Color::LightYellow
#define SUPPLY_OUTPUT_GOOD_COLOR	System::Drawing::Color::LightGreen
#define SUPPLY_DISABLED_COLOR		System::Drawing::Color::WhiteSmoke
#define HYDRA_DISCONNECTED_COLOR	System::Drawing::Color::LightGray
#define HYDRA_INPUT_CUTOFF_COLOR	System::Drawing::Color::Salmon

#define CAL_STATE_BEGIN				0
#define CAL_STATE_SET_DEFAULTS		1
#define CAL_STATE_V1				3
#define CAL_STATE_V2				4
#define CAL_STATE_V3				5
#define CAL_STATE_VALIDATE1			6
#define CAL_STATE_VALIDATE2			7
#define CAL_STATE_VALIDATE3			8
#define CAL_STATE_FINISH			9

#define DEFAULT_C1					2150400.0f
#define DEFAULT_C2					-800.0f
#define DEFAULT_C3					-162.0f

namespace SmartPowerSupply {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for Form1
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Form1(void)
		{
			InitializeComponent();
			
			output1_voltage = 0.0f;
			output2_voltage = 0.0f;
			output3_voltage = 0.0f;

			output1_current = 0.0f;
			output2_current = 0.0f;
			output3_current = 0.0f;

			output1_enabled = 0;
			output2_enabled = 0;
			output3_enabled = 0;

			low_current1_enabled = 0;
			low_current2_enabled = 0;
			low_current3_enabled = 0;

			iout1_max = 2.5f;
			iout2_max = 2.5f;
			iout3_max = 2.5f;

			v1_target = 3.0f;
			v2_target = 3.0f;
			v3_target = 3.0f;

			out1_changing = 0;
			out1_changed = 0;
			out2_changing = 0;
			out2_changed = 0;
			out3_changing = 0;
			out3_changed = 0;

			vin_changed = 0;

			vin_cutoff_active = 0;
			vin_cutoff = 5.0f;

			cutoff1_override = 0;
			cutoff2_override = 0;
			cutoff3_override = 0;

			CAL_state = CAL_STATE_BEGIN;

			// received_a_packet flag.  This is used to track whether the Hydra is still connected to the serial port.
			// If the serial port is connected and the timeout fires, and received_a_packet is still 0, then something is wrong.
			received_a_packet = 0;

			// Local member variable tracks the serial port connection.  If the serial connector disconnects without this flag being changed, then
			// something unexpected happened (like maybe the Hydra was unplugged and its port disappeared).
			serial_port_connected = 0;

			// Flag indicating whether we have config data for the supply
			have_config_data = 0;

			serialConnector = gcnew SerialConnector();

			packetsToSend = gcnew cli::array<SerialPacket^>(100);
			this->packetCount = 0;
			this->packetRetryCount = 0;

			// Add event handlers for dealing with packets
			serialConnector->OnSerialPacketError += gcnew SerialPacketErrorEventHandler( this, &Form1::SerialPacketError_eventHandler );
			serialConnector->OnSerialPacketReceived += gcnew SerialPacketReceivedEventHandler( this, &Form1::SerialPacketReceived_eventHandler );
	
			// Populate combo boxes for serial communication settings
			cli::array<String^>^ portNames = serialConnector->GetPortNames();
			
			if( portNames->GetLength(0) > 0 )
			{
				for each( String^ portName in portNames )
				{
					this->serialComboBox->Items->Add( portName );
				}
			}
			else
			{
				System::String^ no_ports = gcnew System::String("None");
				this->serialComboBox->Items->Add( no_ports );
			}
			
			this->serialComboBox->SelectedIndex = 0;
			 
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
			try
			{
				serialConnector->Close();
			}
			catch( Exception^ /*e*/ )
			{
			}

			if (components)
			{
				delete components;
			}
		}

		private: delegate void UpdateOutputDelegate( SerialPacket^ new_packet );
				 delegate void UpdateConfigDelegate( SerialPacket^ packet );
				 delegate void NoArgumentDelegate( void );
				 delegate void StringDelegate( System::String^ text );
				 delegate void EventHandlerDelegate( System::Object^  sender, System::EventArgs^  e );

	private:
		SerialConnector^ serialConnector;
		private: cli::array<SerialPacket^>^ packetsToSend;
				 int packetCount;
				 int packetRetryCount;
	
	public:
		// Supply data
		float output1_voltage;
		float output2_voltage;
		float output3_voltage;

		float output1_current;
		float output2_current;
		float output3_current;

		// Supply status indicators
		int output1_enabled, output2_enabled, output3_enabled;
		int low_current1_enabled, low_current2_enabled, low_current3_enabled;
		float iout1_max, iout2_max, iout3_max;
		float v1_target, v2_target, v3_target;
		float vin_cutoff;
		int cutoff1_override, cutoff2_override, cutoff3_override;
	
	private:
		// Flags for UI control
		int received_a_packet;
		int serial_port_connected;
		int have_config_data;
		int out1_changing;
		int out2_changing;
		int out3_changing;

		int out1_changed;
		int out2_changed;
		int out3_changed;

		int vin_changed;

		int vin_cutoff_active;

		// Variable for storing the state of the calibration code
		int CAL_state;

		// Variables for storing calibration data
		float cal_out1_v1;
		float cal_out1_v2;
		float cal_out1_v3;

		float cal_out2_v1;
		float cal_out2_v2;
		float cal_out2_v3;

		float cal_out3_v1;
		float cal_out3_v2;
		float cal_out3_v3;

		float cal_x1;
		float cal_x2;
		float cal_x3;

		// Calibration coefficients
		Int32 cal_out1_c1;
		Int32 cal_out1_c2;
		Int32 cal_out1_c3;

		Int32 cal_out2_c1;
		Int32 cal_out2_c2;
		Int32 cal_out2_c3;

		Int32 cal_out3_c1;
		Int32 cal_out3_c2;
		Int32 cal_out3_c3;

		// Variables for storing errors in the calibration results
		float out1_error;
		float out2_error;
		float out3_error;


	protected: 

	private: System::Windows::Forms::CheckBox^  output3_enabled_checkbox;

	private: System::Windows::Forms::CheckBox^  output2_enabled_checkbox;

	private: System::Windows::Forms::CheckBox^  output1_enabled_checkbox;










	private: System::Windows::Forms::TextBox^  output3_current_text;

	private: System::Windows::Forms::TextBox^  output3_voltage_text;
	private: System::Windows::Forms::TextBox^  output2_current_text;


	private: System::Windows::Forms::TextBox^  output2_voltage_text;

	private: System::Windows::Forms::TextBox^  output1_current_text;


	private: System::Windows::Forms::TextBox^  output1_voltage_text;



















	private: System::Windows::Forms::TrackBar^  output1_trackbar;

	private: System::Windows::Forms::TrackBar^  output2_trackbar;
	private: System::Windows::Forms::TrackBar^  output3_trackbar;




















private: System::Windows::Forms::Label^  portLabel;
private: System::Windows::Forms::ComboBox^  serialComboBox;
private: System::Windows::Forms::Button^  disconnectButton;
private: System::Windows::Forms::Button^  connectButton;

private: System::Windows::Forms::Label^  statusLabelP;
private: System::Windows::Forms::Label^  statusLabel;
private: System::Windows::Forms::Timer^  COM_timeout_timer;
private: System::Windows::Forms::TrackBar^  max_current3_trackbar;
private: System::Windows::Forms::TrackBar^  max_current2_trackbar;
private: System::Windows::Forms::TrackBar^  max_current1_trackbar;








private: System::Windows::Forms::GroupBox^  groupBox2;
private: System::Windows::Forms::Label^  v1_status_label;
private: System::Windows::Forms::Label^  v1_target_label;
private: System::Windows::Forms::Label^  c1_max_label;

private: System::Windows::Forms::Label^  c1_status_label;
private: System::Windows::Forms::CheckBox^  output3_lowc_enabled_Checkbox;
private: System::Windows::Forms::CheckBox^  output2_lowc_enabled_Checkbox;
private: System::Windows::Forms::CheckBox^  output1_lowc_enabled_Checkbox;
private: System::Windows::Forms::Label^  v3_status_label;
private: System::Windows::Forms::Label^  v3_target_label;
private: System::Windows::Forms::Label^  c3_max_label;

private: System::Windows::Forms::Label^  c2_max_label;

private: System::Windows::Forms::Label^  c3_status_label;
private: System::Windows::Forms::Label^  c2_status_label;
private: System::Windows::Forms::Label^  v2_status_label;
private: System::Windows::Forms::Label^  v2_target_label;
private: System::Windows::Forms::GroupBox^  groupBox3;
private: System::Windows::Forms::GroupBox^  groupBox4;
private: System::Windows::Forms::TextBox^  input_voltage_text;
private: System::Windows::Forms::Label^  cutoff_text_label;


private: System::Windows::Forms::Label^  vin_cutoff_label;
private: System::Windows::Forms::GroupBox^  groupBox5;
private: System::Windows::Forms::Label^  label1;
private: System::Windows::Forms::GroupBox^  calGroupBox;

private: System::Windows::Forms::Button^  cal_cancel_button;
private: System::Windows::Forms::Button^  cal_start_button;
private: System::Windows::Forms::Button^  accept_button;

private: System::Windows::Forms::Label^  cal_status_label;
private: System::Windows::Forms::Label^  label2;
private: System::Windows::Forms::TextBox^  output3_error_text;

private: System::Windows::Forms::TextBox^  output2_error_text;

private: System::Windows::Forms::TextBox^  output1_error_text;
private: System::Windows::Forms::TextBox^  out2_c3_text;


private: System::Windows::Forms::TextBox^  out2_c2_text;

private: System::Windows::Forms::TextBox^  out2_c1_text;

private: System::Windows::Forms::TextBox^  out1_c3_text;

private: System::Windows::Forms::TextBox^  out1_c2_text;

private: System::Windows::Forms::TextBox^  out1_c1_text;
private: System::Windows::Forms::TextBox^  out3_c3_text;


private: System::Windows::Forms::TextBox^  out3_c2_text;

private: System::Windows::Forms::TextBox^  out3_c1_text;

private: System::Windows::Forms::Label^  label3;
private: System::Windows::Forms::Label^  label6;
private: System::Windows::Forms::Label^  label5;
private: System::Windows::Forms::Label^  label4;
private: System::Windows::Forms::Label^  label9;
private: System::Windows::Forms::Label^  label8;
private: System::Windows::Forms::Label^  label7;
private: System::Windows::Forms::Label^  label16;
private: System::Windows::Forms::Label^  label17;
private: System::Windows::Forms::Label^  label18;
private: System::Windows::Forms::Label^  label13;
private: System::Windows::Forms::Label^  label14;
private: System::Windows::Forms::Label^  label15;
private: System::Windows::Forms::Label^  label12;
private: System::Windows::Forms::Label^  label11;
private: System::Windows::Forms::Label^  label10;
private: System::Windows::Forms::Timer^  CAL_timer;
private: System::Windows::Forms::Timer^  packetResponseTimer;
private: System::Windows::Forms::MenuStrip^  menuStrip1;
private: System::Windows::Forms::ToolStripMenuItem^  toolStripMenuItem1;
private: System::Windows::Forms::ToolStripMenuItem^  exitMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  optionsToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  showCalibrationToolsMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  hideCalibrationToolsMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  helpToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  aboutMenuItem;
private: System::Windows::Forms::LinkLabel^  cal_help_label;
private: System::Windows::Forms::Label^  label19;
private: System::Windows::Forms::ComboBox^  baudComboBox;
private: System::Windows::Forms::ToolStripMenuItem^  serialPortSettingsToolStripMenuItem;



















	private: System::ComponentModel::IContainer^  components;




	protected: 















	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>

		private:

		void SerialPacketError_eventHandler( String^ text )
		{
			// TODO: Add serial error handler code
		}
		
		/* -------------------------------------------------------------------------------------------------------
		* Name: SerialPacketReceived_eventHandler( SerialPacket^ packet )
		* Description:	
		*	Event handler called when a serial packet has been received
		------------------------------------------------------------------------------------------------------- */
		void SerialPacketReceived_eventHandler( SerialPacket^ packet )
		{


			if( this->InvokeRequired )
			{
				cli::array<System::Object^>^ args = gcnew cli::array<System::Object^>(1);
				args[0] = packet;

				this->BeginInvoke( gcnew UpdateConfigDelegate( this, &Form1::SerialPacketReceived_eventHandler ),  args);
			}
			else
			{

				// First, check to see if this packet acknowledges the reception of an earlier packet transmission
				if( this->packetCount > 0 && (packet->Address == packetsToSend[0]->Address) )
				{
					// Stop the packet wait timer
					this->packetResponseTimer->Stop();

					// Shift other packets in the buffer down.
					for( int i = 1; i < this->packetCount; i++ )
					{
						packetsToSend[i-1] = packetsToSend[i];
					}

					this->packetCount--;
					this->packetRetryCount = 0;

					// If there are still packets to send, then start the timer and send the next one
					if( packetCount > 0 )
					{
						if( serialConnector->IsOpen )
						{
							try
							{
								serialConnector->TransmitPacket( packetsToSend[0] );
								this->packetResponseTimer->Start();
							}
							catch( Exception^ /*e*/ )
							{
								
							}
						}
					}
				}


				// Check to see if this is a data packet and if it contains all the data we expect to receive
				if( packet->Address == 85 && packet->IsBatch && packet->BatchLength == 4 )
				{
					changeOutput_safe(packet);

					received_a_packet = 1;

					// Check to see if we have updated config data for the supply.  If not, send a packet requesting the data.
					// We do this here so that we can guarantee that the Hydra is in Binary mode before sending the config request.
					// (we won't have received a packet if not in binary mode)
					// Also, we can be sure that we'll get here if the Hydra is working properly because it is automatically transmitting data like it is supposed to.
					// Finally, putting the request here is a convenient way to have the request sent twice a second for as long as we don't have the config data,
					// because the Hydra transmits twice a second by default.
					if( this->have_config_data == 0 )
					{
						SerialPacket^ request = gcnew SerialPacket();

						request->Address = 0;
						request->IsBatch = true;
						request->BatchLength = 4;
						request->ComputeChecksum();

						this->AddTXPacket(request);
					}
				}
				// Check to see if we received a packet containing configuration information.
				else if( (packet->Address == 0) && packet->IsBatch && (packet->BatchLength == 4) )
				{
					updateAllConfig_safe( packet );
				}
				// Check to see if we've received a packet acknowledging that supply 1 has been configured
				else if( (packet->Address == 0) && !packet->HasData )
				{
					this->out1_changing = 0;
					this->out1_changed = 0;

					if( this->output1_enabled )
					{
						change_out1_back_color(SUPPLY_OUTPUT_GOOD_COLOR);
					}
					else
					{
						change_out1_back_color(SUPPLY_DISABLED_COLOR);
					}

					// Initiate a read to get the new values back
					this->have_config_data = 0;
				}
				// Check to see if we've received a packet acknowledging that supply 2 has been configured
				else if( (packet->Address == 1) && !packet->HasData )
				{
					this->out2_changing = 0;
					this->out2_changed = 0;

					if( this->output2_enabled )
					{
						change_out2_back_color(SUPPLY_OUTPUT_GOOD_COLOR);
					}
					else
					{
						change_out2_back_color(SUPPLY_DISABLED_COLOR);
					}

					// Initiate a read to get the new values back
					this->have_config_data = 0;
				}
				// Check to see if we've received a packet acknowledging that supply 3 has been configured
				else if( (packet->Address == 2) && !packet->HasData )
				{
					this->out3_changing = 0;
					this->out3_changed = 0;

					if( this->output3_enabled )
					{
						change_out3_back_color(SUPPLY_OUTPUT_GOOD_COLOR);
					}
					else
					{
						change_out3_back_color(SUPPLY_DISABLED_COLOR);
					}

					// Initiate a read to get the new values back
					this->have_config_data = 0;
				}
				// Check to see if we've received a packet acknowledging that vin has been configured
				else if( (packet->Address == 3) && !packet->HasData )
				{
					this->vin_changed = 0;

					if( this->vin_cutoff_active )
					{
						change_vin_back_color(HYDRA_INPUT_CUTOFF_COLOR);
					}
					else
					{
						change_vin_back_color(SUPPLY_OUTPUT_GOOD_COLOR);
					}

					update_cutoff_display();

					// Initiate a read to get the new values back
					this->have_config_data = 0;
				}
				// Check to see if we received a confirmation that calibration coefficients were written properly
				else if( (packet->Address == 4) && !packet->HasData )
				{
					// TODO: Add code to handle this case if it is needed.
				}


			}

		}

		/* -------------------------------------------------------------------------------------------------------
		* Name: update_calibration_status( System::String^ new_text )
		* Description:	
		*	Updates the calibration status string in a thread-safe manner
		* ------------------------------------------------------------------------------------------------------- */
private: void update_calibration_status( System::String^ new_text )
		 {
			 if( this->InvokeRequired )
			 {
				 cli::array<System::Object^>^ args = gcnew cli::array<System::Object^>(1);
				 args[0] = new_text;
				
				 this->BeginInvoke( gcnew StringDelegate( this, &Form1::update_calibration_status ),  args);
			 }
			 else
			 {
				 this->cal_status_label->Text = new_text;
			 }
		 }

/* -------------------------------------------------------------------------------------------------------
		* Name: update_cutoff_display( void )
		* Description:	
		*	If input voltage cutoff is active, sets the enabled/disabled states of voltage controls based on whether
		*   the override is enabled or not.
		* ------------------------------------------------------------------------------------------------------- */
private: void update_cutoff_display( void )
		 {
			 // If voltage cutoff is active, enable and disable the individual supplies as needed
			if( this->vin_cutoff_active )
			{
				if( this->cutoff1_override )
				{
					enable_v1_controls();
				}
				else
				{
					disable_v1_controls();
				}

				if( this->cutoff2_override )
				{
					enable_v2_controls();
				}
				else
				{
					disable_v2_controls();
				}

				if( this->cutoff3_override )
				{
					enable_v3_controls();
				}
				else
				{
					disable_v3_controls();
				}
			}

		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: updateAllConfig_safe( SerialPacket^ packet )
		* Description:	
		*	Function call for updating the displayed configuration options for the connected Hydra
		* ------------------------------------------------------------------------------------------------------- */
		void updateAllConfig_safe( SerialPacket^ packet )
		{
			if( this->InvokeRequired )
			{
				cli::array<System::Object^>^ args = gcnew cli::array<System::Object^>(1);
				args[0] = packet;
				
				this->BeginInvoke( gcnew UpdateConfigDelegate( this, &Form1::updateAllConfig_safe ),  args);
			}
			else
			{
				// Extract the packet data, copy it to relevant local member variables, and update the interface to reflect the settings.
				UInt32 REG_V1_SETTINGS = (UInt32)packet->GetDataByte(0) << 24 | 
										(UInt32)packet->GetDataByte(1) << 16 | 
										(UInt32)packet->GetDataByte(2) << 8 | 
										(UInt32)packet->GetDataByte(3);
				UInt32 REG_V2_SETTINGS = (UInt32)packet->GetDataByte(4) << 24 | 
										(UInt32)packet->GetDataByte(5) << 16 | 
										(UInt32)packet->GetDataByte(6) << 8 | 
										(UInt32)packet->GetDataByte(7);
				UInt32 REG_V3_SETTINGS = (UInt32)packet->GetDataByte(8) << 24 | 
										(UInt32)packet->GetDataByte(9) << 16 | 
										(UInt32)packet->GetDataByte(10) << 8 | 
										(UInt32)packet->GetDataByte(11);
				UInt32 REG_VIN_SETTINGS = (UInt32)packet->GetDataByte(12) << 24 | 
										(UInt32)packet->GetDataByte(13) << 16 | 
										(UInt32)packet->GetDataByte(14) << 8 | 
										(UInt32)packet->GetDataByte(15);

				this->output1_enabled = (REG_V1_SETTINGS >> 31) & 0x01;
				this->output2_enabled = (REG_V2_SETTINGS >> 31) & 0x01;
				this->output3_enabled = (REG_V3_SETTINGS >> 31) & 0x01;
				
				this->low_current1_enabled = (REG_V1_SETTINGS >> 30) & 0x01;
				this->low_current2_enabled = (REG_V2_SETTINGS >> 30) & 0x01;
				this->low_current3_enabled = (REG_V3_SETTINGS >> 30) & 0x01;

				this->iout1_max = (float)((REG_V1_SETTINGS >> 16) & 0x0FFF);
				this->iout1_max *= IOUT_REG_SCALE;
				
				this->iout2_max = (float)((REG_V2_SETTINGS >> 16) & 0x0FFF);
				this->iout2_max *= IOUT_REG_SCALE;

				this->iout3_max = (float)((REG_V3_SETTINGS >> 16) & 0x0FFF);
				this->iout3_max *= IOUT_REG_SCALE;

				this->v1_target = float(REG_V1_SETTINGS & 0x0FFFF);
				this->v1_target *= VOUT_REG_SCALE;

				this->v2_target = float(REG_V2_SETTINGS & 0x0FFFF);
				this->v2_target *= VOUT_REG_SCALE;

				this->v3_target = float(REG_V3_SETTINGS & 0x0FFFF);
				this->v3_target *= VOUT_REG_SCALE;

				this->vin_cutoff = float( REG_VIN_SETTINGS & 0x0FFFF );
				this->vin_cutoff *= VOUT_REG_SCALE;

				this->cutoff1_override = (REG_VIN_SETTINGS >> 31) & 0x01;
				this->cutoff2_override = (REG_VIN_SETTINGS >> 30) & 0x01;
				this->cutoff3_override = (REG_VIN_SETTINGS >> 29) & 0x01;

				// Enable all the config controls unless the voltage cutoff is turned on
				if( this->vin_cutoff_active == 0 )
				{
					enable_voltage_controls();
				}

				updateConfigDisplay();

				// Now that we've received and applied config data, set flag to indicate that we are ready to do things.
				this->have_config_data = 1;
			}
		}

		/* -------------------------------------------------------------------------------------------------------
		* Name: updateConfigDisplay()
		* Description:	
		*	Uses local member variables to set the status of the outputs on the GUI
		------------------------------------------------------------------------------------------------------- */
		void updateConfigDisplay()
		{
			// Voltage and current settings
			this->v1_target_label->Text = System::String::Concat( gcnew String(this->v1_target.ToString("F2")), gcnew String("V") );
			this->c1_max_label->Text = System::String::Concat( gcnew String(this->iout1_max.ToString("F2")), gcnew String("A") );

			this->v2_target_label->Text = System::String::Concat( gcnew String(this->v2_target.ToString("F2")), gcnew String("V") );
			this->c2_max_label->Text = System::String::Concat( gcnew String(this->iout2_max.ToString("F2")), gcnew String("A") );

			this->v3_target_label->Text = System::String::Concat( gcnew String(this->v3_target.ToString("F2")), gcnew String("V") );
			this->c3_max_label->Text = System::String::Concat( gcnew String(this->iout3_max.ToString("F2")), gcnew String("A") );

			this->vin_cutoff_label->Text = System::String::Concat( gcnew String(this->vin_cutoff.ToString("F2")), gcnew String("V") );

			// Output 1 enabled
			if( this->output1_enabled )
			{
				this->output1_enabled_checkbox->Checked = true;

				change_out1_back_color(SUPPLY_OUTPUT_GOOD_COLOR);
			}
			else
			{
				this->output1_enabled_checkbox->Checked = false;

				change_out1_back_color(SUPPLY_DISABLED_COLOR);
			}

			// Output 2 enabled
			if( this->output2_enabled )
			{
				this->output2_enabled_checkbox->Checked = true;

				change_out2_back_color(SUPPLY_OUTPUT_GOOD_COLOR);
			}
			else
			{
				this->output2_enabled_checkbox->Checked = false;

				change_out2_back_color(SUPPLY_DISABLED_COLOR);
			}

			// Output 3 enabled
			if( this->output3_enabled )
			{
				this->output3_enabled_checkbox->Checked = true;

				change_out3_back_color(SUPPLY_OUTPUT_GOOD_COLOR);
			}
			else
			{
				this->output3_enabled_checkbox->Checked = false;

				change_out3_back_color(SUPPLY_DISABLED_COLOR);
			}

			// Low current 1 enabled
			if( this->low_current1_enabled )
			{
				this->output1_lowc_enabled_Checkbox->Checked = true;
			}
			else
			{
				this->output1_lowc_enabled_Checkbox->Checked = false;
			}

			// Low current 2 enabled
			if( this->low_current2_enabled )
			{
				this->output2_lowc_enabled_Checkbox->Checked = true;
			}
			else
			{
				this->output2_lowc_enabled_Checkbox->Checked = false;
			}

			// Low current 3 enabled
			if( this->low_current3_enabled )
			{
				this->output3_lowc_enabled_Checkbox->Checked = true;
			}
			else
			{
				this->output3_lowc_enabled_Checkbox->Checked = false;
			}

			// Vin cutoff
			if( this->vin_cutoff_active )
			{
				change_vin_back_color(HYDRA_INPUT_CUTOFF_COLOR);
			}
			else
			{
				change_vin_back_color(SUPPLY_OUTPUT_GOOD_COLOR);
			}

			// Trackbars
			this->output1_trackbar->Value = I32_saturate( (Int32)((this->v1_target)*10), this->output1_trackbar->Maximum, this->output1_trackbar->Minimum );
			this->output2_trackbar->Value = I32_saturate( (Int32)((this->v2_target)*10), this->output2_trackbar->Maximum, this->output2_trackbar->Minimum );
			this->output3_trackbar->Value = I32_saturate( (Int32)((this->v3_target)*10), this->output3_trackbar->Maximum, this->output3_trackbar->Minimum );
		
			this->max_current1_trackbar->Value = I32_saturate( (Int32)(this->iout1_max/0.05f), this->max_current1_trackbar->Maximum, this->max_current1_trackbar->Minimum );
			this->max_current2_trackbar->Value = I32_saturate( (Int32)(this->iout2_max/0.05f), this->max_current2_trackbar->Maximum, this->max_current2_trackbar->Minimum );
			this->max_current3_trackbar->Value = I32_saturate( (Int32)(this->iout3_max/0.05f), this->max_current3_trackbar->Maximum, this->max_current3_trackbar->Minimum );

			// In case we are in cutoff mode, update the enabled/disabled properties of supply controls appropriately
			update_cutoff_display();
		}

		/* -------------------------------------------------------------------------------------------------------
		* Name: I32_saturate( Int32 input, Int32 max, Int32 min )
		* Description:	
		*	Function limits the value in 'input' to the range specified by 'max' and 'min'
		------------------------------------------------------------------------------------------------------- */
private: Int32 I32_saturate( Int32 input, Int32 max, Int32 min )
		{
			if( input < min )
			{
				return min;
			}

			if( input > max )
			{
				return max;
			}

			return input;
		}

		/* -------------------------------------------------------------------------------------------------------
		* Name: changeOutput_safe( SerialPacket^ packet )
		* Description:	
		*	Function call for changing the output display given received serial data
		------------------------------------------------------------------------------------------------------- */
		void changeOutput_safe( SerialPacket^ packet )
		{
			if( this->InvokeRequired )
			{
				cli::array<System::Object^>^ args = gcnew cli::array<System::Object^>(1);
				args[0] = packet;
				
				this->BeginInvoke( gcnew UpdateOutputDelegate( this, &Form1::changeOutput_safe ),  args);
			}
			else
			{
				// Extract data from packet
				int i1 = packet->GetDataByte(1) | ((packet->GetDataByte(0) & 0x0F) << 8);
				int v1 = packet->GetDataByte(3) | (packet->GetDataByte(2) << 8);
				
				int i2 = packet->GetDataByte(5) | ((packet->GetDataByte(4) & 0x0F) << 8);
				int v2 = packet->GetDataByte(7) | (packet->GetDataByte(6) << 8);

				int i3 = packet->GetDataByte(9) | ((packet->GetDataByte(8) & 0x0F) << 8);
				int v3 = packet->GetDataByte(11) | (packet->GetDataByte(10) << 8);

				int vin = packet->GetDataByte(15) | (packet->GetDataByte(14) << 8);
				
				// Check for undervoltage condition
				if( packet->GetDataByte(12) & 0x80 )
				{
					// Only run this once after the voltage cutoff is turned on
					if( this->vin_cutoff_active == 0 )
					{

						// Set flag so that we don't run this code again.
						this->vin_cutoff_active = 1;

						// Set supply BG color to indicate that the cutoff voltage has been reached
						change_vin_back_color( HYDRA_INPUT_CUTOFF_COLOR );

						// Disable voltage controls
						if( !this->cutoff1_override )
						{
							disable_v1_controls();
						}
						if( !this->cutoff2_override )
						{
							disable_v2_controls();
						}
						if( !this->cutoff3_override )
						{
							disable_v3_controls();
						}

						// Clear config_data flag so that the app will request new config data from the supply
						// This is so that the check-boxes 
						this->have_config_data = 0;

						// Display message box to tell user that the input voltage is too low
						MessageBox::Show("The input voltage dropped below the input cutoff voltage threshold.  All outputs are disabled except those with the cutoff override enabled.\r\n\r\nTo turn disabled outputs back on, either decrease the voltage cutoff threshold or increase the input voltage.\r\n\r\nIn either case, the Hydra must still be restarted before the outputs will turn back on.  Alternatively, the cutoff override can be set to turn the outputs back on.", "Input Voltage Too Low", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
					}
				}
				else
				{
					// If cutoff is (was) active according to the internal member variable, reset the background color and enable
					// voltage controls as appropriate.
					// 
					if( this->vin_cutoff_active )
					{
						this->vin_cutoff_active = 0;

						// Clear config data flag to trigger a read.  Once the data is read, the voltage controls will be re-enabled.
						this->have_config_data = 0;

						change_vin_back_color( SUPPLY_OUTPUT_GOOD_COLOR );
					}
				}

				int out1_cv = (packet->GetDataByte(0) >> 7) & 0x01;
				int out2_cv = (packet->GetDataByte(4) >> 7) & 0x01;
				int out3_cv = (packet->GetDataByte(8) >> 7) & 0x01;

				int out1_cc = (packet->GetDataByte(0) >> 6) & 0x01;
				int out2_cc = (packet->GetDataByte(4) >> 6) & 0x01;
				int out3_cc = (packet->GetDataByte(8) >> 6) & 0x01;

				int out1_fault = ((packet->GetDataByte(0) >> 4) & 0x01);
				int out2_fault = ((packet->GetDataByte(4) >> 4) & 0x01);
				int out3_fault = ((packet->GetDataByte(8) >> 4) & 0x01);

				// Convert data to voltages and currents
				float f_i1 = (float)i1*IOUT_REG_SCALE;
				float f_i2 = (float)i2*IOUT_REG_SCALE;
				float f_i3 = (float)i3*IOUT_REG_SCALE;

				// Voltage...
				float f_v1 = (float)v1*VOUT_REG_SCALE;
				float f_v2 = (float)v2*VOUT_REG_SCALE;
				float f_v3 = (float)v3*VOUT_REG_SCALE;

				float f_vin = (float)vin*VOUT_REG_SCALE;

				this->output1_voltage = f_v1;
				this->output2_voltage = f_v2;
				this->output3_voltage = f_v3;

				this->output1_current = f_i1;
				this->output2_current = f_i2;
				this->output3_current = f_i3;

				this->output1_voltage_text->Text = System::String::Concat( output1_voltage.ToString("F2"), gcnew System::String(" V") );
				this->output1_current_text->Text = System::String::Concat(gcnew System::String(output1_current.ToString("F2")), gcnew System::String(" A") );

				this->output2_voltage_text->Text = System::String::Concat( output2_voltage.ToString("F2"), gcnew System::String(" V") );
				this->output2_current_text->Text = System::String::Concat(gcnew System::String(output2_current.ToString("F2")), gcnew System::String(" A") );
				
				this->output3_voltage_text->Text = System::String::Concat( output3_voltage.ToString("F2"), gcnew System::String(" V") );
				this->output3_current_text->Text = System::String::Concat(gcnew System::String(output3_current.ToString("F2")), gcnew System::String(" A") );

				this->input_voltage_text->Text = System::String::Concat( gcnew System::String( f_vin.ToString("F2")), gcnew System::String(" V"));

				// Set CC, CV, OFF flag
				if( !out1_fault )
				{
					if( out1_cv )
					{
						this->v1_status_label->Text = gcnew System::String("CV");
						this->c1_status_label->Text = gcnew System::String("CV");
					}
					else if( out1_cc )
					{
						this->v1_status_label->Text = gcnew System::String("CC");
						this->c1_status_label->Text = gcnew System::String("CC");
					}
					else
					{
						this->v1_status_label->Text = gcnew System::String("OFF");
						this->c1_status_label->Text = gcnew System::String("OFF");
					}
				}
				else
				{
					this->v1_status_label->Text = System::String::Concat(gcnew String("ERR"), gcnew String(out1_fault.ToString()));
					this->c1_status_label->Text = System::String::Concat(gcnew String("ERR"), gcnew String(out1_fault.ToString()));
				}

				// Set CC, CV, OFF flag
				if( !out2_fault )
				{
					if( out2_cv )
					{
						this->v2_status_label->Text = gcnew System::String("CV");
						this->c2_status_label->Text = gcnew System::String("CV");
					}
					else if( out2_cc )
					{
						this->v2_status_label->Text = gcnew System::String("CC");
						this->c2_status_label->Text = gcnew System::String("CC");
					}
					else
					{
						this->v2_status_label->Text = gcnew System::String("OFF");
						this->c2_status_label->Text = gcnew System::String("OFF");
					}
				}
				else
				{
					this->v2_status_label->Text = System::String::Concat(gcnew String("ERR"), gcnew String(out2_fault.ToString()));
					this->c2_status_label->Text = System::String::Concat(gcnew String("ERR"), gcnew String(out2_fault.ToString()));
				}

				// Set CC, CV, OFF flag
				if( !out3_fault )
				{
					if( out3_cv )
					{
						this->v3_status_label->Text = gcnew System::String("CV");
						this->c3_status_label->Text = gcnew System::String("CV");
					}
					else if( out3_cc )
					{
						this->v3_status_label->Text = gcnew System::String("CC");
						this->c3_status_label->Text = gcnew System::String("CC");
					}
					else
					{
						this->v3_status_label->Text = gcnew System::String("OFF");
						this->c3_status_label->Text = gcnew System::String("OFF");
					}
				}
				else
				{
					this->v3_status_label->Text = System::String::Concat(gcnew String("ERR"), gcnew String(out3_fault.ToString()));
					this->c3_status_label->Text = System::String::Concat(gcnew String("ERR"), gcnew String(out3_fault.ToString()));
				}
			}
		}


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(Form1::typeid));
			this->max_current3_trackbar = (gcnew System::Windows::Forms::TrackBar());
			this->max_current2_trackbar = (gcnew System::Windows::Forms::TrackBar());
			this->max_current1_trackbar = (gcnew System::Windows::Forms::TrackBar());
			this->disconnectButton = (gcnew System::Windows::Forms::Button());
			this->connectButton = (gcnew System::Windows::Forms::Button());
			this->serialComboBox = (gcnew System::Windows::Forms::ComboBox());
			this->portLabel = (gcnew System::Windows::Forms::Label());
			this->output1_trackbar = (gcnew System::Windows::Forms::TrackBar());
			this->output2_trackbar = (gcnew System::Windows::Forms::TrackBar());
			this->output3_trackbar = (gcnew System::Windows::Forms::TrackBar());
			this->output3_current_text = (gcnew System::Windows::Forms::TextBox());
			this->output3_voltage_text = (gcnew System::Windows::Forms::TextBox());
			this->output2_current_text = (gcnew System::Windows::Forms::TextBox());
			this->output2_voltage_text = (gcnew System::Windows::Forms::TextBox());
			this->output1_current_text = (gcnew System::Windows::Forms::TextBox());
			this->output1_voltage_text = (gcnew System::Windows::Forms::TextBox());
			this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
			this->output1_lowc_enabled_Checkbox = (gcnew System::Windows::Forms::CheckBox());
			this->v1_status_label = (gcnew System::Windows::Forms::Label());
			this->v1_target_label = (gcnew System::Windows::Forms::Label());
			this->c1_max_label = (gcnew System::Windows::Forms::Label());
			this->c1_status_label = (gcnew System::Windows::Forms::Label());
			this->output1_enabled_checkbox = (gcnew System::Windows::Forms::CheckBox());
			this->output2_enabled_checkbox = (gcnew System::Windows::Forms::CheckBox());
			this->output3_enabled_checkbox = (gcnew System::Windows::Forms::CheckBox());
			this->statusLabelP = (gcnew System::Windows::Forms::Label());
			this->statusLabel = (gcnew System::Windows::Forms::Label());
			this->COM_timeout_timer = (gcnew System::Windows::Forms::Timer(this->components));
			this->output2_lowc_enabled_Checkbox = (gcnew System::Windows::Forms::CheckBox());
			this->output3_lowc_enabled_Checkbox = (gcnew System::Windows::Forms::CheckBox());
			this->v2_status_label = (gcnew System::Windows::Forms::Label());
			this->v2_target_label = (gcnew System::Windows::Forms::Label());
			this->c2_max_label = (gcnew System::Windows::Forms::Label());
			this->c2_status_label = (gcnew System::Windows::Forms::Label());
			this->c3_max_label = (gcnew System::Windows::Forms::Label());
			this->c3_status_label = (gcnew System::Windows::Forms::Label());
			this->v3_status_label = (gcnew System::Windows::Forms::Label());
			this->v3_target_label = (gcnew System::Windows::Forms::Label());
			this->groupBox3 = (gcnew System::Windows::Forms::GroupBox());
			this->groupBox4 = (gcnew System::Windows::Forms::GroupBox());
			this->input_voltage_text = (gcnew System::Windows::Forms::TextBox());
			this->cutoff_text_label = (gcnew System::Windows::Forms::Label());
			this->vin_cutoff_label = (gcnew System::Windows::Forms::Label());
			this->groupBox5 = (gcnew System::Windows::Forms::GroupBox());
			this->label19 = (gcnew System::Windows::Forms::Label());
			this->baudComboBox = (gcnew System::Windows::Forms::ComboBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->calGroupBox = (gcnew System::Windows::Forms::GroupBox());
			this->cal_help_label = (gcnew System::Windows::Forms::LinkLabel());
			this->label16 = (gcnew System::Windows::Forms::Label());
			this->label17 = (gcnew System::Windows::Forms::Label());
			this->label18 = (gcnew System::Windows::Forms::Label());
			this->label13 = (gcnew System::Windows::Forms::Label());
			this->label14 = (gcnew System::Windows::Forms::Label());
			this->label15 = (gcnew System::Windows::Forms::Label());
			this->label12 = (gcnew System::Windows::Forms::Label());
			this->label11 = (gcnew System::Windows::Forms::Label());
			this->label10 = (gcnew System::Windows::Forms::Label());
			this->label9 = (gcnew System::Windows::Forms::Label());
			this->label8 = (gcnew System::Windows::Forms::Label());
			this->label7 = (gcnew System::Windows::Forms::Label());
			this->label6 = (gcnew System::Windows::Forms::Label());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->output3_error_text = (gcnew System::Windows::Forms::TextBox());
			this->output2_error_text = (gcnew System::Windows::Forms::TextBox());
			this->output1_error_text = (gcnew System::Windows::Forms::TextBox());
			this->out2_c3_text = (gcnew System::Windows::Forms::TextBox());
			this->out2_c2_text = (gcnew System::Windows::Forms::TextBox());
			this->out2_c1_text = (gcnew System::Windows::Forms::TextBox());
			this->out1_c3_text = (gcnew System::Windows::Forms::TextBox());
			this->out1_c2_text = (gcnew System::Windows::Forms::TextBox());
			this->out1_c1_text = (gcnew System::Windows::Forms::TextBox());
			this->out3_c3_text = (gcnew System::Windows::Forms::TextBox());
			this->out3_c2_text = (gcnew System::Windows::Forms::TextBox());
			this->out3_c1_text = (gcnew System::Windows::Forms::TextBox());
			this->cal_status_label = (gcnew System::Windows::Forms::Label());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->accept_button = (gcnew System::Windows::Forms::Button());
			this->cal_cancel_button = (gcnew System::Windows::Forms::Button());
			this->cal_start_button = (gcnew System::Windows::Forms::Button());
			this->CAL_timer = (gcnew System::Windows::Forms::Timer(this->components));
			this->packetResponseTimer = (gcnew System::Windows::Forms::Timer(this->components));
			this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
			this->toolStripMenuItem1 = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exitMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->optionsToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->showCalibrationToolsMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->hideCalibrationToolsMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->helpToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->aboutMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->serialPortSettingsToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->max_current3_trackbar))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->max_current2_trackbar))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->max_current1_trackbar))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->output1_trackbar))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->output2_trackbar))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->output3_trackbar))->BeginInit();
			this->groupBox2->SuspendLayout();
			this->groupBox3->SuspendLayout();
			this->groupBox4->SuspendLayout();
			this->groupBox5->SuspendLayout();
			this->calGroupBox->SuspendLayout();
			this->menuStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// max_current3_trackbar
			// 
			this->max_current3_trackbar->BackColor = System::Drawing::SystemColors::Window;
			this->max_current3_trackbar->Enabled = false;
			this->max_current3_trackbar->LargeChange = 1;
			this->max_current3_trackbar->Location = System::Drawing::Point(159, 93);
			this->max_current3_trackbar->Maximum = 60;
			this->max_current3_trackbar->Name = L"max_current3_trackbar";
			this->max_current3_trackbar->Size = System::Drawing::Size(135, 45);
			this->max_current3_trackbar->TabIndex = 42;
			this->max_current3_trackbar->TabStop = false;
			this->max_current3_trackbar->TickStyle = System::Windows::Forms::TickStyle::None;
			this->max_current3_trackbar->Value = 50;
			this->max_current3_trackbar->Scroll += gcnew System::EventHandler(this, &Form1::max_current3_trackbar_Scroll);
			this->max_current3_trackbar->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::max_current3_trackbar_MouseDown);
			this->max_current3_trackbar->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::max_current3_trackbar_MouseUp);
			// 
			// max_current2_trackbar
			// 
			this->max_current2_trackbar->BackColor = System::Drawing::SystemColors::Window;
			this->max_current2_trackbar->Enabled = false;
			this->max_current2_trackbar->LargeChange = 1;
			this->max_current2_trackbar->Location = System::Drawing::Point(161, 93);
			this->max_current2_trackbar->Maximum = 60;
			this->max_current2_trackbar->Name = L"max_current2_trackbar";
			this->max_current2_trackbar->Size = System::Drawing::Size(135, 45);
			this->max_current2_trackbar->TabIndex = 41;
			this->max_current2_trackbar->TabStop = false;
			this->max_current2_trackbar->TickStyle = System::Windows::Forms::TickStyle::None;
			this->max_current2_trackbar->Value = 50;
			this->max_current2_trackbar->Scroll += gcnew System::EventHandler(this, &Form1::max_current2_trackbar_Scroll);
			this->max_current2_trackbar->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::max_current2_trackbar_MouseDown);
			this->max_current2_trackbar->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::max_current2_trackbar_MouseUp);
			// 
			// max_current1_trackbar
			// 
			this->max_current1_trackbar->BackColor = System::Drawing::SystemColors::Window;
			this->max_current1_trackbar->Enabled = false;
			this->max_current1_trackbar->LargeChange = 1;
			this->max_current1_trackbar->Location = System::Drawing::Point(159, 93);
			this->max_current1_trackbar->Maximum = 60;
			this->max_current1_trackbar->Name = L"max_current1_trackbar";
			this->max_current1_trackbar->Size = System::Drawing::Size(136, 45);
			this->max_current1_trackbar->TabIndex = 40;
			this->max_current1_trackbar->TabStop = false;
			this->max_current1_trackbar->TickStyle = System::Windows::Forms::TickStyle::None;
			this->max_current1_trackbar->Value = 50;
			this->max_current1_trackbar->Scroll += gcnew System::EventHandler(this, &Form1::max_current1_trackbar_Scroll);
			this->max_current1_trackbar->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::max_current1_trackbar_MouseDown);
			this->max_current1_trackbar->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::max_current1_trackbar_MouseUp);
			// 
			// disconnectButton
			// 
			this->disconnectButton->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"disconnectButton.BackgroundImage")));
			this->disconnectButton->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->disconnectButton->Enabled = false;
			this->disconnectButton->Location = System::Drawing::Point(531, 18);
			this->disconnectButton->Name = L"disconnectButton";
			this->disconnectButton->Size = System::Drawing::Size(26, 25);
			this->disconnectButton->TabIndex = 39;
			this->disconnectButton->UseVisualStyleBackColor = true;
			this->disconnectButton->Click += gcnew System::EventHandler(this, &Form1::disconnectButton_Click);
			// 
			// connectButton
			// 
			this->connectButton->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"connectButton.BackgroundImage")));
			this->connectButton->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Zoom;
			this->connectButton->Location = System::Drawing::Point(499, 18);
			this->connectButton->Name = L"connectButton";
			this->connectButton->Size = System::Drawing::Size(26, 25);
			this->connectButton->TabIndex = 38;
			this->connectButton->UseVisualStyleBackColor = true;
			this->connectButton->Click += gcnew System::EventHandler(this, &Form1::connectButton_Click);
			// 
			// serialComboBox
			// 
			this->serialComboBox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->serialComboBox->FormattingEnabled = true;
			this->serialComboBox->Location = System::Drawing::Point(248, 19);
			this->serialComboBox->Name = L"serialComboBox";
			this->serialComboBox->Size = System::Drawing::Size(89, 24);
			this->serialComboBox->TabIndex = 37;
			// 
			// portLabel
			// 
			this->portLabel->AutoSize = true;
			this->portLabel->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->portLabel->Location = System::Drawing::Point(211, 22);
			this->portLabel->Name = L"portLabel";
			this->portLabel->Size = System::Drawing::Size(38, 17);
			this->portLabel->TabIndex = 36;
			this->portLabel->Text = L"Port:";
			// 
			// output1_trackbar
			// 
			this->output1_trackbar->BackColor = System::Drawing::SystemColors::Window;
			this->output1_trackbar->Enabled = false;
			this->output1_trackbar->LargeChange = 1;
			this->output1_trackbar->Location = System::Drawing::Point(11, 93);
			this->output1_trackbar->Maximum = 140;
			this->output1_trackbar->Minimum = 25;
			this->output1_trackbar->Name = L"output1_trackbar";
			this->output1_trackbar->Size = System::Drawing::Size(145, 45);
			this->output1_trackbar->TabIndex = 35;
			this->output1_trackbar->TabStop = false;
			this->output1_trackbar->TickStyle = System::Windows::Forms::TickStyle::None;
			this->output1_trackbar->Value = 25;
			this->output1_trackbar->Scroll += gcnew System::EventHandler(this, &Form1::output1_trackbar_Scroll);
			this->output1_trackbar->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::output1_trackbar_MouseDown);
			this->output1_trackbar->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::output1_trackbar_MouseUp);
			// 
			// output2_trackbar
			// 
			this->output2_trackbar->BackColor = System::Drawing::SystemColors::Window;
			this->output2_trackbar->Enabled = false;
			this->output2_trackbar->LargeChange = 1;
			this->output2_trackbar->Location = System::Drawing::Point(11, 93);
			this->output2_trackbar->Maximum = 140;
			this->output2_trackbar->Minimum = 25;
			this->output2_trackbar->Name = L"output2_trackbar";
			this->output2_trackbar->Size = System::Drawing::Size(144, 45);
			this->output2_trackbar->TabIndex = 34;
			this->output2_trackbar->TabStop = false;
			this->output2_trackbar->TickStyle = System::Windows::Forms::TickStyle::None;
			this->output2_trackbar->Value = 25;
			this->output2_trackbar->Scroll += gcnew System::EventHandler(this, &Form1::output2_trackbar_Scroll);
			this->output2_trackbar->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::output2_trackbar_MouseDown);
			this->output2_trackbar->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::output2_trackbar_MouseUp);
			// 
			// output3_trackbar
			// 
			this->output3_trackbar->BackColor = System::Drawing::SystemColors::Window;
			this->output3_trackbar->Enabled = false;
			this->output3_trackbar->LargeChange = 1;
			this->output3_trackbar->Location = System::Drawing::Point(11, 93);
			this->output3_trackbar->Maximum = 140;
			this->output3_trackbar->Minimum = 25;
			this->output3_trackbar->Name = L"output3_trackbar";
			this->output3_trackbar->Size = System::Drawing::Size(142, 45);
			this->output3_trackbar->TabIndex = 33;
			this->output3_trackbar->TabStop = false;
			this->output3_trackbar->TickStyle = System::Windows::Forms::TickStyle::None;
			this->output3_trackbar->Value = 25;
			this->output3_trackbar->Scroll += gcnew System::EventHandler(this, &Form1::output3_trackbar_Scroll);
			this->output3_trackbar->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::output3_trackbar_MouseDown);
			this->output3_trackbar->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::output3_trackbar_MouseUp);
			// 
			// output3_current_text
			// 
			this->output3_current_text->BackColor = System::Drawing::Color::LightGray;
			this->output3_current_text->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->output3_current_text->Cursor = System::Windows::Forms::Cursors::Hand;
			this->output3_current_text->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 18, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->output3_current_text->ForeColor = System::Drawing::Color::Black;
			this->output3_current_text->Location = System::Drawing::Point(167, 52);
			this->output3_current_text->Name = L"output3_current_text";
			this->output3_current_text->ReadOnly = true;
			this->output3_current_text->Size = System::Drawing::Size(121, 35);
			this->output3_current_text->TabIndex = 20;
			this->output3_current_text->TabStop = false;
			this->output3_current_text->Text = L"0.00 A";
			this->output3_current_text->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->output3_current_text->Click += gcnew System::EventHandler(this, &Form1::v3_target_label_Click);
			// 
			// output3_voltage_text
			// 
			this->output3_voltage_text->BackColor = System::Drawing::Color::LightGray;
			this->output3_voltage_text->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->output3_voltage_text->Cursor = System::Windows::Forms::Cursors::Hand;
			this->output3_voltage_text->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 18, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->output3_voltage_text->ForeColor = System::Drawing::Color::Black;
			this->output3_voltage_text->Location = System::Drawing::Point(15, 52);
			this->output3_voltage_text->Name = L"output3_voltage_text";
			this->output3_voltage_text->ReadOnly = true;
			this->output3_voltage_text->Size = System::Drawing::Size(130, 35);
			this->output3_voltage_text->TabIndex = 19;
			this->output3_voltage_text->TabStop = false;
			this->output3_voltage_text->Text = L"0.00 V";
			this->output3_voltage_text->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->output3_voltage_text->Click += gcnew System::EventHandler(this, &Form1::v3_target_label_Click);
			// 
			// output2_current_text
			// 
			this->output2_current_text->BackColor = System::Drawing::Color::LightGray;
			this->output2_current_text->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->output2_current_text->Cursor = System::Windows::Forms::Cursors::Hand;
			this->output2_current_text->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 18, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->output2_current_text->ForeColor = System::Drawing::Color::Black;
			this->output2_current_text->Location = System::Drawing::Point(169, 52);
			this->output2_current_text->Name = L"output2_current_text";
			this->output2_current_text->ReadOnly = true;
			this->output2_current_text->Size = System::Drawing::Size(121, 35);
			this->output2_current_text->TabIndex = 18;
			this->output2_current_text->TabStop = false;
			this->output2_current_text->Text = L"0.00 A";
			this->output2_current_text->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->output2_current_text->Click += gcnew System::EventHandler(this, &Form1::v2_target_label_Click);
			// 
			// output2_voltage_text
			// 
			this->output2_voltage_text->BackColor = System::Drawing::Color::LightGray;
			this->output2_voltage_text->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->output2_voltage_text->Cursor = System::Windows::Forms::Cursors::Hand;
			this->output2_voltage_text->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 18, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->output2_voltage_text->ForeColor = System::Drawing::Color::Black;
			this->output2_voltage_text->Location = System::Drawing::Point(16, 52);
			this->output2_voltage_text->Name = L"output2_voltage_text";
			this->output2_voltage_text->ReadOnly = true;
			this->output2_voltage_text->Size = System::Drawing::Size(130, 35);
			this->output2_voltage_text->TabIndex = 17;
			this->output2_voltage_text->TabStop = false;
			this->output2_voltage_text->Text = L"0.00 V";
			this->output2_voltage_text->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->output2_voltage_text->Click += gcnew System::EventHandler(this, &Form1::v2_target_label_Click);
			// 
			// output1_current_text
			// 
			this->output1_current_text->BackColor = System::Drawing::Color::LightGray;
			this->output1_current_text->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->output1_current_text->Cursor = System::Windows::Forms::Cursors::Hand;
			this->output1_current_text->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 18, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->output1_current_text->ForeColor = System::Drawing::Color::Black;
			this->output1_current_text->Location = System::Drawing::Point(167, 52);
			this->output1_current_text->Name = L"output1_current_text";
			this->output1_current_text->ReadOnly = true;
			this->output1_current_text->Size = System::Drawing::Size(121, 35);
			this->output1_current_text->TabIndex = 16;
			this->output1_current_text->TabStop = false;
			this->output1_current_text->Text = L"0.00 A";
			this->output1_current_text->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->output1_current_text->Click += gcnew System::EventHandler(this, &Form1::v1_target_label_Click);
			// 
			// output1_voltage_text
			// 
			this->output1_voltage_text->BackColor = System::Drawing::Color::LightGray;
			this->output1_voltage_text->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->output1_voltage_text->Cursor = System::Windows::Forms::Cursors::Hand;
			this->output1_voltage_text->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 18, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->output1_voltage_text->ForeColor = System::Drawing::Color::Black;
			this->output1_voltage_text->Location = System::Drawing::Point(15, 52);
			this->output1_voltage_text->Name = L"output1_voltage_text";
			this->output1_voltage_text->ReadOnly = true;
			this->output1_voltage_text->Size = System::Drawing::Size(133, 35);
			this->output1_voltage_text->TabIndex = 15;
			this->output1_voltage_text->TabStop = false;
			this->output1_voltage_text->Text = L"12.00 V";
			this->output1_voltage_text->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->output1_voltage_text->Click += gcnew System::EventHandler(this, &Form1::v1_target_label_Click);
			// 
			// groupBox2
			// 
			this->groupBox2->BackColor = System::Drawing::Color::Transparent;
			this->groupBox2->Controls->Add(this->output1_lowc_enabled_Checkbox);
			this->groupBox2->Controls->Add(this->v1_status_label);
			this->groupBox2->Controls->Add(this->output1_trackbar);
			this->groupBox2->Controls->Add(this->max_current1_trackbar);
			this->groupBox2->Controls->Add(this->v1_target_label);
			this->groupBox2->Controls->Add(this->c1_max_label);
			this->groupBox2->Controls->Add(this->c1_status_label);
			this->groupBox2->Controls->Add(this->output1_voltage_text);
			this->groupBox2->Controls->Add(this->output1_current_text);
			this->groupBox2->Controls->Add(this->output1_enabled_checkbox);
			this->groupBox2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->groupBox2->Location = System::Drawing::Point(12, 87);
			this->groupBox2->Name = L"groupBox2";
			this->groupBox2->Size = System::Drawing::Size(307, 139);
			this->groupBox2->TabIndex = 49;
			this->groupBox2->TabStop = false;
			this->groupBox2->Text = L"Output 1";
			// 
			// output1_lowc_enabled_Checkbox
			// 
			this->output1_lowc_enabled_Checkbox->AutoSize = true;
			this->output1_lowc_enabled_Checkbox->Enabled = false;
			this->output1_lowc_enabled_Checkbox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->output1_lowc_enabled_Checkbox->Location = System::Drawing::Point(167, 29);
			this->output1_lowc_enabled_Checkbox->Name = L"output1_lowc_enabled_Checkbox";
			this->output1_lowc_enabled_Checkbox->Size = System::Drawing::Size(127, 19);
			this->output1_lowc_enabled_Checkbox->TabIndex = 53;
			this->output1_lowc_enabled_Checkbox->Text = L"Low Current Mode";
			this->output1_lowc_enabled_Checkbox->UseVisualStyleBackColor = true;
			this->output1_lowc_enabled_Checkbox->CheckedChanged += gcnew System::EventHandler(this, &Form1::output1_lowc_enabled_CheckedChanged);
			// 
			// v1_status_label
			// 
			this->v1_status_label->AutoSize = true;
			this->v1_status_label->BackColor = System::Drawing::Color::LightGray;
			this->v1_status_label->Cursor = System::Windows::Forms::Cursors::Hand;
			this->v1_status_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->v1_status_label->Location = System::Drawing::Point(17, 55);
			this->v1_status_label->Name = L"v1_status_label";
			this->v1_status_label->Size = System::Drawing::Size(23, 13);
			this->v1_status_label->TabIndex = 52;
			this->v1_status_label->Text = L"CV";
			this->v1_status_label->Click += gcnew System::EventHandler(this, &Form1::v1_target_label_Click);
			// 
			// v1_target_label
			// 
			this->v1_target_label->AutoSize = true;
			this->v1_target_label->BackColor = System::Drawing::Color::LightGray;
			this->v1_target_label->Cursor = System::Windows::Forms::Cursors::Hand;
			this->v1_target_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->v1_target_label->Location = System::Drawing::Point(16, 71);
			this->v1_target_label->Name = L"v1_target_label";
			this->v1_target_label->Size = System::Drawing::Size(41, 13);
			this->v1_target_label->TabIndex = 51;
			this->v1_target_label->Text = L"12.00V";
			this->v1_target_label->Click += gcnew System::EventHandler(this, &Form1::v1_target_label_Click);
			// 
			// c1_max_label
			// 
			this->c1_max_label->AutoSize = true;
			this->c1_max_label->BackColor = System::Drawing::Color::LightGray;
			this->c1_max_label->Cursor = System::Windows::Forms::Cursors::Hand;
			this->c1_max_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->c1_max_label->Location = System::Drawing::Point(170, 71);
			this->c1_max_label->Name = L"c1_max_label";
			this->c1_max_label->Size = System::Drawing::Size(35, 13);
			this->c1_max_label->TabIndex = 50;
			this->c1_max_label->Text = L"2.50A";
			this->c1_max_label->Click += gcnew System::EventHandler(this, &Form1::v1_target_label_Click);
			// 
			// c1_status_label
			// 
			this->c1_status_label->AutoSize = true;
			this->c1_status_label->BackColor = System::Drawing::Color::LightGray;
			this->c1_status_label->Cursor = System::Windows::Forms::Cursors::Hand;
			this->c1_status_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->c1_status_label->Location = System::Drawing::Point(170, 55);
			this->c1_status_label->Name = L"c1_status_label";
			this->c1_status_label->Size = System::Drawing::Size(23, 13);
			this->c1_status_label->TabIndex = 49;
			this->c1_status_label->Text = L"CV";
			this->c1_status_label->Click += gcnew System::EventHandler(this, &Form1::v1_target_label_Click);
			// 
			// output1_enabled_checkbox
			// 
			this->output1_enabled_checkbox->AutoSize = true;
			this->output1_enabled_checkbox->Enabled = false;
			this->output1_enabled_checkbox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->output1_enabled_checkbox->Location = System::Drawing::Point(15, 29);
			this->output1_enabled_checkbox->Name = L"output1_enabled_checkbox";
			this->output1_enabled_checkbox->Size = System::Drawing::Size(114, 19);
			this->output1_enabled_checkbox->TabIndex = 30;
			this->output1_enabled_checkbox->Text = L"Enable Output 1";
			this->output1_enabled_checkbox->UseVisualStyleBackColor = true;
			this->output1_enabled_checkbox->CheckedChanged += gcnew System::EventHandler(this, &Form1::output1_enabled_CheckedChanged);
			// 
			// output2_enabled_checkbox
			// 
			this->output2_enabled_checkbox->AutoSize = true;
			this->output2_enabled_checkbox->Enabled = false;
			this->output2_enabled_checkbox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->output2_enabled_checkbox->Location = System::Drawing::Point(16, 28);
			this->output2_enabled_checkbox->Name = L"output2_enabled_checkbox";
			this->output2_enabled_checkbox->Size = System::Drawing::Size(114, 19);
			this->output2_enabled_checkbox->TabIndex = 31;
			this->output2_enabled_checkbox->Text = L"Enable Output 2";
			this->output2_enabled_checkbox->UseVisualStyleBackColor = true;
			this->output2_enabled_checkbox->CheckedChanged += gcnew System::EventHandler(this, &Form1::output2_enabled_CheckedChanged);
			// 
			// output3_enabled_checkbox
			// 
			this->output3_enabled_checkbox->AutoSize = true;
			this->output3_enabled_checkbox->Enabled = false;
			this->output3_enabled_checkbox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->output3_enabled_checkbox->Location = System::Drawing::Point(15, 29);
			this->output3_enabled_checkbox->Name = L"output3_enabled_checkbox";
			this->output3_enabled_checkbox->Size = System::Drawing::Size(114, 19);
			this->output3_enabled_checkbox->TabIndex = 32;
			this->output3_enabled_checkbox->Text = L"Enable Output 3";
			this->output3_enabled_checkbox->UseVisualStyleBackColor = true;
			this->output3_enabled_checkbox->CheckedChanged += gcnew System::EventHandler(this, &Form1::output3_enabled_checkbox_CheckedChanged);
			// 
			// statusLabelP
			// 
			this->statusLabelP->AutoSize = true;
			this->statusLabelP->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->statusLabelP->Location = System::Drawing::Point(568, 24);
			this->statusLabelP->Name = L"statusLabelP";
			this->statusLabelP->Size = System::Drawing::Size(55, 16);
			this->statusLabelP->TabIndex = 16;
			this->statusLabelP->Text = L"Status:";
			// 
			// statusLabel
			// 
			this->statusLabel->AutoSize = true;
			this->statusLabel->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->statusLabel->ForeColor = System::Drawing::Color::Red;
			this->statusLabel->Location = System::Drawing::Point(620, 24);
			this->statusLabel->Name = L"statusLabel";
			this->statusLabel->Size = System::Drawing::Size(97, 16);
			this->statusLabel->TabIndex = 17;
			this->statusLabel->Text = L"Not Connected";
			// 
			// COM_timeout_timer
			// 
			this->COM_timeout_timer->Interval = 1000;
			this->COM_timeout_timer->Tick += gcnew System::EventHandler(this, &Form1::COM_timeout_timer_Tick);
			// 
			// output2_lowc_enabled_Checkbox
			// 
			this->output2_lowc_enabled_Checkbox->AutoSize = true;
			this->output2_lowc_enabled_Checkbox->Enabled = false;
			this->output2_lowc_enabled_Checkbox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->output2_lowc_enabled_Checkbox->Location = System::Drawing::Point(169, 27);
			this->output2_lowc_enabled_Checkbox->Name = L"output2_lowc_enabled_Checkbox";
			this->output2_lowc_enabled_Checkbox->Size = System::Drawing::Size(127, 19);
			this->output2_lowc_enabled_Checkbox->TabIndex = 54;
			this->output2_lowc_enabled_Checkbox->Text = L"Low Current Mode";
			this->output2_lowc_enabled_Checkbox->UseVisualStyleBackColor = true;
			this->output2_lowc_enabled_Checkbox->CheckedChanged += gcnew System::EventHandler(this, &Form1::output2_lowc_enabled_CheckedChanged);
			// 
			// output3_lowc_enabled_Checkbox
			// 
			this->output3_lowc_enabled_Checkbox->AutoSize = true;
			this->output3_lowc_enabled_Checkbox->Enabled = false;
			this->output3_lowc_enabled_Checkbox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->output3_lowc_enabled_Checkbox->Location = System::Drawing::Point(167, 29);
			this->output3_lowc_enabled_Checkbox->Name = L"output3_lowc_enabled_Checkbox";
			this->output3_lowc_enabled_Checkbox->Size = System::Drawing::Size(127, 19);
			this->output3_lowc_enabled_Checkbox->TabIndex = 55;
			this->output3_lowc_enabled_Checkbox->Text = L"Low Current Mode";
			this->output3_lowc_enabled_Checkbox->UseVisualStyleBackColor = true;
			this->output3_lowc_enabled_Checkbox->CheckedChanged += gcnew System::EventHandler(this, &Form1::output3_lowc_enabled_CheckedChanged);
			// 
			// v2_status_label
			// 
			this->v2_status_label->AutoSize = true;
			this->v2_status_label->BackColor = System::Drawing::Color::LightGray;
			this->v2_status_label->Cursor = System::Windows::Forms::Cursors::Hand;
			this->v2_status_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->v2_status_label->Location = System::Drawing::Point(19, 55);
			this->v2_status_label->Name = L"v2_status_label";
			this->v2_status_label->Size = System::Drawing::Size(23, 13);
			this->v2_status_label->TabIndex = 54;
			this->v2_status_label->Text = L"CV";
			this->v2_status_label->Click += gcnew System::EventHandler(this, &Form1::v2_target_label_Click);
			// 
			// v2_target_label
			// 
			this->v2_target_label->AutoSize = true;
			this->v2_target_label->BackColor = System::Drawing::Color::LightGray;
			this->v2_target_label->Cursor = System::Windows::Forms::Cursors::Hand;
			this->v2_target_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->v2_target_label->Location = System::Drawing::Point(18, 71);
			this->v2_target_label->Name = L"v2_target_label";
			this->v2_target_label->Size = System::Drawing::Size(41, 13);
			this->v2_target_label->TabIndex = 53;
			this->v2_target_label->Text = L"12.00V";
			this->v2_target_label->Click += gcnew System::EventHandler(this, &Form1::v2_target_label_Click);
			// 
			// c2_max_label
			// 
			this->c2_max_label->AutoSize = true;
			this->c2_max_label->BackColor = System::Drawing::Color::LightGray;
			this->c2_max_label->Cursor = System::Windows::Forms::Cursors::Hand;
			this->c2_max_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->c2_max_label->Location = System::Drawing::Point(171, 71);
			this->c2_max_label->Name = L"c2_max_label";
			this->c2_max_label->Size = System::Drawing::Size(35, 13);
			this->c2_max_label->TabIndex = 57;
			this->c2_max_label->Text = L"2.50A";
			this->c2_max_label->Click += gcnew System::EventHandler(this, &Form1::v2_target_label_Click);
			// 
			// c2_status_label
			// 
			this->c2_status_label->AutoSize = true;
			this->c2_status_label->BackColor = System::Drawing::Color::LightGray;
			this->c2_status_label->Cursor = System::Windows::Forms::Cursors::Hand;
			this->c2_status_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->c2_status_label->Location = System::Drawing::Point(171, 55);
			this->c2_status_label->Name = L"c2_status_label";
			this->c2_status_label->Size = System::Drawing::Size(23, 13);
			this->c2_status_label->TabIndex = 56;
			this->c2_status_label->Text = L"CV";
			this->c2_status_label->Click += gcnew System::EventHandler(this, &Form1::v2_target_label_Click);
			// 
			// c3_max_label
			// 
			this->c3_max_label->AutoSize = true;
			this->c3_max_label->BackColor = System::Drawing::Color::LightGray;
			this->c3_max_label->Cursor = System::Windows::Forms::Cursors::Hand;
			this->c3_max_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->c3_max_label->Location = System::Drawing::Point(170, 71);
			this->c3_max_label->Name = L"c3_max_label";
			this->c3_max_label->Size = System::Drawing::Size(35, 13);
			this->c3_max_label->TabIndex = 52;
			this->c3_max_label->Text = L"2.50A";
			this->c3_max_label->Click += gcnew System::EventHandler(this, &Form1::v3_target_label_Click);
			// 
			// c3_status_label
			// 
			this->c3_status_label->AutoSize = true;
			this->c3_status_label->BackColor = System::Drawing::Color::LightGray;
			this->c3_status_label->Cursor = System::Windows::Forms::Cursors::Hand;
			this->c3_status_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->c3_status_label->Location = System::Drawing::Point(170, 55);
			this->c3_status_label->Name = L"c3_status_label";
			this->c3_status_label->Size = System::Drawing::Size(23, 13);
			this->c3_status_label->TabIndex = 51;
			this->c3_status_label->Text = L"CV";
			this->c3_status_label->Click += gcnew System::EventHandler(this, &Form1::v3_target_label_Click);
			// 
			// v3_status_label
			// 
			this->v3_status_label->AutoSize = true;
			this->v3_status_label->BackColor = System::Drawing::Color::LightGray;
			this->v3_status_label->Cursor = System::Windows::Forms::Cursors::Hand;
			this->v3_status_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->v3_status_label->Location = System::Drawing::Point(18, 55);
			this->v3_status_label->Name = L"v3_status_label";
			this->v3_status_label->Size = System::Drawing::Size(23, 13);
			this->v3_status_label->TabIndex = 56;
			this->v3_status_label->Text = L"CV";
			this->v3_status_label->Click += gcnew System::EventHandler(this, &Form1::v3_target_label_Click);
			// 
			// v3_target_label
			// 
			this->v3_target_label->AutoSize = true;
			this->v3_target_label->BackColor = System::Drawing::Color::LightGray;
			this->v3_target_label->Cursor = System::Windows::Forms::Cursors::Hand;
			this->v3_target_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->v3_target_label->Location = System::Drawing::Point(17, 71);
			this->v3_target_label->Name = L"v3_target_label";
			this->v3_target_label->Size = System::Drawing::Size(41, 13);
			this->v3_target_label->TabIndex = 55;
			this->v3_target_label->Text = L"12.00V";
			this->v3_target_label->Click += gcnew System::EventHandler(this, &Form1::v3_target_label_Click);
			// 
			// groupBox3
			// 
			this->groupBox3->Controls->Add(this->c2_status_label);
			this->groupBox3->Controls->Add(this->v2_status_label);
			this->groupBox3->Controls->Add(this->v2_target_label);
			this->groupBox3->Controls->Add(this->output2_voltage_text);
			this->groupBox3->Controls->Add(this->c2_max_label);
			this->groupBox3->Controls->Add(this->output2_current_text);
			this->groupBox3->Controls->Add(this->output2_trackbar);
			this->groupBox3->Controls->Add(this->output2_enabled_checkbox);
			this->groupBox3->Controls->Add(this->max_current2_trackbar);
			this->groupBox3->Controls->Add(this->output2_lowc_enabled_Checkbox);
			this->groupBox3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->groupBox3->Location = System::Drawing::Point(335, 87);
			this->groupBox3->Name = L"groupBox3";
			this->groupBox3->Size = System::Drawing::Size(307, 139);
			this->groupBox3->TabIndex = 58;
			this->groupBox3->TabStop = false;
			this->groupBox3->Text = L"Output 2";
			// 
			// groupBox4
			// 
			this->groupBox4->Controls->Add(this->c3_max_label);
			this->groupBox4->Controls->Add(this->c3_status_label);
			this->groupBox4->Controls->Add(this->v3_status_label);
			this->groupBox4->Controls->Add(this->v3_target_label);
			this->groupBox4->Controls->Add(this->output3_voltage_text);
			this->groupBox4->Controls->Add(this->output3_trackbar);
			this->groupBox4->Controls->Add(this->max_current3_trackbar);
			this->groupBox4->Controls->Add(this->output3_current_text);
			this->groupBox4->Controls->Add(this->output3_enabled_checkbox);
			this->groupBox4->Controls->Add(this->output3_lowc_enabled_Checkbox);
			this->groupBox4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->groupBox4->Location = System::Drawing::Point(657, 87);
			this->groupBox4->Name = L"groupBox4";
			this->groupBox4->Size = System::Drawing::Size(307, 139);
			this->groupBox4->TabIndex = 59;
			this->groupBox4->TabStop = false;
			this->groupBox4->Text = L"Output 3";
			// 
			// input_voltage_text
			// 
			this->input_voltage_text->BackColor = System::Drawing::Color::LightGray;
			this->input_voltage_text->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->input_voltage_text->Cursor = System::Windows::Forms::Cursors::Hand;
			this->input_voltage_text->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 18, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->input_voltage_text->ForeColor = System::Drawing::Color::Black;
			this->input_voltage_text->Location = System::Drawing::Point(72, 12);
			this->input_voltage_text->Name = L"input_voltage_text";
			this->input_voltage_text->ReadOnly = true;
			this->input_voltage_text->Size = System::Drawing::Size(133, 35);
			this->input_voltage_text->TabIndex = 60;
			this->input_voltage_text->TabStop = false;
			this->input_voltage_text->Text = L"12.00 V";
			this->input_voltage_text->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->input_voltage_text->Click += gcnew System::EventHandler(this, &Form1::vin_cutoff_label_Click);
			// 
			// cutoff_text_label
			// 
			this->cutoff_text_label->AutoSize = true;
			this->cutoff_text_label->BackColor = System::Drawing::Color::LightGray;
			this->cutoff_text_label->Cursor = System::Windows::Forms::Cursors::Hand;
			this->cutoff_text_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->cutoff_text_label->Location = System::Drawing::Point(75, 15);
			this->cutoff_text_label->Name = L"cutoff_text_label";
			this->cutoff_text_label->Size = System::Drawing::Size(35, 13);
			this->cutoff_text_label->TabIndex = 61;
			this->cutoff_text_label->Text = L"Cutoff";
			this->cutoff_text_label->Click += gcnew System::EventHandler(this, &Form1::vin_cutoff_label_Click);
			// 
			// vin_cutoff_label
			// 
			this->vin_cutoff_label->AutoSize = true;
			this->vin_cutoff_label->BackColor = System::Drawing::Color::LightGray;
			this->vin_cutoff_label->Cursor = System::Windows::Forms::Cursors::Hand;
			this->vin_cutoff_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->vin_cutoff_label->Location = System::Drawing::Point(75, 31);
			this->vin_cutoff_label->Name = L"vin_cutoff_label";
			this->vin_cutoff_label->Size = System::Drawing::Size(35, 13);
			this->vin_cutoff_label->TabIndex = 62;
			this->vin_cutoff_label->Text = L"9.30V";
			this->vin_cutoff_label->Click += gcnew System::EventHandler(this, &Form1::vin_cutoff_label_Click);
			// 
			// groupBox5
			// 
			this->groupBox5->BackColor = System::Drawing::SystemColors::Window;
			this->groupBox5->Controls->Add(this->label19);
			this->groupBox5->Controls->Add(this->baudComboBox);
			this->groupBox5->Controls->Add(this->label1);
			this->groupBox5->Controls->Add(this->serialComboBox);
			this->groupBox5->Controls->Add(this->vin_cutoff_label);
			this->groupBox5->Controls->Add(this->cutoff_text_label);
			this->groupBox5->Controls->Add(this->input_voltage_text);
			this->groupBox5->Controls->Add(this->portLabel);
			this->groupBox5->Controls->Add(this->statusLabelP);
			this->groupBox5->Controls->Add(this->statusLabel);
			this->groupBox5->Controls->Add(this->connectButton);
			this->groupBox5->Controls->Add(this->disconnectButton);
			this->groupBox5->Location = System::Drawing::Point(0, 27);
			this->groupBox5->Name = L"groupBox5";
			this->groupBox5->Size = System::Drawing::Size(972, 54);
			this->groupBox5->TabIndex = 63;
			this->groupBox5->TabStop = false;
			// 
			// label19
			// 
			this->label19->AutoSize = true;
			this->label19->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->label19->Location = System::Drawing::Point(349, 22);
			this->label19->Name = L"label19";
			this->label19->Size = System::Drawing::Size(45, 17);
			this->label19->TabIndex = 65;
			this->label19->Text = L"Baud:";
			// 
			// baudComboBox
			// 
			this->baudComboBox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
			this->baudComboBox->FormattingEnabled = true;
			this->baudComboBox->Items->AddRange(gcnew cli::array< System::Object^  >(2) {L"9600", L"14400"});
			this->baudComboBox->Location = System::Drawing::Point(395, 19);
			this->baudComboBox->Name = L"baudComboBox";
			this->baudComboBox->Size = System::Drawing::Size(87, 24);
			this->baudComboBox->TabIndex = 64;
			this->baudComboBox->Text = L"9600";
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 18, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label1->Location = System::Drawing::Point(16, 14);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(50, 29);
			this->label1->TabIndex = 63;
			this->label1->Text = L"Vin";
			// 
			// calGroupBox
			// 
			this->calGroupBox->Controls->Add(this->cal_help_label);
			this->calGroupBox->Controls->Add(this->label16);
			this->calGroupBox->Controls->Add(this->label17);
			this->calGroupBox->Controls->Add(this->label18);
			this->calGroupBox->Controls->Add(this->label13);
			this->calGroupBox->Controls->Add(this->label14);
			this->calGroupBox->Controls->Add(this->label15);
			this->calGroupBox->Controls->Add(this->label12);
			this->calGroupBox->Controls->Add(this->label11);
			this->calGroupBox->Controls->Add(this->label10);
			this->calGroupBox->Controls->Add(this->label9);
			this->calGroupBox->Controls->Add(this->label8);
			this->calGroupBox->Controls->Add(this->label7);
			this->calGroupBox->Controls->Add(this->label6);
			this->calGroupBox->Controls->Add(this->label5);
			this->calGroupBox->Controls->Add(this->label4);
			this->calGroupBox->Controls->Add(this->label3);
			this->calGroupBox->Controls->Add(this->output3_error_text);
			this->calGroupBox->Controls->Add(this->output2_error_text);
			this->calGroupBox->Controls->Add(this->output1_error_text);
			this->calGroupBox->Controls->Add(this->out2_c3_text);
			this->calGroupBox->Controls->Add(this->out2_c2_text);
			this->calGroupBox->Controls->Add(this->out2_c1_text);
			this->calGroupBox->Controls->Add(this->out1_c3_text);
			this->calGroupBox->Controls->Add(this->out1_c2_text);
			this->calGroupBox->Controls->Add(this->out1_c1_text);
			this->calGroupBox->Controls->Add(this->out3_c3_text);
			this->calGroupBox->Controls->Add(this->out3_c2_text);
			this->calGroupBox->Controls->Add(this->out3_c1_text);
			this->calGroupBox->Controls->Add(this->cal_status_label);
			this->calGroupBox->Controls->Add(this->label2);
			this->calGroupBox->Controls->Add(this->accept_button);
			this->calGroupBox->Controls->Add(this->cal_cancel_button);
			this->calGroupBox->Controls->Add(this->cal_start_button);
			this->calGroupBox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->calGroupBox->Location = System::Drawing::Point(12, 232);
			this->calGroupBox->Name = L"calGroupBox";
			this->calGroupBox->Size = System::Drawing::Size(952, 215);
			this->calGroupBox->TabIndex = 64;
			this->calGroupBox->TabStop = false;
			this->calGroupBox->Text = L"Calibration and Test";
			this->calGroupBox->Visible = false;
			// 
			// cal_help_label
			// 
			this->cal_help_label->AutoSize = true;
			this->cal_help_label->Location = System::Drawing::Point(136, 0);
			this->cal_help_label->Name = L"cal_help_label";
			this->cal_help_label->Size = System::Drawing::Size(15, 16);
			this->cal_help_label->TabIndex = 33;
			this->cal_help_label->TabStop = true;
			this->cal_help_label->Text = L"\?";
			this->cal_help_label->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &Form1::cal_help_label_LinkClicked);
			// 
			// label16
			// 
			this->label16->AutoSize = true;
			this->label16->Location = System::Drawing::Point(666, 136);
			this->label16->Name = L"label16";
			this->label16->Size = System::Drawing::Size(24, 16);
			this->label16->TabIndex = 32;
			this->label16->Text = L"C2";
			// 
			// label17
			// 
			this->label17->AutoSize = true;
			this->label17->Location = System::Drawing::Point(666, 108);
			this->label17->Name = L"label17";
			this->label17->Size = System::Drawing::Size(24, 16);
			this->label17->TabIndex = 31;
			this->label17->Text = L"C1";
			// 
			// label18
			// 
			this->label18->AutoSize = true;
			this->label18->Location = System::Drawing::Point(666, 164);
			this->label18->Name = L"label18";
			this->label18->Size = System::Drawing::Size(24, 16);
			this->label18->TabIndex = 30;
			this->label18->Text = L"C3";
			// 
			// label13
			// 
			this->label13->AutoSize = true;
			this->label13->Location = System::Drawing::Point(517, 136);
			this->label13->Name = L"label13";
			this->label13->Size = System::Drawing::Size(24, 16);
			this->label13->TabIndex = 29;
			this->label13->Text = L"C2";
			// 
			// label14
			// 
			this->label14->AutoSize = true;
			this->label14->Location = System::Drawing::Point(517, 108);
			this->label14->Name = L"label14";
			this->label14->Size = System::Drawing::Size(24, 16);
			this->label14->TabIndex = 28;
			this->label14->Text = L"C1";
			// 
			// label15
			// 
			this->label15->AutoSize = true;
			this->label15->Location = System::Drawing::Point(517, 164);
			this->label15->Name = L"label15";
			this->label15->Size = System::Drawing::Size(24, 16);
			this->label15->TabIndex = 27;
			this->label15->Text = L"C3";
			// 
			// label12
			// 
			this->label12->AutoSize = true;
			this->label12->Location = System::Drawing::Point(355, 136);
			this->label12->Name = L"label12";
			this->label12->Size = System::Drawing::Size(24, 16);
			this->label12->TabIndex = 26;
			this->label12->Text = L"C2";
			// 
			// label11
			// 
			this->label11->AutoSize = true;
			this->label11->Location = System::Drawing::Point(355, 108);
			this->label11->Name = L"label11";
			this->label11->Size = System::Drawing::Size(24, 16);
			this->label11->TabIndex = 25;
			this->label11->Text = L"C1";
			// 
			// label10
			// 
			this->label10->AutoSize = true;
			this->label10->Location = System::Drawing::Point(355, 164);
			this->label10->Name = L"label10";
			this->label10->Size = System::Drawing::Size(24, 16);
			this->label10->TabIndex = 24;
			this->label10->Text = L"C3";
			// 
			// label9
			// 
			this->label9->AutoSize = true;
			this->label9->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label9->Location = System::Drawing::Point(689, 83);
			this->label9->Name = L"label9";
			this->label9->Size = System::Drawing::Size(110, 16);
			this->label9->TabIndex = 23;
			this->label9->Text = L"Out 3 Coefficients";
			// 
			// label8
			// 
			this->label8->AutoSize = true;
			this->label8->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label8->Location = System::Drawing::Point(540, 83);
			this->label8->Name = L"label8";
			this->label8->Size = System::Drawing::Size(110, 16);
			this->label8->TabIndex = 22;
			this->label8->Text = L"Out 2 Coefficients";
			// 
			// label7
			// 
			this->label7->AutoSize = true;
			this->label7->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label7->Location = System::Drawing::Point(379, 83);
			this->label7->Name = L"label7";
			this->label7->Size = System::Drawing::Size(110, 16);
			this->label7->TabIndex = 21;
			this->label7->Text = L"Out 1 Coefficients";
			// 
			// label6
			// 
			this->label6->AutoSize = true;
			this->label6->Location = System::Drawing::Point(113, 164);
			this->label6->Name = L"label6";
			this->label6->Size = System::Drawing::Size(88, 16);
			this->label6->TabIndex = 20;
			this->label6->Text = L"Output 3 Error";
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->Location = System::Drawing::Point(113, 136);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(88, 16);
			this->label5->TabIndex = 19;
			this->label5->Text = L"Output 2 Error";
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Location = System::Drawing::Point(113, 108);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(88, 16);
			this->label4->TabIndex = 18;
			this->label4->Text = L"Output 1 Error";
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->label3->Location = System::Drawing::Point(224, 83);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(53, 16);
			this->label3->TabIndex = 17;
			this->label3->Text = L"Results";
			// 
			// output3_error_text
			// 
			this->output3_error_text->Location = System::Drawing::Point(203, 161);
			this->output3_error_text->Name = L"output3_error_text";
			this->output3_error_text->ReadOnly = true;
			this->output3_error_text->Size = System::Drawing::Size(100, 22);
			this->output3_error_text->TabIndex = 16;
			// 
			// output2_error_text
			// 
			this->output2_error_text->Location = System::Drawing::Point(203, 133);
			this->output2_error_text->Name = L"output2_error_text";
			this->output2_error_text->ReadOnly = true;
			this->output2_error_text->Size = System::Drawing::Size(100, 22);
			this->output2_error_text->TabIndex = 15;
			// 
			// output1_error_text
			// 
			this->output1_error_text->Location = System::Drawing::Point(203, 105);
			this->output1_error_text->Name = L"output1_error_text";
			this->output1_error_text->ReadOnly = true;
			this->output1_error_text->Size = System::Drawing::Size(100, 22);
			this->output1_error_text->TabIndex = 14;
			// 
			// out2_c3_text
			// 
			this->out2_c3_text->Location = System::Drawing::Point(543, 161);
			this->out2_c3_text->Name = L"out2_c3_text";
			this->out2_c3_text->ReadOnly = true;
			this->out2_c3_text->Size = System::Drawing::Size(100, 22);
			this->out2_c3_text->TabIndex = 13;
			// 
			// out2_c2_text
			// 
			this->out2_c2_text->Location = System::Drawing::Point(543, 133);
			this->out2_c2_text->Name = L"out2_c2_text";
			this->out2_c2_text->ReadOnly = true;
			this->out2_c2_text->Size = System::Drawing::Size(100, 22);
			this->out2_c2_text->TabIndex = 12;
			// 
			// out2_c1_text
			// 
			this->out2_c1_text->Location = System::Drawing::Point(543, 105);
			this->out2_c1_text->Name = L"out2_c1_text";
			this->out2_c1_text->ReadOnly = true;
			this->out2_c1_text->Size = System::Drawing::Size(100, 22);
			this->out2_c1_text->TabIndex = 11;
			// 
			// out1_c3_text
			// 
			this->out1_c3_text->Location = System::Drawing::Point(382, 161);
			this->out1_c3_text->Name = L"out1_c3_text";
			this->out1_c3_text->ReadOnly = true;
			this->out1_c3_text->Size = System::Drawing::Size(100, 22);
			this->out1_c3_text->TabIndex = 10;
			// 
			// out1_c2_text
			// 
			this->out1_c2_text->Location = System::Drawing::Point(382, 133);
			this->out1_c2_text->Name = L"out1_c2_text";
			this->out1_c2_text->ReadOnly = true;
			this->out1_c2_text->Size = System::Drawing::Size(100, 22);
			this->out1_c2_text->TabIndex = 9;
			// 
			// out1_c1_text
			// 
			this->out1_c1_text->Location = System::Drawing::Point(381, 105);
			this->out1_c1_text->Name = L"out1_c1_text";
			this->out1_c1_text->ReadOnly = true;
			this->out1_c1_text->Size = System::Drawing::Size(100, 22);
			this->out1_c1_text->TabIndex = 8;
			// 
			// out3_c3_text
			// 
			this->out3_c3_text->Location = System::Drawing::Point(692, 161);
			this->out3_c3_text->Name = L"out3_c3_text";
			this->out3_c3_text->ReadOnly = true;
			this->out3_c3_text->Size = System::Drawing::Size(100, 22);
			this->out3_c3_text->TabIndex = 7;
			// 
			// out3_c2_text
			// 
			this->out3_c2_text->Location = System::Drawing::Point(692, 133);
			this->out3_c2_text->Name = L"out3_c2_text";
			this->out3_c2_text->ReadOnly = true;
			this->out3_c2_text->Size = System::Drawing::Size(100, 22);
			this->out3_c2_text->TabIndex = 6;
			// 
			// out3_c1_text
			// 
			this->out3_c1_text->Location = System::Drawing::Point(692, 105);
			this->out3_c1_text->Name = L"out3_c1_text";
			this->out3_c1_text->ReadOnly = true;
			this->out3_c1_text->Size = System::Drawing::Size(100, 22);
			this->out3_c1_text->TabIndex = 5;
			// 
			// cal_status_label
			// 
			this->cal_status_label->AutoSize = true;
			this->cal_status_label->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->cal_status_label->Location = System::Drawing::Point(74, 28);
			this->cal_status_label->Name = L"cal_status_label";
			this->cal_status_label->Size = System::Drawing::Size(30, 16);
			this->cal_status_label->TabIndex = 4;
			this->cal_status_label->Text = L"Idle";
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(13, 28);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(48, 16);
			this->label2->TabIndex = 3;
			this->label2->Text = L"Status:";
			// 
			// accept_button
			// 
			this->accept_button->Enabled = false;
			this->accept_button->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->accept_button->Location = System::Drawing::Point(213, 55);
			this->accept_button->Name = L"accept_button";
			this->accept_button->Size = System::Drawing::Size(75, 23);
			this->accept_button->TabIndex = 2;
			this->accept_button->Text = L"Accept";
			this->accept_button->UseVisualStyleBackColor = true;
			this->accept_button->Click += gcnew System::EventHandler(this, &Form1::accept_button_Click);
			// 
			// cal_cancel_button
			// 
			this->cal_cancel_button->Enabled = false;
			this->cal_cancel_button->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->cal_cancel_button->Location = System::Drawing::Point(92, 55);
			this->cal_cancel_button->Name = L"cal_cancel_button";
			this->cal_cancel_button->Size = System::Drawing::Size(75, 23);
			this->cal_cancel_button->TabIndex = 1;
			this->cal_cancel_button->Text = L"Cancel";
			this->cal_cancel_button->UseVisualStyleBackColor = true;
			this->cal_cancel_button->Click += gcnew System::EventHandler(this, &Form1::cal_cancel_button_Click);
			// 
			// cal_start_button
			// 
			this->cal_start_button->Enabled = false;
			this->cal_start_button->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->cal_start_button->Location = System::Drawing::Point(11, 55);
			this->cal_start_button->Name = L"cal_start_button";
			this->cal_start_button->Size = System::Drawing::Size(75, 23);
			this->cal_start_button->TabIndex = 0;
			this->cal_start_button->Text = L"Start";
			this->cal_start_button->UseVisualStyleBackColor = true;
			this->cal_start_button->Click += gcnew System::EventHandler(this, &Form1::cal_start_button_Click);
			// 
			// CAL_timer
			// 
			this->CAL_timer->Tick += gcnew System::EventHandler(this, &Form1::CAL_timer_Tick);
			// 
			// packetResponseTimer
			// 
			this->packetResponseTimer->Interval = 550;
			this->packetResponseTimer->Tick += gcnew System::EventHandler(this, &Form1::packetResponseTimer_Tick);
			// 
			// menuStrip1
			// 
			this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {this->toolStripMenuItem1, 
				this->optionsToolStripMenuItem, this->helpToolStripMenuItem});
			this->menuStrip1->Location = System::Drawing::Point(0, 0);
			this->menuStrip1->Name = L"menuStrip1";
			this->menuStrip1->Size = System::Drawing::Size(970, 24);
			this->menuStrip1->TabIndex = 65;
			this->menuStrip1->Text = L"menuStrip1";
			// 
			// toolStripMenuItem1
			// 
			this->toolStripMenuItem1->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->exitMenuItem});
			this->toolStripMenuItem1->Name = L"toolStripMenuItem1";
			this->toolStripMenuItem1->Size = System::Drawing::Size(37, 20);
			this->toolStripMenuItem1->Text = L"File";
			// 
			// exitMenuItem
			// 
			this->exitMenuItem->Name = L"exitMenuItem";
			this->exitMenuItem->Size = System::Drawing::Size(92, 22);
			this->exitMenuItem->Text = L"Exit";
			this->exitMenuItem->Click += gcnew System::EventHandler(this, &Form1::exitMenuItem_Click);
			// 
			// optionsToolStripMenuItem
			// 
			this->optionsToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->showCalibrationToolsMenuItem, 
				this->hideCalibrationToolsMenuItem});
			this->optionsToolStripMenuItem->Name = L"optionsToolStripMenuItem";
			this->optionsToolStripMenuItem->Size = System::Drawing::Size(61, 20);
			this->optionsToolStripMenuItem->Text = L"Options";
			// 
			// showCalibrationToolsMenuItem
			// 
			this->showCalibrationToolsMenuItem->Name = L"showCalibrationToolsMenuItem";
			this->showCalibrationToolsMenuItem->Size = System::Drawing::Size(196, 22);
			this->showCalibrationToolsMenuItem->Text = L"Show Calibration Tools";
			this->showCalibrationToolsMenuItem->Click += gcnew System::EventHandler(this, &Form1::showCalibrationToolsMenuItem_Click);
			// 
			// hideCalibrationToolsMenuItem
			// 
			this->hideCalibrationToolsMenuItem->Enabled = false;
			this->hideCalibrationToolsMenuItem->Name = L"hideCalibrationToolsMenuItem";
			this->hideCalibrationToolsMenuItem->Size = System::Drawing::Size(196, 22);
			this->hideCalibrationToolsMenuItem->Text = L"Hide Calibration Tools";
			this->hideCalibrationToolsMenuItem->Click += gcnew System::EventHandler(this, &Form1::hideCalibrationToolsMenuItem_Click);
			// 
			// helpToolStripMenuItem
			// 
			this->helpToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->serialPortSettingsToolStripMenuItem, 
				this->aboutMenuItem});
			this->helpToolStripMenuItem->Name = L"helpToolStripMenuItem";
			this->helpToolStripMenuItem->Size = System::Drawing::Size(44, 20);
			this->helpToolStripMenuItem->Text = L"Help";
			// 
			// aboutMenuItem
			// 
			this->aboutMenuItem->Name = L"aboutMenuItem";
			this->aboutMenuItem->Size = System::Drawing::Size(172, 22);
			this->aboutMenuItem->Text = L"About";
			this->aboutMenuItem->Click += gcnew System::EventHandler(this, &Form1::aboutMenuItem_Click);
			// 
			// serialPortSettingsToolStripMenuItem
			// 
			this->serialPortSettingsToolStripMenuItem->Name = L"serialPortSettingsToolStripMenuItem";
			this->serialPortSettingsToolStripMenuItem->Size = System::Drawing::Size(172, 22);
			this->serialPortSettingsToolStripMenuItem->Text = L"Serial Port Settings";
			this->serialPortSettingsToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::serialPortSettingsToolStripMenuItem_Click);
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::SystemColors::Window;
			this->ClientSize = System::Drawing::Size(970, 234);
			this->Controls->Add(this->calGroupBox);
			this->Controls->Add(this->groupBox5);
			this->Controls->Add(this->groupBox4);
			this->Controls->Add(this->groupBox3);
			this->Controls->Add(this->groupBox2);
			this->Controls->Add(this->menuStrip1);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::Fixed3D;
			this->MainMenuStrip = this->menuStrip1;
			this->MaximizeBox = false;
			this->Name = L"Form1";
			this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
			this->Text = L"Hydra - Not Connected";
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->max_current3_trackbar))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->max_current2_trackbar))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->max_current1_trackbar))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->output1_trackbar))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->output2_trackbar))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->output3_trackbar))->EndInit();
			this->groupBox2->ResumeLayout(false);
			this->groupBox2->PerformLayout();
			this->groupBox3->ResumeLayout(false);
			this->groupBox3->PerformLayout();
			this->groupBox4->ResumeLayout(false);
			this->groupBox4->PerformLayout();
			this->groupBox5->ResumeLayout(false);
			this->groupBox5->PerformLayout();
			this->calGroupBox->ResumeLayout(false);
			this->calGroupBox->PerformLayout();
			this->menuStrip1->ResumeLayout(false);
			this->menuStrip1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e) {
			 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: output1_lowc_enabled_CheckedChanged(System::Object^  sender, System::EventArgs^  e) 
		* Description:	
		*	Handles supply enable/disable functionality
		------------------------------------------------------------------------------------------------------- */
private: System::Void output1_lowc_enabled_CheckedChanged(System::Object^  sender, System::EventArgs^  e) 
		 {
			 // Only send the change if the check box was changed as the result of the user clicking, not the app reading the checked state and applying it for the first time.
			 if( this->have_config_data )
			 {
				 // Set outputs to indicate that supply is disabled
				 if( output1_lowc_enabled_Checkbox->Checked )
				 {
					 this->low_current1_enabled = 1;
				 }
				 else
				 {
					 this->low_current1_enabled = 0;
				 }

				 change_out1_back_color( CHANGE_PENDING_COLOR );

				 // Set flag indicating that the settings have been changed.
				 this->out1_changed = 1;
			 
				 // Send new settings.
				 send_v1_settings();
			 }
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: output2_lowc_enabled_CheckedChanged(System::Object^  sender, System::EventArgs^  e) 
		* Description:	
		*	Handles supply enable/disable functionality
		------------------------------------------------------------------------------------------------------- */
private: System::Void output2_lowc_enabled_CheckedChanged(System::Object^  sender, System::EventArgs^  e) 
		 {
			 // Only send the change if the check box was changed as the result of the user clicking, not the app reading the checked state and applying it for the first time.
			 if( this->have_config_data )
			 {
				 // Set outputs to indicate that supply is disabled
				 if( output2_lowc_enabled_Checkbox->Checked )
				 {
					 this->low_current2_enabled = 1;
				 }
				 else
				 {
					 this->low_current2_enabled = 0;
				 }

				 change_out2_back_color( CHANGE_PENDING_COLOR );

				 // Set flag indicating that the settings have been changed.
				 this->out2_changed = 1;
			 
				 // Send new settings.
				 send_v2_settings();
			 }
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: output2_lowc_enabled_CheckedChanged(System::Object^  sender, System::EventArgs^  e) 
		* Description:	
		*	Handles supply enable/disable functionality
		------------------------------------------------------------------------------------------------------- */
private: System::Void output3_lowc_enabled_CheckedChanged(System::Object^  sender, System::EventArgs^  e) 
		 {
			 // Only send the change if the check box was changed as the result of the user clicking, not the app reading the checked state and applying it for the first time.
			 if( this->have_config_data )
			 {
				 // Set outputs to indicate that supply is disabled
				 if( output3_lowc_enabled_Checkbox->Checked )
				 {
					 this->low_current3_enabled = 1;
				 }
				 else
				 {
					 this->low_current3_enabled = 0;
				 }

				 change_out3_back_color( CHANGE_PENDING_COLOR );

				 // Set flag indicating that the settings have been changed.
				 this->out3_changed = 1;
			 
				 // Send new settings.
				 send_v3_settings();
			 }
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: output1_enabled_CheckedChanged(System::Object^  sender, System::EventArgs^  e) 
		* Description:	
		*	Handles supply enable/disable functionality
		------------------------------------------------------------------------------------------------------- */
private: System::Void output1_enabled_CheckedChanged(System::Object^  sender, System::EventArgs^  e) 
		 {
			 // Only send the change if the check box was changed as the result of the user clicking, not the app reading the checked state and applying it for the first time.
			 if( this->have_config_data )
			 {
				 // Set outputs to indicate that supply is disabled
				 if( output1_enabled_checkbox->Checked )
				 {
					 this->output1_enabled = 1;
				 }
				 else
				 {
					 this->output1_enabled = 0;
				 }

				 change_out1_back_color( CHANGE_PENDING_COLOR );

				 // Set flag indicating that the settings have been changed.
				 this->out1_changed = 1;
			 
				 // Send new settings.
				 send_v1_settings();
			 }
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: checkBox2_CheckedChanged(System::Object^  sender, System::EventArgs^  e) 
		* Description:	
		*	Handles supply enable/disable functionality
		------------------------------------------------------------------------------------------------------- */
private: System::Void output2_enabled_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {

			 // Only send the change if the check box was changed as the result of the user clicking, not the app reading the checked state and applying it for the first time.
			 if( this->have_config_data )
			 {
				 // Set outputs to indicate that supply is disabled
				 if( output2_enabled_checkbox->Checked )
				 {
					 this->output2_enabled = 1;
				 }
				 else
				 {
					 this->output2_enabled = 0;
				 }

				 change_out2_back_color( CHANGE_PENDING_COLOR );

				 // Set flag indicating that the settings have been changed.
				 this->out2_changed = 1;
			 
				 // Send new settings.
				 send_v2_settings();
			 }

		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: output3_enabled_checkbox_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
		* Description:	
		*	Handles supply enable/disable functionality
		------------------------------------------------------------------------------------------------------- */
private: System::Void output3_enabled_checkbox_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {

			 // Only send the change if the check box was changed as the result of the user clicking, not the app reading the checked state and applying it for the first time.
			 if( this->have_config_data )
			 {
				 // Set outputs to indicate that supply is disabled
				 if( output3_enabled_checkbox->Checked )
				 {
					 this->output3_enabled = 1;
				 }
				 else
				 {
					 this->output3_enabled = 0;
				 }

				 change_out3_back_color( CHANGE_PENDING_COLOR );

				 // Set flag indicating that the settings have been changed.
				 this->out3_changed = 1;
			 
				 // Send new settings.
				 send_v3_settings();
			 }
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: output1_trackbar_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
		* Description:	
		*	Starts the process to change the target power supply voltage and display the new target in the voltage box.
		------------------------------------------------------------------------------------------------------- */
private: System::Void output1_trackbar_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
			 
			 this->out1_changing = 1;

			 change_out1_back_color(CHANGE_PENDING_COLOR);
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: output1_trackbar_Scroll(System::Object^  sender, System::EventArgs^  e)
		* Description:	
		*	Updates the displayed voltage to indicate what the new target will be when the slider is released.
		------------------------------------------------------------------------------------------------------- */
private: System::Void output1_trackbar_Scroll(System::Object^  sender, System::EventArgs^  e) {
			 this->v1_target = (float)output1_trackbar->Value/10.0f;
			 v1_target_label->Text = String::Concat( gcnew String(this->v1_target.ToString("F2")), gcnew System::String("V") );
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: output1_trackbar_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
		* Description:	
		*	Finishes the process to change the target power supply voltage and display the new target in the voltage box.
		------------------------------------------------------------------------------------------------------- */
private: System::Void output1_trackbar_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {

			 // Set flag indicating that we should watch for a packet showing a successfull configuration write.
			 // In the MouseDown function call, we changed the background color of the voltage and current indicators
			 // to show that we are changing them.  Once we receive the packet response, we'll change the color back.
			 // If enough time elapses without receiving the packet, we'll send the config write again.
			 this->out1_changed = 1;

			 send_v1_settings();
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: output2_trackbar_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
		* Description:	
		*	Starts the process to change the target power supply voltage and display the new target in the voltage box.
		------------------------------------------------------------------------------------------------------- */
private: System::Void output2_trackbar_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
			 
			 this->out2_changing = 1;

			 change_out2_back_color(CHANGE_PENDING_COLOR);
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: output2_trackbar_Scroll(System::Object^  sender, System::EventArgs^  e)
		* Description:	
		*	Updates the displayed voltage to indicate what the new target will be when the slider is released.
		------------------------------------------------------------------------------------------------------- */
private: System::Void output2_trackbar_Scroll(System::Object^  sender, System::EventArgs^  e) {
			 this->v2_target = (float)output2_trackbar->Value/10.0f;
			 v2_target_label->Text = String::Concat( gcnew String(this->v2_target.ToString("F2")), gcnew System::String("V") );
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: output2_trackbar_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
		* Description:	
		*	Finishes the process to change the target power supply voltage and display the new target in the voltage box.
		------------------------------------------------------------------------------------------------------- */
private: System::Void output2_trackbar_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {

			 // Set flag indicating that we should watch for a packet showing a successfull configuration write.
			 // In the MouseDown function call, we changed the background color of the voltage and current indicators
			 // to show that we are changing them.  Once we receive the packet response, we'll change the color back.
			 // If enough time elapses without receiving the packet, we'll send the config write again.
			 this->out2_changed = 1;

			 send_v2_settings();
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: output3_trackbar_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
		* Description:	
		*	Starts the process to change the target power supply voltage and display the new target in the voltage box.
		------------------------------------------------------------------------------------------------------- */
private: System::Void output3_trackbar_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
			 
			 this->out3_changing = 1;

			 change_out3_back_color(CHANGE_PENDING_COLOR);
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: output3_trackbar_Scroll(System::Object^  sender, System::EventArgs^  e)
		* Description:	
		*	Updates the displayed voltage to indicate what the new target will be when the slider is released.
		------------------------------------------------------------------------------------------------------- */
private: System::Void output3_trackbar_Scroll(System::Object^  sender, System::EventArgs^  e) {
			 this->v3_target = (float)output3_trackbar->Value/10.0f;
			 v3_target_label->Text = String::Concat( gcnew String(this->v3_target.ToString("F2")), gcnew System::String("V") );
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: output2_trackbar_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
		* Description:	
		*	Finishes the process to change the target power supply voltage and display the new target in the voltage box.
		------------------------------------------------------------------------------------------------------- */
private: System::Void output3_trackbar_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {

			 // Set flag indicating that we should watch for a packet showing a successfull configuration write.
			 // In the MouseDown function call, we changed the background color of the voltage and current indicators
			 // to show that we are changing them.  Once we receive the packet response, we'll change the color back.
			 // If enough time elapses without receiving the packet, we'll send the config write again.
			 this->out3_changed = 1;

			 send_v3_settings();
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: change_out1_back_color( System::Drawing::Color )
		* Description:	
		*	Sets the background color of all display elements on supply 1
		------------------------------------------------------------------------------------------------------- */
private: System::Void change_vin_back_color( System::Drawing::Color new_color )
		 {
			 input_voltage_text->BackColor = new_color;
			 vin_cutoff_label->BackColor = new_color;
			 cutoff_text_label->BackColor = new_color;
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: change_out1_back_color( System::Drawing::Color )
		* Description:	
		*	Sets the background color of all display elements on supply 1
		------------------------------------------------------------------------------------------------------- */
private: System::Void change_out1_back_color( System::Drawing::Color new_color )
		 {
			 output1_voltage_text->BackColor = new_color;
			 output1_current_text->BackColor = new_color;

			 c1_status_label->BackColor = new_color;
			 v1_status_label->BackColor = new_color;

			 v1_target_label->BackColor = new_color;
			 c1_max_label->BackColor = new_color;
		 }

		 /* -------------------------------------------------------------------------------------------------------
		* Name: change_out2_back_color( System::Drawing::Color )
		* Description:	
		*	Sets the background color of all display elements on supply 2
		------------------------------------------------------------------------------------------------------- */
private: System::Void change_out2_back_color( System::Drawing::Color new_color )
		 {
			 output2_voltage_text->BackColor = new_color;
			 output2_current_text->BackColor = new_color;

			 c2_status_label->BackColor = new_color;
			 v2_status_label->BackColor = new_color;

			 v2_target_label->BackColor = new_color;
			 c2_max_label->BackColor = new_color;
		 }

		 /* -------------------------------------------------------------------------------------------------------
		* Name: change_out3_back_color( System::Drawing::Color )
		* Description:	
		*	Sets the background color of all display elements on supply 1
		------------------------------------------------------------------------------------------------------- */
private: System::Void change_out3_back_color( System::Drawing::Color new_color )
		 {
			 output3_voltage_text->BackColor = new_color;
			 output3_current_text->BackColor = new_color;

			 c3_status_label->BackColor = new_color;
			 v3_status_label->BackColor = new_color;

			 v3_target_label->BackColor = new_color;
			 c3_max_label->BackColor = new_color;
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: max_current1_trackbar_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
		* Description:	
		*	Starts process to change max current with the trackbar.
		------------------------------------------------------------------------------------------------------- */
private: System::Void max_current1_trackbar_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {

			 this->out1_changing = 1;

			 change_out1_back_color(CHANGE_PENDING_COLOR);
		 }

		 /* -------------------------------------------------------------------------------------------------------
		* Name: max_current1_trackbar_Scroll(System::Object^  sender, System::EventArgs^  e)
		* Description:	
		*	Tracks changes to the maximum current setting
		------------------------------------------------------------------------------------------------------- */
private: System::Void max_current1_trackbar_Scroll(System::Object^  sender, System::EventArgs^  e) {
			 this->iout1_max = (float)this->max_current1_trackbar->Value*0.05f;
			 c1_max_label->Text = System::String::Concat(  gcnew String(this->iout1_max.ToString("F2")), gcnew String("A")  );
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: output1_trackbar_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
		* Description:	
		*	Finishes the process to change the target power supply voltage and display the new target in the voltage box.
		------------------------------------------------------------------------------------------------------- */
private: System::Void max_current1_trackbar_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {

			 // Set flag indicating that we should watch for a packet showing a successfull configuration write.
			 // In the MouseDown function call, we changed the background color of the voltage and current indicators
			 // to show that we are changing them.  Once we receive the packet response, we'll change the color back.
			 // If enough time elapses without receiving the packet, we'll send the config write again.
			 this->out1_changed = 1;

			 send_v1_settings();
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: max_current2_trackbar_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
		* Description:	
		*	Starts process to change max current with the trackbar.
		------------------------------------------------------------------------------------------------------- */
private: System::Void max_current2_trackbar_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {

			 this->out2_changing = 1;

			 change_out2_back_color(CHANGE_PENDING_COLOR);
		 }

		 /* -------------------------------------------------------------------------------------------------------
		* Name: max_current2_trackbar_Scroll(System::Object^  sender, System::EventArgs^  e)
		* Description:	
		*	Tracks changes to the maximum current setting
		------------------------------------------------------------------------------------------------------- */
private: System::Void max_current2_trackbar_Scroll(System::Object^  sender, System::EventArgs^  e) {
			 this->iout2_max = (float)this->max_current2_trackbar->Value*0.05f;
			 c2_max_label->Text = System::String::Concat(  gcnew String(this->iout2_max.ToString("F2")), gcnew String("A")  );
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: output2_trackbar_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
		* Description:	
		*	Finishes the process to change the target power supply voltage and display the new target in the voltage box.
		------------------------------------------------------------------------------------------------------- */
private: System::Void max_current2_trackbar_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {

			 // Set flag indicating that we should watch for a packet showing a successfull configuration write.
			 // In the MouseDown function call, we changed the background color of the voltage and current indicators
			 // to show that we are changing them.  Once we receive the packet response, we'll change the color back.
			 // If enough time elapses without receiving the packet, we'll send the config write again.
			 this->out2_changed = 1;

			 send_v2_settings();
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: max_current3_trackbar_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
		* Description:	
		*	Starts process to change max current with the trackbar.
		------------------------------------------------------------------------------------------------------- */
private: System::Void max_current3_trackbar_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {

			 this->out3_changing = 1;

			 change_out3_back_color(CHANGE_PENDING_COLOR);
		 }

		 /* -------------------------------------------------------------------------------------------------------
		* Name: max_current3_trackbar_Scroll(System::Object^  sender, System::EventArgs^  e)
		* Description:	
		*	Tracks changes to the maximum current setting
		------------------------------------------------------------------------------------------------------- */
private: System::Void max_current3_trackbar_Scroll(System::Object^  sender, System::EventArgs^  e) {
			 this->iout3_max = (float)this->max_current3_trackbar->Value*0.05f;
			 c3_max_label->Text = System::String::Concat(  gcnew String(this->iout3_max.ToString("F2")), gcnew String("A")  );
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: output3_trackbar_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
		* Description:	
		*	Finishes the process to change the target power supply voltage and display the new target in the voltage box.
		------------------------------------------------------------------------------------------------------- */
private: System::Void max_current3_trackbar_MouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {

			 // Set flag indicating that we should watch for a packet showing a successfull configuration write.
			 // In the MouseDown function call, we changed the background color of the voltage and current indicators
			 // to show that we are changing them.  Once we receive the packet response, we'll change the color back.
			 // If enough time elapses without receiving the packet, we'll send the config write again.
			 this->out3_changed = 1;

			 send_v3_settings();
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: send_v1_settings()
		* Description:	
		*	Sends a packet to write the settings for controlling output 1 on the Hydra
		------------------------------------------------------------------------------------------------------- */
private: System::Void send_v1_settings()
		 {
			  
			  SerialPacket^ packet = gcnew SerialPacket();

			  UInt32 reg_value = 0;

			  if( this->output1_enabled )
			  {
				  reg_value |= (UInt32)1 << 31;
			  }

			  if( this->low_current1_enabled )
			  {
				  reg_value |= (UInt32)1 << 30;
			  }

			  reg_value |= ((UInt32)(this->v1_target / VOUT_REG_SCALE) & 0x0FFFF);
			  reg_value |= (((UInt32)(this->iout1_max / IOUT_REG_SCALE)) & 0x0FFF) << 16;

			  // Construct a packet and send it
			  packet->Address = V1_SETTINGS_REG_ADDRESS;
			  packet->HasData = true;
			  packet->IsBatch = false;
			  packet->SetDataByte(0, (reg_value >> 24) & 0x0FF);
			  packet->SetDataByte(1, (reg_value >> 16) & 0x0FF);
			  packet->SetDataByte(2, (reg_value >> 8) & 0x0FF);
			  packet->SetDataByte(3, reg_value & 0x0FF);

			  packet->ComputeChecksum();

			  AddTXPacket( packet );
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: send_v2_settings()
		* Description:	
		*	Sends a packet to write the settings for controlling output 2 on the Hydra
		------------------------------------------------------------------------------------------------------- */
private: System::Void send_v2_settings()
		 {
			  SerialPacket^ packet = gcnew SerialPacket();

			  UInt32 reg_value = 0;

			  if( this->output2_enabled )
			  {
				  reg_value |= (UInt32)1 << 31;
			  }

			  if( this->low_current2_enabled )
			  {
				  reg_value |= (UInt32)1 << 30;
			  }

			  reg_value |= ((UInt32)(this->v2_target / VOUT_REG_SCALE) & 0x0FFFF);
			  reg_value |= (((UInt32)(this->iout2_max / IOUT_REG_SCALE)) & 0x0FFF) << 16;

			  // Construct a packet and send it
			  packet->Address = V2_SETTINGS_REG_ADDRESS;
			  packet->HasData = true;
			  packet->IsBatch = false;
			  packet->SetDataByte(0, (reg_value >> 24) & 0x0FF);
			  packet->SetDataByte(1, (reg_value >> 16) & 0x0FF);
			  packet->SetDataByte(2, (reg_value >> 8) & 0x0FF);
			  packet->SetDataByte(3, reg_value & 0x0FF);

			  packet->ComputeChecksum();

			  AddTXPacket( packet );
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: send_v3_settings()
		* Description:	
		*	Sends a packet to write the settings for controlling output 2 on the Hydra
		------------------------------------------------------------------------------------------------------- */
private: System::Void send_v3_settings()
		 {
			  SerialPacket^ packet = gcnew SerialPacket();

			  UInt32 reg_value = 0;

			  if( this->output3_enabled )
			  {
				  reg_value |= (UInt32)1 << 31;
			  }

			  if( this->low_current3_enabled )
			  {
				  reg_value |= (UInt32)1 << 30;
			  }

			  reg_value |= ((UInt32)(this->v3_target / VOUT_REG_SCALE) & 0x0FFFF);
			  reg_value |= (((UInt32)(this->iout3_max / IOUT_REG_SCALE)) & 0x0FFF) << 16;

			  // Construct a packet and send it
			  packet->Address = V3_SETTINGS_REG_ADDRESS;
			  packet->HasData = true;
			  packet->IsBatch = false;
			  packet->SetDataByte(0, (reg_value >> 24) & 0x0FF);
			  packet->SetDataByte(1, (reg_value >> 16) & 0x0FF);
			  packet->SetDataByte(2, (reg_value >> 8) & 0x0FF);
			  packet->SetDataByte(3, reg_value & 0x0FF);

			  packet->ComputeChecksum();

			  AddTXPacket( packet );
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: send_all_settings()
		* Description:	
		*	Sends a packet to write the settings for controlling all the outputs on the Hydra
		------------------------------------------------------------------------------------------------------- */
		 private: System::Void send_all_settings( void )
		 {
			 SerialPacket^ packet = gcnew SerialPacket();

			  UInt32 reg1_value = 0;
			  UInt32 reg2_value = 0;
			  UInt32 reg3_value = 0;
			  UInt32 reg4_value = 0;

			  if( this->output1_enabled )
			  {
				  reg1_value |= (UInt32)1 << 31;
			  }

			  if( this->low_current1_enabled )
			  {
				  reg1_value |= (UInt32)1 << 30;
			  }

			  reg1_value |= ((UInt32)(this->v1_target / VOUT_REG_SCALE) & 0x0FFFF);
			  reg1_value |= (((UInt32)(this->iout1_max / IOUT_REG_SCALE)) & 0x0FFF) << 16;

			  if( this->output2_enabled )
			  {
				  reg2_value |= (UInt32)1 << 31;
			  }

			  if( this->low_current2_enabled )
			  {
				  reg2_value |= (UInt32)1 << 30;
			  }

			  reg2_value |= ((UInt32)(this->v3_target / VOUT_REG_SCALE) & 0x0FFFF);
			  reg2_value |= (((UInt32)(this->iout3_max / IOUT_REG_SCALE)) & 0x0FFF) << 16;

			  if( this->output3_enabled )
			  {
				  reg3_value |= (UInt32)1 << 31;
			  }

			  if( this->low_current3_enabled )
			  {
				  reg3_value |= (UInt32)1 << 30;
			  }

			  reg3_value |= ((UInt32)(this->v3_target / VOUT_REG_SCALE) & 0x0FFFF);
			  reg3_value |= (((UInt32)(this->iout3_max / IOUT_REG_SCALE)) & 0x0FFF) << 16;

			  reg4_value = ((UInt32)(this->vin_cutoff / VOUT_REG_SCALE) & 0x0FFFF);

			  reg4_value |= ((this->cutoff1_override & 0x01) << 31);
			  reg4_value |= ((this->cutoff2_override & 0x01) << 30);
			  reg4_value |= ((this->cutoff3_override & 0x01) << 29);

			  // Construct a packet and send it
			  packet->Address = V1_SETTINGS_REG_ADDRESS;
			  packet->HasData = true;
			  packet->IsBatch = true;
			  packet->BatchLength = 4;
			  packet->SetDataByte(0, (reg1_value >> 24) & 0x0FF);
			  packet->SetDataByte(1, (reg1_value >> 16) & 0x0FF);
			  packet->SetDataByte(2, (reg1_value >> 8) & 0x0FF);
			  packet->SetDataByte(3, reg1_value & 0x0FF);
			  packet->SetDataByte(4, (reg2_value >> 24) & 0x0FF);
			  packet->SetDataByte(5, (reg2_value >> 16) & 0x0FF);
			  packet->SetDataByte(6, (reg2_value >> 8) & 0x0FF);
			  packet->SetDataByte(7, reg2_value & 0x0FF);
			  packet->SetDataByte(8, (reg3_value >> 24) & 0x0FF);
			  packet->SetDataByte(9, (reg3_value >> 16) & 0x0FF);
			  packet->SetDataByte(10, (reg3_value >> 8) & 0x0FF);
			  packet->SetDataByte(11, reg3_value & 0x0FF);
			  packet->SetDataByte(12, (reg4_value >> 24) & 0x0FF);
			  packet->SetDataByte(13, (reg4_value >> 16) & 0x0FF);
			  packet->SetDataByte(14, (reg4_value >> 8) & 0x0FF);
			  packet->SetDataByte(15, reg4_value & 0x0FF);

			  packet->ComputeChecksum();

			  AddTXPacket( packet );
		 }

		/* -------------------------------------------------------------------------------------------------------
		* Name: send_vin_settings()
		* Description:	
		*	Sends a packet to write the settings for controlling the voltage input settings on the Hydra
		------------------------------------------------------------------------------------------------------- */
private: System::Void send_vin_settings()
		 {
			  SerialPacket^ packet = gcnew SerialPacket();

			  UInt32 reg_value = 0;

			  reg_value = ((UInt32)(this->vin_cutoff / VOUT_REG_SCALE) & 0x0FFFF);

			  reg_value |= ((this->cutoff1_override & 0x01) << 31);
			  reg_value |= ((this->cutoff2_override & 0x01) << 30);
			  reg_value |= ((this->cutoff3_override & 0x01) << 29);

			  // Construct a packet and send it
			  packet->Address = VIN_SETTINGS_REG_ADDRESS;
			  packet->HasData = true;
			  packet->IsBatch = false;
			  packet->SetDataByte(0, (reg_value >> 24) & 0x0FF);
			  packet->SetDataByte(1, (reg_value >> 16) & 0x0FF);
			  packet->SetDataByte(2, (reg_value >> 8) & 0x0FF);
			  packet->SetDataByte(3, reg_value & 0x0FF);

			  packet->ComputeChecksum();

			  AddTXPacket( packet );
		 }

		 /* -------------------------------------------------------------------------------------------------------
		* Name: send_coefficients()
		* Description:	
		*	Sends a packet to write the calibration coefficients for the Hydra
		------------------------------------------------------------------------------------------------------- */
private: System::Void send_coefficients()
		 {
			  SerialPacket^ packet = gcnew SerialPacket();

			  // Construct a packet and send it
			  packet->Address = 4;
			  packet->HasData = true;
			  packet->IsBatch = true;
			  packet->BatchLength = 3;

			  packet->SetDataByte(0, ((UInt32)this->cal_out1_c1 >> 24) & 0x0FF);
			  packet->SetDataByte(1, ((UInt32)this->cal_out1_c1 >> 16) & 0x0FF);
			  packet->SetDataByte(2, ((UInt32)this->cal_out1_c1 >> 8) & 0x0FF);
			  packet->SetDataByte(3, (UInt32)this->cal_out1_c1 & 0x0FF);

			  packet->SetDataByte(4, ((UInt32)this->cal_out1_c2 >> 24) & 0x0FF);
			  packet->SetDataByte(5, ((UInt32)this->cal_out1_c2 >> 16) & 0x0FF);
			  packet->SetDataByte(6, ((UInt32)this->cal_out1_c2 >> 8) & 0x0FF);
			  packet->SetDataByte(7, (UInt32)this->cal_out1_c2 & 0x0FF);

			  packet->SetDataByte(8, ((UInt32)this->cal_out1_c3 >> 24) & 0x0FF);
			  packet->SetDataByte(9, ((UInt32)this->cal_out1_c3 >> 16) & 0x0FF);
			  packet->SetDataByte(10, ((UInt32)this->cal_out1_c3 >> 8) & 0x0FF);
			  packet->SetDataByte(11, (UInt32)this->cal_out1_c3 & 0x0FF);

			  packet->ComputeChecksum();

			  AddTXPacket( packet );

			  SerialPacket^ packet2 = gcnew SerialPacket();

			  // Construct a packet and send it
			  packet2->Address = 7;
			  packet2->HasData = true;
			  packet2->IsBatch = true;
			  packet2->BatchLength = 3;

			  packet2->SetDataByte(0, ((UInt32)this->cal_out2_c1 >> 24) & 0x0FF);
			  packet2->SetDataByte(1, ((UInt32)this->cal_out2_c1 >> 16) & 0x0FF);
			  packet2->SetDataByte(2, ((UInt32)this->cal_out2_c1 >> 8) & 0x0FF);
			  packet2->SetDataByte(3, (UInt32)this->cal_out2_c1 & 0x0FF);

			  packet2->SetDataByte(4, ((UInt32)this->cal_out2_c2 >> 24) & 0x0FF);
			  packet2->SetDataByte(5, ((UInt32)this->cal_out2_c2 >> 16) & 0x0FF);
			  packet2->SetDataByte(6, ((UInt32)this->cal_out2_c2 >> 8) & 0x0FF);
			  packet2->SetDataByte(7, (UInt32)this->cal_out2_c2 & 0x0FF);

			  packet2->SetDataByte(8, ((UInt32)this->cal_out2_c3 >> 24) & 0x0FF);
			  packet2->SetDataByte(9, ((UInt32)this->cal_out2_c3 >> 16) & 0x0FF);
			  packet2->SetDataByte(10, ((UInt32)this->cal_out2_c3 >> 8) & 0x0FF);
			  packet2->SetDataByte(11, (UInt32)this->cal_out2_c3 & 0x0FF);

			  packet->ComputeChecksum();

			  AddTXPacket( packet2 );

			  SerialPacket^ packet3 = gcnew SerialPacket();

			  // Construct a packet and send it
			  packet3->Address = 10;
			  packet3->HasData = true;
			  packet3->IsBatch = true;
			  packet3->BatchLength = 3;

			  packet3->SetDataByte(0, ((UInt32)this->cal_out3_c1 >> 24) & 0x0FF);
			  packet3->SetDataByte(1, ((UInt32)this->cal_out3_c1 >> 16) & 0x0FF);
			  packet3->SetDataByte(2, ((UInt32)this->cal_out3_c1 >> 8) & 0x0FF);
			  packet3->SetDataByte(3, (UInt32)this->cal_out3_c1 & 0x0FF);

			  packet3->SetDataByte(4, ((UInt32)this->cal_out3_c2 >> 24) & 0x0FF);
			  packet3->SetDataByte(5, ((UInt32)this->cal_out3_c2 >> 16) & 0x0FF);
			  packet3->SetDataByte(6, ((UInt32)this->cal_out3_c2 >> 8) & 0x0FF);
			  packet3->SetDataByte(7, (UInt32)this->cal_out3_c2 & 0x0FF);

			  packet3->SetDataByte(8, ((UInt32)this->cal_out3_c3 >> 24) & 0x0FF);
			  packet3->SetDataByte(9, ((UInt32)this->cal_out3_c3 >> 16) & 0x0FF);
			  packet3->SetDataByte(10, ((UInt32)this->cal_out3_c3 >> 8) & 0x0FF);
			  packet3->SetDataByte(11, (UInt32)this->cal_out3_c3 & 0x0FF);

			  packet->ComputeChecksum();

			  AddTXPacket( packet3 );
		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: connectButton_Click(System::Object^  sender, System::EventArgs^  e)
		 *
		 * Description: 
		 *	Attempts to connect to the serial port specified by member variable serialComboBox
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void connectButton_Click(System::Object^  sender, System::EventArgs^  e) {

			 // Copy settings to serial port
			 serialConnector->Parity = System::IO::Ports::Parity::None;
			 serialConnector->StopBits = System::IO::Ports::StopBits::One;

			 try
			 {
				 serialConnector->PortName = dynamic_cast<String^>(this->serialComboBox->SelectedItem);
				 serialConnector->BaudRate = Convert::ToInt32(dynamic_cast<String^>(this->baudComboBox->SelectedItem));
			 }
			 catch( Exception^ /*e*/ )
			 {
			 }

			 // Check to see if "None" is selected.  If so, no serial ports were found.
			 if( System::String::Compare(serialConnector->PortName,gcnew System::String("None"), true) == 0 )
			 {
				 this->statusLabel->Text = gcnew System::String("No serial ports are available. Is the Hydra plugged in?");
				 this->statusLabel->ForeColor = System::Drawing::Color::Red;

				 return;
			 }
		
			 // Attemp to connect
			 try
			 {
				 serialConnector->Open();

				 serialConnector->TransmitRaw( gcnew System::String(":b\r\n:b\r\n") );

				 // Disable serial port selector and the connect button, enable the disconnect button
				 this->serialComboBox->Enabled = false;
				 this->baudComboBox->Enabled = false;
				 this->connectButton->Enabled = false;
				 this->disconnectButton->Enabled = true;

				 this->Text = gcnew System::String("Hydra - Connecting...");

				 this->statusLabel->Text = gcnew System::String("Connecting...");
				 this->statusLabel->ForeColor = System::Drawing::Color::Green;

				 this->serial_port_connected = 1;

				 this->cal_start_button->Enabled = true;

				 COM_timeout_timer->Start();
			 }
			 catch( Exception^ /*e*/ )
			 {
				 this->statusLabel->Text = gcnew System::String("Could not open serial port. Is something else using it?");
				 this->statusLabel->ForeColor = System::Drawing::Color::Red;

				 return;
			 }

		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: disconnectButton_Click(System::Object^  sender, System::EventArgs^  e)
		 *
		 * Description: 
		 *	Disconnects from the serial port.
		 * ----------------------------------------------------------------------------------------------------------- */
	private: System::Void disconnectButton_Click(System::Object^  sender, System::EventArgs^  e) {

				 try
				 {
					serialConnector->Close();
				 }
				 catch( Exception^ /* e*/ )
				 {

				 }

				 this->connectButton->Enabled = true;
				 this->disconnectButton->Enabled = false;
				 this->serialComboBox->Enabled = true;
				 this->baudComboBox->Enabled = true;

				 disable_voltage_controls();

				 change_vin_back_color(HYDRA_DISCONNECTED_COLOR);
				 change_out1_back_color(HYDRA_DISCONNECTED_COLOR);
				 change_out2_back_color(HYDRA_DISCONNECTED_COLOR);
				 change_out3_back_color(HYDRA_DISCONNECTED_COLOR);

				 this->Text = gcnew System::String("Hydra - Not Connected");
				 this->statusLabel->Text = gcnew System::String("Not Connected");
				 this->statusLabel->ForeColor = System::Drawing::Color::Red;

				 this->serial_port_connected = 0;
				 this->cal_start_button->Enabled = false;


				 this->have_config_data = 0;

				 // Set cutoff_active flag to zero so that when we reconnect, the interface will display the error message again (and disable voltage output controls)
				 this->vin_cutoff_active = 0;

				 COM_timeout_timer->Stop();
			 }
		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: disable_voltage_controls()
		 *
		 * Description: 
		 *	Disables all controls used to set up the power supply outputs
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void disable_voltage_controls()
		 {
			 // Change the enabled/disabled status of everything
			 disable_v1_controls();
			 disable_v2_controls();
			 disable_v3_controls();
		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: disable_v1_controls()
		 *
		 * Description: 
		 *	Disables controls for v1
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void disable_v1_controls()
		 {
			 if( this->InvokeRequired )
			 {
				 this->BeginInvoke( gcnew NoArgumentDelegate( this, &Form1::disable_v1_controls ),  nullptr);
			 }
			 else
			 {
				 this->output1_enabled_checkbox->Enabled = false;
				 this->output1_trackbar->Enabled = false;
				 this->max_current1_trackbar->Enabled = false;
				 this->output1_lowc_enabled_Checkbox->Enabled = false;
			 }

			 
		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: disable_v2_controls()
		 *
		 * Description: 
		 *	Disables controls for v2
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void disable_v2_controls()
		 {
			 if( this->InvokeRequired )
			 {
				 this->BeginInvoke( gcnew NoArgumentDelegate( this, &Form1::disable_v2_controls ),  nullptr);
			 }
			 else
			 {
				 this->output2_enabled_checkbox->Enabled = false;
				 this->output2_trackbar->Enabled = false;
				 this->max_current2_trackbar->Enabled = false;
				 this->output2_lowc_enabled_Checkbox->Enabled = false;
			 }
		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: disable_v3_controls()
		 *
		 * Description: 
		 *	Disables controls for v3
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void disable_v3_controls()
		 {
			 if( this->InvokeRequired )
			 {
				 this->BeginInvoke( gcnew NoArgumentDelegate( this, &Form1::disable_v3_controls ),  nullptr);
			 }
			 else
			 {
				 this->output3_enabled_checkbox->Enabled = false;
				 this->output3_trackbar->Enabled = false;
				 this->max_current3_trackbar->Enabled = false;
				 this->output3_lowc_enabled_Checkbox->Enabled = false;
			 }
		 }


		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: enable_voltage_controls()
		 *
		 * Description:
		 *   Enables all controls used to set up the power supply outputsDisconnects from the serial port.
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void enable_voltage_controls()
		 {
			 // Change the enabled/disabled status of everything
			 enable_v1_controls();
			 enable_v2_controls();
			 enable_v3_controls();
		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: enable_v1_controls()
		 *
		 * Description: 
		 *	Enables controls for v1
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void enable_v1_controls()
		 {
			 if( this->InvokeRequired )
			 {
				 this->BeginInvoke( gcnew NoArgumentDelegate( this, &Form1::enable_v1_controls ),  nullptr);
			 }
			 else
			 {
				 this->output1_enabled_checkbox->Enabled = true;
				 this->output1_trackbar->Enabled = true;
				 this->max_current1_trackbar->Enabled = true;
				 this->output1_lowc_enabled_Checkbox->Enabled = true;
			 }
		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: enable_v2_controls()
		 *
		 * Description: 
		 *	Enables controls for v2
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void enable_v2_controls()
		 {
			 if( this->InvokeRequired )
			 {
				 this->BeginInvoke( gcnew NoArgumentDelegate( this, &Form1::enable_v2_controls ),  nullptr);
			 }
			 else
			 {
				 this->output2_enabled_checkbox->Enabled = true;
				 this->output2_trackbar->Enabled = true;
				 this->max_current2_trackbar->Enabled = true;
				 this->output2_lowc_enabled_Checkbox->Enabled = true;
			 }
		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: enable_v3_controls()
		 *
		 * Description: 
		 *	Enables controls for v3
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void enable_v3_controls()
		 {
			 if( this->InvokeRequired )
			 {
				 this->BeginInvoke( gcnew NoArgumentDelegate( this, &Form1::enable_v3_controls ),  nullptr);
			 }
			 else
			 {
				 this->output3_enabled_checkbox->Enabled = true;
				 this->output3_trackbar->Enabled = true;
				 this->max_current3_trackbar->Enabled = true;
				 this->output3_lowc_enabled_Checkbox->Enabled = true;
			 }
		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: COM_timeout_timer_Tick(System::Object^  sender, System::EventArgs^  e)
		 *
		 * Description: 
		 *	Tick function for COM_timeout_timer.  This timer monitors the packets coming in from the Hydra to determine
		 *  whether the Hydra is sending packets.  If the timer ticks, the serial port is connected, and we haven't received
		 *  a packet since the last tick, there is a problem.  If we HAVE received a packet, we are good to go...
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void COM_timeout_timer_Tick(System::Object^  sender, System::EventArgs^  e) 
		 {
			 if( this->InvokeRequired )
			 {
			 	cli::array<System::Object^>^ args = gcnew cli::array<System::Object^>(2);
			 	args[0] = sender;
				args[1] = e;

			 	this->BeginInvoke( gcnew EventHandlerDelegate( this, &Form1::COM_timeout_timer_Tick ),  args);
			 }
			 else
			 {
			 
				 // If we think that the port is open, but it really isn't, the Hydra was probably unplugged.  Do something about it.
				 if( this->serial_port_connected && !this->serialConnector->IsOpen )
				 {
					 this->statusLabel->Text = gcnew System::String("The serial port closed. Is the Hydra still plugged in?");
					 this->statusLabel->ForeColor = System::Drawing::Color::Red;

					 this->serial_port_connected = 0;

					 this->connectButton->Enabled = true;
					 this->disconnectButton->Enabled = false;
					 this->serialComboBox->Enabled = true;
					 this->baudComboBox->Enabled = true;
					 this->cal_start_button->Enabled = false;

					 this->Text = gcnew System::String("Hydra - Not Connected");
				 }

				 // Now check to see if we are connected but not receiving packets.  If so, then the wrong COM port might be selected,
				 // or something might be wrong with the Hydra
				 if( this->serialConnector->IsOpen )
				 {
					 if( this->received_a_packet )
					 {
						 this->received_a_packet = 0;
						 this->statusLabel->Text = gcnew System::String("Connected.");
						 this->statusLabel->ForeColor = System::Drawing::Color::Green;

						 this->Text = gcnew System::String("Hydra - Connected");
					 }
					 else
					 {
						 // We haven't received a packet.  Either the Hydra is broken, the wrong COM port is selected, the COM hardware is wrong, or the Hydra somehow didn't get
						 // put into binary COM mode.
						 this->statusLabel->Text = gcnew System::String("We can't hear the Hydra. Check port and baud settings.");
						 this->statusLabel->ForeColor = System::Drawing::Color::Red;

						 // Try sending a packet to the Hydra to change it to binary COM mode.
						 serialConnector->TransmitRaw(gcnew System::String(":b\r\n:b\r\n"));
					 }
				 }
			 }
		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: vin_cutoff_label_Click(System::Object^  sender, System::EventArgs^  e) 
		 *
		 * Description: 
		 *	Display dialog to modify input voltage cutoff settings
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void vin_cutoff_label_Click(System::Object^  sender, System::EventArgs^  e) 
		 {

			 VinForm^ form = gcnew VinForm(this);

			// Only allow the dialog to be shown if the supply is connected
			if( this->serial_port_connected )
			{
				if( form->ShowDialog() == System::Windows::Forms::DialogResult::OK )
				{
					float new_cutoff;

					try
					{
						new_cutoff = (float)Convert::ToDouble( form->get_cutoff_text() );
					}
					catch( Exception^ /*e*/ )
					{
						MessageBox::Show("Error: the cutoff voltage must be a decimal number between 0.0 and 13.0", "Bad input", MessageBoxButtons::OK, MessageBoxIcon::Error);
						return;
					}

					if( new_cutoff > 13.0f || new_cutoff < 0.0f )
					{
						MessageBox::Show("Error: the cutoff voltage must be a decimal number between 0.0 and 13.0", "Bad input", MessageBoxButtons::OK, MessageBoxIcon::Error);
						return;
					}

					this->vin_cutoff = new_cutoff;

					if( form->get_override1_checked() )
					{
						this->cutoff1_override = 1;
					}
					else
					{
						this->cutoff1_override = 0;
					}

					if( form->get_override2_checked() )
					{
						this->cutoff2_override = 1;
					}
					else
					{
						this->cutoff2_override = 0;
					}

					if( form->get_override3_checked() )
					{
						this->cutoff3_override = 1;
					}
					else
					{
						this->cutoff3_override = 0;
					}

					this->vin_changed = 1;

					change_vin_back_color( CHANGE_PENDING_COLOR );

					send_vin_settings();
				}
			}
		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: v1_target_label_Click(System::Object^  sender, System::EventArgs^  e)
		 *
		 * Description: 
		 *	Display dialog to modify supply 1 settings
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void v1_target_label_Click(System::Object^  sender, System::EventArgs^  e) 
		 {

			 VoutForm^ form = gcnew VoutForm( this, 1 );

			 // Only modify settings if we are connected and undervoltage cutout isn't enabled
			 if( this->serial_port_connected && (!this->vin_cutoff_active || this->cutoff1_override) )
			 {
				 if( form->ShowDialog() == System::Windows::Forms::DialogResult::OK )
				 {
					 // Retrieve selected values and write them to the supply output.
					 // First get current and voltage and make sure that they make sense.
					 // If not, leave before we apply any new settings.
					 float new_voltage;
					 float new_max_current;

					 // Convert the string to a float
					 try
					 {
						 new_voltage = (float)Convert::ToDouble( form->get_voltage_text() );
					 }
					 catch( Exception^ /*e*/ )
					 {
						 MessageBox::Show("Error: The voltage must be a decimal number between 2.5 and 14.0", "Bad voltage input", MessageBoxButtons::OK, MessageBoxIcon::Information);

						 return;
					 }

					 // Check the voltage range
					 if( new_voltage < 2.5f || new_voltage > 14.0f )
					 {
						 MessageBox::Show("Error: The voltage must be a decimal number between 2.5 and 14.0", "Bad voltage input", MessageBoxButtons::OK, MessageBoxIcon::Information);

						 return;
					 }

					 // Convert the current string to float
					 try
					 {
						 new_max_current = (float)Convert::ToDouble( form->get_max_current_text() );
					 }
					 catch( Exception^ /*e*/ )
					 {
						 MessageBox::Show("Error: The max current must be a decimal number between 0.0 and 4.0", "Bad current input", MessageBoxButtons::OK, MessageBoxIcon::Information);

						 return;
					 }

					 // Check the current range
					 if( new_max_current < 0.0f || new_max_current > 4.0f )
					 {
						 MessageBox::Show("Error: The max current must be a decimal number between 0.0 and 4.0", "Bad current input", MessageBoxButtons::OK, MessageBoxIcon::Information);

						 return;
					 }

					 // Now get the rest of the values and apply them
					 this->v1_target = new_voltage;
					 this->iout1_max = new_max_current;

					 // Low-current mode enabled
					 if( form->get_lowc_enabled() )
					 {
						 this->low_current1_enabled = 1;
					 }
					 else
					 {
						 this->low_current1_enabled = 0;
					 }

					 // Output enabled
					 if( form->get_output_enabled() )
					 {
						 this->output1_enabled = 1;
					 }
					 else
					 {
						 this->output1_enabled = 0;
					 }

					 /*
					 if( form->get_cutoff_override() )
					 {
						 this->cutoff1_override = 1;
					 }
					 else
					 {
						 this->cutoff1_override = 0;
					 }
					 */

					 this->out1_changed = 1;

					 change_out1_back_color( CHANGE_PENDING_COLOR );

					 send_v1_settings();

				 }
			 }
		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: v2_target_label_Click(System::Object^  sender, System::EventArgs^  e)
		 *
		 * Description: 
		 *	Display dialog to modify supply 2 settings
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void v2_target_label_Click(System::Object^  sender, System::EventArgs^  e) 
		 {
			 VoutForm^ form = gcnew VoutForm( this, 2 );

			 // Only modify settings if we are connected and undervoltage cutout isn't enabled
			 if( this->serial_port_connected && (!this->vin_cutoff_active || this->cutoff2_override) )
			 {
				 if( form->ShowDialog() == System::Windows::Forms::DialogResult::OK )
				 {
					 // Retrieve selected values and write them to the supply output.
					 // First get current and voltage and make sure that they make sense.
					 // If not, leave before we apply any new settings.
					 float new_voltage;
					 float new_max_current;

					 // Convert the string to a float
					 try
					 {
						 new_voltage = (float)Convert::ToDouble( form->get_voltage_text() );
					 }
					 catch( Exception^ /*e*/ )
					 {
						 MessageBox::Show("Error: The voltage must be a decimal number between 2.5 and 14.0", "Bad voltage input", MessageBoxButtons::OK, MessageBoxIcon::Information);

						 return;
					 }

					 // Check the voltage range
					 if( new_voltage < 2.5f || new_voltage > 14.0f )
					 {
						 MessageBox::Show("Error: The voltage must be a decimal number between 2.5 and 14.0", "Bad voltage input", MessageBoxButtons::OK, MessageBoxIcon::Information);

						 return;
					 }

					 // Convert the current string to float
					 try
					 {
						 new_max_current = (float)Convert::ToDouble( form->get_max_current_text() );
					 }
					 catch( Exception^ /*e*/ )
					 {
						 MessageBox::Show("Error: The max current must be a decimal number between 0.0 and 4.0", "Bad current input", MessageBoxButtons::OK, MessageBoxIcon::Information);

						 return;
					 }

					 // Check the current range
					 if( new_max_current < 0.0f || new_max_current > 4.0f )
					 {
						 MessageBox::Show("Error: The max current must be a decimal number between 0.0 and 4.0", "Bad current input", MessageBoxButtons::OK, MessageBoxIcon::Information);

						 return;
					 }

					 // Now get the rest of the values and apply them
					 this->v2_target = new_voltage;
					 this->iout2_max = new_max_current;

					 // Low-current mode enabled
					 if( form->get_lowc_enabled() )
					 {
						 this->low_current2_enabled = 1;
					 }
					 else
					 {
						 this->low_current2_enabled = 0;
					 }

					 // Output enabled
					 if( form->get_output_enabled() )
					 {
						 this->output2_enabled = 1;
					 }
					 else
					 {
						 this->output2_enabled = 0;
					 }

					 /*
					 if( form->get_cutoff_override() )
					 {
						 this->cutoff2_override = 1;
					 }
					 else
					 {
						 this->cutoff2_override = 0;
					 }
					 */

					 this->out2_changed = 1;

					 change_out2_back_color( CHANGE_PENDING_COLOR );

					 send_v2_settings();
				 }
			 }
		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: v3_target_label_Click(System::Object^  sender, System::EventArgs^  e)
		 *
		 * Description: 
		 *	Display dialog to modify supply 3 settings
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void v3_target_label_Click(System::Object^  sender, System::EventArgs^  e) 
		 {
			 VoutForm^ form = gcnew VoutForm( this, 3 );

			 // Only modify settings if we are connected and undervoltage cutout isn't enabled
			 if( this->serial_port_connected && (!this->vin_cutoff_active || this->cutoff3_override) )
			 {
				 if( form->ShowDialog() == System::Windows::Forms::DialogResult::OK )
				 {
					 // Retrieve selected values and write them to the supply output.
					 // First get current and voltage and make sure that they make sense.
					 // If not, leave before we apply any new settings.
					 float new_voltage;
					 float new_max_current;

					 // Convert the string to a float
					 try
					 {
						 new_voltage = (float)Convert::ToDouble( form->get_voltage_text() );
					 }
					 catch( Exception^ /*e*/ )
					 {
						 MessageBox::Show("Error: The voltage must be a decimal number between 2.5 and 14.0", "Bad voltage input", MessageBoxButtons::OK, MessageBoxIcon::Information);

						 return;
					 }

					 // Check the voltage range
					 if( new_voltage < 2.5f || new_voltage > 14.0f )
					 {
						 MessageBox::Show("Error: The voltage must be a decimal number between 2.5 and 14.0", "Bad voltage input", MessageBoxButtons::OK, MessageBoxIcon::Information);

						 return;
					 }

					 // Convert the current string to float
					 try
					 {
						 new_max_current = (float)Convert::ToDouble( form->get_max_current_text() );
					 }
					 catch( Exception^ /*e*/ )
					 {
						 MessageBox::Show("Error: The max current must be a decimal number between 0.0 and 4.0", "Bad current input", MessageBoxButtons::OK, MessageBoxIcon::Information);

						 return;
					 }

					 // Check the current range
					 if( new_max_current < 0.0f || new_max_current > 4.0f )
					 {
						 MessageBox::Show("Error: The max current must be a decimal number between 0.0 and 4.0", "Bad current input", MessageBoxButtons::OK, MessageBoxIcon::Information);

						 return;
					 }

					 // Now get the rest of the values and apply them
					 this->v3_target = new_voltage;
					 this->iout3_max = new_max_current;

					 // Low-current mode enabled
					 if( form->get_lowc_enabled() )
					 {
						 this->low_current3_enabled = 1;
					 }
					 else
					 {
						 this->low_current3_enabled = 0;
					 }

					 // Output enabled
					 if( form->get_output_enabled() )
					 {
						 this->output3_enabled = 1;
					 }
					 else
					 {
						 this->output3_enabled = 0;
					 }

					 /*
					 if( form->get_cutoff_override() )
					 {
						 this->cutoff3_override = 1;
					 }
					 else
					 {
						 this->cutoff3_override = 0;
					 }
					 */

					 this->out3_changed = 1;

					 change_out3_back_color( CHANGE_PENDING_COLOR );

					 send_v3_settings();
				 }
			 }
		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: CAL_timer_Tick(System::Object^  sender, System::EventArgs^  e)
		 *
		 * Description: 
		 *	Timer governing calibration operation.  Specific execution depends on the current state.
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void CAL_timer_Tick(System::Object^  sender, System::EventArgs^  e) 
		 {
			 
			 double out1_C1;
			 double out1_C2;
			 double out1_C3;

			 double out2_C1;
			 double out2_C2;
			 double out2_C3;

			 double out3_C1;
			 double out3_C2;
			 double out3_C3;

			 double v1,v2,v3,x1,x2,x3;

			 if( this->InvokeRequired )
			 {
			 	cli::array<System::Object^>^ args = gcnew cli::array<System::Object^>(2);
			 	args[0] = sender;
				args[1] = e;

			 	this->BeginInvoke( gcnew EventHandlerDelegate( this, &Form1::CAL_timer_Tick ),  args);
			 }
			 else
			 {

				 this->CAL_timer->Enabled = false;

				 switch( this->CAL_state )
				 {
				 case CAL_STATE_BEGIN:

					 // Set default calibration coefficients
					 this->cal_out1_c1 = 2150400;
					 this->cal_out1_c2 = -800;
					 this->cal_out1_c3 = -162;

					 this->cal_out2_c1 = 2150400;
					 this->cal_out2_c2 = -800;
					 this->cal_out2_c3 = -162;

					 this->cal_out3_c1 = 2150400;
					 this->cal_out3_c2 = -800;
					 this->cal_out3_c3 = -162;

					 send_coefficients();

					 // Set the next state
					 this->CAL_state = CAL_STATE_SET_DEFAULTS;

					 // Enable the timer
					 this->CAL_timer->Enabled = true;

					 update_calibration_status(gcnew System::String("Setting default calibration.") );

					 break;

				 case CAL_STATE_SET_DEFAULTS:
					 this->v1_target = 3.0f;
					 this->v2_target = 3.0f;
					 this->v3_target = 3.0f;

					 this->cal_x1 = DEFAULT_C1/(this->v1_target*1000.0f + DEFAULT_C2) + DEFAULT_C3;

					 this->output1_enabled = 1;
					 this->output2_enabled = 1;
					 this->output3_enabled = 1;

					 change_out1_back_color( CHANGE_PENDING_COLOR );
					 change_out2_back_color( CHANGE_PENDING_COLOR );
					 change_out3_back_color( CHANGE_PENDING_COLOR );
					 change_vin_back_color( CHANGE_PENDING_COLOR );

					 send_all_settings();

					 this->CAL_timer->Interval = 2000;

					 // Set the next state
					 this->CAL_state = CAL_STATE_V1;

					 // Enable the timer
					 this->CAL_timer->Enabled = true;

					 update_calibration_status(gcnew System::String("Measuring at 3.0V") );

					 break;

				 case CAL_STATE_V1:
					 // In the V1 state, we log data from the supplies, set new outputs, set an appropriate delay, and exit.
					 this->cal_out1_v1 = this->output1_voltage;
					 this->cal_out2_v1 = this->output2_voltage;
					 this->cal_out3_v1 = this->output3_voltage;
				 
					 this->v1_target = 8.0f;
					 this->v2_target = 8.0f;
					 this->v3_target = 8.0f;

					 this->cal_x2 = DEFAULT_C1/(this->v1_target*1000.0f + DEFAULT_C2) + DEFAULT_C3;
				 
					 change_out1_back_color( CHANGE_PENDING_COLOR );
					 change_out2_back_color( CHANGE_PENDING_COLOR );
					 change_out3_back_color( CHANGE_PENDING_COLOR );
					 change_vin_back_color( CHANGE_PENDING_COLOR );

					 send_all_settings();

					 this->CAL_timer->Interval = 2000;

					 // Set the next state
					 this->CAL_state = CAL_STATE_V2;

					 // Enable the timer
					 this->CAL_timer->Enabled = true;

					 update_calibration_status(gcnew System::String("Measuring at 8.0V") );

					 break;

				 case CAL_STATE_V2:
					 // In state "V2", we log data from the previous voltage, set the new target, and giddyup to the next state...
					 this->cal_out1_v2 = this->output1_voltage;
					 this->cal_out2_v2 = this->output2_voltage;
					 this->cal_out3_v2 = this->output3_voltage;

					 this->v1_target = 12.0f;
					 this->v2_target = 12.0f;
					 this->v3_target = 12.0f;

					 this->cal_x3 = DEFAULT_C1/(this->v1_target*1000.0f + DEFAULT_C2) + DEFAULT_C3;
				 
					 change_out1_back_color( CHANGE_PENDING_COLOR );
					 change_out2_back_color( CHANGE_PENDING_COLOR );
					 change_out3_back_color( CHANGE_PENDING_COLOR );
					 change_vin_back_color( CHANGE_PENDING_COLOR );

					 send_all_settings();

					 this->CAL_timer->Interval = 2000;

					 // Set the next state
					 this->CAL_state = CAL_STATE_V3;

					 // Enable the timer
					 this->CAL_timer->Enabled = true;

					 update_calibration_status(gcnew System::String("Measuring at 12.0V") );

					 break;

				 case CAL_STATE_V3:
					 // In state "V3", we log the last bit of data and compute our calibration coefficients.
					 this->cal_out1_v3 = this->output1_voltage;
					 this->cal_out2_v3 = this->output2_voltage;
					 this->cal_out3_v3 = this->output3_voltage;
				 
					 // Make some local copies of data for convenience
					 x1 = this->cal_x1;
					 x2 = this->cal_x2;
					 x3 = this->cal_x3;

					 v1 = this->cal_out1_v1*1000;
					 v2 = this->cal_out1_v2*1000;
					 v3 = this->cal_out1_v3*1000;

					 // Compute our calibration coefficients
					 // C1 = -((v1 - v2)*(v1 - v3)*(v2 - v3)*(x1 - x2)*(x1 - x3)*(x2 - x3))/(v1*x2 - v2*x1 - v1*x3 + v3*x1 + v2*x3 - v3*x2)^2
					 // C2 = (v1*v2*x1 - v1*v2*x2 - v1*v3*x1 + v1*v3*x3 + v2*v3*x2 - v2*v3*x3)/(v1*x2 - v2*x1 - v1*x3 + v3*x1 + v2*x3 - v3*x2)
					 // C3 = (v1*x1*x2 - v1*x1*x3 - v2*x1*x2 + v2*x2*x3 + v3*x1*x3 - v3*x2*x3)/(v1*x2 - v2*x1 - v1*x3 + v3*x1 + v2*x3 - v3*x2)
					 out1_C1 = -((v1 - v2)*(v1 - v3)*(v2 - v3)*(x1 - x2)*(x1 - x3)*(x2 - x3))/((v1*x2 - v2*x1 - v1*x3 + v3*x1 + v2*x3 - v3*x2)*(v1*x2 - v2*x1 - v1*x3 + v3*x1 + v2*x3 - v3*x2));
					 out1_C2 = (v1*v2*x1 - v1*v2*x2 - v1*v3*x1 + v1*v3*x3 + v2*v3*x2 - v2*v3*x3)/(v1*x2 - v2*x1 - v1*x3 + v3*x1 + v2*x3 - v3*x2);
					 out1_C3 = (v1*x1*x2 - v1*x1*x3 - v2*x1*x2 + v2*x2*x3 + v3*x1*x3 - v3*x2*x3)/(v1*x2 - v2*x1 - v1*x3 + v3*x1 + v2*x3 - v3*x2);

					 v1 = this->cal_out2_v1*1000;
					 v2 = this->cal_out2_v2*1000;
					 v3 = this->cal_out2_v3*1000;

					 out2_C1 = -((v1 - v2)*(v1 - v3)*(v2 - v3)*(x1 - x2)*(x1 - x3)*(x2 - x3))/((v1*x2 - v2*x1 - v1*x3 + v3*x1 + v2*x3 - v3*x2)*(v1*x2 - v2*x1 - v1*x3 + v3*x1 + v2*x3 - v3*x2));
					 out2_C2 = (v1*v2*x1 - v1*v2*x2 - v1*v3*x1 + v1*v3*x3 + v2*v3*x2 - v2*v3*x3)/(v1*x2 - v2*x1 - v1*x3 + v3*x1 + v2*x3 - v3*x2);
					 out2_C3 = (v1*x1*x2 - v1*x1*x3 - v2*x1*x2 + v2*x2*x3 + v3*x1*x3 - v3*x2*x3)/(v1*x2 - v2*x1 - v1*x3 + v3*x1 + v2*x3 - v3*x2);

					 v1 = this->cal_out3_v1*1000;
					 v2 = this->cal_out3_v2*1000;
					 v3 = this->cal_out3_v3*1000;

					 out3_C1 = -((v1 - v2)*(v1 - v3)*(v2 - v3)*(x1 - x2)*(x1 - x3)*(x2 - x3))/((v1*x2 - v2*x1 - v1*x3 + v3*x1 + v2*x3 - v3*x2)*(v1*x2 - v2*x1 - v1*x3 + v3*x1 + v2*x3 - v3*x2));
					 out3_C2 = (v1*v2*x1 - v1*v2*x2 - v1*v3*x1 + v1*v3*x3 + v2*v3*x2 - v2*v3*x3)/(v1*x2 - v2*x1 - v1*x3 + v3*x1 + v2*x3 - v3*x2);
					 out3_C3 = (v1*x1*x2 - v1*x1*x3 - v2*x1*x2 + v2*x2*x3 + v3*x1*x3 - v3*x2*x3)/(v1*x2 - v2*x1 - v1*x3 + v3*x1 + v2*x3 - v3*x2);

					 // Copy new coefficients out to the class member variables
					 this->cal_out1_c1 = (Int32)out1_C1;
					 this->cal_out1_c2 = (Int32)out1_C2;
					 this->cal_out1_c3 = (Int32)out1_C3;

					 this->cal_out2_c1 = (Int32)out2_C1;
					 this->cal_out2_c2 = (Int32)out2_C2;
					 this->cal_out2_c3 = (Int32)out2_C3;

					 this->cal_out3_c1 = (Int32)out3_C1; 
					 this->cal_out3_c2 = (Int32)out3_C2;
					 this->cal_out3_c3 = (Int32)out3_C3;

					 display_coefficients();

					 // Don't transition to the next state until the user presses the "accept" button.
					 update_calibration_status(gcnew System::String("Coefficients computed.  Press 'Accept' to download to device.") );

					 this->accept_button->Enabled = true;

					 break;
				 
				 case CAL_STATE_VALIDATE1:
					 // Clear output error variables
					 this->out1_error = 0;
					 this->out2_error = 0;
					 this->out3_error = 0;

					 this->v1_target = 13.0f;
					 this->v2_target = 13.0f;
					 this->v3_target = 13.0f;
				 
					 change_out1_back_color( CHANGE_PENDING_COLOR );
					 change_out2_back_color( CHANGE_PENDING_COLOR );
					 change_out3_back_color( CHANGE_PENDING_COLOR );
					 change_vin_back_color( CHANGE_PENDING_COLOR );

					 send_all_settings();

					 this->CAL_timer->Interval = 2000;

					 // Set the next state
					 this->CAL_state = CAL_STATE_VALIDATE2;

					 // Enable the timer
					 this->CAL_timer->Enabled = true;

					 update_calibration_status(gcnew System::String("Measuring results at 13.0V") );

					 break;

				 case CAL_STATE_VALIDATE2:
					 // Compute squared error
					 this->out1_error += (this->v1_target - this->output1_voltage)*(this->v1_target - this->output1_voltage);
					 this->out2_error += (this->v2_target - this->output2_voltage)*(this->v2_target - this->output2_voltage);
					 this->out3_error += (this->v3_target - this->output3_voltage)*(this->v3_target - this->output3_voltage);

					 this->v1_target = 9.0f;
					 this->v2_target = 9.0f;
					 this->v3_target = 9.0f;
				 
					 change_out1_back_color( CHANGE_PENDING_COLOR );
					 change_out2_back_color( CHANGE_PENDING_COLOR );
					 change_out3_back_color( CHANGE_PENDING_COLOR );
					 change_vin_back_color( CHANGE_PENDING_COLOR );

					 send_all_settings();

					 this->CAL_timer->Interval = 2000;

					 // Set the next state
					 this->CAL_state = CAL_STATE_VALIDATE3;

					 this->CAL_timer->Enabled = true;

					 update_calibration_status(gcnew System::String("Measuring results at 9.0V") );

					 break;

				 case CAL_STATE_VALIDATE3:
					 // Compute squared error
					 this->out1_error += (this->v1_target - this->output1_voltage)*(this->v1_target - this->output1_voltage);
					 this->out2_error += (this->v2_target - this->output2_voltage)*(this->v2_target - this->output2_voltage);
					 this->out3_error += (this->v3_target - this->output3_voltage)*(this->v3_target - this->output3_voltage);

					 this->v1_target = 4.0f;
					 this->v2_target = 4.0f;
					 this->v3_target = 4.0f;
				 
					 change_out1_back_color( CHANGE_PENDING_COLOR );
					 change_out2_back_color( CHANGE_PENDING_COLOR );
					 change_out3_back_color( CHANGE_PENDING_COLOR );
					 change_vin_back_color( CHANGE_PENDING_COLOR );

					 send_all_settings();

					 // Set the next state
					 this->CAL_state = CAL_STATE_FINISH;

					 this->CAL_timer->Enabled = true;

					 update_calibration_status(gcnew System::String("Measuring results at 4.0V") );

					 break;

				 case CAL_STATE_FINISH:
					 // Compute squared error
					 this->out1_error += (this->v1_target - this->output1_voltage)*(this->v1_target - this->output1_voltage);
					 this->out2_error += (this->v2_target - this->output2_voltage)*(this->v2_target - this->output2_voltage);
					 this->out3_error += (this->v3_target - this->output3_voltage)*(this->v3_target - this->output3_voltage);

					 // Take square root of error terms
					 this->out1_error = (float)Math::Sqrt( this->out1_error );
					 this->out2_error = (float)Math::Sqrt( this->out2_error );
					 this->out3_error = (float)Math::Sqrt( this->out3_error );

					 this->v1_target = 3.0f;
					 this->v2_target = 3.0f;
					 this->v3_target = 3.0f;

					 this->output1_enabled = 0;
					 this->output2_enabled = 0;
					 this->output3_enabled = 0;

					 change_out1_back_color( CHANGE_PENDING_COLOR );
					 change_out2_back_color( CHANGE_PENDING_COLOR );
					 change_out3_back_color( CHANGE_PENDING_COLOR );
					 change_vin_back_color( CHANGE_PENDING_COLOR );

					 send_all_settings();

					 update_calibration_status(gcnew System::String("Finished") );

					 // Finished!
					 this->cal_start_button->Enabled = true;
					 this->cal_cancel_button->Enabled = false;
					 this->accept_button->Enabled = false;

					 display_error_terms();

					 break;


				 }
			 }

		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: display_error_terms( void )
		 *
		 * Description: 
		 *	Displays error terms in text boxes safely
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void display_error_terms( void )
		 {
			 if( this->InvokeRequired )
			 {
				 this->BeginInvoke( gcnew NoArgumentDelegate( this, &Form1::display_error_terms ),  nullptr);
			 }
			 else
			 {
				 this->output1_error_text->Text = gcnew String(this->out1_error.ToString("F4"));
				 this->output2_error_text->Text = gcnew String(this->out2_error.ToString("F4"));
				 this->output3_error_text->Text = gcnew String(this->out3_error.ToString("F4"));
			 }
		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: display_coefficients( void )
		 *
		 * Description: 
		 *	Displays coefficients safely
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void display_coefficients( void )
		 {
			 if( this->InvokeRequired )
			 {
				 this->BeginInvoke( gcnew NoArgumentDelegate( this, &Form1::display_coefficients ),  nullptr);
			 }
			 else
			 {
				 this->out1_c1_text->Text = gcnew String(this->cal_out1_c1.ToString());
				 this->out1_c2_text->Text = gcnew String(this->cal_out1_c2.ToString());
				 this->out1_c3_text->Text = gcnew String(this->cal_out1_c3.ToString());

				 this->out2_c1_text->Text = gcnew String(this->cal_out2_c1.ToString());
				 this->out2_c2_text->Text = gcnew String(this->cal_out2_c2.ToString());
				 this->out2_c3_text->Text = gcnew String(this->cal_out2_c3.ToString());

				 this->out3_c1_text->Text = gcnew String(this->cal_out3_c1.ToString());
				 this->out3_c2_text->Text = gcnew String(this->cal_out3_c2.ToString());
				 this->out3_c3_text->Text = gcnew String(this->cal_out3_c3.ToString());

			 }
		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: cal_start_button_Click(System::Object^  sender, System::EventArgs^  e)
		 *
		 * Description: 
		 *	Function call starts calibration
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void cal_start_button_Click(System::Object^  sender, System::EventArgs^  e) 
		 {
			 this->CAL_timer->Interval = 100;
			 this->CAL_state = CAL_STATE_BEGIN;
			 this->CAL_timer->Enabled = true;

			 update_calibration_status(gcnew System::String("Idle") );

			 this->cal_start_button->Enabled = false;
			 this->cal_cancel_button->Enabled = true;
			 this->accept_button->Enabled = false;
		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: cal_cancel_button_Click(System::Object^  sender, System::EventArgs^  e)
		 *
		 * Description: 
		 *	Function call cancels calibration
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void cal_cancel_button_Click(System::Object^  sender, System::EventArgs^  e) 
		 {
			 this->CAL_timer->Enabled = false;
			 this->cal_start_button->Enabled = true;
			 this->cal_cancel_button->Enabled = false;
			 this->accept_button->Enabled = false;

			 this->CAL_state = CAL_STATE_BEGIN;

			 update_calibration_status(gcnew System::String("Canceled") );
		 }

		 /* -----------------------------------------------------------------------------------------------------------
		 * Name: accept_button_Click(System::Object^  sender, System::EventArgs^  e)
		 *
		 * Description: 
		 *	Accepts calibration and writes it to the device
		 * ----------------------------------------------------------------------------------------------------------- */
private: System::Void accept_button_Click(System::Object^  sender, System::EventArgs^  e) 
		 {
			 // Write the new calibration coefficients to the device
			 send_coefficients();

			 update_calibration_status(gcnew System::String("Sending Calibration Coefficients.") );

			 this->CAL_timer->Interval = 2000;

			 // Set the next state
			 this->CAL_state = CAL_STATE_VALIDATE1;

			 this->accept_button->Enabled = false;

			 // Enable the timer
			 this->CAL_timer->Enabled = true;
		 }

		 /* *****************************************************************************
		 * Name: AddTXPacket
		 * Description: 
			Adds the current packet to the queue to be transmitted as soon as a
			possible
		 ** ****************************************************************************/
private: void AddTXPacket( SerialPacket^ packet )
		 {
			 this->packetsToSend[this->packetCount] = packet;
			 this->packetCount++;
			 
			 // Check to see if the packet timer is running.  If it is, then a packet has been
			 // transmitted and we are waiting for a response.
			 // If the timer is not running, then start it and transmit the first packet in the
			 // buffer
			 if( !this->packetResponseTimer->Enabled )
			 {
				 try
				 {
					 this->packetResponseTimer->Start();
					 serialConnector->TransmitPacket( packetsToSend[0] );
				 }
				 catch( Exception^ /*e*/ )
				 {
					 this->statusLabel->Text = gcnew System::String("COM error.  Is the Hydra plugged in?");
				 }				 
			 }
		 }

		/* *****************************************************************************
		 * Name: packetResponseTimer_Tick
		 * Description: 
			
			If this timer fires, then it means a packet was sent and the expected response
			was not received within a reasonable amount of time.  This code will either 
			send the packet again, or if it has already retried more than a few times,
			then it gives up and removes everything from the TX buffer.

		 ** ****************************************************************************/
private: System::Void packetResponseTimer_Tick(System::Object^  sender, System::EventArgs^  e) 
		 {
			 if( this->InvokeRequired )
			 {
			 	cli::array<System::Object^>^ args = gcnew cli::array<System::Object^>(2);
			 	args[0] = sender;
				args[1] = e;

			 	this->BeginInvoke( gcnew EventHandlerDelegate( this, &Form1::packetResponseTimer_Tick ),  args);
			 }
			 else
			 {

				 if( this->packetRetryCount >= 3 )
				 {
					 this->packetCount = 0;
					 this->packetResponseTimer->Stop();
					 this->packetRetryCount = 0;
				 }
				 else
				 {
					 // Make sure the serial connector is still open
					 if( serialConnector->IsOpen )
					 {
						 try
						 {
							 this->packetRetryCount++;
							 serialConnector->TransmitPacket( this->packetsToSend[0] );
						 }
						 catch( Exception^ /*e*/ )
						 {
							 this->statusLabel->Text = gcnew String("COM error.  Is the Hydra plugged in?");
						 }
					 }
					 else
					 {
						 this->packetCount = 0;
						 this->packetRetryCount = 0;
						 this->packetResponseTimer->Stop();
					 }
				 }
			 }
		 }

		 /* *****************************************************************************
		 * Name: showCalibrationToolsMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
		 * Description: 
			
			If clicked, resizes the main dialog and displays the calibration utility.

		 ** ****************************************************************************/
private: System::Void showCalibrationToolsMenuItem_Click(System::Object^  sender, System::EventArgs^  e) 
		 {
			 this->Height = 496;
			 this->calGroupBox->Visible = true;
			 this->showCalibrationToolsMenuItem->Enabled = false;
			 this->hideCalibrationToolsMenuItem->Enabled = true;
		 }

		 /* *****************************************************************************
		 * Name: hideCalibrationToolsMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
		 * Description: 
			
			If clicked, resizes the main dialog and hides the calibration utility.

		 ** ****************************************************************************/
private: System::Void hideCalibrationToolsMenuItem_Click(System::Object^  sender, System::EventArgs^  e) 
		 {
			 this->Height = 277;
			 this->calGroupBox->Visible = false;
			 this->showCalibrationToolsMenuItem->Enabled = true;
			 this->hideCalibrationToolsMenuItem->Enabled = false;
		 }

		 /* *****************************************************************************
		 * Name: aboutMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
		 * Description: 
			
			Displays a dialog containing version information

		 ** ****************************************************************************/
private: System::Void aboutMenuItem_Click(System::Object^  sender, System::EventArgs^  e) 
		 {
			 MessageBox::Show("Smart Power Supply v1.2\r\nCH Robotics LLC\r\n\r\nFor support, please contact us at support@chrobotics.com", "About", MessageBoxButtons::OK, MessageBoxIcon::Information);
		 }

		 /* *****************************************************************************
		 * Name: exitMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
		 * Description: 
			
			Closes the program

		 ** ****************************************************************************/
private: System::Void exitMenuItem_Click(System::Object^  sender, System::EventArgs^  e) 
		 {
			 try
			 {
				this->Close();
			 }
			 catch( Exception^ /*e*/ )
			 {

			 }
		 }

		 /* *****************************************************************************
		 * Name: cal_help_label_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e)
		 * Description: 
			
			Displays a help dialog to describe the calibration menu

		 ** ****************************************************************************/
private: System::Void cal_help_label_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e) 
		 {
			 MessageBox::Show("The calibration tool is used to improve the accuracy of the Hydra's outputs by cycling the Hydra through a range of voltages and measuring the outputs.  If running calibration, make sure that nothing is connected to the Hydra's outputs.\r\n\r\nCalibration is performed at the factory, so unless instructed otherwise by CH Robotics, there should be no reason to run the calibration cycle again.", "Calibration", MessageBoxButtons::OK, MessageBoxIcon::Information);
		 }

		 /* *****************************************************************************
		 * Name: serialPortSettingsToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
		 * Description: 
			
			Display dialog box with help topics for the serial port.

		 ** ****************************************************************************/
private: System::Void serialPortSettingsToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
			 MessageBox::Show("The standard baud-rate for the Hydra is 9600 bps.  This setting should not need to be changed, except for some early Hydra models that shipped with a 14400 baud rate.\r\n\r\nThe correct COM port is identified in the Windows Device Manager as a 'Silicon Labs CP210x USB to UART Bridge (COMxx)'.  This is the COM port you should select when connecting to the Hydra.\r\n\r\nIf the Device Manager does not show the Hydra, please ensure that the Hydra is connected and powered, and that the correct drivers are installed.  See http://www.chrobotics.com/hydra \r\n\r\nIf you can connect to the serial port, but you cannot interact with the Hydra, make sure the correct port and baud rate settings are selected.  If problems persist, contact us at support@chrobotics.com", "Serial Port Help", MessageBoxButtons::OK, MessageBoxIcon::Information);
		 }
};
}

