using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using HtmlAgilityPack;
using GenLib.Text;

namespace Contextify.Util
{
	public class AddItemStatus
	{
		private string _status;
		public AddItemStatus(string status)
		{
			_status = status;
			XmlDocument doc = new XmlDocument();
			doc.LoadXml(_status);
			
			ItemId = -1;
			ThreadId = -1;
			var node = doc.DocumentNode.SelectSingleNode("/addItemStatus");
			if (node != null) {
				ItemId = node.GetAttributeValue("itemId", -1);
				ThreadId = node.GetAttributeValue("threadId", -1);
			}
			var errorNode = doc.DocumentNode.SelectSingleNode("/error");
			if (errorNode != null) {
				ErrorMessage = errorNode.GetAttributeValue("message", null);
				ErrorAdditionalInfo = errorNode.GetAttributeValue("additionalInfo", null);
			}
		}

		public static AddItemStatus GetInvalidItemStatus(string errorMessage, string errorAdditionalInfo = null)
		{
			string msg = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<error message=\"" + errorMessage.EncodeXMLString() + "\" additionalInfo=\"" + errorAdditionalInfo.EncodeXMLString() + "\" />";
			return new AddItemStatus(msg);
		}

		public string StatusStr { get { return _status; }}

		public int ItemId { get; private set; }
		public int ThreadId { get; private set; }
		public string ErrorMessage	{ get; private set; }
		public string ErrorAdditionalInfo	{ get; private set; }
	}
}
