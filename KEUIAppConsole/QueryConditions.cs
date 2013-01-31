using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using HtmlAgilityPack;
using GenLib.Text;
using Contextify.Shared.Types;

namespace KEUIApp
{
	// account conditions
	//<accounts><account id="" role="from,to,cc,bcc,author,@ANY,...">+</accounts>	// write type for each email but in the interface have the same type for one person			
	public class QAccountNameCond : QCond
	{
		public class QAccount
		{
			public string Name { get; private set; }
			public string Roles { get; private set; }

			public QAccount(string name, string roles)
			{
				Name = name;
				Roles = roles;
			}
			public override string ToString()
			{
				return String.Format("<account name=\"{0}\" roles=\"{1}\" />", Name, Roles);
			}
		}

		public static string TagName { get { return "accountName"; } }
		public IEnumerable<QAccount> Accounts { get; private set; }

		public QAccountNameCond(QAccount account)
		{
			Accounts = new[] { account };
		}

		public QAccountNameCond(IEnumerable<QAccount> accounts)
		{
			Accounts = accounts;
		}

		public QAccountNameCond(HtmlNode node)
		{
			Debug.Assert(node.Name == TagName);
			Accounts = from accountNode in node.SelectNodes("./" + TagName) ?? new HtmlNodeCollection(null) select new QAccount(accountNode.GetAttributeValue("name", ""), accountNode.GetAttributeValue("roles", ""));
		}

		public static QAccountNameCond CreateAccountNameCond(HtmlNode node)
		{
			return new QAccountNameCond(node);
		}

		public override string ToString()
		{
			if (Accounts == null) return "";
			return "<" + TagName + ">" + String.Join("", from a in Accounts select a.ToString()) + "</" + TagName + ">\n";
		}
	}


	public class QConceptCond : QCond
	{
		public class QConcept
		{
			public string Uri { get; private set; }
			
			public QConcept(string uri)
			{
				Uri = uri;
			}
			public override string ToString()
			{
				return String.Format("<concept>{0}</concept>", Uri.EncodeXMLString());
			}
		}

		public static string TagName { get { return "concepts"; } }
		public IEnumerable<QConcept> Concepts { get; private set; }

		public QConceptCond(QConcept concept)
		{
			Concepts = new[] { concept };
		}

		public QConceptCond(IEnumerable<QConcept> concepts)
		{
			Concepts = concepts;
		}

		public QConceptCond(HtmlNode node)
		{
			Debug.Assert(node.Name == TagName);
			Concepts = from conceptNode in node.SelectNodes("./" + TagName) ?? new HtmlNodeCollection(null) select new QConcept(conceptNode.InnerText);
		}

		public static QConceptCond CreateConceptCond(HtmlNode node)
		{
			return new QConceptCond(node);
		}

		public override string ToString()
		{
			if (Concepts == null) return "";
			return "<" + TagName + ">" + String.Join("", from c in Concepts select c.ToString()) + "</" + TagName + ">\n";
		}
	}

	public class QEnrychableKeywordsCond : QCond
	{
		public static string TagName { get { return "enrychableKeywords"; } }
		public string Keywords { get; private set; }
		public bool Optional { get; set; }

		public QEnrychableKeywordsCond(string keywords, bool optional = false)
		{
			Keywords = keywords;
			Optional = optional;
		}

		public QEnrychableKeywordsCond(HtmlNode node)
		{
			Debug.Assert(node.Name == TagName);
			Keywords = node.InnerText;
			string strOptional = node.GetAttributeValue("optional", "").ToLower();
			Optional = strOptional == "1" || strOptional == "true";
		}

		public static QEnrychableKeywordsCond CreateEnrychableKeywordsCond(HtmlNode node)
		{
			return new QEnrychableKeywordsCond(node);
		}

		public override string ToString()
		{
			return String.Format("<{0} optional=\"{1}\" >{2}</{0}>\n", TagName, Optional, Keywords.EncodeXMLString());
		}
	}

	public class QBugIdsCond : QCond
	{
		public static string TagName { get { return "bugIds"; } }
		public int BugId { get; private set; }

		public QBugIdsCond(int bugId)
		{
			BugId = bugId;
		}

		public QBugIdsCond(HtmlNode node)
		{
			Debug.Assert(node.Name == TagName);
			BugId = int.Parse(node.InnerText);
		}

		public static QBugIdsCond CreateBugIdsCond(HtmlNode node)
		{
			return new QBugIdsCond(node);
		}

		public override string ToString()
		{
			return "<" + TagName + ">" + BugId.ToString() + "</" + TagName + ">\n";
		}
	}

	// tagIdStr conditions
	// <tagIdStr>tagIdStr</tagIdStr>		// one tag!! containing the tagIdStr
	public class QTagIdStrCond : QCond
	{
		public static string TagName { get { return "tagIdStr"; } }
		public IEnumerable<string> TagIdStrs { get; private set; }
		public QTagIdStrCond(string tagIdStr)
		{
			TagIdStrs = new[] { tagIdStr };
		}
		public QTagIdStrCond(IEnumerable<string> tagIdStrs)
		{
			TagIdStrs = tagIdStrs;
		}

		public QTagIdStrCond(HtmlNode node)
		{
			Debug.Assert(node.Name == TagName);
			TagIdStrs = from tagIdStr in node.InnerText.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries) select tagIdStr;
		}

		public static QTagIdStrCond CreateTagIdStrCond(HtmlNode node)
		{
			return new QTagIdStrCond(node);
		}

		public override string ToString()
		{
			return "<" + TagName + ">" + String.Join(",", TagIdStrs) + "</" + TagName + ">\n";
		}
	}

	// postTypes conditions
	// <postTypes>type1,type2,...</postTypes>		// one or more tags (folderId or custom tags) - there is OR between them. to have AND specify multiple <tags>.
	public class QPostTypesCond : QCond
	{
		public static string TagName { get { return "postTypes"; } }
		public IEnumerable<string> PostTypes { get; private set; }
		public QPostTypesCond(string postType)
		{
			PostTypes = new[] { postType };
		}
		public QPostTypesCond(IEnumerable<string> postTypes)
		{
			PostTypes = postTypes;
		}

		public QPostTypesCond(HtmlNode node)
		{
			Debug.Assert(node.Name == TagName);
			PostTypes = from tagIdStr in node.InnerText.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries) select tagIdStr;
		}

		public static QPostTypesCond CreatePostTypesCond(HtmlNode node)
		{
			return new QPostTypesCond(node);
		}

		public override string ToString()
		{
			return "<" + TagName + ">" + String.Join(",", PostTypes) + "</" + TagName + ">\n";
		}
	}
}
