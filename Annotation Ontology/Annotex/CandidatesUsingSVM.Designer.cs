namespace ExtractProjectSpecificConcepts
{
	partial class CandidatesUsingSVM
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
			this.ButtonCancel = new System.Windows.Forms.Button();
			this.ButtonOK = new System.Windows.Forms.Button();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.label1 = new System.Windows.Forms.Label();
			this.ButtonProjectTags = new System.Windows.Forms.Button();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.ButtonNeutralTags = new System.Windows.Forms.Button();
			this.label2 = new System.Windows.Forms.Label();
			this.groupBox3 = new System.Windows.Forms.GroupBox();
			this.TextBoxCandidateCount = new System.Windows.Forms.TextBox();
			this.label6 = new System.Windows.Forms.Label();
			this.groupBox4 = new System.Windows.Forms.GroupBox();
			this.label3 = new System.Windows.Forms.Label();
			this.TextBoxTimeLimit = new System.Windows.Forms.TextBox();
			this.groupBox1.SuspendLayout();
			this.groupBox2.SuspendLayout();
			this.groupBox3.SuspendLayout();
			this.groupBox4.SuspendLayout();
			this.SuspendLayout();
			// 
			// ButtonCancel
			// 
			this.ButtonCancel.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonCancel.Location = new System.Drawing.Point(203, 354);
			this.ButtonCancel.Name = "ButtonCancel";
			this.ButtonCancel.Size = new System.Drawing.Size(98, 33);
			this.ButtonCancel.TabIndex = 14;
			this.ButtonCancel.Text = "Cancel";
			this.ButtonCancel.UseVisualStyleBackColor = true;
			this.ButtonCancel.Click += new System.EventHandler(this.ButtonCancel_Click);
			// 
			// ButtonOK
			// 
			this.ButtonOK.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonOK.Location = new System.Drawing.Point(81, 354);
			this.ButtonOK.Name = "ButtonOK";
			this.ButtonOK.Size = new System.Drawing.Size(98, 33);
			this.ButtonOK.TabIndex = 13;
			this.ButtonOK.Text = "OK";
			this.ButtonOK.UseVisualStyleBackColor = true;
			this.ButtonOK.Click += new System.EventHandler(this.ButtonOK_Click);
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.label1);
			this.groupBox1.Controls.Add(this.ButtonProjectTags);
			this.groupBox1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.groupBox1.Location = new System.Drawing.Point(25, 14);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(331, 93);
			this.groupBox1.TabIndex = 22;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Project Related Content";
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.label1.Location = new System.Drawing.Point(11, 21);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(277, 15);
			this.label1.TabIndex = 3;
			this.label1.Text = "Choose tags that are used by project related posts";
			// 
			// ButtonProjectTags
			// 
			this.ButtonProjectTags.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonProjectTags.Location = new System.Drawing.Point(213, 49);
			this.ButtonProjectTags.Name = "ButtonProjectTags";
			this.ButtonProjectTags.Size = new System.Drawing.Size(97, 28);
			this.ButtonProjectTags.TabIndex = 2;
			this.ButtonProjectTags.Text = "Choose Tags";
			this.ButtonProjectTags.UseVisualStyleBackColor = true;
			this.ButtonProjectTags.Click += new System.EventHandler(this.ButtonChooseTags_Click);
			// 
			// groupBox2
			// 
			this.groupBox2.Controls.Add(this.ButtonNeutralTags);
			this.groupBox2.Controls.Add(this.label2);
			this.groupBox2.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.groupBox2.Location = new System.Drawing.Point(26, 113);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(331, 93);
			this.groupBox2.TabIndex = 23;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "Neutral Content";
			// 
			// ButtonNeutralTags
			// 
			this.ButtonNeutralTags.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonNeutralTags.Location = new System.Drawing.Point(213, 49);
			this.ButtonNeutralTags.Name = "ButtonNeutralTags";
			this.ButtonNeutralTags.Size = new System.Drawing.Size(97, 28);
			this.ButtonNeutralTags.TabIndex = 2;
			this.ButtonNeutralTags.Text = "Choose Tags";
			this.ButtonNeutralTags.UseVisualStyleBackColor = true;
			this.ButtonNeutralTags.Click += new System.EventHandler(this.ButtonNeutralTags_Click);
			// 
			// label2
			// 
			this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.label2.Location = new System.Drawing.Point(11, 21);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(278, 34);
			this.label2.TabIndex = 3;
			this.label2.Text = "Choose tags that are used by posts that have no relation with the project";
			// 
			// groupBox3
			// 
			this.groupBox3.Controls.Add(this.TextBoxCandidateCount);
			this.groupBox3.Controls.Add(this.label6);
			this.groupBox3.Location = new System.Drawing.Point(26, 270);
			this.groupBox3.Name = "groupBox3";
			this.groupBox3.Size = new System.Drawing.Size(331, 63);
			this.groupBox3.TabIndex = 24;
			this.groupBox3.TabStop = false;
			this.groupBox3.Text = "Number of Candidates";
			// 
			// TextBoxCandidateCount
			// 
			this.TextBoxCandidateCount.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.TextBoxCandidateCount.Location = new System.Drawing.Point(257, 26);
			this.TextBoxCandidateCount.Name = "TextBoxCandidateCount";
			this.TextBoxCandidateCount.Size = new System.Drawing.Size(51, 21);
			this.TextBoxCandidateCount.TabIndex = 12;
			this.TextBoxCandidateCount.Text = "100";
			// 
			// label6
			// 
			this.label6.AutoSize = true;
			this.label6.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.label6.Location = new System.Drawing.Point(22, 29);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(229, 15);
			this.label6.TabIndex = 22;
			this.label6.Text = "Number of candidate concepts to extract:";
			// 
			// groupBox4
			// 
			this.groupBox4.Controls.Add(this.TextBoxTimeLimit);
			this.groupBox4.Controls.Add(this.label3);
			this.groupBox4.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.groupBox4.Location = new System.Drawing.Point(26, 209);
			this.groupBox4.Name = "groupBox4";
			this.groupBox4.Size = new System.Drawing.Size(331, 55);
			this.groupBox4.TabIndex = 24;
			this.groupBox4.TabStop = false;
			this.groupBox4.Text = "Time Limit";
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(11, 23);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(221, 15);
			this.label3.TabIndex = 0;
			this.label3.Text = "Time limit for extracting keywords (sec):";
			// 
			// TextBoxTimeLimit
			// 
			this.TextBoxTimeLimit.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.TextBoxTimeLimit.Location = new System.Drawing.Point(257, 20);
			this.TextBoxTimeLimit.Name = "TextBoxTimeLimit";
			this.TextBoxTimeLimit.Size = new System.Drawing.Size(53, 21);
			this.TextBoxTimeLimit.TabIndex = 23;
			this.TextBoxTimeLimit.Text = "20";
			// 
			// CandidatesUsingSVM
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(386, 414);
			this.Controls.Add(this.groupBox4);
			this.Controls.Add(this.groupBox2);
			this.Controls.Add(this.groupBox3);
			this.Controls.Add(this.groupBox1);
			this.Controls.Add(this.ButtonCancel);
			this.Controls.Add(this.ButtonOK);
			this.Name = "CandidatesUsingSVM";
			this.Text = "Candidates From Neutral Sources";
			this.groupBox1.ResumeLayout(false);
			this.groupBox1.PerformLayout();
			this.groupBox2.ResumeLayout(false);
			this.groupBox3.ResumeLayout(false);
			this.groupBox3.PerformLayout();
			this.groupBox4.ResumeLayout(false);
			this.groupBox4.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Button ButtonCancel;
		private System.Windows.Forms.Button ButtonOK;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.Button ButtonProjectTags;
		private System.Windows.Forms.GroupBox groupBox3;
		public System.Windows.Forms.TextBox TextBoxCandidateCount;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Button ButtonNeutralTags;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.GroupBox groupBox4;
		public System.Windows.Forms.TextBox TextBoxTimeLimit;
		private System.Windows.Forms.Label label3;
	}
}