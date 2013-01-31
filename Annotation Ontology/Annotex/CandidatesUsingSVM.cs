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
	public partial class CandidatesUsingSVM : Form
	{
		//public MailData MailData = null;
		public HashSet<int> ProjectTags = new HashSet<int>();
		public HashSet<int> NeutralTags = new HashSet<int>();
		public ProjectSpecificConcepts ConceptsForm;

		public CandidatesUsingSVM(ProjectSpecificConcepts conceptsForm)
		{
			InitializeComponent();
			ConceptsForm = conceptsForm;
		}

		private void ButtonChooseTags_Click(object sender, EventArgs e)
		{
			SelectTags tagsDialog = new SelectTags();
			tagsDialog.LoadTags(ConceptsForm, ProjectTags);
			if (tagsDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
				ProjectTags = tagsDialog.GetSelectedTags();
		}


		private void ButtonNeutralTags_Click(object sender, EventArgs e)
		{
			SelectTags tagsDialog = new SelectTags();
			tagsDialog.LoadTags(ConceptsForm, NeutralTags);
			if (tagsDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
				NeutralTags = tagsDialog.GetSelectedTags();
		}

		public int GetConceptCount()
		{
			int count = 100;
			int.TryParse(TextBoxCandidateCount.Text, out count);
			return count;
		}

		public int GetTimeLimit()
		{
			int limit = 20;
			int.TryParse(TextBoxTimeLimit.Text, out limit);
			return limit;
		}

		private void ButtonOK_Click(object sender, EventArgs e)
		{
			if (ProjectTags.Count == 0)
			{
				MessageBox.Show("No project specific tags are selected.", "Error");
				return;
			}
			if (NeutralTags.Count == 0)
			{
				MessageBox.Show("No tags for neutral posts are selected.", "Error");
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

		private void ButtonCancel_Click(object sender, EventArgs e)
		{
			DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.Close();
		}


	}
}
