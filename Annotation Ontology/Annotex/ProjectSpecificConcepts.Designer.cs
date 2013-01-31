namespace ExtractProjectSpecificConcepts
{
	partial class ProjectSpecificConcepts
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
			this.components = new System.ComponentModel.Container();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.ButtonClear = new System.Windows.Forms.Button();
			this.ListViewCandidates = new System.Windows.Forms.ListView();
			this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.ignoreSelectedCandidatesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.ButtonSave = new System.Windows.Forms.Button();
			this.ButtonLoad = new System.Windows.Forms.Button();
			this.ButtonGetCandidates = new System.Windows.Forms.Button();
			this.contextMenuStrip2 = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.extractCandidatesUsingClusteringToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.extractCandidatesFromRecentPostsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.extractPhrasesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.extractCandidatesUsingNeutralContentMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.statusStrip1 = new System.Windows.Forms.StatusStrip();
			this.StatusText = new System.Windows.Forms.ToolStripStatusLabel();
			this.StatusProgressBar = new System.Windows.Forms.ToolStripProgressBar();
			this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
			this.TextBoxDescription = new System.Windows.Forms.TextBox();
			this.TextBoxLabels = new System.Windows.Forms.TextBox();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.ButtonGroupAdd = new System.Windows.Forms.Button();
			this.groupBox4 = new System.Windows.Forms.GroupBox();
			this.ButtonSuggestConceptURI = new System.Windows.Forms.Button();
			this.label2 = new System.Windows.Forms.Label();
			this.TextBoxConceptUri = new System.Windows.Forms.TextBox();
			this.groupBox3 = new System.Windows.Forms.GroupBox();
			this.ComboBoxRelations = new System.Windows.Forms.ComboBox();
			this.ButtonAutoAddRelatedConcepts = new System.Windows.Forms.Button();
			this.ButtonManuallyAddRelatedConcepts = new System.Windows.Forms.Button();
			this.ListViewRelated = new ExtractProjectSpecificConcepts.ListViewWithCombo();
			this.ColumnHeaderRemove = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.ColumnHeaderConceptName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.ColumnHeaderRelation = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.groupBox5 = new System.Windows.Forms.GroupBox();
			this.ButtonLearnDescription = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.ButtonAddSelectedConcept = new System.Windows.Forms.Button();
			this.ButtonEditExisting = new System.Windows.Forms.Button();
			this.groupBox2.SuspendLayout();
			this.contextMenuStrip1.SuspendLayout();
			this.contextMenuStrip2.SuspendLayout();
			this.statusStrip1.SuspendLayout();
			this.groupBox1.SuspendLayout();
			this.groupBox4.SuspendLayout();
			this.groupBox3.SuspendLayout();
			this.groupBox5.SuspendLayout();
			this.SuspendLayout();
			// 
			// groupBox2
			// 
			this.groupBox2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
			this.groupBox2.Controls.Add(this.ButtonClear);
			this.groupBox2.Controls.Add(this.ListViewCandidates);
			this.groupBox2.Controls.Add(this.ButtonSave);
			this.groupBox2.Controls.Add(this.ButtonLoad);
			this.groupBox2.Controls.Add(this.ButtonGetCandidates);
			this.groupBox2.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.groupBox2.Location = new System.Drawing.Point(12, 59);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(226, 566);
			this.groupBox2.TabIndex = 11;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "New Concept Candidates";
			// 
			// ButtonClear
			// 
			this.ButtonClear.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonClear.Location = new System.Drawing.Point(153, 48);
			this.ButtonClear.Name = "ButtonClear";
			this.ButtonClear.Size = new System.Drawing.Size(67, 28);
			this.ButtonClear.TabIndex = 29;
			this.ButtonClear.Text = "Clear";
			this.ButtonClear.UseVisualStyleBackColor = true;
			this.ButtonClear.Click += new System.EventHandler(this.ButtonClear_Click);
			// 
			// ListViewCandidates
			// 
			this.ListViewCandidates.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
			this.ListViewCandidates.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
			this.ListViewCandidates.ContextMenuStrip = this.contextMenuStrip1;
			this.ListViewCandidates.FullRowSelect = true;
			this.ListViewCandidates.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
			this.ListViewCandidates.HideSelection = false;
			this.ListViewCandidates.Location = new System.Drawing.Point(8, 82);
			this.ListViewCandidates.Name = "ListViewCandidates";
			this.ListViewCandidates.Size = new System.Drawing.Size(212, 474);
			this.ListViewCandidates.TabIndex = 28;
			this.ListViewCandidates.UseCompatibleStateImageBehavior = false;
			this.ListViewCandidates.View = System.Windows.Forms.View.Details;
			this.ListViewCandidates.SelectedIndexChanged += new System.EventHandler(this.ListViewCandidates_SelectedIndexChanged);
			// 
			// columnHeader1
			// 
			this.columnHeader1.Text = "Concept Label";
			this.columnHeader1.Width = 208;
			// 
			// contextMenuStrip1
			// 
			this.contextMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ignoreSelectedCandidatesToolStripMenuItem});
			this.contextMenuStrip1.Name = "contextMenuStrip1";
			this.contextMenuStrip1.Size = new System.Drawing.Size(215, 26);
			// 
			// ignoreSelectedCandidatesToolStripMenuItem
			// 
			this.ignoreSelectedCandidatesToolStripMenuItem.Name = "ignoreSelectedCandidatesToolStripMenuItem";
			this.ignoreSelectedCandidatesToolStripMenuItem.Size = new System.Drawing.Size(214, 22);
			this.ignoreSelectedCandidatesToolStripMenuItem.Text = "Ignore selected candidates";
			this.ignoreSelectedCandidatesToolStripMenuItem.Click += new System.EventHandler(this.ignoreSelectedCandidatesToolStripMenuItem_Click);
			// 
			// ButtonSave
			// 
			this.ButtonSave.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonSave.Location = new System.Drawing.Point(75, 48);
			this.ButtonSave.Name = "ButtonSave";
			this.ButtonSave.Size = new System.Drawing.Size(72, 28);
			this.ButtonSave.TabIndex = 27;
			this.ButtonSave.Text = "Save";
			this.ButtonSave.UseVisualStyleBackColor = true;
			this.ButtonSave.Click += new System.EventHandler(this.ButtonSave_Click);
			// 
			// ButtonLoad
			// 
			this.ButtonLoad.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonLoad.Location = new System.Drawing.Point(8, 48);
			this.ButtonLoad.Name = "ButtonLoad";
			this.ButtonLoad.Size = new System.Drawing.Size(61, 28);
			this.ButtonLoad.TabIndex = 26;
			this.ButtonLoad.Text = "Load";
			this.ButtonLoad.UseVisualStyleBackColor = true;
			this.ButtonLoad.Click += new System.EventHandler(this.ButtonLoad_Click);
			// 
			// ButtonGetCandidates
			// 
			this.ButtonGetCandidates.ContextMenuStrip = this.contextMenuStrip2;
			this.ButtonGetCandidates.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonGetCandidates.Location = new System.Drawing.Point(8, 17);
			this.ButtonGetCandidates.Name = "ButtonGetCandidates";
			this.ButtonGetCandidates.Size = new System.Drawing.Size(212, 28);
			this.ButtonGetCandidates.TabIndex = 25;
			this.ButtonGetCandidates.Text = "Compute Candidates";
			this.ButtonGetCandidates.UseVisualStyleBackColor = true;
			this.ButtonGetCandidates.Click += new System.EventHandler(this.ButtonGetCandidates_Click);
			// 
			// contextMenuStrip2
			// 
			this.contextMenuStrip2.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.extractCandidatesUsingClusteringToolStripMenuItem,
            this.extractCandidatesFromRecentPostsToolStripMenuItem,
            this.extractPhrasesToolStripMenuItem,
            this.extractCandidatesUsingNeutralContentMenuItem});
			this.contextMenuStrip2.Name = "contextMenuStrip2";
			this.contextMenuStrip2.Size = new System.Drawing.Size(286, 92);
			// 
			// extractCandidatesUsingClusteringToolStripMenuItem
			// 
			this.extractCandidatesUsingClusteringToolStripMenuItem.Name = "extractCandidatesUsingClusteringToolStripMenuItem";
			this.extractCandidatesUsingClusteringToolStripMenuItem.Size = new System.Drawing.Size(285, 22);
			this.extractCandidatesUsingClusteringToolStripMenuItem.Text = "Extract candidates using clustering";
			this.extractCandidatesUsingClusteringToolStripMenuItem.Click += new System.EventHandler(this.extractCandidatesUsingClusteringToolStripMenuItem_Click);
			// 
			// extractCandidatesFromRecentPostsToolStripMenuItem
			// 
			this.extractCandidatesFromRecentPostsToolStripMenuItem.Name = "extractCandidatesFromRecentPostsToolStripMenuItem";
			this.extractCandidatesFromRecentPostsToolStripMenuItem.Size = new System.Drawing.Size(285, 22);
			this.extractCandidatesFromRecentPostsToolStripMenuItem.Text = "Extract candidates from recent posts";
			this.extractCandidatesFromRecentPostsToolStripMenuItem.Click += new System.EventHandler(this.extractCandidatesFromRecentPostsToolStripMenuItem_Click);
			// 
			// extractPhrasesToolStripMenuItem
			// 
			this.extractPhrasesToolStripMenuItem.Name = "extractPhrasesToolStripMenuItem";
			this.extractPhrasesToolStripMenuItem.Size = new System.Drawing.Size(285, 22);
			this.extractPhrasesToolStripMenuItem.Text = "Extract phrases";
			this.extractPhrasesToolStripMenuItem.Click += new System.EventHandler(this.extractPhrasesToolStripMenuItem_Click);
			// 
			// extractCandidatesUsingNeutralContentMenuItem
			// 
			this.extractCandidatesUsingNeutralContentMenuItem.Name = "extractCandidatesUsingNeutralContentMenuItem";
			this.extractCandidatesUsingNeutralContentMenuItem.Size = new System.Drawing.Size(285, 22);
			this.extractCandidatesUsingNeutralContentMenuItem.Text = "Extract candidates using neutral content";
			this.extractCandidatesUsingNeutralContentMenuItem.Click += new System.EventHandler(this.extractCandidatesUsingNeutralContentMenuItem_Click);
			// 
			// statusStrip1
			// 
			this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.StatusText,
            this.StatusProgressBar});
			this.statusStrip1.Location = new System.Drawing.Point(0, 634);
			this.statusStrip1.Name = "statusStrip1";
			this.statusStrip1.Size = new System.Drawing.Size(792, 22);
			this.statusStrip1.TabIndex = 32;
			this.statusStrip1.Text = "statusStrip1";
			// 
			// StatusText
			// 
			this.StatusText.Name = "StatusText";
			this.StatusText.Size = new System.Drawing.Size(675, 17);
			this.StatusText.Spring = true;
			this.StatusText.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
			// 
			// StatusProgressBar
			// 
			this.StatusProgressBar.Name = "StatusProgressBar";
			this.StatusProgressBar.Size = new System.Drawing.Size(100, 16);
			// 
			// TextBoxDescription
			// 
			this.TextBoxDescription.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.TextBoxDescription.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.TextBoxDescription.Location = new System.Drawing.Point(10, 18);
			this.TextBoxDescription.Multiline = true;
			this.TextBoxDescription.Name = "TextBoxDescription";
			this.TextBoxDescription.Size = new System.Drawing.Size(492, 67);
			this.TextBoxDescription.TabIndex = 18;
			this.toolTip1.SetToolTip(this.TextBoxDescription, "Description of the concept. Important for automatically finding related concepts." +
        "");
			this.TextBoxDescription.TextChanged += new System.EventHandler(this.TextBoxDescription_TextChanged);
			// 
			// TextBoxLabels
			// 
			this.TextBoxLabels.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.TextBoxLabels.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.TextBoxLabels.Location = new System.Drawing.Point(73, 17);
			this.TextBoxLabels.Name = "TextBoxLabels";
			this.TextBoxLabels.Size = new System.Drawing.Size(451, 21);
			this.TextBoxLabels.TabIndex = 40;
			this.toolTip1.SetToolTip(this.TextBoxLabels, "Provide a comma separated list of labels for the concept");
			// 
			// groupBox1
			// 
			this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.groupBox1.Controls.Add(this.ButtonGroupAdd);
			this.groupBox1.Controls.Add(this.groupBox4);
			this.groupBox1.Controls.Add(this.groupBox3);
			this.groupBox1.Controls.Add(this.groupBox5);
			this.groupBox1.Controls.Add(this.TextBoxLabels);
			this.groupBox1.Controls.Add(this.label1);
			this.groupBox1.Controls.Add(this.ButtonAddSelectedConcept);
			this.groupBox1.Location = new System.Drawing.Point(244, 12);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(541, 613);
			this.groupBox1.TabIndex = 33;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Concept information";
			// 
			// ButtonGroupAdd
			// 
			this.ButtonGroupAdd.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.ButtonGroupAdd.Location = new System.Drawing.Point(16, 554);
			this.ButtonGroupAdd.Name = "ButtonGroupAdd";
			this.ButtonGroupAdd.Size = new System.Drawing.Size(201, 48);
			this.ButtonGroupAdd.TabIndex = 44;
			this.ButtonGroupAdd.Text = "Group add";
			this.ButtonGroupAdd.UseVisualStyleBackColor = true;
			this.ButtonGroupAdd.Click += new System.EventHandler(this.ButtonGroupAdd_Click);
			// 
			// groupBox4
			// 
			this.groupBox4.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.groupBox4.Controls.Add(this.ButtonSuggestConceptURI);
			this.groupBox4.Controls.Add(this.label2);
			this.groupBox4.Controls.Add(this.TextBoxConceptUri);
			this.groupBox4.Location = new System.Drawing.Point(16, 175);
			this.groupBox4.Name = "groupBox4";
			this.groupBox4.Size = new System.Drawing.Size(508, 82);
			this.groupBox4.TabIndex = 42;
			this.groupBox4.TabStop = false;
			this.groupBox4.Text = "Set Concept URI";
			// 
			// ButtonSuggestConceptURI
			// 
			this.ButtonSuggestConceptURI.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.ButtonSuggestConceptURI.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonSuggestConceptURI.Location = new System.Drawing.Point(346, 47);
			this.ButtonSuggestConceptURI.Name = "ButtonSuggestConceptURI";
			this.ButtonSuggestConceptURI.Size = new System.Drawing.Size(156, 28);
			this.ButtonSuggestConceptURI.TabIndex = 27;
			this.ButtonSuggestConceptURI.Text = "Suggest";
			this.ButtonSuggestConceptURI.UseVisualStyleBackColor = true;
			this.ButtonSuggestConceptURI.Click += new System.EventHandler(this.ButtonSuggestConceptURI_Click);
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.label2.Location = new System.Drawing.Point(8, 22);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(79, 15);
			this.label2.TabIndex = 26;
			this.label2.Text = "Concept URI:";
			// 
			// TextBoxConceptUri
			// 
			this.TextBoxConceptUri.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.TextBoxConceptUri.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.TextBoxConceptUri.Location = new System.Drawing.Point(93, 20);
			this.TextBoxConceptUri.Name = "TextBoxConceptUri";
			this.TextBoxConceptUri.Size = new System.Drawing.Size(409, 21);
			this.TextBoxConceptUri.TabIndex = 18;
			// 
			// groupBox3
			// 
			this.groupBox3.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.groupBox3.Controls.Add(this.ComboBoxRelations);
			this.groupBox3.Controls.Add(this.ButtonAutoAddRelatedConcepts);
			this.groupBox3.Controls.Add(this.ButtonManuallyAddRelatedConcepts);
			this.groupBox3.Controls.Add(this.ListViewRelated);
			this.groupBox3.Location = new System.Drawing.Point(16, 273);
			this.groupBox3.Name = "groupBox3";
			this.groupBox3.Size = new System.Drawing.Size(508, 275);
			this.groupBox3.TabIndex = 43;
			this.groupBox3.TabStop = false;
			this.groupBox3.Text = "Related Concepts";
			// 
			// ComboBoxRelations
			// 
			this.ComboBoxRelations.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.ComboBoxRelations.FormattingEnabled = true;
			this.ComboBoxRelations.Location = new System.Drawing.Point(130, 87);
			this.ComboBoxRelations.Name = "ComboBoxRelations";
			this.ComboBoxRelations.Size = new System.Drawing.Size(121, 23);
			this.ComboBoxRelations.TabIndex = 33;
			this.ComboBoxRelations.Visible = false;
			// 
			// ButtonAutoAddRelatedConcepts
			// 
			this.ButtonAutoAddRelatedConcepts.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.ButtonAutoAddRelatedConcepts.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonAutoAddRelatedConcepts.Location = new System.Drawing.Point(229, 241);
			this.ButtonAutoAddRelatedConcepts.Name = "ButtonAutoAddRelatedConcepts";
			this.ButtonAutoAddRelatedConcepts.Size = new System.Drawing.Size(159, 28);
			this.ButtonAutoAddRelatedConcepts.TabIndex = 24;
			this.ButtonAutoAddRelatedConcepts.Text = "Extract automatically";
			this.ButtonAutoAddRelatedConcepts.UseVisualStyleBackColor = true;
			this.ButtonAutoAddRelatedConcepts.Click += new System.EventHandler(this.ButtonAutoAddRelatedConcepts_Click);
			// 
			// ButtonManuallyAddRelatedConcepts
			// 
			this.ButtonManuallyAddRelatedConcepts.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.ButtonManuallyAddRelatedConcepts.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonManuallyAddRelatedConcepts.Location = new System.Drawing.Point(402, 241);
			this.ButtonManuallyAddRelatedConcepts.Name = "ButtonManuallyAddRelatedConcepts";
			this.ButtonManuallyAddRelatedConcepts.Size = new System.Drawing.Size(100, 28);
			this.ButtonManuallyAddRelatedConcepts.TabIndex = 23;
			this.ButtonManuallyAddRelatedConcepts.Text = "Add manually";
			this.ButtonManuallyAddRelatedConcepts.UseVisualStyleBackColor = true;
			this.ButtonManuallyAddRelatedConcepts.Click += new System.EventHandler(this.ButtonManuallyAddRelatedConcepts_Click);
			// 
			// ListViewRelated
			// 
			this.ListViewRelated.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.ListViewRelated.CheckBoxes = true;
			this.ListViewRelated.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.ColumnHeaderRemove,
            this.ColumnHeaderConceptName,
            this.ColumnHeaderRelation});
			this.ListViewRelated.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ListViewRelated.FullRowSelect = true;
			this.ListViewRelated.GridLines = true;
			this.ListViewRelated.Location = new System.Drawing.Point(10, 21);
			this.ListViewRelated.Name = "ListViewRelated";
			this.ListViewRelated.Size = new System.Drawing.Size(492, 214);
			this.ListViewRelated.TabIndex = 20;
			this.ListViewRelated.UseCompatibleStateImageBehavior = false;
			this.ListViewRelated.View = System.Windows.Forms.View.Details;
			// 
			// ColumnHeaderRemove
			// 
			this.ColumnHeaderRemove.Text = "X";
			this.ColumnHeaderRemove.Width = 27;
			// 
			// ColumnHeaderConceptName
			// 
			this.ColumnHeaderConceptName.Text = "Related concept";
			this.ColumnHeaderConceptName.Width = 259;
			// 
			// ColumnHeaderRelation
			// 
			this.ColumnHeaderRelation.Text = "Relation type";
			this.ColumnHeaderRelation.Width = 202;
			// 
			// groupBox5
			// 
			this.groupBox5.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.groupBox5.Controls.Add(this.ButtonLearnDescription);
			this.groupBox5.Controls.Add(this.TextBoxDescription);
			this.groupBox5.Location = new System.Drawing.Point(16, 47);
			this.groupBox5.Name = "groupBox5";
			this.groupBox5.Size = new System.Drawing.Size(508, 122);
			this.groupBox5.TabIndex = 41;
			this.groupBox5.TabStop = false;
			this.groupBox5.Text = "Description / Keywords";
			// 
			// ButtonLearnDescription
			// 
			this.ButtonLearnDescription.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.ButtonLearnDescription.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonLearnDescription.Location = new System.Drawing.Point(346, 88);
			this.ButtonLearnDescription.Name = "ButtonLearnDescription";
			this.ButtonLearnDescription.Size = new System.Drawing.Size(156, 28);
			this.ButtonLearnDescription.TabIndex = 19;
			this.ButtonLearnDescription.Text = "Learn from data";
			this.ButtonLearnDescription.UseVisualStyleBackColor = true;
			this.ButtonLearnDescription.Click += new System.EventHandler(this.ButtonLearnDescription_Click);
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.label1.Location = new System.Drawing.Point(20, 21);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(47, 15);
			this.label1.TabIndex = 39;
			this.label1.Text = "Labels:";
			// 
			// ButtonAddSelectedConcept
			// 
			this.ButtonAddSelectedConcept.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.ButtonAddSelectedConcept.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonAddSelectedConcept.Location = new System.Drawing.Point(277, 554);
			this.ButtonAddSelectedConcept.Name = "ButtonAddSelectedConcept";
			this.ButtonAddSelectedConcept.Size = new System.Drawing.Size(247, 48);
			this.ButtonAddSelectedConcept.TabIndex = 38;
			this.ButtonAddSelectedConcept.Text = "Add / update selected concept";
			this.ButtonAddSelectedConcept.UseVisualStyleBackColor = true;
			this.ButtonAddSelectedConcept.Click += new System.EventHandler(this.ButtonAddSelectedConcept_Click);
			// 
			// ButtonEditExisting
			// 
			this.ButtonEditExisting.ContextMenuStrip = this.contextMenuStrip2;
			this.ButtonEditExisting.Enabled = false;
			this.ButtonEditExisting.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.ButtonEditExisting.Location = new System.Drawing.Point(20, 20);
			this.ButtonEditExisting.Name = "ButtonEditExisting";
			this.ButtonEditExisting.Size = new System.Drawing.Size(212, 28);
			this.ButtonEditExisting.TabIndex = 34;
			this.ButtonEditExisting.Text = "Edit existing concept";
			this.ButtonEditExisting.UseVisualStyleBackColor = true;
			this.ButtonEditExisting.Click += new System.EventHandler(this.ButtonEditExisting_Click);
			// 
			// ProjectSpecificConcepts
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(792, 656);
			this.Controls.Add(this.ButtonEditExisting);
			this.Controls.Add(this.groupBox1);
			this.Controls.Add(this.statusStrip1);
			this.Controls.Add(this.groupBox2);
			this.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(238)));
			this.Margin = new System.Windows.Forms.Padding(4);
			this.Name = "ProjectSpecificConcepts";
			this.Text = "Adding concepts to the Annotation Ontology";
			this.groupBox2.ResumeLayout(false);
			this.contextMenuStrip1.ResumeLayout(false);
			this.contextMenuStrip2.ResumeLayout(false);
			this.statusStrip1.ResumeLayout(false);
			this.statusStrip1.PerformLayout();
			this.groupBox1.ResumeLayout(false);
			this.groupBox1.PerformLayout();
			this.groupBox4.ResumeLayout(false);
			this.groupBox4.PerformLayout();
			this.groupBox3.ResumeLayout(false);
			this.groupBox5.ResumeLayout(false);
			this.groupBox5.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.Button ButtonGetCandidates;
		private System.Windows.Forms.Button ButtonSave;
		private System.Windows.Forms.Button ButtonLoad;
		private System.Windows.Forms.ContextMenuStrip contextMenuStrip1;
		private System.Windows.Forms.ToolStripMenuItem ignoreSelectedCandidatesToolStripMenuItem;
		private System.Windows.Forms.ListView ListViewCandidates;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.StatusStrip statusStrip1;
		private System.Windows.Forms.ToolStripStatusLabel StatusText;
		private System.Windows.Forms.ToolStripProgressBar StatusProgressBar;
		private System.Windows.Forms.ContextMenuStrip contextMenuStrip2;
		private System.Windows.Forms.ToolStripMenuItem extractCandidatesUsingClusteringToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem extractCandidatesFromRecentPostsToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem extractPhrasesToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem extractCandidatesUsingNeutralContentMenuItem;
		private System.Windows.Forms.ToolTip toolTip1;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.GroupBox groupBox4;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox TextBoxConceptUri;
		private System.Windows.Forms.GroupBox groupBox3;
		private System.Windows.Forms.ComboBox ComboBoxRelations;
		private System.Windows.Forms.Button ButtonAutoAddRelatedConcepts;
		private System.Windows.Forms.Button ButtonManuallyAddRelatedConcepts;
		private ListViewWithCombo ListViewRelated;
		private System.Windows.Forms.ColumnHeader ColumnHeaderRemove;
		private System.Windows.Forms.ColumnHeader ColumnHeaderConceptName;
		private System.Windows.Forms.ColumnHeader ColumnHeaderRelation;
		private System.Windows.Forms.GroupBox groupBox5;
		private System.Windows.Forms.Button ButtonLearnDescription;
		private System.Windows.Forms.TextBox TextBoxDescription;
		private System.Windows.Forms.TextBox TextBoxLabels;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Button ButtonAddSelectedConcept;
		private System.Windows.Forms.Button ButtonEditExisting;
		private System.Windows.Forms.Button ButtonGroupAdd;
		private System.Windows.Forms.Button ButtonSuggestConceptURI;
		private System.Windows.Forms.Button ButtonClear;

	}
}

