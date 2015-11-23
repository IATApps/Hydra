#pragma once

namespace SmartPowerSupply {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for VoutForm
	/// </summary>
	public ref class VoutForm : public System::Windows::Forms::Form
	{
	public:
		VoutForm(void);
		VoutForm(System::Windows::Forms::Form^, int output);

		System::String^ get_voltage_text( void);
		System::String^ get_max_current_text( void );
		bool get_cutoff_override( void );
		bool get_lowc_enabled( void );
		bool get_output_enabled( void );
		

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~VoutForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::CheckBox^  enable_output_checkbox;
	private: System::Windows::Forms::TextBox^  voltage_text;
	private: System::Windows::Forms::TextBox^  max_current_text;


	private: System::Windows::Forms::CheckBox^  enable_lowc_mode_checkbox;
	private: System::Windows::Forms::CheckBox^  cutoff_override_checkbox;

	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::Label^  label4;
	private: System::Windows::Forms::LinkLabel^  output_enable_help_label;
	private: System::Windows::Forms::LinkLabel^  low_current_help_label;
	private: System::Windows::Forms::LinkLabel^  voltage_cutoff_help_label;



	private: System::Windows::Forms::LinkLabel^  voltage_help_label;
	private: System::Windows::Forms::LinkLabel^  current_help_label;


	private: System::Windows::Forms::Button^  OK_button;
	private: System::Windows::Forms::Button^  Cancel_button;
	protected: 

	protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->enable_output_checkbox = (gcnew System::Windows::Forms::CheckBox());
			this->voltage_text = (gcnew System::Windows::Forms::TextBox());
			this->max_current_text = (gcnew System::Windows::Forms::TextBox());
			this->enable_lowc_mode_checkbox = (gcnew System::Windows::Forms::CheckBox());
			this->cutoff_override_checkbox = (gcnew System::Windows::Forms::CheckBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->output_enable_help_label = (gcnew System::Windows::Forms::LinkLabel());
			this->low_current_help_label = (gcnew System::Windows::Forms::LinkLabel());
			this->voltage_cutoff_help_label = (gcnew System::Windows::Forms::LinkLabel());
			this->voltage_help_label = (gcnew System::Windows::Forms::LinkLabel());
			this->current_help_label = (gcnew System::Windows::Forms::LinkLabel());
			this->OK_button = (gcnew System::Windows::Forms::Button());
			this->Cancel_button = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// enable_output_checkbox
			// 
			this->enable_output_checkbox->AutoSize = true;
			this->enable_output_checkbox->Location = System::Drawing::Point(83, 61);
			this->enable_output_checkbox->Name = L"enable_output_checkbox";
			this->enable_output_checkbox->Size = System::Drawing::Size(94, 17);
			this->enable_output_checkbox->TabIndex = 3;
			this->enable_output_checkbox->Text = L"Enable Output";
			this->enable_output_checkbox->UseVisualStyleBackColor = true;
			// 
			// voltage_text
			// 
			this->voltage_text->Location = System::Drawing::Point(83, 9);
			this->voltage_text->MaxLength = 6;
			this->voltage_text->Name = L"voltage_text";
			this->voltage_text->Size = System::Drawing::Size(52, 20);
			this->voltage_text->TabIndex = 1;
			// 
			// max_current_text
			// 
			this->max_current_text->Location = System::Drawing::Point(83, 35);
			this->max_current_text->MaxLength = 6;
			this->max_current_text->Name = L"max_current_text";
			this->max_current_text->Size = System::Drawing::Size(52, 20);
			this->max_current_text->TabIndex = 2;
			// 
			// enable_lowc_mode_checkbox
			// 
			this->enable_lowc_mode_checkbox->AutoSize = true;
			this->enable_lowc_mode_checkbox->Location = System::Drawing::Point(83, 84);
			this->enable_lowc_mode_checkbox->Name = L"enable_lowc_mode_checkbox";
			this->enable_lowc_mode_checkbox->Size = System::Drawing::Size(149, 17);
			this->enable_lowc_mode_checkbox->TabIndex = 4;
			this->enable_lowc_mode_checkbox->Text = L"Enable Low Current Mode";
			this->enable_lowc_mode_checkbox->UseVisualStyleBackColor = true;
			// 
			// cutoff_override_checkbox
			// 
			this->cutoff_override_checkbox->AutoSize = true;
			this->cutoff_override_checkbox->Location = System::Drawing::Point(83, 107);
			this->cutoff_override_checkbox->Name = L"cutoff_override_checkbox";
			this->cutoff_override_checkbox->Size = System::Drawing::Size(172, 17);
			this->cutoff_override_checkbox->TabIndex = 5;
			this->cutoff_override_checkbox->Text = L"Enable Voltage Cutoff Override";
			this->cutoff_override_checkbox->UseVisualStyleBackColor = true;
			this->cutoff_override_checkbox->Visible = false;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(34, 12);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(43, 13);
			this->label1->TabIndex = 5;
			this->label1->Text = L"Voltage";
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(12, 38);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(65, 13);
			this->label2->TabIndex = 6;
			this->label2->Text = L"Current Limit";
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(138, 12);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(14, 13);
			this->label3->TabIndex = 7;
			this->label3->Text = L"V";
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Location = System::Drawing::Point(138, 38);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(14, 13);
			this->label4->TabIndex = 8;
			this->label4->Text = L"A";
			// 
			// output_enable_help_label
			// 
			this->output_enable_help_label->AutoSize = true;
			this->output_enable_help_label->Location = System::Drawing::Point(177, 61);
			this->output_enable_help_label->Name = L"output_enable_help_label";
			this->output_enable_help_label->Size = System::Drawing::Size(13, 13);
			this->output_enable_help_label->TabIndex = 9;
			this->output_enable_help_label->TabStop = true;
			this->output_enable_help_label->Text = L"\?";
			this->output_enable_help_label->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &VoutForm::output_enable_help_label_LinkClicked);
			// 
			// low_current_help_label
			// 
			this->low_current_help_label->AutoSize = true;
			this->low_current_help_label->Location = System::Drawing::Point(230, 85);
			this->low_current_help_label->Name = L"low_current_help_label";
			this->low_current_help_label->Size = System::Drawing::Size(13, 13);
			this->low_current_help_label->TabIndex = 10;
			this->low_current_help_label->TabStop = true;
			this->low_current_help_label->Text = L"\?";
			this->low_current_help_label->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &VoutForm::low_current_help_label_LinkClicked);
			// 
			// voltage_cutoff_help_label
			// 
			this->voltage_cutoff_help_label->AutoSize = true;
			this->voltage_cutoff_help_label->Location = System::Drawing::Point(255, 108);
			this->voltage_cutoff_help_label->Name = L"voltage_cutoff_help_label";
			this->voltage_cutoff_help_label->Size = System::Drawing::Size(13, 13);
			this->voltage_cutoff_help_label->TabIndex = 11;
			this->voltage_cutoff_help_label->TabStop = true;
			this->voltage_cutoff_help_label->Text = L"\?";
			this->voltage_cutoff_help_label->Visible = false;
			this->voltage_cutoff_help_label->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &VoutForm::voltage_cutoff_help_label_LinkClicked);
			// 
			// voltage_help_label
			// 
			this->voltage_help_label->AutoSize = true;
			this->voltage_help_label->Location = System::Drawing::Point(156, 12);
			this->voltage_help_label->Name = L"voltage_help_label";
			this->voltage_help_label->Size = System::Drawing::Size(13, 13);
			this->voltage_help_label->TabIndex = 12;
			this->voltage_help_label->TabStop = true;
			this->voltage_help_label->Text = L"\?";
			this->voltage_help_label->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &VoutForm::voltage_help_label_LinkClicked);
			// 
			// current_help_label
			// 
			this->current_help_label->AutoSize = true;
			this->current_help_label->Location = System::Drawing::Point(156, 38);
			this->current_help_label->Name = L"current_help_label";
			this->current_help_label->Size = System::Drawing::Size(13, 13);
			this->current_help_label->TabIndex = 13;
			this->current_help_label->TabStop = true;
			this->current_help_label->Text = L"\?";
			this->current_help_label->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &VoutForm::current_help_label_LinkClicked);
			// 
			// OK_button
			// 
			this->OK_button->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->OK_button->Location = System::Drawing::Point(12, 130);
			this->OK_button->Name = L"OK_button";
			this->OK_button->Size = System::Drawing::Size(75, 23);
			this->OK_button->TabIndex = 6;
			this->OK_button->Text = L"OK";
			this->OK_button->UseVisualStyleBackColor = true;
			// 
			// Cancel_button
			// 
			this->Cancel_button->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->Cancel_button->Location = System::Drawing::Point(102, 130);
			this->Cancel_button->Name = L"Cancel_button";
			this->Cancel_button->Size = System::Drawing::Size(75, 23);
			this->Cancel_button->TabIndex = 7;
			this->Cancel_button->Text = L"Cancel";
			this->Cancel_button->UseVisualStyleBackColor = true;
			// 
			// VoutForm
			// 
			this->AcceptButton = this->OK_button;
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->CancelButton = this->Cancel_button;
			this->ClientSize = System::Drawing::Size(335, 164);
			this->ControlBox = false;
			this->Controls->Add(this->Cancel_button);
			this->Controls->Add(this->OK_button);
			this->Controls->Add(this->current_help_label);
			this->Controls->Add(this->voltage_help_label);
			this->Controls->Add(this->voltage_cutoff_help_label);
			this->Controls->Add(this->low_current_help_label);
			this->Controls->Add(this->output_enable_help_label);
			this->Controls->Add(this->label4);
			this->Controls->Add(this->label3);
			this->Controls->Add(this->label2);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->cutoff_override_checkbox);
			this->Controls->Add(this->enable_lowc_mode_checkbox);
			this->Controls->Add(this->max_current_text);
			this->Controls->Add(this->voltage_text);
			this->Controls->Add(this->enable_output_checkbox);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"VoutForm";
			this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Output Settings";
			this->TopMost = true;
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

private: System::Void voltage_help_label_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e) 
		 {
			 MessageBox::Show("The target voltage setting controls the output voltage of the power supply.  The actual output voltage can be slightly different from the target voltage depending on the load connected to the output.", "Voltage Setting", MessageBoxButtons::OK, MessageBoxIcon::Information);
		 }

private: System::Void current_help_label_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e) 
		 {
			 MessageBox::Show("The maximum current sets a software limit on the output current for the supply.  If the output current goes over this setting, the Hydra will automatically adjust the output voltage to reduce the output current.\r\n\r\nNOTE: This is a slow-responding current control loop, so it should not be used to prevent damage to electronics (it does accel at controlling the input current while charging batteries, for example).", "Max Current Setting", MessageBoxButtons::OK, MessageBoxIcon::Information);
		 }

private: System::Void output_enable_help_label_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e) 
		 {
			 MessageBox::Show("This setting turns the supply output on or off.", "Output Enable Setting", MessageBoxButtons::OK, MessageBoxIcon::Information);
		 }

private: System::Void low_current_help_label_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e) 
		 {
			 MessageBox::Show("Low-current mode allows the Hydra to perform more efficiently when the output current is very low.  The efficiency is improved at the expense of limiting the total available output current.", "Low Current Mode", MessageBoxButtons::OK, MessageBoxIcon::Information);
		 }

private: System::Void voltage_cutoff_help_label_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e) 
		 {
			 MessageBox::Show("The input voltage cutoff feature disables the Hydra outputs if the input voltage drops too low.  This helps protect batteries from being over-discharged.\r\n\r\nThe cutoff override setting allows the output to remain enabled even if the input voltage drops below the threshold.", "Input Voltage Cutoff Override", MessageBoxButtons::OK, MessageBoxIcon::Information);
		 }

};
}
