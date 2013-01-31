using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using GenLib.Text;
using System.Xml;
using System.Xml.XPath;
using System.IO;

namespace Contextify.Shared.Types
{
#if! SILVERLIGHT
	[Serializable()]
#endif
	public class TagInfoBase
	{
		public string TagName;
		public int TagId;			// numeric id that is used in miner and sql tables
		public string TagIdStr;		// entry id of the folder or unique name such as [GMAIL]\Inbox
		public int ParentTagId;
		public string TagMeta;

		public TagInfoBase()
		{ }

		public static int InvalidTagId { get { return -99; } }

		public TagInfoBase(string tagName, int tagId, string tagIdStr, int parentTagId, string tagMeta = null)
		{
			TagName = tagName;
			TagId = tagId;
			TagIdStr = tagIdStr;
			ParentTagId = parentTagId;
			TagMeta = tagMeta;
		}

		public override string ToString()
		{
			return String.Format("{0} - {1}{2}", TagId, TagName, !string.IsNullOrEmpty(TagIdStr) ? " (" + TagIdStr + ")" : "");
		}

		public string ToXML()
		{
			return String.Format("<TagInfoBase name=\"{0}\" id=\"{1}\" idStr=\"{2}\" parent=\"{3}\" {4}/>", Text.EncodeXMLString(TagName), TagId, Text.EncodeXMLString(TagIdStr), ParentTagId, TagMeta != null ? "tagMeta=\"" + TagMeta.EncodeXMLString() + "\"" : "");
		}

		public static TagInfoBase FromXML(string data)
		{
			try
			{
#if SILVERLIGHT
				System.Xml.Linq.XDocument Doc = System.Xml.Linq.XDocument.Load(GenLib.Text.GetStream(data));
				XPathNavigator navigator = Doc.CreateNavigator();
#else
				XPathDocument Doc = new XPathDocument(new StringReader(data));
				XPathNavigator navigator = Doc.CreateNavigator();
#endif

				XPathNavigator node = navigator.SelectSingleNode("/TagInfoBase");
				string tagName = node.GetAttribute("name", "");
				string tagId = node.GetAttribute("id", "");
				string tagIdStr = node.GetAttribute("idStr", "");
				string parentId = node.GetAttribute("parent", "");
				string tagMeta = node.GetAttribute("tagMeta", "");
				TagInfoBase tagInfo = new TagInfoBase();
				tagInfo.TagName = Text.DecodeXMLString(tagName);
				tagInfo.TagId = int.Parse(tagId);
				tagInfo.TagIdStr = Text.DecodeXMLString(tagIdStr);
				tagInfo.ParentTagId = int.Parse(parentId);
				tagInfo.TagMeta = tagMeta != "" ? tagMeta : null;
				return tagInfo;
			}
			catch (Exception)
			{
				return null;
			}
		}
	}

}
