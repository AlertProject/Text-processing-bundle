using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace ExtractProjectSpecificConcepts
{
	public partial class CandidatesRecent : Form
	{
		public CandidatesRecent()
		{
			InitializeComponent();
			ComboBoxMethod.SelectedIndex = 0;
			DateTimeStart.Value = DateTime.Now - new TimeSpan(30, 0, 0, 0);
			DateTimeEnd.Value = DateTime.Now;
		}

		public string GetConceptExtractionMethod()
		{
			switch (ComboBoxMethod.SelectedIndex)
			{
				case 0: return "SVM";
				case 1: return "globalConceptSpV";
				case 2: return "localConceptSpV";
				default:
					return "SVM";
			}
		}

		public int GetConceptCount()
		{
			int count = 100;
			int.TryParse(TextBoxCandidateCount.Text, out count);
			return count;
		}

		private void ButtonCancel_Click(object sender, EventArgs e)
		{
			DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.Close();
		}

		private void ButtonOK_Click(object sender, EventArgs e)
		{
			if (DateTimeStart.Value >= DateTimeEnd.Value)
			{
				MessageBox.Show("The starting date has to be before the ending date.");
				return;
			}
			int foo;
			if (!int.TryParse(TextBoxCandidateCount.Text, out foo))
			{
				MessageBox.Show("The entered value is not a valid number. Please correct the mistake!", "Error");
				TextBoxCandidateCount.Focus();
				return;
			}
			DialogResult = System.Windows.Forms.DialogResult.OK;
			this.Close();
		}
	}
}
