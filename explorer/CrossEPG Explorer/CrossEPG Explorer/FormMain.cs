using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace CrossEPG_Explorer
{
    public partial class FormMain : Form
    {
        public FormMain()
        {
            InitializeComponent();
        }

        private void FormMain_Load(object sender, EventArgs e)
        {
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Id");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Start time");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Duration");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Title");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Description");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "MJD");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Title CRC");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Title seek");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Desc. CRC");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Desc. seek");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Genre ID");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Flags");
            listViewTitles.Columns.Add(new ColumnHeader().Name = "Revision");
            EpgDB db = new EpgDB();
            if (db.LoadLameDBFromFTP(new Uri("ftp://172.16.1.139/etc/enigma2/lamedb"), "root", ""))
                db.LoadCrossEPGDBFromFTP(new Uri("ftp://172.16.1.139/hdd/crossepg/crossepg.headers.db"), new Uri("ftp://172.16.1.139/hdd/crossepg/crossepg.descriptors.db"), "root", "");

            foreach (EpgChannelEntry entry in db.channels)
            {
                if (entry.titles.Count > 0)
                    listBoxChannels.Items.Add(entry);
            }
        }

        private void listBoxChannels_SelectedIndexChanged(object sender, EventArgs e)
        {
            EpgChannelEntry channel = (EpgChannelEntry)listBoxChannels.SelectedItem;
            listViewTitles.BeginUpdate();
            listViewTitles.Items.Clear();
            foreach (EpgTitleEntry entry in channel.titles)
            {
                ListViewItem item = new ListViewItem(entry.eventId.ToString());
                item.SubItems.Add(entry.startTime.ToString());
                item.SubItems.Add(entry.length.ToString());
                item.SubItems.Add(entry.description);
                item.SubItems.Add(entry.longDescription);
                item.SubItems.Add(entry.mjd.ToString());
                item.SubItems.Add(String.Format("0x{0:X}", entry.descriptionCrc));
                item.SubItems.Add(entry.descriptionSeek.ToString());
                item.SubItems.Add(String.Format("0x{0:X}", entry.longDescriptionCrc));
                item.SubItems.Add(entry.longDescriptionSeek.ToString());
                item.SubItems.Add(String.Format("0x{0:X}", entry.genreId));
                item.SubItems.Add(String.Format("0x{0:X}", entry.flags));
                item.SubItems.Add(entry.revision.ToString());
                listViewTitles.Items.Add(item);
            }

            for (int i=0; i<listViewTitles.Columns.Count; i++)
                listViewTitles.Columns[i].Width = -2;

            listViewTitles.Columns[4].Width = 300;

            listViewTitles.EndUpdate();
            textBoxDescription.Text = "";
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
    }
}
