using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using HtmlAgilityPack;
using System.Diagnostics;
using GenLib.Text;
using Contextify.Shared.Types;

namespace Contextify.Shared.Types
{
	public enum QCustomParamsType { threadId, thread, similarThreads, similarItems, similarItemsUsingAnnotations, keywordsUsingKMeans, keywordsUsingHKMeans, keywordsUsingSVM } 
	public class QCustomParams
	{
		public string Type { get; set;}
		public bool IncludePeopleData { get; set; }
		public int MaxCount { get; set; }
		public int Offset { get; set; }

		public QCustomParams()
		{
			IncludePeopleData = false;
			MaxCount = 100;
			Offset = 0;
		}
		public QCustomParams(HtmlNode node)
		{
			IncludePeopleData = node.GetAttributeValue("includePeopleData", false);
			MaxCount = node.GetAttributeValue("maxCount", 100);
			Offset = node.GetAttributeValue("offset", 0);
		}

		public static QCustomParams CreateParams(HtmlNode node)
		{
			string type = node.GetAttributeValue("type", "");
			if (string.IsNullOrEmpty(type)) return null;
			if (type == "threadId") return new QThreadIdParams(node);
			else if (type == "thread") return new QThreadParams(node);
			else if (type == "similarThreads") return new QSimilarThreadsParam(node);
			else if (type == "similarItems") return new QSimilarItemsParam(node);
			else if (type == "similarItemsUsingAnnotations") return new QSimilarItemsAnnotParam(node);
			else if (type == "nGrams") return new QNGramsParam(node);
			else if (type == "keywordsUsingKMeans") return new QKwdKMeansParam(node);
			else if (type == "keywordsUsingHKMeans") return new QKwdHKMeansParam(node);
			else if (type == "keywordsUsingSVM") return new QKwdSVMParam(node);
			else if (type == "frequentSocialGroups") return new QFrequentSocialGroupsParam(node);
			else {
				Trace.WriteLine("Ignoring unknown param type: " + node.InnerText);
				return null;
			}
		}

		public string ArgsToString()
		{
			return string.Format(" includePeopleData=\"{0}\" maxCount=\"{1}\" offset=\"{2}\" ", IncludePeopleData, MaxCount, Offset);
		}
	}

	public class QThreadIdParams : QCustomParams
	{
		public string ThreadName { get; set; }
		public QThreadIdParams()
			: base()
		{
		}
		public QThreadIdParams(HtmlNode node)
			: base(node)
		{
			ThreadName = node.InnerText;
		}
		public override string ToString()
		{
			return String.Format("<params type=\"threadId\" {0} >{1}</params>", base.ArgsToString(), ThreadName.EncodeXMLString());
		}
	}

	public class QThreadParams : QCustomParams
	{ 
		public int ThreadId { get; set; }
		public QThreadParams() : base() 
		{
			ThreadId = -1;
		}
		public QThreadParams(HtmlNode node)
			: base(node)
		{
			ThreadId = node.GetAttributeValue("threadId", -1);
		}
		public override string ToString()
		{
			return String.Format("<params type=\"thread\" threadId=\"{0}\" {1} />", ThreadId, base.ArgsToString());
		}
	}

	public class QSimilarThreadsParam : QCustomParams
	{
		public int ThreadId { get; set; }
		public bool IncludeItemData { get; set; }
		public bool IncludeOnlyFirstInThread { get; set; }
		public bool IncludeItemIds { get; set; }
		public int ItemDataSnipLen { get; set; }

		public QSimilarThreadsParam() : base() {
			ThreadId = -1;
			IncludeItemData = false;
			IncludeOnlyFirstInThread = false;
			IncludeItemIds = false;
			ItemDataSnipLen = 200;
		}
		public QSimilarThreadsParam(HtmlNode node) : base(node)
		{
			ThreadId = node.GetAttributeValue("threadId", -1);
			IncludeItemData = node.GetAttributeValue("includeItemData", false);
			IncludeOnlyFirstInThread = node.GetAttributeValue("includeOnlyFirstInThread", false);
			IncludeItemIds = node.GetAttributeValue("includeItemIds", false);
			ItemDataSnipLen = node.GetAttributeValue("itemDataSnipLen", 200);
		}
		
		public override string ToString()
		{
			return String.Format("<params type=\"similarThreads\" threadId=\"{0}\" includeItemData=\"{1}\" includeOnlyFirstInThread=\"{2}\" includeItemIds=\"{3}\" itemDataSnipLen=\"{4}\" {5} />", ThreadId, IncludeItemData, IncludeOnlyFirstInThread, IncludeItemIds, ItemDataSnipLen, base.ArgsToString());
		}
	}

	public class QSimilarItemsParam : QCustomParams
	{
		public int ItemId { get; set; }
		public bool IncludeItemData { get; set; }
		public string TextToCompare { get; set; }
		public int ItemDataSnipLen { get; set; }

		public QSimilarItemsParam() : base() 
		{
			ItemId = -1;
			IncludeItemData = false;
			ItemDataSnipLen = 200;
			TextToCompare = null;
		}
		public QSimilarItemsParam(HtmlNode node)
			: base(node)
		{
			ItemId = node.GetAttributeValue("itemId", -1);
			IncludeItemData = node.GetAttributeValue("includeItemData", false);
			ItemDataSnipLen = node.GetAttributeValue("itemDataSnipLen", 200);
			var textNode = node.SelectSingleNode("./textToCompare");
			TextToCompare = textNode != null ? textNode.InnerText : null;
		}
		
		public override string ToString()
		{
			if (!String.IsNullOrEmpty(TextToCompare))
				return String.Format("<params type=\"similarItems\" includeItemData=\"{0}\" itemDataSnipLen=\"{1}\" {2} ><textToCompare>{3}</textToCompare></params>", IncludeItemData, ItemDataSnipLen, base.ArgsToString(), TextToCompare.EncodeXMLString());
			else
				return String.Format("<params type=\"similarItems\" itemId=\"{0}\" includeItemData=\"{1}\" itemDataSnipLen=\"{2}\" {3} />", ItemId, IncludeItemData, ItemDataSnipLen, base.ArgsToString());
		}
	}

	public class QSimilarItemsAnnotParam : QCustomParams
	{
		public int ItemId { get; set; }

		public QSimilarItemsAnnotParam() : base() 
		{
			ItemId = -1;
		}
		public QSimilarItemsAnnotParam(HtmlNode node)
			: base(node)
		{
			ItemId = node.GetAttributeValue("itemId", -1);
		}
		
		public override string ToString()
		{
			return String.Format("<params type=\"similarItemsUsingAnnotations\" itemId=\"{0}\" {1} />", ItemId, base.ArgsToString());
		}
	}

	public class QNGramsParam : QCustomParams
	{
		public int MnNGramFq { get; set; }
		public int MnNGramLen { get; set; }
		public int MxNGramCount { get; set; }
		
		public QNGramsParam() : base() 
		{
			MnNGramFq = 10;
			MnNGramLen = 2;
			MxNGramCount = 500;
		}
		public QNGramsParam(HtmlNode node)
			: base(node)
		{
			MnNGramFq = node.GetAttributeValue("mnNGramFq", 10);
			MnNGramLen = node.GetAttributeValue("mnNGramLen", 2);
			MxNGramCount = node.GetAttributeValue("mxNGramCount", 500);
		}

		public override string ToString()
		{
			return String.Format("<params type=\"nGrams\" mnNGramFq=\"{0}\" mnNGramLen=\"{1}\" mxNGramCount=\"{2}\" {3} />", MnNGramFq, MnNGramLen, MxNGramCount, base.ArgsToString());
		}
	}

	public class QFrequentSocialGroupsParam : QCustomParams
	{
		public QFrequentSocialGroupsParam()
		{
			MaxCount = 20;
		}
		public QFrequentSocialGroupsParam(HtmlNode node)
		{
			MaxCount = node.GetAttributeValue("maxCount", 20);
		}

		public override string ToString()
		{
			return String.Format("<params type=\"frequentSocialGroups\" maxCount=\"{0}\" />", MaxCount);
		}
	}

	public class QKwdKMeansParam : QCustomParams
	{
		public int KeywordCount { get; set; }
		public bool ComputeOnThreads { get; set; }
		public int K { get; set; }
		public int RndSeed { get; set; }
		public int ClustTrials { get; set; }
		public int ConvergEps { get; set; }
		public double CutWordWgtSumPrc  { get; set; }
		public int MnWordFq { get; set; }
		public QKeywordMethod KeywordMethod { get; set; }	// resultData=keywordData: the method used for extracting keywords		

		public QKwdKMeansParam() : base() 
		{
			KeywordCount = 20;
			ComputeOnThreads = false;
			K = 5;
			RndSeed = 1;
			ClustTrials = 1;
			ConvergEps = 10;
			CutWordWgtSumPrc = 0.5;
			MnWordFq = 5;
			KeywordMethod = QKeywordMethod.localConceptSpV;
		}
		public QKwdKMeansParam(HtmlNode node)
			: base(node)
		{
			KeywordCount = node.GetAttributeValue("keywordCount", 20);
			ComputeOnThreads = node.GetAttributeValue("computeOnThreads", false);
			K = node.GetAttributeValue("k", 5);
			RndSeed = node.GetAttributeValue("rndSeed", 1);
			ClustTrials = node.GetAttributeValue("clustTrials", 1);
			ConvergEps = node.GetAttributeValue("convergEps", 10);
			CutWordWgtSumPrc = double.Parse(node.GetAttributeValue("cutWordWgtSumPrc", "0.5"));
			MnWordFq = node.GetAttributeValue("mnWordFq", 5);
			string keywordMethodStr = node.GetAttributeValue("keywordMethod", "localConceptSpV");
			KeywordMethod = KeywordMethodConverter.GetKeywordMethod(keywordMethodStr);
		}
		public override string ToString()
		{
			return String.Format("<params type=\"keywordsUsingKMeans\" keywordCount=\"{0}\" computeOnThreads=\"{1}\" k=\"{2}\" rndSeed=\"{3}\" clustTrials=\"{4}\" convergEps=\"{5}\" cutWordWgtSumPrc=\"{6}\" mnWordFq=\"{7}\" keywordMethod=\"{8}\" />", KeywordCount, ComputeOnThreads, K, RndSeed, ClustTrials, ConvergEps, CutWordWgtSumPrc, MnWordFq, KeywordMethod);
		}
	}

	public class QKwdHKMeansParam : QCustomParams
	{
		public int KeywordCount { get; set; }
		public bool ComputeOnThreads { get; set; }
		public int K { get; set; }
		public int MnDocsPerCluster { get; set; }
		public int MxDocsPerCluster { get; set; }
		public int RndSeed { get; set; }
		public int ClustTrials { get; set; }
		public int ConvergEps { get; set; }
		public double CutWordWgtSumPrc  { get; set; }
		public int MnWordFq { get; set; }
		public QKeywordMethod KeywordMethod { get; set; }	// resultData=keywordData: the method used for extracting keywords		


		public QKwdHKMeansParam() : base() 
		{
			KeywordCount = 20;
			ComputeOnThreads = false;
			K = 5;
			MnDocsPerCluster = 50;
			MxDocsPerCluster = 1000;
			RndSeed = 1;
			ClustTrials = 1;
			ConvergEps = 10;
			CutWordWgtSumPrc = 0.5;
			MnWordFq = 5;
			KeywordMethod = QKeywordMethod.localConceptSpV;
		}
		public QKwdHKMeansParam(HtmlNode node)
			: base(node)
		{
			KeywordCount = node.GetAttributeValue("keywordCount", 20);
			ComputeOnThreads = node.GetAttributeValue("computeOnThreads", false);
			K = node.GetAttributeValue("k", 5);
			MnDocsPerCluster = node.GetAttributeValue("mnDocsPerCluster", 50);
			MxDocsPerCluster = node.GetAttributeValue("mxDocsPerCluster", 1000);
			RndSeed = node.GetAttributeValue("rndSeed", 1);
			ClustTrials = node.GetAttributeValue("clustTrials", 1);
			ConvergEps = node.GetAttributeValue("convergEps", 10);
			CutWordWgtSumPrc = double.Parse(node.GetAttributeValue("cutWordWgtSumPrc", "0.5"));
			MnWordFq = node.GetAttributeValue("mnWordFq", 5);
			string keywordMethodStr = node.GetAttributeValue("keywordMethod", "localConceptSpV");
			KeywordMethod = KeywordMethodConverter.GetKeywordMethod(keywordMethodStr);
		}
		public override string ToString()
		{
			return String.Format("<params type=\"keywordsUsingHKMeans\" keywordCount=\"{0}\" computeOnThreads=\"{1}\" k=\"{2}\" rndSeed=\"{3}\" clustTrials=\"{4}\" convergEps=\"{5}\" cutWordWgtSumPrc=\"{6}\" mnWordFq=\"{7}\" mnDocsPerCluster=\"{8}\" mxDocsPerCluster=\"{9}\" keywordMethod=\"{10}\" />", 
				KeywordCount, ComputeOnThreads, K, RndSeed, ClustTrials, ConvergEps, CutWordWgtSumPrc, MnWordFq, MnDocsPerCluster, MxDocsPerCluster, KeywordMethod);
		}
	}

	public class QKwdSVMParam : QCustomParams
	{
		public int KeywordCount { get; set; }
		public int TimeLimit { get; set; }

		public QKwdSVMParam() : base() 
		{
			KeywordCount = 20;
			TimeLimit = 20;
		}
		public QKwdSVMParam(HtmlNode node)
			: base(node)
		{
			KeywordCount = node.GetAttributeValue("keywordCount", 20);
			TimeLimit = node.GetAttributeValue("timeLimit", 20);
		}

		public override string ToString()
		{
			return String.Format("<params type=\"keywordsUsingSVM\" keywordCount=\"{0}\" timeLimit=\"{1}\" />", KeywordCount, TimeLimit);
		}
	}

	//public class QArgsNegative : QArgs
	//{
	//	public QArgsNegative(QCond singleCondition) : base(singleCondition)
	//	{
	//	}
		
	//	public QArgsNegative(IEnumerable<QCond> conditions, IEnumerable<QCond> ignore) : base(conditions, ignore)
	//	{
	//	}
	//	public QArgsNegative(string conditions, string ignore) : base(conditions, ignore)
	//	{
	//	}
	//	public QArgsNegative(HtmlNode queryArgsNode) : base(queryArgsNode)
	//	{
	//	}
	//	public QArgsNegative(HtmlNode conditionsNode, HtmlNode ignoreNode) : base(conditionsNode, ignoreNode)
	//	{
	//	}
	//	public static new QArgsNegative CreateArgs(HtmlNode queryArgsNegativeNode)
	//	{
	//		if (queryArgsNegativeNode == null)
	//			return null;
	//		return new QArgsNegative(queryArgsNegativeNode);
	//	}

	//	public override string ToString()
	//	{
	//		string ret = "<queryArgsNegative>\n";
	//		if (Conditions != null)
	//			ret += "<conditions>\n" + String.Join("\n", from c in Conditions select c.ToString()) + "</conditions>\n";
	//		if (Ignore != null)
	//			ret += "<ignore>\n" + String.Join("\n", from i in Ignore select i.ToString()) + "</ignore>\n";
	//		ret += "</queryArgsNegative>\n";
	//		return ret;
	//	}
	//}

	// the main class used to create general queries
	public class CustomQuery : QueryBase
	{
		public QCustomParams QueryParams { get; private set; }

		public QArgs QueryArgs { get; private set; }
		public QArgsNegative QueryArgsNegative { get; private set; }
		
		public CustomQuery(string queryStr)
		{
			XmlDocument queryDoc = new XmlDocument();
			queryDoc.LoadXml(queryStr);
			var queryNode = queryDoc.DocumentNode.SelectSingleNode("/query");
			Debug.Assert(queryNode != null);
			string type = queryNode.GetAttributeValue("type", "");
			Debug.Assert(type == "customQuery");
			var queryParamsNode = queryNode.SelectSingleNode("./params");
			var queryArgsNode = queryNode.SelectSingleNode("./queryArgs");
			var queryArgsNegativeNode = queryNode.SelectSingleNode("./queryArgsNegative");
			QueryArgs = QArgs.CreateArgs(queryArgsNode);
			QueryArgsNegative = QArgsNegative.CreateArgs(queryArgsNegativeNode);
			//if (QueryArgsNegative != null)
			//	QueryArgsNegative.TagName = "queryArgsNegative";
			QueryParams = QCustomParams.CreateParams(queryParamsNode);
		}

		public CustomQuery(QCustomParams queryParams, QArgs queryArgs = null, QArgsNegative queryArgsNegative = null)
		{
			QueryParams = queryParams;
			QueryArgs = queryArgs;
			QueryArgsNegative = queryArgsNegative;
			//if (QueryArgsNegative != null)
			//	QueryArgsNegative.TagName = "queryArgsNegative";
		}

		// create a new copy with the same content
		public CustomQuery CreateCopy()
		{
			return new CustomQuery(this.ToString());
		}

		public override string ToString()
		{
			string ret = "<query type=\"customQuery\">\n";
			ret += QueryParams != null ? QueryParams.ToString() : "";
			ret += QueryArgs != null ? QueryArgs.ToString() : "";
			ret += QueryArgsNegative != null ? QueryArgsNegative.ToString() : "";
			ret += "</query>";
			return ret;
		}
	}
}
