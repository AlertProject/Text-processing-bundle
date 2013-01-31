namespace ExtractProjectSpecificConcepts
{
	partial class CandidatesRecent
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
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.label5 = new System.Windows.Forms.Label();
			this.label4 = new System.Windows.Forms.Label();
			this.DateTimeEnd = new System.Windows.Forms.DateTimePicker();
			this.DateTimeStart = new System.Windows.Forms.DateTimePicker();
			this.TextBoxCandidateCount = new System.Windows.Forms.TextBox();
			this.groupBox3 = new System.Windows.Forms.GroupBox();
			this.label6 = new System.Windows.Forms.Label();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.ComboBoxMethod = new System.Windows.Forms.ComboBox();
			this.label3 = new System.Windows.Forms.Label();
			this.ButtonCancel = new System.Windows.Forms.Button();
			this.ButtonOK = new System.Windows.Forms.Button();
			this.groupBox1.SuspendLayout();
			this.groupBox3.SuspendLayout();
			this.groupBox2.SuspendLayout();
			this.SuspendLayout();
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.label5);
			this.groupBox1.Controls.Add(this.label4);
			this.groupBox1.Controls.Add(this.DateTimeEnd);
			this.groupBox1.Controls.Add(this.DateTimeStart);
			this.groupBox1.Location = new System.Drawing.Point(21, 28);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(331, 86);
			this.groupBox1.TabIndex = 0;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Time period";
			// 
			// label5
			// 
			this.label5.AutoSize = true;
			this.label5.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.label5.Location = new System.Drawing.Point(26, 50);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(59, 15);
			this.label5.TabIndex = 36;
			this.label5.Text = "End date:";
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.label4.Location = new System.Drawing.Point(26, 25);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(62, 15);
			this.label4.TabIndex = 35;
			this.label4.Text = "Start date:";
			// 
			// DateTimeEnd
			// 
			this.DateTimeEnd.Location = new System.Drawing.Point(136, 49);
			this.DateTimeEnd.Name = "DateTimeEnd";
			this.DateTimeEnd.Size = new System.Drawing.Size(155, 20);
			this.DateTimeEnd.TabIndex = 34;
			// 
			// DateTimeStart
			// 
			this.DateTimeStart.Location = new System.Drawing.Point(136, 23);
			this.DateTimeStart.Name = "DateTimeStart";
			this.DateTimeStart.Size = new System.Drawing.Size(155, 20);
			this.DateTimeStart.TabIndex = 33;
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
			// groupBox3
			// 
			this.groupBox3.Controls.Add(this.TextBoxCandidateCount);
			this.groupBox3.Controls.Add(this.label6);
			this.groupBox3.Location = new System.Drawing.Point(21, 198);
			this.groupBox3.Name = "groupBox3";
			this.groupBox3.Size = new System.Drawing.Size(331, 63);
			this.groupBox3.TabIndex = 28;
			this.groupBox3.TabStop = false;
			this.groupBox3.Text = "Number of Candidates";
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
			// groupBox2
			// 
			this.groupBox2.Controls.Add(this.ComboBoxMethod);
			this.groupBox2.Controls.Add(this.label3);
			this.groupBox2.Location = new System.Drawing.Point(21, 120);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(331, 72);
			this.groupBox2.TabIndex = 27;
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
			// ButtonCancel
			// 
			this.ButtonCancel.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonCancel.Location = new System.Drawing.Point(188, 280);
			this.ButtonCancel.Name = "ButtonCancel";
			this.ButtonCancel.Size = new System.Drawing.Size(98, 33);
			this.ButtonCancel.TabIndex = 26;
			this.ButtonCancel.Text = "Cancel";
			this.ButtonCancel.UseVisualStyleBackColor = true;
			this.ButtonCancel.Click += new System.EventHandler(this.ButtonCancel_Click);
			// 
			// ButtonOK
			// 
			this.ButtonOK.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonOK.Location = new System.Drawing.Point(66, 280);
			this.ButtonOK.Name = "ButtonOK";
			this.ButtonOK.Size = new System.Drawing.Size(98, 33);
			this.ButtonOK.TabIndex = 25;
			this.ButtonOK.Text = "OK";
			this.ButtonOK.UseVisualStyleBackColor = true;
			this.ButtonOK.Click += new System.EventHandler(this.ButtonOK_Click);
			// 
			// CandidatesRecent
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(374, 345);
			this.Controls.Add(this.groupBox3);
			this.Controls.Add(this.groupBox2);
			this.Controls.Add(this.ButtonCancel);
			this.Controls.Add(this.ButtonOK);
			this.Controls.Add(this.groupBox1);
			this.Name = "CandidatesRecent";
			this.Text = "Candidates From Recent Posts";
			this.groupBox1.ResumeLayout(false);
			this.groupBox1.PerformLayout();
			this.groupBox3.ResumeLayout(false);
			this.groupBox3.PerformLayout();
			this.groupBox2.ResumeLayout(false);
			this.groupBox2.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.Label label4;
		public System.Windows.Forms.DateTimePicker DateTimeEnd;
		public System.Windows.Forms.DateTimePicker DateTimeStart;
		public System.Windows.Forms.TextBox TextBoxCandidateCount;
		private System.Windows.Forms.GroupBox groupBox3;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.GroupBox groupBox2;
		public System.Windows.Forms.ComboBox ComboBoxMethod;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Button ButtonCancel;
		private System.Windows.Forms.Button ButtonOK;
	}
}