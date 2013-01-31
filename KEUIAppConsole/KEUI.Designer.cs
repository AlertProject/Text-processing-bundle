namespace KEUIApp
{
#if UI
	partial class KEUIDialog
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
			this.ListBoxLog = new System.Windows.Forms.ListBox();
			this.label1 = new System.Windows.Forms.Label();
			this.ButtonReload = new System.Windows.Forms.Button();
			this.ButtonClose = new System.Windows.Forms.Button();
			this.label2 = new System.Windows.Forms.Label();
			this.LabelTime = new System.Windows.Forms.Label();
			this.LabelLastId = new System.Windows.Forms.Label();
			this.label4 = new System.Windows.Forms.Label();
			this.LabelProcessedCount = new System.Windows.Forms.Label();
			this.label5 = new System.Windows.Forms.Label();
			this.SuspendLayout();
			// 
			// ListBoxLog
			// 
			this.ListBoxLog.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.ListBoxLog.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.ListBoxLog.FormattingEnabled = true;
			this.ListBoxLog.ItemHeight = 16;
			this.ListBoxLog.Location = new System.Drawing.Point(14, 40);
			this.ListBoxLog.Name = "ListBoxLog";
			this.ListBoxLog.Size = new System.Drawing.Size(523, 212);
			this.ListBoxLog.TabIndex = 8;
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.label1.Location = new System.Drawing.Point(14, 13);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(76, 17);
			this.label1.TabIndex = 9;
			this.label1.Text = "Event Log:";
			// 
			// ButtonReload
			// 
			this.ButtonReload.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.ButtonReload.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonReload.Location = new System.Drawing.Point(12, 367);
			this.ButtonReload.Name = "ButtonReload";
			this.ButtonReload.Size = new System.Drawing.Size(251, 35);
			this.ButtonReload.TabIndex = 10;
			this.ButtonReload.Text = "Reload Annotation Data";
			this.ButtonReload.UseVisualStyleBackColor = true;
			this.ButtonReload.Visible = false;
			this.ButtonReload.Click += new System.EventHandler(this.ButtonReload_Click);
			// 
			// ButtonClose
			// 
			this.ButtonClose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.ButtonClose.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonClose.Location = new System.Drawing.Point(284, 367);
			this.ButtonClose.Name = "ButtonClose";
			this.ButtonClose.Size = new System.Drawing.Size(251, 35);
			this.ButtonClose.TabIndex = 11;
			this.ButtonClose.Text = "Close Application";
			this.ButtonClose.UseVisualStyleBackColor = true;
			this.ButtonClose.Click += new System.EventHandler(this.ButtonClose_Click);
			// 
			// label2
			// 
			this.label2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.label2.AutoSize = true;
			this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.label2.Location = new System.Drawing.Point(14, 339);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(98, 15);
			this.label2.TabIndex = 12;
			this.label2.Text = "Processing time:";
			// 
			// LabelTime
			// 
			this.LabelTime.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.LabelTime.AutoSize = true;
			this.LabelTime.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.LabelTime.Location = new System.Drawing.Point(118, 339);
			this.LabelTime.Name = "LabelTime";
			this.LabelTime.Size = new System.Drawing.Size(26, 15);
			this.LabelTime.TabIndex = 13;
			this.LabelTime.Text = "0 s";
			// 
			// LabelLastId
			// 
			this.LabelLastId.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.LabelLastId.AutoSize = true;
			this.LabelLastId.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.LabelLastId.Location = new System.Drawing.Point(140, 314);
			this.LabelLastId.Name = "LabelLastId";
			this.LabelLastId.Size = new System.Drawing.Size(0, 15);
			this.LabelLastId.TabIndex = 15;
			// 
			// label4
			// 
			this.label4.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.label4.AutoSize = true;
			this.label4.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.label4.Location = new System.Drawing.Point(12, 314);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(122, 15);
			this.label4.TabIndex = 14;
			this.label4.Text = "ID of last added item:";
			// 
			// LabelProcessedCount
			// 
			this.LabelProcessedCount.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.LabelProcessedCount.AutoSize = true;
			this.LabelProcessedCount.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.LabelProcessedCount.Location = new System.Drawing.Point(179, 288);
			this.LabelProcessedCount.Name = "LabelProcessedCount";
			this.LabelProcessedCount.Size = new System.Drawing.Size(15, 15);
			this.LabelProcessedCount.TabIndex = 17;
			this.LabelProcessedCount.Text = "0";
			// 
			// label5
			// 
			this.label5.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.label5.AutoSize = true;
			this.label5.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.label5.Location = new System.Drawing.Point(12, 288);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(161, 15);
			this.label5.TabIndex = 16;
			this.label5.Text = "Number of processed items:";
			// 
			// KEUIDialog
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(552, 414);
			this.Controls.Add(this.LabelProcessedCount);
			this.Controls.Add(this.label5);
			this.Controls.Add(this.LabelLastId);
			this.Controls.Add(this.label4);
			this.Controls.Add(this.LabelTime);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.ButtonClose);
			this.Controls.Add(this.ButtonReload);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.ListBoxLog);
			this.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.MinimumSize = new System.Drawing.Size(560, 400);
			this.Name = "KEUIDialog";
			this.Text = "KEUI";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ListBox ListBoxLog;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button ButtonReload;
		private System.Windows.Forms.Button ButtonClose;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label LabelTime;
		private System.Windows.Forms.Label LabelLastId;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.Label LabelProcessedCount;
		private System.Windows.Forms.Label label5;
	}
#endif
}

