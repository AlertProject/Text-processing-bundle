using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using HtmlAgilityPack;
using System.Diagnostics;
using GenLib.Text;
using Contextify.Shared.Interfaces;
//using System.ComponentModel;

namespace Contextify.Shared.Types
{
	public class QueryBase
	{
		public static QueryBase CreateQuery(string queryStr)
		{
			XmlDocument queryDoc = new XmlDocument();
			queryDoc.LoadXml(queryStr);
			var queryNode = queryDoc.DocumentNode.SelectSingleNode("/query");
			Debug.Assert(queryNode != null);
			var queryParamsNode = queryNode.SelectSingleNode("./params");
			var queryArgsNode = queryNode.SelectSingleNode("./queryArgs");
			var queryArgsNegativeNode = queryNode.SelectSingleNode("./queryArgsNegative");
			if (queryParamsNode == null)
				return null;
			string type = queryNode.GetAttributeValue("type", "");
			if (type == "generalQuery")
				return new GeneralQuery(new QGeneralParams(queryParamsNode), new QArgs(queryArgsNode));
			else if (type == "customQuery")
				return new CustomQuery(QCustomParams.CreateParams(queryParamsNode), QArgs.CreateArgs(queryArgsNode), QArgsNegative.CreateArgs(queryArgsNegativeNode));
			else
				throw new ArgumentException("The query doesn't contain it's type : " + queryStr);
		}

		public override string ToString()
		{
			if (this is GeneralQuery) return (this as GeneralQuery).ToString();
			else if (this is CustomQuery) return (this as CustomQuery).ToString();
			return base.ToString();
		}

		public QArgs GetQueryArgs()
		{
			if (this is GeneralQuery) return (this as GeneralQuery).QueryArgs;
			else if (this is CustomQuery) return (this as CustomQuery).QueryArgs;
			return null;
		}
	}

	
	public delegate QCond CustomCondDelegate(HtmlNode node); 
	public abstract class QCond
	{
		private static Dictionary<string, CustomCondDelegate> _customDelegates = new Dictionary<string, CustomCondDelegate>();
		public static void AddCustomCondDelegate(string name, CustomCondDelegate handler)
		{
			_customDelegates[name] = handler;
		}
		public static void RemoveCustomCond(string name)
		{
			if (_customDelegates.ContainsKey(name))
				_customDelegates.Remove(name);
		}

		public static QCond CreateCond(HtmlNode node)
		{
			if (node.Name == "accounts")
				return new QAccountCond(node);
			else if (node.Name == "keywords")
				return new QKeywordCond(node);
			else if (node.Name == "concepts")
				return new QConceptCond(node);
			else if (node.Name == "timeline")
				return new QTimelineCond(node);
			else if (node.Name == "tagIds")
				return new QTagIdCond(node);
			else if (node.Name == "itemIds")
				return new QItemIdCond(node);
			else if (node.Name == "threads")
				return new QThreadCond(node);
			else if (node.Name == "threadIds")
				return new QThreadIdsCond(node);
			else if (node.Name == "threadStr")
				return new QThreadStrCond(node);
			else if (node.Name == "itemTypes")
				return new QItemTypeCond(node);
			else if (_customDelegates.ContainsKey(node.Name))
				return _customDelegates[node.Name](node);
			else {
				Debug.Assert(false, "Found unknown query type: " + node.OuterHtml);
				return null;
			}
		}

		//public override bool Equals(object obj)
		//{
		//	if (obj == null) return false;
		//	if (obj as QCond == null) return false;
		//	return (obj as QCond).ToString() == this.ToString();
		//}

		//public bool Equals(QCond cond)
		//{
		//	return cond.ToString() == this.ToString();
		//}
	}

	// account conditions
	//<accounts><account id="" role="from,to,cc,bcc,author,_ANY_,...">+</accounts>	// write type for each email but in the interface have the same type for one person			
	public class QAccountCond : QCond
	{
		public class QAccount
		{
			public static string AnyRole { get { return "_ANY_"; } }
			public int Id { get; private set; }
			public string Roles { get; private set; }

			public QAccount(int id, string roles)
			{
				Id = id;
				Roles = roles;
			}
			public override string ToString()
			{
				return String.Format("<account id=\"{0}\" role=\"{1}\" />", Id, Roles);
			}
		}

		public IEnumerable<QAccount> Accounts { get; private set; }

		public QAccountCond(QAccount account)
		{
			Accounts = new[] { account };
		}

		public QAccountCond(IEnumerable<QAccount> accounts)
		{
			Accounts = accounts;
		}

		public QAccountCond(HtmlNode node)
		{
			Debug.Assert(node.Name == "accounts");
			Accounts = from accountNode in node.SelectNodes("./account") ?? new HtmlNodeCollection(null) select new QAccount(accountNode.GetAttributeValue("id", -1), accountNode.GetAttributeValue("role", QAccount.AnyRole));
		}

		public override string ToString()
		{
			if (Accounts == null) return "";
			return "<accounts>" + String.Join("", from a in Accounts select a.ToString()) + "</accounts>\n";
		}
	}

	// keyword conditions
	// <keywords><kw>blabla</kw><kw>bla bla</kw></keywords>		// keywords. OR is done between each <kw>. if sortBy=relevance && resultData==itemData then items are sorted by cosine similarity to all specified keywords
	public class QKeywordCond : QCond
	{
		public class QKeyword
		{
			public string Keyword { get; set; }
			public QKeyword(string keyword)
			{
				Keyword = keyword;
			}
			
			public override string ToString()
			{
				return "<kw>" + Keyword.EncodeXMLString() + "</kw>";
			}
		}

		public IEnumerable<QKeyword> Keywords { get; private set; }
		public bool Optional { get; set; }
		public QKeywordCond(IEnumerable<QKeyword> keywords, bool optional = false)
		{
			Keywords = keywords;
			Optional = optional;
		}

		public QKeywordCond(string singleKeyword, bool optional = false)
		{
			Keywords = new QKeyword[] { new QKeyword(singleKeyword) };
			Optional = optional;
		}

		public QKeywordCond(IEnumerable<string> keywords, bool optional = false)
		{
			string joinedKeywords = String.Join(" ", keywords);
			Keywords = from keyword in keywords select new QKeyword(keyword);
			Optional = optional;
		}

		public QKeywordCond(HtmlNode node)
		{
			Debug.Assert(node.Name == "keywords");
			Keywords = from keywordNode in node.SelectNodes("./kw") ?? new HtmlNodeCollection(null) select new QKeyword(keywordNode.InnerText);
			Optional = node.GetAttributeValue("optional", false);
		}

		public override string ToString()
		{
			if (Keywords == null) return "";
			return String.Format("<keywords optional=\"{0}\">", Optional) + String.Join("", from kw in Keywords select kw.ToString()) + "</keywords>\n";
		}
	}

	// concept conditions
	// <concepts><concept>conceptUri1</concept><concept>conceptUri2</concept></concepts>	// search for concepts
	public class QConceptCond : QCond
	{
		public class QConcept
		{
			public string Concept { get; private set; }
			public QConcept(string concept)
			{
				Concept = concept;
			}

			public override string ToString()
			{
				return "<concept>" + Concept + "</concept>";
			}
		}

		public IEnumerable<QConcept> Concepts { get; private set; }
		public QConceptCond(IEnumerable<QConcept> concepts)
		{
			Concepts = concepts;
		}

		public QConceptCond(QConcept singleConcept)
		{
			Concepts = new QConcept[1] { singleConcept };
		}

		public QConceptCond(HtmlNode node)
		{
			Debug.Assert(node.Name == "concepts");
			Concepts = from conceptNode in node.SelectNodes("./concept") ?? new HtmlNodeCollection(null) select new QConcept(conceptNode.InnerText);
		}

		public override string ToString()
		{
			if (Concepts == null) return "";
			return "<concepts>" + String.Join("", from c in Concepts select c.ToString()) + "</concepts>\n";
		}
	}

	// timeline conditions
	// <timeline><time start=\"{0}\" end=\"{1}\" />+</timeline>		// one or more timeslices - or between them
	public class QTimelineCond : QCond
	{
		public ulong Start { get; private set; }
		public ulong End { get; private set; }

		public QTimelineCond(DateTime start, DateTime end)
		{
			Start = GenLib.Time.ToUInt64Time(start);
			End = GenLib.Time.ToUInt64Time(end);
		}

		public QTimelineCond(ulong start, ulong end)
		{
			Start = start;
			End = end;
		}

		public QTimelineCond(HtmlNode node)
		{
			Debug.Assert(node.Name == "timeline");
			Start = ulong.Parse(node.GetAttributeValue("start", "0"));
			End = ulong.Parse(node.GetAttributeValue("end", "0"));
		}

		public override string ToString()
		{
			return String.Format("<timeline start=\"{0}\" end=\"{1}\" />\n", Start, End);
		}
	}

	// tagid conditions
	// <tagIds>id1,id2,...</tagIds>		// one or more tags (folderId or custom tags) - there is OR between them. to have AND specify multiple <tags>.
	public class QTagIdCond : QCond
	{
		public IEnumerable<int> TagIds { get; private set; }
		public QTagIdCond(int tagId)
		{
			TagIds = new[] { tagId };
		}
		public QTagIdCond(IEnumerable<int> tagIds)
		{
			TagIds = tagIds;
		}

		public QTagIdCond(HtmlNode node)
		{
			Debug.Assert(node.Name == "tagIds");
			TagIds = from tagIdStr in node.InnerText.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries) select int.Parse(tagIdStr);
		}

		public override string ToString()
		{
			return "<tagIds>" + String.Join(",", TagIds) + "</tagIds>\n";
		}
	}
	
	// itemid conditions
	// <itemIds>id1,id2,....</itemIds> 		// individual items
	public class QItemIdCond : QCond
	{
		public IEnumerable<int> ItemIds { get; private set; }
		public QItemIdCond(IEnumerable<int> itemIds)
		{
			ItemIds = itemIds;
		}

		public QItemIdCond(int itemId)
		{
			ItemIds = new[] {itemId};
		}

		public QItemIdCond(HtmlNode node)
		{
			Debug.Assert(node.Name == "itemIds");
			ItemIds = from itemIdStr in node.InnerText.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries) select int.Parse(itemIdStr);
		}

		public override string ToString()
		{
			return "<itemIds>" + String.Join(",", ItemIds) + "</itemIds>\n";
		}
	}

	// threads conditions
	// <threads>itemId1,itemId2, ...</threads>		// for each itemId we select all items in the same thread
	public class QThreadCond : QCond
	{
		public IEnumerable<int> ItemIds { get; private set; }
		public QThreadCond(IEnumerable<int> itemIds)
		{
			ItemIds = itemIds;
		}

		public QThreadCond(int itemId)
		{
			ItemIds = new[] {itemId};
		}

		public QThreadCond(HtmlNode node)
		{
			Debug.Assert(node.Name == "threads");
			ItemIds = from itemIdStr in node.InnerText.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries) select int.Parse(itemIdStr);
		}

		public override string ToString()
		{
			return "<threads>" + String.Join(",", ItemIds) + "</threads>\n";
		}
	}

	// threads conditions
	// <threadIds>threadId1,threadId2, ...</threadIds>		// for each threadId we select all items in the thread
	public class QThreadIdsCond : QCond
	{
		public IEnumerable<int> ThreadIds { get; private set; }
		public QThreadIdsCond(IEnumerable<int> threadIds)
		{
			ThreadIds = threadIds;
		}

		public QThreadIdsCond(int itemId)
		{
			ThreadIds = new[] { itemId };
		}

		public QThreadIdsCond(HtmlNode node)
		{
			Debug.Assert(node.Name == "threadIds");
			ThreadIds = from threadIdStr in node.InnerText.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries) select int.Parse(threadIdStr);
		}

		public override string ToString()
		{
			return "<threadIds>" + String.Join(",", ThreadIds) + "</threadIds>\n";
		}
	}

	// thread condition
	// <threadStr>thread topic</threadStr>		// find all itemIds that belong to the specified thread
	public class QThreadStrCond : QCond
	{
		public string Topic { get; private set; }
		public QThreadStrCond(string topic)
		{
			Topic = topic;
		}

		public QThreadStrCond(HtmlNode node)
		{
			Debug.Assert(node.Name == "threadStr");
			Topic = node.InnerText;
		}

		public override string ToString()
		{
			return "<threadStr>" + Topic + "</threadStr>\n";
		}
	}

	// item type conditions
	// <itemTypes>inttype1,inttype2,...</itemTypes>		// what kind of items are valid results
	public class QItemTypeCond : QCond
	{
		public IEnumerable<int> ItemTypes { get; private set; }
		public QItemTypeCond(int itemType)
		{
			ItemTypes = new[] { itemType };
		}
		public QItemTypeCond(IEnumerable<int> itemTypes)
		{
			ItemTypes = itemTypes;
		}

		public QItemTypeCond(HtmlNode node)
		{
			Debug.Assert(node.Name == "itemTypes");
			ItemTypes = from itemType in node.InnerText.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries) select int.Parse(itemType);
		}

		public override string ToString()
		{
			return "<itemTypes>" + String.Join(",", ItemTypes) + "</itemTypes>\n";
		}
	}

	// class that contains info which items should be a result and which not
	public class QArgs
	{
		public string TagName { get { return "queryArgs"; } }
		public List<QCond> Conditions { get; private set; }
		public List<QCond> Ignore { get; private set; }

		public QArgs()
		{
			Conditions = new List<QCond>();
			Ignore = new List<QCond>();
		}

		public QArgs(QCond singleCondition)
		{
			Conditions = new List<QCond>() { singleCondition };
			Ignore = new List<QCond>() ;
		}
		
		public QArgs(IEnumerable<QCond> conditions, IEnumerable<QCond> ignore)
		{
			Conditions = conditions != null ? conditions.ToList() : new	List<QCond>();
			Ignore = ignore != null ? ignore.ToList() : new List<QCond>();
		}
		public QArgs(string conditions, string ignore)
		{
			HtmlNode conditionsNode = null;
			HtmlNode ignoreNode = null;
			if (!string.IsNullOrEmpty(conditions)) {
				XmlDocument condDoc = new XmlDocument();
				condDoc.LoadXml(conditions);
				conditionsNode = condDoc.DocumentNode;
			}
			if (!string.IsNullOrEmpty(ignore)) {
				XmlDocument ignoreDoc = new XmlDocument();
				ignoreDoc.LoadXml(conditions);
				ignoreNode = ignoreDoc.DocumentNode;
			}
			ParseConditionsAndIgnores(conditionsNode, ignoreNode);
		}

		public QArgs(HtmlNode queryArgsNode)
		{
			if (queryArgsNode == null) {
				Conditions = new List<QCond>();
				Ignore = new List<QCond>();
				return;
			}
			Debug.Assert(queryArgsNode.Name == TagName);
			var conditionsNode = queryArgsNode.SelectSingleNode("./conditions");
			var ignoreNode = queryArgsNode.SelectSingleNode("./ignore");
			ParseConditionsAndIgnores(conditionsNode, ignoreNode);
		}

		public QArgs(HtmlNode conditionsNode, HtmlNode ignoreNode)
		{
			ParseConditionsAndIgnores(conditionsNode, ignoreNode);
		}

		public static QArgs CreateArgs(HtmlNode queryArgsNode)
		{
			if (queryArgsNode == null)
				return new QArgs();
			return new QArgs(queryArgsNode);
		}

		public void ClearConditions()
		{
			Conditions.Clear();
		}

		public void ClearIgnores()
		{
			Ignore.Clear();
		}

		public void AddConditions(IEnumerable<QCond> conditions)
		{
			if (conditions == null) return;
			Conditions.AddRange(conditions);
		}

		public void AddCondition(QCond condition)
		{
			if (condition == null) return;
			Conditions.Add(condition);
		}

		public void AddIgnores(IEnumerable<QCond> ignores)
		{
			if (ignores == null) return;
			Ignore.AddRange(ignores);
		}

		public void AddIgnore(QCond ignore)
		{
			if (ignore == null) return;
			Ignore.Add(ignore);
		}

		private void ParseConditionsAndIgnores(HtmlNode conditionsNode, HtmlNode ignoreNode)
		{
			if (conditionsNode != null)
				Conditions = (from node in conditionsNode.ChildNodes where node.NodeType == HtmlNodeType.Element select QCond.CreateCond(node)).ToList();
			else Conditions = new List<QCond>();
			if (ignoreNode != null)
				Ignore = (from node in ignoreNode.ChildNodes where node.NodeType == HtmlNodeType.Element select QCond.CreateCond(node)).ToList();
			else Ignore = new List<QCond>();
		}

		public override string ToString()
		{
			string ret = "<" + TagName + ">\n";
			if (Conditions != null && Conditions.Count > 0)
				ret += "<conditions>\n" + String.Join("", from c in Conditions where c != null select c.ToString()) + "</conditions>\n";
			if (Ignore != null && Ignore.Count > 0)
				ret += "<ignore>\n" + String.Join("", from i in Ignore where i != null select i.ToString()) + "</ignore>\n";
			ret += "</" + TagName + ">\n";
			return ret;
		}
	}

	public class QArgsNegative : QArgs
	{
		public new string TagName { get { return "queryArgsNegative"; } }
		public QArgsNegative() : base() { }
		public QArgsNegative(QCond singleCondition) : base(singleCondition) {}
		
		public QArgsNegative(IEnumerable<QCond> conditions, IEnumerable<QCond> ignore) : base(conditions, ignore) { }
		
		public QArgsNegative(string conditions, string ignore) : base(conditions, ignore) {}
		public QArgsNegative(HtmlNode queryArgsNode) :base(queryArgsNode) {}

		public QArgsNegative(HtmlNode conditionsNode, HtmlNode ignoreNode) : base (conditionsNode, ignoreNode) {}
		
		public new static QArgsNegative CreateArgs(HtmlNode queryArgsNode)
		{
			if (queryArgsNode == null)
				return new QArgsNegative();
			return new QArgsNegative(queryArgsNode);
		}
	}

	// results can be a combination of the bottom flags
	[Flags]
	public enum QResultData { None = 0, itemData = 0x1, peopleData = 0x2, timelineData = 0x4, keywordData = 0x8, itemIdData = 0x10 }

	public enum QResultSorting { none, relevance, dateAsc, dateDesc, itemIdAsc, itemIdDesc }

	public enum QKeywordSource { text, concepts }
	public enum QKeywordMethod { localConceptSpV, globalConceptSpV, SVM }
	public static class KeywordMethodConverter
	{
		public static QKeywordMethod GetKeywordMethod(string keywordMethod)
		{
			if (keywordMethod == QKeywordMethod.localConceptSpV.ToString())
				return QKeywordMethod.localConceptSpV;
			else if (keywordMethod == QKeywordMethod.globalConceptSpV.ToString())
				return QKeywordMethod.globalConceptSpV;
			else if (keywordMethod == QKeywordMethod.SVM.ToString())
				return QKeywordMethod.SVM;
			return QKeywordMethod.SVM;
		}
	}

	public class QGeneralParams
	{
		//[DefaultValue(QResultData.itemData)]
		public QResultData ResultData { get; set; }
		public bool IncludePeopleData { get; set; }
		public int Offset { get; set; }
		
		// for item data		
		public int MaxCount { get; set; }
		public QResultSorting SortBy { get; set; }
		public int ItemDataSnipLen { get; set; }
		public bool SnipMatchKeywords { get; set; }
		public static string AllRoleTypes = "from,to,cc,bcc,author,originalFrom";
		public string RoleTypes { get; set; }
		public bool IncludeAttachments { get; set; }
		public int KeywordMatchOffset { get; set; }

		// for people data
		public int MaxCountItems { get; set; }		// limit the number of edges returned back. default is 1000
		
		// for keyword data
		public int SampleSize { get; set; }			// resultData=keywordData: the sample size of results used to extract keyword
		public int KeywordCount { get; set; }		// resultData=keywordData: the number of extracted keywords
		public QKeywordSource KeywordSource { get; set; }	// resultData=keywordData: what should be used as a source for extracting keywords - the original text or the annotated concepts
		public QKeywordMethod KeywordMethod { get; set; }	// resultData=keywordData: the method used for extracting keywords		

		// used in the ToString() method
		public string ResultDataStr { get { return String.Join("|", from QResultData enumVal in Enum.GetValues(typeof(QResultData)) where (ResultData & enumVal) != QResultData.None select enumVal); } }

		// constructor for default parameters
		public QGeneralParams() {
			ResultData = QResultData.itemData;
			IncludePeopleData = false;
			Offset = 0;
			MaxCount = 30;
			SortBy = QResultSorting.dateDesc;
			ItemDataSnipLen = 100;
			SnipMatchKeywords = true;
			RoleTypes = AllRoleTypes;
			IncludeAttachments = true;
			KeywordMatchOffset = 25;
			MaxCountItems = 1000;
			SampleSize = -1;
			KeywordCount = 30;
			KeywordSource = QKeywordSource.text;
			KeywordMethod = QKeywordMethod.localConceptSpV;
		}

		public QGeneralParams(HtmlNode paramNode)
		{
			string resultDataStr = paramNode.GetAttributeValue("resultData", "");
			if (resultDataStr.Contains(QResultData.itemData.ToString()))
				ResultData |= QResultData.itemData;
			if (resultDataStr.Contains(QResultData.peopleData.ToString()))
				ResultData |= QResultData.peopleData;
			if (resultDataStr.Contains(QResultData.timelineData.ToString()))
				ResultData |= QResultData.timelineData;
			if (resultDataStr.Contains(QResultData.keywordData.ToString()))
				ResultData |= QResultData.keywordData;
			if (resultDataStr.Contains(QResultData.itemIdData.ToString()))
				ResultData |= QResultData.itemIdData;

			IncludePeopleData = paramNode.GetAttributeValue("includePeopleData", false);
			Offset = paramNode.GetAttributeValue("offset", 0);
			MaxCount = paramNode.GetAttributeValue("maxCount", 30);
			
			string sortByStr = paramNode.GetAttributeValue("sortBy", "none");
			if (sortByStr == QResultSorting.dateAsc.ToString())
				SortBy = QResultSorting.dateAsc;
			else if (sortByStr == QResultSorting.dateDesc.ToString())
				SortBy = QResultSorting.dateDesc;
			else if (sortByStr == QResultSorting.relevance.ToString())
				SortBy = QResultSorting.relevance;
			
			ItemDataSnipLen = paramNode.GetAttributeValue("itemDataSnipLen", 100);
			SnipMatchKeywords = paramNode.GetAttributeValue("SnipMatchKeywords", true);
			RoleTypes = paramNode.GetAttributeValue("roleTypes", "");
			IncludeAttachments = paramNode.GetAttributeValue("includeAttachments", true);
			KeywordMatchOffset = paramNode.GetAttributeValue("keywordMatchOffset", 25);

			MaxCountItems = paramNode.GetAttributeValue("maxCountItems", 1000);

			SampleSize = paramNode.GetAttributeValue("sampleSize", -1);
			KeywordCount = paramNode.GetAttributeValue("keywordCount", 30);

			string keywordSourceStr = paramNode.GetAttributeValue("keywordSource", "text");
			if (keywordSourceStr == QKeywordSource.text.ToString())
				KeywordSource = QKeywordSource.text;
			else if (keywordSourceStr == QKeywordSource.concepts.ToString())
				KeywordSource = QKeywordSource.concepts;

			string keywordMethodStr = paramNode.GetAttributeValue("keywordMethod", "localConceptSpV");
			KeywordMethod = KeywordMethodConverter.GetKeywordMethod(keywordMethodStr);
		}

		public override string ToString()
		{
			return @"<params
		resultData=""{ResultDataStr}""
		includePeopleData=""{IncludePeopleData}""
		offset=""{Offset}"" maxCount=""{MaxCount}""
		sortBy=""{SortBy}""
		itemDataSnipLen=""{ItemDataSnipLen}""
		snipMatchKeywords=""{SnipMatchKeywords}""
		roleTypes=""{RoleTypes}""
		includeAttachments=""{IncludeAttachments}""
		keywordMatchOffset=""{KeywordMatchOffset}""
		maxCountItems=""{MaxCountItems}""
		sampleSize = ""{SampleSize}""
		keywordCount = ""{KeywordCount}""
		keywordSource= ""{KeywordSource}""
		keywordMethod = ""{KeywordMethod}""
	/>\n".Format(this);
		}
	}

	// the main class used to create general queries
	public class GeneralQuery : QueryBase
	{
		public QGeneralParams QueryParams { get; private set; }
		public QArgs QueryArgs { get; private set; }

		public GeneralQuery(string queryStr)
		{
			XmlDocument queryDoc = new XmlDocument();
			queryDoc.LoadXml(queryStr);
			var queryNode = queryDoc.DocumentNode.SelectSingleNode("/query");
			Debug.Assert(queryNode != null);
			string type = queryNode.GetAttributeValue("type", "");
			Debug.Assert(type == "generalQuery");
			var queryParamsNode = queryNode.SelectSingleNode("./params");
			var queryArgsNode = queryNode.SelectSingleNode("./queryArgs");
			QueryArgs = new QArgs(queryArgsNode);
			QueryParams = new QGeneralParams(queryParamsNode);
		}
		
		public GeneralQuery(string queryParams, string queryArgs)
		{
			XmlDocument queryArgsDoc = new XmlDocument();
			queryArgsDoc.LoadXml(queryArgs);
			XmlDocument queryParamsDoc = new XmlDocument();
			queryParamsDoc.LoadXml(queryArgs);
			QueryArgs = new QArgs(queryArgsDoc.DocumentNode);
			QueryParams = new QGeneralParams(queryParamsDoc.DocumentNode);
		}

		public GeneralQuery(QGeneralParams queryParams, QArgs queryArgs)
		{
			QueryParams = queryParams;
			QueryArgs = queryArgs;
		}

		// create a new copy with the same content
		public GeneralQuery CreateCopy()
		{
			return new GeneralQuery(this.ToString());
		}

		public override string ToString()
		{
			return "<query type=\"generalQuery\">\n" + QueryParams.ToString() + QueryArgs.ToString() + "</query>";
		}
	}

	public static class GeneralQueryHelper
	{
		public static QArgs GetQArgsForPersonEmails(IPersonInfo person, string roles)
		{
			QAccountCond accountCond = new QAccountCond(from id in person.GetEmailIds() select new QAccountCond.QAccount(id, roles));
			QArgs args = new QArgs(accountCond);
			return args;
		}

		public static QArgs GetQArgsForPersonAccounts(IPersonInfo person, string roles)
		{
			QAccountCond accountCond = new QAccountCond(from id in person.GetAccountIds() select new QAccountCond.QAccount(id, roles));
			QArgs args = new QArgs(accountCond);
			return args;
		}

		public static QArgs GetQArgsForPersonEmails(IEnumerable<IPersonInfo> persons, string roles)
		{
			List<int> accounts = new List<int>();
			foreach (var person in persons)
				accounts.AddRange(person.GetEmailIds());
			QAccountCond accountCond = new QAccountCond(from id in accounts select new QAccountCond.QAccount(id, roles));
			QArgs args = new QArgs(accountCond);
			return args;
		}

		public static QArgs GetQArgsForPersonAccounts(IEnumerable<IPersonInfo> persons, string roles)
		{
			List<int> accounts = new List<int>();
			foreach (var person in persons)
				accounts.AddRange(person.GetAccountIds());
			QAccountCond accountCond = new QAccountCond(from id in accounts select new QAccountCond.QAccount(id, roles));
			QArgs args = new QArgs(accountCond);
			return args;
		}
	}
}
