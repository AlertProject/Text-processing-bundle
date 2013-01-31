using GenLib.Text;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Contextify.Shared.Types;
using System.Diagnostics;

namespace Contextify.Shared.Base
{
	public partial class Templates
	{
		//public enum EQueryType
		//{
		//	GeneralQuery = 0,
		//	CustomQuery
		//}
		public enum ResultTypeEnum { itemData, peopleData, timelineData, keywordData, itemIdData };

		public static string BuildSetDataSetItemStateForTagId(int tagId, ESQLItemState newState)
		{
			return String.Format("<data target=\"MailData\" content=\"ItemStateForTagId\" tagId=\"{0}\" newState=\"{1}\" />", tagId, (short)newState);
		}

		public static string BuildSetDataSetItemFolderId(string entryId, int folderId)
		{
			return String.Format("<data target=\"MailData\" content=\"ItemFolderId\" entryId=\"{0}\" folderId=\"{1}\" />", entryId.EncodeXMLString(), folderId);
		}

		public static string BuildSetDataSetOwnerAccount(string emailAddress)
		{
			return String.Format("<data target=\"PeopleData\" content=\"SetOwnerAccount\" emailAddress=\"{0}\" />", emailAddress.EncodeXMLString());
		}

		public static string BuildSetDataPotentialOwner(string emailAddress)
		{
			return String.Format("<data target=\"PeopleData\" content=\"PotentialOwner\" emailAddress=\"{0}\" />", emailAddress.EncodeXMLString());
		}

		public static string BuildSetDataNewPersonName(int personId, string newName, EPersonNameTrust nameTrust)
		{
			return String.Format("<data target=\"PeopleData\" content=\"NewName\" personId=\"{0}\" newName=\"{1}\" nameTrust=\"{2}\" />", personId, newName.EncodeXMLString(), (int)nameTrust);
		}

		public static string BuildSetDataAddPerson(string personName, string account, EAccountType accountType, EPersonNameTrust nameTrust)
		{
			return String.Format("<data target=\"PeopleData\" content=\"AddPerson\" personName=\"{0}\" account=\"{1}\" accountType=\"{2}\" nameTrust=\"{3}\" />", personName.EncodeXMLString(), account.EncodeXMLString(), (int)accountType, (int)nameTrust);
		}

		public static string BuildSetDataSetPersonInfo(int personId, string property, string value)
		{
			return String.Format("<data target=\"PeopleData\" content=\"SetPersonInfo\" personId=\"{0}\" property=\"{1}\" value=\"{2}\" />", personId, property, value.EncodeXMLString());
		}

		public static string BuildSetDataSetDomainInfo(string domain, string property, string value)
		{
			return String.Format("<data target=\"DomainData\" content=\"SetDomainInfo\" domain=\"{0}\" {1}=\"{2}\" />", domain.EncodeXMLString(), property, value.EncodeXMLString());
		}

		public static string BuildSetDataSetDomainInfo(string domain, string linkedInCompanyId, string customName, string customNameAbbr, bool isManuallyCleared)
		{
			return String.Format("<data target=\"DomainData\" content=\"SetDomainInfo\" domain=\"{0}\" linkedInCompanyId=\"{1}\" customName=\"{2}\" customNameAbbr=\"{3}\" isManuallyCleared=\"{4}\" />", domain.EncodeXMLString(), linkedInCompanyId.EncodeXMLString(), customName.EncodeXMLString(), customNameAbbr.EncodeXMLString(), isManuallyCleared ? "1" : "0");
		}

		//public static string BuildSetDataSetDomainInfo(int domainId, string linkedInCompanyId, string value)
		//{
		//    return String.Format("<data target=\"DomainData\" content=\"SetDomainInfo\" domainId=\"{0}\" {1}=\"{2}\" />", domainId, property, value.EncodeXMLString());
		//}

		public static string BuildSetDataCreateTag(string tagName, string tagIdStr, int parentTagId)
		{
			return String.Format("<data target=\"TagData\" content=\"CreateTag\" tagName=\"{0}\" tagIdStr=\"{1}\" parentTagId=\"{2}\" />", tagName.EncodeXMLString(), tagIdStr.EncodeXMLString(), parentTagId);
		}

		public static string BuildSetDataRenameTag(int tagId, string tagName)
		{
			return String.Format("<data target=\"TagData\" content=\"RenameTag\" tagId=\"{0}\" tagName=\"{1}\" />", tagId, tagName.EncodeXMLString());
		}

		public static string BuildSetDataDeleteTag(int tagId)
		{
			return String.Format("<data target=\"TagData\" content=\"DeleteTag\" tagId=\"{0}\" />", tagId);
		}

		public static string BuildSetDataAssignTag(int tagId, int itemId)
		{
			return String.Format("<data target=\"TagData\" content=\"AssignTag\" tagId=\"{0}\" itemIds=\"{1}\" />", tagId, itemId);
		}

		public static string BuildSetDataAssignTag(int tagId, IEnumerable<int> itemIds)
		{
			string itemIdsStr = string.Join(",", itemIds);
			return String.Format("<data target=\"TagData\" content=\"AssignTag\" tagId=\"{0}\" itemIds=\"{1}\" />", tagId, itemIdsStr);
		}

		public static string BuildSetDataRemoveTag(int tagId, int itemId)
		{
			return String.Format("<data target=\"TagData\" content=\"RemoveTag\" tagId=\"{0}\" itemIds=\"{1}\" />", tagId, itemId);
		}

		public static string BuildSetDataRemoveTag(int tagId, List<int> itemIds)
		{
			string itemIdsStr = string.Join(",", itemIds);
			return String.Format("<data target=\"TagData\" content=\"RemoveTag\" tagId=\"{0}\" itemIds=\"{1}\" />", tagId, itemIdsStr);
		}

		public static string BuildSetDataAddGroup(string groupName)
		{
			return String.Format("<data target=\"PeopleData\" content=\"AddGroup\" groupName=\"{0}\" />", groupName.EncodeXMLString());
		}

		public static string BuildSetDataRemoveGroup(string groupName)
		{
			return String.Format("<data target=\"PeopleData\" content=\"RemoveGroup\" groupName=\"{0}\" />", groupName.EncodeXMLString());
		}

		public static string BuildSetDataAddPersonToGroup(string groupName, int personId)
		{
			return String.Format("<data target=\"PeopleData\" content=\"AddPersonToGroup\" groupName=\"{0}\" personId=\"{1}\" />", groupName.EncodeXMLString(), personId);
		}

		public static string BuildSetDataRemovePersonFromGroup(string groupName, int personId)
		{
			return String.Format("<data target=\"PeopleData\" content=\"RemovePersonFromGroup\" groupName=\"{0}\" personId=\"{1}\" />", groupName.EncodeXMLString(), personId);
		}

		public static string BuildSetDataRenameGroup(string oldName, string newName)
		{
			return String.Format("<data target=\"PeopleData\" content=\"RenameGroup\" oldName=\"{0}\" newName=\"{1}\" />", oldName.EncodeXMLString(), newName.EncodeXMLString());
		}

		public static string BuildCommand(string target, string command)
		{
			return String.Format("<commandData target=\"{0}\" command=\"{1}\" />", target.EncodeXMLString(), command.EncodeXMLString());
		}

		public static string BuildCommandCleanMergeContacts(bool cleanNames, bool mergeContacts)
		{
			return String.Format("<commandData target=\"PeopleData\" command=\"CleanMergeContacts\" cleanNames=\"{0}\" mergeContacts=\"{1}\" />", cleanNames ? 1 : 0, mergeContacts ? 1 : 0);
		}

		public static string BuildCommandReassignAccountToNewAccount(int accountId)
		{
			return String.Format("<commandData target=\"PeopleData\" command=\"ReassignAccountToNewAccount\" accountId=\"{0}\" />", accountId);
		}

		public static string BuildCommandReassignAccountToExistingAccount(int accountId, int fromPersonId, int toPersonId)
		{
			return String.Format("<commandData target=\"PeopleData\" command=\"ReassignAccountToExistingAccount\" accountId=\"{0}\" fromPersonId=\"{1}\" toPersonId=\"{2}\" />", accountId, fromPersonId, toPersonId);
		}

		public static string BuildCommandMergePersons(int fromPersonId, int toPersonId)
		{
			return String.Format("<commandData target=\"PeopleData\" command=\"MergePersons\" fromPersonId=\"{0}\" toPersonId=\"{1}\" />", fromPersonId, toPersonId);
		}

		//public static string BuildCommandGetKeywordsUsingKMeans(int k = 5, int keywordCount = 100, bool computeOnThreads = false, string queryArgs = null, int rndSeed = 1, int clustTrials = 1, int convergEps = 10, double cutWordWgtSumPrc = 0.5, int mnWordFq = 5)
		//{
		//	string command = String.Format("<commandData target=\"Miner\" command=\"keywordsUsingKMeans\" k=\"{0}\" keywordCount=\"{1}\" rndSeed=\"{2}\" clustTrials=\"{3}\" convergEps=\"{4}\" cutWordWgtSumPrc=\"{5}\" mnWordFq=\"{6}\" computeOnThreads = \"{7}\" >", k, keywordCount, rndSeed, clustTrials, convergEps, cutWordWgtSumPrc, mnWordFq, computeOnThreads ? "1" : "0");
		//	if (queryArgs != null)
		//		command += queryArgs;
		//	command += "</commandData>";
		//	return command;
		//}

		//public static string BuildCommandGetKeywordsUsingHKMeans(int mnDocsPerCluster = 50, int mxDocsPerCluster = 1000, int keywordCount = 100, bool computeOnThreads = false, string queryArgs = null, int rndSeed = 1, int clustTrials = 1, int convergEps = 10, double cutWordWgtSumPrc = 0.5, int mnWordFq = 5)
		//{
		//	string command = String.Format("<commandData target=\"Miner\" command=\"keywordsUsingHKMeans\" mnDocsPerCluster=\"{0}\" mxDocsPerCluster=\"{1}\" keywordCount=\"{2}\" rndSeed=\"{3}\" clustTrials=\"{4}\" convergEps=\"{5}\" cutWordWgtSumPrc=\"{6}\" mnWordFq=\"{7}\" computeOnThreads = \"{8}\" >", mnDocsPerCluster, mxDocsPerCluster, keywordCount, rndSeed, clustTrials, convergEps, cutWordWgtSumPrc, mnWordFq, computeOnThreads ? "1" : "0");
		//	if (queryArgs != null)
		//		command += queryArgs;
		//	command += "</commandData>";
		//	return command;
		//}

		//public static string BuildCommandGetKeywordsUsingSVM(string positiveExamples, string negativeExamples, int keywordCount = 100, int timeLimit = 20)
		//{
		//	String command = String.Format("<commandData target=\"Miner\" command=\"keywordsUsingSVM\" keywordCount=\"{0}\" timeLimit=\"{1}\" >\n", keywordCount, timeLimit);
		//	command += "<positiveExamples>" + positiveExamples + "</positiveExamples>\n";
		//	command += "<negativeExamples>" + negativeExamples + "</negativeExamples>\n";
		//	command += "</commandData>";
		//	return command;
		//}
			
		public static string BuildCommandComputeBowWgt()
		{
			return "<commandData target=\"Miner\" command=\"computeBowWgt\" />";
		}

		//public static string BuildCommandGetNGrams(int mnNGramFq = 10, int mnNGramLen = 2, int mxNGramCount = 500)
		//{
		//	return String.Format("<commandData target=\"Miner\" command=\"nGrams\" mnNGramFq=\"{0}\" mnNGramLen=\"{1}\" mxNGramCount=\"{2}\"  />", mnNGramFq, mnNGramLen, mxNGramCount);
		//}

		/// <summary>
		/// remove the specified words from BowWgt. Remove their importance from computing similarities between items
		/// </summary>
		/// <param name="words">words separated by a space</param>
		/// <returns>empty</returns>
		public static string BuildCommandIgnoreBowWgtWords(string words)
		{
			return String.Format("<commandData target=\"Miner\" command=\"ignoreBowWgtWords\" >{0}</commandData>", words);
		}
		
		public static string BuildPerson(string account, string name, string role, EAccountType accountType, EPersonNameTrust nameTrust)
		{
			Debug.Assert(!string.IsNullOrEmpty(account));
			Debug.Assert(name != null);
			return String.Format("<person account=\"{0}\" name=\"{1}\" role=\"{2}\" accountType=\"{3}\" nameTrust=\"{4}\" />", account.EncodeXMLString(), name.EncodeXMLString(), role, (int)accountType, (int)nameTrust);
		}

		public static string BuildPerson(int accountId, string role)
		{
			return String.Format("<person id=\"{0}\" role=\"{1}\" />", accountId, role);
		}

		public static string BuildRequestData(string source, string content)
		{
			return String.Format("<query source=\"{0}\" content=\"{1}\" />", source.EncodeXMLString(), content.EncodeXMLString());
		}

		public static string BuildRequestAccountData(string account, EAccountType accountType)
		{
			return BuildRequestData("PeopleData", "GetContactData", "account", account, "accountType", ((int)accountType).ToString());
		}

		public static string BuildRequestAccountData(int accountId)
		{
			return BuildRequestData("PeopleData", "GetContactData", "accountId", accountId.ToString());
		}

		public static string BuildRequestPersonData(int personId)
		{
			return BuildRequestData("PeopleData", "GetContactData", "personId", personId.ToString());
		}

		public static string BuildRequestItemsForTagId(int tagId)
		{
			return BuildRequestData("TagData", "GetItemsForTagId", "tagId", tagId.ToString());
		}

		public static string BuildRequestTagsForItemId(int itemId)
		{
			return BuildRequestData("TagData", "GetTagsForItemId", "itemId", itemId.ToString());
		}

		public static string BuildRequestData(string source, string content, string arg1, string val1)
		{
			return String.Format("<query source=\"{0}\" content=\"{1}\" {2}=\"{3}\" />", source.EncodeXMLString(), content.EncodeXMLString(), arg1, val1.EncodeXMLString());
		}

		public static string BuildRequestData(string source, string content, string arg1, string val1, string arg2, string val2)
		{
			return String.Format("<query source=\"{0}\" content=\"{1}\" {2}=\"{3}\" {4}=\"{5}\" />", source.EncodeXMLString(), content.EncodeXMLString(), arg1, val1.EncodeXMLString(), arg2, val2.EncodeXMLString());
		}

		public static string BuildRequestData(string source, string content, Dictionary<string, string> optionalArgs = null)
		{
			string query = String.Format("<query source=\"{0}\" content=\"{1}\" ", source.EncodeXMLString(), content.EncodeXMLString());
			if (optionalArgs != null)
			{
				foreach (string key in optionalArgs.Keys)
					query += String.Format("{0}=\"{1}\" ", key, optionalArgs[key].EncodeXMLString());
			}
			query += " />";
			return query;
		}

		//public static string BuildCustomQueryGetSimilarThreads(int threadId, int maxCount = 100, List<int> tagIds = null, List<int> itemIds = null, bool includeItemIdsForThread = false, bool includeItemData = false, bool includeOnlyFirstInThread = true, int maxItemCount = 50, int itemOffset = 0, int itemDataSnipLen = 200, bool includePeopleData = false)
		//{
		//	string query = String.Format("<query type=\"similarThreads\" threadId=\"{0}\" count=\"{1}\" includeItemIds=\"{2}\" includeItemData=\"{3}\" includeOnlyFirstInThread=\"{4}\" maxCount=\"{5}\" offset=\"{6}\" itemDataSnipLen=\"{7}\" includePeopleData=\"{8}\" >", threadId, maxCount, includeItemIdsForThread ? 1 : 0, includeItemData ? 1 : 0, includeOnlyFirstInThread ? 1 : 0, maxItemCount, itemOffset, itemDataSnipLen, includePeopleData ? 1 : 0);
		//	if (tagIds != null)
		//		query += String.Format("<conditions><tagIds>{0}</tagIds></conditions>", string.Join(",", tagIds));
		//	if (itemIds != null)
		//		query += String.Format("<conditions><itemIds>{0}</itemIds></conditions>", string.Join(",", itemIds));
		//	query += "</query>";
		//	return query;
		//}
		
		//public static string BuildCustomQueryGetSimilarItems(int itemId, int maxCount = 100, List<int> candidateTagIds = null, List<int> candidateItemIds = null, bool includeItemData = false, int maxItemCount = 50, int itemOffset = 0, int itemDataSnipLen = 200, bool includePeopleData = false)
		//{
		//	string query = String.Format("<query type=\"similarItems\" itemId=\"{0}\" maxCount=\"{1}\" includeItemData=\"{2}\" maxCount=\"{3}\" offset=\"{4}\" itemDataSnipLen=\"{5}\" includePeopleData=\"{6}\">", itemId, maxCount, includeItemData ? 1 : 0, maxItemCount, itemOffset, itemDataSnipLen, includePeopleData ? 1 : 0);
		//	if (candidateTagIds != null)
		//		query += String.Format("<conditions><tagIds>{0}</tagIds></conditions>", string.Join(",", candidateTagIds));
		//	if (candidateItemIds != null)
		//		query += String.Format("<conditions><itemIds>{0}</itemIds></conditions>", string.Join(",", candidateItemIds));
		//	query += "</query>";
		//	return query;
		//}

		//public static string BuildCustomQueryGetSimilarItems(string textToCompare, int maxCount = 100, List<int> candidateTagIds = null, List<int> candidateItemIds = null)
		//{
		//	string query = String.Format("<query type=\"similarItems\" maxCount=\"{0}\" ><textToCompare>{1}</textToCompare>", maxCount, textToCompare.EncodeXMLString());
		//	if (candidateTagIds != null)
		//		query += String.Format("<conditions><tagIds>{0}</tagIds></conditions>", string.Join(",", candidateTagIds));
		//	if (candidateItemIds != null)
		//		query += String.Format("<conditions><itemIds>{0}</itemIds></conditions>", string.Join(",", candidateItemIds));
		//	query += "</query>";
		//	return query;
		//}

		//public static string BuildCustomQueryGetSimilarItemsUsingAnnotations(int itemId, int maxCount = 100, List<int> candidateTagIds = null, List<int> candidateItemIds = null)
		//{
		//	string query = String.Format("<query type=\"similarItemsUsingAnnotations\" itemId=\"{0}\" maxCount=\"{1}\" >", itemId, maxCount);
		//	if (candidateTagIds != null)
		//		query += String.Format("<conditions><tagIds>{0}</tagIds></conditions>", string.Join(",", candidateTagIds));
		//	if (candidateItemIds != null)
		//		query += String.Format("<conditions><itemIds>{0}</itemIds></conditions>", string.Join(",", candidateItemIds));
		//	query += "</query>";
		//	return query;
		//}

		//public static string BuildCommandGetClassSeparation(string positiveExamples, string trainExamples = null, int folds = 10, int timeLimit = -1, int rndSeed = 1, double j = 1.0)
		//{
		//	String command = String.Format("<commandData target=\"Miner\" command=\"classSeparation\" folds=\"{0}\" rndSeed=\"{1}\" timeLimit=\"{2}\" j=\"{3}\" >", folds, rndSeed, timeLimit, j);
		//	command += "<positiveExamples>" + positiveExamples + "</positiveExamples>";
		//	if (trainExamples != null)
		//		command += "<trainExamples>" + trainExamples + "</trainExamples>";
		//	command += "</commandData>";
		//	return command;
		//}

		public static string BuildConceptsStrFromDict(Dictionary<string, double> conceptsToWeight)
		{
			string output = "";
			foreach (string uri in conceptsToWeight.Keys)
				output += String.Format("<concept uri=\"{0}\" weight=\"{1}\" />", uri.EncodeXMLString(), conceptsToWeight[uri].ToString(System.Globalization.NumberFormatInfo.InvariantInfo));
			return output;
		}

		public static string ItemTemplate = @"<item accountId=""{accountId}"" itemType=""{itemType}"" time=""{time}"" indexText=""{indexText}"" {optionalItemArgs}>
<subject>{subject}</subject>
<thread>{thread}</thread>
<entryId>{entryId}</entryId>
<people>{people}</people>
<attachments>{attachments}</attachments>
<links>{links}</links>
<textToIndex>{textToIndex}</textToIndex>
<metaData>{metaData}</metaData>
<relatedItems>{relatedItems}</relatedItems>
<tags>{tags}</tags>
<concepts>{concepts}</concepts>
</item>";

//        public static string ItemTemplate = @"<item accountId=""{0}"" itemType=""{1}"" time=""{2}"" indexText=""{14}"">
//<thread>{3}</thread>
//<subject>{4}</subject>
//<entryId>{5}</entryId>
//<people>{6}</people>
//<attachments>{7}</attachments>
//<links>{8}</links>
//<textToIndex>{9}</textToIndex>
//<metaData>{10}</metaData>
//<relatedItems>{11}</relatedItems>
//<tags>{12}</tags>
//<concepts>{13}</concepts>
//</item>";

		public static string DocumentTemplate = @"<item accountId=""{accountId}"" itemType=""{itemType}"" time=""{time}"" {optionalItemArgs}>
<entryId>{entryId}</entryId>
<people>{people}</people>
<textToIndex>{textToIndex}</textToIndex>
<metaData>{metaData}</metaData>
<relatedItems>{relatedItems}</relatedItems>
<tags>{tags}</tags>
</item>";

		public static string FBWallPostTemplate = @"<item itemId=""{itemId}"" itemType=""{itemType}"" time=""{time}"">
<entryId>{entryId}</entryId>
<people>{people}</people>
<links>{links}</links>
<textToIndex>{textToIndex}</textToIndex>
<tags>{tags}</tags>
</item>";

		public static string FBMessageTemplate = @"<item itemId=""{itemId}"" itemType=""{itemType}"" time=""{time}"">
<thread>{subject}</thread>
<subject>{subject}</subject>
<entryId>{entryId}</entryId>
<people>{people}</people>
<links>{links}</links>
<textToIndex>{textToIndex}</textToIndex>
<tags>{tags}</tags>
</item>";

		public static string TweetTemplate = @"<item itemId=""{itemId}"" itemType=""{itemType}"" time=""{time}"">
<entryId>{entryId}</entryId>
<people>{people}</people>
<links>{links}</links>
<textToIndex>{textToIndex}</textToIndex>
<tags>{tags}</tags>
</item>";

//		public static string BuildItemQueryInfo(string queryArgs, int offset, int maxCount, bool includeAttachments = true, string sortBy = "dateDesc", int itemDataSnipLen = 200, bool snipMatchKeywords = true, int keywordMatchOffset = 25, bool includePeopleData = false)
//		{
//			var obj = new { queryArgs = queryArgs, offset = offset, maxCount = maxCount, resultData = ResultTypeEnum.itemData, includeAttachments = includeAttachments ? 1 : 0, sortBy = sortBy, itemDataSnipLen = itemDataSnipLen, snipMatchKeywords = snipMatchKeywords ? 1 : 0, keywordMatchOffset = keywordMatchOffset, includePeopleData = includePeopleData ? 1 : 0 };
//			return @"<query><queryArgs>{queryArgs}</queryArgs>
//				<params	offset=""{offset}"" maxCount=""{maxCount}"" resultData=""{resultData}"" includeAttachments=""{includeAttachments}"" 
//				sortBy=""{sortBy}"" itemDataSnipLen=""{itemDataSnipLen}"" snipMatchKeywords=""{snipMatchKeywords}"" keywordMatchOffset=""{keywordMatchOffset}"" includePeopleData=""{includePeopleData}""/></query>".Format(obj);
//		}

		//public static string BuildPeopleQueryInfo(string queryArgs, int maxCountItems = 1000, bool includePeopleData = false)
		//{
		//	return String.Format(@"<query><queryArgs>{0}</queryArgs><params resultData=""{1}"" maxCountItems=""{2}"" includePeopleData=""{3}"" /></query>", queryArgs, ResultTypeEnum.peopleData, maxCountItems, includePeopleData ? 1 : 0);
		//}

		//public static string BuildTimelineQueryInfo(string queryArgs)
		//{
		//	return String.Format(@"<query><queryArgs>{0}</queryArgs><params resultData=""{1}"" /></query>", queryArgs, ResultTypeEnum.timelineData);
		//}

		//public static string BuildKeywordQueryInfo(string queryArgs, int keywordCount = 30, int sampleSize = -1, string keywordMethod = "localConceptSpV", string keywordSource = "text")
		//{
		//	return String.Format(@"<query><queryArgs>{0}</queryArgs><params resultData=""keywordData"" keywordCount=""{1}"" sampleSize=""{2}"" keywordMethod=""{3}"" keywordSource=""{4}"" /></query>", queryArgs, keywordCount, sampleSize, keywordMethod, keywordSource);
		//}

		public static string BuildItemInfo(int accountId, DateTime time, string threadName, string subject, string entryId, string people, string attachments = "", string links = "", string textToIndex = "", string metaData = "", string relatedItems = "", IEnumerable<int> tags = null, string concepts = "", bool indexText = true, int itemType = (int)ESQLItemType.Email, string optionalItemArgs = "")
		{
			var obj = new { accountId = accountId, itemType = itemType, time = GenLib.Time.ToUInt64Time(time), subject = subject, thread = threadName, entryId = entryId, people = people, attachments = attachments, links = links, textToIndex = textToIndex, metaData = metaData, relatedItems = relatedItems, tags = BuildTagsStr(tags), concepts = concepts, indexText = indexText ? 1 : 0, optionalItemArgs = optionalItemArgs };
			return ItemTemplate.Format(obj);
		}

		public static string BuildAttachmentInfo(int accountId, DateTime time, string entryId, string people, string textToIndex = "", string metaData = "", string relatedItems = "", IEnumerable<int> tags = null, string optionalItemArgs = "")
		{
			var obj = new { accountId = accountId, itemType = (int)ESQLItemType.Attachment, time = GenLib.Time.ToUInt64Time(time), entryId = entryId, people = people, textToIndex = textToIndex, metaData = metaData, relatedItems = relatedItems, tags = BuildTagsStr(tags), optionalItemArgs = optionalItemArgs };
			return DocumentTemplate.Format(obj);
		}

		public static string BuildDocumentInfo(int accountId, DateTime time, string entryId, string people, string textToIndex = "", string metaData = "", string relatedItems = "", IEnumerable<int> tags = null, string optionalItemArgs = "")
		{
			var obj = new { accountId = accountId, itemType = (int)ESQLItemType.Document, time = GenLib.Time.ToUInt64Time(time), entryId = entryId, people = people, textToIndex = textToIndex, metaData = metaData, relatedItems = relatedItems, tags = BuildTagsStr(tags), optionalItemArgs = optionalItemArgs };
			return DocumentTemplate.Format(obj);
		}

		public static string BuildFBWallPostInfo(int itemId, DateTime time, string entryId, string people, string links, string textToIndex = "", IEnumerable<int> tags = null)
		{
			var obj = new { itemId = itemId, itemType = (int)ESQLItemType.FBWallPost, time = GenLib.Time.ToUInt64Time(time), entryId = entryId, people = people, links = links, textToIndex = textToIndex, tags = BuildTagsStr(tags) };
			return FBWallPostTemplate.Format(obj);
		}

		public static string BuildFBMessageInfo(int itemId, DateTime time, string thread, string entryId, string people, string links, string textToIndex = "", IEnumerable<int> tags = null)
		{
			var obj = new { itemId = itemId, itemType = (int)ESQLItemType.FBMessage, time = GenLib.Time.ToUInt64Time(time), entryId = entryId, subject = thread, people = people, links = links, textToIndex = textToIndex, tags = BuildTagsStr(tags) };
			return FBMessageTemplate.Format(obj);
		}

		public static string BuildTweetInfo(int itemId, DateTime time, string entryId, string people, string links, string textToIndex = "", IEnumerable<int> tags = null)
		{
			var obj = new { itemId = itemId, itemType = (int)ESQLItemType.Tweet, time = GenLib.Time.ToUInt64Time(time), entryId = entryId, people = people, links = links, textToIndex = textToIndex, tags = BuildTagsStr(tags) };
			return TweetTemplate.Format(obj);
		}

		//public static string BuildGetTagDataForItemId(int itemId)
		//{
		//	return String.Format("<tagData itemId=\"{0}\" />", itemId);
		//}

		//public static string BuildGetTagDataForTagId(int tagId)
		//{
		//	return String.Format("<tagData tagId=\"{0}\" />", tagId);
		//}

		private static string BuildTagsStr(IEnumerable<int> tags)
		{
			if (tags == null || tags.Count() == 0) return "";
			return string.Join(",", tags.Where(tag => tag != TagInfoBase.InvalidTagId));
		}
	}
}
