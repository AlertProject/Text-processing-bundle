using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using KEUIApp;

namespace ExtractProjectSpecificConcepts
{
	public partial class SelectExistingConcept : Form
	{
		private AnnotationOntology _ao;
		public List<string> SelectedConceptUris = new List<string>();
		private bool _showCheckboxes;

		public SelectExistingConcept(AnnotationOntology ao, bool showCheckboxes)
		{
			_ao = ao;
			_showCheckboxes = showCheckboxes;
			InitializeComponent();
			this.Load += new EventHandler(SelectExistingConcept_Load);
		}

		void SelectExistingConcept_Load(object sender, EventArgs e)
		{
			ListViewExistingConcepts.CheckBoxes = _showCheckboxes;
			LoadExistingConcepts();
		}


		private void ButtonCancel_Click(object sender, EventArgs e)
		{
			DialogResult = System.Windows.Forms.DialogResult.Cancel;
			Close();
		}

		private void ButtonChooseConcept_Click(object sender, EventArgs e)
		{
			SelectedConceptUris.Clear();
			if (_showCheckboxes)
			{
				// we want to add related concepts
				if (ListViewExistingConcepts.CheckedItems.Count > 0)
				{
					foreach (ListViewItem item in ListViewExistingConcepts.CheckedItems)
						SelectedConceptUris.Add(item.SubItems[1].Text);
					DialogResult = System.Windows.Forms.DialogResult.OK;
					Close();
				}
				else
				{
					MessageBox.Show("You have to select at least one concept that you would like to add as a related concept.", "Information", MessageBoxButtons.OK, MessageBoxIcon.Information);
				}
			}
			else
			{
				// we want to edit an existing concept
				if (ListViewExistingConcepts.SelectedItems.Count == 1)
				{
					SelectedConceptUris.Add(ListViewExistingConcepts.SelectedItems[0].SubItems[1].Text);
					DialogResult = System.Windows.Forms.DialogResult.OK;
					Close();
				}
				else
				{
					MessageBox.Show("You have to select exactly one concept that you would like to edit.", "Information", MessageBoxButtons.OK, MessageBoxIcon.Information);
				}
			}
		}

		//List<ListViewItem> _allConceptItems = new List<ListViewItem>();
		List<ListViewItem> _hiddenConceptItems = new List<ListViewItem>();

		// load the existing ontology concepts. we need them to be able to add a label to an existing concept
		private void LoadExistingConcepts()
		{
			_hiddenConceptItems = new List<ListViewItem>();
			// load the concepts into the _conceptIdToLabels

			ListViewExistingConcepts.BeginUpdate();
			ListViewExistingConcepts.Items.Clear();
			foreach (string key in _ao.ConceptUriToLabels.Keys)
			{
				string labels = string.Join(", ", _ao.ConceptUriToLabels[key]);
				ListViewItem item = new ListViewItem(labels);
				item.SubItems.Add(key);
				ListViewExistingConcepts.Items.Add(item);
				//_allConceptItems.Add(item);
			}
			ListViewExistingConcepts.EndUpdate();
			ListViewExistingConcepts.Sorting = SortOrder.None;
		}
		
		private void ButtonUpdate_Click(object sender, EventArgs e)
		{
			System.Collections.IComparer comparer = ListViewExistingConcepts.ListViewItemSorter;
			ListViewExistingConcepts.ListViewItemSorter = null;

			ListViewExistingConcepts.BeginUpdate();
			foreach (var item in _hiddenConceptItems)
				ListViewExistingConcepts.Items.Add(item);
			_hiddenConceptItems.Clear();

			string filter = TextBoxFilter.Text.ToLower();
			if (filter != "")
			{
				for (int i = ListViewExistingConcepts.Items.Count - 1; i >= 0; i--)
				{
					ListViewItem item = ListViewExistingConcepts.Items[i];
					if (!item.SubItems[0].Text.ToLower().Contains(filter))
					{
						_hiddenConceptItems.Add(item);
						ListViewExistingConcepts.Items.RemoveAt(i);
					}
				}
			}
			ListViewExistingConcepts.EndUpdate();
			ListViewExistingConcepts.ListViewItemSorter = comparer;
		}


	}
}
