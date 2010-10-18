using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Windows.Forms;
using System.Diagnostics;
using System.Globalization;

namespace CrossEPG_Explorer
{
    class EpgTitleEntry
    {
        public UInt16 eventId;
        public UInt16 mjd;
        public DateTime startTime;
        public UInt16 length;
        public byte genreId;
        public byte flags;
        public UInt32 descriptionCrc;
        public Int32 descriptionSeek;
        public UInt32 longDescriptionCrc;
        public Int32 longDescriptionSeek;
        public UInt16 descriptionLength;
        public UInt16 longDescriptionLength;
        public string iso639;
        public byte revision;
        public string description;
        public string longDescription;
    }

    class EpgChannelEntry
    {
        public List<EpgTitleEntry> titles = new List<EpgTitleEntry>();

        public string Name { get; set; }
        public UInt16 Nid { get; set; }
        public UInt16 Tsid { get; set; }
        public UInt16 Sid { get; set; }

        public EpgChannelEntry(string name, UInt16 nid, UInt16 tsid, UInt16 sid)
        {
            Name = name;
            Nid = nid;
            Tsid = tsid;
            Sid = sid;
        }

        public override string ToString()
        {
            return Name;
        }
    }

    class EpgChannelEntrySorter : IComparer<EpgChannelEntry>
    {
        public int Compare(EpgChannelEntry obj1, EpgChannelEntry obj2)
        {
            return obj1.Name.CompareTo(obj2.Name);
        }
    }

    class EpgDB
    {
        public List<EpgChannelEntry> channels = new List<EpgChannelEntry>();

        public EpgDB()
        {

        }

        public bool LoadLameDBFromFTP(Uri lamedbUri, string username, string password)
        {
            if (lamedbUri.Scheme != Uri.UriSchemeFtp)
                return false;

            WebClient request = new WebClient();

            request.Credentials = new NetworkCredential(username, password);
            try
            {
                byte[] newFileData = request.DownloadData(lamedbUri.ToString());
                string fileString = System.Text.Encoding.UTF8.GetString(newFileData);
                string[] rows = fileString.Split('\n');
                int count = 0;
                while (count < rows.Length)
                {
                    string[] tmp = rows[count].Split(':');
                    count++;

                    if (tmp.Length != 6)
                        continue;

                    try
                    {
                        if (rows[count].Trim().Length > 0)
                            channels.Add(new EpgChannelEntry(rows[count].Trim(),
                                                UInt16.Parse(tmp[3], NumberStyles.AllowHexSpecifier),
                                                UInt16.Parse(tmp[2], NumberStyles.AllowHexSpecifier),
                                                UInt16.Parse(tmp[0], NumberStyles.AllowHexSpecifier)));
                    }
                    catch (FormatException e)
                    {
                        Debug.WriteLine(e.ToString());
                    }

                    count++;
                }
            }
            catch (WebException e)
            {
                MessageBox.Show(e.ToString(), "Error reading lamedb", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            channels.Sort(new EpgChannelEntrySorter());
            return true;
        }

        public bool LoadCrossEPGDBFromFTP(Uri headersUri, Uri descriptorsUri, string username, string password)
        {
            byte[] headersData, descriptorsData;
            CultureInfo culture = CultureInfo.CreateSpecificCulture("en-EN");

            if (headersUri.Scheme != Uri.UriSchemeFtp || descriptorsUri.Scheme != Uri.UriSchemeFtp)
                return false;

            WebClient request = new WebClient();

            request.Credentials = new NetworkCredential(username, password);
            try
            {
                headersData = request.DownloadData(headersUri.ToString());
                descriptorsData = request.DownloadData(descriptorsUri.ToString());
            }
            catch (WebException e)
            {
                MessageBox.Show(e.ToString(), "Error reading crossepgdb", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }

            if (Encoding.ASCII.GetString(headersData, 0, 13) != "_xEPG_HEADERS")
            {
                MessageBox.Show("Invalid db header", "Error reading crossepgdb", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }

            if (headersData[13] != 0x07)
            {
                MessageBox.Show("Invalid db revision", "Error reading crossepgdb", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }
            CreationTime = new DateTime(1970, 1, 1, 0, 0, 0, 0);
            UpdateTime = new DateTime(1970, 1, 1, 0, 0, 0, 0);

            CreationTime = CreationTime.AddSeconds(BitConverter.ToUInt32(headersData, 14));
            UpdateTime = UpdateTime.AddSeconds(BitConverter.ToUInt32(headersData, 18));

            int channels_count = BitConverter.ToInt32(headersData, 22);
            int offset = 26;
            for (int i = 0; i < channels_count; i++)
            {
                UInt16 nid = BitConverter.ToUInt16(headersData, offset);
                UInt16 tsid = BitConverter.ToUInt16(headersData, offset+2);
                UInt16 sid = BitConverter.ToUInt16(headersData, offset+4);
                int titles_count = BitConverter.ToInt32(headersData, offset+6);
                offset += 10;
                EpgChannelEntry channel = channels.Find(delegate(EpgChannelEntry ch)
                {
                    if (ch.Nid == nid && ch.Tsid == tsid && ch.Sid == sid)
                        return true;
                    return false;
                });

                for (int j = 0; j < titles_count; j++)
                {
                    EpgTitleEntry title = new EpgTitleEntry();
                    title.eventId = BitConverter.ToUInt16(headersData, offset);
                    title.mjd = BitConverter.ToUInt16(headersData, offset + 2);
                    title.startTime = new DateTime(1970, 1, 1, 0, 0, 0, 0);
                    title.startTime = title.startTime.AddSeconds(BitConverter.ToUInt32(headersData, offset + 4));
                    title.length = BitConverter.ToUInt16(headersData, offset + 8);
                    title.genreId = headersData[10];
                    title.flags = headersData[11];
                    title.descriptionCrc = BitConverter.ToUInt32(headersData, offset + 12);
                    title.descriptionSeek = BitConverter.ToInt32(headersData, offset + 16);
                    title.longDescriptionCrc = BitConverter.ToUInt32(headersData, offset + 20);
                    title.longDescriptionSeek = BitConverter.ToInt32(headersData, offset + 24);
                    title.descriptionLength = BitConverter.ToUInt16(headersData, offset + 28);
                    title.longDescriptionLength = BitConverter.ToUInt16(headersData, offset + 30);
                    title.iso639 = Encoding.ASCII.GetString(headersData, offset + 32, 3);
                    title.revision = headersData[35];
                    offset += 36;

                    if ((title.flags & 0x01) == 0x01)
                    {
                        title.description = Encoding.UTF8.GetString(descriptorsData, title.descriptionSeek, title.descriptionLength);
                        title.longDescription = Encoding.UTF8.GetString(descriptorsData, title.longDescriptionSeek, title.longDescriptionLength);
                    }
                    else
                    {
                        if (culture.ThreeLetterISOLanguageName != title.iso639)
                        {
                            culture = CultureInfo.CreateSpecificCulture("en-EN");
                            CultureInfo[] cultures = CultureInfo.GetCultures(CultureTypes.AllCultures);
                            foreach (CultureInfo c in cultures)
                            {
                                if (c.ThreeLetterISOLanguageName == title.iso639)
                                {
                                    culture = c;
                                    break;
                                }
                            }
                        }
                        title.description = Encoding.GetEncoding(culture.TextInfo.ANSICodePage).GetString(descriptorsData, title.descriptionSeek, title.descriptionLength);
                        title.longDescription = Encoding.GetEncoding(culture.TextInfo.ANSICodePage).GetString(descriptorsData, title.longDescriptionSeek, title.longDescriptionLength);
                    }

                    if (channel != null)
                        channel.titles.Add(title);
                }
            }

            return true;
        }

        public DateTime CreationTime { get; set; }
        public DateTime UpdateTime { get; set; }
    }
}
