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
	public class AccountInfoBase
	{
		public int AccountId;
		public string Account;
		public short AccountType;
		public int PersonId;
		public string MetaData;

		public override string ToString()
		{
			return String.Format("{0} - {1} ({2})", AccountId, Account, PersonId);
		}

		public string ToXML()
		{
			return String.Format("<AccountInfoBase accountId=\"{0}\" account=\"{1}\" accountType=\"{2}\" personId=\"{3}\" metaData=\"{4}\" />", AccountId, Text.EncodeXMLString(Account), AccountType, PersonId, Text.EncodeXMLString(MetaData));
		}

		public static AccountInfoBase FromXML(string data)
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
				AccountInfoBase accountInfo = new AccountInfoBase();
				XPathNavigator node = navigator.SelectSingleNode("/AccountInfoBase");
				string accountId = node.GetAttribute("accountId", "");
				string account = node.GetAttribute("account", "");
				string accountType = node.GetAttribute("accountType", "");
				string personId = node.GetAttribute("personId", "");
				string metaData = node.GetAttribute("metaData", "");
				accountInfo.AccountId = int.Parse(accountId);
				accountInfo.Account = Text.DecodeXMLString(account);
				accountInfo.AccountType = short.Parse(accountType);
				accountInfo.PersonId = int.Parse(personId);
				accountInfo.MetaData = Text.DecodeXMLString(metaData);
				return accountInfo;
			}
			catch (Exception)
			{
				return null;
			}
		}
	}
}
