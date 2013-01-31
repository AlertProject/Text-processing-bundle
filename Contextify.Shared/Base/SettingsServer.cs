using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

using System.Reflection;
using System.IO;
using System.Xml.Serialization;

namespace ContextifyServer.Base
{
	public class SettingsServer
	{
		[XmlIgnoreAttribute]
		public static string ProfileNamePrefix { get { return ""; } }

		[XmlIgnoreAttribute]
		public static String AppName = "KEUIApp";
		[XmlIgnoreAttribute]
		public static String AppFolder = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), AppName);
		[XmlIgnoreAttribute]
		public static String LogFolder { get { return Path.Combine(AppFolder, "Log"); } }
		[XmlIgnoreAttribute]
		public static String StoreToFileFolder { get { return Path.Combine(AppFolder, "StoredCalls"); } }


		[XmlIgnoreAttribute]
		public String ProfileFolder { get { return Path.Combine(AppFolder, ProfileNamePrefix + ProfileFolderName); } }
		[XmlIgnoreAttribute]
		public String MinerDataFolder { get { return Path.Combine(ProfileFolder, "MinerData"); } }

		[XmlIgnoreAttribute]
		public String CompanyFolder { get { return Path.Combine(ProfileFolder, "Companies"); } }
		[XmlIgnoreAttribute]
		public String PeopleFolder { get { return Path.Combine(ProfileFolder, "People"); } }
		[XmlIgnoreAttribute]
		public String AccountsFolder { get { return Path.Combine(ProfileFolder, "Accounts"); } }

		[XmlIgnoreAttribute]
		public String ProfileFolderName { get; private set; }

		[XmlIgnoreAttribute]
		public String ProfileDisplayName { get; private set; }

		[XmlIgnoreAttribute]
		public String OutlookImagesFolder { get { return Path.Combine(PeopleFolder, "Outlook"); } }
		
		[XmlIgnoreAttribute]
		public String CustomImagesFolder { get { return Path.Combine(PeopleFolder, "CustomImages"); } }
		[XmlIgnoreAttribute]
		public String IgnoredLinksFile { get { return Path.Combine(ProfileFolder, "ignoredLinks.txt"); } }
		[XmlIgnoreAttribute]
		public String PublicMailDomainFile { get { return Path.Combine(ProfileFolder, "publicMailDomain.txt"); } }

		[XmlIgnoreAttribute]
		public static String ProfilesFile { get { return Path.Combine(AppFolder, "profiles.txt"); } }
		[XmlIgnoreAttribute]
		public String IndexingStatusFile { get { return Path.Combine(ProfileFolder, "indexingStatus.txt"); } }
		[XmlIgnoreAttribute]
		public String IndexingAccountsFile { get { return Path.Combine(ProfileFolder, "accounts.txt"); } }

		[XmlIgnoreAttribute]
		public String ExportedItemsRoot 
		{ 
			get { if (!string.IsNullOrWhiteSpace(IndexingCustomItemsRoot)) return IndexingCustomItemsRoot; return Path.Combine(ProfileFolder, "ExportedItems"); }
			set { IndexingCustomItemsRoot = value; }
		}

		public String GetModulePath()
		{
			return Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
		}

		private String SettingsFileName = "settings.xml";

		private object lockObj = new object();
		
		// options
		public bool IndexingIndexAttachments { get; set; }
		public bool IndexingUpdateDomainData { get; set; }
		public string IndexingCustomItemsRoot { get; set; }
		public bool IndexingSaveEmails { get; set; }
		public int IndexingSaveEmailsFormat { get; set; }
		public bool IndexingIndexFullEmailBody { get; set; }
		public bool IndexFacebookWallPosts { get; set; }
		public bool IndexFacebookInbox { get; set; }
		public bool IndexTwitterPosts { get; set; }

		public bool ContactsAutoCleanNames { get; set; }
		public bool ContactsAutoMergeContacts { get; set; }

		public string LastAttachmentSaveFolder { get; set; }
		public int LastUsedAccountId { get; set; }

		
		public bool StoreToFileAddItemInfo { get; set; }
		public bool StoreToFileAddItemContent { get; set; }
		public bool StoreToFileQueryRequest { get; set; }
		public bool StoreToFileQueryResponse { get; set; }

		public SettingsServer()
		{
			InitValues();
		}

		public SettingsServer(string profileFolderName, string profileDisplayName)
		{
			InitValues();

			ProfileFolderName = profileFolderName;
			ProfileDisplayName = profileDisplayName;

			try
			{
				if (!Directory.Exists(ProfileFolder))
					Directory.CreateDirectory(ProfileFolder);

				foreach (string path in new string[] { ProfileFolder, StoreToFileFolder })
				{
					try
					{
						if (!System.IO.Directory.Exists(path))
							System.IO.Directory.CreateDirectory(path);
					}
					catch (System.Exception ex) { GenLib.Log.LogService.LogException("Unable to create folder: " + path + "\nError: ", ex); }
				}

				if (File.Exists(Path.Combine(ProfileFolder, SettingsFileName)))
				{
					lock (lockObj)
					{
						using (System.Xml.XmlTextReader input = new System.Xml.XmlTextReader(Path.Combine(ProfileFolder, SettingsFileName)))
						{
							XmlSerializer serializer = new XmlSerializer(typeof(SettingsServer));
							SettingsServer sett = (SettingsServer)serializer.Deserialize(input);
							InitValues(sett);
						}
					}
				}
				else
				{
					// since this is the first time we are loading this profile (settings don't exist yet)
					// create a new version file so that we know what version of data is contained in this folder
					WriteVersionData(ProfileFolder);
					SaveSettings();
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("Settings. LoadSettings failed.", ex); }

			try { File.Create(Path.Combine(ProfileFolder, "ProfileName-" + profileDisplayName + ".txt")); }		// try creating the filename containing the profile name
			catch	// if there are invalid chars then create a file and inside of it put the profile name
			{
				try { File.WriteAllText(Path.Combine(ProfileFolder, "ProfileName.txt"), profileDisplayName); }
				catch { }
			}
		}

		
		
		public void InitValues(SettingsServer sett = null)
		{
			IndexingIndexAttachments = sett != null ? sett.IndexingIndexAttachments : false;		// by default we want to index the attachments. TODO: change this to false in release version		
			IndexingCustomItemsRoot = sett != null ? sett.IndexingCustomItemsRoot : null;						// did we set some special folder for storing items. if not, use the DataFolder + Items
			IndexingSaveEmails = sett != null ? sett.IndexingSaveEmails : false;
			IndexingSaveEmailsFormat = sett != null ? sett.IndexingSaveEmailsFormat : 9;			// by default we save emails in MSGUnicode format. A good option would also be the .eml format which is number 1024
			IndexingIndexFullEmailBody = sett != null ? sett.IndexingIndexFullEmailBody : false;	// by default we want to index only the last email body - remove the quoted text and previous replies
			IndexFacebookWallPosts = sett != null ? sett.IndexFacebookWallPosts : true;
			IndexFacebookInbox = sett != null ? sett.IndexFacebookInbox : true;
			IndexTwitterPosts = sett != null ? sett.IndexTwitterPosts : true;

			ContactsAutoCleanNames = sett != null ? sett.ContactsAutoCleanNames : true;
			ContactsAutoMergeContacts = sett != null ? sett.ContactsAutoMergeContacts : true;
			//OptionsSaveAttachments = sett != null ? sett.OptionsSaveAttachments : false;

			IndexingUpdateDomainData = sett != null ? sett.IndexingUpdateDomainData : true;

			LastAttachmentSaveFolder = sett != null ? sett.LastAttachmentSaveFolder : null;

			LastUsedAccountId = sett != null ? sett.LastUsedAccountId : 0;

			StoreToFileAddItemInfo = sett != null ? sett.StoreToFileAddItemInfo : false;
			StoreToFileAddItemContent = sett != null ? sett.StoreToFileAddItemContent : false;
			StoreToFileQueryRequest = sett != null ? sett.StoreToFileQueryRequest : false;
			StoreToFileQueryResponse = sett != null ? sett.StoreToFileQueryResponse : false;
		}

		// save the settings to the config file
		public void SaveSettings()
		{
			lock (lockObj)
			{
				FileStream fs = new FileStream(Path.Combine(ProfileFolder, SettingsFileName), FileMode.Create, FileAccess.Write);
				try
				{
					XmlSerializer serializer = new XmlSerializer(typeof(SettingsServer));
					serializer.Serialize(fs, this);
				}
				catch (Exception ex) { GenLib.Log.LogService.LogException("Settings. SaveSettings failed.", ex); }
				finally { if (fs != null) fs.Close(); } // If the FileStream is open, close it. 
			}
		}

		// get the full path to the folders that contain profiles
		public static List<string> GetProfiles()
		{
			try
			{
				if (!Directory.Exists(SettingsServer.AppFolder))
					Directory.CreateDirectory(SettingsServer.AppFolder);
				List<string> profiles = new List<string>(Directory.GetDirectories(SettingsServer.AppFolder, SettingsServer.ProfileNamePrefix + "*"));
				return profiles;
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("Settings. GetProfiles failed.", ex); }
			return new List<string>();
		}

		// get the names of the available profiles in the contextify folder
		public static List<string> GetProfileNames()
		{
			try
			{
				List<string> profiles = GetProfiles();
				for (int i = 0; i < profiles.Count; i++)
					profiles[i] = profiles[i].Substring(profiles[i].LastIndexOf(Path.DirectorySeparatorChar) + 1).Replace(SettingsServer.ProfileNamePrefix, "");
				return profiles;
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("Settings. GetProfileNames failed.", ex); }
			return new List<string>();
		}

		public static string GetProfilePath(string profileName)
		{
			return Path.Combine(AppFolder, ProfileNamePrefix + profileName);
		}

		public int GetNewAccountId()
		{
			LastUsedAccountId++;
			SaveSettings();
			return LastUsedAccountId;
		}

		public static VersionDataServer GetVersionData(string profileDir)
		{
			string path = Path.Combine(profileDir, "version.xml");
			if (File.Exists(path))
			{
				try
				{
					VersionDataServer versionData = null;
					using (System.Xml.XmlTextReader input = new System.Xml.XmlTextReader(path))
					{
						XmlSerializer serializer = new XmlSerializer(typeof(VersionDataServer));
						versionData = (VersionDataServer)serializer.Deserialize(input);
					}
					return versionData;
				}
				catch (System.Exception ex) { GenLib.Log.LogService.LogException("Failed to load settings from " + path, ex); }
			}
			return null;
		}

		public static void WriteVersionData(string profileDir)
		{
			VersionDataServer versionData = new VersionDataServer();
			string path = Path.Combine(profileDir, "version.xml");
			try
			{
				using (FileStream fs = new FileStream(path, FileMode.Create))
				{
					new XmlSerializer(typeof(VersionDataServer)).Serialize(fs, versionData);
				}
			}
			catch (System.Exception ex) { GenLib.Log.LogService.LogException("Saving version data failed.", ex); }
		}

	}

	// note: update the values of these variables each time we have some data that is incompatible with previous version of contextify
	[Serializable()]
	public class VersionDataServer
	{
		public string PeopleDataVersion = "3.0";
		public string MailDataVersion = "3.0";
	}
}
