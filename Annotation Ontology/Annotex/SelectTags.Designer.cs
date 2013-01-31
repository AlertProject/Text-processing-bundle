namespace ExtractProjectSpecificConcepts
{
	partial class SelectTags
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
			this.TreeViewTags = new System.Windows.Forms.TreeView();
			this.label3 = new System.Windows.Forms.Label();
			this.ButtonOK = new System.Windows.Forms.Button();
			this.ButtonCancel = new System.Windows.Forms.Button();
			this.CheckBoxAutoCheck = new System.Windows.Forms.CheckBox();
			this.SuspendLayout();
			// 
			// TreeViewTags
			// 
			this.TreeViewTags.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.TreeViewTags.CheckBoxes = true;
			this.TreeViewTags.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.TreeViewTags.Location = new System.Drawing.Point(12, 35);
			this.TreeViewTags.Name = "TreeViewTags";
			this.TreeViewTags.Size = new System.Drawing.Size(282, 309);
			this.TreeViewTags.TabIndex = 2;
			this.TreeViewTags.AfterCheck += new System.Windows.Forms.TreeViewEventHandler(this.TreeViewTags_AfterCheck);
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.label3.Location = new System.Drawing.Point(12, 9);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(285, 15);
			this.label3.TabIndex = 10;
			this.label3.Text = "Select tags to consider when performing clustering:";
			// 
			// ButtonOK
			// 
			this.ButtonOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.ButtonOK.Location = new System.Drawing.Point(44, 396);
			this.ButtonOK.Name = "ButtonOK";
			this.ButtonOK.Size = new System.Drawing.Size(98, 33);
			this.ButtonOK.TabIndex = 11;
			this.ButtonOK.Text = "OK";
			this.ButtonOK.UseVisualStyleBackColor = true;
			this.ButtonOK.Click += new System.EventHandler(this.ButtonOK_Click);
			// 
			// ButtonCancel
			// 
			this.ButtonCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.ButtonCancel.Location = new System.Drawing.Point(166, 396);
			this.ButtonCancel.Name = "ButtonCancel";
			this.ButtonCancel.Size = new System.Drawing.Size(98, 33);
			this.ButtonCancel.TabIndex = 12;
			this.ButtonCancel.Text = "Cancel";
			this.ButtonCancel.UseVisualStyleBackColor = true;
			this.ButtonCancel.Click += new System.EventHandler(this.ButtonCancel_Click);
			// 
			// CheckBoxAutoCheck
			// 
			this.CheckBoxAutoCheck.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.CheckBoxAutoCheck.AutoSize = true;
			this.CheckBoxAutoCheck.Location = new System.Drawing.Point(12, 360);
			this.CheckBoxAutoCheck.Name = "CheckBoxAutoCheck";
			this.CheckBoxAutoCheck.Size = new System.Drawing.Size(240, 17);
			this.CheckBoxAutoCheck.TabIndex = 13;
			this.CheckBoxAutoCheck.Text = "Automatically check/uncheck children nodes";
			this.CheckBoxAutoCheck.UseVisualStyleBackColor = true;
			// 
			// SelectTags
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(314, 447);
			this.Controls.Add(this.CheckBoxAutoCheck);
			this.Controls.Add(this.ButtonCancel);
			this.Controls.Add(this.ButtonOK);
			this.Controls.Add(this.label3);
			this.Controls.Add(this.TreeViewTags);
			this.Name = "SelectTags";
			this.Text = "Tags";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		public System.Windows.Forms.TreeView TreeViewTags;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Button ButtonOK;
		private System.Windows.Forms.Button ButtonCancel;
		private System.Windows.Forms.CheckBox CheckBoxAutoCheck;
	}
}