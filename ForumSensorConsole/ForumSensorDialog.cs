using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Collections.Specialized;

using GenLib.Web;
using GenLib.Text;
using System.Diagnostics;

using System.IO;
using System.Reflection;
using System.Threading;

using HtmlAgilityPack;
using Apache.NMS;
using Apache.NMS.ActiveMQ;
using KEUIApp;

namespace ForumSensor
{
	public partial class ForumSensorDialog
	{
		public class AccountInfo
		{
			public AccountInfo(int forumId, string forumName, string address, int checkBackDays)
			{
				ForumId = forumId;
				ForumName = forumName;
				Address = address;
				CheckBackDays = checkBackDays;
			}

			public string Address;
			public int ForumId;
			public string ForumName;
			public int CheckBackDays;
		}

		public AsyncWebGetter WebGetter { get; set; }
		public bool CanRun = true;
		public bool DownloadTextOnly = false;
		public string DownloadPath = "";

		private int _lastOffset = 0;
		private int _stepSize = 200;
		private int _currentAccountIndex = -1;
		private int _totalSentCount = 0;
		private int _processedCountSinceLastCheck = 0;

		public int CheckEveryMin = 5;
		public int SaveIdsEveryMin = 6 * 60;
		public bool StartOnStartup = true;
		public int NrOfThreads = 5;
		public List<AccountInfo> AccountInfoList = new List<AccountInfo>();

		private System.Threading.Timer _timer;
		private System.Threading.Timer _timerSaveIds;
		private System.Threading.Timer _timerStatus;
		public bool CheckingInProgress { get; private set; }

		private object _publisherLock = new object();
		private object _postHashLock = new object();
		public ActiveMqHelper.TopicPublisher AQPublisher = null;
		public ISession AQSession = null;

		private static String _appName = "ForumSensor";

		private String _forumSettingsFolder = null;
		public string _settingsFileName = "ForumSensor.xml";
		public string _importedIdsFileTemplate = "importedItemIdsForForumId{0}.txt";
		string _fileNameActiveMQSettings = "ActiveMQSettings.xml";


		Dictionary<int, HashSet<string>> _forumIdToImportedIdH = new Dictionary<int, HashSet<string>>();
		//HashSet<string> _importedIds = new HashSet<string>();

		KEUIApp.ActiveMQSettings _activeMQSettings = null;

		public ForumSensorDialog(string appName = "ForumSensor")
		{
			_appName = appName;
			_forumSettingsFolder = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), _appName);
		}

		void ForumSensorDialog_Load(object sender, EventArgs e)
		{
			Init();

			if (StartOnStartup)
				ButtonStart_Click(null, null);
		}

		public void ButtonStart_Click(object sender, EventArgs e)
		{
			if (_timer == null) {
				CanRun = true;
				_timer = new System.Threading.Timer(new TimerCallback(NewData_Timer), null, 10, CheckEveryMin * 60 * 1000);
				_timerSaveIds = new System.Threading.Timer(new TimerCallback(state => SaveImportedIds()), null, 10, SaveIdsEveryMin * 60 * 1000);
			}
			else {
				CanRun = false;

				DisposeTimers();
			}
		}

		private void DisposeTimers()
		{
			if (_timerSaveIds != null)
				_timerSaveIds.Dispose();
			_timerSaveIds = null;
			if (_timer != null)
				_timer.Dispose();
			_timer = null;
		}

		public void Init()
		{
			try {
				// first start the logging service
				try {
					string logFolder = Path.Combine(_forumSettingsFolder, "Log");
					if (!Directory.Exists(logFolder))
						Directory.CreateDirectory(logFolder);
					GenLib.Log.LogService.AddLogger(new GenLib.Log.FileLogger(logFolder, "events.txt"));
				}
				catch (Exception ex) { AddEvent("Unable to start the logging service. Error: " + ex.Message); }
				GenLib.Log.LogService.LogInfo("Forum Sensor is starting.");

				LoadSettings();

				IConnectionFactory factory = new ConnectionFactory(_activeMQSettings.BrokerUri);
				IConnection connection = factory.CreateConnection();
				connection.Start();
				AQSession = connection.CreateSession();

				lock (_publisherLock) {
					AQPublisher = new ActiveMqHelper.TopicPublisher(AQSession, _activeMQSettings.TopicNameForumSensorPublishForumPost);
				}
				AddEvent("Publishing events on " + _activeMQSettings.TopicNameForumSensorPublishForumPost);
			}
			catch (Exception ex) {
				AddEvent("Unable to use ActiveMQ: " + ex.Message);
			}

			// start the web getter
			WebGetter = new AsyncWebGetter(NrOfThreads, QueueEmptyCallbackFunc);
			WebGetter.SetChromBrowserHeaders();

			// for some reason, KDE's forum has some protection that returns invalid forum data for some time at the beginning. 
			// for this reason we first create a few requests and then also wait 10 sec. Using this approach we don't seem to get empty body errors
			foreach (var account in AccountInfoList) {
				for (int i = 0; i < NrOfThreads * 2; i++)
					WebGetter.DownloadPageAsync(this, null, new SmallUri(account.Address), Foo);
			}
			Thread.Sleep(10000);
		}

		public void Finish()
		{
			DisposeTimers();
			lock (_publisherLock) {
				if (AQPublisher != null)
					AQPublisher.Dispose();
				AQPublisher = null;
			}
			SaveSettings();
			WebGetter.Dispose();
		}

		private void Foo(object sender, PageCompletedEventArgs e)
		{ }

		// after we process the whole queue we allow the next timer event to start again
		private void QueueEmptyCallbackFunc(object sender, QueueEmptyEventArgs e)
		{
			CheckingInProgress = false;
		}

		void NewData_Timer(object state)
		{
			if (!CanRun)
				return;
			if (CheckingInProgress)
				return;
			if (AccountInfoList.Count == 0)
				return;

			CheckingInProgress = true;
			try {
				_lastOffset = 0;
				_currentAccountIndex = (_currentAccountIndex + 1) % AccountInfoList.Count;
				AddEvent("Checking forum " + AccountInfoList[_currentAccountIndex].Address);
				DownloadNextRSSPage();
			}
			catch (Exception ex) {
				AddEvent("Unhandled exception: " + ex.Message + "\n" + ex.StackTrace);
				GenLib.Log.LogService.LogException("ForumSensorDialog.TimerCheckNewData_Tick exception.", ex);
			}
		}

		/// <summary>
		/// download next rss page
		/// </summary>
		private void DownloadNextRSSPage()
		{
			try {
				Debug.Assert(_currentAccountIndex < AccountInfoList.Count);
				AccountInfo account = AccountInfoList[_currentAccountIndex];
				string urlTemplate = account.Address;

				if (!urlTemplate.EndsWith("/"))
					urlTemplate += "/";

				//urlTemplate += @"search.php?feed_type=RSS2.0&sd=a&feed_style=BASIC&countlimit={0}&submit=Search&start={1}&st={2}";
				urlTemplate += @"search.php?keywords=&terms=all&author=&tags=&sv=0&sc=1&sf=all&sk=t&sd=a&feed_type=RSS2.0&feed_style=BASIC&submit=Search&countlimit={0}&start={1}&st={2}";

				string url = String.Format(urlTemplate, _stepSize, _lastOffset * _stepSize, account.CheckBackDays);
				Trace.WriteLine("Offset value: " + _lastOffset * _stepSize);
				Trace.WriteLine("Downloading RSS page: " + url);
				WebGetter.DownloadPageAsync(this, null, new SmallUri(url), OnRSSPageDownloaded);
				_lastOffset += 1;
			}
			catch (Exception ex) {
				AddEvent("Exception while publishing new data: " + ex.Message);
				GenLib.Log.LogService.LogException("ForumSensorDialog.OnPostPageDownloaded Exception while publishing new data: ", ex);
			}
		}


		//private Dictionary<string, string> linkToPageContent = new Dictionary<string, string>();
		public void OnRSSPageDownloaded(object sender, PageCompletedEventArgs e)
		{
			try {
				if (AQPublisher == null && !DownloadTextOnly) {
					AddEvent("Active MQ publisher is null. Unable to post the event to the queue.");
					return;
				}
				if (!CanRun)
					return;

				int currentForumId = AccountInfoList[_currentAccountIndex].ForumId;
				XmlDocument doc = new HtmlAgilityPack.XmlDocument();
				doc.LoadXml(e.PageContent);
				
				foreach (HtmlNode item in doc.DocumentNode.SelectNodes("//channel/item") ?? new HtmlNodeCollection(null)) {
					string link = item.Element("link").InnerText.DecodeXMLString();
					if (link.LastIndexOf('#') >= 0)
						link = link.Substring(0, link.LastIndexOf('#'));
					Dictionary<string, string> nameValueCollection = Utility.ParseQueryArguments(link);
					string itemId = "";
					if (!nameValueCollection.ContainsKey("p")) {
						AddEvent("A post in the RSS feed didn't contain post id. Ignoring the post. RSS feed: " + e.SmallUri.GetString());
						continue;
					}

					itemId = nameValueCollection["p"];
					lock (_postHashLock) {
						if (_forumIdToImportedIdH[currentForumId].Contains(itemId))
							continue;
					}

					WebGetter.DownloadPageAsync(this, item, new GenLib.Web.SmallUri(link), OnPostPageDownloaded);
				}
				//AddEvent("Parsed RSS page: " + e.SmallUri.GetString());
				//Trace.WriteLine("Parsed RSS page: " + e.SmallUri.GetString());
				GenLib.Log.LogService.LogInfo("Parsed RSS page " + e.SmallUri.GetString());
				if (CanRun)
					DownloadNextRSSPage();
			}
			catch (Exception ex) {
				AddEvent("Exception while publishing new data: " + ex.Message);
				GenLib.Log.LogService.LogException("ForumSensorDialog.OnPostPageDownloaded Exception while publishing new data: ", ex);
			}
		}

		public void OnPostPageDownloaded(object sender, PageCompletedEventArgs e)
		{
			if (!CanRun)
				return;
			try {
				_processedCountSinceLastCheck++;
				HtmlNode item = e.UserState as HtmlNode;

				string link = e.SmallUri.GetString();
				Dictionary<string, string> nameValueCollection = GenLib.Web.Utility.ParseQueryArguments(link);
				int postId = -1;
				int threadId = -1;
				int forumId = -1;
				if (nameValueCollection.ContainsKey("p"))
					postId = int.Parse(nameValueCollection["p"]);
				if (nameValueCollection.ContainsKey("t"))
					threadId = int.Parse(nameValueCollection["t"]);
				if (nameValueCollection.ContainsKey("f"))
					forumId = int.Parse(nameValueCollection["f"]);

				//if (!linkToPageContent.ContainsKey(link))
				//    linkToPageContent[link] = e.PageContent;

				HtmlAgilityPack.XmlDocument doc = new HtmlAgilityPack.XmlDocument();
				doc.LoadXml(e.PageContent);
				var bodyNode = doc.DocumentNode.SelectSingleNode(String.Format("//div[@id='article{0}']", postId));
				if (bodyNode == null) {
					ReportError("Body for post page was null. Error downloading post information.");
					return;
				}
				string body = bodyNode.InnerHtml;
				body = body.StripHtml();		// todo: test if this removes everything ok
				//body = body.Replace("\n", " ");
				//body = Regex.Replace(body, "<br[ ]*[/]?>", Environment.NewLine);
				//body = body.Replace(" \r\n", "\r\n");
				//body = body.DecodeXMLString();
				//body = RemoveBlockQuotes(body);

				//body = System.Web.HttpUtility.HtmlDecode(body);

				// use item and body to create an event that is sent
				string subject = item.Element("title").InnerText;
				string author = item.Element("author").InnerText;
				string category = item.Element("category").InnerText;
				string dateStr = item.Element("pubDate").InnerText;
				string postIdStr = nameValueCollection["p"];
				DateTime date = DateTime.Parse(dateStr);

				//AddEvent("Parsed post with id " + postId);
				//Trace.WriteLine("Parsed post " + postId);
				AccountInfo account = AccountInfoList[_currentAccountIndex];

				string eventData = Defaults.BuildNewForumItem(account.ForumId, account.ForumName, forumId, postId, threadId, link, date, subject, body, author, category);

				if (DownloadTextOnly) {
					string fileName = Path.Combine(DownloadPath, postId.ToString());
					File.WriteAllText(fileName, eventData);
				}
				else {
					lock (_publisherLock) {
						StoreEventData(_activeMQSettings.TopicNameForumSensorPublishForumPost, eventData, false);
						AQPublisher.SendMessage(eventData);
					}
					GenLib.Log.LogService.LogInfo("Parsed post with id " + postId);
				}

				lock (_postHashLock) {
					_forumIdToImportedIdH[account.ForumId].Add(postIdStr);
				}

				if (postId % 10 == 0)
					AddEvent("downloaded post with id " + postId);
			}
			catch (Exception ex) {
				AddEvent("Exception while publishing new data: " + ex.Message);
				GenLib.Log.LogService.LogException("ForumSensorDialog.OnPostPageDownloaded Exception while publishing new data: ", ex);
			}

			//string currentFolder = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
			//File.WriteAllText(Path.Combine(currentFolder, itemIdStr + ".xml"), eventData);

			_totalSentCount++;
			//this.Invoke((Action)(() => { LabelStatus.Text = String.Format("Sent {0} posts", _totalSentCount); }));
		}

		#region settings
		private void StoreEventData(string topic, string message, bool forceStoring = false, string namePrefix = "")
		{
			if (_activeMQSettings.StoreEvents || forceStoring == true) {
				string path = Path.Combine(_forumSettingsFolder, "StoredEvents", topic);
				path = path.Replace("*", "_");
				if (!Directory.Exists(path))
					Directory.CreateDirectory(path);
				string filename = namePrefix + DateTime.Now.Ticks.ToString() + ".xml";
				File.WriteAllText(Path.Combine(path, filename), message);
			}
		}

		public void ReportError(string text)
		{
#if UI	
			Forms.MessageBox.Show(text, "Error", Forms.MessageBoxButtons.OK, Forms.MessageBoxIcon.Error);
#else
			Console.WriteLine(text);
#endif
		}
		public void LoadSettings()
		{
			//string currentFolder = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
			try {
				// modify the default template folder
				Defaults.TemplateFolder = Path.Combine(_forumSettingsFolder, "Event Templates");

				if (!Directory.Exists(_forumSettingsFolder))
					Directory.CreateDirectory(_forumSettingsFolder);

				if (!Directory.Exists(Defaults.TemplateFolder))
					ReportError(String.Format("The folder with templates ({0}) does not exist. Create it and put templates there.", Defaults.TemplateFolder));

				_activeMQSettings = new ActiveMQSettings(Path.Combine(_forumSettingsFolder, _fileNameActiveMQSettings));
				if (!string.IsNullOrEmpty(_activeMQSettings.LastInfo))
					AddEvent("Error while loading Active MQ settings: " + _activeMQSettings.LastInfo);
				else
					AddEvent("Active MQ settings loaded successfully.");

				_forumIdToImportedIdH = new Dictionary<int, HashSet<string>>();
				AccountInfoList = new List<AccountInfo>();

				if (File.Exists(Path.Combine(_forumSettingsFolder, _settingsFileName))) {
					string text = File.ReadAllText(Path.Combine(_forumSettingsFolder, _settingsFileName));
					HtmlAgilityPack.XmlDocument xmlDoc = new HtmlAgilityPack.XmlDocument();
					xmlDoc.LoadXml(text);
					var forumsNode = xmlDoc.DocumentNode.SelectSingleNode("/settings/forums");
					CheckEveryMin = forumsNode.GetAttributeValue("checkEveryMin", 5);
					SaveIdsEveryMin = forumsNode.GetAttributeValue("saveIdsEveryMin", 360);
					StartOnStartup = forumsNode.GetAttributeValue("startOnStartup", true);
					NrOfThreads = forumsNode.GetAttributeValue("nrOfThreads", 5);

					foreach (var account in forumsNode.SelectNodes("./forum") ?? new HtmlNodeCollection(null)) {
						int forumId = account.GetAttributeValue("forumId", -999);
						string address = account.GetAttributeValue("address", "");
						int checkBackDays = account.GetAttributeValue("checkBackDays", 1);
						string forumName = account.GetAttributeValue("forumName", "");

						if (forumId == -999) {
							ReportError("No \"forumId\" value was specified for forum " + address + ". Since the attribute value is mandatory the forum is currently not being indexed.");
							continue;
						}

						if (string.IsNullOrEmpty(address)) {
							ReportError("The \"address\" value is not specified or contains an invalid value for forum with id " + forumId.ToString() + ". The forum is currently not being indexed.");
							continue;
						}

						if (string.IsNullOrEmpty(forumName)) {
							ReportError("The forumName value for forum " + address + " is empty. The value is mandatory so the forum is currently not being indexed.");
							continue;
						}
						if (!(new[] { 0, 1, 7, 14, 30, 90, 180, 365 }).ToList().Contains(checkBackDays))
							ReportError("The phpBB forum supports only a limited number of values for checkBackDays. These values are 0 (for whole history), 1, 7, 14, 30, 90, 180 and 365. Please specify in the settings one of these values otherwise the results could be unexpected.");
						
						AccountInfoList.Add(new AccountInfo(forumId, forumName, address, checkBackDays));

						// load the file with aleready imported forum post ids
						lock (_postHashLock) {
							string importedIdsFile = string.Format(_importedIdsFileTemplate, forumId);
							if (File.Exists(Path.Combine(_forumSettingsFolder, importedIdsFile)))
								_forumIdToImportedIdH[forumId] = new HashSet<string>(File.ReadAllLines(Path.Combine(_forumSettingsFolder, importedIdsFile)));
							else
								_forumIdToImportedIdH[forumId] = new HashSet<string>();
						}
					}

					AddEvent(String.Format("The configuration file was read."));
					AddEvent(String.Format("Data will be monitored on {0} sources.", AccountInfoList.Count));
					AddEvent(String.Format("ActiveMQ Uri: {0}", _activeMQSettings.BrokerUri));
				}
				else
					AddEvent("The configuration file was not found. No settings were loaded");
			}
			catch (Exception ex) {
				GenLib.Log.LogService.LogException("Settings. LoadSettings failed.", ex);
				AddEvent("Settings. LoadSettings failed." + ex.Message);
			}
		}

		// save the settings to the config file
		public void SaveSettings()
		{
			string settingsText = String.Format(@"<?xml version=""1.0""?>
<settings>
	<forums checkEveryMin=""{0}"" SaveIdsEveryMin=""{1}"" startOnStartup=""{2}"" nrOfThreads=""{3}"" >", CheckEveryMin, SaveIdsEveryMin, StartOnStartup, NrOfThreads) + "\n";

			foreach (var account in AccountInfoList)
				settingsText += "\t\t" + String.Format(@"<forum forumId=""{0}"" forumName=""{1}"" address=""{2}"" checkBackDays=""{3}""  />", account.ForumId, account.ForumName, account.Address, account.CheckBackDays) + "\n";
			settingsText += "\t</forums>\n";
			settingsText += "</settings>";

			SaveImportedIds();
			try {
				// save active mq settings
				_activeMQSettings.SaveSettings(Path.Combine(_forumSettingsFolder, _fileNameActiveMQSettings));

				File.WriteAllText(Path.Combine(_forumSettingsFolder, _settingsFileName), settingsText);
			}
			catch (Exception ex) {
				GenLib.Log.LogService.LogException("Settings. SaveSettings failed.", ex);
				AddEvent("Settings. SaveSettings failed." + ex.Message);
			}
		}

		// save all imported forum post ids for each forum
		void SaveImportedIds()
		{
			try
			{
				lock (_postHashLock) {
					foreach (int forumId in _forumIdToImportedIdH.Keys) {
						string importedIdsFile = string.Format(_importedIdsFileTemplate, forumId);
						File.WriteAllLines(Path.Combine(_forumSettingsFolder, importedIdsFile), _forumIdToImportedIdH[forumId].ToList());
					}
				}
			}
			catch (Exception ex)
			{
				GenLib.Log.LogService.LogException("SaveImportedIds failed.", ex);
				AddEvent("SaveImportedIds failed." + ex.Message);
			}
		}
		#endregion

		#region misc functions
		/// <summary>
		/// remove previous qoutes from the specified text
		/// </summary>
		/// <param name="text"></param>
		/// <returns></returns>
		private string RemoveBlockQuotes(string text)
		{
			int start = text.IndexOf("<blockquote");
			int last = text.LastIndexOf("</blockquote>");
			if (start >= 0 && last >= 0 && start < last)
				text = text.Remove(start, last - start + "</blockquote>".Length);
			//Match m = Regex.Match(text, "<blockquote>.*</blockquote>");
			//if (!m.Success)
			//    return text;
			//string match = m.Value;
			//Debug.Assert(Regex.Matches(match, "<blockquote>").Count == Regex.Matches(match, "</blockquote>").Count);
			//text = text.Replace(match, "");
			Debug.Assert(!text.Contains("<blockquote"));
			Debug.Assert(!text.Contains("</blockquote>"));
			return text;
		}

#if UI
		private void ButtonBrowse_Click(object sender, EventArgs e)
		{
			Forms.FolderBrowserDialog dlg = new Forms.FolderBrowserDialog();
			if (dlg.ShowDialog() == Forms.DialogResult.OK)
			{
				TextBoxPath.Text = dlg.SelectedPath;
			}
		}

		void Form1_FormClosed(object sender, Forms.FormClosedEventArgs e)
		{
			Finish();
		}
#endif

		private void AddEvent(string text)
		{
#if UI
			if (!this.IsHandleCreated)
				return;
			this.Invoke((Action)(() =>
			{
				string[] lines = text.Split(new[] { '\n' }, StringSplitOptions.RemoveEmptyEntries);
				var lines2 = lines.Reverse();
				foreach (var line in lines2)
					ListBoxLog.Items.Insert(0, DateTime.Now + " " + line);
				while (ListBoxLog.Items.Count > 200)
					ListBoxLog.Items.RemoveAt(200);
			}));
#else
			Console.WriteLine(text);
#endif
		}
		#endregion
	}
}
