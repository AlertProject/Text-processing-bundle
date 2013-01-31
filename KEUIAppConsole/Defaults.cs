using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using GenLib.Text;
using System.IO;
using System.Diagnostics;

namespace KEUIApp
{
	static class Defaults
	{
		private static string _templateFolder = null;
		public static string TemplateFolder 
		{
			get
			{
				if (string.IsNullOrEmpty(_templateFolder))
					_templateFolder = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "KEUIApp", "Event Templates");
				return _templateFolder;
			}
			set { _templateFolder = value; }
		}
		
		private static string _keuiRequestTemplate = null;
		public static string KeuiRequestTemplate
		{
			get
			{
				Debug.Assert(!string.IsNullOrEmpty(TemplateFolder), "You forgot to assign the folder that contains event templates");
				if (_keuiRequestTemplate == null && File.Exists(Path.Combine(TemplateFolder, "keuiRequest.xml")))
					_keuiRequestTemplate = File.ReadAllText(Path.Combine(TemplateFolder, "keuiRequest.xml"));
				return _keuiRequestTemplate;
			}
		}

		private static string _newForumPostTemplate = null;
		public static string NewForumPostTemplate { 
			get
			{
				Debug.Assert(!string.IsNullOrEmpty(TemplateFolder), "You forgot to assign the folder that contains event templates");
				if (_newForumPostTemplate == null && File.Exists(Path.Combine(TemplateFolder, "newForumPost.xml")))
					_newForumPostTemplate = File.ReadAllText(Path.Combine(TemplateFolder, "newForumPost.xml"));
				return _newForumPostTemplate;
			}
		}

		private static string _textToAnnotateTemplate = null;
		public static string TextToAnnotateTemplate
		{
			get
			{
				Debug.Assert(!string.IsNullOrEmpty(TemplateFolder), "You forgot to assign the folder that contains event templates");
				if (_textToAnnotateTemplate == null && File.Exists(Path.Combine(TemplateFolder, "textToAnnotate.xml")))
					_textToAnnotateTemplate = File.ReadAllText(Path.Combine(TemplateFolder, "textToAnnotate.xml"));
				return _textToAnnotateTemplate;
			}
		}

		private static string _customItemToIndexTemplate = null;
		public static string CustomItemToIndexTemplate
		{
			get
			{
				Debug.Assert(!string.IsNullOrEmpty(TemplateFolder), "You forgot to assign the folder that contains event templates");
				if (_customItemToIndexTemplate == null && File.Exists(Path.Combine(TemplateFolder, "customItemToIndex.xml")))
					_customItemToIndexTemplate = File.ReadAllText(Path.Combine(TemplateFolder, "customItemToIndex.xml"));
				return _customItemToIndexTemplate;
			}
		}

		private static string _newRDFDataTemplate = null;
		public static string NewRDFDataTemplate
		{
			get
			{
				Debug.Assert(!string.IsNullOrEmpty(TemplateFolder), "You forgot to assign the folder that contains event templates");
				if (_newRDFDataTemplate == null && File.Exists(Path.Combine(TemplateFolder, "conceptNew.xml")))
					_newRDFDataTemplate = File.ReadAllText(Path.Combine(TemplateFolder, "conceptNew.xml"));
				return _newRDFDataTemplate;
			}
		}

		public static string BuildNewRDFData(string rdfData)
		{
			long longTimeNow = GenLib.Time.ToUnixTime(DateTime.Now);
			var obj = new
			{
				timestamp = longTimeNow,
				sequencenumber = 0,
				startTime = longTimeNow,
				endTime = longTimeNow,
				rdfData = rdfData
			};
			return NewRDFDataTemplate.Format(obj);
		}

		public static string BuildNewForumItem(int forumSensorAccountId, string forumName, int forumId, int postId, int threadId, string link, DateTime time, string subject, string body, string author, string category)
		{
			long longTimeNow = GenLib.Time.ToUnixTime(DateTime.Now);
			var obj = new { timestamp = longTimeNow, 
							sequencenumber = 0,
							startTime = longTimeNow,
							endTime = longTimeNow,
							forumSensorAccountId = forumSensorAccountId, 
							forumName = forumName.EncodeXMLString(), 
							forumId = forumId, 
							forumItemId = postId, 
							forumThreadId = threadId, 
							forumItemUrl = link.EncodeXMLString(), 
							time = time.ToString("yyyy-MM-dd HH:mm:ss"), 
							subject = subject, 
							body = body, 
							author = author.EncodeXMLString(), 
							category = category.EncodeXMLString() };
			return NewForumPostTemplate.Format(obj);
		}

		public static string BuildNewTextToAnnotate(string source, string text)
		{
			long longTimeNow = GenLib.Time.ToUnixTime(DateTime.Now);
			var obj = new { timestamp = DateTime.Now, 
							sequencenumber = 0,
							startTime = longTimeNow,
							endTime = longTimeNow,
							source = source, 
							text = text.EncodeXMLString() };
			return TextToAnnotateTemplate.Format(obj);
		}

		public static string BuildCustomItemToIndex(string metaData, string content)
		{
			long longTimeNow = GenLib.Time.ToUnixTime(DateTime.Now);
			var obj = new { timestamp = DateTime.Now, 
							sequencenumber = 0,
							startTime = longTimeNow,
							endTime = longTimeNow,
							metaData = metaData, 
							content = content.EncodeXMLString() };
			return CustomItemToIndexTemplate.Format(obj);
		}

		public static string BuildRequest(string sender, string requestType, string requestData, out string sequencenumber)
		{
			long longTimeNow = GenLib.Time.ToUnixTime(DateTime.Now);
			sequencenumber = System.Guid.NewGuid().ToString();
			var obj = new { sender = sender,
							timestamp = longTimeNow, 
							sequencenumber = sequencenumber,
							startTime = longTimeNow,
							endTime = longTimeNow,
							requestType = requestType, 
							requestData = requestData };
			return KeuiRequestTemplate.Format(obj);
		}
	}
}
