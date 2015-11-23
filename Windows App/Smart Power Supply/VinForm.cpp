#include "stdafx.h"

namespace SmartPowerSupply {

	VinForm::VinForm(void)
	{
		InitializeComponent();
	}

	// Custom constructor passing reference to parent
	VinForm::VinForm( System::Windows::Forms::Form^ ParentForm )
	{
		InitializeComponent();

		float active_setting = (safe_cast<Form1^>(ParentForm))->vin_cutoff;
		int cutoff1_override = (safe_cast<Form1^>(ParentForm))->cutoff1_override;
		int cutoff2_override = (safe_cast<Form1^>(ParentForm))->cutoff2_override;
		int cutoff3_override = (safe_cast<Form1^>(ParentForm))->cutoff3_override;

		this->cutoff_text->Text = gcnew String(active_setting.ToString("F2"));

		// Input cutoff override settings
		if( cutoff1_override )
		{
			this->override1_checkbox->Checked = true;
		}
		else
		{
			this->override1_checkbox->Checked = false;
		}

		if( cutoff2_override )
		{
			this->override2_checkbox->Checked = true;
		}
		else
		{
			this->override2_checkbox->Checked = false;
		}

		if( cutoff3_override )
		{
			this->override3_checkbox->Checked = true;
		}
		else
		{
			this->override3_checkbox->Checked = false;
		}


	}

	System::String^ VinForm::get_cutoff_text( void )
	{
		return cutoff_text->Text;
	}

	bool VinForm::get_override1_checked( void )
	{
		return this->override1_checkbox->Checked;
	}

	bool VinForm::get_override2_checked( void )
	{
		return this->override2_checkbox->Checked;
	}

	bool VinForm::get_override3_checked( void )
	{
		return this->override3_checkbox->Checked;
	}

}
