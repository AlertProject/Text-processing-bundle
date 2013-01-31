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
	public partial class CandidatesPhrases : Form
	{
		public CandidatesPhrases()
		{
			InitializeComponent();
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
			DialogResult = System.Windows.Forms.DialogResult.OK;
			this.Close();
		}
	}
}
