#include "stdafx.h"

namespace SmartPowerSupply {
	
	// Default constructor
	VoutForm::VoutForm() 
	{
		InitializeComponent();
	}

	// Custom constructor
	VoutForm::VoutForm(System::Windows::Forms::Form^ ParentForm, int output)
	{
		InitializeComponent();

		// Now do custom initialization
		switch( output )
		{
		case 1:
			this->Text = gcnew String("Output 1 Settings");

			// Voltage
			this->voltage_text->Text = gcnew String((safe_cast<Form1^>(ParentForm))->v1_target.ToString("F2"));

			// Max current
			this->max_current_text->Text = gcnew String((safe_cast<Form1^>(ParentForm))->iout1_max.ToString("F2"));

			// Output enabled check box
			if( (safe_cast<Form1^>(ParentForm))->output1_enabled )
			{
				this->enable_output_checkbox->Checked = true;
			}
			else
			{
				this->enable_output_checkbox->Checked = false;
			}

			// Low current mode enabled check box
			if( (safe_cast<Form1^>(ParentForm))->low_current1_enabled )
			{
				this->enable_lowc_mode_checkbox->Checked = true;
			}
			else
			{
				this->enable_lowc_mode_checkbox->Checked = false;
			}

			// Low-voltage Cutoff Override
			if( (safe_cast<Form1^>(ParentForm))->cutoff1_override )
			{
				this->cutoff_override_checkbox->Checked = true;
			}
			else
			{
				this->cutoff_override_checkbox->Checked = false;
			}

			break;

		case 2:

			this->Text = gcnew String("Output 2 Settings");

			// Voltage
			this->voltage_text->Text = gcnew String((safe_cast<Form1^>(ParentForm))->v2_target.ToString("F2"));

			// Max current
			this->max_current_text->Text = gcnew String((safe_cast<Form1^>(ParentForm))->iout2_max.ToString("F2"));

			// Output enabled check box
			if( (safe_cast<Form1^>(ParentForm))->output2_enabled )
			{
				this->enable_output_checkbox->Checked = true;
			}
			else
			{
				this->enable_output_checkbox->Checked = false;
			}

			// Low current mode enabled check box
			if( (safe_cast<Form1^>(ParentForm))->low_current2_enabled )
			{
				this->enable_lowc_mode_checkbox->Checked = true;
			}
			else
			{
				this->enable_lowc_mode_checkbox->Checked = false;
			}

			// Low-voltage Cutoff Override
			if( (safe_cast<Form1^>(ParentForm))->cutoff2_override )
			{
				this->cutoff_override_checkbox->Checked = true;
			}
			else
			{
				this->cutoff_override_checkbox->Checked = false;
			}

			break;

		case 3:

			this->Text = gcnew String("Output 3 Settings");

			// Voltage
			this->voltage_text->Text = gcnew String((safe_cast<Form1^>(ParentForm))->v3_target.ToString("F2"));

			// Max current
			this->max_current_text->Text = gcnew String((safe_cast<Form1^>(ParentForm))->iout3_max.ToString("F2"));

			// Output enabled check box
			if( (safe_cast<Form1^>(ParentForm))->output3_enabled )
			{
				this->enable_output_checkbox->Checked = true;
			}
			else
			{
				this->enable_output_checkbox->Checked = false;
			}

			// Low current mode enabled check box
			if( (safe_cast<Form1^>(ParentForm))->low_current3_enabled )
			{
				this->enable_lowc_mode_checkbox->Checked = true;
			}
			else
			{
				this->enable_lowc_mode_checkbox->Checked = false;
			}

			// Low-voltage Cutoff Override
			if( (safe_cast<Form1^>(ParentForm))->cutoff3_override )
			{
				this->cutoff_override_checkbox->Checked = true;
			}
			else
			{
				this->cutoff_override_checkbox->Checked = false;
			}

			break;
		}


	}

	System::String^ VoutForm::get_voltage_text( void)
	{
		return gcnew String(this->voltage_text->Text);
	}

	System::String^ VoutForm::get_max_current_text( void )
	{
		return gcnew String(this->max_current_text->Text);
	}

	bool VoutForm::get_cutoff_override( void )
	{
		return this->cutoff_override_checkbox->Checked;
	}

	bool VoutForm::get_lowc_enabled( void )
	{
		return this->enable_lowc_mode_checkbox->Checked;
	}

	bool VoutForm::get_output_enabled( void )
	{
		return this->enable_output_checkbox->Checked;
	}

}
