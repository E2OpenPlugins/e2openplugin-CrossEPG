using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using CrossEPG_Explorer.Properties;
using System.Diagnostics;

namespace CrossEPG_Explorer
{
    public partial class FormMain : Form
    {
        EpgDB db = new EpgDB();

        public FormMain()
        {
            if (Settings.Default.need_upgrade)
            {
                Settings.Default.Upgrade();
                Settings.Default.Reload();
                Settings.Default.need_upgrade = false;
                Settings.Default.Save();
            }

            InitializeComponent();
        }

        private void FormMain_Load(object sender, EventArgs e)
        {
            Text = String.Format("SIFTeam CrossEPG Explorer {0}.{1}.{2}", Application.ProductVersion.Split('.'));
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Id");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Start time");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Duration");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Title");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Description");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "MJD");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Title CRC");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Title seek");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Title length");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Desc. CRC");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Desc. seek");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Desc. length");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Genre ID");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Flags");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Revision");
        }

        private void listBoxChannels_SelectedIndexChanged(object sender, EventArgs e)
        {
            PopulatePane();
        }

        private void listViewTitles_SelectedIndexChanged(object sender, EventArgs e)
        {
            try
            {
                ListView.SelectedListViewItemCollection tmp = listViewTitles.SelectedItems;
                textBoxDescription.Text = tmp[0].SubItems[4].Text;
            }
            catch (Exception)
            {
            }
        }

        private void toolStripButtonConfiguration_Click(object sender, EventArgs e)
        {
            FormConfiguration cfg = new FormConfiguration();
            cfg.ShowDialog();
        }

        private void toolStripButtonLoadDecoder_Click(object sender, EventArgs e)
        {
            LoadFromDecoder();
        }

        private void toolStripSplitButtonLoadFile_ButtonClick(object sender, EventArgs e)
        {
            LoadOnlyCrossEPGFromFile();
        }

        private void useLamedbFromDecoderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            LoadOnlyCrossEPGFromFile();
        }

        private void useLamedbFromFileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            LoadAllFromFile();
        }

        private void LoadFromDecoder()
        {
            db = new EpgDB();
            if (db.LoadLameDBFromFTP(new Uri(String.Format("ftp://{0}/{1}", Settings.Default.device_ip, Settings.Default.lamedb_path)),
                Settings.Default.device_username,
                Settings.Default.device_password))
            {
                db.LoadCrossEPGDBFromFTP(new Uri(String.Format("ftp://{0}/{1}/crossepg.headers.db", Settings.Default.device_ip, Settings.Default.crossepg_path)),
                    new Uri(String.Format("ftp://{0}/{1}/crossepg.descriptors.db", Settings.Default.device_ip, Settings.Default.crossepg_path)),
                    Settings.Default.device_username,
                    Settings.Default.device_password);
            }
            RefreshUI();
        }

        private void LoadAllFromFile()
        {
            db = new EpgDB();
            OpenFileDialog dialog = new OpenFileDialog();

            dialog.Filter = "lamedb|lamedb|All files (*.*)|*.*";
            dialog.FilterIndex = 0;
            dialog.RestoreDirectory = true;

            if (dialog.ShowDialog() == DialogResult.OK)
            {
                db.LoadLameDBFromFile(dialog.FileName);

                FolderBrowserDialog folder = new FolderBrowserDialog();
                if (folder.ShowDialog() == DialogResult.OK)
                {
                    db.LoadCrossEPGDBFromDir(folder.SelectedPath);
                }
            }
            RefreshUI();
        }

        private void LoadOnlyCrossEPGFromFile()
        {
            db = new EpgDB();
            OpenFileDialog dialog = new OpenFileDialog();

            dialog.Filter = "lamedb|lamedb|All files (*.*)|*.*";
            dialog.FilterIndex = 0;
            dialog.RestoreDirectory = true;

            if (db.LoadLameDBFromFTP(new Uri(String.Format("ftp://{0}/{1}", Settings.Default.device_ip, Settings.Default.lamedb_path)),
                Settings.Default.device_username,
                Settings.Default.device_password))
            {
                FolderBrowserDialog folder = new FolderBrowserDialog();
                if (folder.ShowDialog() == DialogResult.OK)
                {
                    db.LoadCrossEPGDBFromDir(folder.SelectedPath);
                }
            }
            RefreshUI();
        }

        private void RefreshUI()
        {
            listBoxChannels.BeginUpdate();
            listBoxChannels.Items.Clear();
            foreach (EpgChannelEntry entry in db.channels)
            {
                if (entry.titles.Count > 0)
                    listBoxChannels.Items.Add(entry);
            }
            listBoxChannels.EndUpdate();

            listViewTitles.Items.Clear();
            UpdateStatusBar();
        }

        private void UpdateStatusBar()
        {
            if (toolStripButtonUTC.Checked == true)
            {
                toolStripStatusLabelCreation.Text = "Created: " + db.CreationTime.ToString();
                toolStripStatusLabelUpdate.Text = "Updated: " + db.UpdateTime.ToString();
            }
            else
            {
                toolStripStatusLabelCreation.Text = "Created: " + db.CreationTime.ToLocalTime().ToString();
                toolStripStatusLabelUpdate.Text = "Updated: " + db.UpdateTime.ToLocalTime().ToString();
            }
        }

        private void PopulatePane()
        {
            EpgChannelEntry channel = (EpgChannelEntry)listBoxChannels.SelectedItem;
            if (channel == null)
                return;

            listViewTitles.BeginUpdate();
            listViewTitles.Items.Clear();
            foreach (EpgTitleEntry entry in channel.titles)
            {
                ListViewItem item = new ListViewItem(entry.eventId.ToString());
                if (toolStripButtonUTC.Checked == true)
                    item.SubItems.Add(entry.startTime.ToString());
                else
                    item.SubItems.Add(entry.startTime.ToLocalTime().ToString());
                item.SubItems.Add(entry.length.ToString());
                item.SubItems.Add(entry.description);
                item.SubItems.Add(entry.longDescription);
                item.SubItems.Add(entry.mjd.ToString());
                item.SubItems.Add(String.Format("0x{0:X}", entry.descriptionCrc));
                item.SubItems.Add(entry.descriptionSeek.ToString());
                item.SubItems.Add(entry.descriptionLength.ToString());
                item.SubItems.Add(String.Format("0x{0:X}", entry.longDescriptionCrc));
                item.SubItems.Add(entry.longDescriptionSeek.ToString());
                item.SubItems.Add(entry.longDescriptionLength.ToString());
                item.SubItems.Add(String.Format("0x{0:X}", entry.genreId));
                item.SubItems.Add(String.Format("0x{0:X}", entry.flags));
                item.SubItems.Add(entry.revision.ToString());
                listViewTitles.Items.Add(item);
            }

            for (int i = 0; i < listViewTitles.Columns.Count; i++)
                listViewTitles.Columns[i].Width = -2;

            listViewTitles.Columns[4].Width = 300;

            listViewTitles.EndUpdate();
            textBoxDescription.Text = "";
        }

        private void toolStripButtonUTC_CheckStateChanged(object sender, EventArgs e)
        {
            toolStripButtonLocal.CheckStateChanged -= toolStripButtonLocal_CheckStateChanged;
            toolStripButtonLocal.Checked = !toolStripButtonUTC.Checked;
            toolStripButtonLocal.CheckStateChanged += toolStripButtonLocal_CheckStateChanged;

            PopulatePane();
            UpdateStatusBar();
        }

        private void toolStripButtonLocal_CheckStateChanged(object sender, EventArgs e)
        {
            toolStripButtonUTC.CheckStateChanged -= toolStripButtonUTC_CheckStateChanged;
            toolStripButtonUTC.Checked = !toolStripButtonLocal.Checked;
            toolStripButtonUTC.CheckStateChanged += toolStripButtonUTC_CheckStateChanged;

            PopulatePane();
            UpdateStatusBar();
        }
    }
}
