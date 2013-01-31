namespace ExtractProjectSpecificConcepts
{
	partial class SelectExistingConcept
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.label1 = new System.Windows.Forms.Label();
			this.TextBoxFilter = new System.Windows.Forms.TextBox();
			this.ButtonUpdate = new System.Windows.Forms.Button();
			this.ListViewExistingConcepts = new ExtractProjectSpecificConcepts.ListViewWithCombo();
			this.ColumnHeaderLabels = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.ColumnHeaderConceptURI = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.ButtonChooseConcept = new System.Windows.Forms.Button();
			this.ButtonCancel = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.label1.Location = new System.Drawing.Point(12, 15);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(37, 15);
			this.label1.TabIndex = 0;
			this.label1.Text = "Filter:";
			// 
			// TextBoxFilter
			// 
			this.TextBoxFilter.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.TextBoxFilter.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.TextBoxFilter.Location = new System.Drawing.Point(59, 12);
			this.TextBoxFilter.Name = "TextBoxFilter";
			this.TextBoxFilter.Size = new System.Drawing.Size(441, 21);
			this.TextBoxFilter.TabIndex = 1;
			// 
			// ButtonUpdate
			// 
			this.ButtonUpdate.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.ButtonUpdate.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonUpdate.Location = new System.Drawing.Point(506, 10);
			this.ButtonUpdate.Name = "ButtonUpdate";
			this.ButtonUpdate.Size = new System.Drawing.Size(75, 23);
			this.ButtonUpdate.TabIndex = 2;
			this.ButtonUpdate.Text = "Update";
			this.ButtonUpdate.UseVisualStyleBackColor = true;
			this.ButtonUpdate.Click += new System.EventHandler(this.ButtonUpdate_Click);
			// 
			// ListViewExistingConcepts
			// 
			this.ListViewExistingConcepts.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.ListViewExistingConcepts.CheckBoxes = true;
			this.ListViewExistingConcepts.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.ColumnHeaderLabels,
            this.ColumnHeaderConceptURI});
			this.ListViewExistingConcepts.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ListViewExistingConcepts.FullRowSelect = true;
			this.ListViewExistingConcepts.GridLines = true;
			this.ListViewExistingConcepts.HideSelection = false;
			this.ListViewExistingConcepts.Location = new System.Drawing.Point(15, 53);
			this.ListViewExistingConcepts.Name = "ListViewExistingConcepts";
			this.ListViewExistingConcepts.Size = new System.Drawing.Size(566, 404);
			this.ListViewExistingConcepts.TabIndex = 21;
			this.ListViewExistingConcepts.UseCompatibleStateImageBehavior = false;
			this.ListViewExistingConcepts.View = System.Windows.Forms.View.Details;
			// 
			// ColumnHeaderLabels
			// 
			this.ColumnHeaderLabels.Text = "Concept Labels";
			this.ColumnHeaderLabels.Width = 278;
			// 
			// ColumnHeaderConceptURI
			// 
			this.ColumnHeaderConceptURI.Text = "Concept URI";
			this.ColumnHeaderConceptURI.Width = 264;
			// 
			// ButtonChooseConcept
			// 
			this.ButtonChooseConcept.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.ButtonChooseConcept.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonChooseConcept.Location = new System.Drawing.Point(260, 476);
			this.ButtonChooseConcept.Name = "ButtonChooseConcept";
			this.ButtonChooseConcept.Size = new System.Drawing.Size(199, 41);
			this.ButtonChooseConcept.TabIndex = 22;
			this.ButtonChooseConcept.Text = "Choose selected concept";
			this.ButtonChooseConcept.UseVisualStyleBackColor = true;
			this.ButtonChooseConcept.Click += new System.EventHandler(this.ButtonChooseConcept_Click);
			// 
			// ButtonCancel
			// 
			this.ButtonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.ButtonCancel.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonCancel.Location = new System.Drawing.Point(480, 476);
			this.ButtonCancel.Name = "ButtonCancel";
			this.ButtonCancel.Size = new System.Drawing.Size(101, 41);
			this.ButtonCancel.TabIndex = 23;
			this.ButtonCancel.Text = "Cancel";
			this.ButtonCancel.UseVisualStyleBackColor = true;
			this.ButtonCancel.Click += new System.EventHandler(this.ButtonCancel_Click);
			// 
			// SelectExistingConcept
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(598, 529);
			this.Controls.Add(this.ButtonCancel);
			this.Controls.Add(this.ButtonChooseConcept);
			this.Controls.Add(this.ListViewExistingConcepts);
			this.Controls.Add(this.ButtonUpdate);
			this.Controls.Add(this.TextBoxFilter);
			this.Controls.Add(this.label1);
			this.Name = "SelectExistingConcept";
			this.Text = "Select an existing concept";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox TextBoxFilter;
		private System.Windows.Forms.Button ButtonUpdate;
		private ListViewWithCombo ListViewExistingConcepts;
		private System.Windows.Forms.ColumnHeader ColumnHeaderLabels;
		private System.Windows.Forms.ColumnHeader ColumnHeaderConceptURI;
		private System.Windows.Forms.Button ButtonChooseConcept;
		private System.Windows.Forms.Button ButtonCancel;
	}
}