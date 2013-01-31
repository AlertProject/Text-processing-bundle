using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

//using Contextify;
//using Contextify.Base;
using Contextify.Shared.Types;

namespace ExtractProjectSpecificConcepts
{
	public partial class CandidatesClusters : Form
	{
		//public MailData MailData = null;
		public HashSet<int> SelectedTags = new HashSet<int>();
		public ProjectSpecificConcepts ConceptsForm;
		public CandidatesClusters(ProjectSpecificConcepts conceptsForm)
		{
			InitializeComponent();
			ConceptsForm = conceptsForm;
			ComboBoxMethod.SelectedIndex = 0;
		}

		private void ButtonChooseTags_Click(object sender, EventArgs e)
		{
			SelectTags tagsDialog = new SelectTags();
			tagsDialog.LoadTags(ConceptsForm, SelectedTags);
			if (tagsDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
				SelectedTags = tagsDialog.GetSelectedTags();
		}

		public QKeywordMethod GetConceptExtractionMethod()
		{
			switch (ComboBoxMethod.SelectedIndex)
			{
				case 0: return QKeywordMethod.SVM;
				case 1: return QKeywordMethod.globalConceptSpV;
				case 2: return QKeywordMethod.localConceptSpV;
				default:
					return QKeywordMethod.SVM;
			}
		}

		public int GetConceptCount()
		{
			int count = 100;
			int.TryParse(TextBoxCandidateCount.Text, out count);
			return count;
		}

		private void ButtonOK_Click(object sender, EventArgs e)
		{
			if (RadioTags.Checked && SelectedTags.Count == 0)
			{
				MessageBox.Show("In order to compute concepts using tags you first have to select which tags to use. Click the \"Choose Tags\" button to do so.", "Error");
				return;
			}
			int foo;
			if (RadioHKMeans.Checked && !int.TryParse(TextBoxHKMeansMin.Text, out foo))
			{
				MessageBox.Show("The entered value is not a valid number. Please correct the mistake!", "Error");
				TextBoxHKMeansMin.Focus();
				return;
			}
			if (RadioHKMeans.Checked && !int.TryParse(TextBoxHKMeansMax.Text, out foo))
			{
				MessageBox.Show("The entered value is not a valid number. Please correct the mistake!", "Error");
				TextBoxHKMeansMax.Focus();
				return;
			}
			if (RadioKMeans.Checked && !int.TryParse(TextBoxKMeans.Text, out foo))
			{
				MessageBox.Show("The entered value is not a valid number. Please correct the mistake!", "Error");
				TextBoxKMeans.Focus();
				return;
			}
			if (!int.TryParse(TextBoxCandidateCount.Text, out foo))
			{
				MessageBox.Show("The entered value is not a valid number. Please correct the mistake!", "Error");
				TextBoxCandidateCount.Focus();
				return;
			}
			DialogResult = System.Windows.Forms.DialogResult.OK;
			this.Close();
		}

		private void ButtonCancel_Click(object sender, EventArgs e)
		{
			DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.Close();
		}

	}
}
