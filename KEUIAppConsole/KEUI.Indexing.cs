using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using LemmaSharp;
using TextLib.TextMining;
using System.Diagnostics;
using Contextify;
using ContextifyServer.Base;
using System.Text.RegularExpressions;
using GenLib.Text;
using Contextify.Shared.Types;
using Contextify.Shared.Base;
using HtmlAgilityPack;
using System.IO;
using SemWeb;
using Contextify.Util;

namespace KEUIApp
{
	public partial class KEUI
	{
		private int _tagIdEmails = TagInfoBase.InvalidTagId;
		private int _tagIdForums = TagInfoBase.InvalidTagId;
		private int _tagIdWikis = TagInfoBase.InvalidTagId;
		private int _tagIdIssues = TagInfoBase.InvalidTagId;
		private int _tagIdIssuesMeta = TagInfoBase.InvalidTagId;
		private int _tagIdIssuesResolution = TagInfoBase.InvalidTagId;
		private int _tagIdIssuesStatus = TagInfoBase.InvalidTagId;
		private int _tagIdCommits = TagInfoBase.InvalidTagId;
		private int _tagIdAnnotationConcepts = TagInfoBase.InvalidTagId;
		private int _tagIdSourceCode = TagInfoBase.InvalidTagId;
		private int _tagIdCustomSources = TagInfoBase.InvalidTagId;
		//private int _tagIdNonIssue = TagInfoBase.InvalidTagId;

		//private int _tagIdIssuesStatusOpen = TagInfoBase.InvalidTagId;
		//private int _tagIdIssuesStatusVerified = TagInfoBase.InvalidTagId;
		//private int _tagIdIssuesStatusAssigned = TagInfoBase.InvalidTagId;
		//private int _tagIdIssuesStatusResolved = TagInfoBase.InvalidTagId;
		//private int _tagIdIssuesStatusClosed = TagInfoBase.InvalidTagId;

		//private int _tagIdIssuesResolutionNone = TagInfoBase.InvalidTagId;
		//private int _tagIdIssuesResolutionFixed = TagInfoBase.InvalidTagId;
		//private int _tagIdIssuesResolutionWontFix = TagInfoBase.InvalidTagId;
		//private int _tagIdIssuesResolutionInvalid = TagInfoBase.InvalidTagId;
		//private int _tagIdIssuesResolutionDuplicate = TagInfoBase.InvalidTagId;
		//private int _tagIdIssuesResolutionWorksForMe = TagInfoBase.InvalidTagId;
		//private int _tagIdIssuesResolutionUnknown = TagInfoBase.InvalidTagId;

		public const string TagMetaMeta = "meta";
		public const string TagMetaIssueTracker = "tracker";
		public const string TagMetaProduct = "product";
		public const string TagMetaComponent = "component";
		public const string TagMetaStatus = "status";
		public const string TagMetaResolution = "resolution";
		public const string TagMetaIssue = "issue";
		public const string TagPrefixIssues = "issues:";
		public const string TagPrefixMeta = "meta:";
		public const string TagPrefixResolution = "resolution:";
		public const string TagPrefixStatus = "status:";

		// there could be a huge number of methods, files and modules so we shorten the uris for them to take less space
		public const string TagIdPrefixFileShort = "F#";
		public const string TagIdPrefixModuleShort = "O#";
		public const string TagIdPrefixMethodShort = "M#";
		public const string TagIdPrefixFileLong =   "http://www.alert-project.eu/ontologies/alert_scm.owl#";
		public const string TagIdPrefixModuleLong = "http://www.alert-project.eu/ontologies/alert_scm.owl#";
		public const string TagIdPrefixMethodLong = "http://www.alert-project.eu/ontologies/alert.owl#";

		public enum ItemType { Email = 10, ForumPost, BugDescription, BugComment, Commit, WikiPost, CustomItem, OntologyConcept }

		public MailData MailData { get; private set; }
		public PeopleData PeopleData { get; private set; }
		private SourcesOntology _so = null;

		private Dictionary<string, List<int>> _personPrefixToPersonId = new Dictionary<string, List<int>>();
		private const int _prefixPersonSuggestionCount = 50;		// max number of concepts that can be suggested for a given prefix
		private const int _prefixProductSuggestionCount = 50;		// max number of products that can be suggested for a given prefix
		private const int _prefixIssueSuggestionCount = 50;		// max number of issues that can be suggested for a given prefix
		private const int _prefixPersonMaxSize = 6;				// max length of the prefix considered for suggestion

		public KEUI()
		{
			MailData = null;
			PeopleData = null;

			QCond.AddCustomCondDelegate(QAccountNameCond.TagName, QAccountNameCond.CreateAccountNameCond);
			QCond.AddCustomCondDelegate(QEnrychableKeywordsCond.TagName, QEnrychableKeywordsCond.CreateEnrychableKeywordsCond);
			QCond.AddCustomCondDelegate(QConceptCond.TagName, QConceptCond.CreateConceptCond);
			QCond.AddCustomCondDelegate(QBugIdsCond.TagName, QBugIdsCond.CreateBugIdsCond);
			QCond.AddCustomCondDelegate(QTagIdStrCond.TagName, QTagIdStrCond.CreateTagIdStrCond);
			QCond.AddCustomCondDelegate(QPostTypesCond.TagName, QPostTypesCond.CreatePostTypesCond);
		}

		#region indexing functions
		private static Dictionary<string, Contextify.Shared.Base.Templates.ResultTypeEnum> resultTypeToEnum = new Dictionary<string, Contextify.Shared.Base.Templates.ResultTypeEnum>();

		public void InitIndexingService(string profileName)
		{
			try {
				// init the indexing service
				//resultTypeToEnum["attachmentData"] = Templates.ResultTypeEnum.attachmentData;
				resultTypeToEnum["itemData"] = Contextify.Shared.Base.Templates.ResultTypeEnum.itemData;
				resultTypeToEnum["keywordData"] = Contextify.Shared.Base.Templates.ResultTypeEnum.keywordData;
				resultTypeToEnum["peopleData"] = Contextify.Shared.Base.Templates.ResultTypeEnum.peopleData;
				resultTypeToEnum["timelineData"] = Contextify.Shared.Base.Templates.ResultTypeEnum.timelineData;

				// put initialization here
				MailData = new MailData(new SettingsServer(profileName, profileName));
				MailData.LogInfoHandler = AddEvent;
				PeopleData = MailData.PeopleData;

				AddEventAndLog("Loading profile data...");
				MailData.LoadData();
				AddEventAndLog("Profile loaded.");
				MailData.ProfileSettings.MinerUpdateNGrams = true;
				MailData.MinerUpdateSettings();

				_tagIdEmails = MailData.CreateTagIfNotExisting("Emails", "ROOT::Emails", MailData.TagIdRoot);
				_tagIdForums = MailData.CreateTagIfNotExisting("Forums", "ROOT::Forums", MailData.TagIdRoot);
				_tagIdWikis = MailData.CreateTagIfNotExisting("Wiki pages", "ROOT::Wiki pages", MailData.TagIdRoot);
				_tagIdIssues = MailData.CreateTagIfNotExisting("Issues", "ROOT::Issues", MailData.TagIdRoot);
				_tagIdIssuesMeta = MailData.CreateTagIfNotExisting("Issues meta", "ROOT::Issues meta", MailData.TagIdRoot);
				_tagIdIssuesStatus = MailData.CreateTagIfNotExisting("Issues status", "Issues meta::Issues status", _tagIdIssuesMeta);
				_tagIdIssuesResolution = MailData.CreateTagIfNotExisting("Issues resolution", "Issues meta::Issues resolution", _tagIdIssuesMeta);
				_tagIdCommits = MailData.CreateTagIfNotExisting("Source code commits", "ROOT::Source code commits", MailData.TagIdRoot);
				_tagIdAnnotationConcepts = MailData.CreateTagIfNotExisting("Annotation ontology", "ROOT::Annotation ontology", MailData.TagIdRoot);
				_tagIdSourceCode = MailData.CreateTagIfNotExisting("Information Sources ontology", "ROOT::Information Sources ontology", MailData.TagIdRoot);
				_tagIdCustomSources = MailData.CreateTagIfNotExisting("Custom sources", "ROOT::Custom sources", MailData.TagIdRoot);
				// this tag is assigned to all posts that are not issues. we need this so that we support filtering by status and resolution
				//_tagIdNonIssue = MailData.CreateTagIfNotExisting("NonIssue", "ROOT::NonIssue", MailData.TagIdRoot);		
				
				//_tagIdIssuesStatusOpen = MailData.CreateTagIfNotExisting("Open", TagPrefixStatus + "Open", _tagIdIssuesStatus, TagMetaStatus);
				//_tagIdIssuesStatusVerified = MailData.CreateTagIfNotExisting("Verified", TagPrefixStatus + "Verified", _tagIdIssuesStatus, TagMetaStatus);
				//_tagIdIssuesStatusAssigned = MailData.CreateTagIfNotExisting("Assigned", TagPrefixStatus + "Assigned", _tagIdIssuesStatus, TagMetaStatus);
				//_tagIdIssuesStatusResolved = MailData.CreateTagIfNotExisting("Resolved", TagPrefixStatus + "Resolved", _tagIdIssuesStatus, TagMetaStatus);
				//_tagIdIssuesStatusClosed = MailData.CreateTagIfNotExisting("Closed", TagPrefixStatus + "Closed", _tagIdIssuesStatus, TagMetaStatus);

				//_tagIdIssuesResolutionNone = MailData.CreateTagIfNotExisting("None", TagPrefixStatus + "None", _tagIdIssuesResolution, TagMetaResolution);
				//_tagIdIssuesResolutionFixed = MailData.CreateTagIfNotExisting("Fixed", TagPrefixStatus + "Fixed", _tagIdIssuesResolution, TagMetaResolution);
				//_tagIdIssuesResolutionWontFix = MailData.CreateTagIfNotExisting("WontFix", TagPrefixStatus + "WontFix", _tagIdIssuesResolution, TagMetaResolution);
				//_tagIdIssuesResolutionInvalid = MailData.CreateTagIfNotExisting("", TagPrefixStatus + "", _tagIdIssuesResolution, TagMetaResolution);
				//_tagIdIssuesResolutionDuplicate = MailData.CreateTagIfNotExisting("", TagPrefixStatus + "", _tagIdIssuesResolution, TagMetaResolution);
				//_tagIdIssuesResolutionWorksForMe = MailData.CreateTagIfNotExisting("", TagPrefixStatus + "", _tagIdIssuesResolution, TagMetaResolution);
				//_tagIdIssuesResolutionUnknown = MailData.CreateTagIfNotExisting("", TagPrefixStatus + "", _tagIdIssuesResolution, TagMetaResolution);

				_so = new SourcesOntology(MailData);
				_so.AddEventHandler = AddEvent;
				AddEventAndLog("Caching source code information...");
				_so.UpdateSourcesSuggestionsDict(_tagIdSourceCode);
				AddEventAndLog("Finished.");
				AddEventAndLog("Caching people information...");
				BuildSuggestionsPersonDict();
				AddEventAndLog("Finished");
			}
			catch (Exception ex) {
				AddEvent("Failed to create the indexing service. Error message: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while initializing indexing service", ex);
			}
		}

		public void IndexNewOntologyConcepts()
		{
			AddEvent("Adding new concepts to the ontology...");
			Trace.Assert(MailData != null, "MailData is not initialized yet");
			Trace.Assert(MailData.ProfileHandle != -1, "The profile handle for Miner is -1 (MailData.ProfileHandle == -1)");

			int addedCount = 0;
			foreach (KeyValuePair<string, List<string>> pair in _ao.ConceptUriToLabels) {
				string conceptUri = pair.Key;

				if (pair.Value == null || pair.Value.Count == 0)
					continue;
				string label = "";
				foreach (string l in pair.Value) {
					label = l;
					if (!string.IsNullOrEmpty(label))
						break;
				}
				if (string.IsNullOrEmpty(label))
					continue;

				string description = (_ao.ConceptUriToDescription.ContainsKey(conceptUri)) ? _ao.ConceptUriToDescription[conceptUri] : "";
				ESQLItemState state = MailData.SQLGetItemState(conceptUri);

				if (state != ESQLItemState.Indexed) {
					int conceptTagId = MailData.CreateTagIfNotExisting(label, conceptUri, _tagIdAnnotationConcepts);
					string metaData = Templates.BuildItemInfo(-1, DateTime.Now, "", "", conceptUri.EncodeXMLString(), "", tags: new[] { _tagIdAnnotationConcepts, conceptTagId }, indexText: false, itemType: (int)ItemType.OntologyConcept);
					AddItemStatus itemStatus = MailData.AddItem(metaData, description);
					if (itemStatus.ItemId == -1)
						AddEventAndLog(String.Format("Failed to index description for concept {0}: {1}", conceptUri, description));
					else
						addedCount++;
				}
			}
			if (addedCount > 0)
				AddEventAndLog(String.Format("Added {0} new concepts to the index.", addedCount));
		}


		public void DisposeIndexingService()
		{
			if (MailData != null) {
				MailData.Dispose();
				MailData = null;
			}
		}
		#endregion

		#region indexing server misc functions
		public IEnumerable<TagInfoBase> GetTagInfoList(int startIndex, int count)
		{
			DateTime start = DateTime.Now;
			var ret = MailData.GetTagInfoList();
			ret = ret.Skip(startIndex).Take(count);
			return ret;
		}

		public string ExecuteCommand(string commandData)
		{
			string ret = "";
			if (string.IsNullOrEmpty(commandData))
				return ret;
			DateTime start = DateTime.Now;
			MailData.LogInfo("ExecuteCommand command: " + commandData.Trim());
			HtmlAgilityPack.XmlDocument commandDoc = new HtmlAgilityPack.XmlDocument();
			commandDoc.LoadXml(commandData);

			var commandNode = commandDoc.DocumentNode.SelectSingleNode("/commandData");
			string target = commandNode.GetAttributeValue("target", null);
			string command = commandNode.GetAttributeValue("command", null);
			if (target == "PeopleData")
				ret = PeopleData.ExecuteCommand(command, commandNode);
			else if (target == "MailData")
				ret = MailData.ExecuteCommand(command, commandNode);
			else if (target == "Miner") {
				ret = MailData.MinerExecuteCommand(commandData);
				GenLib.Log.LogService.LogInfo("Profile. ExecuteCommand result: " + ret);
			}
			else
				MailData.LogInfo("Warning: Don't know how to process the command: " + commandData);
			MailData.LogInfo(String.Format("ExecuteCommand needed {0:N0} ms, returning {1:N0} bytes", (DateTime.Now - start).TotalMilliseconds, ret.Length));
			return ret;
		}

		public string ProcessQuery(string queryInfo)
		{
			DateTime start = DateTime.Now;
			QueryBase query = QueryBase.CreateQuery(queryInfo);
			String ret = MailData.MinerQuery(query);

			if (query is GeneralQuery && (query as GeneralQuery).QueryParams.KeywordSource == QKeywordSource.concepts) {
				XmlDocument retXml = new XmlDocument();
				retXml.LoadXml(ret);
				foreach (var kw in (retXml.DocumentNode.SelectNodes("//keywords/kw") ?? new HtmlNodeCollection(null)).Reverse()) {
					string tagIdStr = kw.GetAttributeValue("str", null);
					if (!string.IsNullOrEmpty(tagIdStr))
						kw.SetAttributeValue("str", MailData.GetTagName(tagIdStr) ?? "");
				}
				ret = retXml.DocumentNode.InnerHtml;
			}

			UpdateTotalTime(DateTime.Now - start);
			AddEvent(String.Format("Query result length = {0:N0} bytes, time needed is {1:N0} ms", ret != null ? ret.Length.ToString() : "null", (DateTime.Now - start).TotalMilliseconds));
			return ret;
		}

		public string SetData(string data)
		{
			string ret = "";
			DateTime start = DateTime.Now;
			MailData.LogInfo("SetData data: " + data);
			HtmlAgilityPack.XmlDocument dataDoc = new HtmlAgilityPack.XmlDocument();
			dataDoc.LoadXml(data);

			var dataNode = dataDoc.DocumentNode.SelectSingleNode("/data");
			string target = dataNode.GetAttributeValue("target", null);
			string content = dataNode.GetAttributeValue("content", null);
			if (target == "MailData")
				ret = MailData.SetData(content, dataNode);
			else if (target == "TagData")
				ret = MailData.SetTagData(content, dataNode);
			else
				MailData.LogInfo("Warning: Don't know how to use the data: " + data);
			MailData.LogInfo(String.Format("SetData needed {0:N0} ms", (DateTime.Now - start).TotalMilliseconds));
			return ret;
		}

		public string RequestData(string query)
		{
			string ret = "";
			DateTime start = DateTime.Now;
			HtmlAgilityPack.XmlDocument queryDoc = new HtmlAgilityPack.XmlDocument();
			queryDoc.LoadXml(query);
			var queryNode = queryDoc.DocumentNode.SelectSingleNode("/query");
			string source = queryNode.GetAttributeValue("source", null);
			string content = queryNode.GetAttributeValue("content", null);

			if (source == "PeopleData")
				ret = PeopleData.RequestData(content, queryNode);
			else if (source == "MailData")
				ret = MailData.RequestData(content, queryNode);
			else if (source == "TagData")
				ret = MailData.RequestTagData(content, queryNode);
			else
				MailData.LogInfo("Warning: Don't know how to respond to query: " + query);

			if (source != "MailData") {
				MailData.LogInfo("RequestData query: " + query);
				MailData.LogInfo(String.Format("RequestData needed {0:N0} ms, returning {1:N0} bytes", (DateTime.Now - start).TotalMilliseconds, ret.Length));
			}
			return ret;
		}

		public string ProcessKEUIRequest(HtmlAgilityPack.XmlDocument xmlDoc, string requestType)
		{
			Debug.Assert(MailData != null, "MailData is null. The indexing service has to be initialize before requests can be processed.");

			var requestNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData/s1:keuiRequest");
			string requestData = requestNode.SelectSingleNode("./s1:requestData").InnerHtml;
			//requestData = requestData.Replace("<s1:", "<").Replace("</s1:", "</");
			var sequenceNumberNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:head/ns1:sequencenumber");
			if (sequenceNumberNode != null)
				Trace.WriteLine("Received request with sequencenumber: " + sequenceNumberNode.InnerText);

			string returnData = "";
			if (requestType == "Query") {
				requestData = ChangeBugIdToThreadId(requestData);		// if we have specified a bug id this will fix it to threadId
				requestData = FixQueryArgs(requestData);
				returnData = ProcessQuery(requestData);
				returnData = returnData.Replace("<?xml version=\"1.0\" encoding=\"utf-8\"?>", "");
			}
			else if (requestType == "GetSimilarConcepts") {
				requestData = FixQueryArgs(requestData);
				returnData = ProcessQuery(requestData);
				HtmlAgilityPack.XmlDocument returnDataXml = new HtmlAgilityPack.XmlDocument();
				returnDataXml.LoadXml(returnData);
				foreach (HtmlNode itemNode in returnDataXml.DocumentNode.SelectNodes("//item") ?? new HtmlNodeCollection(null)) {
					string idStr = itemNode.GetAttributeValue("id", "");
					if (!string.IsNullOrEmpty(idStr)) {
						int id = int.Parse(idStr);
						string conceptUri = MailData.SQLGetEntryId(id);
						// add the concept uri
						if (!string.IsNullOrEmpty(conceptUri))
							itemNode.SetAttributeValue("uri", conceptUri.EncodeXMLString());
						// add the label
						if (_ao.ConceptUriToLabels.ContainsKey(conceptUri) && _ao.ConceptUriToLabels[conceptUri].Count > 0)
							itemNode.SetAttributeValue("label", _ao.ConceptUriToLabels[conceptUri][0].EncodeXMLString());
					}
				}
				returnData = returnDataXml.DocumentNode.InnerHtml;
				returnData = returnData.Replace("<?xml version=\"1.0\" encoding=\"utf-8\"?>", "");
			}
			//else if (requestType == "GetItemDataForSimilarThreads")
			//{
			//    var paramNode = requestNode.SelectSingleNode("./s1:requestData/query");
			//    int threadId = paramNode.GetAttributeValue("threadId", 0);
			//    int maxCount = paramNode.GetAttributeValue("maxCount", 20);
			//    int includeOnlyFirstInThread = paramNode.GetAttributeValue("includeOnlyFirstInThread", 1);	// do we only want to return itemData for bug description or also for comments?
			//    int offset = paramNode.GetAttributeValue("offset", 0);
			//    returnData = GetItemDataForSimilarThreads(threadId, maxCount, offset, includeOnlyFirstInThread == 1);
			//}
			//else if (requestType == "GetItemDataForSimilarItems")
			//{
			//    var paramNode = requestNode.SelectSingleNode("./s1:requestData/query");
			//    int threadId = paramNode.GetAttributeValue("itemId", 0);
			//    int maxCount = paramNode.GetAttributeValue("maxCount", 20);
			//    int offset = paramNode.GetAttributeValue("offset", 0);
			//    returnData = GetItemDataForSimilarItems(threadId, maxCount, offset);
			//}
			else if (requestType == "GetTagInfo") {
				var paramNode = requestNode.SelectSingleNode("./s1:requestData/params");
				int startIndex = paramNode.GetAttributeValue("startIndex", 0);
				int count = paramNode.GetAttributeValue("count", 1000);
				IEnumerable<TagInfoBase> tags = GetTagInfoList(startIndex, count);
				StringBuilder text = new StringBuilder();
				foreach (var tag in tags)
					text.Append(tag.ToXML());
				returnData = text.ToString();
			}
			else if (requestType == "ExecuteCommand")
				returnData = ExecuteCommand(requestData);
			else if (requestType == "SetData")
				returnData = SetData(requestData);
			else if (requestType == "RequestData")
				returnData = RequestData(requestData);
			else
				AddEvent("Ignoring unknown requestType:" + requestType);

			AddEvent("KEUI Indexing processed " + requestType + " event.");
			return returnData;
		}

		public int IndexWikiPost(HtmlAgilityPack.XmlDocument xmlDoc)
		{
			try {
				var eventDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData");

				var mdNode = eventDataNode.SelectSingleNode("./o:mdservice");
				string entryId = GetNodeInnerText(mdNode, "./o:wikiPageUri");		// todo: check if this is valid name!!
				if (string.IsNullOrEmpty(entryId)) {
					AddEventAndLog("The wikiPageUri was not specified for a new wiki post. Ignoring.");
					return -1;
				}

				// try to delete the item with this id from the index (in case we are calling this for wiki update event
				MailData.RemoveItems(new string[] { entryId });
				Debug.Assert(MailData.GetItemInfo(-1, entryId) == null, "The existing wiki page was not correctly deleted from the index");

				var wikiNode = eventDataNode.SelectSingleNode("./r2:wikiSensor");
				string subject = GetNodeInnerText(wikiNode, "./r2:title");
				string body = GetNodeInnerText(wikiNode, "./r2:rawText");
				string id = GetNodeInnerText(wikiNode, "./r2:id");
				
				string timeStr = GetNodeInnerText(wikiNode, "./r2:date");	//
				
				string author = GetNodeInnerText(wikiNode, "./r2:user/r2:name");
				string authorUri = GetNodeInnerText(mdNode, "./o:fromUri");		//
				
				DateTime time = DateTime.Now;
				if (!string.IsNullOrEmpty(timeStr) && !DateTime.TryParse(timeStr, out time))
					AddEventAndLog("Unable to successfully parse wiki page date " + timeStr);

				List<int> tags = new List<int> { _tagIdWikis };
				tags.AddRange(GetTagsForIssueReferences(xmlDoc.DocumentNode, "//s1:keui/s1:titleAnnotated"));
				tags.AddRange(GetTagsForIssueReferences(xmlDoc.DocumentNode, "//s1:keui/s1:rawTextAnnotated"));

				string concepts = GetConcepts(xmlDoc.DocumentNode, "//s1:keui/s1:titleConcepts");
				concepts += GetConcepts(xmlDoc.DocumentNode, "//s1:keui/s1:rawTextConcepts");
				
				string people = (String.IsNullOrEmpty(authorUri) || String.IsNullOrEmpty(author)) ? "" : GetXmlForPerson(author, authorUri, "author");

				string itemInfo = Templates.BuildItemInfo(-1, time, entryId.EncodeXMLString(), subject.EncodeXMLString(), entryId.EncodeXMLString(), people, tags: tags, concepts: concepts, itemType: (int)ItemType.WikiPost);
				AddItemStatus itemStatus = MailData.AddItem(itemInfo, body);
				if (itemStatus.ItemId == -1)
					AddEventAndLog("Failed to index wiki post. Error information: " + MailData.MinerGetLastInformation());
				SetLastItemId(itemStatus.ItemId);

				// save also the item id in the keui node
				HtmlNode keuiNode = eventDataNode.SelectSingleNode("./s1:keui");
				AddNewLines(keuiNode, 9);
				keuiNode.AppendChild(CreateNodeWithTextContent(xmlDoc, "s1:itemId", itemStatus.ItemId.ToString()));

				// add also thread id
				//keuiNode.AppendChild(CreateNodeWithTextContent(xmlDoc, "s1:threadId", itemStatus.ThreadId.ToString()));

				AddNewLines(keuiNode, 8);				// add newlines before </eventData>
				AddNewLines(eventDataNode, 7);

				// return the id of the last item
				return itemStatus.ItemId;
			}
			catch (Exception ex) {
				AddEvent("Exception while indexing new wiki post: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while indexing new wiki post.", ex);
			}
			return -1;
		}

		public void DeleteWikiPost(HtmlAgilityPack.XmlDocument xmlDoc)
		{
			try {
				var eventDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData");

				var mdNode = eventDataNode.SelectSingleNode("./o:mdservice");
				string entryId = GetNodeInnerText(mdNode, "./o:wikiUri");		// todo: check if this is valid name!!
				if (string.IsNullOrEmpty(entryId)) {
					AddEventAndLog("Unable to delete wiki page from the index. No entryId was specified.");
					return;
				}

				MailData.RemoveItems(new string[] { entryId });
			}
			catch (Exception ex) {
				AddEvent("Exception while deleting wiki post: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while deleting wiki post.", ex);
			}
		}

		public int IndexCommit(HtmlAgilityPack.XmlDocument xmlDoc)
		{
			try {
				var eventDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData");

				var mdNode = eventDataNode.SelectSingleNode("./o:mdservice");
				string entryId = GetNodeInnerText(mdNode, "./o:commitUri");
				if (entryId == null) {
					AddEventAndLog("Source code event did not contain commitUri. Skipping processing of the event.");
					return -1;
				}

				var kesiNode = eventDataNode.SelectSingleNode("./s:kesi");
				string body = GetNodeInnerText(kesiNode, "./s:commitMessageLog", "");
				string timeStr = GetNodeInnerText(kesiNode, "./s:commitDate", "");
				string subject = "";
				string thread = "";
				string author = GetNodeInnerText(kesiNode, "./s:commitCommitter/s:name");
				string authorUri = GetNodeInnerText(mdNode, "./o:commitCommitterUri");

				DateTime time = DateTime.Now;
				if (!DateTime.TryParse(timeStr, out time))
					AddEventAndLog("Unable to successfully parse date " + timeStr);

				string people = GetXmlForPerson(author, authorUri, "author");
				HashSet<int> tags = new HashSet<int> { _tagIdCommits };

				// extract the names of the files, modules and methods that were changed and add them as tags to the commit
				int fileNodeCount = (mdNode.SelectNodes("./o:commitFile") ?? new HtmlNodeCollection(null)).Count;
				for (int fileN = 0; fileN < fileNodeCount; fileN++) {
					var fileNode = mdNode.SelectSingleNode(String.Format("./o:commitFile[{0}]", fileN + 1));
					string fileUri = fileNode.SelectSingleNode("./o:fileUri").InnerText.DecodeXMLString();
					fileUri = MakeFileUriShort(fileUri);
					var fileNameNode = kesiNode.SelectSingleNode(String.Format("./s:commitFile[{0}]/s:fileName", fileN + 1));
					string fileName = fileNameNode != null ? fileNameNode.InnerText.DecodeXMLString() : fileUri;
					int fileTagId = _so.AddFileTagIfNotExisting(fileName, fileUri, _tagIdSourceCode);
					string cleanFileName = GetCleanFileName(fileName);
					tags.Add(fileTagId);

					int moduleNodeCount = (fileNode.SelectNodes("./o:fileModules") ?? new HtmlNodeCollection(null)).Count;
					for (int moduleN = 0; moduleN < moduleNodeCount; moduleN++) {
						var moduleNode = fileNode.SelectSingleNode(String.Format("./o:fileModules[{0}]", moduleN + 1));
						string moduleUri = moduleNode.SelectSingleNode("./o:moduleUri").InnerText.DecodeXMLString();
						moduleUri = MakeModuleUriShort(moduleUri);
						var moduleNameNode = kesiNode.SelectSingleNode(String.Format("./s:commitFile[{0}]/s:fileModules[{1}]/s:moduleName", fileN + 1, moduleN + 1));
						string moduleName = moduleNameNode != null ? moduleNameNode.InnerText.DecodeXMLString() : moduleUri;
						int moduleTagId = _so.AddModuleTagIfNotExisting(moduleName, moduleUri, fileTagId);
						string cleanModuleName = GetCleanClassName(moduleName);
						tags.Add(moduleTagId);

						int methodNodeCount = (moduleNode.SelectNodes("./o:moduleMethods/o:methodUri") ?? new HtmlNodeCollection(null)).Count;
						for (int methodN = 0; methodN < methodNodeCount; methodN++) {
							var methodUriNode = moduleNode.SelectSingleNode(String.Format("./o:moduleMethods[{0}]/o:methodUri", methodN + 1));
							string methodUri = methodUriNode.InnerText.DecodeXMLString();
							methodUri = MakeMethodUriShort(methodUri);
							var methodNameNode = kesiNode.SelectSingleNode(String.Format("./s:commitFile[{0}]/s:fileModules[{1}]/s:moduleMethods[{2}]/s:methodName", fileN + 1, moduleN + 1, methodN + 1));
							string methodName = methodNameNode != null ? methodNameNode.InnerText.DecodeXMLString() : methodUri;
							// add the method name to the dict so that it will be used in suggestions
							int methodTagId = _so.AddMethodTagIfNotExisting(methodName, methodUri, moduleTagId);
							tags.Add(methodTagId);
							int paramCount = GetMethodArgumentCount(methodName);
							string cleanMethodName = GetCleanMethodName(methodName);
							// add the method uri to dict so that the method will be used in annotations
							InformationSourcesAddFileAndMethod(cleanFileName, cleanMethodName, paramCount, methodUri);
							InformationSourcesAddClassAndMethod(cleanModuleName, cleanMethodName, paramCount, methodUri);
						}
					}
				}

				if (tags.Contains(TagInfoBase.InvalidTagId)) {
					AddEventAndLog("Warning. When processing a commit, an invalid tag id was added as a tag. This should not happen.");
					tags.Remove(TagInfoBase.InvalidTagId);
				}

				string concepts = GetConcepts(xmlDoc.DocumentNode, "//s1:keui/s1:commitMessageLogConcepts");

				string itemInfo = Templates.BuildItemInfo(-1, time, thread.EncodeXMLString(), subject.EncodeXMLString(), entryId.EncodeXMLString(), people, tags: tags.ToList(), concepts: concepts, itemType: (int)ItemType.Commit);
				AddItemStatus itemStatus = MailData.AddItem(itemInfo, body);
				if (itemStatus.ItemId == -1)
					AddEventAndLog("Failed to index item. Error information: " + MailData.MinerGetLastInformation());
				SetLastItemId(itemStatus.ItemId);

				// save also the item id in the keui node
				HtmlNode keuiNode = eventDataNode.SelectSingleNode("./s1:keui");
				AddNewLines(keuiNode, 9);
				keuiNode.AppendChild(CreateNodeWithTextContent(xmlDoc, "s1:itemId", itemStatus.ItemId.ToString()));

				AddNewLines(keuiNode, 8);				// add newlines before </eventData>
				AddNewLines(eventDataNode, 7);

				// return the id of the last item
				return itemStatus.ItemId;
			}
			catch (Exception ex) {
				AddEvent("Exception while indexing source code: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while indexing source code.", ex);
			}
			return -1;
		}

		public int IndexEmail(HtmlAgilityPack.XmlDocument xmlDoc)
		{
			try {
				var eventDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData");

				var mdNode = eventDataNode.SelectSingleNode("./o:mdservice");
				string entryId = GetNodeInnerText(mdNode, "./o:emailUri");
				if (string.IsNullOrEmpty(entryId)) {
					AddEventAndLog("The entryId was not specified for a new email. Ignoring.");
					return -1;
				}

				var mailNode = eventDataNode.SelectSingleNode("./r1:mlsensor");
				string body = GetNodeInnerText(mailNode, "./r1:content");
				string timeStr = GetNodeInnerText(mailNode, "./r1:date");
				string subject = GetNodeInnerText(mailNode, "./r1:subject");
				string author = GetNodeInnerText(mailNode, "./r1:from");
				string authorUri = GetNodeInnerText(mdNode, "./o:fromUri");
				string thread = GenLib.Email.GetCleanSubject(subject);

				DateTime time = DateTime.Now;
				if (!DateTime.TryParse(timeStr, out time))
					AddEventAndLog("Unable to successfully parse date " + timeStr);

				string people = GetXmlForPerson(author, authorUri);
				HtmlNode itemNode = GetItemDataForFirstPost(thread, (int)ItemType.Email);
				if (itemNode != null) {
					string fromAccountIdStr = itemNode.GetAttributeValue("from", "");
					int fromAccountId;
					if (!string.IsNullOrEmpty(fromAccountIdStr) && int.TryParse(fromAccountIdStr, out fromAccountId))
						people += Templates.BuildPerson(fromAccountId, "to");
				}

				List<int> tags = new List<int> { _tagIdEmails };
				tags.AddRange(GetTagsForIssueReferences(xmlDoc.DocumentNode, "//s1:keui/s1:subjectAnnotated"));
				tags.AddRange(GetTagsForIssueReferences(xmlDoc.DocumentNode, "//s1:keui/s1:contentAnnotated"));

				string concepts = GetConcepts(xmlDoc.DocumentNode, "//s1:keui/s1:subjectConcepts");
				concepts += GetConcepts(xmlDoc.DocumentNode, "//s1:keui/s1:contentConcepts");

				string itemInfo = Templates.BuildItemInfo(-1, time, thread.EncodeXMLString(), subject.EncodeXMLString(), entryId.EncodeXMLString(), people, tags: tags, concepts: concepts, itemType: (int)ItemType.Email);
				AddItemStatus itemStatus = MailData.AddItem(itemInfo, body);
				if (itemStatus.ItemId == -1)
					AddEventAndLog("Failed to index item. Error information: " + MailData.MinerGetLastInformation());
				SetLastItemId(itemStatus.ItemId);

				// save also the item id in the keui node
				HtmlNode keuiNode = eventDataNode.SelectSingleNode("./s1:keui");
				AddNewLines(keuiNode, 9);
				keuiNode.AppendChild(CreateNodeWithTextContent(xmlDoc, "s1:itemId", itemStatus.ItemId.ToString()));

				// add also thread id
				keuiNode.AppendChild(CreateNodeWithTextContent(xmlDoc, "s1:threadId", itemStatus.ThreadId.ToString()));

				AddNewLines(keuiNode, 8);				// add newlines before </eventData>
				AddNewLines(eventDataNode, 7);

				// return the id of the last item
				return itemStatus.ItemId;
			}
			catch (Exception ex) {
				AddEvent("Exception while annotating email: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while annotating email.", ex);
			}
			return -1;
		}

		public bool IndexIssueNew(HtmlAgilityPack.XmlDocument xmlDoc)
		{
			try {
				HtmlNode eventDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData");
				HtmlNode kesiNode = eventDataNode.SelectSingleNode("./s:kesi");
				HtmlNode keuiNode = eventDataNode.SelectSingleNode("./s1:keui");
				HtmlNode mdNode = eventDataNode.SelectSingleNode("./o:mdservice");
				//string issueTrackerUrl = GetNodeInnerText(kesiNode, "./s:issueTracker/s:issueTrackerURL");
				string issueTrackerUrl = "default tracker";
				string issueId = GetNodeInnerText(kesiNode, "./s:issueId");
				string issueUrl = GetNodeInnerText(kesiNode, "./s:issueUrl");
				string issueUri = GetNodeInnerText(mdNode, "./o:issueUri", null);

				//CreatePeopleAccounts(xmlDoc);
				//return false;

				if (string.IsNullOrEmpty(issueId)) {
					AddEventAndLog("IssueId was not specified in the event. Ignoring the report");
					return false;
				}

				if (string.IsNullOrEmpty(issueUri)) {
					AddEventAndLog("The issueUri was not specified for a bug report. Ignoring the report");
					return false;
				}
				if (string.IsNullOrEmpty(issueTrackerUrl)) {
					AddEventAndLog("The issueTrackerUrl was not specified for a bug report. Ignoring the report");
					return false;
				}

				bool issueIsNew = MailData.GetTagInfo(issueUri) == null;		// if the tag for the issue does not exist yet then this is a new bug report
				if (!issueIsNew) {
					AddEventAndLog(String.Format("IssueNew event received but the issue with id {0} already exists in the database. Ignoring indexing of the event.", issueId));
					return false;
				}
				
				List<int> tags = new List<int> { _tagIdIssues };
				
				// create the tracer tag if not existing
				int trackerUrlTagId = MailData.CreateTagIfNotExisting(issueTrackerUrl, TagPrefixIssues + issueTrackerUrl, _tagIdIssues, TagMetaIssueTracker);
				tags.Add(trackerUrlTagId);
				// create the tag for the issue
				int issueIdTag = MailData.CreateTagIfNotExisting(issueId, issueUri, trackerUrlTagId, TagMetaIssue);
				tags.Add(issueIdTag);
				// add references to issues mentioned in the description
				tags.AddRange(GetTagsForIssueReferences(xmlDoc.DocumentNode, "//s1:keui/s1:issueDescriptionAnnotated"));

				string productName = GetNodeInnerText(kesiNode, "./s:issueProduct/s:productId", null);
				string componentName = GetNodeInnerText(kesiNode, "./s:issueProduct/s:productComponentId", null);
				string productNameUri = GetNodeInnerText(mdNode, "./o:issueProduct/o:productUri", null);
				string componentNameUri = GetNodeInnerText(mdNode, "./o:issueProduct/o:productComponentUri", null);

				int metaTrackerTagId = MailData.CreateTagIfNotExisting(issueTrackerUrl, TagPrefixMeta + issueTrackerUrl, _tagIdIssuesMeta, TagMetaIssueTracker);
				int productTagId = TagInfoBase.InvalidTagId, componentTagId = TagInfoBase.InvalidTagId;
				if (!string.IsNullOrEmpty(productName) && !string.IsNullOrEmpty(productNameUri))
					productTagId = MailData.CreateTagIfNotExisting(productName, productNameUri, metaTrackerTagId, TagMetaProduct);
				if (productTagId > 0 && !string.IsNullOrEmpty(componentName) && !string.IsNullOrEmpty(componentNameUri))
					componentTagId = MailData.CreateTagIfNotExisting(componentName, componentNameUri, productTagId, TagMetaComponent);
				tags.Add(productTagId);
				tags.Add(componentTagId);

				string timeStr = GetNodeInnerText(kesiNode, "./s:issueDateOpened");
				DateTime time = DateTime.Now;
				if (!DateTime.TryParse(timeStr, out time))
					AddEventAndLog("Unable to successfully parse date " + timeStr);

				string description = GetNodeInnerText(kesiNode, "./s:issueDescription");
				string comment = GetNodeInnerText(kesiNode, "./s:issueComment/s:commentText");

				string status = GetNodeInnerText(kesiNode, "./s:issueStatus");
				string resolution = GetNodeInnerText(kesiNode, "./s:issueResolution");
				//string statusUri = GetNodeInnerText(mdNode, "./o:issueStatus");
				//string resolutionUri = GetNodeInnerText(mdNode, "./o:issueResolution");
				
				// add resolution and status tags
				if (!string.IsNullOrEmpty(status))
					tags.Add(MailData.CreateTagIfNotExisting(status, TagPrefixStatus + status, _tagIdIssuesStatus, TagMetaStatus));
				if (!string.IsNullOrEmpty(resolution))
					tags.Add(MailData.CreateTagIfNotExisting(resolution, TagPrefixResolution + resolution, _tagIdIssuesResolution, TagMetaResolution));

				// get the metadata values that should be stored with the item
				XmlDocument metaDataDoc = new XmlDocument();
				if (!string.IsNullOrEmpty(issueUrl))
					metaDataDoc.DocumentNode.AppendChild(CreateNodeWithTextContent(metaDataDoc, "url", issueUrl));
				if (!string.IsNullOrEmpty(issueId))
					metaDataDoc.DocumentNode.AppendChild(CreateNodeWithTextContent(metaDataDoc, "issueId", issueId));
				HtmlNode issueInfoNode = metaDataDoc.CreateElement("issueInfo");
				metaDataDoc.DocumentNode.AppendChild(issueInfoNode);
				PopulateIssueInfoNode(issueInfoNode, kesiNode, mdNode);

				string metaData = metaDataDoc.DocumentNode.OuterHtml;		// use the content inside metadata tag as the metadata to index
				
				string author = GetIssueCommentAuthor(kesiNode, 0);
				string authorUri = GetNodeInnerText(mdNode, "./o:issueAuthorUri", "");	// todo: fix this
				// add the creator as the author (might be useful for the search)
				string people = GetXmlForPerson(author, authorUri, role: "author");

				string entryId = GetNodeInnerText(mdNode, "./o:issueComment/o:commentUri", null);		// todo: fix to the actual value
				if (string.IsNullOrEmpty(entryId)) {
					AddEventAndLog("The entryId was not specified for a bug comment. Ignoring.");
					return false;
				}

				HashSet<int> customTags = GetTagsForIssueReferences(keuiNode, ".//s1:issueDescriptionAnnotated");
				customTags.UnionWith(tags);
				customTags.Remove(TagInfoBase.InvalidTagId);

				string descriptionConcepts = GetConcepts(keuiNode, ".//s1:issueDescriptionConcepts");
				string commentConcepts = GetConcepts(keuiNode, ".//s1:commentTextConcepts");
				string concepts = descriptionConcepts + commentConcepts;

				string itemInfo = Templates.BuildItemInfo(-1, time, issueUri.EncodeXMLString(), description.EncodeXMLString(), entryId.EncodeXMLString(), people, metaData: metaData, tags: customTags, concepts: concepts, itemType: (int)ItemType.BugDescription);
				AddItemStatus itemStatus = MailData.AddItem(itemInfo, comment);
				if (itemStatus.ItemId == -1)
					AddEventAndLog("Failed to index item. Error information: " + MailData.MinerGetLastInformation());
				SetLastItemId(itemStatus.ItemId);

				// save also the item id in the commentNode
				HtmlNode commentNode = keuiNode.SelectSingleNode("./s1:issueComment");
				commentNode.AppendChild(CreateNodeWithTextContent(xmlDoc, "s1:itemId", itemStatus.ItemId.ToString()));

				// add the thread id to keui node
				keuiNode.AppendChild(CreateNodeWithTextContent(xmlDoc, "s1:threadId", itemStatus.ThreadId.ToString()));
								
				AddNewLines(keuiNode, 8);		// add newlines before </eventData>
				AddNewLines(eventDataNode, 7);

				// return the id of the last item
				return true;
			}
			catch (Exception ex) {
				AddEvent("Exception while indexing new bug description: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while indexing new bug description: ", ex);
			}
			return false;
		}

		public void PopulateIssueInfoNode(HtmlNode issueInfoNode, HtmlNode kesiNode, HtmlNode mdNode)
		{
			string status = GetNodeInnerText(kesiNode, "./s:issueStatus", null);
			if (status != null)
				issueInfoNode.SetAttributeValue("status", status.EncodeXMLString());
			string resolution = GetNodeInnerText(kesiNode, "./s:issueResolution", null);
			if (resolution != null)
				issueInfoNode.SetAttributeValue("resolution", resolution.EncodeXMLString());
			string productName = GetNodeInnerText(kesiNode, "./s:issueProduct/s:productId", null);
			if (productName != null && GetNodeInnerText(kesiNode, "./s:issueActivity/s:activityWhat", "Product") == "Product")
				issueInfoNode.SetAttributeValue("productName", productName.EncodeXMLString());
			string componentName = GetNodeInnerText(kesiNode, "./s:issueProduct/s:productComponentId", null);
			if (componentName != null && GetNodeInnerText(kesiNode, "./s:issueActivity/s:activityWhat", "Component") == "Component")
				issueInfoNode.SetAttributeValue("componentName", componentName.EncodeXMLString());
			
			string priority = GetNodeInnerText(kesiNode, "./s:issuePriority", null);
			if (priority != null)
				issueInfoNode.SetAttributeValue("priority", priority.EncodeXMLString());
			string severity = GetNodeInnerText(kesiNode, "./s:issueSeverity", null);
			if (severity != null)
				issueInfoNode.SetAttributeValue("severity", severity.EncodeXMLString());

			string systemOS = GetNodeInnerText(kesiNode, "./s:issueComputerSystem/s:computerSystemOS", null);
			if (systemOS != null)
				issueInfoNode.SetAttributeValue("systemOS", systemOS.EncodeXMLString());
			string systemPlatform = GetNodeInnerText(kesiNode, "./s:issueComputerSystem/s:computerSystemPlatform", null);
			if (systemPlatform != null)
				issueInfoNode.SetAttributeValue("systemPlatform", systemPlatform.EncodeXMLString());
			string productVersion = GetNodeInnerText(kesiNode, "./s:issueProduct/s:productVersion", null);
			if (productVersion != null)
				issueInfoNode.SetAttributeValue("productVersion", productVersion.EncodeXMLString());

			string assignedToName = GetIssueAssignedTo(kesiNode);
			string assignedToUri = GetNodeInnerText(mdNode, "./o:issueAssignedToUri", null);
			if (!string.IsNullOrEmpty(assignedToName)) {
				HtmlNode assignedToNode = issueInfoNode.OwnerDocument.CreateElement("assignedTo");
				issueInfoNode.AppendChild(assignedToNode);
				assignedToNode.SetAttributeValue("name", assignedToName.EncodeXMLString());
				if (!string.IsNullOrEmpty(assignedToUri))
					assignedToNode.SetAttributeValue("uri", assignedToUri.EncodeXMLString());
			}
			string CCName = GetIssueCCPerson(kesiNode);
			string CCUri = GetNodeInnerText(mdNode, "./o:issueCCPerson", null);		// todo: change this to the right value
			if (!string.IsNullOrEmpty(CCName)) {
				HtmlNode assignedToNode = issueInfoNode.OwnerDocument.CreateElement("CC");
				issueInfoNode.AppendChild(assignedToNode);
				assignedToNode.SetAttributeValue("name", CCName.EncodeXMLString());
				if (!string.IsNullOrEmpty(CCUri))
					assignedToNode.SetAttributeValue("uri", CCUri.EncodeXMLString());
			}
		}

		public bool IndexIssueUpdate(HtmlAgilityPack.XmlDocument xmlDoc)
		{
			try {
				var eventDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData");
				HtmlNode kesiNode = eventDataNode.SelectSingleNode("./s:kesi");
				HtmlNode keuiNode = eventDataNode.SelectSingleNode("./s1:keui");
				HtmlNode mdNode = eventDataNode.SelectSingleNode("./o:mdservice");
				string issueId = GetNodeInnerText(kesiNode, "./s:issueId");
				string issueUri = GetNodeInnerText(mdNode, "./o:issueUri", null);
				//string issueTrackerUrl = GetNodeInnerText(kesiNode, "./s:issueTracker/s:issueTrackerURL");
				string issueTrackerUrl = "default tracker";

				if (string.IsNullOrEmpty(issueId)) {
					AddEventAndLog("IssueId was not specified in the event. Ignoring the report");
					return false;
				}
				if (string.IsNullOrEmpty(issueUri)) {
					AddEventAndLog(String.Format("The issueUri was not specified for a bug report #{0}. Ignoring the report", issueId));
					return false;
				}

				bool issueIsNew = MailData.GetTagInfo(issueUri) == null;		// if the tag for the issue does not exist yet then this is a new bug report
				if (issueIsNew) {
					AddEventAndLog(String.Format("IssueUpdate event received for an unknown issue #{0}. Ignoring indexing of the event.", issueId));
					return false;
				}

				int tagIdLastProduct = TagInfoBase.InvalidTagId;
				int tagIdLastComponent = TagInfoBase.InvalidTagId;
				int tagIdLastResolution = TagInfoBase.InvalidTagId;
				int tagIdLastStatus = TagInfoBase.InvalidTagId;
				List<int> commonTags = new List<int> { _tagIdIssues };
				string subject = null;
				string originalAuthor = "";
				XmlDocument bugDescriptionMetaDoc = new XmlDocument();
				string commentMetadata = "";
				//int threadId = -1;
				//string productName = null, componentName = null, productNameUri = null, componentNameUri = null;
				// the issue should be existing so we get the issue info from the index
				HtmlNode node = GetItemDataForBugDescription(issueTrackerUrl, issueId);
				if (node != null) {
					string tags = node.GetAttributeValue("tags", "");
					string[] tagsStr = tags.Split(new[] { ',' }, StringSplitOptions.RemoveEmptyEntries);
					List<int> tagIds = new List<int>(from t in tagsStr select int.Parse(t));
					
					tagIdLastProduct = GetProductTagId(tagIds);
					tagIdLastComponent = GetComponentTagId(tagIds);
					tagIdLastResolution = GetResolutionTagId(tagIds);
					tagIdLastStatus = GetStatusTagId(tagIds);

					subject = GetNodeInnerText(node, "./subject", "");
					//string threadIdStr = node.GetAttributeValue("threadId", "");
					//int.TryParse(threadIdStr, out threadId);
					string authorAccountIdStr = node.GetAttributeValue("author", "");
					int authorAccountId;
					if (!string.IsNullOrEmpty(authorAccountIdStr) && int.TryParse(authorAccountIdStr, out authorAccountId)) {
						originalAuthor += Templates.BuildPerson(authorAccountId, "to");
					}
					else
						AddEventAndLog("Failed to identify the original author of the issue #" + issueId);
					// use the same metadata as the bug description. this will store the issue url and issueId
					string mainMetaData = GetNodeInnerHtml(node, "./metaData", "");
					bugDescriptionMetaDoc.LoadXml(mainMetaData);
				}

				string url = GetNodeInnerText(bugDescriptionMetaDoc.DocumentNode, "/url", "");
				commentMetadata = string.Format("<issueId>{0}</issueId><url>{1}</url>", issueId, url.EncodeXMLString());

				// create the tracer tag if not existing
				int trackerUrlTagId = MailData.CreateTagIfNotExisting(issueTrackerUrl, TagPrefixIssues + issueTrackerUrl, _tagIdIssues, TagMetaIssueTracker);
				commonTags.Add(trackerUrlTagId);
				// create the tag for the issue
				int issueIdTag = MailData.CreateTagIfNotExisting(issueId, issueUri, trackerUrlTagId, TagMetaIssue);
				commonTags.Add(issueIdTag);
				
				int metaTrackerTagId = MailData.CreateTagIfNotExisting(issueTrackerUrl, TagPrefixMeta + issueTrackerUrl, _tagIdIssuesMeta, TagMetaIssueTracker);
				//int productTagId = -1, componentTagId = -1;
				//if (!string.IsNullOrEmpty(productName) && !string.IsNullOrEmpty(productNameUri))
				//	productTagId = MailData.CreateTagIfNotExisting(productName, productNameUri, metaTrackerTagId, TagMetaProduct);
				//if (productTagId > 0 && !string.IsNullOrEmpty(componentName) && !string.IsNullOrEmpty(componentNameUri))
				//	componentTagId = MailData.CreateTagIfNotExisting(componentName, componentNameUri, productTagId, TagMetaComponent);
				commonTags.Add(tagIdLastProduct);
				commonTags.Add(tagIdLastComponent);
				commonTags.Add(tagIdLastStatus);
				commonTags.Add(tagIdLastResolution);

				var kesiCommentNodes = kesiNode.SelectNodes("./s:issueComment");
				int commentCount = kesiCommentNodes != null ? kesiCommentNodes.Count : 0;
				for (int commentN = 0; commentN < commentCount; commentN++) {
					string entryId = GetNodeInnerText(mdNode, string.Format("./o:issueComment[{0}]/o:commentUri", commentN + 1, null));
					if (string.IsNullOrEmpty(entryId)) {
						AddEventAndLog(String.Format("The commentUri was not specified for a bug comment. Ignoring indexing it (issue #{0}).", issueId));
						continue;
					}

					string comment = GetNodeInnerText(kesiNode, string.Format("./s:issueComment[{0}]/s:commentText", commentN + 1), "");
					string timeStr = GetNodeInnerText(kesiNode, string.Format("./s:issueComment[{0}]/s:commentDate", commentN + 1), "");
					DateTime time = DateTime.Now;
					if (!DateTime.TryParse(timeStr, out time))
						AddEventAndLog(String.Format("Unable to successfully parse date {0} (issue #{1})", timeStr, issueId));

					string author = GetIssueCommentAuthor(kesiNode, commentN);
					string authorUri = GetNodeInnerText(mdNode, string.Format("./o:issueComment[{0}]/o:commentPersonUri", commentN + 1), "");
					string people = GetXmlForPerson(author, authorUri, role: "from");
					if (string.IsNullOrEmpty(people))
						AddEventAndLog("The author of the issue comment was not available. Data in the s:commentPerson: " + GetNodeInnerHtml(kesiNode, string.Format("./s:issueComment[{0}]/s:commentPerson", commentN + 1)));
					people += originalAuthor;

					HashSet<int> customTags = GetTagsForIssueReferences(keuiNode, string.Format("./s1:issueComment[{0}]/s1:commentTextAnnotated", commentN + 1));
					customTags.UnionWith(commonTags);
					customTags.Remove(TagInfoBase.InvalidTagId);

					string concepts = GetConcepts(keuiNode, string.Format("./s1:issueComment[{0}]/s1:commentTextConcepts", commentN + 1));

					// is this a bug description or a comment
					string itemInfo = Templates.BuildItemInfo(-1, time, issueUri.EncodeXMLString(), subject.EncodeXMLString(), entryId.EncodeXMLString(), people, metaData: commentMetadata, tags: customTags, concepts: concepts, itemType: (int)ItemType.BugComment);
					AddItemStatus itemStatus = MailData.AddItem(itemInfo, comment);
					if (itemStatus.ItemId == -1)
						AddEventAndLog("Failed to index item. Error information: " + MailData.MinerGetLastInformation());
					SetLastItemId(itemStatus.ItemId);

					// save also the item id in the keui node
					HtmlNode keuiCommentNode = keuiNode.SelectSingleNode(String.Format("./s1:issueComment[{0}]", commentN + 1));
					AddNewLines(keuiCommentNode, 10);
					keuiCommentNode.AppendChild(CreateNodeWithTextContent(xmlDoc, "s1:itemId", itemStatus.ItemId.ToString()));

					// add also thread id
					keuiNode.AppendChild(CreateNodeWithTextContent(xmlDoc, "s1:threadId", itemStatus.ThreadId.ToString()));
				}

				// find the old info about the issue. needed to compare with the new values
				var bugDescriptionIssueInfoNode = bugDescriptionMetaDoc.DocumentNode.SelectSingleNode("//issueInfo");
				
				// ///////
				// if the issueUpdate event doesn't contain any comments this means that some meta info about the issue has changed
				// in this case grab the new meta and create an item with only metadata
				if (commentCount == 0) {
					XmlDocument commentMetaDoc = new XmlDocument();
					HtmlNode commentIssueInfoNode = commentMetaDoc.CreateElement("issueInfo");
					commentMetaDoc.DocumentNode.AppendChild(commentIssueInfoNode);
					PopulateIssueInfoNode(commentIssueInfoNode, kesiNode, mdNode);
					// some attributes are in the issue update even though they didn't change
					// we remove attributes such attributes
					foreach (string attrName in (from attr in commentIssueInfoNode.Attributes select attr.Name).ToList()) {
						if (bugDescriptionIssueInfoNode.GetAttributeValue(attrName, null) == commentIssueInfoNode.GetAttributeValue(attrName, null))
							commentIssueInfoNode.Attributes.Remove(attrName);

					}
					if (commentIssueInfoNode.HasAttributes) {
						// parse time
						string timeStr = GetNodeInnerText(kesiNode, ".//s:issueActivity/s:activityWhen", "");
						DateTime time = DateTime.Now;
						DateTime.TryParse(timeStr, out time);

						// parse author
						string author = GetNodeInnerText(kesiNode, ".//s:issueActivity/s:activityWho", ""); ;
						string authorUri = GetNodeInnerText(mdNode, "./o:issueActivity/o:activityWhoUri", "");
						string people = "";
						if (!string.IsNullOrEmpty(author) && !string.IsNullOrEmpty(authorUri))
							people = GetXmlForPerson(author, authorUri, role: "from");

						string itemInfo = Templates.BuildItemInfo(-1, time, issueUri.EncodeXMLString(), subject.EncodeXMLString(), Guid.NewGuid().ToString().EncodeXMLString(), people, metaData: commentMetaDoc.DocumentNode.OuterHtml, itemType: (int)ItemType.BugComment);
						AddItemStatus itemStatus = MailData.AddItem(itemInfo, "");
						if (itemStatus.ItemId == -1)
							AddEventAndLog("Failed to index item. Error information: " + MailData.MinerGetLastInformation());
						SetLastItemId(itemStatus.ItemId);
					}
				}

				// ///////
				// update the metadata of the original bug description with new info 
				PopulateIssueInfoNode(bugDescriptionIssueInfoNode, kesiNode, mdNode);
				int bugDescriptionItemId = node.GetAttributeValue("id", -1);
				string newMetadata = bugDescriptionMetaDoc.DocumentNode.OuterHtml;
				MailData.SQLSetItemMeta(bugDescriptionItemId, newMetadata);

				// ///////
				// update the tags - find the old and new values for status, resolution, product, component
				var mdActivityNodes = mdNode.SelectNodes("./o:issueActivity") ?? new HtmlNodeCollection(null);
				var kesiActivityNodes = kesiNode.SelectNodes("./s:issueActivity") ?? new HtmlNodeCollection(null);
				if (mdActivityNodes.Count != kesiActivityNodes.Count) {
					AddEventAndLog("IssueUpdate event doesn't contain same number of /o:issueActivity nodes as /s:issueActivity nodes. Not updating the activities for the issue.");
					return false;
				}

				string newStatus = GetNodeInnerText(kesiNode, "./s:issueStatus", null);
				string newResolution = GetNodeInnerText(kesiNode, "./s:issueResolution", null);
				int tagIdNewStatus = TagInfoBase.InvalidTagId;
				int tagIdNewResolution = TagInfoBase.InvalidTagId;
				if (!string.IsNullOrEmpty(newStatus))
					tagIdNewStatus = MailData.CreateTagIfNotExisting(newStatus, TagPrefixStatus + newStatus, _tagIdIssuesStatus, TagMetaStatus);
				if (!string.IsNullOrEmpty(newResolution))
					tagIdNewResolution = MailData.CreateTagIfNotExisting(newResolution, TagPrefixResolution + newResolution, _tagIdIssuesResolution, TagMetaResolution);

				int tagIdNewProduct = TagInfoBase.InvalidTagId;
				int tagIdNewComponent = TagInfoBase.InvalidTagId;

				string newProduct = GetNodeInnerText(kesiNode, "./s:issueProduct/s:productId", null);
				string newComponent = GetNodeInnerText(kesiNode, "./s:issueProduct/s:productComponentId", null);
				string newProductUri = GetNodeInnerText(mdNode, "./o:issueProduct/o:productUri", null);
				string newComponentUri = GetNodeInnerText(mdNode, "./o:issueProduct/o:productComponentUri", null);

				if (newProduct != null && newProductUri != null)
					tagIdNewProduct = MailData.CreateTagIfNotExisting(newProduct, newProductUri, metaTrackerTagId, TagMetaProduct);
				if (tagIdNewProduct == TagInfoBase.InvalidTagId)
					tagIdNewProduct = tagIdLastProduct;

				// component tag has to be created last (we might have to first create the tag id for the product)
				if (newComponent != null && newComponentUri != null)
					tagIdNewComponent = MailData.CreateTagIfNotExisting(newComponent, newComponentUri, tagIdNewProduct, TagMetaComponent);
				
				//// update the status of the issue
				// first find all the posts that belong to the issue
				IEnumerable<int> itemsForIssue = MailData.GetItemsForTagId(issueIdTag);
				// update the items with the new values
				if (tagIdLastResolution != tagIdNewResolution && tagIdNewResolution != TagInfoBase.InvalidTagId) {
					if (tagIdLastResolution != TagInfoBase.InvalidTagId)
						MailData.MinerRemoveTagForItems(tagIdLastResolution, itemsForIssue);
					MailData.MinerSetTagForItems(tagIdNewResolution, itemsForIssue);
				}
				if (tagIdLastStatus != tagIdNewStatus && tagIdNewStatus != TagInfoBase.InvalidTagId ) {
					if (tagIdLastStatus != TagInfoBase.InvalidTagId)
						MailData.MinerRemoveTagForItems(tagIdLastStatus, itemsForIssue);
					MailData.MinerSetTagForItems(tagIdNewStatus, itemsForIssue);
				}
				
				if (tagIdLastProduct != tagIdNewProduct && tagIdNewProduct != TagInfoBase.InvalidTagId) {
					if (tagIdLastProduct != TagInfoBase.InvalidTagId)
						MailData.MinerRemoveTagForItems(tagIdLastProduct, itemsForIssue);
					MailData.MinerSetTagForItems(tagIdNewProduct, itemsForIssue);
				}
				if (tagIdLastComponent != tagIdNewComponent && tagIdNewComponent != TagInfoBase.InvalidTagId) {
					if (tagIdLastComponent != TagInfoBase.InvalidTagId)
						MailData.MinerRemoveTagForItems(tagIdLastComponent, itemsForIssue);
					MailData.MinerSetTagForItems(tagIdNewComponent, itemsForIssue);
				}
				
				AddNewLines(keuiNode, 8);		// add newlines before </eventData>
				AddNewLines(eventDataNode, 7);

				// return the id of the last item
				return true;
			}
			catch (Exception ex) {
				AddEvent("Exception while indexing new bug description: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while indexing new bug description: ", ex);
			}
			return false;
		}

		HashSet<string> seenThreads = new HashSet<string>();
		public int IndexForumPost(HtmlAgilityPack.XmlDocument xmlDoc)
		{
			try {
				var eventDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData");
				var forumNode = eventDataNode.SelectSingleNode("./r:forumSensor");

				//string forumSensorAccountId = forumNode.SelectSingleNode("./r:forumSensorAccountId").InnerText;
				string forumId = GetNodeInnerText(forumNode, "./r:forumId");
				string forumName = GetNodeInnerText(forumNode, "./r:forumName");
				string forumItemId = GetNodeInnerText(forumNode, "./r:forumItemId");
				string forumItemUrl = GetNodeInnerText(forumNode, "./r:forumItemUrl");
				string timeStr = GetNodeInnerText(forumNode, "./r:time");
				string thread = "ForumThread: " + GetNodeInnerText(forumNode, "./r:forumThreadId");
				string subject = GetNodeInnerText(forumNode, "./r:subject");
				string body = GetNodeInnerText(forumNode, "./r:body");
				string author = GetNodeInnerText(forumNode, "./r:author");
				string category = GetNodeInnerText(forumNode, "./r:category");

				if (seenThreads.Contains(thread)) {
					Trace.WriteLine(thread);
				}
				seenThreads.Add(thread);

				var mdNode = eventDataNode.SelectSingleNode("./o:mdservice");
				string entryId = GetNodeInnerText(mdNode, "./o:postUri");
				if (string.IsNullOrEmpty(entryId)) {
					//AddEvent("The entryId was not specified for a new forum post. Ignoring.");
					GenLib.Log.LogService.LogWarning("The forum post did not have a /o:postUri value. Using forumId and forumItemId as Id.");
					entryId = forumId + "-" + forumItemId;
				}

				DateTime time = DateTime.Now;
				if (!DateTime.TryParse(timeStr, out time))
					AddEventAndLog("Unable to successfully parse date " + timeStr);

				// get authorUri for forum post
				string authorUri = GetNodeInnerText(mdNode, "./o:authorUri", null);
				if (string.IsNullOrEmpty(authorUri)) {
					AddEvent("Forum posts did not have an author URI. Skipping indexing of the post");
					return -1;
				}
				string people = GetXmlForPerson(author, authorUri);
				//string people = GetXmlForPerson(author, author);
				HtmlNode itemNode = GetItemDataForFirstPost(thread, (int)ItemType.ForumPost);
				if (itemNode != null) {
					string fromAccountIdStr = itemNode.GetAttributeValue("from", "");
					int fromAccountId;
					if (!string.IsNullOrEmpty(fromAccountIdStr) && int.TryParse(fromAccountIdStr, out fromAccountId))
						people += Templates.BuildPerson(fromAccountId, "to");
				}

				List<int> tags = new List<int> { _tagIdForums };
				if (!string.IsNullOrEmpty(forumName)) {
					int forumTag = MailData.CreateTagIfNotExisting(forumName, forumName, _tagIdForums);
					tags.Add(forumTag);
				}
				tags.AddRange(GetTagsForIssueReferences(xmlDoc.DocumentNode, "//s1:keui/s1:subjectAnnotated"));
				tags.AddRange(GetTagsForIssueReferences(xmlDoc.DocumentNode, "//s1:keui/s1:bodyAnnotated"));

				string concepts = GetConcepts(xmlDoc.DocumentNode, "//s1:keui/s1:subjectConcepts");
				concepts += GetConcepts(xmlDoc.DocumentNode, "//s1:keui/s1:bodyConcepts");

				string metadata = "";
				if (!string.IsNullOrEmpty(forumItemUrl))
					metadata += "<url>" + forumItemUrl.EncodeXMLString() + "</url>";

				string itemInfo = Templates.BuildItemInfo(-1, time, thread.EncodeXMLString(), subject.EncodeXMLString(), entryId.EncodeXMLString(), people, tags: tags, concepts: concepts, metaData: metadata, itemType: (int)ItemType.ForumPost);
				AddItemStatus itemStatus = MailData.AddItem(itemInfo, body);
				if (itemStatus.ItemId == -1)
					AddEventAndLog("Failed to index item. Error information: " + MailData.MinerGetLastInformation());
				SetLastItemId(itemStatus.ItemId);

				// save also the item id in the keui node
				HtmlNode keuiNode = eventDataNode.SelectSingleNode("./s1:keui");
				AddNewLines(keuiNode, 9);
				keuiNode.AppendChild(CreateNodeWithTextContent(xmlDoc, "s1:itemId", itemStatus.ItemId.ToString()));

				// add also thread id
				keuiNode.AppendChild(CreateNodeWithTextContent(xmlDoc, "s1:threadId", itemStatus.ThreadId.ToString()));

				AddNewLines(keuiNode, 8);				// add newlines before </eventData>
				AddNewLines(eventDataNode, 7);

				// return the id of the last item
				return itemStatus.ItemId;
			}
			catch (Exception ex) {
				AddEvent("Exception while indexing new forum post: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while indexing new forum post.", ex);
			}
			return -1;
		}

		public int IndexCustomItemToIndex(HtmlAgilityPack.XmlDocument xmlDoc)
		{
			try {
				Debug.Assert(MailData != null, "MailData is null. The indexing service has to be initialize before posts can be processed.");

				HtmlNode metaDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData/s1:customItem/s1:metaData");
				HtmlNode contentNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData/s1:customItem/s1:content");
				if (metaDataNode == null) {
					AddEventAndLog("The metaData node was not found in the event. use the right template for the event");
					return -1;
				}
				if (contentNode == null) {
					AddEventAndLog("The content node was not found in the event. use the right template for the event");
					return -1;
				}

				string content = contentNode.InnerText.DecodeXMLString();
				string metaData = metaDataNode.InnerHtml;

				AddItemStatus itemStatus = MailData.AddItem(metaData, content);
				if (itemStatus.ItemId == -1)
					AddEventAndLog("Failed to index item. Error information: " + MailData.MinerGetLastInformation());
				SetLastItemId(itemStatus.ItemId);
				return itemStatus.ItemId;
			}
			catch (Exception ex) {
				AddEvent("Exception while indexing custom item: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while indexing custom item.", ex);
			}
			return -1;
		}

		//public void IdentityNew(HtmlAgilityPack.XmlDocument xmlDoc)
		//{
		//	try {
		//		Debug.Assert(MailData != null, "MailData is null. The indexing service has to be initialize before posts can be processed.");
		//		HtmlNodeCollection identityNodes = xmlDoc.DocumentNode.SelectNodes("//ns1:eventData/sm:identities/sm:identity");
		//		if (identityNodes == null)
		//			return;
		//		foreach (var identityNode in identityNodes) {
		//			string uuid = GetNodeInnerText(identityNode, "./sm:uuid", null);
		//			if (uuid == null) {
		//				AddEvent("Failed to locate uuid value for identity node:" + identityNode.InnerHtml);
		//				continue;
		//			}

		//			PersonInfo mergedPersonInfo = null;
		//			HtmlNodeCollection personNodes = identityNode.SelectNodes("./sm:persons/sm:is/sm:person") ?? new HtmlNodeCollection(null);
		//			foreach (HtmlNode personNode in personNodes) {
		//				string uri = GetNodeInnerText(personNode, "./sm:uri", null);
		//				string firstName = GetNodeInnerText(personNode, "./sm:firstname", null);
		//				string lastName = GetNodeInnerText(personNode, "./sm:lastname", null);
		//				string email = GetNodeInnerText(personNode, "./sm:email", null);

		//				PersonInfo personInfo = MailData.PeopleData.GetPerson(uri, EAccountType.Custom);
		//				if (personInfo == null) {
		//					AddEvent("Failed to locate person with uri " + uri);
		//					continue;
		//				}
						
		//				if (!string.IsNullOrEmpty(firstName) && !string.IsNullOrEmpty(lastName)) {
		//					string fullName = firstName + " " + lastName;
		//					personInfo.SetName(fullName, EPersonNameTrust.High);	
		//				}
						
		//				if (mergedPersonInfo == null) {
		//					// remember the person info that all other persons will be merged into
		//					mergedPersonInfo = personInfo;	
		//					// set the uuid for the person - store it simply in the occupation field
		//					mergedPersonInfo.SetOccupation(uuid);	
		//				}
		//				else {
		//					MailData.PeopleData.MergePersons(personInfo, mergedPersonInfo);
		//				}
		//			}
		//		}
		//	}
		//	catch (Exception ex) {
		//		AddEvent("Exception while processing IdentityNew: " + ex.Message);
		//		GenLib.Log.LogService.LogException("Exception while processing IdentityNew: ", ex);
		//	}
		//}

		public void IdentitySnapshot(HtmlAgilityPack.XmlDocument xmlDoc)
		{
			try {
				Debug.Assert(MailData != null, "MailData is null. The indexing service has to be initialize before posts can be processed.");
				HtmlNode identityNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData/sm:identity");
				if (identityNode == null)
					return;
				string UUID = GetNodeInnerText(identityNode, "./sm:uuid", null);
				if (UUID == null) {
					AddEvent("Failed to locate uuid value for identity node:" + identityNode.InnerHtml);
					return;
				}

				// first split all provided accounts into separate accounts
				PersonInfo mergedPerson = null;
				HashSet<string> mergedPersonAccounts = new HashSet<string>();
				HtmlNodeCollection personNodes = identityNode.SelectNodes("./sm:person") ?? new HtmlNodeCollection(null);
				foreach (HtmlNode personNode in personNodes) {
					string accountUri = personNode.InnerText.DecodeXMLString();
					// often we get duplicates of the same account uri. in this case just ignore the duplicate
					if (mergedPersonAccounts.Contains(accountUri))
						continue;
					mergedPersonAccounts.Add(accountUri);
					int accountId = MailData.PeopleData.GetAccountId(accountUri, EAccountType.Custom);
					if (accountId == AccountInfo.InvalidAccountId) {
						AddEvent("Account id was not found for account uri " + accountUri);
						continue;
					}
					MailData.PeopleData.SetAccountUUID(accountId, UUID);		// this will get us account -> UUID

					if (mergedPerson == null) {
						mergedPerson = MailData.PeopleData.GetPerson(accountUri, EAccountType.Custom);
						mergedPerson.SetCustomData(UUID);
						//mergedPerson.SetName(UUID, EPersonNameTrust.High);		// this will get us other accounts for an account (account -> person -> accounts)
					}
					else {
						PersonInfo person = MailData.PeopleData.GetPerson(accountUri, EAccountType.Custom);
						if (person != null)
							MailData.PeopleData.ReassignAccountToExistingAccount(accountId, person, mergedPerson);
						else
							AddEvent(String.Format("Person for account uri {0} (account id {1}) was not found.", accountUri, accountId));
					}
				}

				// go through the accounts that belong to the person and remove the ones that might have been 
				// assigned to the person before but are no longer
				if (mergedPerson != null) {
					foreach (string account in mergedPerson.GetAccounts()) {
						if (!mergedPersonAccounts.Contains(account)) {
							PersonInfo newPerson = MailData.PeopleData.ReassignAccountToNewAccount(MailData.PeopleData.GetAccountId(account, EAccountType.Custom));
							int accountId = MailData.PeopleData.GetAccountId(account, EAccountType.Custom);
							string uuid = MailData.PeopleData.GetAccountUUID(accountId);
							newPerson.SetName(uuid, EPersonNameTrust.High);
						}
					}
				}
			}
			catch (Exception ex) {
				AddEvent("Exception while processing IdentityNew: " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while processing IdentityNew: ", ex);
			}
		}
		#endregion

		#region suggestion functionality
		/// <summary>
		/// compute a list of concepts for which the labels match the given prefix
		/// </summary>
		/// <param name="prefix">the beginning of the label that we would like to match</param>
		/// <returns>a list of tuples (label, concept uri)</returns>
		public List<int> SuggestPeopleForPrefix(string prefix)
		{
			List<int> suggestions = new List<int>();
			try {

				if (string.IsNullOrEmpty(prefix))
					return suggestions;
				prefix = prefix.ToLower(System.Globalization.CultureInfo.InvariantCulture);
				prefix = prefix.Substring(0, Math.Min(prefix.Length, _prefixPersonMaxSize));
				if (_personPrefixToPersonId.ContainsKey(prefix))
					suggestions.AddRange(_personPrefixToPersonId[prefix]);

			}
			catch (Exception ex) {
				AddEvent("Exception while retrieving people suggestions for prefix " + prefix + ". " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while retrieving people suggestions for prefix " + prefix + ". ", ex);
			}
			return suggestions;
		}

		// build person suggestion list
		public void BuildSuggestionsPersonDict()
		{
			try {
				Dictionary<string, List<int>> prefixToPersonIdList = new Dictionary<string, List<int>>();

				Dictionary<int, PeopleDataSettings.FromToInfo> totals = PeopleData.GetPersonIdTotals();
				IEnumerable<int> sortedPeopleIds = from KeyValuePair<int, PeopleDataSettings.FromToInfo> item in totals orderby item.Value.FromCount + item.Value.ToCount descending select item.Key;

				DateTime start = DateTime.Now;
				string key;
				foreach (int personId in sortedPeopleIds) {
					HashSet<string> seenNames = new HashSet<string>();
					PersonInfo person = PeopleData.GetPerson(personId);
					foreach (int accountId in person.GetAccountIds()) {
						string name = PeopleData.GetPersonNameForAccount(accountId);
						string cleanName = name.ToLower();
						cleanName = Regex.Replace(cleanName, @"[-_\.]", " ");
						if (seenNames.Contains(cleanName))
							continue;
						seenNames.Add(cleanName);
						if (String.IsNullOrEmpty(name)) continue;
						//Debug.Assert(!string.IsNullOrEmpty(name));
						name = Text.ReplaceUnicodeCharsWithAscii(name);
						string[] parts = name.ToLower(System.Globalization.CultureInfo.InvariantCulture).Split(" <()>_".ToCharArray(), StringSplitOptions.RemoveEmptyEntries);
						foreach (string part in parts) {
							for (int i = 1; i <= _prefixPersonMaxSize; i++) {
								if (part.Length < i) continue;
								key = part.Substring(0, i);
								if (!prefixToPersonIdList.ContainsKey(key))
									prefixToPersonIdList[key] = new List<int>();
								if (prefixToPersonIdList[key].Count < _prefixPersonSuggestionCount && !prefixToPersonIdList[key].Contains(personId))
									prefixToPersonIdList[key].Add(personId);
							}
						}
					}
				}
				AddEventAndLog("People suggestions dictionary computed. Time needed: " + (int)(DateTime.Now - start).TotalMilliseconds + " ms");
				_personPrefixToPersonId = prefixToPersonIdList;
			}
			catch (Exception ex) {
				AddEvent("Exception while computing people suggestion dictionary: " + ex.Message);
				GenLib.Log.LogService.LogException("PeopleData. BuildSuggestionsPersonDict exception: ", ex);
			}
		}

		public IEnumerable<string> GetUniqueNames(IEnumerable<string> names)
		{
			// use list so that we keep the original order
			List<string> uniqueNames = new List<string>();
			HashSet<string> seenNames = new HashSet<string>();
			foreach (string name in names) {
				string cleanName = name.ToLower();
				cleanName = Regex.Replace(cleanName, @"[-_\.]", " ");
				if (seenNames.Contains(cleanName))
					continue;
				seenNames.Add(cleanName);
				uniqueNames.Add(name);
			}
			return uniqueNames;
		}

		public string GetSuggestedPersonName(int personId)
		{
			PersonInfo person = PeopleData.GetPerson(personId);
			if (person == null)
				return "";

			var names = from id in person.GetAccountIds()
						orderby PeopleData.SQLGetTotalCount(id, PeopleDataSettings.TotalsColumn.FromTotal) + PeopleData.SQLGetTotalCount(id, PeopleDataSettings.TotalsColumn.ToTotal) descending
						select PeopleData.GetPersonNameForAccount(id);
			if (names.Count() == 0) return "";
			names = GetUniqueNames(names);
			string name = names.ElementAt(0);
			if (names.Count() > 1) {
				name += " [AKA " + String.Join(", ", names.Skip(1)) + "]";
			}
			return name;
		}

		public List<int> SuggestFilesForPrefix(string prefix)
		{
			return _so.SuggestFilesForPrefix(prefix);
		}

		public List<int> SuggestModulesForPrefix(string prefix)
		{
			return _so.SuggestModulesForPrefix(prefix);
		}

		public List<int> SuggestMethodsForPrefix(string prefix)
		{
			return _so.SuggestMethodsForPrefix(prefix);
		}

		public List<Tuple<string, string>> SuggestProductsForPrefix(string prefix)
		{
			List<Tuple<string, string>> suggestions = new List<Tuple<string, string>>();
			try {
				prefix = prefix.ToLower(System.Globalization.CultureInfo.InvariantCulture);
				foreach (TagInfoBase trackerTag in MailData.GetTags(_tagIdIssuesMeta)) {
					foreach (TagInfoBase productTag in MailData.GetTags(trackerTag.TagId)) {
						if (suggestions.Count > _prefixProductSuggestionCount)
							break;
						string productName = productTag.TagName;
						productName = Text.ReplaceUnicodeCharsWithAscii(productName);
						if (productName.ToLower().StartsWith(prefix)) {
							suggestions.Add(new Tuple<string, string>(productName, productTag.TagIdStr.ToString()));
							continue;
						}
						foreach (TagInfoBase componentTag in MailData.GetTags(productTag.TagId)) {
							if (suggestions.Count > _prefixProductSuggestionCount)
								break;
							string componentName = componentTag.TagName;
							componentName = Text.ReplaceUnicodeCharsWithAscii(componentName);
							if (componentName.ToLower().StartsWith(prefix))
								suggestions.Add(new Tuple<string, string>(productTag.TagName + "\\" + componentTag.TagName, componentTag.TagIdStr.ToString()));
						}
					}
				}
			}
			catch (Exception ex) {
				AddEvent("Exception while retrieving product suggestions for prefix " + prefix + ". " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while retrieving product suggestions for prefix " + prefix + ". ", ex);
			}
			return suggestions;
		}

		public List<Tuple<string, string>> SuggestIssuesForPrefix(string prefix)
		{
			List<Tuple<string, string>> suggestions = new List<Tuple<string, string>>();
			try {
				prefix = prefix.ToLower(System.Globalization.CultureInfo.InvariantCulture);
				foreach (TagInfoBase trackerTag in MailData.GetTags(_tagIdIssues)) {
					foreach (TagInfoBase issueTag in MailData.GetTags(trackerTag.TagId)) {
						if (suggestions.Count > _prefixIssueSuggestionCount)
							break;
						string issueName = issueTag.TagName;
						if (issueName.StartsWith(prefix)) {
							string issueUri = issueTag.TagIdStr;
							suggestions.Add(new Tuple<string, string>(issueName, issueUri));
							continue;
						}
					}
				}
			}
			catch (Exception ex) {
				AddEvent("Exception while retrieving issue suggestions for prefix " + prefix + ". " + ex.Message);
				GenLib.Log.LogService.LogException("Exception while retrieving issue suggestions for prefix " + prefix + ". ", ex);
			}
			return suggestions;
		}
		#endregion

		#region helper functions
		public string MakeUriLong(string uri)
		{
			if (uri.StartsWith(TagIdPrefixFileShort))
				return TagIdPrefixFileLong + uri.TrimStart(TagIdPrefixFileShort);
			else if (uri.StartsWith(TagIdPrefixModuleShort))
				return TagIdPrefixModuleLong + uri.TrimStart(TagIdPrefixModuleShort);
			else if (uri.StartsWith(TagIdPrefixMethodShort))
				return TagIdPrefixMethodLong + uri.TrimStart(TagIdPrefixMethodShort);
			return uri;
		}

		public string MakeFileUriShort(string fileUri)
		{
			if (fileUri.StartsWith(TagIdPrefixFileLong)) {
				fileUri = TagIdPrefixFileShort + fileUri.TrimStart(TagIdPrefixFileLong);
			}
			return fileUri;
		}

		public string MakeModuleUriShort(string moduleUri)
		{
			if (moduleUri.StartsWith(TagIdPrefixModuleLong)) {
				moduleUri = TagIdPrefixModuleShort + moduleUri.TrimStart(TagIdPrefixModuleLong);
			}
			return moduleUri;
		}

		public string MakeMethodUriShort(string methodUri)
		{
			if (methodUri.StartsWith(TagIdPrefixMethodLong)) {
				methodUri = TagIdPrefixMethodShort + methodUri.TrimStart(TagIdPrefixMethodLong);
			}
			return methodUri;
		}

		private string GetIssueCommentAuthor(HtmlNode kesiNode, int commentN)
		{
			string author = GetNodeInnerText(kesiNode, string.Format("./s:issueComment[{0}]/s:commentPerson/s:name", commentN + 1), "");
			if (string.IsNullOrEmpty(author) || author.ToLower() == "none")
				author = GetNodeInnerText(kesiNode, string.Format("./s:issueComment[{0}]/s:commentPerson/s:email", commentN + 1), "");
			if (string.IsNullOrEmpty(author) || author.ToLower() == "none")
				author = GetNodeInnerText(kesiNode, string.Format("./s:issueComment[{0}]/s:commentPerson/s:id", commentN + 1), "");
			return author;
		}

		private string GetIssueAssignedTo(HtmlNode kesiNode)
		{
			string author = GetNodeInnerText(kesiNode, "./s:issueAssignedTo/s:name", null);
			if (string.IsNullOrEmpty(author) || author.ToLower() == "none")
				author = GetNodeInnerText(kesiNode, "./s:issueAssignedTo/s:email", null);
			if (string.IsNullOrEmpty(author) || author.ToLower() == "none")
				author = GetNodeInnerText(kesiNode, "./s:issueAssignedTo/s:id", null);
			return author;
		}

		private string GetIssueAuthor(HtmlNode kesiNode)
		{
			string author = GetNodeInnerText(kesiNode, "./s:issueAuthor/s:name", "");
			if (string.IsNullOrEmpty(author) || author.ToLower() == "none")
				author = GetNodeInnerText(kesiNode, "./s:issueAuthor/s:email", "");
			if (string.IsNullOrEmpty(author) || author.ToLower() == "none")
				author = GetNodeInnerText(kesiNode, "./s:issueAuthor/s:id", "");
			return author;
		}

		private string GetIssueCCPerson(HtmlNode kesiNode)
		{
			string author = GetNodeInnerText(kesiNode, "./s:issueCCPerson/s:name", null);
			if (string.IsNullOrEmpty(author) || author.ToLower() == "none")
				author = GetNodeInnerText(kesiNode, "./s:issueCCPerson/s:email", null);
			if (string.IsNullOrEmpty(author) || author.ToLower() == "none")
				author = GetNodeInnerText(kesiNode, "./s:issueCCPerson/s:id", null);
			return author;
		}

		/// <summary>
		/// in an issue there are many parts where a person can appear. we need to add these people to PeopleData
		/// otherwise we might be missing some ids later on (for example, in IdentitySnapshots)
		/// </summary>
		/// <param name="xmlDoc"></param>
		private void CreatePeopleAccounts(XmlDocument xmlDoc)
		{
			HtmlNode eventDataNode = xmlDoc.DocumentNode.SelectSingleNode("//ns1:eventData");
			HtmlNode kesiNode = eventDataNode.SelectSingleNode("./s:kesi");
			HtmlNode mdNode = eventDataNode.SelectSingleNode("./o:mdservice");
			if (kesiNode == null || mdNode == null)
				return;

			string author = GetIssueAuthor(kesiNode);
			string authorUri = GetNodeInnerText(mdNode, "./o:issueAuthorUri", null);
			if (!string.IsNullOrEmpty(author) && !string.IsNullOrEmpty(authorUri))
				PeopleData.AddNewPersonOrUpdate(author, authorUri, EAccountType.Custom, EPersonNameTrust.Low);

			string assignedTo = GetIssueAssignedTo(kesiNode);
			string assignedToUri = GetNodeInnerText(mdNode, "./o:issueAssignedToUri", null);
			if (!string.IsNullOrEmpty(assignedTo) && !string.IsNullOrEmpty(assignedToUri))
				PeopleData.AddNewPersonOrUpdate(assignedTo, assignedToUri, EAccountType.Custom, EPersonNameTrust.Low);
		}

        /// <summary>
        /// try all issue trackers that we monitor and try to find the tag that corresponds to bugId
        /// potential problem: if multiple trackers are used, the bugId is not unique
        /// </summary>
        /// <param name="bugId"></param>
        /// <returns></returns>
        private int GetTagIdForBugId(string bugId)
        {
            foreach (TagInfoBase issueTrackerTag in MailData.GetTags(_tagIdIssues))
            {
                int bugTagId = MailData.GetTagId(issueTrackerTag.TagId, bugId);
                if (bugTagId != TagInfoBase.InvalidTagId)
                    return bugTagId;
            }
            return TagInfoBase.InvalidTagId;
        }

        /// <summary>
        /// find the tag id for the given bugId and issue tracking url
        /// </summary>
        /// <param name="issueTrackerUrl"></param>
        /// <param name="bugId"></param>
        /// <returns></returns>
		private int GetTagIdForBugId(string issueTrackerUrl, string bugId)
		{
			if (string.IsNullOrEmpty(bugId))
				return TagInfoBase.InvalidTagId;

			TagInfoBase trackerTagInfo = MailData.GetTagInfo(_tagIdIssues, issueTrackerUrl);
			if (trackerTagInfo == null)
				return TagInfoBase.InvalidTagId;

			TagInfoBase tagInfo = MailData.GetTagInfo(trackerTagInfo.TagId, bugId);
			if (tagInfo == null)
				return TagInfoBase.InvalidTagId;
			return tagInfo.TagId;
		}

		private HtmlNode GetItemDataForBugDescription(string issueTrackerUrl, string bugId)
		{
			int bugTagId = GetTagIdForBugId(issueTrackerUrl, bugId);
			if (bugTagId == TagInfoBase.InvalidTagId)
				return null;
			QArgs args = new QArgs(new QTagIdCond(bugTagId));
			args.AddCondition(new QItemTypeCond((int)ItemType.BugDescription));
			// sort by itemIdAsc. in this way we will get the first item in the thread
			GeneralQuery query = new GeneralQuery(new QGeneralParams() { MaxCount = 1, SortBy = QResultSorting.itemIdAsc, IncludeAttachments = false }, args);
			
			string ret = MailData.MinerQuery(query);
			
			XmlDocument doc = new XmlDocument();
			doc.LoadXml(ret);
			HtmlNode node = doc.DocumentNode.SelectSingleNode("//item");
			if (node == null)
				return null;
			return node;
		}

		/// <summary>
		/// return the item data for the first item that exists in the thread.
		/// typically this is used to find who is the creator of the email or forum thread
		/// </summary>
		/// <param name="thread">string data representing the thread</param>
		/// <param name="itemType">what is the itemType of the item</param>
		/// <returns></returns>
		private HtmlNode GetItemDataForFirstPost(string thread, int itemType = -1)
		{
			QArgs args = new QArgs(new QThreadStrCond(thread));
			if (itemType != -1)
				args.AddCondition(new QItemTypeCond(itemType));
			GeneralQuery query = new GeneralQuery(new QGeneralParams() { ResultData = QResultData.itemData, MaxCount = 1, IncludeAttachments = false, SortBy = QResultSorting.dateAsc }, args);
			string ret = MailData.MinerQuery(query);
			XmlDocument doc = new XmlDocument();
			doc.LoadXml(ret);
			HtmlNode node = doc.DocumentNode.SelectSingleNode("//item");
			if (node == null)
				return null;
			return node;
		}

		private void GetProductComponentFromTags(IEnumerable<int> tagIds, ref string productName, ref string productNameUri, ref string componentName, ref string componentNameUri)
		{
			try
			{
				var productTagIdList = from tagId in tagIds where MailData.GetTagMeta(tagId) == TagMetaProduct select tagId;
				if (productTagIdList.Count() == 1) {
					int productTagId = productTagIdList.ElementAt(0);
					productNameUri = MailData.GetTagIdStr(productTagId);
					productName = MailData.GetTagName(productTagId);
				}
				else
					AddEventAndLog(String.Format("Warning. The list {0} contains {1} product names (instead of 1)", productTagIdList.ToList().ToString(), productTagIdList.Count()));
				
				var componentTagIdList = from tagId in tagIds where MailData.GetTagMeta(tagId) == TagMetaComponent select tagId;
				if (componentTagIdList.Count() == 1) {
					int componentTagId = componentTagIdList.ElementAt(0);
					componentNameUri = MailData.GetTagIdStr(componentTagId);
					componentName = MailData.GetTagName(componentTagId);
				}
				else
					AddEventAndLog(String.Format("Warning. The list {0} contains {1} component names (instead of 1)", componentTagIdList.ToList().ToString(), componentTagIdList.Count()));
			}
			catch (Exception ex)
			{
				AddEvent("Unable to get product/component info from tags. Exception:" + ex.Message);
				GenLib.Log.LogService.LogException("Unable to get product/component info from tags. ", ex);
			}
		}

		private int GetProductTagId(IEnumerable<int> tagIds)
		{
			var tagList = from tagId in tagIds where MailData.GetTagMeta(tagId) == TagMetaProduct select tagId;
			if (tagList.Count() == 1) {
				return tagList.ElementAt(0);
			}
			// some issues don't have assigned product
			if (tagList.Count() > 1)
				AddEventAndLog(String.Format("Warning. The list {0} contains {1} product tags (instead of 1)", tagList.ToList().ToString(), tagList.Count()));
			return TagInfoBase.InvalidTagId;
		}

		private int GetComponentTagId(IEnumerable<int> tagIds)
		{
			var tagList = from tagId in tagIds where MailData.GetTagMeta(tagId) == TagMetaComponent select tagId;
			if (tagList.Count() == 1) {
				return tagList.ElementAt(0);
			}
			// some issues are not assigned to a component
			if (tagList.Count() > 1)
				AddEventAndLog(String.Format("Warning. The list {0} contains {1} component tags (instead of 1)", tagList.ToList().ToString(), tagList.Count()));
			return TagInfoBase.InvalidTagId;
		}

		private int GetStatusTagId(IEnumerable<int> tagIds)
		{
			var tagList = from tagId in tagIds where MailData.GetTagMeta(tagId) == TagMetaStatus select tagId;
			if (tagList.Count() == 1) {
				return tagList.ElementAt(0);
			}
			AddEventAndLog(String.Format("Warning. The list {0} contains {1} status tags (instead of 1)", tagList.ToList().ToString(), tagList.Count()));
			return TagInfoBase.InvalidTagId;
		}

		private int GetResolutionTagId(IEnumerable<int> tagIds)
		{
			var tagList = from tagId in tagIds where MailData.GetTagMeta(tagId) == TagMetaResolution select tagId;
			if (tagList.Count() == 1) {
				return tagList.ElementAt(0);
			}
			AddEventAndLog(String.Format("Warning. The list {0} contains {1} resolution tags (instead of 1)", tagList.ToList().ToString(), tagList.Count()));
			return TagInfoBase.InvalidTagId;
		}

		// run a search to find similar threads and then return item data for the most similar x items
		//private string GetItemDataForSimilarItems(int itemId, int maxCount, int offset)
		//{
		//    string queryInfo = Contextify.Shared.Base.Templates.BuildCustomQueryGetSimilarItems(itemId);
		//    string ret = ProcessQuery((int)Templates.EQueryType.CustomQuery, queryInfo);
		//    HtmlAgilityPack.XmlDocument doc = new XmlDocument();
		//    doc.LoadXml(ret);
		//    List<string> retItemIds = new List<string>();
		//    Dictionary<string, string> itemIdToSimilarity = new Dictionary<string, string>();
		//    foreach (HtmlNode thread in doc.DocumentNode.SelectNodes("//similarities/item") ?? new HtmlNodeCollection(null))
		//    {
		//        string itemIdStr = thread.GetAttributeValue("id", "");
		//        retItemIds.Add(itemIdStr);
		//        string sim = thread.GetAttributeValue("sim", "0.0");
		//        itemIdToSimilarity[itemIdStr] = sim;
		//    }
		//    string queryItemIds = string.Join(",", retItemIds);
		//    string queryArgs = "<conditions><itemIds>" + queryItemIds + "</itemIds></conditions>";
		//    queryInfo = Contextify.Shared.Base.Templates.BuildItemQueryInfo(queryArgs, offset, maxCount, false, sortBy: "none", snipMatchKeywords: false, includePeopleData: true);
		//    // get items
		//    ret = ProcessQuery((int)Templates.EQueryType.GeneralQuery, queryInfo);
		//    // insert similarities to the item data
		//    doc = new XmlDocument();
		//    doc.LoadXml(ret);
		//    foreach (HtmlNode item in doc.DocumentNode.SelectNodes("//items/item") ?? new HtmlNodeCollection(null))
		//    {
		//        string itemIdStr = item.GetAttributeValue("id", "-1");
		//        if (!itemIdToSimilarity.ContainsKey(itemIdStr))
		//            continue;
		//        HtmlNode simNode = doc.CreateElement("similarity");
		//        simNode.SetAttributeValue("comparedItemId", itemId.ToString());
		//        item.AppendChild(simNode);
		//        simNode.AppendChild(doc.CreateTextNode(itemIdToSimilarity[itemIdStr]));
		//    }
		//    return doc.DocumentNode.InnerHtml;
		//}

		// run a search to find similar threads and then return item data for the most similar x items
		//private string GetItemDataForSimilarThreads(int threadId, int maxCount, int offset, bool includeOnlyFirstInThread)
		//{
		//    string queryInfo = Contextify.Shared.Base.Templates.BuildCustomQueryGetSimilarThreads(threadId, includeItemIdsForThread: true);
		//    string ret = ProcessQuery((int)Templates.EQueryType.CustomQuery, queryInfo);
		//    HtmlAgilityPack.XmlDocument doc = new XmlDocument();
		//    doc.LoadXml(ret);
		//    List<string> retItemIds = new List<string>();
		//    Dictionary<string, string> itemIdToSimilarity = new Dictionary<string, string>();
		//    foreach (HtmlNode thread in doc.DocumentNode.SelectNodes("//similarities/thread") ?? new HtmlNodeCollection(null))
		//    {
		//        string itemIdsStr = thread.GetAttributeValue("itemIds", "");
		//        string[] itemIds = itemIdsStr.Split(new[] { ',' }, StringSplitOptions.RemoveEmptyEntries);
		//        string sim = thread.GetAttributeValue("sim", "0.0");
		//        if (itemIds.Count() == 0)
		//            continue;
		//        if (includeOnlyFirstInThread)
		//        {
		//            retItemIds.Add(itemIds[0]);
		//            itemIdToSimilarity[itemIds[0]] = sim;
		//        }
		//        else
		//        {
		//            retItemIds.AddRange(itemIds);
		//            foreach (string item in itemIds)
		//                itemIdToSimilarity[item] = sim;
		//        }
		//    }
		//    string queryItemIds = string.Join(",", retItemIds);
		//    string queryArgs = "<conditions><itemIds>" + queryItemIds + "</itemIds></conditions>";
		//    queryInfo = Contextify.Shared.Base.Templates.BuildItemQueryInfo(queryArgs, offset, maxCount, false, sortBy: "none", snipMatchKeywords: false, includePeopleData: true);
		//    // get items
		//    ret = ProcessQuery((int)Templates.EQueryType.GeneralQuery, queryInfo);
		//    // insert similarities to the item data
		//    doc = new XmlDocument();
		//    doc.LoadXml(ret);
		//    foreach (HtmlNode item in doc.DocumentNode.SelectNodes("//items/item") ?? new HtmlNodeCollection(null))
		//    {
		//        string itemId = item.GetAttributeValue("id", "-1");
		//        if (!itemIdToSimilarity.ContainsKey(itemId))
		//            continue;
		//        HtmlNode simNode = doc.CreateElement("similarity");
		//        simNode.SetAttributeValue("comparedThreadId", threadId.ToString());
		//        item.AppendChild(simNode);
		//        simNode.AppendChild(doc.CreateTextNode(itemIdToSimilarity[itemId]));
		//    }
		//    return doc.DocumentNode.InnerHtml;
		//}

		private string GetConcepts(HtmlNode baseNode, string xpath)
		{
			string ret = "";
			HtmlNode conceptsNode = baseNode.SelectSingleNode(xpath);
			if (conceptsNode == null)
				return ret;
			foreach (var node in conceptsNode.SelectNodes("./s1:concept") ?? new HtmlNodeCollection(null)) {
				string url = "";
				string wgt = "1.0";
				var urlNode = node.SelectSingleNode("./s1:uri");
				if (urlNode != null)
					url = urlNode.InnerText;
				var wgtNode = node.SelectSingleNode("./s1:weight");
				if (wgtNode != null)
					wgt = wgtNode.InnerText;
				if (!string.IsNullOrEmpty(url))
					ret += String.Format("<concept uri=\"{0}\" weight=\"{1}\" />", url, wgt);
			}
			return ret;
		}

		/// <summary>
		/// find in the text references to *known* issues
		/// </summary>
		/// <param name="baseNode">base xml node that contains the text containing the references</param>
		/// <param name="xpath">xpath to find the subnode</param>
		/// <returns>list of tag ids for referenced issues</returns>
		private HashSet<int> GetTagsForIssueReferences(HtmlNode baseNode, string xpath)
		{
			HashSet<int> tags = new HashSet<int>();
			HashSet<string> tagIdStrHS = new HashSet<string>();
			GetIssueReferences(baseNode, xpath, tagIdStrHS);
			foreach (string tagIdStr in tagIdStrHS) {
				TagInfoBase tag = MailData.GetTagInfo(tagIdStr);
				if (tag != null)
					tags.Add(tag.TagId);
				else
					Debug.WriteLine("Did not find tag for " + tagIdStr);
			}
			return tags;
		}

		private void GetIssueReferences(HtmlNode baseNode, string xpath, HashSet<string> references)
		{
			HtmlNode referencesNode = baseNode.SelectSingleNode(xpath);
			if (referencesNode == null)
				return;
			foreach (var node in referencesNode.SelectNodes("./reference") ?? new HtmlNodeCollection(null)) {
				string bugUri = node.GetAttributeValue("bugUri", "");
				if (!string.IsNullOrEmpty(bugUri))
					references.Add(bugUri);
			}
		}

		/// <summary>
		/// fix <enrychableKws> tags. change them to <keywords><kw>content</kw></keywords>, where content contains keywords with expanded labels
		/// find //accounts/account[@name] and add also account id
		/// find //enrychableKeywords tags, expand the keywords using the ontology and rename the tag to //keywords
		/// fix references to bug numbers (bugId -> tagId)
		/// tagIdStr -> tagIds
		/// postTypes -> itemTypes
		/// </summary>
		/// <param name="requestData"></param>
		/// <returns></returns>
		public string FixQueryArgs(string requestData)
		{
			XmlDocument doc = new XmlDocument();
			doc.LoadXml(requestData);
			//foreach (HtmlNode conceptNameNode in doc.DocumentNode.SelectNodes("//queryArgs//conceptName") ?? new HtmlNodeCollection(null))
			//{
			//    string conceptLabel = conceptNameNode.InnerText;
			//    string uri = _ao.GetConceptUri(conceptLabel);
			//    if (string.IsNullOrEmpty(uri))
			//        continue;
			//    conceptNameNode.Name = "concept";
			//    conceptNameNode.InnerHtml = uri.EncodeXMLString();
			//}

			foreach (HtmlNode account in doc.DocumentNode.SelectNodes("//accounts/account[@name]") ?? new HtmlNodeCollection(null)) {
				string accountName = account.GetAttributeValue("name", "");
				short accountType = (short)account.GetAttributeValue("type", (int)Contextify.Shared.Types.EAccountType.Custom);
				int id = PeopleData.GetAccountId(accountName, (Contextify.Shared.Types.EAccountType)accountType);
				account.SetAttributeValue("id", id.ToString());
			}

			foreach (HtmlNode personNode in doc.DocumentNode.SelectNodes("//person") ?? new HtmlNodeCollection(null)) {
				string uuid = personNode.GetAttributeValue("uuid", "").DecodeXMLString();
				string uri = personNode.GetAttributeValue("uri", "").DecodeXMLString();
				PersonInfo personInfo = null;
				if (!string.IsNullOrEmpty(uri))
					personInfo = PeopleData.GetPerson(uri, Contextify.Shared.Types.EAccountType.Custom);
				else if (!string.IsNullOrEmpty(uuid)) {
					
					foreach (var person in PeopleData.GetPeople())
						if (person.CustomData == uuid) {
							personInfo = person;
							break;
						}
					if (personInfo == null)
						AddEvent("unable to find person with uuid " + uuid);
				}
				if (personInfo != null) {
					personNode.Name = "accounts";
					foreach (int accountId in personInfo.GetAccountIds()) {
						HtmlNode accountNode = doc.CreateElement("account");
						accountNode.SetAttributeValue("id", accountId.ToString());
						personNode.AppendChild(accountNode);
					}
				}
			}

			foreach (HtmlNode bugNode in doc.DocumentNode.SelectNodes("//bugId") ?? new HtmlNodeCollection(null)) {
				string bugId = bugNode.InnerText;
				// check bugs for possibly different bug tracking systems
				foreach (var issueTSTag in MailData.GetTags(_tagIdIssues)) {
					TagInfoBase tagInfo = MailData.GetTagInfo(issueTSTag.TagId, bugId);
					if (tagInfo != null) {
						bugNode.Name = "tagIds";
						bugNode.InnerHtml = tagInfo.TagId.ToString();
						break;
					}
				}
			}

			foreach (HtmlNode tagIdStrNode in doc.DocumentNode.SelectNodes("//tagIdStr") ?? new HtmlNodeCollection(null)) {
				string tagIdStr = tagIdStrNode.InnerText.DecodeXMLString();
				int tagId = MailData.GetTagId(tagIdStr);
				if (tagId == TagInfoBase.InvalidTagId)
					tagId = MailData.GetTagId(MakeFileUriShort(tagIdStr));
				if (tagId == TagInfoBase.InvalidTagId)
					tagId = MailData.GetTagId(MakeModuleUriShort(tagIdStr));
				if (tagId == TagInfoBase.InvalidTagId)
					tagId = MailData.GetTagId(MakeMethodUriShort(tagIdStr));		
				if (tagId != TagInfoBase.InvalidTagId) {
					tagIdStrNode.Name = "tagIds";
					tagIdStrNode.InnerHtml = tagId.ToString();
				}
				else
					AddEvent("Unable to find tag id for tagIdStr " + tagIdStr);
			}
			
			foreach (HtmlNode postTypeNode in doc.DocumentNode.SelectNodes("//postTypes") ?? new HtmlNodeCollection(null)) {
				string postTypes = postTypeNode.InnerText.DecodeXMLString();
				HashSet<int> types = new HashSet<int>();
				if (postTypes.Contains("issues")) { types.Add((int)ItemType.BugDescription); types.Add((int)ItemType.BugComment); }
                if (postTypes.Contains("issueDescriptions")) types.Add((int)ItemType.BugDescription);
                if (postTypes.Contains("issueComments")) types.Add((int)ItemType.BugComment);
				if (postTypes.Contains("commits")) types.Add((int)ItemType.Commit);
				if (postTypes.Contains("forums")) types.Add((int)ItemType.ForumPost);
				if (postTypes.Contains("mails")) types.Add((int)ItemType.Email);
				if (postTypes.Contains("wikis")) types.Add((int)ItemType.WikiPost);
				if (postTypes.Contains("ontologyconcepts")) types.Add((int)ItemType.OntologyConcept);
				postTypeNode.Name = "itemTypes";
				postTypeNode.InnerHtml = String.Join(",", types);
				//Trace.WriteLine("asdf");
			}

			foreach (HtmlNode issueStatusNode in doc.DocumentNode.SelectNodes("//issueStatus") ?? new HtmlNodeCollection(null)) {
				string statusValues = issueStatusNode.InnerText.DecodeXMLString();
				HashSet<int> tags = new HashSet<int>() { _tagIdCommits, _tagIdCustomSources, _tagIdEmails, _tagIdForums, _tagIdSourceCode, _tagIdWikis } ;
				//tags.Add(_tagIdNonIssue);		// this is needed so that we still keep nonissues as possible results
				foreach (string statusValue in statusValues.Split(",".ToCharArray(), StringSplitOptions.RemoveEmptyEntries)) {
					string status = statusValue.Trim();
					int tagId = MailData.GetTagId(_tagIdIssuesStatus, status);
					if (tagId != TagInfoBase.InvalidTagId)
						tags.Add(tagId);
					else
						AddEvent("Unable to find a tag for status: " + status);
				}
				issueStatusNode.Name = "tagIds";
				issueStatusNode.InnerHtml = String.Join(",", tags);
			}

			foreach (HtmlNode issueResolutionNode in doc.DocumentNode.SelectNodes("//issueResolution") ?? new HtmlNodeCollection(null)) {
				string resolutionValues = issueResolutionNode.InnerText.DecodeXMLString();
				HashSet<int> tags = new HashSet<int>() { _tagIdCommits, _tagIdCustomSources, _tagIdEmails, _tagIdForums, _tagIdSourceCode, _tagIdWikis };
				//tags.Add(_tagIdNonIssue);		// this is needed so that we still keep nonissues as possible results
				foreach (string resolutionValue in resolutionValues.Split(",".ToCharArray(), StringSplitOptions.RemoveEmptyEntries)) {
					string resolution = resolutionValue.Trim();
					int tagId = MailData.GetTagId(_tagIdIssuesResolution, resolution);
					if (tagId != TagInfoBase.InvalidTagId)
						tags.Add(tagId);
					//else
					//	AddEvent("Unable to find a tag for resolution: " + resolution);
				}
				issueResolutionNode.Name = "tagIds";
				issueResolutionNode.InnerHtml = String.Join(",", tags);
			}

            foreach (HtmlNode bugIdsNode in doc.DocumentNode.SelectNodes("//bugIds") ?? new HtmlNodeCollection(null))
            {
                string bugIdsStr = bugIdsNode.InnerText.DecodeXMLString();
                HashSet<int> tags = new HashSet<int>();
                foreach (string bugIdValue in bugIdsStr.Split(",".ToCharArray(), StringSplitOptions.RemoveEmptyEntries))
                {
                    string bugId = bugIdValue.Trim();
                    int tagId = GetTagIdForBugId(bugId);
                    if (tagId != TagInfoBase.InvalidTagId)
                        tags.Add(tagId);
                    else
                        AddEvent("Unable to find a tag for issue: " + bugId);
                }
                bugIdsNode.Name = "tagIds";
                bugIdsNode.InnerHtml = String.Join(",", tags);
            }

			foreach (HtmlNode bugUrisNode in doc.DocumentNode.SelectNodes("//bugUris") ?? new HtmlNodeCollection(null)) {
				HashSet<int> tags = new HashSet<int>();
				foreach (HtmlNode bugUriNode in bugUrisNode.SelectNodes(".//bugUri") ?? new HtmlNodeCollection(null)) {
					string bugUri = bugUriNode.InnerText.DecodeXMLString();
					int tagId = MailData.GetTagId(bugUri);
					if (tagId != TagInfoBase.InvalidTagId)
						tags.Add(tagId);
					else
						AddEvent("Unable to find a tag for issue: " + bugUri);
				}
				bugUrisNode.Name = "tagIds";
				bugUrisNode.InnerHtml = String.Join(",", tags);
			}

			string editedQuery = doc.DocumentNode.OuterHtml;

			QueryBase query = QueryBase.CreateQuery(editedQuery);
			QArgs qArgs = query.GetQueryArgs();
			List<QCond> editedConditions = new List<QCond>();
			for (int i = 0; i < qArgs.Conditions.Count; i++) {
				QCond c = qArgs.Conditions[i];
				if (c as QEnrychableKeywordsCond == null){
					editedConditions.Add(c);
					continue;
				}
				QEnrychableKeywordsCond cond = qArgs.Conditions[i] as QEnrychableKeywordsCond;
				string keywords = cond.Keywords;
				
				// todo: remove this!!!! testin only
				//cond.Optional = true;

				string ignoresReg = @"(^|\s| )-(?<word>\w+)";
				// first, remove the -keywords. we put them into qArgs.Ignore
				IEnumerable<string> ignores = from Match match in Regex.Matches(keywords, ignoresReg) select match.Groups["word"].Value;
				if (ignores != null && ignores.Count() > 0) {
					qArgs.AddIgnore(new QKeywordCond(ignores));
				}
				
				string kwsToAnnotate = Regex.Replace(keywords, ignoresReg, " RNDSTRINGREWQWE ");
				string annotatedKws = AnnotateWithOntologyConcepts(kwsToAnnotate, null);
				
				keywords = Regex.Replace(keywords, ignoresReg, "  ");
				
				string mandatoryReg = @"(^|\s| )\+(?<word>\w+)";
				IEnumerable<string> mandatories = from Match match in Regex.Matches(keywords, mandatoryReg) select match.Groups["word"].Value;
				if (mandatories != null && mandatories.Count() > 0)
					editedConditions.Add(new QKeywordCond(from kw in mandatories select new QKeywordCond.QKeyword(kw), optional: false));

				keywords = Regex.Replace(keywords, mandatoryReg, "  ");
				HashSet<string> words = new HashSet<string>();
				foreach (string kw in keywords.ToLower().Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries))
					words.Add(kw);

				XmlDocument annotatedKwsDoc = new XmlDocument();
				annotatedKwsDoc.LoadXml(annotatedKws);

				if (cond.Optional == true) {
					// add all the keywords to the conditions - if optional == false, we will afterwards add additional conditions
					foreach (var conceptNode in annotatedKwsDoc.DocumentNode.SelectNodes("/concept") ?? new HtmlNodeCollection(null)) {
						var labelsList = _ao.GetConceptLabels(conceptNode.GetAttributeValue("id", ""));
						// each concept can have very similar labels that contain the same words. in order not to put too much influence on those words use
						// each word only once
						foreach (string label in labelsList)
							foreach (string word in label.ToLower().Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries))
								words.Add(word);
					}
					if (words.Count > 0)
						editedConditions.Add(new QKeywordCond(from kw in words select new QKeywordCond.QKeyword(kw), optional: true));
				}
				else {
					foreach (var conceptNode in annotatedKwsDoc.DocumentNode.SelectNodes("/concept") ?? new HtmlNodeCollection(null)) {
						string conceptText = conceptNode.InnerText.DecodeXMLString();
						foreach (string word in conceptText.Split(" ".ToCharArray(), StringSplitOptions.RemoveEmptyEntries))
							words.Remove(word);
						string conceptUri = conceptNode.GetAttributeValue("id", "");
						if (!string.IsNullOrEmpty(conceptUri))
							editedConditions.Add(new QConceptCond(new QConceptCond.QConcept(conceptUri)));
					}
					// add the non-annotated keywords to conditions
					if (words.Count > 0)
						editedConditions.Add(new QKeywordCond(from kw in words select new QKeywordCond.QKeyword(kw), optional: cond.Optional));
				}
			}
			// replace the old conditions with the edited ones
			qArgs.Conditions.Clear();
			qArgs.AddConditions(editedConditions);

			return query.ToString();
		}

		//private QCond FixKeywordCondition(QCond cond)
		//{
		//	// don't change conditions that are not enrychable keyword conditions
		//	if (cond as QEnrychableKeywordsCond == null)
		//		return cond;

		//	QEnrychableKeywordsCond oldCond = cond as QEnrychableKeywordsCond;

		//}

		private string ChangeBugIdToThreadId(string requestData)
		{
			XmlDocument doc = new XmlDocument();
			doc.LoadXml(requestData);
			HtmlNode bugNode = doc.DocumentNode.SelectSingleNode("//query/params");
			if (bugNode == null)
				return requestData;
			// do we have to find the theadId for the specified bugId?
			if (bugNode.GetAttributeValue("type", "") == "similarThreads" && bugNode.GetAttributeValue("threadId", "") == "-1") {
				string bugId = bugNode.GetAttributeValue("bugId", "");
				foreach (var trackerTag in MailData.GetTags(_tagIdIssues)) {
					HtmlNode node = GetItemDataForBugDescription(trackerTag.TagName, bugId);
					if (node != null) {
						string threadId = node.GetAttributeValue("threadId", "");
						if (!string.IsNullOrEmpty(threadId))
							bugNode.SetAttributeValue("threadId", threadId);
					}
				}
			}
			return doc.DocumentNode.InnerHtml;
		}

		public delegate void UpdateTotalTimeDelegate(TimeSpan timeSpan);
		public UpdateTotalTimeDelegate UpdateTotalTimeHandler = null;

		public void UpdateTotalTime(TimeSpan timeSpan)
		{
			if (UpdateTotalTimeHandler != null)
				UpdateTotalTimeHandler(timeSpan);
		}

		private string GetNodeInnerHtml(HtmlNode node, string xpath, string noneValue = null)
		{
			if (node == null)
				return noneValue;
			var n = node.SelectSingleNode(xpath);
			if (n != null)
				return n.InnerHtml;
			return noneValue;
		}

		private string GetNodeInnerText(HtmlNode node, string xpath, string noneValue = null)
		{
			if (node == null)
				return noneValue;
			var n = node.SelectSingleNode(xpath);
			if (n == null)
				return noneValue;
			string decoded = DecodeXMLString(n.InnerText);
			decoded = decoded.Trim();		// remove any possible extra spaces or newline chars
			return decoded;
		}

		//private string GetNodeInnerHtml(HtmlNode node, string xpath, string noneValue = null)
		//{
		//    var n = node.SelectSingleNode(xpath);
		//    if (n != null)
		//        return n.InnerHtml;
		//    return noneValue;
		//}

		private string GetXmlForPerson(string author, string authorUri, string role = "from", EAccountType type = EAccountType.Custom, EPersonNameTrust trust = EPersonNameTrust.Low)
		{
			if (string.IsNullOrEmpty(author) || string.IsNullOrEmpty(authorUri)) {
				AddEvent(String.Format("Didn't find valid values for person. Name={0}, Uri={1}. Ignoring the person", author, authorUri));
				return "";
			}
			return Templates.BuildPerson(authorUri, author, role, type, trust);
		}

		public void PrintTags(int tagId, string prefix = "")
		{
			var t = MailData.GetTagInfo(tagId);
			if (t != null)
				AddEvent(prefix + t.TagName + " | " + t.TagIdStr);
			foreach (var tag in MailData.GetTags(tagId))
				PrintTags(tag.TagId, prefix + "  ");
		}
		#endregion
	}
}
