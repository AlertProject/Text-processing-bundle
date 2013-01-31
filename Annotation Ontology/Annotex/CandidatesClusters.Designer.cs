namespace ExtractProjectSpecificConcepts
{
	partial class CandidatesClusters
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
			this.TextBoxHKMeansMax = new System.Windows.Forms.TextBox();
			this.label7 = new System.Windows.Forms.Label();
			this.TextBoxHKMeansMin = new System.Windows.Forms.TextBox();
			this.label2 = new System.Windows.Forms.Label();
			this.TextBoxKMeans = new System.Windows.Forms.TextBox();
			this.label1 = new System.Windows.Forms.Label();
			this.RadioKMeans = new System.Windows.Forms.RadioButton();
			this.RadioHKMeans = new System.Windows.Forms.RadioButton();
			this.ButtonChooseTags = new System.Windows.Forms.Button();
			this.RadioTags = new System.Windows.Forms.RadioButton();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.ComboBoxMethod = new System.Windows.Forms.ComboBox();
			this.label3 = new System.Windows.Forms.Label();
			this.groupBox3 = new System.Windows.Forms.GroupBox();
			this.TextBoxCandidateCount = new System.Windows.Forms.TextBox();
			this.label6 = new System.Windows.Forms.Label();
			this.groupBox1.SuspendLayout();
			this.groupBox2.SuspendLayout();
			this.groupBox3.SuspendLayout();
			this.SuspendLayout();
			// 
			// ButtonCancel
			// 
			this.ButtonCancel.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonCancel.Location = new System.Drawing.Point(193, 401);
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
			this.ButtonOK.Location = new System.Drawing.Point(71, 401);
			this.ButtonOK.Name = "ButtonOK";
			this.ButtonOK.Size = new System.Drawing.Size(98, 33);
			this.ButtonOK.TabIndex = 13;
			this.ButtonOK.Text = "OK";
			this.ButtonOK.UseVisualStyleBackColor = true;
			this.ButtonOK.Click += new System.EventHandler(this.ButtonOK_Click);
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.TextBoxHKMeansMax);
			this.groupBox1.Controls.Add(this.label7);
			this.groupBox1.Controls.Add(this.TextBoxHKMeansMin);
			this.groupBox1.Controls.Add(this.label2);
			this.groupBox1.Controls.Add(this.TextBoxKMeans);
			this.groupBox1.Controls.Add(this.label1);
			this.groupBox1.Controls.Add(this.RadioKMeans);
			this.groupBox1.Controls.Add(this.RadioHKMeans);
			this.groupBox1.Controls.Add(this.ButtonChooseTags);
			this.groupBox1.Controls.Add(this.RadioTags);
			this.groupBox1.Location = new System.Drawing.Point(25, 14);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(331, 221);
			this.groupBox1.TabIndex = 22;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Method for Selecting Groups";
			// 
			// TextBoxHKMeansMax
			// 
			this.TextBoxHKMeansMax.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.TextBoxHKMeansMax.Location = new System.Drawing.Point(258, 118);
			this.TextBoxHKMeansMax.Name = "TextBoxHKMeansMax";
			this.TextBoxHKMeansMax.Size = new System.Drawing.Size(51, 21);
			this.TextBoxHKMeansMax.TabIndex = 5;
			this.TextBoxHKMeansMax.Text = "2000";
			// 
			// label7
			// 
			this.label7.AutoSize = true;
			this.label7.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.label7.Location = new System.Drawing.Point(124, 121);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(125, 15);
			this.label7.TabIndex = 33;
			this.label7.Text = "max items in clusters:";
			// 
			// TextBoxHKMeansMin
			// 
			this.TextBoxHKMeansMin.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.TextBoxHKMeansMin.Location = new System.Drawing.Point(258, 91);
			this.TextBoxHKMeansMin.Name = "TextBoxHKMeansMin";
			this.TextBoxHKMeansMin.Size = new System.Drawing.Size(51, 21);
			this.TextBoxHKMeansMin.TabIndex = 4;
			this.TextBoxHKMeansMin.Text = "200";
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.label2.Location = new System.Drawing.Point(127, 94);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(122, 15);
			this.label2.TabIndex = 26;
			this.label2.Text = "min items in clusters:";
			// 
			// TextBoxKMeans
			// 
			this.TextBoxKMeans.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.TextBoxKMeans.Location = new System.Drawing.Point(258, 177);
			this.TextBoxKMeans.Name = "TextBoxKMeans";
			this.TextBoxKMeans.Size = new System.Drawing.Size(51, 21);
			this.TextBoxKMeans.TabIndex = 7;
			this.TextBoxKMeans.Text = "5";
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.label1.Location = new System.Drawing.Point(120, 180);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(115, 15);
			this.label1.TabIndex = 24;
			this.label1.Text = "K (num. of clusters):";
			// 
			// RadioKMeans
			// 
			this.RadioKMeans.AutoSize = true;
			this.RadioKMeans.Checked = true;
			this.RadioKMeans.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.RadioKMeans.Location = new System.Drawing.Point(19, 156);
			this.RadioKMeans.Name = "RadioKMeans";
			this.RadioKMeans.Size = new System.Drawing.Size(232, 19);
			this.RadioKMeans.TabIndex = 6;
			this.RadioKMeans.TabStop = true;
			this.RadioKMeans.Text = "Cluster data using K-means clustering";
			this.RadioKMeans.UseVisualStyleBackColor = true;
			// 
			// RadioHKMeans
			// 
			this.RadioHKMeans.AutoSize = true;
			this.RadioHKMeans.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.RadioHKMeans.Location = new System.Drawing.Point(19, 68);
			this.RadioHKMeans.Name = "RadioHKMeans";
			this.RadioHKMeans.Size = new System.Drawing.Size(299, 19);
			this.RadioHKMeans.TabIndex = 3;
			this.RadioHKMeans.TabStop = true;
			this.RadioHKMeans.Text = "Cluster data using hierarchical K-means clustering";
			this.RadioHKMeans.UseVisualStyleBackColor = true;
			// 
			// ButtonChooseTags
			// 
			this.ButtonChooseTags.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonChooseTags.Location = new System.Drawing.Point(212, 27);
			this.ButtonChooseTags.Name = "ButtonChooseTags";
			this.ButtonChooseTags.Size = new System.Drawing.Size(97, 28);
			this.ButtonChooseTags.TabIndex = 2;
			this.ButtonChooseTags.Text = "Choose Tags";
			this.ButtonChooseTags.UseVisualStyleBackColor = true;
			this.ButtonChooseTags.Click += new System.EventHandler(this.ButtonChooseTags_Click);
			// 
			// RadioTags
			// 
			this.RadioTags.AutoSize = true;
			this.RadioTags.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.RadioTags.Location = new System.Drawing.Point(19, 29);
			this.RadioTags.Name = "RadioTags";
			this.RadioTags.Size = new System.Drawing.Size(149, 19);
			this.RadioTags.TabIndex = 1;
			this.RadioTags.TabStop = true;
			this.RadioTags.Text = "Cluster data using tags";
			this.RadioTags.UseVisualStyleBackColor = true;
			// 
			// groupBox2
			// 
			this.groupBox2.Controls.Add(this.ComboBoxMethod);
			this.groupBox2.Controls.Add(this.label3);
			this.groupBox2.Location = new System.Drawing.Point(26, 241);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(331, 72);
			this.groupBox2.TabIndex = 23;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "Method of Concept Extraction from Clusters";
			// 
			// ComboBoxMethod
			// 
			this.ComboBoxMethod.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ComboBoxMethod.FormattingEnabled = true;
			this.ComboBoxMethod.Items.AddRange(new object[] {
            "Classification using SVM",
            "Concept extraction using global weighting scheme",
            "Concept extraction using local weighting scheme"});
			this.ComboBoxMethod.Location = new System.Drawing.Point(19, 28);
			this.ComboBoxMethod.Name = "ComboBoxMethod";
			this.ComboBoxMethod.Size = new System.Drawing.Size(290, 23);
			this.ComboBoxMethod.TabIndex = 11;
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.label3.Location = new System.Drawing.Point(16, 28);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(0, 15);
			this.label3.TabIndex = 11;
			// 
			// groupBox3
			// 
			this.groupBox3.Controls.Add(this.TextBoxCandidateCount);
			this.groupBox3.Controls.Add(this.label6);
			this.groupBox3.Location = new System.Drawing.Point(26, 319);
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
			// CandidatesClusters
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(386, 463);
			this.Controls.Add(this.groupBox3);
			this.Controls.Add(this.groupBox2);
			this.Controls.Add(this.groupBox1);
			this.Controls.Add(this.ButtonCancel);
			this.Controls.Add(this.ButtonOK);
			this.Name = "CandidatesClusters";
			this.Text = "Candidates From Clusters";
			this.groupBox1.ResumeLayout(false);
			this.groupBox1.PerformLayout();
			this.groupBox2.ResumeLayout(false);
			this.groupBox2.PerformLayout();
			this.groupBox3.ResumeLayout(false);
			this.groupBox3.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Button ButtonCancel;
		private System.Windows.Forms.Button ButtonOK;
		private System.Windows.Forms.GroupBox groupBox1;
		public System.Windows.Forms.TextBox TextBoxHKMeansMax;
		private System.Windows.Forms.Label label7;
		public System.Windows.Forms.TextBox TextBoxHKMeansMin;
		private System.Windows.Forms.Label label2;
		public System.Windows.Forms.TextBox TextBoxKMeans;
		private System.Windows.Forms.Label label1;
		public System.Windows.Forms.RadioButton RadioKMeans;
		public System.Windows.Forms.RadioButton RadioHKMeans;
		private System.Windows.Forms.Button ButtonChooseTags;
		public System.Windows.Forms.RadioButton RadioTags;
		private System.Windows.Forms.GroupBox groupBox2;
		public System.Windows.Forms.ComboBox ComboBoxMethod;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.GroupBox groupBox3;
		public System.Windows.Forms.TextBox TextBoxCandidateCount;
		private System.Windows.Forms.Label label6;
	}
}