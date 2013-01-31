using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using GenLib;
using System.Xml;
using System.Xml.XPath;
using System.IO;
using System.Xml.Serialization;

namespace Contextify.Shared.Types
{
	[Serializable()]
	public class ServerProfileSettings
	{
		[XmlIgnoreAttribute]
		public String ProfileFolderName { get; private set; }
		private String _customSettingsFileName = "serverProfileSettings.xml";
		private object _lockObj = new object();

		// miner specific settings
		public bool MinerUpdateThreadBow { get; set; }
		public bool MinerUpdateNGrams { get; set; }
		public bool MinerNGramsIgnoreSw { get; set; }
		public int MinerMxNGramLen { get; set; }
		public int MinerMxCachedNGrams { get; set; }
		public int MinerIndexCacheSizeMB { get; set; }
		public int MinerItemCacheSizeMB { get; set; }

		// suggestions
		public int GeneralMaxSuggPrefixLen { get; set; }
		public int GeneralMaxSuggItemCount { get; set; }

		[XmlIgnoreAttribute]
		private IEnumerable<string> _peopleRoleList = null;
		[XmlIgnoreAttribute]
		public IEnumerable<string> PeopleRoleList
		{
			get
			{
				if (_peopleRoleList == null)
					_peopleRoleList = PeopleRoles.Split(",".ToCharArray(), StringSplitOptions.RemoveEmptyEntries);
				return _peopleRoleList;
			}
		}
		public string PeopleRoles { get; set; }			// what are the roles that people can have in our items. for emails you can have sender, recipient, for documents author, etc...

		public ServerProfileSettings()
		{
			InitValues();
		}

		public ServerProfileSettings(string fullProfileFolderName)
		{
			InitValues();

			ProfileFolderName = fullProfileFolderName;
			
			try
			{
				if (!Directory.Exists(ProfileFolderName))
					return;

				if (File.Exists(Path.Combine(ProfileFolderName, _customSettingsFileName)))
				{
					lock (_lockObj)
					{
						using (System.Xml.XmlTextReader input = new System.Xml.XmlTextReader(Path.Combine(ProfileFolderName, _customSettingsFileName)))
						{
							XmlSerializer serializer = new XmlSerializer(typeof(ServerProfileSettings));
							ServerProfileSettings sett = (ServerProfileSettings)serializer.Deserialize(input);
							InitValues(sett);
						}
					}
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("CustomizableSettings. LoadSettings failed.", ex); }
		}

		public string SaveAsString()
		{
			StringWriter sw = new StringWriter();
			XmlSerializer serializer = new XmlSerializer(typeof(ServerProfileSettings));
			serializer.Serialize(sw, this);
			return sw.ToString();
		}

		public void LoadFromString(string serializedSettings)
		{
			try
			{
				using (StringReader sr = new StringReader(serializedSettings))
				using (System.Xml.XmlTextReader input = new System.Xml.XmlTextReader(sr))
				{
					XmlSerializer serializer = new XmlSerializer(typeof(ServerProfileSettings));
					ServerProfileSettings sett = (ServerProfileSettings)serializer.Deserialize(input);
					InitValues(sett);
				}
			}
			catch (Exception ex) { GenLib.Log.LogService.LogException("CustomizableSettings. LoadSettings failed.", ex); }
		}

		public void InitValues(ServerProfileSettings sett = null)
		{
			GeneralMaxSuggPrefixLen = sett != null ? sett.GeneralMaxSuggPrefixLen : 5;			// when building the suggestion dictionary, what is the maximum number of chars that we use to build the dictionary
			GeneralMaxSuggItemCount = sett != null ? sett.GeneralMaxSuggItemCount : 20;		// when building the suggestion dictionary, what is the maximum number of suggestions for each prefix

			MinerUpdateThreadBow = sett != null ? sett.MinerUpdateThreadBow : false;
			MinerUpdateNGrams = sett != null ? sett.MinerUpdateNGrams : true;
			MinerNGramsIgnoreSw = sett != null ? sett.MinerNGramsIgnoreSw : true;
			MinerMxNGramLen = sett != null ? sett.MinerMxNGramLen : 4;
			MinerMxCachedNGrams = sett != null ? sett.MinerMxCachedNGrams : 1000000;
			MinerIndexCacheSizeMB = sett != null ? sett.MinerIndexCacheSizeMB : 512;
			MinerItemCacheSizeMB = sett != null ? sett.MinerItemCacheSizeMB : 512;

			PeopleRoles = sett != null ? sett.PeopleRoles : "from,to,cc,bcc,author";
		}

		// save the settings to the config file
		public void SaveSettings()
		{
			lock (_lockObj)
			{
				FileStream fs = new FileStream(Path.Combine(ProfileFolderName, _customSettingsFileName), FileMode.Create, FileAccess.Write);
				try
				{
					XmlSerializer serializer = new XmlSerializer(typeof(ServerProfileSettings));
					serializer.Serialize(fs, this);
				}
				catch (Exception ex) { GenLib.Log.LogService.LogException("CustomizableSettings. SaveSettings failed.", ex); }
				finally { if (fs != null) fs.Close(); } // If the FileStream is open, close it. 
			}
		}
	}
}
