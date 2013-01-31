using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
//using System.Drawing;
using System.Linq;
using System.Text;

using HtmlAgilityPack;
using Apache.NMS;
using Apache.NMS.ActiveMQ;
using System.IO;
using System.Reflection;
using LemmaSharp;
using TextLib.TextMining;
using System.Diagnostics;
using Contextify;
using ContextifyServer.Base;
using System.Text.RegularExpressions;
using GenLib.Text;
using Contextify.Shared.Types;
using Contextify.Shared.Base;

namespace KEUIApp
{
	public partial class KEUIDialog
	{			
		private TimeSpan _totalSpan = new TimeSpan();
		
		string _fileNameSettings = "KEUIApp.xml";
		string _fileNameActiveMQSettings = "ActiveMQSettings.xml";
		private string _defaultProfileName = "IndexingServiceProfile";
		private int _processedCount = 0;

		// publishers, subscribers
		public static string ClientId = "KEUI.client";
		public static string AQConsumerConceptNewId = "KEUI.ConceptNew.subscriber";
		public static string AQConsumerKEUIRequestId = "KEUI.Request.subscriber";
		public static string AQConsumerForumPostId = "KEUI.ForumPost.subscriber";
		public static string AQConsumerBugPostId = "KEUI.BugPost.subscriber";
		public static string AQConsumerBugCommentId = "Test.BugComment.subscriber";
		public static string AQConsumerEmailId = "KEUI.Email.subscriber";
		public static string AQConsumerSourceCodeId = "KEUI.SourceCode.subscriber";
		public static string AQConsumerWikiPostNewId = "KEUI.WikiPostNew.subscriber";
		public static string AQConsumerWikiPostModifiedId = "KEUI.WikiPostModified.subscriber";
		public static string AQConsumerWikiPostDeletedId = "KEUI.WikiPostDeleted.subscriber";
		public static string AQConsumerTextToAnnotateId = "KEUI.TextToAnnotate.subscriber";
		public static string AQConsumerCustomItemToIndexId = "KEUI.CustomItemToIndex.subscriber";
		public static string AQConsumerIdentityNewId = "KEUI.IdentityNew.subscriber";
		public static string AQConsumerIdentityUpdatedId = "KEUI.IdentityUpdated.subscriber";

		public ActiveMqHelper.TopicPublisher AQPublisherBugPost = null;
		public ActiveMqHelper.TopicPublisher AQPublisherEmail = null;
		public ActiveMqHelper.TopicPublisher AQPublisherSourceCode = null;
		public ActiveMqHelper.TopicPublisher AQPublisherWikiPost = null;
		public ActiveMqHelper.TopicPublisher AQPublisherTextToAnnotate = null;

		public ActiveMqHelper.TopicSubscriber AQSubscriberConceptNew = null;
		public ActiveMqHelper.TopicSubscriber AQSubscriberKEUIRequest = null;
		public ActiveMqHelper.TopicSubscriber AQSubscriberForumPost = null;
		public ActiveMqHelper.TopicSubscriber AQSubscriberIssueNew = null;
		public ActiveMqHelper.TopicSubscriber AQSubscriberIssueUpdate = null;
		public ActiveMqHelper.TopicSubscriber AQSubscriberEmail = null;
		public ActiveMqHelper.TopicSubscriber AQSubscriberSourceCode = null;
		public ActiveMqHelper.TopicSubscriber AQSubscriberWikiPostNew = null;
		public ActiveMqHelper.TopicSubscriber AQSubscriberWikiPostModified = null;
		public ActiveMqHelper.TopicSubscriber AQSubscriberWikiPostDeleted = null;
		public ActiveMqHelper.TopicSubscriber AQSubscriberTextToAnnotate = null;
		public ActiveMqHelper.TopicSubscriber AQSubscriberCustomItemToIndex = null;

		public ActiveMqHelper.TopicSubscriber AQSubscriberIdentitySnapshot = null;
		public ISession AQSession = null;
		private IConnection _connection = null;

		//private string _conceptsPath = @"c:\Development\VS Projects\Alert\WebopediaConcepts\concepts.txt";
		private KEUIApp.ActiveMQSettings _activeMQSettings = null;
		
		private KEUI _keui;

		public KEUIDialog(string DataFolderName = "KEUIApp")
		{
			SettingsServer.AppName = DataFolderName;
			SettingsServer.AppFolder = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), SettingsServer.AppName);
			_keui = new KEUI();
			_keui.SetAddEventHandler(AddEvent);
			_keui.IncreaseProcessedCountHandler = IncreaseProcessedCount;
			_keui.SetLastItemIdHandler = SetLastItemId;
			_keui.UpdateTotalTimeHandler = UpdateTotalTime;
		}

		public void Init()
		{
			// first start the logging service
			try
			{
				if (!Directory.Exists(SettingsServer.LogFolder))
					Directory.CreateDirectory(SettingsServer.LogFolder);
				GenLib.Log.LogService.AddLogger(new GenLib.Log.FileLogger(SettingsServer.LogFolder, "events.txt"));
			}
			catch (Exception ex) { AddEvent("Unable to start the logging service. Error: " + ex.Message); }
			GenLib.Log.LogService.LogInfo("KEUI App is starting.");
			LoadSettings();

			// load the indexing service first so that we can in the annotation service index concepts if necessary
			_keui.InitIndexingService(_defaultProfileName);
			_keui.InitAnnotationService(SettingsServer.AppFolder);
			
			_keui.IndexNewOntologyConcepts();
			//ProcessFile(@"e:\Data\ALERT-FTP\KDE\2012_10_23_11_40_37_906_5b276188-91c1-45b3-8afb-e89d80b7acbf_Response.xml", AQSubscriberIssueNew_OnMessageReceived);
			//ProcessFolder(@"e:\Data\ALERT-FTP\KDE\TestIssue", AQSubscriberIssueUpdate_OnMessageReceived);

			bool ok = InitActiveMQ();

			if (!ok)
				AddEvent("Unable to start Active MQ.");
			AddEvent("Storing of events: " + _activeMQSettings.StoreEvents);
		}

		public void Finish()
		{
			// close the MQ connection so that we don't get any new events
			if (_connection != null) {
				_connection.Close();
				_connection.Dispose();
			}
			_connection = null;

			// first close the profile. if something goes wrong with closing the active mq we at least don't have a problem with data
			_keui.DisposeIndexingService();
			_keui.SaveAnnotationOntology();
			SaveSettings();

			AddEvent("Everything is saved. You can close the console if Active MQ is not disposed properly.");
			AddEvent("Disposing active MQ...");
			DisposeActiveMQ();
			
			GenLib.Log.LogService.LogInfo("KEUI App closed.");
		}

		private void DisposeActiveMQ()
		{
			lock (_mqLock)
			{
				if (AQSubscriberConceptNew != null) AQSubscriberConceptNew.Dispose();
				if (AQSubscriberKEUIRequest != null) AQSubscriberKEUIRequest.Dispose();
				if (AQSubscriberForumPost != null) AQSubscriberForumPost.Dispose();
				if (AQSubscriberIssueNew != null) AQSubscriberIssueNew.Dispose();
				if (AQSubscriberIssueUpdate != null) AQSubscriberIssueUpdate.Dispose();
				if (AQSubscriberEmail != null) AQSubscriberEmail.Dispose();
				if (AQSubscriberSourceCode != null) AQSubscriberSourceCode.Dispose();
				if (AQSubscriberWikiPostNew != null) AQSubscriberWikiPostNew.Dispose();
				if (AQSubscriberWikiPostModified != null) AQSubscriberWikiPostModified.Dispose();
				if (AQSubscriberWikiPostDeleted != null) AQSubscriberWikiPostDeleted.Dispose();
				if (AQSubscriberTextToAnnotate != null) AQSubscriberTextToAnnotate.Dispose();
				if (AQSubscriberCustomItemToIndex != null) AQSubscriberCustomItemToIndex.Dispose();
				if (AQSubscriberIdentitySnapshot != null) AQSubscriberIdentitySnapshot.Dispose();
				if (AQSession != null) {
					AQSession.Close();
					AQSession.Dispose();
				}
				if (_connection != null)
					_connection.Dispose();
				_connection = null;
			}
		}

		private object _mqLock = new object();
		public bool InitActiveMQ()
		{	
			try
			{
				AddEvent("ActiveMQ ClientId: " + ClientId);
				AddEvent("Trying to connect to ActiveMQ at: " + _activeMQSettings.BrokerUri);
				
				IConnectionFactory factory = new ConnectionFactory(_activeMQSettings.BrokerUri, ClientId);
				_connection = factory.CreateConnection();
				AQSession = _connection.CreateSession();
				
				AddEvent("ActiveMQ session was successfully created");

				AQSubscriberConceptNew = new ActiveMqHelper.TopicSubscriber(AQSession, _activeMQSettings.TopicNameConceptNew);
				AQSubscriberKEUIRequest = new ActiveMqHelper.TopicSubscriber(AQSession, _activeMQSettings.TopicNameKEUIRequest);
				// TODO: uncomment the next line. only for debugging purposes we directly subscribe to the forum sensor
				AQSubscriberForumPost = new ActiveMqHelper.TopicSubscriber(AQSession, _activeMQSettings.TopicNameForumPost);
				//AQSubscriberForumPost = new ActiveMqHelper.TopicSubscriber(AQSession, _activeMQSettings.TopicNameForumSensorPublishForumPost);
				AQSubscriberIssueNew = new ActiveMqHelper.TopicSubscriber(AQSession, _activeMQSettings.TopicNameIssueNew);
				AQSubscriberIssueUpdate = new ActiveMqHelper.TopicSubscriber(AQSession, _activeMQSettings.TopicNameIssueUpdate);
				AQSubscriberEmail = new ActiveMqHelper.TopicSubscriber(AQSession, _activeMQSettings.TopicNameEmail);
				AQSubscriberSourceCode = new ActiveMqHelper.TopicSubscriber(AQSession, _activeMQSettings.TopicNameCommit);
				AQSubscriberWikiPostNew = new ActiveMqHelper.TopicSubscriber(AQSession, _activeMQSettings.TopicNameWikiPostNew);
				AQSubscriberWikiPostModified = new ActiveMqHelper.TopicSubscriber(AQSession, _activeMQSettings.TopicNameWikiPostModified);
				AQSubscriberWikiPostDeleted = new ActiveMqHelper.TopicSubscriber(AQSession, _activeMQSettings.TopicNameWikiPostDeleted);
				AQSubscriberTextToAnnotate = new ActiveMqHelper.TopicSubscriber(AQSession, _activeMQSettings.TopicNameTextToAnnotate);
				AQSubscriberCustomItemToIndex = new ActiveMqHelper.TopicSubscriber(AQSession, _activeMQSettings.TopicNameCustomItemToIndex);
				AQSubscriberIdentitySnapshot = new ActiveMqHelper.TopicSubscriber(AQSession, _activeMQSettings.TopicNameIdentitySnapshot);
					
				AQSubscriberConceptNew.OnMessageReceived += new ActiveMqHelper.TopicSubscriber.MessageReceivedDelegate(AQSubscriberConceptNew_OnMessageReceived);
				AQSubscriberKEUIRequest.OnMessageReceived += new ActiveMqHelper.TopicSubscriber.MessageReceivedDelegate(AQSubscriberKEUIRequest_OnMessageReceived);
				AQSubscriberForumPost.OnMessageReceived += new ActiveMqHelper.TopicSubscriber.MessageReceivedDelegate(AQSubscriberForumPost_OnMessageReceived);
				AQSubscriberIssueNew.OnMessageReceived += new ActiveMqHelper.TopicSubscriber.MessageReceivedDelegate(AQSubscriberIssueNew_OnMessageReceived);
				AQSubscriberIssueUpdate.OnMessageReceived += new ActiveMqHelper.TopicSubscriber.MessageReceivedDelegate(AQSubscriberIssueUpdate_OnMessageReceived);
				AQSubscriberEmail.OnMessageReceived += new ActiveMqHelper.TopicSubscriber.MessageReceivedDelegate(AQSubscriberEmail_OnMessageReceived);
				AQSubscriberSourceCode.OnMessageReceived += new ActiveMqHelper.TopicSubscriber.MessageReceivedDelegate(AQSubscriberCommit_OnMessageReceived);
				AQSubscriberWikiPostNew.OnMessageReceived += new ActiveMqHelper.TopicSubscriber.MessageReceivedDelegate(AQSubscriberWikiPostNew_OnMessageReceived);
				AQSubscriberWikiPostModified.OnMessageReceived += new ActiveMqHelper.TopicSubscriber.MessageReceivedDelegate(AQSubscriberWikiPostModified_OnMessageReceived);
				AQSubscriberWikiPostDeleted.OnMessageReceived += new ActiveMqHelper.TopicSubscriber.MessageReceivedDelegate(AQSubscriberWikiPostDeleted_OnMessageReceived);
				AQSubscriberTextToAnnotate.OnMessageReceived += new ActiveMqHelper.TopicSubscriber.MessageReceivedDelegate(AQSubscriberTextToAnnotate_OnMessageReceived);
				AQSubscriberCustomItemToIndex.OnMessageReceived += new ActiveMqHelper.TopicSubscriber.MessageReceivedDelegate(AQSubscriberCustomItemToIndex_OnMessageReceived);
				AQSubscriberIdentitySnapshot.OnMessageReceived += new ActiveMqHelper.TopicSubscriber.MessageReceivedDelegate(AQSubscriberIdentitySnapshot_OnMessageReceived);

				AQSubscriberConceptNew.Start(AQConsumerConceptNewId);
				AQSubscriberKEUIRequest.Start(AQConsumerKEUIRequestId);
				AQSubscriberForumPost.Start(AQConsumerForumPostId);
				AQSubscriberIssueNew.Start(AQConsumerBugPostId);
				AQSubscriberIssueUpdate.Start(AQConsumerBugCommentId);
				AQSubscriberEmail.Start(AQConsumerEmailId);
				AQSubscriberSourceCode.Start(AQConsumerSourceCodeId);
				AQSubscriberWikiPostNew.Start(AQConsumerWikiPostNewId);
				AQSubscriberWikiPostModified.Start(AQConsumerWikiPostModifiedId);
				AQSubscriberWikiPostDeleted.Start(AQConsumerWikiPostDeletedId);
				AQSubscriberTextToAnnotate.Start(AQConsumerTextToAnnotateId);
				AQSubscriberCustomItemToIndex.Start(AQConsumerCustomItemToIndexId);
				AQSubscriberIdentitySnapshot.Start(AQConsumerIdentityUpdatedId);

				// start the connection last. this makes sure that we don't receive any events before we finish starting all the subscribers
				_connection.Start();

				AddEvent("ActiveMQ topic subscribers were created...");
			}
			catch (Exception ex)
			{
				AddEvent("Failed to create ActiveMQ topic subscribers. Error message: " + ex.Message);
				GenLib.Log.LogService.LogException("KEUI.InitActiveMQ exception.", ex);
				return false;
			}
			return true;
		}

		public void ProcessFile(string fileName, Action<string> callback)
		{
			string content = File.ReadAllText(fileName);
			callback(content);
		}

		public void ProcessFolder(string folderName, Action<string> callback)
		{
			foreach (string file in Directory.EnumerateFiles(folderName)) {
				string content = File.ReadAllText(file);
				callback(content);
			}
		}

		public void UpdateEventMetaData(HtmlAgilityPack.XmlDocument xmlDoc, string senderEventName)
		{
			var eventNameNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:payload/ns1:meta/ns1:eventName");
			if (eventNameNode != null)
				eventNameNode.InnerHtml = senderEventName;
		}

		private void StoreEventData(string topic, string message, bool forceStoring = false, string namePrefix = "")
		{
			if (_activeMQSettings.StoreEvents || forceStoring == true)
			{
				if (_activeMQSettings.EventStoreLocation == null) {
					AddEvent("Unable to store event. The store location is invalid");
					return;
				}
				try {
					string path = Path.Combine(_activeMQSettings.EventStoreLocation, topic);
					path = path.Replace("*", "_");
					if (!Directory.Exists(path))
						Directory.CreateDirectory(path);
					string filename = namePrefix + DateTime.Now.Ticks.ToString() + ".xml";
					File.WriteAllText(Path.Combine(path, filename), message);
				}
				catch (Exception ex) {
					AddEvent("Unable to store event. Exception: " + ex.Message);
				}
			}
		}

		#region events
		void AQSubscriberConceptNew_OnMessageReceived(string message)
		{
			try
			{
				lock (_mqLock)
				{
					AddEvent("Adding new rdf data to the annotation ontology. " + GetDateStamp());
					StoreEventData(_activeMQSettings.TopicNameConceptNew, message);
					HtmlAgilityPack.XmlDocument xmlDoc = new HtmlAgilityPack.XmlDocument();
					xmlDoc.LoadXml(message);
					var newRDFDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData/ns1:newRDFData");
					if (newRDFDataNode == null) { AddEventAndLog("The event did not contain //ns1:eventData/s1:newRDFData node. Ignoring event"); return; }

					string rdfData = newRDFDataNode.InnerHtml;
					rdfData = rdfData.Trim();		// make sure the <xml > tag is in the first line
					if (!rdfData.StartsWith("<?xml version=\"1.0\"?>"))
						rdfData = "<?xml version=\"1.0\"?>" + Environment.NewLine + rdfData;
					_keui.AddNewRDFData(rdfData);

					// todo: maybe also use the new concepts to tag existing data
				}
			}
			catch (Exception ex)
			{
				AddEvent("Exception while processing new RDF data: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while processing new RDF data. message: " + message, ex);
			}
		}

		void AQSubscriberKEUIRequest_OnMessageReceived(string message)
		{
			try
			{
				lock (_mqLock)
				{
					AddEvent("Received KEUI Request " + GetDateStamp());
					StoreEventData(_activeMQSettings.TopicNameKEUIRequest, message);

					HtmlAgilityPack.XmlDocument xmlDoc = new HtmlAgilityPack.XmlDocument();
					xmlDoc.LoadXml(message);
					var eventDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData");
					var requestNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData/s1:keuiRequest");
					if (requestNode == null) { AddEventAndLog("The KEUI request event did not contain //ns1:eventData/s1:keuiRequest node. Ignoring request"); return; }

					string requestType = requestNode.SelectSingleNode("./s1:requestType").InnerText;
					string publishTopic = requestNode.SelectSingleNode("./s1:publishResponseOnTopic") != null ? requestNode.SelectSingleNode("./s1:publishResponseOnTopic").InnerText.DecodeXMLString() : _activeMQSettings.TopicNameKEUIPublishResponse;

					string keuiResponseData = null;
					
					requestType = requestType.Trim();	// make sure we ignore the newlines and spaces
					AddEvent("Request type: " + requestType);
					if (requestType == "GetSuggestions")
					{
						var queryNode = requestNode.SelectSingleNode("./s1:requestData/query");
						string prefix = queryNode.GetAttributeValue("prefix", "");
						string suggestionTypes = queryNode.GetAttributeValue("suggestionTypes", "");
						suggestionTypes = suggestionTypes.ToLower();

						if (string.IsNullOrEmpty(prefix))
						{
							AddEventAndLog("Unable to provide suggestions. No prefix was specified in the event.");
							return;
						}
						prefix = GenLib.Text.Text.ReplaceUnicodeCharsWithAscii(prefix);
						keuiResponseData = "<suggestions>";
						if (string.IsNullOrEmpty(suggestionTypes) || suggestionTypes.Contains("concepts"))
						{
							List<Tuple<string, string>> suggestionsConcepts = _keui.SuggestConceptsForPrefix(prefix);
							foreach (var sugg in suggestionsConcepts)
								keuiResponseData += String.Format("<concept label=\"{0}\" uri=\"{1}\" />", sugg.Item1.EncodeXMLString(), sugg.Item2.EncodeXMLString());
						}
						if (string.IsNullOrEmpty(suggestionTypes) || suggestionTypes.Contains("people"))
						{
							List<int> suggestionsPeople = _keui.SuggestPeopleForPrefix(prefix);
							foreach (var personId in suggestionsPeople)
							{
								var personInfo = _keui.PeopleData.GetPerson(personId);
								var accountIds = personInfo.GetAccountIds().ToList();
								string account = "";
								if (accountIds.Count > 0)
									account = _keui.PeopleData.GetAccount(accountIds[0]);
								string name = _keui.GetSuggestedPersonName(personInfo.PersonId);
								keuiResponseData += String.Format("<person name=\"{0}\" account=\"{1}\" uuid=\"{2}\" />", name.EncodeXMLString(), account.EncodeXMLString(), personInfo.CustomData.EncodeXMLString());
							}
						}
						if (string.IsNullOrEmpty(suggestionTypes) || suggestionTypes.Contains("products")) {
							foreach (var sugg in _keui.SuggestProductsForPrefix(prefix))
								keuiResponseData += String.Format("<product label=\"{0}\" uri=\"{1}\" />", sugg.Item1.EncodeXMLString(), sugg.Item2.EncodeXMLString());
						}
						if (string.IsNullOrEmpty(suggestionTypes) || suggestionTypes.Contains("issues")) {
							foreach (var sugg in _keui.SuggestIssuesForPrefix(prefix))
								keuiResponseData += String.Format("<issue label=\"{0}\" uri=\"{1}\" />", sugg.Item1.EncodeXMLString(), sugg.Item2.EncodeXMLString());
						}
						if (string.IsNullOrEmpty(suggestionTypes) || suggestionTypes.Contains("sources"))
						{
							foreach (int fileTag in _keui.SuggestFilesForPrefix(prefix)) {
								string fileName = _keui.MailData.GetTagName(fileTag);
								string fileUri = _keui.MailData.GetTagIdStr(fileTag);
								fileUri = _keui.MakeUriLong(fileUri);
								keuiResponseData += String.Format("<file name=\"{0}\" uri=\"{1}\" tooltip=\"{2}\" />", fileName.EncodeXMLString(), fileUri.EncodeXMLString(), fileName.EncodeXMLString());
							}
							foreach (int moduleTag in _keui.SuggestModulesForPrefix(prefix)) {
								int fileTag = _keui.MailData.GetParentTagId(moduleTag);
								string fileName = fileTag != TagInfoBase.InvalidTagId ? _keui.MailData.GetTagName(fileTag) : "";
								string moduleUri = _keui.MailData.GetTagIdStr(moduleTag);
								moduleUri = _keui.MakeUriLong(moduleUri);
								keuiResponseData += String.Format("<module name=\"{0}\" uri=\"{1}\" tooltip=\"{2}\" />", _keui.MailData.GetTagName(moduleTag).EncodeXMLString(), moduleUri.EncodeXMLString(), fileName.EncodeXMLString());
							}
							foreach (int methodTag in _keui.SuggestMethodsForPrefix(prefix)) {
								int moduleTag = _keui.MailData.GetParentTagId(methodTag);
								int fileTag = moduleTag != TagInfoBase.InvalidTagId ? _keui.MailData.GetParentTagId(moduleTag) : TagInfoBase.InvalidTagId;
								string fileName = fileTag != TagInfoBase.InvalidTagId ? _keui.MailData.GetTagName(fileTag) : "";
								string methodUri = _keui.MailData.GetTagIdStr(methodTag);
								methodUri = _keui.MakeUriLong(methodUri);
								keuiResponseData += String.Format("<method name=\"{0}\" uri=\"{1}\" tooltip=\"{2}\" />", _keui.MailData.GetTagName(methodTag).EncodeXMLString(), methodUri.EncodeXMLString(), fileName.EncodeXMLString());
							}
						}
						keuiResponseData += "</suggestions>";
					}
					else if (requestType == "GetAnnotationOntologyRDF")
					{
						var queryNode = requestNode.SelectSingleNode("./s1:requestData/query");
						bool includeComment = queryNode != null && (queryNode.GetAttributeValue("includeComment", 0) == 1);
						bool includeLinksTo = queryNode != null && (queryNode.GetAttributeValue("includeLinksTo", 0) == 1);
						keuiResponseData = _keui.GetAnnotationOntologyRDF(includeComment, includeLinksTo);
						AddEvent("Returning Annotation Ontology RDF. Data contains " + (int)(keuiResponseData.Length / 1024) + "KB");
					}
					else
						keuiResponseData = _keui.ProcessKEUIRequest(xmlDoc, requestType);

					if (keuiResponseData == null) return;

					// update the event name
					UpdateEventMetaData(xmlDoc, _activeMQSettings.TopicNameKEUIPublishResponse);

					// insert the return data 
					_keui.AddNewLines(eventDataNode, 8);
					eventDataNode.AppendChild(xmlDoc.CreateTextNode(String.Format("<s1:keuiResponse>{0}</s1:keuiResponse>", keuiResponseData)));
					
					string returnEvent = xmlDoc.DocumentNode.OuterHtml;
					
					// publish it
					StoreEventData(publishTopic, returnEvent);
					using (var publisher = new ActiveMqHelper.TopicPublisher(AQSession, publishTopic))
					{
						publisher.SendMessage(returnEvent);
					}
					AddEvent("KEUI successfully published KEUI Response event on topic " + publishTopic);
				}
			}
			//catch (Apache.NMS.ActiveMQ.IOException ioe)
			//{
			//    DisposeActiveMQ();
			//    InitActiveMQ();
			//}
			catch (Exception ex)
			{
				AddEvent("Exception while processing KEUI request: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while processing KEUI request. message: " + message, ex);
			}
		}

		void AQSubscriberWikiPostNew_OnMessageReceived(string message)
		{
			try
			{
				lock (_mqLock)
				{
					StoreEventData(_activeMQSettings.TopicNameWikiPostNew, message);
					IncreaseProcessedCount();

					HtmlAgilityPack.XmlDocument xmlDoc = new HtmlAgilityPack.XmlDocument();
					xmlDoc.LoadXml(message);

					_keui.AnnotateWikiPost(xmlDoc);
					int itemId = _keui.IndexWikiPost(xmlDoc);
					if (itemId == -1)
						StoreEventData(_activeMQSettings.TopicNameWikiPostNew, message, true, "invalid_");
					UpdateEventMetaData(xmlDoc, _activeMQSettings.TopicNameKEUIPublishWikiPostNew);

					// publish it
					string annotatedMessage = xmlDoc.DocumentNode.OuterHtml;
					StoreEventData(_activeMQSettings.TopicNameKEUIPublishWikiPostNew, annotatedMessage);
					using (var publisher = new ActiveMqHelper.TopicPublisher(AQSession, _activeMQSettings.TopicNameKEUIPublishWikiPostNew))
					{
						publisher.SendMessage(annotatedMessage);
					}
				}
			}
			catch (Exception ex)
			{
				AddEvent("Exception while processing new wiki post: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while processing new wiki post. message: " + message, ex);
			}
		}

		void AQSubscriberWikiPostModified_OnMessageReceived(string message)
		{
			try {
				lock (_mqLock) {
					StoreEventData(_activeMQSettings.TopicNameWikiPostModified, message);
					IncreaseProcessedCount();

					HtmlAgilityPack.XmlDocument xmlDoc = new HtmlAgilityPack.XmlDocument();
					xmlDoc.LoadXml(message);

					_keui.AnnotateWikiPost(xmlDoc);
					int itemId = _keui.IndexWikiPost(xmlDoc);
					if (itemId == -1)
						StoreEventData(_activeMQSettings.TopicNameWikiPostModified, message, true, "invalid_");
					UpdateEventMetaData(xmlDoc, _activeMQSettings.TopicNameKEUIPublishWikiPostModified);

					// publish it
					string annotatedMessage = xmlDoc.DocumentNode.OuterHtml;
					StoreEventData(_activeMQSettings.TopicNameKEUIPublishWikiPostModified, annotatedMessage);
					using (var publisher = new ActiveMqHelper.TopicPublisher(AQSession, _activeMQSettings.TopicNameKEUIPublishWikiPostModified)) {
						publisher.SendMessage(annotatedMessage);
					}
				}
			}
			catch (Exception ex) {
				AddEvent("Exception while processing wiki modified event: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while processing wiki modified event. message: " + message, ex);
			}
		}


		void AQSubscriberWikiPostDeleted_OnMessageReceived(string message)
		{
			try {
				lock (_mqLock) {
					StoreEventData(_activeMQSettings.TopicNameWikiPostDeleted, message);
					IncreaseProcessedCount();

					HtmlAgilityPack.XmlDocument xmlDoc = new HtmlAgilityPack.XmlDocument();
					xmlDoc.LoadXml(message);

					_keui.DeleteWikiPost(xmlDoc);
					StoreEventData(_activeMQSettings.TopicNameWikiPostDeleted, message, true, "invalid_");
				}
			}
			catch (Exception ex) {
				AddEvent("Exception while processing wiki deleted event: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while processing wiki deleted event. message: " + message, ex);
			}
		}

		void AQSubscriberCommit_OnMessageReceived(string message)
		{
			try
			{
				lock (_mqLock)
				{
					StoreEventData(_activeMQSettings.TopicNameCommit, message);
					IncreaseProcessedCount();

					HtmlAgilityPack.XmlDocument xmlDoc = new HtmlAgilityPack.XmlDocument();
					xmlDoc.LoadXml(message);

					_keui.AnnotateCommit(xmlDoc);
					int itemId = _keui.IndexCommit(xmlDoc);
					if (itemId == -1)
						StoreEventData(_activeMQSettings.TopicNameCommit, message, true, "invalid_");

					UpdateEventMetaData(xmlDoc, _activeMQSettings.TopicNameKEUIPublishCommit);

					// publish it
					string annotatedMessage = xmlDoc.DocumentNode.OuterHtml;
					StoreEventData(_activeMQSettings.TopicNameKEUIPublishCommit, annotatedMessage);
					using (var publisher = new ActiveMqHelper.TopicPublisher(AQSession, _activeMQSettings.TopicNameKEUIPublishCommit))
					{
						publisher.SendMessage(annotatedMessage);
					}
				}
			}
			catch (Exception ex)
			{
				AddEvent("Exception while processing new source code commit: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while processing new source code commit. message: " + message, ex);
			}
		}

		void AQSubscriberEmail_OnMessageReceived(string message)
		{
			try
			{
				lock (_mqLock)
				{
					StoreEventData(_activeMQSettings.TopicNameEmail, message);
					IncreaseProcessedCount();

					HtmlAgilityPack.XmlDocument xmlDoc = new HtmlAgilityPack.XmlDocument();
					xmlDoc.LoadXml(message);

					_keui.AnnotateEmail(xmlDoc);
					int itemId = _keui.IndexEmail(xmlDoc);
					if (itemId == -1)
						StoreEventData(_activeMQSettings.TopicNameEmail, message, true, "invalid_");
					UpdateEventMetaData(xmlDoc, _activeMQSettings.TopicNameKEUIPublishEmail);

					// publish it
					string annotatedMessage = xmlDoc.DocumentNode.OuterHtml;
					StoreEventData(_activeMQSettings.TopicNameKEUIPublishEmail, annotatedMessage);
					using (var publisher = new ActiveMqHelper.TopicPublisher(AQSession, _activeMQSettings.TopicNameKEUIPublishEmail))
					{
						publisher.SendMessage(annotatedMessage);
					}
				}
			}
			catch (Exception ex)
			{
				AddEvent("Exception while processing new email: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while processing new email. message: " + message, ex);
			}
		}

		void AQSubscriberIssueNew_OnMessageReceived(string message)
		{
			try
			{
				lock (_mqLock)
				{
					StoreEventData(_activeMQSettings.TopicNameIssueNew, message);
					IncreaseProcessedCount();
					
					HtmlAgilityPack.XmlDocument xmlDoc = new HtmlAgilityPack.XmlDocument();
					xmlDoc.LoadXml(message);

					_keui.AnnotateIssueNew(xmlDoc);
					bool ok = _keui.IndexIssueNew(xmlDoc);
					if (!ok)
						StoreEventData(_activeMQSettings.TopicNameIssueNew, message, true, "invalid_");
					UpdateEventMetaData(xmlDoc, _activeMQSettings.TopicNameKEUIPublishBugPost);

					// publish it
					string annotatedMessage = xmlDoc.DocumentNode.OuterHtml;
					StoreEventData(_activeMQSettings.TopicNameKEUIPublishBugPost, annotatedMessage);
					using (var publisher = new ActiveMqHelper.TopicPublisher(AQSession, _activeMQSettings.TopicNameKEUIPublishBugPost))
					{
						publisher.SendMessage(annotatedMessage);
					}
				}
			}
			catch (Exception ex)
			{
				AddEvent("Exception while annotating new bug post: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while processing new bug post. message: " + message, ex);
			}
		}

		void AQSubscriberIssueUpdate_OnMessageReceived(string message)
		{
			try
			{
				lock (_mqLock)
				{
					StoreEventData(_activeMQSettings.TopicNameIssueUpdate, message);
					IncreaseProcessedCount();
					
					HtmlAgilityPack.XmlDocument xmlDoc = new HtmlAgilityPack.XmlDocument();
					xmlDoc.LoadXml(message);

					_keui.AnnotateIssueUpdate(xmlDoc);
					bool ok = _keui.IndexIssueUpdate(xmlDoc);
					if (!ok)
						StoreEventData(_activeMQSettings.TopicNameIssueUpdate, message, true, "invalid_");
					UpdateEventMetaData(xmlDoc, _activeMQSettings.TopicNameKEUIPublishBugComment);

					// publish it
					string annotatedMessage = xmlDoc.DocumentNode.OuterHtml;
					StoreEventData(_activeMQSettings.TopicNameKEUIPublishBugComment, annotatedMessage);
					using (var publisher = new ActiveMqHelper.TopicPublisher(AQSession, _activeMQSettings.TopicNameKEUIPublishBugComment))
					{
						publisher.SendMessage(annotatedMessage);
					}
				}
			}
			catch (Exception ex)
			{
				AddEvent("Exception while annotating new bug comment: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while processing new bug comment. message: " + message, ex);
			}
		}

		void AQSubscriberForumPost_OnMessageReceived(string message)
		{
			try
			{
				lock (_mqLock)
				{
					StoreEventData(_activeMQSettings.TopicNameForumPost, message);
					IncreaseProcessedCount();

					HtmlAgilityPack.XmlDocument xmlDoc = new HtmlAgilityPack.XmlDocument();
					xmlDoc.LoadXml(message);

					_keui.AnnotateForumPost(xmlDoc);
					int itemId = _keui.IndexForumPost(xmlDoc);
					if (itemId == -1)
						StoreEventData(_activeMQSettings.TopicNameForumPost, message, true, "invalid_");
					UpdateEventMetaData(xmlDoc, _activeMQSettings.TopicNameKEUIPublishForumPost);
					
					// publish it
					string annotatedMessage = xmlDoc.DocumentNode.OuterHtml;
					StoreEventData(_activeMQSettings.TopicNameKEUIPublishForumPost, annotatedMessage);
					using (var publisher = new ActiveMqHelper.TopicPublisher(AQSession, _activeMQSettings.TopicNameKEUIPublishForumPost))
					{
						publisher.SendMessage(annotatedMessage);
					}
				}
			}
			catch (Exception ex)
			{
				AddEvent("Exception while processing new forum post: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while processing new forum post. message: " + message, ex);
			}
		}

		void AQSubscriberTextToAnnotate_OnMessageReceived(string message)
		{
			try
			{
				lock (_mqLock)
				{
					StoreEventData(_activeMQSettings.TopicNameTextToAnnotate, message);
					IncreaseProcessedCount();

					HtmlAgilityPack.XmlDocument xmlDoc = new HtmlAgilityPack.XmlDocument();
					xmlDoc.LoadXml(message);

					_keui.AnnotateTextToAnnotate(xmlDoc);
					UpdateEventMetaData(xmlDoc, _activeMQSettings.TopicNameKEUIPublishTextToAnnotate);

					// publish it
					string annotatedMessage = xmlDoc.DocumentNode.OuterHtml;
					StoreEventData(_activeMQSettings.TopicNameKEUIPublishTextToAnnotate, annotatedMessage);
					using (var AQPublisherTextToAnnotate = new ActiveMqHelper.TopicPublisher(AQSession, _activeMQSettings.TopicNameKEUIPublishTextToAnnotate))
					{
						AQPublisherTextToAnnotate.SendMessage(annotatedMessage);
					}
				}
			}
			catch (Exception ex)
			{
				AddEvent("Exception while annotating general text: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while annotating general text. message: " + message, ex);
			}
		}

		void AQSubscriberCustomItemToIndex_OnMessageReceived(string message)
		{
			try
			{
				lock (_mqLock)
				{
					StoreEventData(_activeMQSettings.TopicNameCustomItemToIndex, message);
					IncreaseProcessedCount();

					HtmlAgilityPack.XmlDocument xmlDoc = new HtmlAgilityPack.XmlDocument();
					xmlDoc.LoadXml(message);

					_keui.IndexCustomItemToIndex(xmlDoc);
				}
			}
			catch (Exception ex)
			{
				AddEvent("Exception while processing custom item to index: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while processing custom item to index. message: " + message, ex);
			}
		}

		void AQSubscriberIdentitySnapshot_OnMessageReceived(string message)
		{
			try {
				lock (_mqLock) {
					StoreEventData(_activeMQSettings.TopicNameIdentitySnapshot, message);
					IncreaseProcessedCount();

					HtmlAgilityPack.XmlDocument xmlDoc = new HtmlAgilityPack.XmlDocument();
					xmlDoc.LoadXml(message);

					_keui.IdentitySnapshot(xmlDoc);
				}
			}
			catch (Exception ex) {
				AddEvent("Exception while processing custom item to index: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while processing custom item to index. message: " + message, ex);
			}
		}
		#endregion
		
		#region settings
		public void LoadSettings()
		{
			//string currentFolder = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
			string currentFolder = SettingsServer.AppFolder;
			AddEvent("Loading settings from folder " + currentFolder);

			try
			{
				if (!Directory.Exists(Defaults.TemplateFolder))
					AddEvent(String.Format("The folder with templates ({0}) does not exist. Create it and put templates there.", Defaults.TemplateFolder));

				_activeMQSettings = new ActiveMQSettings(Path.Combine(currentFolder, _fileNameActiveMQSettings));

				if (!string.IsNullOrEmpty(_activeMQSettings.LastInfo))
					AddEvent("Error while loading Active MQ settings: " + _activeMQSettings.LastInfo);
				else
					AddEvent("Active MQ settings loaded successfully.");

				if (File.Exists(Path.Combine(currentFolder, _fileNameSettings)))
				{
					string text = File.ReadAllText(Path.Combine(currentFolder, _fileNameSettings));
					HtmlAgilityPack.XmlDocument xmlDoc = new HtmlAgilityPack.XmlDocument();
					xmlDoc.LoadXml(text);

					var indexingNode = xmlDoc.DocumentNode.SelectSingleNode("/settings/indexingService");
					if (indexingNode != null)
						_defaultProfileName = indexingNode.GetAttributeValue("profileName", "ProfileData");

					var pathNode = xmlDoc.DocumentNode.SelectSingleNode("/settings/paths");
					if (pathNode != null)
					{
						_keui.IgnoredConceptsFileName = pathNode.GetAttributeValue("ignoredConcepts", _keui.IgnoredConceptsFileName);
						if (!_keui.IgnoredConceptsFileName.Contains(Path.DirectorySeparatorChar))
							_keui.IgnoredConceptsFileName = Path.Combine(currentFolder, _keui.IgnoredConceptsFileName);
						_keui.CustomLemmasFileName = pathNode.GetAttributeValue("customLemmas", _keui.CustomLemmasFileName);
						if (!_keui.CustomLemmasFileName.Contains(Path.DirectorySeparatorChar))
							_keui.CustomLemmasFileName = Path.Combine(currentFolder, _keui.CustomLemmasFileName);
					}
				}

				_keui.LoadSettings(currentFolder);
			}
			catch (Exception ex)
			{
				GenLib.Log.LogService.LogException("KEUI.LoadSettings failed.", ex);
				AddEvent("KEUI.SaveSettings failed." + ex.Message);
			}
		}

		// save the settings to the config file
		public void SaveSettings()
		{
			//string currentFolder = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
			string folder = SettingsServer.AppFolder;

			AddEvent("Saving settings in folder " + folder);
			string settingsText = @"<?xml version=""1.0""?>" + Environment.NewLine + String.Format(@"<settings >") + Environment.NewLine;

			settingsText += "\t" + String.Format(@"<indexingService profileName=""{0}"" />", _defaultProfileName) + Environment.NewLine;
			settingsText += "\t" + String.Format(@"<paths ignoredConcepts=""{0}"" customLemmas=""{1}"" />", _keui.IgnoredConceptsFileName, _keui.CustomLemmasFileName) + Environment.NewLine;
			settingsText += "</settings>";

			try
			{
				// save active mq settings
				_activeMQSettings.SaveSettings(Path.Combine(folder, _fileNameActiveMQSettings));

				File.WriteAllText(Path.Combine(folder, _fileNameSettings), settingsText);
			}
			catch (Exception ex)
			{
				GenLib.Log.LogService.LogException("KEUI.SaveSettings failed.", ex);
				AddEvent("KEUI.SaveSettings failed." + ex.Message);
			}

			_keui.SaveSettings(folder);
		}
		#endregion


		#region form events
		public void UpdateTotalTime(TimeSpan timeSpan)
		{
		}

		public void SetLastItemId(int itemId)
		{
			if (itemId % 10 == 0)
				AddEvent("Last indexed item id: " + itemId);
		}

		public void IncreaseProcessedCount()
		{
			_processedCount++;
		}

		private void AddEventAndLog(string text)
		{
			AddEvent(text);
			GenLib.Log.LogService.LogInfo(text);
		}

		public void AddEvent(string text)
		{
			Console.WriteLine(DateTime.Now.ToLongTimeString() + " " + text);
		}

		public string GetDateStamp(bool inParenthesis = true)
		{
			if (inParenthesis)
				return "(" + DateTime.Now.ToShortDateString() + ")";
			else
				return DateTime.Now.ToShortDateString();
		}
		#endregion

		

	}
}
