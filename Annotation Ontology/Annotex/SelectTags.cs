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
	public partial class SelectTags : Form
	{
		public SelectTags()
		{
			InitializeComponent();
		}

		public void LoadTags(ProjectSpecificConcepts conceptsForm, HashSet<int> selectedTags )
		{
			List<TagInfoBase> tags = conceptsForm.GetTags(-1);
			foreach (TagInfoBase tag in tags)
			{
				TreeNode node = TreeViewTags.Nodes.Add(tag.TagName);
				node.Tag = tag.TagId;
				node.Checked = selectedTags.Contains(tag.TagId);
				AddChildren(conceptsForm, tag, node, selectedTags);
			}
		}

		private void AddChildren(ProjectSpecificConcepts conceptsForm, TagInfoBase tag, TreeNode node, HashSet<int> selectedTags)
		{
			List<TagInfoBase> childTags = conceptsForm.GetTags(tag.TagId);
			foreach (var childTag in childTags)
			{
				TreeNode childNode = node.Nodes.Add(childTag.TagName);
				childNode.Tag = childTag.TagId;
				childNode.Checked = selectedTags.Contains(childTag.TagId);
				AddChildren(conceptsForm, childTag, childNode, selectedTags);
			}
		}
		
		private void TreeViewTags_AfterCheck(object sender, TreeViewEventArgs e)
		{
			if (!CheckBoxAutoCheck.Checked)
				return;
			foreach (TreeNode child in e.Node.Nodes)
				child.Checked = e.Node.Checked;
		}

		private void ButtonOK_Click(object sender, EventArgs e)
		{
			DialogResult = System.Windows.Forms.DialogResult.OK;
			this.Close();
		}

		private void ButtonCancel_Click(object sender, EventArgs e)
		{
			DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.Close();
		}

		public HashSet<int> GetSelectedTags()
		{
			HashSet<int> selectedTags = new HashSet<int>();
			foreach (TreeNode node in TreeViewTags.Nodes)
				AddSelectedNodes(node, selectedTags);
			return selectedTags;
		}

		private void AddSelectedNodes(TreeNode node, HashSet<int> selectedTags)
		{
			if (node.Checked)
				selectedTags.Add((int) node.Tag);
			foreach (TreeNode child in node.Nodes)
				AddSelectedNodes(child, selectedTags);
		}
	}
}
