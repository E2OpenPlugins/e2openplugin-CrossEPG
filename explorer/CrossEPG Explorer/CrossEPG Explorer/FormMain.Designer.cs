namespace CrossEPG_Explorer
{
    partial class FormMain
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormMain));
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.listBoxChannels = new System.Windows.Forms.ListBox();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.listViewTitles = new System.Windows.Forms.ListView();
            this.textBoxDescription = new System.Windows.Forms.TextBox();
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.toolStripButtonLoadDecoder = new System.Windows.Forms.ToolStripButton();
            this.toolStripSplitButtonLoadFile = new System.Windows.Forms.ToolStripSplitButton();
            this.useLamedbFromDecoderToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.useLamedbFromFileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripButtonConfiguration = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripButtonUTC = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonLocal = new System.Windows.Forms.ToolStripButton();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabelCreation = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripStatusLabelUpdate = new System.Windows.Forms.ToolStripStatusLabel();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            this.toolStrip1.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(0, 25);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.listBoxChannels);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.tableLayoutPanel1);
            this.splitContainer1.Size = new System.Drawing.Size(817, 449);
            this.splitContainer1.SplitterDistance = 130;
            this.splitContainer1.TabIndex = 0;
            // 
            // listBoxChannels
            // 
            this.listBoxChannels.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listBoxChannels.FormattingEnabled = true;
            this.listBoxChannels.Location = new System.Drawing.Point(0, 0);
            this.listBoxChannels.Name = "listBoxChannels";
            this.listBoxChannels.Size = new System.Drawing.Size(130, 449);
            this.listBoxChannels.TabIndex = 0;
            this.listBoxChannels.SelectedIndexChanged += new System.EventHandler(this.listBoxChannels_SelectedIndexChanged);
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 1;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Controls.Add(this.listViewTitles, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.textBoxDescription, 0, 1);
            this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 2;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 100F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(683, 449);
            this.tableLayoutPanel1.TabIndex = 1;
            // 
            // listViewTitles
            // 
            this.listViewTitles.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listViewTitles.FullRowSelect = true;
            this.listViewTitles.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.listViewTitles.Location = new System.Drawing.Point(3, 3);
            this.listViewTitles.Name = "listViewTitles";
            this.listViewTitles.Size = new System.Drawing.Size(677, 343);
            this.listViewTitles.TabIndex = 0;
            this.listViewTitles.UseCompatibleStateImageBehavior = false;
            this.listViewTitles.View = System.Windows.Forms.View.Details;
            this.listViewTitles.SelectedIndexChanged += new System.EventHandler(this.listViewTitles_SelectedIndexChanged);
            // 
            // textBoxDescription
            // 
            this.textBoxDescription.Dock = System.Windows.Forms.DockStyle.Fill;
            this.textBoxDescription.Location = new System.Drawing.Point(3, 352);
            this.textBoxDescription.Multiline = true;
            this.textBoxDescription.Name = "textBoxDescription";
            this.textBoxDescription.ReadOnly = true;
            this.textBoxDescription.Size = new System.Drawing.Size(677, 94);
            this.textBoxDescription.TabIndex = 1;
            // 
            // toolStrip1
            // 
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripButtonLoadDecoder,
            this.toolStripSplitButtonLoadFile,
            this.toolStripSeparator1,
            this.toolStripButtonConfiguration,
            this.toolStripSeparator2,
            this.toolStripButtonUTC,
            this.toolStripButtonLocal});
            this.toolStrip1.Location = new System.Drawing.Point(0, 0);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(817, 25);
            this.toolStrip1.TabIndex = 1;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // toolStripButtonLoadDecoder
            // 
            this.toolStripButtonLoadDecoder.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonLoadDecoder.Image")));
            this.toolStripButtonLoadDecoder.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonLoadDecoder.Name = "toolStripButtonLoadDecoder";
            this.toolStripButtonLoadDecoder.Size = new System.Drawing.Size(128, 22);
            this.toolStripButtonLoadDecoder.Text = "Load from decoder";
            this.toolStripButtonLoadDecoder.Click += new System.EventHandler(this.toolStripButtonLoadDecoder_Click);
            // 
            // toolStripSplitButtonLoadFile
            // 
            this.toolStripSplitButtonLoadFile.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.useLamedbFromDecoderToolStripMenuItem,
            this.useLamedbFromFileToolStripMenuItem});
            this.toolStripSplitButtonLoadFile.Image = ((System.Drawing.Image)(resources.GetObject("toolStripSplitButtonLoadFile.Image")));
            this.toolStripSplitButtonLoadFile.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripSplitButtonLoadFile.Name = "toolStripSplitButtonLoadFile";
            this.toolStripSplitButtonLoadFile.Size = new System.Drawing.Size(113, 22);
            this.toolStripSplitButtonLoadFile.Text = "Load from file";
            this.toolStripSplitButtonLoadFile.ButtonClick += new System.EventHandler(this.toolStripSplitButtonLoadFile_ButtonClick);
            // 
            // useLamedbFromDecoderToolStripMenuItem
            // 
            this.useLamedbFromDecoderToolStripMenuItem.Name = "useLamedbFromDecoderToolStripMenuItem";
            this.useLamedbFromDecoderToolStripMenuItem.Size = new System.Drawing.Size(211, 22);
            this.useLamedbFromDecoderToolStripMenuItem.Text = "Use lamedb from decoder";
            this.useLamedbFromDecoderToolStripMenuItem.Click += new System.EventHandler(this.useLamedbFromDecoderToolStripMenuItem_Click);
            // 
            // useLamedbFromFileToolStripMenuItem
            // 
            this.useLamedbFromFileToolStripMenuItem.Name = "useLamedbFromFileToolStripMenuItem";
            this.useLamedbFromFileToolStripMenuItem.Size = new System.Drawing.Size(211, 22);
            this.useLamedbFromFileToolStripMenuItem.Text = "Use lamedb from file";
            this.useLamedbFromFileToolStripMenuItem.Click += new System.EventHandler(this.useLamedbFromFileToolStripMenuItem_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(6, 25);
            // 
            // toolStripButtonConfiguration
            // 
            this.toolStripButtonConfiguration.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonConfiguration.Image")));
            this.toolStripButtonConfiguration.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonConfiguration.Name = "toolStripButtonConfiguration";
            this.toolStripButtonConfiguration.Size = new System.Drawing.Size(101, 22);
            this.toolStripButtonConfiguration.Text = "Configuration";
            this.toolStripButtonConfiguration.Click += new System.EventHandler(this.toolStripButtonConfiguration_Click);
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(6, 25);
            // 
            // toolStripButtonUTC
            // 
            this.toolStripButtonUTC.Checked = true;
            this.toolStripButtonUTC.CheckOnClick = true;
            this.toolStripButtonUTC.CheckState = System.Windows.Forms.CheckState.Checked;
            this.toolStripButtonUTC.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonUTC.Image")));
            this.toolStripButtonUTC.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonUTC.Name = "toolStripButtonUTC";
            this.toolStripButtonUTC.Size = new System.Drawing.Size(72, 22);
            this.toolStripButtonUTC.Text = "Utc time";
            this.toolStripButtonUTC.CheckStateChanged += new System.EventHandler(this.toolStripButtonUTC_CheckStateChanged);
            // 
            // toolStripButtonLocal
            // 
            this.toolStripButtonLocal.CheckOnClick = true;
            this.toolStripButtonLocal.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonLocal.Image")));
            this.toolStripButtonLocal.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonLocal.Name = "toolStripButtonLocal";
            this.toolStripButtonLocal.Size = new System.Drawing.Size(82, 22);
            this.toolStripButtonLocal.Text = "Local time";
            this.toolStripButtonLocal.CheckStateChanged += new System.EventHandler(this.toolStripButtonLocal_CheckStateChanged);
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabelCreation,
            this.toolStripStatusLabelUpdate});
            this.statusStrip1.Location = new System.Drawing.Point(0, 474);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(817, 22);
            this.statusStrip1.TabIndex = 2;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // toolStripStatusLabelCreation
            // 
            this.toolStripStatusLabelCreation.Name = "toolStripStatusLabelCreation";
            this.toolStripStatusLabelCreation.Size = new System.Drawing.Size(0, 17);
            // 
            // toolStripStatusLabelUpdate
            // 
            this.toolStripStatusLabelUpdate.Name = "toolStripStatusLabelUpdate";
            this.toolStripStatusLabelUpdate.Size = new System.Drawing.Size(0, 17);
            // 
            // FormMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(817, 496);
            this.Controls.Add(this.splitContainer1);
            this.Controls.Add(this.toolStrip1);
            this.Controls.Add(this.statusStrip1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "FormMain";
            this.Text = "SIFTeam CrossEPG Explorer";
            this.WindowState = System.Windows.Forms.FormWindowState.Maximized;
            this.Load += new System.EventHandler(this.FormMain_Load);
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.ListBox listBoxChannels;
        private System.Windows.Forms.ListView listViewTitles;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.TextBox textBoxDescription;
        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripButton toolStripButtonLoadDecoder;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripButton toolStripButtonConfiguration;
        private System.Windows.Forms.ToolStripSplitButton toolStripSplitButtonLoadFile;
        private System.Windows.Forms.ToolStripMenuItem useLamedbFromDecoderToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem useLamedbFromFileToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.ToolStripButton toolStripButtonUTC;
        private System.Windows.Forms.ToolStripButton toolStripButtonLocal;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabelCreation;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabelUpdate;
    }
}

