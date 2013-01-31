using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.IO;
using System.Xml;
using System.Xml.Serialization;
using System.Diagnostics;

namespace KEUIApp
{
	public class ActiveMQSettings
	{
		public ActiveMQSettings()
		{
			LastInfo = null;
			InitValues();
		}

		private string _rootFolder = null;
		public ActiveMQSettings(string fileName)
		{
			_rootFolder = Path.GetDirectoryName(fileName);
			LastInfo = null;
			if (File.Exists(fileName))
			{
				try
				{
					using (XmlTextReader input = new XmlTextReader(fileName))
					{
						XmlSerializer serializer = new XmlSerializer(typeof(ActiveMQSettings));
						ActiveMQSettings sett = (ActiveMQSettings)serializer.Deserialize(input);
						InitValues(sett);
					}
				}
				catch (Exception ex) { LastInfo = "Error loading the ActiveMQ settings. Error: " + ex.Message; }
			}
			else
				InitValues();
		}

		private void InitValues(ActiveMQSettings sett = null)
		{
			StoreEvents = sett != null ? sett.StoreEvents : false;
			CustomEventStoreLocation = sett != null ? sett.CustomEventStoreLocation : "";

			BrokerUri = sett != null ? sett.BrokerUri : "tcp://localhost:61616/";
			TopicNameKEUIPublishNewConcept = sett != null && !string.IsNullOrEmpty(sett.TopicNameKEUIPublishNewConcept) ? sett.TopicNameKEUIPublishNewConcept : "ALERT.KEUI.ConceptNew";
			TopicNameKEUIPublishResponse = sett != null && !string.IsNullOrEmpty(sett.TopicNameKEUIPublishResponse) ? sett.TopicNameKEUIPublishResponse : "ALERT.KEUI.Response";
			
			TopicNameForumSensorPublishForumPost = sett != null && !string.IsNullOrEmpty(sett.TopicNameForumSensorPublishForumPost) ? sett.TopicNameForumSensorPublishForumPost : "ALERT.ForumSensor.ForumPost";

			TopicNameConceptNew = sett != null && !string.IsNullOrEmpty(sett.TopicNameConceptNew) ? sett.TopicNameConceptNew : "ALERT.*.ConceptNew";
			TopicNameKEUIRequest = sett != null && !string.IsNullOrEmpty(sett.TopicNameKEUIRequest) ? sett.TopicNameKEUIRequest : "ALERT.*.KEUIRequest";
			TopicNameForumPost = sett != null && !string.IsNullOrEmpty(sett.TopicNameForumPost) ? sett.TopicNameForumPost : "ALERT.Metadata.ForumPostNew.Stored";
			TopicNameIssueNew = sett != null && !string.IsNullOrEmpty(sett.TopicNameIssueNew) ? sett.TopicNameIssueNew : "ALERT.Metadata.IssueNew.Stored";
			TopicNameIssueUpdate = sett != null && !string.IsNullOrEmpty(sett.TopicNameIssueUpdate) ? sett.TopicNameIssueUpdate : "ALERT.Metadata.IssueUpdate.Stored";
			TopicNameEmail = sett != null && !string.IsNullOrEmpty(sett.TopicNameEmail) ? sett.TopicNameEmail : "ALERT.Metadata.MailNew.Stored";
			TopicNameCommit = sett != null && !string.IsNullOrEmpty(sett.TopicNameCommit) ? sett.TopicNameCommit : "ALERT.Metadata.CommitNew.Stored";
			TopicNameWikiPostNew = sett != null && !string.IsNullOrEmpty(sett.TopicNameWikiPostNew) ? sett.TopicNameWikiPostNew : "ALERT.Metadata.WikiPostNew.Stored";
			TopicNameWikiPostModified = sett != null && !string.IsNullOrEmpty(sett.TopicNameWikiPostModified) ? sett.TopicNameWikiPostModified : "ALERT.Metadata.WikiPostModified.Stored";
			TopicNameWikiPostDeleted = sett != null && !string.IsNullOrEmpty(sett.TopicNameWikiPostDeleted) ? sett.TopicNameWikiPostDeleted : "ALERT.Metadata.WikiPostDeleted.Stored";
			TopicNameTextToAnnotate = sett != null && !string.IsNullOrEmpty(sett.TopicNameTextToAnnotate) ? sett.TopicNameTextToAnnotate : "ALERT.*.TextToAnnotate";

			TopicNameIdentitySnapshot = sett != null && !string.IsNullOrEmpty(sett.TopicNameIdentitySnapshot) ? sett.TopicNameIdentitySnapshot : "ALERT.*.IdentitySnapshot";

			TopicNameCustomItemToIndex = sett != null && !string.IsNullOrEmpty(sett.TopicNameCustomItemToIndex) ? sett.TopicNameCustomItemToIndex : "ALERT.*.CustomItemToIndex";

			TopicNameAnnotexPublishConceptNew = sett != null && !string.IsNullOrEmpty(sett.TopicNameAnnotexPublishConceptNew) ? sett.TopicNameAnnotexPublishConceptNew : "ALERT.Annotex.ConceptNew";
			TopicNameKEUIPublishForumPost = sett != null && !string.IsNullOrEmpty(sett.TopicNameKEUIPublishForumPost) ? sett.TopicNameKEUIPublishForumPost : "ALERT.KEUI.ForumPostNew.Annotated";
			TopicNameKEUIPublishBugPost = sett != null && !string.IsNullOrEmpty(sett.TopicNameKEUIPublishBugPost) ? sett.TopicNameKEUIPublishBugPost : "ALERT.KEUI.IssueNew.Annotated";
			TopicNameKEUIPublishBugComment = sett != null && !string.IsNullOrEmpty(sett.TopicNameKEUIPublishBugComment) ? sett.TopicNameKEUIPublishBugComment : "ALERT.KEUI.IssueUpdate.Annotated";
			TopicNameKEUIPublishEmail = sett != null && !string.IsNullOrEmpty(sett.TopicNameKEUIPublishEmail) ? sett.TopicNameKEUIPublishEmail : "ALERT.KEUI.MailNew.Annotated";
			TopicNameKEUIPublishCommit = sett != null && !string.IsNullOrEmpty(sett.TopicNameKEUIPublishCommit) ? sett.TopicNameKEUIPublishCommit : "ALERT.KEUI.CommitNew.Annotated";
			TopicNameKEUIPublishTextToAnnotate = sett != null && !string.IsNullOrEmpty(sett.TopicNameKEUIPublishTextToAnnotate) ? sett.TopicNameKEUIPublishTextToAnnotate : "ALERT.KEUI.TextToAnnotate.Annotated";

			TopicNameKEUIPublishWikiPostNew = sett != null && !string.IsNullOrEmpty(sett.TopicNameKEUIPublishWikiPostNew) ? sett.TopicNameKEUIPublishWikiPostNew : "ALERT.KEUI.WikiPostNew.Annotated";
			TopicNameKEUIPublishWikiPostModified = sett != null && !string.IsNullOrEmpty(sett.TopicNameKEUIPublishWikiPostModified) ? sett.TopicNameKEUIPublishWikiPostModified : "ALERT.KEUI.WikiPostModified.Annotated";
		}

		[XmlIgnoreAttribute]
		public string LastInfo { get; private set; }

		// save the settings to the config file
		public void SaveSettings(string fileName)
		{
			LastInfo = null;
			try
			{
				using (FileStream fs = new FileStream(fileName, FileMode.Create, FileAccess.Write))
				{
					XmlSerializer serializer = new XmlSerializer(typeof(ActiveMQSettings));
					serializer.Serialize(fs, this);
				}
			}
			catch (Exception ex) { LastInfo = "Settings. SaveSettings failed." + ex.Message; }
		}

		public bool StoreEvents { get; set; }
		public string CustomEventStoreLocation { get; set; }

		private string _eventStoreLocation = null;
		[XmlIgnoreAttribute]
		public string EventStoreLocation
		{
			get
			{
				if (_eventStoreLocation == null) {
					if (!string.IsNullOrEmpty(CustomEventStoreLocation)) {
						Directory.CreateDirectory(CustomEventStoreLocation);
						_eventStoreLocation = CustomEventStoreLocation;
					}
					else if (_rootFolder != null) {
						string path = Path.Combine(_rootFolder, "StoredEvents");
						Directory.CreateDirectory(path);
						_eventStoreLocation = path;
					}
				}
				return _eventStoreLocation;
			}
		}

		public string BrokerUri { get; set; }

		// topic names
		public string TopicNameKEUIPublishNewConcept { get; set; }
		public string TopicNameKEUIPublishResponse { get; set; }

		public string TopicNameForumSensorPublishForumPost { get; set; }

		public string TopicNameConceptNew { get; set; }
		public string TopicNameKEUIRequest { get; set; }
		
		public string TopicNameForumPost { get; set; }
		public string TopicNameIssueNew { get; set; }
		public string TopicNameIssueUpdate { get; set; }
		public string TopicNameEmail { get; set; }
		public string TopicNameCommit { get; set; }
		public string TopicNameWikiPostNew { get; set; }
		public string TopicNameWikiPostModified { get; set; }
		public string TopicNameWikiPostDeleted { get; set; }
		public string TopicNameTextToAnnotate { get; set; }

		public string TopicNameIdentitySnapshot { get; set; }

		// the item that we just want to index. we don't need to send a forward event for this event
		// used by TextImporter to import data from custom sources
		public string TopicNameCustomItemToIndex { get; set; }

		public string TopicNameAnnotexPublishConceptNew { get; set; }
		public string TopicNameKEUIPublishForumPost { get; set; }
		public string TopicNameKEUIPublishBugPost { get; set; }
		public string TopicNameKEUIPublishBugComment { get; set; }
		public string TopicNameKEUIPublishEmail { get; set; }
		public string TopicNameKEUIPublishCommit { get; set; }

		public string TopicNameKEUIPublishWikiPostNew { get; set; }
		public string TopicNameKEUIPublishWikiPostModified { get; set; }
		
		public string TopicNameKEUIPublishTextToAnnotate { get; set; }
	}
}
