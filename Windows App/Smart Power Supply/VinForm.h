#pragma once

namespace SmartPowerSupply {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for VinForm
	/// </summary>
	public ref class VinForm : public System::Windows::Forms::Form
	{
	public:
		VinForm(void);
		VinForm( System::Windows::Forms::Form^ );

		System::String^ get_cutoff_text( void );
		bool get_override1_checked( void );
		bool get_override2_checked( void );
		bool get_override3_checked( void );

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~VinForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  OK_button;
	protected: 
	private: System::Windows::Forms::Button^  Cancel_button;
	private: System::Windows::Forms::TextBox^  cutoff_text;

	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::LinkLabel^  help_link_label;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::CheckBox^  override1_checkbox;
	private: System::Windows::Forms::CheckBox^  override2_checkbox;
	private: System::Windows::Forms::CheckBox^  override3_checkbox;

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
			this->OK_button = (gcnew System::Windows::Forms::Button());
			this->Cancel_button = (gcnew System::Windows::Forms::Button());
			this->cutoff_text = (gcnew System::Windows::Forms::TextBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->help_link_label = (gcnew System::Windows::Forms::LinkLabel());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->override1_checkbox = (gcnew System::Windows::Forms::CheckBox());
			this->override2_checkbox = (gcnew System::Windows::Forms::CheckBox());
			this->override3_checkbox = (gcnew System::Windows::Forms::CheckBox());
			this->SuspendLayout();
			// 
			// OK_button
			// 
			this->OK_button->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->OK_button->Location = System::Drawing::Point(69, 143);
			this->OK_button->Name = L"OK_button";
			this->OK_button->Size = System::Drawing::Size(75, 23);
			this->OK_button->TabIndex = 0;
			this->OK_button->Text = L"OK";
			this->OK_button->UseVisualStyleBackColor = true;
			this->OK_button->MouseClick += gcnew System::Windows::Forms::MouseEventHandler(this, &VinForm::s);
			// 
			// Cancel_button
			// 
			this->Cancel_button->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->Cancel_button->Location = System::Drawing::Point(150, 143);
			this->Cancel_button->Name = L"Cancel_button";
			this->Cancel_button->Size = System::Drawing::Size(75, 23);
			this->Cancel_button->TabIndex = 1;
			this->Cancel_button->Text = L"Cancel";
			this->Cancel_button->UseVisualStyleBackColor = true;
			// 
			// cutoff_text
			// 
			this->cutoff_text->AcceptsTab = true;
			this->cutoff_text->Location = System::Drawing::Point(141, 20);
			this->cutoff_text->MaxLength = 5;
			this->cutoff_text->Name = L"cutoff_text";
			this->cutoff_text->Size = System::Drawing::Size(51, 20);
			this->cutoff_text->TabIndex = 0;
			this->cutoff_text->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
			this->cutoff_text->WordWrap = false;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(17, 23);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(104, 13);
			this->label1->TabIndex = 3;
			this->label1->Text = L"Input Voltage Cutoff:";
			// 
			// help_link_label
			// 
			this->help_link_label->AutoSize = true;
			this->help_link_label->Location = System::Drawing::Point(218, 23);
			this->help_link_label->Name = L"help_link_label";
			this->help_link_label->Size = System::Drawing::Size(13, 13);
			this->help_link_label->TabIndex = 4;
			this->help_link_label->TabStop = true;
			this->help_link_label->Text = L"\?";
			this->help_link_label->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &VinForm::help_link_label_LinkClicked);
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(195, 23);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(14, 13);
			this->label2->TabIndex = 5;
			this->label2->Text = L"V";
			// 
			// override1_checkbox
			// 
			this->override1_checkbox->AutoSize = true;
			this->override1_checkbox->Location = System::Drawing::Point(15, 56);
			this->override1_checkbox->Name = L"override1_checkbox";
			this->override1_checkbox->RightToLeft = System::Windows::Forms::RightToLeft::Yes;
			this->override1_checkbox->Size = System::Drawing::Size(141, 17);
			this->override1_checkbox->TabIndex = 6;
			this->override1_checkbox->Text = L"Output 1 Cutoff Override";
			this->override1_checkbox->UseVisualStyleBackColor = true;
			// 
			// override2_checkbox
			// 
			this->override2_checkbox->AutoSize = true;
			this->override2_checkbox->Location = System::Drawing::Point(15, 79);
			this->override2_checkbox->Name = L"override2_checkbox";
			this->override2_checkbox->RightToLeft = System::Windows::Forms::RightToLeft::Yes;
			this->override2_checkbox->Size = System::Drawing::Size(141, 17);
			this->override2_checkbox->TabIndex = 7;
			this->override2_checkbox->Text = L"Output 2 Cutoff Override";
			this->override2_checkbox->UseVisualStyleBackColor = true;
			// 
			// override3_checkbox
			// 
			this->override3_checkbox->AutoSize = true;
			this->override3_checkbox->Location = System::Drawing::Point(15, 102);
			this->override3_checkbox->Name = L"override3_checkbox";
			this->override3_checkbox->RightToLeft = System::Windows::Forms::RightToLeft::Yes;
			this->override3_checkbox->Size = System::Drawing::Size(141, 17);
			this->override3_checkbox->TabIndex = 8;
			this->override3_checkbox->Text = L"Output 3 Cutoff Override";
			this->override3_checkbox->UseVisualStyleBackColor = true;
			// 
			// VinForm
			// 
			this->AcceptButton = this->OK_button;
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->CancelButton = this->Cancel_button;
			this->ClientSize = System::Drawing::Size(249, 178);
			this->ControlBox = false;
			this->Controls->Add(this->override3_checkbox);
			this->Controls->Add(this->override2_checkbox);
			this->Controls->Add(this->override1_checkbox);
			this->Controls->Add(this->label2);
			this->Controls->Add(this->help_link_label);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->cutoff_text);
			this->Controls->Add(this->Cancel_button);
			this->Controls->Add(this->OK_button);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"VinForm";
			this->ShowInTaskbar = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Input Voltage Cutoff Settings";
			this->TopMost = true;
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void help_link_label_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e) 
			 {
				 MessageBox::Show("The input voltage cutoff setting is used to help prevent the Hydra from over-discharging batteries.\r\n\r\nIf the input voltage drops below the cutoff threshold (even if only for a moment) all outputs all disabled except those with the cutoff override enabled.\r\n\r\nThe outputs will not be re-enabled until the voltage is above the threshold and the Hydra has been restarted.\r\n\r\nThe cutoff override allows only some outputs to be disabled on a low voltage condition, so that (for example) low current controls can continue running while higher current outputs are disabled.", "Input voltage cutoff", MessageBoxButtons::OK, MessageBoxIcon::Information);
			 }
private: System::Void OK_button_Click(System::Object^  sender, System::EventArgs^  e) {

		 }
private: System::Void s(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
		 }
};
}
