using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using HtmlAgilityPack;
using GenLib.Text;

namespace Contextify.Shared.Types
{
	public enum LoadDataStatusEnum { Unknown = 0, DataLoadedOK, MissingDLLs }

	public class ProfileLoadStatus
	{
		public LoadDataStatusEnum LoadDataStatus { get; set; }
		public string ErrorMessage { get; set; }
		public string ProfileId { get; set; }
		public DateTime LastClearIndexTime { get; set; }

		public ProfileLoadStatus(string profileId, LoadDataStatusEnum status, DateTime lastClearIndexTime, string error = null)
		{
			ProfileId = profileId;
			LoadDataStatus = status;
			ErrorMessage = error;
			LastClearIndexTime = lastClearIndexTime;
		}

		private ProfileLoadStatus(string xmlData)
		{
			LoadDataStatus = LoadDataStatusEnum.Unknown;
			ErrorMessage = null;
			ProfileId = null;

			XmlDocument doc = new XmlDocument();
			doc.LoadXml(xmlData);
			HtmlNode node = doc.DocumentNode.SelectSingleNode("./LoadProfileStatus");
			if (node == null)
				return;
			int status = node.GetAttributeValue("LoadDataStatus", (int)LoadDataStatusEnum.Unknown);
			LoadDataStatus = (LoadDataStatusEnum)status;
			ErrorMessage = node.GetAttributeValue("ErrorMessage", "");
			ProfileId = node.GetAttributeValue("ProfileId", "");
			
			string lastClearIndexTimeStr = node.GetAttributeValue("LastClearIndexTime", "0");
			LastClearIndexTime = DateTime.FromFileTime(long.Parse(lastClearIndexTimeStr));
		}

		public static ProfileLoadStatus LoadFromXML(string xmlData)
		{
			if (xmlData == null) return null;
			return new ProfileLoadStatus(xmlData);
		}

		public override string ToString()
		{
			return String.Format(@"<?xml version=""1.0""?>
<LoadProfileStatus LoadDataStatus=""{0}"" ErrorMessage=""{1}"" ProfileId=""{2}"" LastClearIndexTime=""{3}"" />
", (int)LoadDataStatus, ErrorMessage != null ? ErrorMessage.EncodeXMLString() : "", ProfileId != null ? ProfileId.EncodeXMLString() : "", LastClearIndexTime.ToFileTime());
		}
	}
}
