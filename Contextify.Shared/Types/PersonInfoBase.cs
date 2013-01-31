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
	public class PersonInfoBase
	{
		public int PersonId;
		public string AccountIdsStr;
		public short PhotoChoice;

		public string Name;
		public short NameTrust = 0;
		public string OutlookEntryID;
		public string CustomPhoto;
		public string LinkedInPhoto;
		public string FacebookPhoto;
		public string TwitterPhoto;

		public string Occupation;
		public string CustomData;
		public bool LockedContact = false;

		public override string ToString()
		{
			return String.Format("{0} - {1} ({2})", PersonId, Name, AccountIdsStr);
		}

		public string ToXML()
		{
			string ret = String.Format("<PersonInfoBase id=\"{0}\" accs=\"{1}\" name=\"{2}\" nt=\"{3}\" ", PersonId, AccountIdsStr, Text.EncodeXMLString(Name), (int)NameTrust);
			if (PhotoChoice != 0) ret += String.Format("pc=\"{0}\" ", PhotoChoice);
			if (!string.IsNullOrEmpty(OutlookEntryID)) ret += String.Format("entry=\"{0}\" ", OutlookEntryID);
			if (!string.IsNullOrEmpty(CustomPhoto)) ret += String.Format("ph=\"{0}\" ", Text.EncodeXMLString(CustomPhoto));
			if (!string.IsNullOrEmpty(LinkedInPhoto)) ret += String.Format("li=\"{0}\" ", Text.EncodeXMLString(LinkedInPhoto));
			if (!string.IsNullOrEmpty(FacebookPhoto)) ret += String.Format("fb=\"{0}\" ", Text.EncodeXMLString(FacebookPhoto));
			if (!string.IsNullOrEmpty(TwitterPhoto)) ret += String.Format("tw=\"{0}\" ", Text.EncodeXMLString(TwitterPhoto));
			if (!string.IsNullOrEmpty(Occupation)) ret += String.Format("oc=\"{0}\" ", Text.EncodeXMLString(Occupation));
			if (!string.IsNullOrEmpty(CustomData)) ret += String.Format("cd=\"{0}\" ", Text.EncodeXMLString(CustomData));
			if (LockedContact)
				ret += "lc=\"1\" ";
			ret += "/>";
			return ret;
		}

		public static PersonInfoBase FromXML(string data)
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
				PersonInfoBase personInfo = new PersonInfoBase();
				XPathNavigator node = navigator.SelectSingleNode("/PersonInfoBase");
				string personId = node.GetAttribute("id", "");
				string photoChoice = node.GetAttribute("pc", "");
				string nameTrust = node.GetAttribute("nt", "");
				string lockedContact = node.GetAttribute("lc", "");

				personInfo.PersonId = int.Parse(personId);
				personInfo.Name = Text.DecodeXMLString(node.GetAttribute("name", ""));
				personInfo.AccountIdsStr = node.GetAttribute("accs", "");
				if (!string.IsNullOrEmpty(photoChoice))
					personInfo.PhotoChoice = short.Parse(photoChoice);
				if (!string.IsNullOrEmpty(nameTrust))
					personInfo.NameTrust = short.Parse(nameTrust);
				personInfo.OutlookEntryID = Text.DecodeXMLString(node.GetAttribute("entry", ""));
				personInfo.CustomPhoto = Text.DecodeXMLString(node.GetAttribute("ph", ""));
				personInfo.FacebookPhoto = Text.DecodeXMLString(node.GetAttribute("fb", ""));
				personInfo.LinkedInPhoto = Text.DecodeXMLString(node.GetAttribute("li", ""));
				personInfo.TwitterPhoto = Text.DecodeXMLString(node.GetAttribute("tw", ""));
				personInfo.Occupation = Text.DecodeXMLString(node.GetAttribute("oc", ""));
				personInfo.CustomData = Text.DecodeXMLString(node.GetAttribute("cd", ""));
				if (!string.IsNullOrEmpty(lockedContact))
					personInfo.LockedContact = lockedContact == "1";

				return personInfo;
			}
			catch (Exception)
			{
				return null;
			}
		}
	}
}
